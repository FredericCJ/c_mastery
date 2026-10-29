#define _GNU_SOURCE
#include "context.h"
#include "test.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
int main(void){struct pc_capability c={0},saved;CHECK(pc_choose(&c,1,0,1,0)==0&&c.backend==1);CHECK(pc_choose(&c,1,ENOSYS,1,0)==0&&c.backend==1);saved=c;CHECK(pc_choose(&c,2,ENOSYS,1,1)==EBUSY&&!memcmp(&c,&saved,sizeof c));CHECK(pc_choose(&c,2,ENOSYS,1,0)==0&&c.backend==2);CHECK(pc_choose(&c,3,EOPNOTSUPP,1,0)==0&&c.backend==2);CHECK(pc_choose(&c,4,EIO,1,0)==EIO&&c.backend==3);CHECK(pc_choose(&c,5,ENOSYS,0,0)==ENOSYS&&c.backend==3);CHECK(pc_choose(&c,6,0,1,0)==0&&c.backend==1);CHECK(pc_operation_status(ENOSYS,0)==1&&pc_operation_status(EOPNOTSUPP,1)==2&&pc_operation_status(0,2)==0);puts("PC_CAP_MODEL epochs=6 unsupported=ENOSYS,EOPNOTSUPP io_error_preserved weaker_fallback_rejected partial_not_replayed");
 for(int backend=1;backend<=2;++backend){int p[2];char b;CHECK(pipe2(p,O_CLOEXEC)==0);CHECK(pc_ready(backend,p[0])==0);CHECK(write(p[1],"x",1)==1&&pc_ready(backend,p[0])==1&&pc_ready(backend,p[0])==1);CHECK(read(p[0],&b,1)==1&&b=='x'&&pc_ready(backend,p[0])==0);close(p[1]);CHECK(pc_ready(backend,p[0])==1);close(p[0]);printf("PC_BACKEND %s empty=0 readable=1 repeated=1 drained=0 eof=1\n",backend==1?"epoll":"poll");}CHECK(pc_ready(3,0)==-1&&errno==ENOTSUP);puts("PC_CAPABILITY_PASS");return 0;}
