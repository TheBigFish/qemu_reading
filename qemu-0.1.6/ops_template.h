/*
 *  i386 micro operations (included several times to generate
 *  different operand sizes)
 * 
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#define DATA_BITS (1 << (3 + SHIFT))
#define SHIFT_MASK (DATA_BITS - 1)
#define SIGN_MASK (1 << (DATA_BITS - 1))

#if DATA_BITS == 8
#define SUFFIX b
#define DATA_TYPE uint8_t
#define DATA_STYPE int8_t
#define DATA_MASK 0xff
#elif DATA_BITS == 16
#define SUFFIX w
#define DATA_TYPE uint16_t
#define DATA_STYPE int16_t
#define DATA_MASK 0xffff
#elif DATA_BITS == 32
#define SUFFIX l
#define DATA_TYPE uint32_t
#define DATA_STYPE int32_t
#define DATA_MASK 0xffffffff
#else
#error unhandled operand size
#endif

/* dynamic flags computation */

static int glue(compute_all_add, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_SRC; /* 操作数1 */
    src2 = CC_DST - CC_SRC; /* 操作数2 */
    /* 计算进位标志,这个很有意思,如果结果比操作数1小,那说明溢出了,就有进位 */
    cf = (DATA_TYPE)CC_DST < (DATA_TYPE)src1; 
    pf = parity_table[(uint8_t)CC_DST];
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6; /* zf用于判断计算后的结果是否为0 */
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = lshift((src1 ^ src2 ^ -1) & (src1 ^ CC_DST), 12 - DATA_BITS) & CC_O;
    return cf | pf | af | zf | sf | of;
}
/* cf进位值 */
static int glue(compute_c_add, SUFFIX)(void)
{
    int src1, cf;
    src1 = CC_SRC;
    cf = (DATA_TYPE)CC_DST < (DATA_TYPE)src1;
    return cf;
}

/* adc是带进位的加法指令 */
static int glue(compute_all_adc, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_SRC; /* 加数1 */
    src2 = CC_DST - CC_SRC - 1; /* 加数2 */
    cf = (DATA_TYPE)CC_DST <= (DATA_TYPE)src1; /* carry flag */
    pf = parity_table[(uint8_t)CC_DST];
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = lshift((src1 ^ src2 ^ -1) & (src1 ^ CC_DST), 12 - DATA_BITS) & CC_O;
    return cf | pf | af | zf | sf | of;
}

static int glue(compute_c_adc, SUFFIX)(void)
{
    int src1, cf;
    src1 = CC_SRC;
    cf = (DATA_TYPE)CC_DST <= (DATA_TYPE)src1;
    return cf;
}

static int glue(compute_all_sub, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;
    cf = (DATA_TYPE)src1 < (DATA_TYPE)src2;
    pf = parity_table[(uint8_t)CC_DST];
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = lshift((src1 ^ src2) & (src1 ^ CC_DST), 12 - DATA_BITS) & CC_O;
    return cf | pf | af | zf | sf | of;
}

static int glue(compute_c_sub, SUFFIX)(void)
{
    int src1, src2, cf;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;
    cf = (DATA_TYPE)src1 < (DATA_TYPE)src2;
    return cf;
}

static int glue(compute_all_sbb, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST - 1;
    cf = (DATA_TYPE)src1 <= (DATA_TYPE)src2;
    pf = parity_table[(uint8_t)CC_DST];
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = lshift((src1 ^ src2) & (src1 ^ CC_DST), 12 - DATA_BITS) & CC_O;
    return cf | pf | af | zf | sf | of;
}

static int glue(compute_c_sbb, SUFFIX)(void)
{
    int src1, src2, cf;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST - 1;
    cf = (DATA_TYPE)src1 <= (DATA_TYPE)src2;
    return cf;
}

static int glue(compute_all_logic, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    cf = 0;
    pf = parity_table[(uint8_t)CC_DST];
    af = 0;
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = 0;
    return cf | pf | af | zf | sf | of;
}

static int glue(compute_c_logic, SUFFIX)(void)
{
    return 0;
}

static int glue(compute_all_inc, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_DST - 1; /* CC_DST应该是计算结果 */
    src2 = 1;
    cf = CC_SRC; /* 进位标志 */
    pf = parity_table[(uint8_t)CC_DST]; /* 奇偶位标志 */
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6; /* 零标志位 */
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = ((CC_DST & DATA_MASK) == SIGN_MASK) << 11;
    return cf | pf | af | zf | sf | of;
}

