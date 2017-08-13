// 
// for license see LICENSE file
//

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
#include <sys/stat.h>
#include <stdarg.h>
#include <ctype.h>
#include <libgen.h>
//#include <time.h>

#ifdef __linux__

#include <elf.h>
#define libnm(x) (const char*)(x)

#elif __MACH__

#include <crt_externs.h>

void replace_symbol(char *nm, uint32_t orig, uint32_t addr) {
			int i;
			Dl_info info;
                        dladdr((void*)orig, &info);
			uint32_t *got = (uint32_t*)((uint32_t)info.dli_fbase & 0xFFFF0000); //info.dli_fbase;
			fprintf(stderr, "library %s @ %p (%X), looking for 0x%08X, replace with 0x%08X\n", nm, got, info.dli_fbase, orig, addr);
			uint32_t count = 100000;
			for (i = 0; i < count; i++) {
				if (got[i] == (uint32_t)orig) {
					fprintf(stderr, "found at 0x%X (%d)!!!\n", info.dli_fbase + i*4, i);
					mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE);
					got[i] = addr;
					//mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ);
					break;
				}
			}
}

char *libnm(char *nm) {
	char *result = malloc(strlen(nm)+6);
	strcpy(result, nm);
	int i;
	for (i = 0; i < strlen(nm); i++) if (nm[i] == '.') break;
	strcpy(result + i, ".dylib\0");
	return result;
}
#endif

void pr_error(const char* msg) {
  perror(msg);
  abort();
}

char *__progname, *__progname_full;

typedef struct {
	char *name;
	uint32_t pointer;
} function_t;

function_t functions[8000];

#define DEBUG
#include "undef.h"

int function_count = 0;
void* add_function(char *name, uint32_t pointer) {
	functions[function_count].name = strdup(name);
	functions[function_count].pointer = pointer;
	function_count++;
	return pointers[function_count-1];
}

int g_argc;
char** g_argv;
int H__libc_start_main(int (*m)(int, char**, char**),
                       int argc, char** argv) {
  if (g_argc) {
    argc = g_argc;
    argv = g_argv;
  }
  int result = m(argc, argv, 0);
  printf("finished with %d\n", result);
  exit(result);
}

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

#define R_386_NONE      0
#define R_386_32        1
#define R_386_PC32      2
#define R_386_GOT32     3
#define R_386_PLT32     4
#define R_386_COPY      5
#define R_386_GLOB_DAT  6
#define R_386_JMP_SLOT  7
#define R_386_RELATIVE  8
#define R_386_GOTOFF    9
#define R_386_GOTPC     10

char *nm(int id) {
	switch (id) {
		case R_386_NONE: return "386_NONE";
		case R_386_32: return "386_32";
		case R_386_COPY: return "386_COPY";
		case R_386_GLOB_DAT: return "386_GLOB_DAT";
		case R_386_JMP_SLOT: return "386_JMP_SLOT";
		case R_386_RELATIVE: return "386_RELATIVE";
		default: return "unknown";
	}
}

#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7

#define ELF_MAGIC 0x464C457F
#define  LIBRARY_ADDRESS_BY_HANDLE(dlhandle) ((NULL == dlhandle) ? NULL :  (void*)*(size_t const*)(dlhandle)) 

// taken form ELF-Hook 

#ifdef __linux__

#define Elf_Ehdr Elf32_Ehdr
#define Elf_Rel Elf32_Rel

static int read_header(int d, Elf_Ehdr **header)
{
    *header = (Elf_Ehdr *)malloc(sizeof(Elf_Ehdr));
    if(NULL == *header)
    {
        return errno;
    }
    if (lseek(d, 0, SEEK_SET) < 0)
    {
        free(*header);
        return errno;
    }
    if (read(d, *header, sizeof(Elf_Ehdr)) <= 0)
    {
        free(*header);
        return errno = EINVAL;
    }
    return 0;
}

static int read_section_table(int d, Elf_Ehdr const *header, Elf32_Shdr **table)
{
    size_t size;
    if (NULL == header)
        return EINVAL;
    size = header->e_shnum * sizeof(Elf32_Shdr);
    *table = (Elf32_Shdr *)malloc(size);
    if(NULL == *table)
    {
        return errno;
    }
    if (lseek(d, header->e_shoff, SEEK_SET) < 0)
    {
        free(*table);
        return errno;
    }
    if (read(d, *table, size) <= 0)
    {
        free(*table);
        return errno = EINVAL;
    }
    return 0;
}

