/*
 * QEMU DMA emulation
 *
 * Copyright (c) 2003-2004 Vassili Karpov (malc)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "hw.h"
#include "isa.h"

/* #define DEBUG_DMA */

#define dolog(...) fprintf (stderr, "dma: " __VA_ARGS__)
#ifdef DEBUG_DMA
#define lwarn(...) fprintf (stderr, "dma: " __VA_ARGS__)
#define linfo(...) fprintf (stderr, "dma: " __VA_ARGS__)
#define ldebug(...) fprintf (stderr, "dma: " __VA_ARGS__)
#else
#define lwarn(...)
#define linfo(...)
#define ldebug(...)
#endif

struct dma_regs {
    int now[2];
    uint16_t base[2];
    uint8_t mode; /* 模式寄存器 */
    /* page以及pageh以及now[0]用于构建32bit的目标地址 */
    uint8_t page;
    uint8_t pageh;
    uint8_t dack;
    uint8_t eop;
    DMA_transfer_handler transfer_handler; /* dma传输函数 */
    void *opaque;
};

#define ADDR 0
#define COUNT 1

/* dma控制器 */
static struct dma_cont {
    uint8_t status; /* 状态寄存器 */
    uint8_t command; /* 命令寄存器 */
    uint8_t mask; /* 屏蔽寄存器 */
    /* flip_flop为0,则读/写低字节,否则读/些高字节 */
    uint8_t flip_flop; /* 触位器 */
    int dshift;
    struct dma_regs regs[4];
} dma_controllers[2];

enum {
    CMD_MEMORY_TO_MEMORY = 0x01,
    CMD_FIXED_ADDRESS    = 0x02,
    CMD_BLOCK_CONTROLLER = 0x04,
    CMD_COMPRESSED_TIME  = 0x08,
    CMD_CYCLIC_PRIORITY  = 0x10,
    CMD_EXTENDED_WRITE   = 0x20,
    CMD_LOW_DREQ         = 0x40,
    CMD_LOW_DACK         = 0x80,
    CMD_NOT_SUPPORTED    = CMD_MEMORY_TO_MEMORY | CMD_FIXED_ADDRESS
    | CMD_COMPRESSED_TIME | CMD_CYCLIC_PRIORITY | CMD_EXTENDED_WRITE
    | CMD_LOW_DREQ | CMD_LOW_DACK

};

static void DMA_run (void);

static int channels[8] = {-1, 2, 3, 1, -1, -1, -1, 0};

static void write_page (void *opaque, uint32_t nport, uint32_t data)
{
    struct dma_cont *d = opaque;
    int ichan;

    ichan = channels[nport & 7]; /* 选择通道 */
    if (-1 == ichan) {
        dolog ("invalid channel %#x %#x\n", nport, data);
        return;
    }
    d->regs[ichan].page = data; /* 页号的低字节 */
}

static void write_pageh (void *opaque, uint32_t nport, uint32_t data)
{
    struct dma_cont *d = opaque;
    int ichan;

    ichan = channels[nport & 7];
    if (-1 == ichan) {
        dolog ("invalid channel %#x %#x\n", nport, data);
        return;
    }
    d->regs[ichan].pageh = data; /* 页号的高字节 */
}

static uint32_t read_page (void *opaque, uint32_t nport)
{
    struct dma_cont *d = opaque;
    int ichan;

    ichan = channels[nport & 7];
    if (-1 == ichan) {
        dolog ("invalid channel read %#x\n", nport);
        return 0;
    }
    return d->regs[ichan].page;
}

static uint32_t read_pageh (void *opaque, uint32_t nport)
{
    struct dma_cont *d = opaque;
    int ichan;

    ichan = channels[nport & 7];
    if (-1 == ichan) {
        dolog ("invalid channel read %#x\n", nport);
        return 0;
    }
    return d->regs[ichan].pageh;
}

/* dma通道的初始化
 * @param ichan 通道号
 */
static inline void init_chan (struct dma_cont *d, int ichan)
{
    struct dma_regs *r;

    r = d->regs + ichan;
    r->now[ADDR] = r->base[ADDR] << d->dshift; /* 记录下传输地址 */
    r->now[COUNT] = 0;
}

/* 获取flip_flop的值,这个值每次都会翻转 */
static inline int getff (struct dma_cont *d)
{
    int ff;

    ff = d->flip_flop;
    d->flip_flop = !ff;
    return ff;
}

static uint32_t read_chan (void *opaque, uint32_t nport)
{
    struct dma_cont *d = opaque;
    int ichan, nreg, iport, ff, val, dir;
    struct dma_regs *r;

    iport = (nport >> d->dshift) & 0x0f;
    ichan = iport >> 1;
    nreg = iport & 1;
    r = d->regs + ichan;

    dir = ((r->mode >> 5) & 1) ? -1 : 1;
    ff = getff (d);
    if (nreg)
        val = (r->base[COUNT] << d->dshift) - r->now[COUNT];
    else
        val = r->now[ADDR] + r->now[COUNT] * dir;

    ldebug ("read_chan %#x -> %d\n", iport, val);
    return (val >> (d->dshift + (ff << 3))) & 0xff;
}

