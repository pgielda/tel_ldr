// 
// for license see LICENSE file
//

#define _GNU_SOURCE

#if __linux__
#include <link.h>
#endif

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
#include <time.h>

#define COLORS 1
//#define NO_LOGGING

static void inner_log(char* source, const char* text, va_list argList, char *type, int color)
{
#ifndef NO_LOGGING
    char color_s[10];
    if (COLORS) {
            sprintf(color_s, "\x1b[1;%dm", color);
    }
    time_t timer;
    char Buff[9];
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(Buff, 9, "%H:%M:%S", tm_info);
    fprintf(stderr, "[%s%s%s @ %s] %s%s%s: ", COLORS ? color_s : "", type, COLORS ? "\x1b[0m" : "", Buff, COLORS ? "\x1b[1;37m" : "", source, COLORS ? "\x1b[0m" : "");
    vfprintf(stderr, text, argList);
    fprintf(stderr, "\n\r");
#endif
}

#define LOG_WARNING "WRN", 95
#define LOG_INFO "INF", 34
#define LOG_TRACE "TRC", 39
#define LOG_DEBUG "DBG", 39
#define LOG_ERROR "ERR", 91
#define LOG_IGNORE "IGN", 39


void log_msg(char *type, int color, char* source, const char* text, ...) 
{
    va_list argList;
    va_start(argList, text);
    inner_log(source, text, argList, type, color);
    va_end(argList);
	
}
	
void vlog_msg(char *type, int color, char* source, const char* text, va_list argList)
{
    inner_log(source, text, argList, type, color);
}	


#ifdef __linux__

#include <elf.h>
#define libnm(x) (const char*)(x)

#elif __MACH__

#include <crt_externs.h>

