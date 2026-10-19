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

typedef long int __jmp_buf[8];
typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
struct __jmp_buf_tag
  {
    __jmp_buf __jmpbuf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };
typedef struct __jmp_buf_tag jmp_buf[1];
extern int setjmp (jmp_buf __env) __attribute__ ((__nothrow__));
extern int __sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask) __attribute__ ((__nothrow__));
extern int _setjmp (struct __jmp_buf_tag __env[1]) __attribute__ ((__nothrow__));
extern void longjmp (struct __jmp_buf_tag __env[1], int __val)
     __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));

typedef enum { ER_OK, ER_ABSENT, ER_INVALID, ER_RANGE, ER_AGAIN, ER_IO,
               ER_THROWN, ER_CONTRACT, ER_NOT_IMPLEMENTED } er_code;
typedef struct { er_code tag; union { uint32_t value; size_t offset; } as; } er_result;
typedef struct { size_t next; er_code code; } er_progress;
typedef er_progress (*er_sink)(void *, const unsigned char *, size_t);
typedef struct { int age, quota, denominator; } er_config;
typedef struct { size_t total, stored; int truncated; } er_validation;
typedef void (*er_cleanup)(void *);
typedef struct {
    jmp_buf jump;
    er_code thrown;
    int active, cleaning;
    size_t count;
    struct { er_cleanup fn; void *context; } cleanup[4];
} er_frame;
typedef enum { ER_CHECKED, ER_WRAPPING, ER_SATURATING } er_policy;
typedef enum { ER_OFF, ER_OBSERVE, ER_ENFORCE } er_mode;
typedef struct { uint32_t numerator, denominator; } er_fraction;
er_result er_ok(uint32_t value);
er_result er_error(er_code code, size_t offset);
er_code er_optional_pack(uint32_t value, int present, uint32_t *encoded);
er_result er_optional_unpack(uint32_t encoded);
er_result er_parse_pair(const unsigned char *bytes, size_t length, uint32_t *sum);
er_progress er_transfer(const unsigned char *bytes, size_t length, size_t next,
                        er_sink sink, void *context);
er_result er_map(er_result input, uint32_t (*fn)(uint32_t, void *), void *context);
er_result er_then(er_result input, er_result (*fn)(uint32_t, void *), void *context);
er_validation er_validate(er_config input, er_config *output, unsigned *errors, size_t capacity);
er_code er_defer(er_frame *frame, er_cleanup fn, void *context);
er_code er_raise(er_frame *frame, er_code code);
er_result er_run(er_frame *frame, er_result (*body)(er_frame *, void *), void *context);
er_code er_add16(uint32_t a, uint32_t b, er_policy policy, uint32_t *output);
er_code er_serial16(uint32_t a, uint32_t b, int *order);
er_code er_contract_counter(er_mode mode, int amount, int *counter, size_t *reports);
er_code er_refine(uint32_t numerator, uint32_t denominator, er_fraction *output);
typedef struct {
    size_t position,error;
    uint32_t first,second,sum;
    unsigned phase,digits,negative,signed_first;
    er_code terminal;
} EprParser;
typedef struct { er_code code; size_t consumed,absolute,error; } EprStep;
er_code epr_init(EprParser *parser,unsigned signed_first);
EprStep epr_feed(EprParser *parser,const unsigned char *bytes,size_t length,
                 unsigned final,uint32_t *sum);
er_result epr_pair_explicit(const unsigned char *bytes,size_t length,uint32_t *sum);
er_result epr_pair_macro(const unsigned char *bytes,size_t length,uint32_t *sum);
static inline er_result epr_identity(er_result result){return result;}
int main(void){er_result out=er_ok(0);
    uint32_t temporary=7;
    if(temporary)do { er_result fresh=(er_ok(temporary)); *(&out)=fresh; if(fresh.tag!=ER_OK && 0!=5)goto failed; } while(0);else return 2;
    if(out.tag!=ER_OK||out.as.value!=7)return 3;
    return 0;
failed:return 1;
}