/* 设置通道 */
static void write_chan (void *opaque, uint32_t nport, uint32_t data)
{
    struct dma_cont *d = opaque;
    int iport, ichan, nreg;
    struct dma_regs *r;

    iport = (nport >> d->dshift) & 0x0f;
    ichan = iport >> 1;
    nreg = iport & 1;
    r = d->regs + ichan;
    if (getff (d)) { /* 读/写高字节 */
        r->base[nreg] = (r->base[nreg] & 0xff) | ((data << 8) & 0xff00);
        init_chan (d, ichan);
    } else { /* 读/写低字节 */
        r->base[nreg] = (r->base[nreg] & 0xff00) | (data & 0xff);
    }
}

/* 设置dma控制器 */
static void write_cont (void *opaque, uint32_t nport, uint32_t data)
{
    struct dma_cont *d = opaque;
    int iport, ichan = 0;

    iport = (nport >> d->dshift) & 0x0f;
    switch (iport) {
    case 0x08:                  /* command */
        if ((data != 0) && (data & CMD_NOT_SUPPORTED)) {
            dolog ("command %#x not supported\n", data);
            return;
        }
        d->command = data; /* 设置命令寄存器 */
        break;

    case 0x09:
        ichan = data & 3; /* 通道 */
        if (data & 4) {
            d->status |= 1 << (ichan + 4); /* 设置dma请求 */
        }
        else {
            d->status &= ~(1 << (ichan + 4)); /* 清除dma请求 */
        }
        d->status &= ~(1 << ichan);
        DMA_run();
        break;

    case 0x0a:                  /* single mask */
        if (data & 4)
            d->mask |= 1 << (data & 3);
        else
            d->mask &= ~(1 << (data & 3));
        DMA_run();
        break;

    case 0x0b:                  /* mode */
        {
            ichan = data & 3; /* 获取通道编号 */
#ifdef DEBUG_DMA
            {
                int op, ai, dir, opmode;
               /* 00 校验传送
                * 01 写传送
                * 10 读传送
                * 11 非法
                */
                op = (data >> 2) & 3;
                ai = (data >> 4) & 1;  /* 0表示禁止自动初始化,否则允许自动初始化 */
                dir = (data >> 5) & 1; /* 0表示地址+1,1表示地址-1 */
               /* 00 请求传送方式 
                * 01 单字节传送方式
                * 10 块传送方式
                * 11 级联方式
                */
                opmode = (data >> 6) & 3;

                linfo ("ichan %d, op %d, ai %d, dir %d, opmode %d\n",
                       ichan, op, ai, dir, opmode);
            }
#endif
            d->regs[ichan].mode = data; /* 设置模式 */
            break;
        }

    case 0x0c:                  /* clear flip flop */
        d->flip_flop = 0;
        break;

    case 0x0d:                  /* reset */
        d->flip_flop = 0;
        d->mask = ~0;
        d->status = 0;
        d->command = 0;
        break;

    case 0x0e:                  /* clear mask for all channels */
        d->mask = 0;
        DMA_run();
        break;

    case 0x0f:                  /* write mask for all channels */
        d->mask = data;
        DMA_run();
        break;

    default:
        dolog ("unknown iport %#x\n", iport);
        break;
    }

#ifdef DEBUG_DMA
    if (0xc != iport) {
        linfo ("write_cont: nport %#06x, ichan % 2d, val %#06x\n",
               nport, ichan, data);
    }
#endif
}

static uint32_t read_cont (void *opaque, uint32_t nport)
{
    struct dma_cont *d = opaque;
    int iport, val;

    iport = (nport >> d->dshift) & 0x0f;
    switch (iport) {
    case 0x08:                  /* status */
        val = d->status;
        d->status &= 0xf0;
        break;
    case 0x0f:                  /* mask */
        val = d->mask;
        break;
    default:
        val = 0;
        break;
    }

    ldebug ("read_cont: nport %#06x, iport %#04x val %#x\n", nport, iport, val);
    return val;
}

/* 读取传输模式 */
int DMA_get_channel_mode (int nchan)
{
    return dma_controllers[nchan > 3].regs[nchan & 3].mode;
}

void DMA_hold_DREQ (int nchan)
{
    int ncont, ichan;

    ncont = nchan > 3;
    ichan = nchan & 3;
    linfo ("held cont=%d chan=%d\n", ncont, ichan);
    dma_controllers[ncont].status |= 1 << (ichan + 4); /* ichan号通道发生dma请求 */
    DMA_run();
}

