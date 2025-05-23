/* This is the Linux kernel elf-loading code, ported into user space */

#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "qemu.h"

#ifdef TARGET_I386

#define ELF_START_MMAP 0x80000000

typedef uint32_t  elf_greg_t;

#define ELF_NGREG (sizeof (struct target_pt_regs) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct user_i387_struct elf_fpregset_t;

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) ( ((x) == EM_386) || ((x) == EM_486) )

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS   ELFCLASS32
#define ELF_DATA    ELFDATA2LSB
#define ELF_ARCH    EM_386

/* SVR4/i386 ABI (pages 3-31, 3-32) says that when the program
   starts %edx contains a pointer to a function which might be
   registered using `atexit'.  This provides a mean for the
   dynamic linker to call DT_FINI functions for shared libraries
   that have been loaded before the code runs.

   A value of 0 tells we have no such handler.  */
#define ELF_PLAT_INIT(_r)   _r->edx = 0

#define USE_ELF_CORE_DUMP
#define ELF_EXEC_PAGESIZE   4096

#endif

#include "elf.h"

/*
 * MAX_ARG_PAGES defines the number of pages allocated for arguments
 * and envelope for the new program. 32 should suffice, this gives
 * a maximum env+arg of 128kB w/4KB pages!
 */
#define MAX_ARG_PAGES 32

/*
 * This structure is used to hold the arguments that are
 * used when loading binaries.
 * 此结构体用于记录加载二进制文件时所持有的参数信息
 */
struct linux_binprm
{
    char buf[128];
    unsigned long page[MAX_ARG_PAGES];
    unsigned long p;
    int sh_bang;
    int fd;
    int e_uid, e_gid;
    int argc; /* 参数个数 */
    int envc;
    /* 二进制文件的名称 */
    char * filename;        /* Name of binary */
    unsigned long loader;
    unsigned long exec;
    int dont_iput;          /* binfmt handler has put inode */
};

struct exec
{
    unsigned int a_info;   /* Use macros N_MAGIC, etc for access */
    unsigned int a_text;   /* length of text, in bytes */
    unsigned int a_data;   /* length of data, in bytes */
    unsigned int a_bss;    /* length of uninitialized data area, in bytes */
    unsigned int a_syms;   /* length of symbol table data in file, in bytes */
    unsigned int a_entry;  /* start address */
    unsigned int a_trsize; /* length of relocation info for text, in bytes */
    unsigned int a_drsize; /* length of relocation info for data, in bytes */
};


#define N_MAGIC(exec) ((exec).a_info & 0xffff)
#define OMAGIC 0407
#define NMAGIC 0410
#define ZMAGIC 0413
#define QMAGIC 0314

#define X86_STACK_TOP 0x7d000000

/* max code+data+bss space allocated to elf interpreter */
#define INTERP_MAP_SIZE (32 * 1024 * 1024)

/* max code+data+bss+brk space allocated to ET_DYN executables */
#define ET_DYN_MAP_SIZE (128 * 1024 * 1024)

/* from personality.h */

/* Flags for bug emulation. These occupy the top three bytes. */
#define STICKY_TIMEOUTS     0x4000000
#define WHOLE_SECONDS       0x2000000

/* Personality types. These go in the low byte. Avoid using the top bit,
 * it will conflict with error returns.
 */
#define PER_MASK        (0x00ff)
#define PER_LINUX       (0x0000)
#define PER_SVR4        (0x0001 | STICKY_TIMEOUTS)
#define PER_SVR3        (0x0002 | STICKY_TIMEOUTS)
#define PER_SCOSVR3     (0x0003 | STICKY_TIMEOUTS | WHOLE_SECONDS)
#define PER_WYSEV386        (0x0004 | STICKY_TIMEOUTS)
#define PER_ISCR4       (0x0005 | STICKY_TIMEOUTS)
#define PER_BSD         (0x0006)
#define PER_XENIX       (0x0007 | STICKY_TIMEOUTS)

/* Necessary parameters */
#define ALPHA_PAGE_SIZE 4096
#define X86_PAGE_SIZE 4096

#define ALPHA_PAGE_MASK (~(ALPHA_PAGE_SIZE-1))
#define X86_PAGE_MASK (~(X86_PAGE_SIZE-1))

#define ALPHA_PAGE_ALIGN(addr) ((((addr)+ALPHA_PAGE_SIZE)-1)&ALPHA_PAGE_MASK)
#define X86_PAGE_ALIGN(addr) ((((addr)+X86_PAGE_SIZE)-1)&X86_PAGE_MASK)

#define NGROUPS 32

#define X86_ELF_EXEC_PAGESIZE X86_PAGE_SIZE
#define X86_ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(X86_ELF_EXEC_PAGESIZE-1))
#define X86_ELF_PAGEOFFSET(_v) ((_v) & (X86_ELF_EXEC_PAGESIZE-1))

#define ALPHA_ELF_PAGESTART(_v) ((_v) & ~(unsigned long)(ALPHA_PAGE_SIZE-1))
#define ALPHA_ELF_PAGEOFFSET(_v) ((_v) & (ALPHA_PAGE_SIZE-1))

#define INTERPRETER_NONE 0
#define INTERPRETER_AOUT 1
#define INTERPRETER_ELF 2