#if DATA_BITS == 32
static int glue(compute_c_inc, SUFFIX)(void)
{
    return CC_SRC;
}
#endif

static int glue(compute_all_dec, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    int src1, src2;
    src1 = CC_DST + 1;
    src2 = 1;
    cf = CC_SRC;
    pf = parity_table[(uint8_t)CC_DST];
    af = (CC_DST ^ src1 ^ src2) & 0x10;
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    of = ((CC_DST & DATA_MASK) == ((uint32_t)SIGN_MASK - 1)) << 11;
    return cf | pf | af | zf | sf | of;
}

static int glue(compute_all_shl, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    cf = (CC_SRC >> (DATA_BITS - 1)) & CC_C;
    pf = parity_table[(uint8_t)CC_DST];
    af = 0; /* undefined */
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    /* of is defined if shift count == 1 */
    of = lshift(CC_SRC ^ CC_DST, 12 - DATA_BITS) & CC_O;
    return cf | pf | af | zf | sf | of;
}

#if DATA_BITS == 32
static int glue(compute_c_shl, SUFFIX)(void)
{
    return CC_SRC & 1;
}
#endif

static int glue(compute_all_sar, SUFFIX)(void)
{
    int cf, pf, af, zf, sf, of;
    cf = CC_SRC & 1;
    pf = parity_table[(uint8_t)CC_DST];
    af = 0; /* undefined */
    zf = ((DATA_TYPE)CC_DST == 0) << 6;
    sf = lshift(CC_DST, 8 - DATA_BITS) & 0x80;
    /* of is defined if shift count == 1 */
    of = lshift(CC_SRC ^ CC_DST, 12 - DATA_BITS) & CC_O; 
    return cf | pf | af | zf | sf | of;
}

/* various optimized jumps cases */