void DMA_release_DREQ (int nchan)
{
    int ncont, ichan;

    ncont = nchan > 3;
    ichan = nchan & 3;
    linfo ("released cont=%d chan=%d\n", ncont, ichan);
    dma_controllers[ncont].status &= ~(1 << (ichan + 4));
    DMA_run();
}

static void channel_run (int ncont, int ichan)
{
    int n;
    struct dma_regs *r = &dma_controllers[ncont].regs[ichan];
#ifdef DEBUG_DMA
    int dir, opmode;

    dir = (r->mode >> 5) & 1; /* 方向 */
    opmode = (r->mode >> 6) & 3;

    if (dir) {
        dolog ("DMA in address decrement mode\n");
    }
    if (opmode != 1) {
        dolog ("DMA not in single mode select %#x\n", opmode);
    }
#endif

    r = dma_controllers[ncont].regs + ichan; /* 获得通道的寄存器 */
    /* 开始数据传输 */
    n = r->transfer_handler (r->opaque, ichan + (ncont << 2),
                             r->now[COUNT], (r->base[COUNT] + 1) << ncont);
    r->now[COUNT] = n; /* 记录已经传输的字节数目 */
    ldebug ("dma_pos %d size %d\n", n, (r->base[COUNT] + 1) << ncont);
}

static QEMUBH *dma_bh;

/* 运行DMA */
static void DMA_run (void)
{
    struct dma_cont *d;
    int icont, ichan;
    int rearm = 0;

    d = dma_controllers;

    for (icont = 0; icont < 2; icont++, d++) {
        for (ichan = 0; ichan < 4; ichan++) { /* 一共4个通道 */
            int mask;
            mask = 1 << ichan;
            /* d->mask & mask为0表示ichan这个通道的请求没有被屏蔽掉
             * d->status & (mask << 4)不为0说明ichan通道存在请求
             */
            if ((0 == (d->mask & mask)) && (0 != (d->status & (mask << 4)))) {
                channel_run (icont, ichan);
                rearm = 1;
            }
        }
    }

    if (rearm)
        qemu_bh_schedule_idle(dma_bh);
}

static void DMA_run_bh(void *unused)
{
    DMA_run();
}

/* 注册通道
* @param nchan 通道号
 * @param transfer_handler 回调函数
 */
void DMA_register_channel (int nchan,
                           DMA_transfer_handler transfer_handler,
                           void *opaque)
{
    struct dma_regs *r;
    int ichan, ncont;

    ncont = nchan > 3; /* dma控制器编号,因为1个dma控制仅有4个通道 */
    ichan = nchan & 3; /* dma通道编号 */

    r = dma_controllers[ncont].regs + ichan;
    r->transfer_handler = transfer_handler; /* 记录下回调函数 */
    r->opaque = opaque;
}

/* DMA执行读取动作,也就是将数据读取到buf之中
 * @param nchan 通道编号
 * @param buf 目的
 * @param pos 偏移
 * @param len 要读取的数据的长度
 */
int DMA_read_memory (int nchan, void *buf, int pos, int len)
{
    struct dma_regs *r = &dma_controllers[nchan > 3].regs[nchan & 3];
    target_phys_addr_t addr = ((r->pageh & 0x7f) << 24) | (r->page << 16) | r->now[ADDR];

    if (r->mode & 0x20) {
        int i;
        uint8_t *p = buf;

        cpu_physical_memory_read (addr - pos - len, buf, len);
        /* What about 16bit transfers? */
        for (i = 0; i < len >> 1; i++) {
            uint8_t b = p[len - i - 1];
            p[i] = b;
        }
    }
    else
        cpu_physical_memory_read (addr + pos, buf, len);

    return len;
}

/* DMA执行写操作,也就是将buf中存放的长度为len的数据写入到对应的内存处
 * @param nchan 通道编号
 * @param buf 要拷贝的数据存放的位置
 * @param pos 偏移
 * @param len 要拷贝的数据的长度
 */
int DMA_write_memory (int nchan, void *buf, int pos, int len)
{
    struct dma_regs *r = &dma_controllers[nchan > 3].regs[nchan & 3];
    /* 构建目标地址, pageh取7bit,page取8bit,now[0]取16bit */
    target_phys_addr_t addr = ((r->pageh & 0x7f) << 24) | (r->page << 16) | r->now[ADDR];

    if (r->mode & 0x20) {
        int i;
        uint8_t *p = buf;
        /* 从buf开始拷贝len字节的数据到addr-pos-len开始的内存处 */
        cpu_physical_memory_write (addr - pos - len, buf, len);
        /* What about 16bit transfers? */
        for (i = 0; i < len; i++) {
            uint8_t b = p[len - i - 1];
            p[i] = b;
        }
    }
    else
        cpu_physical_memory_write (addr + pos, buf, len);

    return len;
}

