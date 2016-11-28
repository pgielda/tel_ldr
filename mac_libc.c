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
int klogctl (int type, char *buf, int len) {
	printf("klogctl not implemented!\n");
	return 0;
}

int _IO_getc(FILE* __fp) {
	return getc(__fp);
}

int _IO_putc(char c, FILE *__fp) {
	return putc(c, __fp);
}

int __printf_chk(int flag, const char * format, ...) {
	va_list ap;
        int result;
        va_start (ap, format);
        result = vfprintf (stdout, format, ap);
        va_end (ap);
        return result;
}

int open64(const char *pathname, int flags) {
	return open(pathname, flags);
}

int openat64(int dirfd, const char *pathname, int flags) {
	return openat(dirfd, pathname, flags);
}

int __fxstat64(int vers, int fd, struct stat *buf) {
	if (vers != 3) printf("error!\n");
	return fstat(fd, buf);
}

int __open64 (const char *file, int oflag, ...) {
        int mode = 0;

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

	return open(file, oflag, mode);
}

int __open64_2 (const char *file, int oflag)
{
  return __open64 (file, oflag);
}

char *bindtextdomain(const char *domainname, const char *dirname)
{
	static const char dir[] = "/";
	if (!domainname || !*domainname
		|| (dirname && ((dirname[0] != '/') || dirname[1]))
		) {
		errno = EINVAL;
		return NULL;
	}
	return (char *) dir;
}

char *textdomain(const char *domainname)
{
	static const char default_str[] = "messages";
	if (domainname && *domainname && strcmp(domainname, default_str)) {
		errno = EINVAL;
		return NULL;
	}
	return (char *) default_str;
}

char *dcgettext(const char *domainname, const char *msgid, int category)
{
	return (char *) msgid;
}

char * dcngettext(const char * domainname, const char * msgid1, const char * msgid2, unsigned long int n, int category)
{
        return (n == 1) ? (char*)msgid1 : (char*)msgid2;
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

int __xstat64(int version, const char *path, struct stat *stat_buf) {
   	if (version != 3) printf("error!\n");
	return stat(path, stat_buf);
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
	printf("fwrite_unlocked: '%s' (%p), size=%zu nmemb=%zu, f=%p\n", src, src, size, nmemb, f);
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

int __vfprintf_chk(FILE *fp, int flag, char *format, va_list ap) { 
  return vfprintf(fp, format, ap); 
}

size_t __ctype_get_mb_cur_max()
{
	return 4;
}

struct dirent *readdir64(DIR *dirp) { return readdir(dirp); }

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
