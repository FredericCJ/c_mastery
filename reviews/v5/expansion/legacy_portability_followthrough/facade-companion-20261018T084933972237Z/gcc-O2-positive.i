typedef long int ptrdiff_t;
typedef long unsigned int size_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;
typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;
typedef long int __quad_t;
typedef unsigned long int __u_quad_t;
typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;
typedef int __daddr_t;
typedef int __key_t;
typedef int __clockid_t;
typedef void * __timer_t;
typedef long int __blksize_t;
typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;
typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;
typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;
typedef long int __fsword_t;
typedef long int __ssize_t;
typedef long int __syscall_slong_t;
typedef unsigned long int __syscall_ulong_t;
typedef __off64_t __loff_t;
typedef char *__caddr_t;
typedef long int __intptr_t;
typedef unsigned int __socklen_t;
typedef int __sig_atomic_t;
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;
typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;
typedef signed char int_fast8_t;
typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned char uint_fast8_t;
typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int intptr_t;
typedef unsigned long int uintptr_t;
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
#pragma GCC diagnostic pop
enum B2Status { B2_OK, B2_INVALID, B2_RANGE, B2_FULL, B2_ABSENT, B2_STALE, B2_FAILED };
int b2_parse(const unsigned char *bytes, size_t length, int32_t *out);
typedef struct { unsigned state; uint16_t key; int value; } B2Entry;
typedef struct { B2Entry slots[7]; size_t count; } B2Map;
void b2_map_init(B2Map *map);
int b2_map_put(B2Map *map, uint16_t key, int value);
int b2_map_get(const B2Map *map, uint16_t key, int *out);
int b2_map_remove(B2Map *map, uint16_t key);
typedef struct { int key; unsigned serial; } B2Item;
int b2_sort(B2Item *items, size_t length, B2Item *scratch, size_t capacity);
typedef struct { size_t cursor, depth; unsigned next_id; size_t saved[4]; unsigned ids[4]; } B2Arena;
typedef struct { const B2Arena *owner; size_t depth; unsigned id; } B2Mark;
void b2_arena_init(B2Arena *arena);
int b2_reserve(B2Arena *arena, size_t length, size_t alignment, size_t *offset);
int b2_mark(B2Arena *arena, B2Mark *out);
int b2_rollback(B2Arena *arena, B2Mark mark);
int b2_q7(const int8_t *a, const int8_t *b, size_t length, int8_t *out);
typedef struct { int (*acquire)(void *, unsigned); void (*release)(void *, unsigned); void *context; } B2Backend;
int b2_transaction(B2Backend backend, unsigned *published);
typedef struct { unsigned tag; union { int number; struct { const unsigned char *data; size_t length; } text; int error; } value; } B2Result;
int b2_render(const B2Result *result, unsigned char *out, size_t capacity, size_t *written);
int b2_identity_int(int value);
unsigned b2_identity_unsigned(unsigned value);
typedef struct {int value;} BfRecord;
typedef int (*BfGet)(void *);
BfRecord *bf_mutable(BfRecord *);
const BfRecord *bf_const(const BfRecord *);
int bf_binary_int(int,int);
unsigned bf_binary_unsigned(unsigned,unsigned);
int bf_sequence(BfGet,void *,BfGet,void *,int *);

typedef __builtin_va_list __gnuc_va_list;
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
struct _IO_FILE;
typedef struct _IO_FILE __FILE;
struct _IO_FILE;
typedef struct _IO_FILE FILE;
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;
typedef void _IO_lock_t;
struct _IO_FILE
{
  int _flags;
  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;
  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;
  struct _IO_marker *_markers;
  struct _IO_FILE *_chain;
  int _fileno;
  int _flags2:24;
  char _short_backupbuf[1];
  __off_t _old_offset;
  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];
  _IO_lock_t *_lock;
  __off64_t _offset;
  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  struct _IO_FILE **_prevchain;
  int _mode;
  int _unused3;
  __uint64_t _total_written;
  char _unused2[12 * sizeof (int) - 5 * sizeof (void *)];
};
typedef __fpos_t fpos_t;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
extern int remove (const char *__filename) __attribute__ ((__nothrow__ , __leaf__));
extern int rename (const char *__old, const char *__new) __attribute__ ((__nothrow__ , __leaf__));
extern int fclose (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));
extern char *tmpnam (char[20]) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__));
extern int fflush (FILE *__stream);
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) __attribute__ ((__warn_unused_result__));
extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__ , __leaf__))
  __attribute__ ((__nonnull__ (1)));
extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...) __attribute__ ((__nonnull__ (1)));
extern int printf (const char *__restrict __format, ...);
extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) __attribute__ ((__nothrow__));
extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nonnull__ (1)));
extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);
extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));
extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));
extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));
extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __attribute__ ((__warn_unused_result__));
extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__));
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf") __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf") __attribute__ ((__warn_unused_result__));
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) __asm__ ("" "__isoc99_sscanf") __attribute__ ((__nothrow__ , __leaf__));
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));
extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")
     __attribute__ ((__format__ (__scanf__, 2, 0))) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")
     __attribute__ ((__format__ (__scanf__, 1, 0))) __attribute__ ((__warn_unused_result__));
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vsscanf") __attribute__ ((__nothrow__ , __leaf__))
     __attribute__ ((__format__ (__scanf__, 2, 0)));
extern int fgetc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getc (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int getchar (void);
extern int fputc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern int putchar (int __c);
extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (3)));
extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __attribute__ ((__nonnull__ (2)));
extern int puts (const char *__s);
extern int ungetc (int __c, FILE *__stream) __attribute__ ((__nonnull__ (2)));
extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) __attribute__ ((__warn_unused_result__))
  __attribute__ ((__nonnull__ (4)));
extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s) __attribute__ ((__nonnull__ (4)));
extern int fseek (FILE *__stream, long int __off, int __whence)
  __attribute__ ((__nonnull__ (1)));
extern long int ftell (FILE *__stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern void rewind (FILE *__stream) __attribute__ ((__nonnull__ (1)));
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos)
  __attribute__ ((__nonnull__ (1)));
extern int fsetpos (FILE *__stream, const fpos_t *__pos) __attribute__ ((__nonnull__ (1)));
extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int feof (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern int ferror (FILE *__stream) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (1)));
extern void perror (const char *__s) __attribute__ ((__cold__));
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);
extern int __sprintf_chk (char *__restrict __s, int __flag, size_t __slen,
     const char *__restrict __format, ...) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 3)));
extern int __vsprintf_chk (char *__restrict __s, int __flag, size_t __slen,
      const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 3)));
extern int __snprintf_chk (char *__restrict __s, size_t __n, int __flag,
      size_t __slen, const char *__restrict __format,
      ...) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __vsnprintf_chk (char *__restrict __s, size_t __n, int __flag,
       size_t __slen, const char *__restrict __format,
       __gnuc_va_list __ap) __attribute__ ((__nothrow__ , __leaf__))
    __attribute__ ((__access__ (__write_only__, 1, 2)));
extern int __fprintf_chk (FILE *__restrict __stream, int __flag,
     const char *__restrict __format, ...)
    __attribute__ ((__nonnull__ (1)));
extern int __printf_chk (int __flag, const char *__restrict __format, ...);
extern int __vfprintf_chk (FILE *__restrict __stream, int __flag,
      const char *__restrict __format,
      __gnuc_va_list __ap) __attribute__ ((__nonnull__ (1)));
extern int __vprintf_chk (int __flag, const char *__restrict __format,
     __gnuc_va_list __ap);
extern char *__fgets_alias (char *__restrict __s, int __n, FILE *__restrict __stream) __asm__ ("" "fgets")
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 2)));
extern char *__fgets_chk_warn (char *__restrict __s, size_t __size, int __n, FILE *__restrict __stream) __asm__ ("" "__fgets_chk")
     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fgets called with bigger size than length " "of destination buffer")));
extern char *__fgets_chk (char *__restrict __s, size_t __size, int __n,
     FILE *__restrict __stream)
    __attribute__ ((__warn_unused_result__)) __attribute__ ((__access__ (__write_only__, 1, 3))) __attribute__ ((__nonnull__ (4)));
extern size_t __fread_alias (void *__restrict __ptr, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "fread") __attribute__ ((__warn_unused_result__));
extern size_t __fread_chk_warn (void *__restrict __ptr, size_t __ptrlen, size_t __size, size_t __n, FILE *__restrict __stream) __asm__ ("" "__fread_chk")
     __attribute__ ((__warn_unused_result__)) __attribute__((__warning__ ("fread called with bigger size * nmemb than length " "of destination buffer")));
extern size_t __fread_chk (void *__restrict __ptr, size_t __ptrlen,
      size_t __size, size_t __n,
      FILE *__restrict __stream) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (5)));