static int read_string_table(int d, Elf32_Shdr const *section, char const **strings)
{
    if (NULL == section)
        return EINVAL;
    *strings = (char const *)malloc(section->sh_size);
    if(NULL == *strings)
    {
        return errno;
    }
    if (lseek(d, section->sh_offset, SEEK_SET) < 0)
    {
        free((void *)*strings);
        return errno;
    }
    if (read(d, (char *)*strings, section->sh_size) <= 0)
    {
        free((void *)*strings);
        return errno = EINVAL;
    }
    return 0;
}

static int section_by_name(int d, char const *section_name, Elf32_Shdr **section)
{
    Elf_Ehdr *header = NULL;
    Elf32_Shdr *sections = NULL;
    char const *strings = NULL;
    size_t i;

    *section = NULL;

    if (
        read_header(d, &header) ||
        read_section_table(d, header, &sections) ||
        read_string_table(d, &sections[header->e_shstrndx], &strings)
        )
        return errno;

    for (i = 0; i < header->e_shnum; ++i)
        if (!strcmp(section_name, &strings[sections[i].sh_name]))
        {
            *section = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr));

            if (NULL == *section)
            {
                free(header);
                free(sections);
                free((void *)strings);
                return errno;
            }

            memcpy(*section, sections + i, sizeof(Elf32_Shdr));
            break;
        }

    free(header);
    free(sections);
    free((void *)strings);
    return 0;
}

uint32_t get_got_addr(char *nm) {
			void *libhandle =  dlopen(nm, RTLD_NOW | RTLD_GLOBAL);
			void *lib = LIBRARY_ADDRESS_BY_HANDLE(libhandle);
                        int descriptor = open(nm, O_RDONLY);
                        Elf32_Shdr *got;
                        section_by_name(descriptor, ".got", &got);
			close(descriptor);
                        void *got_table = (void *)(((size_t)lib) + got->sh_addr);
			return (uint32_t)got_table;
}

uint32_t get_got_count(char *nm) {
	int descriptor = open(nm, O_RDONLY);
	Elf32_Shdr *got;
	section_by_name(descriptor, ".got", &got);
	close(descriptor);
	return got->sh_size;
}

void replace_symbol(char *fname, uint32_t orig, uint32_t addr) {
	uint32_t *got = (uint32_t*)get_got_addr(fname);
	uint32_t count = get_got_count(fname);
	int i;
	for (i = 0; i < count; i++)  if (got[i] == (uint32_t)orig) {
		fprintf(stderr, "replacing the symbol in %s\n", fname);
		mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE);
		got[i] = (uint32_t)addr;
		mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ);
		break;
	}
}
#endif

// from net

#define HEXDUMP_COLS 8

void hexdump(void *mem, unsigned int len)
{
        unsigned int i, j;
        
        for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
        {
                /* print offset */
                if(i % HEXDUMP_COLS == 0)
                {
                        printf("0x%08x: ", (uint32_t)mem + i);
                }
 
                /* print hex data */
                if(i < len)
                {
                        printf("%02x ", 0xFF & ((char*)mem)[i]);
                }
                else /* end of block, just aligning for ASCII dump */
                {
                        printf("   ");
                }
                
                /* print ASCII dump */
                if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1))
                {
                        for(j = i - (HEXDUMP_COLS - 1); j <= i; j++)
                        {
                                if(j >= len) /* end of block, not really printing */
                                {
                                        putchar(' ');
                                }
                                else if(isprint(((char*)mem)[j])) /* printable char */
                                {
                                        putchar(0xFF & ((char*)mem)[j]);        
                                }
                                else /* other char */
                                {
                                        putchar('.');
                                }
                        }
                        putchar('\n');
                }
        }
}


/// end of taken

char* library_list[255];
int library_count = 0;
int add_library(char *nm) {
	int i;
	char *nm_b = strdup(basename(nm)); // TODO: missing free
	for (i = 0; i < library_count; i++) if (!strcmp(library_list[i], nm)) return 0;
	if (!strncmp(nm_b, "libsystem",9)) return 0;
	if (!strncmp(nm_b, "libc.", 5)) return 0; 
        Dl_info info;
        dladdr(add_library, &info);
        if (!strcmp(nm_b, basename((char*)info.dli_fname))) return 0;
	library_list[library_count] = strdup(nm);
	library_count++;
	return 1;
}

