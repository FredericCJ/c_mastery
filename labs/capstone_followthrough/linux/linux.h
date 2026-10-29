#ifndef CAPSTONE_LINUX_H
#define CAPSTONE_LINUX_H
#include "service.h"
#include <stdint.h>
struct cs_session { int fd,live,ready,eof;unsigned generation; };
struct cs_linux { struct cs_service *core;int epoll;struct cs_session session[2]; };
struct cs_turn { size_t bytes;enum cs_status status;int error; };
void csl_init(struct cs_linux *,struct cs_service *,int);
uint64_t csl_token(const struct cs_linux *,unsigned);
int csl_open(struct cs_linux *,unsigned,int,int);
int csl_close(struct cs_linux *,unsigned);
enum cs_status csl_event(struct cs_linux *,uint64_t);
struct cs_turn csl_turn(struct cs_linux *,unsigned,size_t,int);
struct cs_io csl_write(void *,const unsigned char *,size_t);
#endif
