#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

void error(const char* msg) {
  perror(msg);
  abort();
}

__asm("undefined:         \n\
     jmp _undefined       \n\
   ");

void undefined();


void _undefined() {
  int32_t *d = (int32_t*) (__builtin_return_address(0)-4);
  uint32_t add = (uint32_t) __builtin_return_address(0) + d[0];
  uint32_t *dd = (uint32_t*) (add+2);
  fprintf(stderr, "undefined function is called (call 0x%08X 0x%08X)\n", add, dd[0]);
}


int g_argc;
char** g_argv;
int H__libc_start_main(int (*m)(int, char**, char**),
                       int argc, char** argv /*,
                       void (*init)(void), void (*fini)(void),
                       void (*rtld_fini)(void),
                       void (*stack_end)
                       */
                       ) {
  if (g_argc) {
    argc = g_argc;
    argv = g_argv;
  }
  /*printf("%d %s\n", argc, argv[0]);*/
  exit(m(argc, argv, 0));
}

#ifdef __MACH__
void* __rawmemchr(const void* s, int c) {
  return memchr(s, c, -1);
}
#endif

int Hlseek(int fd, int off, int wh) {
  return lseek(fd, off, wh);
}

void* Hdlsym(void* h, char* p) {
  if(!h)
    h=RTLD_DEFAULT;
  return dlsym(h, p);
}

void* Hmmap(void* a, size_t l, int p, int f, int fd, int o) {
  return mmap(a, l, p, (f&0x1f)|(f&32?0x1000:0), fd, o);
}

int* H__errno_location() {
  return &errno;
}

struct {
  const char* n;
  void* f;
} T[] = {
#define H(n) { #n, (void*)&H ## n },
  H(__libc_start_main)
  H(__errno_location)
  H(mmap)
  H(dlsym)
  H(lseek)
  {0,0},
};

#define DT_NULL         0
#define DT_NEEDED       1
#define DT_PLTRELSZ     2
#define DT_PLTGOT       3
#define DT_HASH         4
#define DT_STRTAB       5
#define DT_SYMTAB       6
#define DT_RELA         7
#define DT_RELASZ       8
#define DT_RELAENT      9
#define DT_STRSZ        10
#define DT_SYMENT       11
#define DT_INIT         12
#define DT_FINI         13
#define DT_SONAME       14
#define DT_RPATH        15
#define DT_SYMBOLIC     16
#define DT_REL          17
#define DT_RELSZ        18
#define DT_RELENT       19
#define DT_PLTREL       20
#define DT_DEBUG        21
#define DT_TEXTREL      22
#define DT_JMPREL       23
#define DT_ENCODING     32

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7