int main(int argc, char* argv[]) {
  int i;
  int fd, len;
  char* elf;
  int entry, phoff, phnum, init;
  int* ph;
  if (argc < 2)
    pr_error("Usage: el <elf>");
  printf("loading %s\n", argv[1]);
  fd = open(argv[1], O_RDONLY);
  if (fd < 0)
    pr_error("Usage: el <elf>");
  len = lseek(fd, 0, SEEK_END);
  elf = malloc(len);
  lseek(fd, 0, SEEK_SET);
  read(fd, elf, len);
  if (*(int*)elf != ELF_MAGIC) {
    close(fd);
    pr_error("not elf");
  }
  if ((*(int*)(elf+16) != 0x30002) && (*(int*)(elf+16) != 0x30003)) {
   close(fd);
   pr_error("not i386 exec");
  }

  __progname = strdup(argv[0]);
  __progname_full = strdup(argv[1]);

  entry = *(int*)(elf+24);
  phoff = *(int*)(elf+28);
  phnum = *(int*)(elf+42);

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
      fprintf(stderr, "PT_LOAD size=%d fsize=%d flag=%d addr=%x prot=%d poff=%d\n",
             psize, pafsize, pflag, paddr, prot, poff);
      if (mmap((void*)paddr, pafsize, prot, MAP_FILE|MAP_PRIVATE|MAP_FIXED,
               fd, poff) == MAP_FAILED) {
        pr_error("mmap(file)");
      }
      if ((prot & PROT_WRITE)) {
        for (; pfsize < pafsize; pfsize++) {
          char* p = (char*)paddr;
          p[pfsize] = 0;
        }
        if (pfsize != psize) {
          if (mmap((void*)(paddr + pfsize),
                   psize - pfsize, prot, MAP_ANON|MAP_PRIVATE,
                   -1, 0) == MAP_FAILED) {
            pr_error("mmap(anon)");
          }
        }
      }
      break;
    }
    case PT_INTERP: {
		  fprintf(stderr, "omitting PT_INTERP\n");
		  break;
		    }

    case PT_PHDR: {
		  fprintf(stderr, "omitting PT_PHDR\n");
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
        case DT_NEEDED: {
          *neededp++ = dval;
	  break;
        }
        case DT_PLTRELSZ: {
          pltrelsz = dval;
          fprintf(stderr, "pltrelsz: %d\n", pltrelsz);
          break;
        }
	case DT_STRTAB: {
          dstr = (char*)dval;
          fprintf(stderr, "dstr: %p %s\n", dstr, dstr+1);
          break;
        }
        case DT_SYMTAB: {
          dsym = (char*)dval;
          fprintf(stderr, "dsym: %p\n", dsym);
          break;
        }
        case DT_REL: {
          rel = (char*)dval;
          fprintf(stderr, "rel: %p\n", rel);
          break;
        }
        case DT_RELSZ: {
          relsz = dval;
          fprintf(stderr, "relsz: %d\n", relsz);
          break;
        }
        case DT_RELENT: {
          relent = dval;
          fprintf(stderr, "relent: %d\n", relent);
          break;
        }
        case DT_PLTREL: {
          pltrel = (char*)dval;
          fprintf(stderr, "pltrel: %p\n", pltrel);
          break;
        }
        default:
          fprintf(stderr, "unknown DYN dtag=%d dval=%X\n", dtag, dval);
        }
      }

      if (!dsym || !dstr) {
        pr_error("no dsym or dstr");
      }

      for (neededp = needed; *neededp; neededp++) {
        fprintf(stderr, "needed: %s -> %s", dstr + *neededp, libnm(dstr+*neededp));
	void *libpointer = dlopen(libnm(dstr + *neededp), RTLD_NOW | RTLD_GLOBAL);
        if (!libpointer) {
		fprintf(stderr, " (not found)\n");
	} else {
                Dl_info info;
                dladdr(dlsym(libpointer, "start"), &info);
		fprintf(stderr, " (loaded at %X)\n", info.dli_fbase);
	}
      }

      {
        int i, j;
	char* oldrel = rel;
        for (j = 0; j < 2; j++) for (i = 0; i < relsz; rel += relent, i += relent) {
            int* addr = *(int**)rel;
            int info = *(int*)(rel + 4);
            int sym = info >> 8;
            int type = info & 0xff;
            int* ds = (int*)(dsym + 16 * sym);
	    uint32_t *sz = (uint32_t*)(dsym + 16 * sym + 8);
	    //uint32_t *sym_addr = (uint32_t*)(dsym + 16 * sym + 4);
            char* sname = dstr + *ds;

            void* val=0;
	    val = dlsym(RTLD_DEFAULT, sname);
	    if (val) {
		Dl_info info;
                dladdr(val, &info);
		fprintf(stderr, "Successfully resolved %s as %p @ %s of size %d\n", sname, val, info.dli_fname, *sz);
	        add_library((char*)info.dli_fname);
	    }
	}
	rel = oldrel;
        for (j = 0; j < 2; j++) {
          for (i = 0; i < relsz; rel += relent, i += relent) {
            int* addr = *(int**)rel;
            int info = *(int*)(rel + 4);
            int sym = info >> 8;
            int type = info & 0xff;

            int* ds = (int*)(dsym + 16 * sym);
            char* sname = dstr + *ds;
	    uint32_t *sz = (uint32_t*)(dsym + 16 * sym + 8);
	    //uint32_t *sym_addr = (uint32_t*)(dsym + 16 * sym + 4);
            void* val=0;
            int k;
            for (k=0; T[k].n; k++) {
              if (!strcmp(sname,T[k].n)) {
                 val = T[k].f;
                 break;
              }
            }

            if (!val) {
		val = dlsym(RTLD_DEFAULT, sname);
            }

	    #ifdef __MACH__
            if (!val) {
                        if (!strcmp(sname, "stdin")) val = &stdin;
                        if (!strcmp(sname, "stdout")) val = &stdout;
                        if (!strcmp(sname, "stderr")) val = &stderr;
			if (!strcmp(sname, "__environ")) val = (void*)_NSGetEnviron();
                }
	#endif 
            fprintf(stderr, "%srel: %p %s(%d) (type=%d %s) => %p\n",
                   j ? "plt" : "", (void*)addr, sname, sym, type, nm(type), val);

            switch (type) {
            case R_386_32: {
	      if (val) {
                *addr = (int)val;
	      } else {
	        fprintf(stderr, "undefined relocation %s\n", sname);
		*addr = 0;
		abort();
	      }
	      break;
            }
            case R_386_COPY: { // 5
	    /*R_386_COPY	read a string of bytes from the "symbol" address and deposit a copy into this location; the "symbol" object has an intrinsic length 
	      i.e. move initialized data from a library down into the app data space */
              if (val) {
		      memcpy((void*)addr, (void*)val, *sz);
		      if ((val != &stdout) && (val != &stdin) && (val != &stderr) /*&& (*sz > 0)*/) {
			      // very primitive got replacement
			      Dl_info info;
		              dladdr(val, &info);
         		if (info.dli_saddr == val) {
	         		fprintf(stderr, "found symbol %s of size %d @ %p in loaded lib %s (%p)\n", info.dli_sname, *sz, val, info.dli_fname, info.dli_fbase);
				int iter;
				for (iter = 0; iter < library_count; iter++) { 
					fprintf(stderr, "trying to replace symbol in %s\n", library_list[iter]);
					replace_symbol(library_list[iter], (uint32_t)val, (uint32_t)addr);
				}

			}
		}
              } else {
                fprintf(stderr, "undefined symbol %s\n", sname);
		//abort();
              }
	      break;
            }
            case R_386_GLOB_DAT: {
              if (val) {
                *addr = (int)val;
              } else {
                fprintf(stderr, "undefined data %s\n", sname);
		//*addr = 0;
              }
              break;
            }
            case R_386_JMP_SLOT: {
              if (val) {
                *addr = (int)add_function(sname, (uint32_t)val);
              } else {
	        fprintf(stderr, "undefined function %s\n", sname);
                *addr = (int)add_function(sname, 0);
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
    case PT_NOTE:
		 printf("omitting PT_NOTE\n");
		 break;
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
  printf("our pid is %d\n", getpid());
  ((void*(*)(int, char**))entry)(argc, argv);
  // we should never return here
  return 1;
}