void OPPROTO glue(op_jb_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    if ((DATA_TYPE)src1 < (DATA_TYPE)src2)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_jz_sub, SUFFIX)(void)
{
    if ((DATA_TYPE)CC_DST == 0)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_jbe_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    if ((DATA_TYPE)src1 <= (DATA_TYPE)src2)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_js_sub, SUFFIX)(void)
{
    if (CC_DST & SIGN_MASK)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_jl_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    if ((DATA_STYPE)src1 < (DATA_STYPE)src2)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_jle_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    if ((DATA_STYPE)src1 <= (DATA_STYPE)src2)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

/* oldies */

#if DATA_BITS >= 16

void OPPROTO glue(op_loopnz, SUFFIX)(void)
{
    unsigned int tmp;
    int eflags;
    eflags = cc_table[CC_OP].compute_all();
    tmp = (ECX - 1) & DATA_MASK;
    ECX = (ECX & ~DATA_MASK) | tmp;
    if (tmp != 0 && !(eflags & CC_Z))
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_loopz, SUFFIX)(void)
{
    unsigned int tmp;
    int eflags;
    eflags = cc_table[CC_OP].compute_all();
    tmp = (ECX - 1) & DATA_MASK;
    ECX = (ECX & ~DATA_MASK) | tmp;
    if (tmp != 0 && (eflags & CC_Z))
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_loop, SUFFIX)(void)
{
    unsigned int tmp;
    tmp = (ECX - 1) & DATA_MASK;
    ECX = (ECX & ~DATA_MASK) | tmp;
    if (tmp != 0)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

void OPPROTO glue(op_jecxz, SUFFIX)(void)
{
    if ((DATA_TYPE)ECX == 0)
        EIP = PARAM1;
    else
        EIP = PARAM2;
    FORCE_RET();
}

#endif

/* various optimized set cases */

void OPPROTO glue(op_setb_T0_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    T0 = ((DATA_TYPE)src1 < (DATA_TYPE)src2);
}

void OPPROTO glue(op_setz_T0_sub, SUFFIX)(void)
{
    T0 = ((DATA_TYPE)CC_DST == 0);
}

void OPPROTO glue(op_setbe_T0_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    T0 = ((DATA_TYPE)src1 <= (DATA_TYPE)src2);
}

void OPPROTO glue(op_sets_T0_sub, SUFFIX)(void)
{
    T0 = lshift(CC_DST, -(DATA_BITS - 1)) & 1;
}

void OPPROTO glue(op_setl_T0_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    T0 = ((DATA_STYPE)src1 < (DATA_STYPE)src2);
}

void OPPROTO glue(op_setle_T0_sub, SUFFIX)(void)
{
    int src1, src2;
    src1 = CC_SRC;
    src2 = CC_SRC - CC_DST;

    T0 = ((DATA_STYPE)src1 <= (DATA_STYPE)src2);
}

/* shifts */

void OPPROTO glue(glue(op_rol, SUFFIX), _T0_T1_cc)(void)
{
    int count, src;
    count = T1 & SHIFT_MASK;
    if (count) {
        CC_SRC = cc_table[CC_OP].compute_all() & ~(CC_O | CC_C);
        src = T0;
        T0 &= DATA_MASK;
        T0 = (T0 << count) | (T0 >> (DATA_BITS - count));
        CC_SRC |= (lshift(src ^ T0, 11 - (DATA_BITS - 1)) & CC_O) | 
            (T0 & CC_C);
        CC_OP = CC_OP_EFLAGS;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_rol, SUFFIX), _T0_T1)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    if (count) {
        T0 &= DATA_MASK;
        T0 = (T0 << count) | (T0 >> (DATA_BITS - count));
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_ror, SUFFIX), _T0_T1_cc)(void)
{
    int count, src;
    count = T1 & SHIFT_MASK;
    if (count) {
        CC_SRC = cc_table[CC_OP].compute_all() & ~(CC_O | CC_C);
        src = T0;
        T0 &= DATA_MASK;
        T0 = (T0 >> count) | (T0 << (DATA_BITS - count));
        CC_SRC |= (lshift(src ^ T0, 11 - (DATA_BITS - 1)) & CC_O) | 
            ((T0 >> (DATA_BITS - 1)) & CC_C);
        CC_OP = CC_OP_EFLAGS;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_ror, SUFFIX), _T0_T1)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    if (count) {
        T0 &= DATA_MASK;
        T0 = (T0 >> count) | (T0 << (DATA_BITS - count));
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_rcl, SUFFIX), _T0_T1_cc)(void)
{
    int count, res, eflags;
    unsigned int src;

    count = T1 & 0x1f;
#if DATA_BITS == 16
    count = rclw_table[count];
#elif DATA_BITS == 8
    count = rclb_table[count];
#endif
    if (count) {
        eflags = cc_table[CC_OP].compute_all();
        T0 &= DATA_MASK;
        src = T0;
        res = (T0 << count) | ((eflags & CC_C) << (count - 1));
        if (count > 1)
            res |= T0 >> (DATA_BITS + 1 - count);
        T0 = res;
        CC_SRC = (eflags & ~(CC_C | CC_O)) |
            (lshift(src ^ T0, 11 - (DATA_BITS - 1)) & CC_O) | 
            ((src >> (DATA_BITS - count)) & CC_C);
        CC_OP = CC_OP_EFLAGS;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_rcr, SUFFIX), _T0_T1_cc)(void)
{
    int count, res, eflags;
    unsigned int src;

    count = T1 & 0x1f;
#if DATA_BITS == 16
    count = rclw_table[count];
#elif DATA_BITS == 8
    count = rclb_table[count];
#endif
    if (count) {
        eflags = cc_table[CC_OP].compute_all();
        T0 &= DATA_MASK;
        src = T0;
        res = (T0 >> count) | ((eflags & CC_C) << (DATA_BITS - count));
        if (count > 1)
            res |= T0 << (DATA_BITS + 1 - count);
        T0 = res;
        CC_SRC = (eflags & ~(CC_C | CC_O)) |
            (lshift(src ^ T0, 11 - (DATA_BITS - 1)) & CC_O) | 
            ((src >> (count - 1)) & CC_C);
        CC_OP = CC_OP_EFLAGS;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_shl, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & 0x1f;
    if (count) {
        CC_SRC = (DATA_TYPE)T0 << (count - 1);
        T0 = T0 << count;
        CC_DST = T0;
        CC_OP = CC_OP_SHLB + SHIFT;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_shl, SUFFIX), _T0_T1)(void)
{
    int count;
    count = T1 & 0x1f;
    T0 = T0 << count;
    FORCE_RET();
}

void OPPROTO glue(glue(op_shr, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & 0x1f;
    if (count) {
        T0 &= DATA_MASK;
        CC_SRC = T0 >> (count - 1);
        T0 = T0 >> count;
        CC_DST = T0;
        CC_OP = CC_OP_SARB + SHIFT;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_shr, SUFFIX), _T0_T1)(void)
{
    int count;
    count = T1 & 0x1f;
    T0 &= DATA_MASK;
    T0 = T0 >> count;
    FORCE_RET();
}

void OPPROTO glue(glue(op_sar, SUFFIX), _T0_T1_cc)(void)
{
    int count, src;
    count = T1 & 0x1f;
    if (count) {
        src = (DATA_STYPE)T0;
        CC_SRC = src >> (count - 1);
        T0 = src >> count;
        CC_DST = T0;
        CC_OP = CC_OP_SARB + SHIFT;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_sar, SUFFIX), _T0_T1)(void)
{
    int count, src;
    count = T1 & 0x1f;
    src = (DATA_STYPE)T0;
    T0 = src >> count;
    FORCE_RET();
}

#if DATA_BITS == 16
/* XXX: overflow flag might be incorrect in some cases in shldw */
void OPPROTO glue(glue(op_shld, SUFFIX), _T0_T1_im_cc)(void)
{
    int count;
    unsigned int res;
    count = PARAM1;
    T1 &= 0xffff;
    res = T1 | (T0 << 16);
    CC_SRC = res >> (32 - count);
    res <<= count;
    if (count > 16)
        res |= T1 << (count - 16);
    T0 = res >> 16;
    CC_DST = T0;
}

void OPPROTO glue(glue(op_shld, SUFFIX), _T0_T1_ECX_cc)(void)
{
    int count;
    unsigned int res;
    count = ECX & 0x1f;
    if (count) {
        T1 &= 0xffff;
        res = T1 | (T0 << 16);
        CC_SRC = res >> (32 - count);
        res <<= count;
        if (count > 16)
          res |= T1 << (count - 16);
        T0 = res >> 16;
        CC_DST = T0;
        CC_OP = CC_OP_SARB + SHIFT;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_shrd, SUFFIX), _T0_T1_im_cc)(void)
{
    int count;
    unsigned int res;

    count = PARAM1;
    res = (T0 & 0xffff) | (T1 << 16);
    CC_SRC = res >> (count - 1);
    res >>= count;
    if (count > 16)
        res |= T1 << (32 - count);
    T0 = res;
    CC_DST = T0;
}


void OPPROTO glue(glue(op_shrd, SUFFIX), _T0_T1_ECX_cc)(void)
{
    int count;
    unsigned int res;

    count = ECX & 0x1f;
    if (count) {
        res = (T0 & 0xffff) | (T1 << 16);
        CC_SRC = res >> (count - 1);
        res >>= count;
        if (count > 16)
            res |= T1 << (32 - count);
        T0 = res;
        CC_DST = T0;
        CC_OP = CC_OP_SARB + SHIFT;
    }
    FORCE_RET();
}
#endif

#if DATA_BITS == 32
void OPPROTO glue(glue(op_shld, SUFFIX), _T0_T1_im_cc)(void)
{
    int count;
    count = PARAM1;
    T0 &= DATA_MASK;
    T1 &= DATA_MASK;
    CC_SRC = T0 << (count - 1);
    T0 = (T0 << count) | (T1 >> (DATA_BITS - count));
    CC_DST = T0;
}

void OPPROTO glue(glue(op_shld, SUFFIX), _T0_T1_ECX_cc)(void)
{
    int count;
    count = ECX & 0x1f;
    if (count) {
        T0 &= DATA_MASK;
        T1 &= DATA_MASK;
        CC_SRC = T0 << (count - 1);
        T0 = (T0 << count) | (T1 >> (DATA_BITS - count));
        CC_DST = T0;
        CC_OP = CC_OP_SHLB + SHIFT;
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_shrd, SUFFIX), _T0_T1_im_cc)(void)
{
    int count;
    count = PARAM1;
    T0 &= DATA_MASK;
    T1 &= DATA_MASK;
    CC_SRC = T0 >> (count - 1);
    T0 = (T0 >> count) | (T1 << (DATA_BITS - count));
    CC_DST = T0;
}


void OPPROTO glue(glue(op_shrd, SUFFIX), _T0_T1_ECX_cc)(void)
{
    int count;
    count = ECX & 0x1f;
    if (count) {
        T0 &= DATA_MASK;
        T1 &= DATA_MASK;
        CC_SRC = T0 >> (count - 1);
        T0 = (T0 >> count) | (T1 << (DATA_BITS - count));
        CC_DST = T0;
        CC_OP = CC_OP_SARB + SHIFT;
    }
    FORCE_RET();
}
#endif

/* carry add/sub (we only need to set CC_OP differently) */

void OPPROTO glue(glue(op_adc, SUFFIX), _T0_T1_cc)(void)
{
    int cf;
    cf = cc_table[CC_OP].compute_c();
    CC_SRC = T0;
    T0 = T0 + T1 + cf;
    CC_DST = T0;
    CC_OP = CC_OP_ADDB + SHIFT + cf * 3;
}

void OPPROTO glue(glue(op_sbb, SUFFIX), _T0_T1_cc)(void)
{
    int cf;
    cf = cc_table[CC_OP].compute_c();
    CC_SRC = T0;
    T0 = T0 - T1 - cf;
    CC_DST = T0;
    CC_OP = CC_OP_SUBB + SHIFT + cf * 3;
}

void OPPROTO glue(glue(op_cmpxchg, SUFFIX), _T0_T1_EAX_cc)(void)
{
    CC_SRC = EAX;
    CC_DST = EAX - T0;
    if ((DATA_TYPE)CC_DST == 0) {
        T0 = T1;
    } else {
        EAX = (EAX & ~DATA_MASK) | (T0 & DATA_MASK);
    }
    FORCE_RET();
}

/* bit operations */
#if DATA_BITS >= 16

void OPPROTO glue(glue(op_bt, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    CC_SRC = T0 >> count;
}

void OPPROTO glue(glue(op_bts, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    CC_SRC = T0 >> count;
    T0 |= (1 << count);
}

void OPPROTO glue(glue(op_btr, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    CC_SRC = T0 >> count;
    T0 &= ~(1 << count);
}

void OPPROTO glue(glue(op_btc, SUFFIX), _T0_T1_cc)(void)
{
    int count;
    count = T1 & SHIFT_MASK;
    CC_SRC = T0 >> count;
    T0 ^= (1 << count);
}

void OPPROTO glue(glue(op_bsf, SUFFIX), _T0_cc)(void)
{
    int res, count;
    res = T0 & DATA_MASK;
    if (res != 0) {
        count = 0;
        while ((res & 1) == 0) {
            count++;
            res >>= 1;
        }
        T0 = count;
        CC_DST = 1; /* ZF = 1 */
    } else {
        CC_DST = 0; /* ZF = 1 */
    }
    FORCE_RET();
}

void OPPROTO glue(glue(op_bsr, SUFFIX), _T0_cc)(void)
{
    int res, count;
    res = T0 & DATA_MASK;
    if (res != 0) {
        count = DATA_BITS - 1;
        while ((res & SIGN_MASK) == 0) {
            count--;
            res <<= 1;
        }
        T0 = count;
        CC_DST = 1; /* ZF = 1 */
    } else {
        CC_DST = 0; /* ZF = 1 */
    }
    FORCE_RET();
}

#endif

/* string operations */
/* XXX: maybe use lower level instructions to ease 16 bit / segment handling */

#define STRING_SUFFIX _fast
#define SI_ADDR (void *)ESI
#define DI_ADDR (void *)EDI
#define INC_SI() ESI += inc
#define INC_DI() EDI += inc
#define CX ECX
#define DEC_CX() ECX--
#include "op_string.h"

#define STRING_SUFFIX _a32
#define SI_ADDR (uint8_t *)A0 + ESI
#define DI_ADDR env->seg_cache[R_ES].base + EDI
#define INC_SI() ESI += inc
#define INC_DI() EDI += inc
#define CX ECX
#define DEC_CX() ECX--
#include "op_string.h"

#define STRING_SUFFIX _a16
#define SI_ADDR (uint8_t *)A0 + (ESI & 0xffff)
#define DI_ADDR env->seg_cache[R_ES].base + (EDI & 0xffff)
#define INC_SI() ESI = (ESI & ~0xffff) | ((ESI + inc) & 0xffff)
#define INC_DI() EDI = (EDI & ~0xffff) | ((EDI + inc) & 0xffff)
#define CX (ECX & 0xffff)
#define DEC_CX() ECX = (ECX & ~0xffff) | ((ECX - 1) & 0xffff)
#include "op_string.h"

/* port I/O */

void OPPROTO glue(glue(op_out, SUFFIX), _T0_T1)(void)
{
    glue(cpu_x86_out, SUFFIX)(T0 & 0xffff, T1 & DATA_MASK);
}

void OPPROTO glue(glue(op_in, SUFFIX), _T0_T1)(void)
{
    T1 = glue(cpu_x86_in, SUFFIX)(T0 & 0xffff);
}

#undef DATA_BITS
#undef SHIFT_MASK
#undef SIGN_MASK
#undef DATA_TYPE
#undef DATA_STYPE
#undef DATA_MASK
#undef SUFFIX