extern __inline __attribute__ ((__gnu_inline__)) int
getchar (void)
{
  return getc (stdin);
}
extern __inline __attribute__ ((__gnu_inline__)) int
putchar (int __c)
{
  return putc (__c, stdout);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) sprintf (char *__restrict __s, const char *__restrict __fmt, ...)
{
  return __builtin___sprintf_chk (__s, 3 - 1,
      __builtin_dynamic_object_size (__s, 1), __fmt,
      __builtin_va_arg_pack ());
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) vsprintf (char * __restrict __s, const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __builtin___vsprintf_chk (__s, 3 - 1,
       __builtin_dynamic_object_size (__s, 1), __fmt, __ap);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) snprintf (char *__restrict __s, size_t __n, const char *__restrict __fmt, ...)
{
  return __builtin___snprintf_chk (__s, __n, 3 - 1,
       __builtin_dynamic_object_size (__s, 1), __fmt,
       __builtin_va_arg_pack ());
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
__attribute__ ((__nothrow__ , __leaf__)) vsnprintf (char * __restrict __s, size_t __n, const char *__restrict __fmt, __gnuc_va_list __ap)
    
{
  return __builtin___vsnprintf_chk (__s, __n, 3 - 1,
        __builtin_dynamic_object_size (__s, 1), __fmt, __ap);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
fprintf (FILE *__restrict __stream, const char *__restrict __fmt, ...)
{
  return __fprintf_chk (__stream, 3 - 1, __fmt,
   __builtin_va_arg_pack ());
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
printf (const char *__restrict __fmt, ...)
{
  return __printf_chk (3 - 1, __fmt, __builtin_va_arg_pack ());
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) int
vprintf (const char * __restrict __fmt,
  __gnuc_va_list __ap)
{
  return __vfprintf_chk (stdout, 3 - 1, __fmt, __ap);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__nonnull__ (1))) int
vfprintf (FILE *__restrict __stream,
   const char *__restrict __fmt, __gnuc_va_list __ap)
{
  return __vfprintf_chk (__stream, 3 - 1, __fmt, __ap);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__))
__attribute__ ((__nonnull__ (3))) char *
fgets (char * __restrict __s, int __n,
       FILE *__restrict __stream)
    
{
  size_t __sz = __builtin_dynamic_object_size (__s, 1);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char))))) && (((long unsigned int) (__n)) <= ((__sz)) / ((sizeof (char)))))))
    return __fgets_alias (__s, __n, __stream);
  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))) && !(((long unsigned int) (__n)) <= (__sz) / (sizeof (char)))))
    return __fgets_chk_warn (__s, __sz, __n, __stream);
  return __fgets_chk (__s, __sz, __n, __stream);
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) __attribute__ ((__warn_unused_result__)) __attribute__ ((__nonnull__ (4))) size_t
fread (void * __restrict __ptr,
       size_t __size, size_t __n, FILE *__restrict __stream)
    
{
  size_t __sz = __builtin_dynamic_object_size (__ptr, 0);
  if (((__builtin_constant_p (__sz) && (__sz) == (long unsigned int) -1) || (((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= ((__sz)) / ((__size)))) && (((long unsigned int) (__n)) <= ((__sz)) / ((__size))))))
    return __fread_alias (__ptr, __size, __n, __stream);
  if ((((__typeof (__n)) 0 < (__typeof (__n)) -1 || (__builtin_constant_p (__n) && (__n) > 0)) && __builtin_constant_p ((((long unsigned int) (__n)) <= (__sz) / (__size))) && !(((long unsigned int) (__n)) <= (__sz) / (__size))))
    return __fread_chk_warn (__ptr, __sz, __size, __n, __stream);
  return __fread_chk (__ptr, __sz, __size, __n, __stream);
}


extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *memmove (void *__dest, const void *__src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *memset (void *__s, int __c, size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern void *memchr (const void *__s, int __c, size_t __n)
      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
extern int strcmp (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern int strcoll (const char *__s1, const char *__s2)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
    __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 1, 3)));
extern char *strchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern char *strrchr (const char *__s, int __c)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern size_t strcspn (const char *__s, const char *__reject)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern size_t strspn (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strpbrk (const char *__s, const char *__accept)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strstr (const char *__haystack, const char *__needle)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2)));
extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (2, 3)));
extern size_t strlen (const char *__s)
     __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern char *strerror (int __errnum) __attribute__ ((__nothrow__ , __leaf__));
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memcpy (void *__restrict __dest, const void *__restrict __src, size_t __len)
{
  return __builtin___memcpy_chk (__dest, __src, __len,
     __builtin_dynamic_object_size (__dest, 0));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memmove (void *__dest, const void *__src, size_t __len)
{
  return __builtin___memmove_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 0));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) void *
