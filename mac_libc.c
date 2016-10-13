// 
// for license see LICENSE file
//

#ifdef __MACH__

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
#include <crt_externs.h>
#include <dirent.h>

// 
// linux stuff for mac
// stuff taken from https://github.com/evanphx/ulysses-libc

char *bindtextdomain(const char *domainname, const char *dirname) {
	return NULL;
}

char *textdomain(const char *domainname) {
	return NULL;
}

char *dcgettext(const char *domainname, const char *msgid, int category)
{
	return (char *) msgid;
}

char *gettext(const char *msgid)
{
	return (char *) msgid;
}

static const int32_t table[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
64,
'a','b','c','d','e','f','g','h','i','j','k','l','m',
'n','o','p','q','r','s','t','u','v','w','x','y','z',
91,92,93,94,95,96,
'a','b','c','d','e','f','g','h','i','j','k','l','m',
'n','o','p','q','r','s','t','u','v','w','x','y','z',
123,124,125,126,127,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const int32_t *const ptable = table+128;

const int32_t **__ctype_tolower_loc(void)
{
	return (void *)&ptable;
}

int fputs_unlocked(const char *str, FILE *stream) {
	printf("str = %s   file=%p\n", str, stream);
	return fputs(str, stream);
}

size_t __fpending (FILE *fp) {
  return fp->_p - fp->_bf._base;
}

int __freading(FILE *f)
{
	return 0;
}

void* __rawmemchr(const void* s, int c) {
	  return memchr(s, c, -1);
}

int rl_filename_quoting_desired = 1;

void *rl_last_func = NULL;

int rl_sort_completion_matches = 1;
int history_max_entries=10;

void *rl_directory_completion_hook = NULL;
void *rl_filename_rewrite_hook = NULL;

int __xstat64(int version, const char *file, void *buf) {
   return stat(file, buf);
}

FILE *fopen64(const char *path, const char *mode) {
	return fopen(path, mode);
}

ssize_t __getdelim(char ** linep, size_t *linecap, int delimeter, FILE *stream) {
	return getdelim(linep, linecap, delimeter, stream);
}

#define X(x) x

static const unsigned short table2[] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x200),X(0x320),X(0x220),X(0x220),X(0x220),X(0x220),X(0x200),X(0x200),
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),X(0x200),
X(0x160),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),X(0x8d8),
X(0x8d8),X(0x8d8),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8d5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),X(0x8c5),
X(0x8c5),X(0x8c5),X(0x8c5),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),
X(0x4c0),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8d6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),X(0x8c6),
X(0x8c6),X(0x8c6),X(0x8c6),X(0x4c0),X(0x4c0),X(0x4c0),X(0x4c0),X(0x200),
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static const unsigned short *const ptable2 = table2+128;

const unsigned short **__ctype_b_loc(void)
{
	return (void *)&ptable2;
}

char *__strdup(char *str) { return strdup(str); }
char *__strndup(char *str, int n) { return strndup(str, n); }

void *mempcpy(void *dest, const void *src, size_t n)
{
	return (char *)memcpy(dest, src, n) + n;
}

size_t fwrite_unlocked(char *src, size_t size, size_t nmemb, FILE * f) {
	return fwrite((char*)src, size, nmemb, f);
}

int
__fprintf_chk (FILE *fp, int flag, const char *format, ...)
{
  va_list ap;
  int done;

  va_start (ap, format);
  done = vfprintf (fp, format, ap);
  va_end (ap);

  return done;
}

int __vfprintf_chk(FILE *fp, char *format, va_list ap) { return vfprintf(fp, format, ap); }

size_t __ctype_get_mb_cur_max()
{
	return 4;
}

struct dirent *readdir64(DIR *dirp) { return readdir(dirp); }
//__asm__("readdir64:\njmp readdir\n"); // TODO: test this

FILE *setmntent(const char *name, const char *mode)
{
	return fopen(name, mode);
}

int endmntent(FILE *f)
{
	fclose(f);
	return 1;
}
#endif