void replace_symbol(char *nm, uint32_t orig, uint32_t addr, char *sname) {
			int i;
			Dl_info info;
                        dladdr((void*)orig, &info);
			uint32_t *got = (uint32_t*)info.dli_fbase;
			log_msg(LOG_INFO, "ELF_LOADER", "library %s @ %p, looking for 0x%08X, replace with 0x%08X", nm, got, orig, addr);
			uint32_t count = 300000;
			for (i = 0; i < count; i++) {
				if (got[i] == (uint32_t)orig) {
					mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE);
					got[i] = addr;
					log_msg(LOG_INFO, "ELF_LOADER", "Symbol found at position %d!!!", i);
					mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ);
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

extern char* __cxa_demangle(const char* mangled_name, char* output_buffer, size_t* length, int* status);


// TODO: this eats memory !
char *fancy_name(char *s) {
	int status;
	char *res = __cxa_demangle(s, NULL, NULL, &status);
	if (res == NULL) return s;
	return res;
}

#define DEBUG
#include "undef.h"

int function_count = 0;
void* add_function(char *name, uint32_t pointer) {
	functions[function_count].name = strdup(name);
	functions[function_count].pointer = pointer;
	function_count++;
	if (function_count > 8000) {
		printf("problem!\n");
		exit(1);
	}
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
  log_msg(LOG_INFO, "APP", "==============================================");
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

void get_got(int descriptor, Elf32_Shdr **got, char *pltnm) {
    section_by_name(descriptor, pltnm, got);
}

uint32_t get_got_addr(char *nm, char *pltnm) {
			void *libhandle =  dlopen(nm, RTLD_NOW | RTLD_LOCAL);
			void *lib = LIBRARY_ADDRESS_BY_HANDLE(libhandle);
                        int descriptor = open(nm, O_RDONLY);
                        Elf32_Shdr *got;
			get_got(descriptor, &got, pltnm);
			if (got == NULL) {
			    return 0;
			}
			close(descriptor);
                        void *got_table = (void *)(((uint32_t)lib) + got->sh_addr);
			return (uint32_t)got_table;
}

uint32_t get_got_count(char *nm, char *pltnm) {
	int descriptor = open(nm, O_RDONLY);
	Elf32_Shdr *got;
	get_got(descriptor, &got, pltnm);
	close(descriptor);
	return (got->sh_size/got->sh_entsize);
}

static inline Elf32_Shdr *elf_sheader(Elf32_Ehdr *hdr) {
	return (Elf32_Shdr *)((int)hdr + hdr->e_shoff);
}

static inline Elf32_Shdr *elf_section(Elf32_Ehdr *hdr, int idx) {
	return &elf_sheader(hdr)[idx];
}

int replace_symbol(char *fname, uint32_t orig, uint32_t addr, char *sname) {
        void *libhandle =  dlopen(fname, RTLD_NOW | RTLD_LOCAL);
	void *lib = LIBRARY_ADDRESS_BY_HANDLE(libhandle);
        void *val = dlsym(libhandle, sname);
	if (!val) return 0;
	log_msg(LOG_INFO, "ELF_LOADER", "Symbol '%s' exists in library '%s' under ptr '%p' (0x%08X)", sname, basename(fname), val, val - (uint32_t)lib);
	if ((uint32_t)val != orig) {
		log_msg(LOG_ERROR, "ELF_LOADER", "There is a difference between pointers for symbol '%s'", sname);
	}
        int replaced = 0;

	uint32_t *got;
	uint32_t count;
	char section[255];
	strcpy(section, ".got");

        Elf32_Shdr *symtab;
	int descriptor = open(fname, O_RDONLY);
	Elf_Ehdr *header = NULL;
	Elf32_Shdr *sections = NULL;
	char const *strings = NULL;

        read_header(descriptor, &header);
        read_section_table(descriptor, header, &sections);
	get_got(descriptor, &symtab, ".dynsym");
        read_string_table(descriptor, &sections[symtab->sh_link], &strings);


	uint32_t symtab_entries = symtab->sh_size / symtab->sh_entsize;
	got = (uint32_t*)get_got_addr(fname, section);

	uint32_t symaddr = (int)lib + symtab->sh_offset;
	for (int i = 0; i < symtab_entries; i++) {

	Elf32_Sym *symbol = &((Elf32_Sym *)symaddr)[i];

        char *nm = (char *)((uint32_t)strings + (uint32_t)symbol->st_name);
	if (strcmp(nm, sname) == 0) {
	        log_msg(LOG_INFO, "ELF_LOADER", "symbol %d %s %X size %d ind=%d other=%d", i, nm, symbol->st_value, symbol->st_size, symbol->st_shndx, symbol->st_other);
	}
	}

	got = (uint32_t*)get_got_addr(fname, section);
	count = get_got_count(fname, section);

	if (got != NULL) {
		log_msg(LOG_INFO, "ELF_LOADER", "trying to replace symbol '%s' addr 0x%08X with 0x%08X in library '%s' in section '%s' (%d entries)", sname, orig, addr, basename(fname), section, count);
			int i;
		for (i = 0; i < count; i++)  if (got[i] == (uint32_t)orig) {
			log_msg(LOG_INFO, "ELF_LOADER", "replacing the symbol '%s' in %s in .got (at position %d addr 0x%08X)", sname, basename(fname), i, (uint32_t)got + i *4);
			mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE);
			got[i] = (uint32_t)addr;
			mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ);
			replaced++;
		} 

	}
	if (replaced > 1) {
		log_msg(LOG_ERROR, "ELF_LOADER", "symbol '%s' got replaced more than once!", sname);
	}
/*
	got = (uint32_t*)get_got_addr(fname, ".plt.got");
        if (got != NULL) {
		uint32_t count = get_got_count(fname, ".plt.got");
		log_msg(LOG_ERROR, "ELF_LOADER", "Trying to replace 0x%08X with 0x%08X", orig, addr);
		int i;
		for (i = 0; i < count; i++)  if (got[i] == (uint32_t)orig) {
			log_msg(LOG_INFO, "ELF_LOADER", "replacing the symbol '%s' in %s in .got.plt", sname, fname);
			mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ | PROT_WRITE);
			got[i] = (uint32_t)addr;
			mprotect((void*)((uint32_t)(&(got[i])) & 0xFFFFF000), 0x1000, PROT_READ);
			replaced++;
		}
	}
*/	
	return replaced;
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
void* library_list_pointers[255];
int library_count = 0;
int add_library(char *nm, void *ptr) {
	int i;
	char nm_b[255];
	strcpy(nm_b, basename(nm));
	for (i = 0; i < library_count; i++) if (!strcmp(library_list[i], nm)) return 0;
	if (!strncmp(nm_b, "libsystem",9)) return 0;
	if (!strncmp(nm_b, "ld-linux", 8)) return 0;
        Dl_info info;
        dladdr(add_library, &info);
        if (!strcmp(nm_b, basename((char*)info.dli_fname))) return 0;
	library_list[library_count] = strdup(nm);
	library_list_pointers[library_count] = ptr;
	library_count++;
	return 1;
}

int gg = 0;

void test() {
	exit(1);
}

int do_debug = 0;

int load_elf(char *fname, int*rinit) {
  int i;
  int fd, len;
  int cnt = 0;
  char* elf;
  int entry, phoff, phnum, init;
  int* ph;

  log_msg(LOG_WARNING, "load_elf", "Loading %s", fname);
  fd = open(fname, O_RDONLY);
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
      log_msg(LOG_INFO, "ELF_LOADER", "PT_LOAD size=%d fsize=%d flag=%d addr=%x prot=%d poff=%d",
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
		  log_msg(LOG_WARNING, "ELF_LOADER", "omitting PT_INTERP");
		  break;
		    }

    case PT_PHDR: {
		  log_msg(LOG_WARNING, "ELF_LOADER", "omitting PT_PHDR");
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
          //printf("pltrelsz: %d\n", pltrelsz);
          break;
        }
	case DT_STRTAB: {
          dstr = (char*)dval;
          //printf("dstr: %p %s\n", dstr, dstr+1);
          break;
        }
        case DT_SYMTAB: {
          dsym = (char*)dval;
          //printf("dsym: %p\n", dsym);
          break;
        }
        case DT_REL: {
          rel = (char*)dval;
          //printf("rel: %p\n", rel);
          break;
        }
        case DT_RELSZ: {
          relsz = dval;
          //printf("relsz: %d\n", relsz);
          break;
        }
        case DT_RELENT: {
          relent = dval;
          //printf("relent: %d\n", relent);
          break;
        }
        case DT_PLTREL: {
          pltrel = (char*)dval;
          //printf("pltrel: %p\n", pltrel);
          break;
        }
        default:
          log_msg(LOG_WARNING, "ELF_LOADER", "unknown DYN dtag=%d dval=%X", dtag, dval);
        }
      }

      if (!dsym || !dstr) {
        pr_error("no dsym or dstr");
      }

      for (neededp = needed; *neededp; neededp++) {
        log_msg(LOG_INFO, "ELF_LOADER", "shared library needed: %s", dstr + *neededp);
	void *libpointer = dlopen(libnm(dstr + *neededp), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
        if (!libpointer) {
		log_msg(LOG_WARNING, "ELF_LOADER", "Library %s not found", dstr + *neededp);
	} else {
		log_msg(LOG_INFO, "ELF_LOADER", "Library %s loaded", dstr+*neededp);
		#if __linux__
		struct link_map *p;
		dlinfo(libpointer, RTLD_DI_LINKMAP, (struct link_map*)&p);
		add_library(p->l_name, libpointer);
		#endif
	}
      }

      {

        int i, j;
	char* oldrel = rel;
	#if !__linux__
        for (j = 0; j < 2; j++) for (i = 0; i < relsz; rel += relent, i += relent) {
            int* addr = *(int**)rel;
            int info = *(int*)(rel + 4);
            int sym = info >> 8;
            int type = info & 0xff;
            int* ds = (int*)(dsym + 16 * sym);
	    uint32_t *sz = (uint32_t*)(dsym + 16 * sym + 8);
	    uint32_t *sym_addr = (uint32_t*)(dsym + 16 * sym + 4);
            char* sname = dstr + *ds;
            void* val=0;
	    val = dlsym(RTLD_DEFAULT, sname);
	    if (val) {
		Dl_info info;
                dladdr(val, &info);
		add_library((char*)info.dli_fname);
		#if 0
                if (*sym_addr) {
                        log_msg(LOG_ERROR, "ELF_LOADER", "symbol %s '%s' : there is a conflict (%s@%p vs %p)", nm(type), sname, info.dli_fname,val, *sym_addr);
                                int iter;
                                for (iter = 0; iter < library_count; iter++) {
                                        lonng_msg(LOG_INFO, "ELF_LOADER", "Iterating over library %s (%d)", library_list[iter], iter);
                //                        replace_symbol(library_list[iter], (uint32_t)val, (uint32_t)*sym_addr);
                                }


//			replace_symbol(info.dli_fname, (uint32_t)val, (uint32_t)*sym_addr);
                }
		log_msg(LOG_INFO, "ELF_LOADER", "Successfully resolved %s as %p @ %s of size %d", sname, val, info.dli_fname, *sz);
		#endif
	    }
	}
	rel = oldrel;
	#endif
        for (j = 0; j < 2; j++) {
          for (i = 0; i < relsz; rel += relent, i += relent) {
            int* addr = *(int**)rel;
            int info = *(int*)(rel + 4);
            int sym = info >> 8;
            int type = info & 0xff;

            int* ds = (int*)(dsym + 16 * sym);
            char* sname = dstr + *ds;
	    uint32_t *sz = (uint32_t*)(dsym + 16 * sym + 8);
	    uint32_t *sym_addr = (uint32_t*)(dsym + 16 * sym + 4);
            void* val=0;
            int k;

            for (k=0; T[k].n; k++) {
              if (!strcmp(sname,T[k].n)) {
                 log_msg(LOG_INFO, "ELF_LOADER", "Overriding %s to builtin wrapper", sname);
                 val = T[k].f;
                 break;
              }
            }

            if (rinit == NULL) {
	     val = 0;
	    } else {
            if (!val) {
	//	val = dlsym(RTLD_DEFAULT, sname);
            }
	    if (!val) {
	    	int iter;
		for (iter = 0; iter < library_count; iter++) {
			val = dlsym(library_list_pointers[iter], sname);
			if (val) break;
		}
	    }
	    }

	    #ifdef __MACH__
            if (!strcmp(sname, "stdin")) val = &stdin;
            if (!strcmp(sname, "stdout")) val = &stdout;
            if (!strcmp(sname, "stderr")) val = &stderr;
	    if (!strcmp(sname, "__environ")) val = (void*)_NSGetEnviron();
 	    #endif 
	    //if (!strcmp(sname, "__environ")) val = &__environ;

            log_msg(LOG_INFO, "ELF_LOADER", "%srel: %p %s(%d) (type=%d %s) => %p",
                   j ? "plt" : "", (void*)addr, sname, sym, type, nm(type), val);

            switch (type) {
            case R_386_32: {
	      if (val) {
                *addr = (int)val;
	      } else {
	        log_msg(LOG_WARNING, "ELF_LOADER", "undefined relocation %s", sname);
		*addr = 0;
//		abort();
	      }
	      break;
            }
            case R_386_COPY: { // 5
	    /*R_386_COPY	read a string of bytes from the "symbol" address and deposit a copy into this location; the "symbol" object has an intrinsic length 
	      i.e. move initialized data from a library down into the app data space */
              if (val) {
	              if (*sz > 0) {
   		         memcpy((void*)addr, (void*)val, *sz);
			 } else log_msg(LOG_ERROR, "ELF_LOADER", "symbol %s has size 0", sname);

		      if ((val != &stdout) && (val != &stdin) && (val != &stderr) && (val != &__environ)) {
			      // very primitive got replacement
			      Dl_info info;
		              dladdr(val, &info);

         		if (info.dli_saddr == val) {
	         		log_msg(LOG_INFO, "ELF_LOADER", "found symbol %s of size %d @ %p in loaded lib %s (%p)", info.dli_sname, *sz, val, info.dli_fname, info.dli_fbase);
				int iter;
				log_msg(LOG_INFO, "ELF_LOADER", "trying to replace %s symbol %s in shared libs from %p to %p", nm(type), sname, val, addr);
				int replaced = 0;
				for (iter = 0; iter < library_count; iter++) {
				        log_msg(LOG_INFO, "ELF_LOADER", "Iterating over library %s (%d)", library_list[iter], iter);
					replaced += replace_symbol(library_list[iter], (uint32_t)val, (uint32_t)addr, sname);
				}
				log_msg(LOG_INFO, "ELF_LOADER", "done replacing %s symbol %s in shared libs from %p to %p, %d replaces occured", nm(type), sname, val, addr, replaced);
				if (replaced == 0) {
					log_msg(LOG_ERROR, "ELF_LOADER", "There is a problem with symbol '%s'", sname);
				}


			}
		}

              } else {
                log_msg(LOG_ERROR, "ELF_LOADER", "undefined symbol %s", sname);
		abort();
              }
	      break;
            }
            case R_386_GLOB_DAT: {
		// R_386_COPY and R_386_GLOB_DATA can be considered complements of each other. Suppose you have a global data object defined in a dynamic library. The library will have the binary version of the object in its .data space. When the application is built, the linker puts a R_386_COPY reloc in there to copy the data down to the application's .bss space. In turn, the library never references the original global object; it references the copy that is in the application data space, through a corresponding R_386_GLOB_DATA. Wierd, huh? After loading and copying, the original data (from the library) is never used; only the copy (in the app data space). 
	      if (*sym_addr) {
		      log_msg(LOG_INFO, "ELF_LOADER", "global data %s tied to local@%p", sname, *sym_addr);
		      *addr = *sym_addr;
		      if (val) log_msg(LOG_WARNING, "ELF_LOADER", "global data %s has also an external reference %p. It should have a matching R_386_COPY reference.", sname, val);
	      } else if (val) {
		      log_msg(LOG_INFO, "ELF_LOADER", "global data %s tied to external@%p", sname, val);
	              *addr = (int)val;
              } else {
                      log_msg(LOG_ERROR, "ELF_LOADER", "undefined global data %s of size %d (cnt=%d)", sname, *sz, cnt);
		      if (strcmp(sname, "__gmon_start__")) {
		//	      abort();
		      }
#if 0
		if (strcmp(sname, "__gmon_start__")) {
			*addr = *sym_addr;
			//*addr = malloc(*sz); // TODO
			log_msg(LOG_WARNING, "ELF_LOADER", "pointing undefined global data %s of size %d to 0x%08X", sname, *sz, *addr);
			cnt++;
		}
#endif
              }
              break;
            }
            case R_386_JMP_SLOT: {
              if (!val) log_msg(LOG_ERROR, "ELF_LOADER", "undefined function %s", sname);
	      *addr = do_debug ? (int)add_function(sname, (uint32_t)val) : (int)val;
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
		 log_msg(LOG_INFO, "ELF_LOADER", "omitting PT_NOTE");
		 break;
    default:
      log_msg(LOG_WARNING, "ELF_LOADER", "unknown PT 0x%X", ph[0]);
    }
    ph += 8;
  }

  log_msg(LOG_INFO, "APP", "function count is %d", function_count);
  
  if (rinit != NULL) *rinit = init;
  return entry;
}

int main(int argc, char* argv[]) {
  int init, entry;
  if (argc < 2)
    pr_error("Usage: el <elf>");
  log_msg(LOG_INFO, "APP", "loading %s", argv[1]);

  __progname = strdup(argv[1]);
  __progname_full = strdup(argv[1]);

  //load_elf("./gentoo32/lib/libc-2.23.so", NULL);
  entry = load_elf(argv[1], &init); 


  g_argc = argc-1;
  g_argv = argv+1;

  log_msg(LOG_INFO, "APP", "our pid is %d", getpid());
  if (init != 0) {
	  log_msg(LOG_INFO, "APP", "init");
	  ((void*(*)())init)();
  }
  log_msg(LOG_INFO, "APP", "start!: %s entry=%x", argv[1], entry);
  log_msg(LOG_INFO, "APP", "==============================================");

  ((void*(*)(int, char**))entry)(argc, argv);
  // we should never return here
  return 1;
}