#define DLINFO_ITEMS 12

#define put_user(x,ptr) (void)(*(ptr) = (typeof(*ptr))(x))
#define get_user(ptr) (typeof(*ptr))(*(ptr))

static inline void memcpy_fromfs(void * to, const void * from, unsigned long n)
{
    memcpy(to, from, n);
}

static inline void memcpy_tofs(void * to, const void * from, unsigned long n)
{
    memcpy(to, from, n);
}

//extern void * mmap4k();
#define mmap4k(a, b, c, d, e, f) mmap((void *)(a), b, c, d, e, f)

extern unsigned long x86_stack_size;

static int load_aout_interp(void * exptr, int interp_fd);

#ifdef BSWAP_NEEDED
static void bswap_ehdr(Elf32_Ehdr *ehdr)
{
    bswap16s(&ehdr->e_type);            /* Object file type */
    bswap16s(&ehdr->e_machine);     /* Architecture */
    bswap32s(&ehdr->e_version);     /* Object file version */
    bswap32s(&ehdr->e_entry);       /* Entry point virtual address */
    bswap32s(&ehdr->e_phoff);       /* Program header table file offset */
    bswap32s(&ehdr->e_shoff);       /* Section header table file offset */
    bswap32s(&ehdr->e_flags);       /* Processor-specific flags */
    bswap16s(&ehdr->e_ehsize);      /* ELF header size in bytes */
    bswap16s(&ehdr->e_phentsize);       /* Program header table entry size */
    bswap16s(&ehdr->e_phnum);       /* Program header table entry count */
    bswap16s(&ehdr->e_shentsize);       /* Section header table entry size */
    bswap16s(&ehdr->e_shnum);       /* Section header table entry count */
    bswap16s(&ehdr->e_shstrndx);        /* Section header string table index */
}

static void bswap_phdr(Elf32_Phdr *phdr)
{
    bswap32s(&phdr->p_type);            /* Segment type */
    bswap32s(&phdr->p_offset);      /* Segment file offset */
    bswap32s(&phdr->p_vaddr);       /* Segment virtual address */
    bswap32s(&phdr->p_paddr);       /* Segment physical address */
    bswap32s(&phdr->p_filesz);      /* Segment size in file */
    bswap32s(&phdr->p_memsz);       /* Segment size in memory */
    bswap32s(&phdr->p_flags);       /* Segment flags */
    bswap32s(&phdr->p_align);       /* Segment alignment */
}
#endif