int main(int argc, char* argv[]) {
  int i;
  int fd, len;
  char* elf;
  int entry, phoff, phnum, init;
  int* ph;
  if (argc < 2)
    error("Usage: el <elf>");
  printf("loading %s\n", argv[1]);
  fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    error("Usage: el <elf>");
  len = lseek(fd, 0, SEEK_END);
  elf = malloc(len);
  lseek(fd, 0, SEEK_SET);
  read(fd, elf, len);

  if (*(int*)elf != 0x464c457f)
    error("not elf");
  if (*(int*)(elf+16) != 0x30002)
    error("not i386 exec");

  entry = *(int*)(elf+24);
  phoff = *(int*)(elf+28);
  phnum = *(int*)(elf+42);
  printf("%x %x %x\n", entry, phoff, phnum);

  ph = (int*)(elf + phoff);
  for (i = 0; i < phnum >> 16; i++) {
    int poff, paddr, pfsize, psize, pafsize, pflag /*, palign */;
    poff = ph[1];
    paddr = ph[2];
    pfsize = ph[4];
    psize = ph[5];
    pflag = ph[6];
    /*palign = ph[7];*/
    switch (ph[0]) {
    case PT_LOAD: {
      int prot = 0;
      if (pflag & 1)
        prot |= PROT_EXEC;
      if (pflag & 2)
        prot |= PROT_WRITE;
      if (pflag & 4)
        prot |= PROT_READ;
      psize += paddr & 0xfff;
      pfsize += paddr & 0xfff;
      poff -= paddr & 0xfff;
      paddr &= ~0xfff;
      pafsize = (pfsize + 0xfff) & ~0xfff;
      psize = (psize + 0xfff) & ~0xfff;
      printf("PT_LOAD size=%d fsize=%d flag=%d addr=%x prot=%d poff=%d\n",
             psize, pafsize, pflag, paddr, prot, poff);
      if (mmap((void*)paddr, pafsize, prot, MAP_FILE|MAP_PRIVATE|MAP_FIXED,
               fd, poff) == MAP_FAILED) {
        error("mmap(file)");
      }
      if ((prot & PROT_WRITE)) {
        printf("%p\n", (char*)paddr);
        for (; pfsize < pafsize; pfsize++) {
          char* p = (char*)paddr;
          p[pfsize] = 0;
        }
        if (pfsize != psize) {
          if (mmap((void*)(paddr + pfsize),
                   psize - pfsize, prot, MAP_ANON|MAP_PRIVATE,
                   -1, 0) == MAP_FAILED) {
            error("mmap(anon)");
          }
        }
      }
      break;
    }
    case PT_DYNAMIC: {
      char* dyn;
      char* dstr = NULL;
      char* dsym = NULL;
      char* rel = NULL;
      char* pltrel = NULL;
      int relsz, relent, pltrelsz = 0;
      int needed[999] = {}, *neededp = needed;
      puts("PT_DYNAMIC");
      dyn = elf + poff;
      for (;;) {
        unsigned short dtag = *(unsigned short*)dyn;
        int dval = *(int*)(dyn + 4);
        dyn += 8;
        if (dtag == 0)
          break;
        switch (dtag) {
	case DT_INIT: {
	   init = dval;
	   break;
	}
        case DT_NEEDED: {  /* DT_NEEDED */
          *neededp++ = dval;
	  break;
        }
        case DT_PLTRELSZ: {
          pltrelsz = dval;
          printf("pltrelsz: %d\n", pltrelsz);
          break;
        }
	case DT_STRTAB: {
          dstr = (char*)dval;
          printf("dstr: %p %s\n", dstr, dstr+1);
          break;
        }
        case DT_SYMTAB: {
          dsym = (char*)dval;
          printf("dsym: %p\n", dsym);
          break;
        }
        case DT_REL: {
          rel = (char*)dval;
          printf("rel: %p\n", rel);
          break;
        }
        case DT_RELSZ: {
          relsz = dval;
          printf("relsz: %d\n", relsz);
          break;
        }
        case DT_RELENT: {
          relent = dval;
          printf("relent: %d\n", relent);
          break;
        }
        case DT_PLTREL: {
          pltrel = (char*)dval;
          printf("pltrel: %p\n", pltrel);
          break;
        }
        default:
          printf("unknown DYN dtag=%d dval=%X\n", dtag, dval);
        }
      }
      if (!dsym || !dstr) {
        error("no dsym or dstr");
      }

      for (neededp = needed; *neededp; neededp++) {
        printf("needed: %s", dstr + *neededp);
	/* TODO: temporarily we are not loading the libs */
        if (dlopen(dstr + *neededp, RTLD_NOW | RTLD_GLOBAL) == NULL) {
		printf(" (not found)\n");
	} else {
		printf(" (loaded)\n");
	}
	printf(" (omitted)\n");
      }

      {
        int i, j;
        for (j = 0; j < 2; j++) {
          for (i = 0; i < relsz; rel += relent, i += relent) {
            int* addr = *(int**)rel;
            int info = *(int*)(rel + 4);
            int sym = info >> 8;
            int type = info & 0xff;

            int* ds = (int*)(dsym + 16 * sym);
            char* sname = dstr + *ds;
            void* val=0;
            int k;
            for(k=0;T[k].n;k++){
              if(!strcmp(sname,T[k].n)){
                 val = T[k].f;
                 break;
              }
            }

            if(!val){
                val = dlsym(RTLD_DEFAULT, sname);
            }

            printf("%srel: %p %s(%d) %d => %p\n",
                   j ? "plt" : "", (void*)addr, sname, sym, type, val);

            switch (type) {
            case 1: {
	      if (val)
                *addr += (int)val;
	      break;
            }
            case 5: {
              if (val) {
                *addr = *(int*)val;
              } else {
                fprintf(stderr, "undefined function %s\n", sname);
		*addr = (int)&undefined;
              }
	      break;
            }
            case 6: {
              if (val) {
                *addr = (int)val;
              } else {
                fprintf(stderr, "undefined data %s\n", sname);
              }
              break;
            }
            case 7: {
              if (val) {
                *addr = (int)val;
              } else {
	        fprintf(stderr, "undefined function %s\n", sname);
                *addr = (int)&undefined;
              }
              break;
            }
            }
          }

          if ((int)pltrel != 17) {
            rel = pltrel;
          }
          relsz = pltrelsz;
        }
      }

      break;
    }
    default:
      printf("unknown PT 0x%X\n", ph[0]);
    }
    ph += 8;
  }

  g_argc = argc-1;
  g_argv = argv+1;
  printf("init...\n");
  ((void*(*)())init)();
  printf("start!: %s %x\n", argv[1], entry);
  ((void*(*)(int, char**))entry)(argc, argv);
  return 1;
}
