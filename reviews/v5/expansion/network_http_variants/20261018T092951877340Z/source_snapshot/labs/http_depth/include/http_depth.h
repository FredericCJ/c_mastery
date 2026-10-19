#ifndef HTTP_DEPTH_H
#define HTTP_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum { HD_OK=0, HD_INVALID=1, HD_FULL=2, HD_STALE=3, HD_UNIMPLEMENTED=99 };
/* Decimal field-value after OWS trimming, independent of size_t width.
   Return 0 for valid <=cap, 400 for syntax/u32 overflow, 413 for >cap.
   Output unchanged unless zero. cap <=1024. No terminator required. */
int hd_length(const unsigned char *,size_t,uint32_t,uint32_t *);
/* Proleptic Gregorian days since1970-01-01 and ordinary seconds within day.
   available=0 omits Date: length=0 and output unchanged. Otherwise exactly
   29 ASCII bytes + NUL, length29. No leap-second or clock-accuracy claim. */
int hd_date(unsigned available,uint32_t days,uint32_t seconds,char *,size_t,size_t *);
typedef struct { uint32_t index,generation; } HDHandle;
typedef struct { int fd; uint32_t generation; unsigned live,retired,interest; } HDSlot;
typedef struct { HDSlot slots[4]; } HDPool;
/* Zero-initialize pool once before use. flags_ready declares adapter verified
   nonblocking+close-on-exec; caller retains descriptor ownership on failure. */
int hd_admit(HDPool *,int fd,unsigned flags_ready,HDHandle *);
int hd_interest(HDPool *,HDHandle,unsigned mask);
int hd_event(const HDPool *,HDHandle,int fd,unsigned ready,unsigned *dispatch);
int hd_close(HDPool *,HDHandle,int *fd_to_close);
#endif