static void * get_free_page(void)
{
    void *  retval;

    /* User-space version of kernel get_free_page.  Returns a page-aligned
     * page-sized chunk of memory.
     */
    retval = mmap4k(0, ALPHA_PAGE_SIZE, PROT_READ|PROT_WRITE,
                    MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if((long)retval == -1)
    {
        perror("get_free_page");
        exit(-1);
    }
    else
    {
        return(retval);
    }
}

static void free_page(void * pageaddr)
{
    (void)munmap(pageaddr, ALPHA_PAGE_SIZE);
}

/*
 * 'copy_string()' copies argument/envelope strings from user
 * memory to free pages in kernel mem. These are in a format ready
 * to be put directly into the top of new user memory.
 *
 */
static unsigned long copy_strings(int argc,char ** argv,unsigned long *page,
                                  unsigned long p)
{
    char *tmp, *tmp1, *pag = NULL;
    int len, offset = 0;

    if (!p)
    {
        return 0;       /* bullet-proofing */
    }
    while (argc-- > 0)
    {
        if (!(tmp1 = tmp = get_user(argv+argc)))
        {
            fprintf(stderr, "VFS: argc is wrong");
            exit(-1);
        }
        while (get_user(tmp++));
        len = tmp - tmp1;
        if (p < len)    /* this shouldn't happen - 128kB */
        {
            return 0;
        }
        while (len)
        {
            --p;
            --tmp;
            --len;
            if (--offset < 0)
            {
                offset = p % X86_PAGE_SIZE;
                if (!(pag = (char *) page[p/X86_PAGE_SIZE]) &&
                    !(pag = (char *) page[p/X86_PAGE_SIZE] =
                                (unsigned long *) get_free_page()))
                {
                    return 0;
                }
            }
            if (len == 0 || offset == 0)
            {
                *(pag + offset) = get_user(tmp);
            }
            else
            {
                int bytes_to_copy = (len > offset) ? offset : len;
                tmp -= bytes_to_copy;
                p -= bytes_to_copy;
                offset -= bytes_to_copy;
                len -= bytes_to_copy;
                memcpy_fromfs(pag + offset, tmp, bytes_to_copy + 1);
            }
        }
    }
    return p;
}

static int in_group_p(gid_t g)
{
    /* return TRUE if we're in the specified group, FALSE otherwise */
    int     ngroup;
    int     i;
    gid_t   grouplist[NGROUPS];

    ngroup = getgroups(NGROUPS, grouplist);
    for(i = 0; i < ngroup; i++)
    {
        if(grouplist[i] == g)
        {
            return 1;
        }
    }
    return 0;
}

static int count(char ** vec)
{
    int     i;

    for(i = 0; *vec; i++)
    {
        vec++;
    }

    return(i);
}
/* 初始化二进制程序参数 */
static int prepare_binprm(struct linux_binprm *bprm)
{
    struct stat     st;
    int mode;
    int retval, id_change;

    if(fstat(bprm->fd, &st) < 0)
    {
        return(-errno);
    }

    mode = st.st_mode;
    if(!S_ISREG(mode))      /* Must be regular file */
    {
        return(-EACCES);
    }
    if(!(mode & 0111))      /* Must have at least one execute bit set */
    {
        return(-EACCES);
    }

    bprm->e_uid = geteuid();
    bprm->e_gid = getegid();
    id_change = 0;

    /* Set-uid? */
    if(mode & S_ISUID)
    {
        bprm->e_uid = st.st_uid;
        if(bprm->e_uid != geteuid())
        {
            id_change = 1;
        }
    }

    /* Set-gid? */
    /*
     * If setgid is set but no group execute bit then this
     * is a candidate for mandatory locking, not a setgid
     * executable.
     */
    if ((mode & (S_ISGID | S_IXGRP)) == (S_ISGID | S_IXGRP))
    {
        bprm->e_gid = st.st_gid;
        if (!in_group_p(bprm->e_gid))
        {
            id_change = 1;
        }
    }

    memset(bprm->buf, 0, sizeof(bprm->buf));
    retval = lseek(bprm->fd, 0L, SEEK_SET);
    if(retval >= 0)
    {
        retval = read(bprm->fd, bprm->buf, 128);
    }
    if(retval < 0)
    {
        perror("prepare_binprm");
        exit(-1);
        /* return(-errno); */
    }
    else
    {
        return(retval);
    }
}

unsigned long setup_arg_pages(unsigned long p, struct linux_binprm * bprm,
                              struct image_info * info)
{
    unsigned long stack_base, size, error;
    int i;

    /* Create enough stack to hold everything.  If we don't use
     * it for args, we'll use it for something else...
     * 创造一个足够的stack来保存everything
     */
    size = x86_stack_size;
    if (size < MAX_ARG_PAGES*X86_PAGE_SIZE)
        size = MAX_ARG_PAGES*X86_PAGE_SIZE;
    /* error记录的是栈的首地址 */
    error = (unsigned long)mmap4k(NULL,
                                  size + X86_PAGE_SIZE,
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS,
                                  -1, 0);
    if (error == -1)
    {
        perror("stk mmap");
        exit(-1);
    }
    /* we reserve one extra page at the top of the stack as guard */
    /* 在栈的顶部保留一个额外的page用作保护 */
    mprotect((void *)(error + size), X86_PAGE_SIZE, PROT_NONE);
    /* 栈基址 */
    stack_base = error + size - MAX_ARG_PAGES*X86_PAGE_SIZE; /* 栈前方要放置参数信息 */
    p += stack_base; /* 注意,这里的p始终指向程序参数的首地址 */

    if (bprm->loader)
    {
        bprm->loader += stack_base;
    }
    bprm->exec += stack_base;

    for (i = 0 ; i < MAX_ARG_PAGES ; i++)
    {
        if (bprm->page[i])
        {
            info->rss++;
            /* 拷贝参数信息 */
            memcpy((void *)stack_base, (void *)bprm->page[i], X86_PAGE_SIZE);
            free_page((void *)bprm->page[i]);
        }
        stack_base += X86_PAGE_SIZE;
    }
    return p;
}

static void set_brk(unsigned long start, unsigned long end)
{
    /* page-align the start and end addresses... */
    start = ALPHA_PAGE_ALIGN(start);
    end = ALPHA_PAGE_ALIGN(end);
    if (end <= start)
        return;
    if((long)mmap4k(start, end - start,
                    PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) == -1)
    {
        perror("cannot mmap brk");
        exit(-1);
    }
}


/* We need to explicitly zero any fractional pages
   after the data section (i.e. bss).  This would
   contain the junk from the file that should not
   be in memory */


static void padzero(unsigned long elf_bss)
{
    unsigned long nbyte;
    char * fpnt;

    nbyte = elf_bss & (ALPHA_PAGE_SIZE-1);  /* was X86_PAGE_SIZE - JRP */
    if (nbyte)
    {
        nbyte = ALPHA_PAGE_SIZE - nbyte;
        fpnt = (char *) elf_bss;
        do
        {
            *fpnt++ = 0;
        }
        while (--nbyte);
    }
}

static unsigned int * create_elf_tables(char *p, int argc, int envc,
                                        struct elfhdr * exec,
                                        unsigned long load_addr,
                                        unsigned long load_bias,
                                        unsigned long interp_load_addr, int ibcs,
                                        struct image_info *info)
{
    target_ulong *argv, *envp, *dlinfo;
    target_ulong *sp;

    /*
     * Force 16 byte alignment here for generality.
     */
    sp = (unsigned int *) (~15UL & (unsigned long) p);
    sp -= exec ? DLINFO_ITEMS*2 : 2;
    dlinfo = sp;
    sp -= envc+1;
    envp = sp;
    sp -= argc+1;
    argv = sp;
    if (!ibcs)
    {
        put_user(tswapl((target_ulong)envp),--sp);
        put_user(tswapl((target_ulong)argv),--sp);
    }

#define NEW_AUX_ENT(id, val) \
          put_user (tswapl(id), dlinfo++); \
          put_user (tswapl(val), dlinfo++)

    if (exec)   /* Put this here for an ELF program interpreter */
    {
        NEW_AUX_ENT (AT_PHDR, (target_ulong)(load_addr + exec->e_phoff));
        NEW_AUX_ENT (AT_PHENT, (target_ulong)(sizeof (struct elf_phdr)));
        NEW_AUX_ENT (AT_PHNUM, (target_ulong)(exec->e_phnum));
        NEW_AUX_ENT (AT_PAGESZ, (target_ulong)(ALPHA_PAGE_SIZE));
        NEW_AUX_ENT (AT_BASE, (target_ulong)(interp_load_addr));
        NEW_AUX_ENT (AT_FLAGS, (target_ulong)0);
        NEW_AUX_ENT (AT_ENTRY, load_bias + exec->e_entry);
        NEW_AUX_ENT (AT_UID, (target_ulong) getuid());
        NEW_AUX_ENT (AT_EUID, (target_ulong) geteuid());
        NEW_AUX_ENT (AT_GID, (target_ulong) getgid());
        NEW_AUX_ENT (AT_EGID, (target_ulong) getegid());
    }
    NEW_AUX_ENT (AT_NULL, 0);
#undef NEW_AUX_ENT
    put_user(tswapl(argc),--sp);
    info->arg_start = (unsigned int)((unsigned long)p & 0xffffffff);
    while (argc-->0)
    {
        put_user(tswapl((target_ulong)p),argv++);
        while (get_user(p++)) /* nothing */ ;
    }
    put_user(0,argv);
    info->arg_end = info->env_start = (unsigned int)((unsigned long)p & 0xffffffff);
    while (envc-->0)
    {
        put_user(tswapl((target_ulong)p),envp++);
        while (get_user(p++)) /* nothing */ ;
    }
    put_user(0,envp);
    info->env_end = (unsigned int)((unsigned long)p & 0xffffffff);
    return sp;
}


/* 加载动态库 */
static unsigned long load_elf_interp(struct elfhdr * interp_elf_ex,
                                     int interpreter_fd,
                                     unsigned long *interp_load_addr)
{
    struct elf_phdr *elf_phdata  =  NULL;
    struct elf_phdr *eppnt;
    unsigned long load_addr = 0;
    int load_addr_set = 0;
    int retval;
    unsigned long last_bss, elf_bss;
    unsigned long error;
    int i;

    elf_bss = 0;
    last_bss = 0;
    error = 0;

#ifdef BSWAP_NEEDED
    bswap_ehdr(interp_elf_ex);
#endif
    /* First of all, some simple consistency checks */
    if ((interp_elf_ex->e_type != ET_EXEC &&
         interp_elf_ex->e_type != ET_DYN) ||
        !elf_check_arch(interp_elf_ex->e_machine))
    {
        return ~0UL;
    }


    /* Now read in all of the header information */

    if (sizeof(struct elf_phdr) * interp_elf_ex->e_phnum > X86_PAGE_SIZE)
        return ~0UL;

    elf_phdata =  (struct elf_phdr *)
                  malloc(sizeof(struct elf_phdr) * interp_elf_ex->e_phnum);

    if (!elf_phdata)
        return ~0UL;

    /*
     * If the size of this structure has changed, then punt, since
     * we will be doing the wrong thing.
     */
    if (interp_elf_ex->e_phentsize != sizeof(struct elf_phdr))
    {
        free(elf_phdata);
        return ~0UL;
    }

    retval = lseek(interpreter_fd, interp_elf_ex->e_phoff, SEEK_SET);
    if(retval >= 0)
    {
        retval = read(interpreter_fd,
                      (char *) elf_phdata,
                      sizeof(struct elf_phdr) * interp_elf_ex->e_phnum);
    }
    if (retval < 0)
    {
        perror("load_elf_interp");
        exit(-1);
        free (elf_phdata);
        return retval;
    }
#ifdef BSWAP_NEEDED
    eppnt = elf_phdata;
    for (i=0; i<interp_elf_ex->e_phnum; i++, eppnt++)
    {
        bswap_phdr(eppnt);
    }
#endif

    if (interp_elf_ex->e_type == ET_DYN)
    {
        /* in order to avoid harcoding the interpreter load
           address in qemu, we allocate a big enough memory zone */
        error = (unsigned long)mmap4k(NULL, INTERP_MAP_SIZE,
                                      PROT_NONE, MAP_PRIVATE | MAP_ANON,
                                      -1, 0);
        if (error == -1)
        {
            perror("mmap");
            exit(-1);
        }
        load_addr = error;
        load_addr_set = 1;
    }

    eppnt = elf_phdata;
    for(i=0; i<interp_elf_ex->e_phnum; i++, eppnt++)
        if (eppnt->p_type == PT_LOAD)
        {
            int elf_type = MAP_PRIVATE | MAP_DENYWRITE;
            int elf_prot = 0;
            unsigned long vaddr = 0;
            unsigned long k;

            if (eppnt->p_flags & PF_R) elf_prot =  PROT_READ;
            if (eppnt->p_flags & PF_W) elf_prot |= PROT_WRITE;
            if (eppnt->p_flags & PF_X) elf_prot |= PROT_EXEC;
            if (interp_elf_ex->e_type == ET_EXEC || load_addr_set)
            {
                elf_type |= MAP_FIXED;
                vaddr = eppnt->p_vaddr;
            }
            /* 执行映射操作 */
            error = (unsigned long)mmap4k(load_addr+X86_ELF_PAGESTART(vaddr),
                                          eppnt->p_filesz + X86_ELF_PAGEOFFSET(eppnt->p_vaddr),
                                          elf_prot,
                                          elf_type,
                                          interpreter_fd,
                                          eppnt->p_offset - X86_ELF_PAGEOFFSET(eppnt->p_vaddr));

            if (error > -1024UL)
            {
                /* Real error */
                close(interpreter_fd);
                free(elf_phdata);
                return ~0UL;
            }

            if (!load_addr_set && interp_elf_ex->e_type == ET_DYN)
            {
                load_addr = error;
                load_addr_set = 1;
            }

            /*
             * Find the end of the file  mapping for this phdr, and keep
             * track of the largest address we see for this.
             * elf_bss记录最大的地址
             */
            k = load_addr + eppnt->p_vaddr + eppnt->p_filesz;
            if (k > elf_bss) elf_bss = k;

            /*
             * Do the same thing for the memory mapping - between
             * elf_bss and last_bss is the bss section.
             */
            k = load_addr + eppnt->p_memsz + eppnt->p_vaddr;
            if (k > last_bss) last_bss = k;
        }

    /* Now use mmap to map the library into memory. */

    close(interpreter_fd);

    /*
     * Now fill out the bss section.  First pad the last page up
     * to the page boundary, and then perform a mmap to make sure
     * that there are zeromapped pages up to and including the last
     * bss page.
     * 现在填充bss section.
     */
    padzero(elf_bss);
    elf_bss = X86_ELF_PAGESTART(elf_bss + ALPHA_PAGE_SIZE - 1); /* What we have mapped so far */

    /* Map the last of the bss segment */
    if (last_bss > elf_bss)
    {
        mmap4k(elf_bss, last_bss-elf_bss,
               PROT_READ|PROT_WRITE|PROT_EXEC,
               MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    }
    free(elf_phdata);

    *interp_load_addr = load_addr;
    return ((unsigned long) interp_elf_ex->e_entry) + load_addr;
}


/* 加载elf文件,这个函数实际实现了一个elf程序加载器
 * @param regs 寄存器信息
 */
static int load_elf_binary(struct linux_binprm * bprm, struct target_pt_regs * regs,
                           struct image_info * info)
{
    struct elfhdr elf_ex;
    struct elfhdr interp_elf_ex;
    struct exec interp_ex;
    int interpreter_fd = -1; /* avoid warning */
    unsigned long load_addr, load_bias;
    int load_addr_set = 0;
    unsigned int interpreter_type = INTERPRETER_NONE;
    unsigned char ibcs2_interpreter;
    int i;
    void * mapped_addr;
    struct elf_phdr * elf_ppnt;
    struct elf_phdr *elf_phdata;
    unsigned long elf_bss, k, elf_brk;
    int retval;
    char * elf_interpreter;
    unsigned long elf_entry, interp_load_addr = 0;
    int status;
    unsigned long start_code, end_code, end_data;
    unsigned long elf_stack;
    char passed_fileno[6];

    ibcs2_interpreter = 0;
    status = 0;
    load_addr = 0;
    load_bias = 0;
    elf_ex = *((struct elfhdr *) bprm->buf);          /* exec-header */
#ifdef BSWAP_NEEDED
    bswap_ehdr(&elf_ex);
#endif

    if (elf_ex.e_ident[0] != 0x7f ||
        strncmp(&elf_ex.e_ident[1], "ELF",3) != 0) /* 校验elf头部标识,保证是elf文件 */
    {
        return  -ENOEXEC;
    }

    /* First of all, some simple consistency checks */
    if ((elf_ex.e_type != ET_EXEC && elf_ex.e_type != ET_DYN) ||
        (! elf_check_arch(elf_ex.e_machine)))
    {
        return -ENOEXEC;
    }

    /* Now read in all of the header information */
    /* 现在读取所有的头部信息 */
    elf_phdata = (struct elf_phdr *)malloc(elf_ex.e_phentsize*elf_ex.e_phnum);
    if (elf_phdata == NULL)
    {
        return -ENOMEM;
    }
    /* 读取程序头表,加载elf程序,实际只需要头表信息即可 */
    retval = lseek(bprm->fd, elf_ex.e_phoff, SEEK_SET);
    if(retval > 0)
    {
        retval = read(bprm->fd, (char *) elf_phdata,
                      elf_ex.e_phentsize * elf_ex.e_phnum);
    }

    if (retval < 0)
    {
        perror("load_elf_binary");
        exit(-1);
        free (elf_phdata);
        return -errno;
    }

#ifdef BSWAP_NEEDED
    elf_ppnt = elf_phdata;
    for (i=0; i<elf_ex.e_phnum; i++, elf_ppnt++) /* 遍历程序头表 */
    {
        bswap_phdr(elf_ppnt);
    }
#endif
    elf_ppnt = elf_phdata;

    elf_bss = 0;
    elf_brk = 0;


    elf_stack = ~0UL;
    elf_interpreter = NULL;
    start_code = ~0UL;
    end_code = 0;
    end_data = 0;
    /* 这里可以默认elf_phdr为elf32_phdr
     * Phdr表示Program Header Table 也就是程序表头
     */
    for(i=0; i < elf_ex.e_phnum; i++)
    {
    	/* 解析器,用户写的代码并不能直接跑,还需要一个libc.so来引导,程序入口并不是main
		 * 而是libc中的_start
    	 */
        if (elf_ppnt->p_type == PT_INTERP) 
        {
            if ( elf_interpreter != NULL )
            {
                free (elf_phdata);
                free(elf_interpreter);
                close(bprm->fd);
                return -EINVAL;
            }

            /* This is the program interpreter used for
             * shared libraries - for now assume that this
             * is an a.out format binary
             */

            elf_interpreter = (char *)malloc(elf_ppnt->p_filesz);

            if (elf_interpreter == NULL)
            {
                free (elf_phdata);
                close(bprm->fd);
                return -ENOMEM;
            }
            /* 读取Segment(段)的信息 */
            retval = lseek(bprm->fd, elf_ppnt->p_offset, SEEK_SET);
            if(retval >= 0)
            {
                retval = read(bprm->fd, elf_interpreter, elf_ppnt->p_filesz);
            }
            if(retval < 0)
            {
                perror("load_elf_binary2");
                exit(-1);
            }

            /* If the program interpreter is one of these two,
               then assume an iBCS2 image. Otherwise assume
               a native linux image. */

            /* JRP - Need to add X86 lib dir stuff here... */
            /* 共享库的路径 */
            if (strcmp(elf_interpreter,"/usr/lib/libc.so.1") == 0 ||
                strcmp(elf_interpreter,"/usr/lib/ld.so.1") == 0)
            {
                ibcs2_interpreter = 1;
            }

#if 0
            printf("Using ELF interpreter %s\n", elf_interpreter);
#endif
            if (retval >= 0)
            {
                /* 加载共享库 */
                retval = open(path(elf_interpreter), O_RDONLY);
                if(retval >= 0)
                {
                    interpreter_fd = retval;
                }
                else
                {
                    perror(elf_interpreter);
                    exit(-1);
                    /* retval = -errno; */
                }
            }

            if (retval >= 0)
            {
                retval = lseek(interpreter_fd, 0, SEEK_SET);
                if(retval >= 0)
                {
                    retval = read(interpreter_fd,bprm->buf,128);
                }
            }
            if (retval >= 0)
            {
                interp_ex = *((struct exec *) bprm->buf); /* aout exec-header */
                interp_elf_ex=*((struct elfhdr *) bprm->buf); /* elf exec-header */
            }
            if (retval < 0)
            {
                perror("load_elf_binary3");
                exit(-1);
                free (elf_phdata);
                free(elf_interpreter);
                close(bprm->fd);
                return retval;
            }
        }
        elf_ppnt++; /* 指向下一个程序头 */
    }

    /* Some simple consistency checks for the interpreter */
    if (elf_interpreter)
    {
        interpreter_type = INTERPRETER_ELF | INTERPRETER_AOUT;

        /* Now figure out which format our binary is */
        if ((N_MAGIC(interp_ex) != OMAGIC) && (N_MAGIC(interp_ex) != ZMAGIC) &&
            (N_MAGIC(interp_ex) != QMAGIC))
        {
            interpreter_type = INTERPRETER_ELF;
        }

        if (interp_elf_ex.e_ident[0] != 0x7f ||
            strncmp(&interp_elf_ex.e_ident[1], "ELF",3) != 0)
        {
            interpreter_type &= ~INTERPRETER_ELF;
        }

        if (!interpreter_type)
        {
            free(elf_interpreter);
            free(elf_phdata);
            close(bprm->fd);
            return -ELIBBAD;
        }
    }

    /* OK, we are done with that, now set up the arg stuff,
       and then start this sucker up */
    /* 开始设置参数 */
    if (!bprm->sh_bang)
    {
        char * passed_p;

        if (interpreter_type == INTERPRETER_AOUT)
        {
            sprintf(passed_fileno, "%d", bprm->fd);
            passed_p = passed_fileno;

            if (elf_interpreter)
            {
                bprm->p = copy_strings(1,&passed_p,bprm->page,bprm->p);
                bprm->argc++;
            }
        }
        if (!bprm->p)
        {
            if (elf_interpreter)
            {
                free(elf_interpreter);
            }
            free (elf_phdata);
            close(bprm->fd);
            return -E2BIG;
        }
    }

    /* OK, This is the point of no return */
    info->end_data = 0;
    info->end_code = 0;
    info->start_mmap = (unsigned long)ELF_START_MMAP; /* 硬编码程序起始地址 */
    info->mmap = 0;
    /* 入口地址,规定ELF程序的入口虚拟地址,操作系统在加载完该程序后从这个地址开始执行进程的指令,可重定向
     * 文件一般没有入口地址,则这个值为0
     */
    elf_entry = (unsigned long) elf_ex.e_entry;

    /* Do this so that we can load the interpreter, if need be.  We will
       change some of these later */
    info->rss = 0;
    bprm->p = setup_arg_pages(bprm->p, bprm, info);
    info->start_stack = bprm->p; /* 记录下堆栈开始的位置 */

    /* Now we do a little grungy work by mmaping the ELF image into
     * the correct location in memory.  At this point, we assume that
     * the image should be loaded at fixed address, not at a variable
     * address.
     * 将ELF镜像映射到内存中正确的位置,在这个时候,我们假定镜像应当被加载到固定的地址,而不是一个可变的地址
     */

    for (i = 0, elf_ppnt = elf_phdata; i < elf_ex.e_phnum; i++, elf_ppnt++)
    {
        int elf_prot = 0; /* 属性 */
        int elf_flags = 0;
        unsigned long error;
        /* 只有LOAD类型的segment才是需要被映射的 */
        if (elf_ppnt->p_type != PT_LOAD) /* 只处理LOAD类型的segment */
            continue;

        if (elf_ppnt->p_flags & PF_R) elf_prot |= PROT_READ;
        if (elf_ppnt->p_flags & PF_W) elf_prot |= PROT_WRITE;
        if (elf_ppnt->p_flags & PF_X) elf_prot |= PROT_EXEC;
        elf_flags = MAP_PRIVATE | MAP_DENYWRITE;
        if (elf_ex.e_type == ET_EXEC || load_addr_set)
        {
            elf_flags |= MAP_FIXED; /* 映射到固定的位置 */
        }
        else if (elf_ex.e_type == ET_DYN) /* 动态库 */
        {
            /* Try and get dynamic programs out of the way of the default mmap
               base, as well as whatever program they might try to exec.  This
               is because the brk will follow the loader, and is not movable.  */
            /* NOTE: for qemu, we do a big mmap to get enough space
               without harcoding any address */
            error = (unsigned long)mmap4k(NULL, ET_DYN_MAP_SIZE,
                                          PROT_NONE, MAP_PRIVATE | MAP_ANON,
                                          -1, 0);
            if (error == -1)
            {
                perror("mmap");
                exit(-1);
            }
            /* 到这里,error记录的是实际映射的位置,这个和elf中期望映射的地址其实还是有偏差的
             * 因此就出现了load_bias
             */
            load_bias = X86_ELF_PAGESTART(error - elf_ppnt->p_vaddr);
        }
        /* 这里直接做了映射操作,为每一个segment都执行一个映射
         * elf_ppnt->p_vaddr记录的是虚拟地址
         */
        error = (unsigned long)mmap4k(
                    X86_ELF_PAGESTART(load_bias + elf_ppnt->p_vaddr),
                    (elf_ppnt->p_filesz +
                     X86_ELF_PAGEOFFSET(elf_ppnt->p_vaddr)),
                    elf_prot,
                    (MAP_FIXED | MAP_PRIVATE | MAP_DENYWRITE),
                    bprm->fd,
                    (elf_ppnt->p_offset -
                     X86_ELF_PAGEOFFSET(elf_ppnt->p_vaddr)));
        if (error == -1)
        {
            perror("mmap");
            exit(-1);
        }

#ifdef LOW_ELF_STACK
        if (X86_ELF_PAGESTART(elf_ppnt->p_vaddr) < elf_stack)
            elf_stack = X86_ELF_PAGESTART(elf_ppnt->p_vaddr);
#endif

        if (!load_addr_set)
        {
            load_addr_set = 1; /* 加载地址确定了 */
            /* load_addr记录实际的虚拟加载地址
             * 这里举一个简单的例子:
             * Program Headers:
             *  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
             *  LOAD           0x000000 0x08048000 0x08048000 0x000ac 0x000ac R E 0x1000
             * 在这个segment中, p_vaddr为0x08048000, p_offset为0
             * load_bias在大多数情况下,都是0
             */
            load_addr = elf_ppnt->p_vaddr - elf_ppnt->p_offset;
            if (elf_ex.e_type == ET_DYN) /* 动态库 */
            {
                load_bias += error -
                             X86_ELF_PAGESTART(load_bias + elf_ppnt->p_vaddr);
                load_addr += load_bias;
            }
        }
        k = elf_ppnt->p_vaddr; /* 期望的虚拟地址 */
        if (k < start_code)
            start_code = k;
        /* p_filesz表示segment在elf文件中所占用空间的长度 */
        k = elf_ppnt->p_vaddr + elf_ppnt->p_filesz;
        if (k > elf_bss)
            elf_bss = k; /* elf_bss指向bss段最尾部的地址 ? */
        if ((elf_ppnt->p_flags & PF_X) && end_code <  k) /* 可执行的段就是代码段 */
            end_code = k; /* 更新end_code的值 */
        if (end_data < k)
            end_data = k;
        /* p_memsz表示segment在进程虚拟地址空间中所占的长度 */
        k = elf_ppnt->p_vaddr + elf_ppnt->p_memsz;
        if (k > elf_brk) elf_brk = k;
    }
    /* elf_entry是ELF程序的入口虚拟地址,在x86下,这个值一般为0x804874 */
    elf_entry += load_bias;
    elf_bss += load_bias;
    elf_brk += load_bias;
    start_code += load_bias;
    end_code += load_bias;
    //    start_data += load_bias;
    end_data += load_bias;

    if (elf_interpreter) /* 动态库?? */
    {
        if (interpreter_type & 1)
        {
            elf_entry = load_aout_interp(&interp_ex, interpreter_fd);
        }
        else if (interpreter_type & 2)
        {
            elf_entry = load_elf_interp(&interp_elf_ex, interpreter_fd,
                                        &interp_load_addr);
        }

        close(interpreter_fd);
        free(elf_interpreter);

        if (elf_entry == ~0UL)
        {
            printf("Unable to load interpreter\n");
            free(elf_phdata);
            exit(-1);
            return 0;
        }
    }

    free(elf_phdata);

    if (interpreter_type != INTERPRETER_AOUT) close(bprm->fd);
    info->personality = (ibcs2_interpreter ? PER_SVR4 : PER_LINUX);

#ifdef LOW_ELF_STACK
    info->start_stack = bprm->p = elf_stack - 4;
#endif
    bprm->p = (unsigned long)
              create_elf_tables((char *)bprm->p,
                                bprm->argc,
                                bprm->envc,
                                (interpreter_type == INTERPRETER_ELF ? &elf_ex : NULL),
                                load_addr, load_bias,
                                interp_load_addr,
                                (interpreter_type == INTERPRETER_AOUT ? 0 : 1),
                                info);
    if (interpreter_type == INTERPRETER_AOUT)
        info->arg_start += strlen(passed_fileno) + 1;
    info->start_brk = info->brk = elf_brk;
    info->end_code = end_code;
    info->start_code = start_code;
    info->end_data = end_data;
    info->start_stack = bprm->p;

    /* Calling set_brk effectively mmaps the pages that we need for the bss and break
       sections */
    set_brk(elf_bss, elf_brk);

    padzero(elf_bss);

#if 0
    printf("(start_brk) %x\n", info->start_brk);
    printf("(end_code) %x\n", info->end_code);
    printf("(start_code) %x\n", info->start_code);
    printf("(end_data) %x\n", info->end_data);
    printf("(start_stack) %x\n", info->start_stack);
    printf("(brk) %x\n", info->brk);
#endif

    if ( info->personality == PER_SVR4 )
    {
        /* Why this, you ask???  Well SVr4 maps page 0 as read-only,
           and some applications "depend" upon this behavior.
           Since we do not have the power to recompile these, we
           emulate the SVr4 behavior.  Sigh.  */
        mapped_addr = mmap4k(NULL, ALPHA_PAGE_SIZE, PROT_READ | PROT_EXEC,
                             MAP_FIXED | MAP_PRIVATE, -1, 0);
    }

#ifdef ELF_PLAT_INIT
    /*
     * The ABI may specify that certain registers be set up in special
     * ways (on i386 %edx is the address of a DT_FINI function, for
     * example.  This macro performs whatever initialization to
     * the regs structure is required.
     */
    ELF_PLAT_INIT(regs);
#endif


    info->entry = elf_entry;

    return 0;
}


/* 加载并解析elf文件 */
int elf_exec(const char * filename, char ** argv, char ** envp,
             struct target_pt_regs * regs, struct image_info *infop)
{
    struct linux_binprm bprm;
    int retval;
    int i;
    /* p指向参数内存块的顶部 */
    bprm.p = X86_PAGE_SIZE*MAX_ARG_PAGES-sizeof(unsigned int);
    for (i=0 ; i<MAX_ARG_PAGES ; i++)       /* clear page-table */
        bprm.page[i] = 0;
    retval = open(filename, O_RDONLY);
    if (retval == -1)
    {
        perror(filename);
        exit(-1);
        /* return retval; */
    }
    else
    {
        bprm.fd = retval;
    }
    bprm.filename = (char *)filename;
    bprm.sh_bang = 0;
    bprm.loader = 0;
    bprm.exec = 0;
    bprm.dont_iput = 0;
    bprm.argc = count(argv);
    bprm.envc = count(envp);

    retval = prepare_binprm(&bprm);

    if(retval>=0)
    {
        bprm.p = copy_strings(1, &bprm.filename, bprm.page, bprm.p);
        bprm.exec = bprm.p;
        bprm.p = copy_strings(bprm.envc,envp,bprm.page,bprm.p);
        bprm.p = copy_strings(bprm.argc,argv,bprm.page,bprm.p);
        if (!bprm.p)
        {
            retval = -E2BIG;
        }
    }

    if(retval>=0)
    {
        retval = load_elf_binary(&bprm,regs,infop);
    }
    if(retval>=0)
    {
        /* success.  Initialize important registers */
        regs->esp = infop->start_stack;
        regs->eip = infop->entry;
        return retval;
    }

    /* Something went wrong, return the inode and free the argument pages*/
    for (i=0 ; i<MAX_ARG_PAGES ; i++)
    {
        free_page((void *)bprm.page[i]);
    }
    return(retval);
}


static int load_aout_interp(void * exptr, int interp_fd)
{
    printf("a.out interpreter not yet supported\n");
    return(0);
}

