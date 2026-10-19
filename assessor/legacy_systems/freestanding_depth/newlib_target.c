/* Explicit permitted-library comparison, NEVER the strict no-library target. */
#include "target_port.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
static unsigned heap_calls,write_calls,refuse;
static unsigned char captured[64];static size_t accepted;
#ifndef OMIT_HOOKS
void *_sbrk(ptrdiff_t n){(void)n;++heap_calls;errno=ENOMEM;return(void *)-1;}
_ssize_t _write(int fd,const void *b,size_t n){
 if(fd!=1&&fd!=2){errno=EBADF;return -1;}
 ++write_calls;if(refuse){errno=EIO;return -1;}
 size_t count=n<2?n:2;if(count>sizeof captured-accepted){errno=ENOSPC;return -1;}
 for(size_t i=0;i<count;++i)captured[accepted++]=((const unsigned char *)b)[i];return(_ssize_t)count;
}
int _close(int fd){(void)fd;errno=ENOSYS;return -1;}
int _fstat(int fd,struct stat *st){(void)fd;(void)st;errno=ENOSYS;return -1;}
int _isatty(int fd){(void)fd;errno=ENOTTY;return 0;}
_off_t _lseek(int fd,_off_t offset,int whence){(void)fd;(void)offset;(void)whence;errno=ESPIPE;return(_off_t)-1;}
_ssize_t _read(int fd,void *buf,size_t n){(void)fd;(void)buf;(void)n;errno=ENOSYS;return -1;}
#endif
void fr_fault(void){fr_semihost(4,"NEWLIB-FAULT\n");fr_stop(2);}
void target_main(void){
 static char storage[24];
#ifndef OMIT_HOOKS
 if(_sbrk(1)!=(void *)-1||errno!=ENOMEM||heap_calls!=1)fr_stop(7);
 unsigned char untouched=93;struct stat state;state.st_mode=77;
 if(_read(0,&untouched,1)!=-1||untouched!=93||_fstat(1,&state)!=-1||state.st_mode!=77||_close(1)!=-1||_lseek(1,0,0)!=-1||_isatty(1)!=0)fr_stop(8);
#endif
 if(setvbuf(stdout,storage,_IOFBF,sizeof storage)){fr_stop(3);}
 int result=printf("value=%u\n",37U);int flushed=fflush(stdout);const char expected[]="value=37\n";
 if(result!=9||flushed||accepted!=9||write_calls<5||heap_calls!=1){fr_semihost(4,"NEWLIB-RESULT-FAIL\n");fr_stop(4);}
 for(unsigned i=0;i<9;++i)if(captured[i]!=(unsigned char)expected[i])fr_stop(5);
 refuse=1;if(printf("x")!=1||fflush(stdout)!=EOF||!ferror(stdout)){fr_stop(6);}
 fr_semihost(4,"NEWLIB-PASS:short-write=5:refused-heap:buffer=24:error-visible\n");fr_stop(0);
}