__attribute__ ((__nothrow__ , __leaf__)) memset (void *__dest, int __ch, size_t __len)
{
  return __builtin___memset_chk (__dest, __ch, __len,
     __builtin_dynamic_object_size (__dest, 0));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strcpy (char * __restrict __dest, const char *__restrict __src)
    
{
  return __builtin___strcpy_chk (__dest, __src, __builtin_dynamic_object_size (__dest, 1));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strncpy (char * __restrict __dest, const char *__restrict __src, size_t __len)
    
{
  return __builtin___strncpy_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 1));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strcat (char * __restrict __dest, const char *__restrict __src)
    
{
  return __builtin___strcat_chk (__dest, __src, __builtin_dynamic_object_size (__dest, 1));
}
extern __inline __attribute__ ((__always_inline__)) __attribute__ ((__gnu_inline__)) __attribute__ ((__artificial__)) char *
__attribute__ ((__nothrow__ , __leaf__)) strncat (char * __restrict __dest, const char *__restrict __src, size_t __len)
    
{
  return __builtin___strncat_chk (__dest, __src, __len,
      __builtin_dynamic_object_size (__dest, 1));
}

_Static_assert(_Generic(_Generic((1), int: b2_identity_int, unsigned: b2_identity_unsigned)(1),int:1,default:0),"identity int result");
_Static_assert(_Generic(_Generic((1u), int: b2_identity_int, unsigned: b2_identity_unsigned)(1u),unsigned:1,default:0),"identity unsigned result");
_Static_assert(_Generic(((void)sizeof(char[_Generic((1),int:_Generic((2),int:1,default:-1),unsigned:_Generic((2),unsigned:1,default:-1),default:-1)]),_Generic((1),int:bf_binary_int,unsigned:bf_binary_unsigned)(1,2)),int:1,default:0),"binary int result");
_Static_assert(_Generic(((void)sizeof(char[_Generic((1u),int:_Generic((2u),int:1,default:-1),unsigned:_Generic((2u),unsigned:1,default:-1),default:-1)]),_Generic((1u),int:bf_binary_int,unsigned:bf_binary_unsigned)(1u,2u)),unsigned:1,default:0),"binary unsigned result");
_Static_assert(_Generic(_Generic(((BfRecord*)0),BfRecord*:bf_mutable,const BfRecord*:bf_const)((BfRecord*)0),BfRecord*:1,default:0),"mutable borrow result");
_Static_assert(_Generic(_Generic(((const BfRecord*)0),BfRecord*:bf_mutable,const BfRecord*:bf_const)((const BfRecord*)0),const BfRecord*:1,default:0),"const borrow result");
static unsigned counts[2],n;static char order[3];
static int left(void *p){(void)p;++counts[0];order[n++]='L';return 7;}
static int right(void *p){(void)p;++counts[1];order[n++]='R';return 9;}
static int same(int x){return b2_identity_int(x);}
int main(void){int i=7,out=0;unsigned u=9;int(*fn)(int)=same;
 if(_Generic((i++), int: b2_identity_int, unsigned: b2_identity_unsigned)(i++)!=7 || i!=8 || _Generic((u++), int: b2_identity_int, unsigned: b2_identity_unsigned)(u++)!=9 || u!=10 || fn(3)!=3 || ((3)+1)!=4 || (same)(3)!=3)return 1;
 if(strcmp("A","A") || strcmp("73","73") || b2_identity_int(73)!=73)return 1;
 if(((void)sizeof(char[_Generic((left(0)),int:_Generic((right(0)),int:1,default:-1),unsigned:_Generic((right(0)),unsigned:1,default:-1),default:-1)]),_Generic((left(0)),int:bf_binary_int,unsigned:bf_binary_unsigned)(left(0),right(0)))!=709 || n!=2 || counts[0]!=1 || counts[1]!=1)return 1;
 printf("direct=%c%c counts=%u,%u\n",order[0],order[1],counts[0],counts[1]);n=0;counts[0]=counts[1]=0;
 if(bf_sequence(left,0,right,0,&out) || out!=709 || n!=2 || order[0]!='L' || order[1]!='R' || counts[0]!=1 || counts[1]!=1)return 1;
 printf("sequenced=%c%c counts=%u,%u raw=%s expanded=%s\n",order[0],order[1],counts[0],counts[1],"A","73");return 0;}