/* request the emulator to transfer a new DMA memory block ASAP */
void DMA_schedule(int nchan)
{
    CPUState *env = cpu_single_env;
    if (env)
        cpu_interrupt(env, CPU_INTERRUPT_EXIT);
}

static void dma_reset(void *opaque)
{
    struct dma_cont *d = opaque;
    write_cont (d, (0x0d << d->dshift), 0);
}

static int dma_phony_handler (void *opaque, int nchan, int dma_pos, int dma_len)
{
    dolog ("unregistered DMA channel used nchan=%d dma_pos=%d dma_len=%d\n",
           nchan, dma_pos, dma_len);
    return dma_pos;
}

/* dshift = 0: 8 bit DMA, 1 = 16 bit DMA */
static void dma_init2(struct dma_cont *d, int base, int dshift,
                      int page_base, int pageh_base)
{
    static const int page_port_list[] = { 0x1, 0x2, 0x3, 0x7 };
    int i;

    d->dshift = dshift;
    for (i = 0; i < 8; i++) {
        register_ioport_write (base + (i << dshift), 1, 1, write_chan, d);
        register_ioport_read (base + (i << dshift), 1, 1, read_chan, d);
    }
    for (i = 0; i < ARRAY_SIZE (page_port_list); i++) {
        register_ioport_write (page_base + page_port_list[i], 1, 1,
                               write_page, d);
        register_ioport_read (page_base + page_port_list[i], 1, 1,
                              read_page, d);
        if (pageh_base >= 0) {
            register_ioport_write (pageh_base + page_port_list[i], 1, 1,
                                   write_pageh, d);
            register_ioport_read (pageh_base + page_port_list[i], 1, 1,
                                  read_pageh, d);
        }
    }
    for (i = 0; i < 8; i++) {
        register_ioport_write (base + ((i + 8) << dshift), 1, 1,
                               write_cont, d);
        register_ioport_read (base + ((i + 8) << dshift), 1, 1,
                              read_cont, d);
    }
    qemu_register_reset(dma_reset, d);
    dma_reset(d);
    for (i = 0; i < ARRAY_SIZE (d->regs); ++i) {
        d->regs[i].transfer_handler = dma_phony_handler;
    }
}

static void dma_save (QEMUFile *f, void *opaque)
{
    struct dma_cont *d = opaque;
    int i;

    /* qemu_put_8s (f, &d->status); */
    qemu_put_8s (f, &d->command);
    qemu_put_8s (f, &d->mask);
    qemu_put_8s (f, &d->flip_flop);
    qemu_put_be32 (f, d->dshift);

    for (i = 0; i < 4; ++i) {
        struct dma_regs *r = &d->regs[i];
        qemu_put_be32 (f, r->now[0]);
        qemu_put_be32 (f, r->now[1]);
        qemu_put_be16s (f, &r->base[0]);
        qemu_put_be16s (f, &r->base[1]);
        qemu_put_8s (f, &r->mode);
        qemu_put_8s (f, &r->page);
        qemu_put_8s (f, &r->pageh);
        qemu_put_8s (f, &r->dack);
        qemu_put_8s (f, &r->eop);
    }
}

static int dma_load (QEMUFile *f, void *opaque, int version_id)
{
    struct dma_cont *d = opaque;
    int i;

    if (version_id != 1)
        return -EINVAL;

    /* qemu_get_8s (f, &d->status); */
    qemu_get_8s (f, &d->command);
    qemu_get_8s (f, &d->mask);
    qemu_get_8s (f, &d->flip_flop);
    d->dshift=qemu_get_be32 (f);

    for (i = 0; i < 4; ++i) {
        struct dma_regs *r = &d->regs[i];
        r->now[0]=qemu_get_be32 (f);
        r->now[1]=qemu_get_be32 (f);
        qemu_get_be16s (f, &r->base[0]);
        qemu_get_be16s (f, &r->base[1]);
        qemu_get_8s (f, &r->mode);
        qemu_get_8s (f, &r->page);
        qemu_get_8s (f, &r->pageh);
        qemu_get_8s (f, &r->dack);
        qemu_get_8s (f, &r->eop);
    }

    DMA_run();

    return 0;
}

/* dma初始化 */
void DMA_init (int high_page_enable)
{
    dma_init2(&dma_controllers[0], 0x00, 0, 0x80,
              high_page_enable ? 0x480 : -1);
    dma_init2(&dma_controllers[1], 0xc0, 1, 0x88,
              high_page_enable ? 0x488 : -1);
    register_savevm ("dma", 0, 1, dma_save, dma_load, &dma_controllers[0]);
    register_savevm ("dma", 1, 1, dma_save, dma_load, &dma_controllers[1]);

    dma_bh = qemu_bh_new(DMA_run_bh, NULL);
}
