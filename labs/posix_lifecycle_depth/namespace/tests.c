#define _GNU_SOURCE
#include "path.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/openat2.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/statfs.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/xattr.h>
#include <unistd.h>
#ifndef PN_MUTANT
#define PN_MUTANT 0
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PL_CHECK:%d:%s errno=%d\n",__LINE__,#x,errno);exit(1);}}while(0)
static int injected;
long __real_syscall(long,...);
long __wrap_syscall(long number,...){
    va_list ap;int d;const char *p;const struct open_how *h;size_t n;
    CHECK(number==SYS_openat2);va_start(ap,number);d=va_arg(ap,int);p=va_arg(ap,const char *);
    h=va_arg(ap,const struct open_how *);n=va_arg(ap,size_t);va_end(ap);
    if(injected){errno=injected;return -1;}return __real_syscall(number,d,p,h,n);
}
static int count_fds(void){int count=0;DIR *d=opendir("/proc/self/fd");struct dirent *e;CHECK(d);while((e=readdir(d)))if(strcmp(e->d_name,".")&&strcmp(e->d_name,".."))++count;CHECK(closedir(d)==0);return count;}
static void write_byte(int fd,char c){ssize_t n;do{n=write(fd,&c,1);}while(n<0&&errno==EINTR);CHECK(n==1);}
static char read_byte(int fd){char c='?';ssize_t n;do{n=read(fd,&c,1);}while(n<0&&errno==EINTR);CHECK(n==1);return c;}
static void reap(pid_t pid){int status;pid_t got;do{got=waitpid(pid,&status,0);}while(got<0&&errno==EINTR);CHECK(got==pid&&WIFEXITED(status)&&WEXITSTATUS(status)==0);}
static int file_at(int d,const char *name,const char *text){int fd=openat(d,name,O_RDWR|O_CREAT|O_EXCL|O_CLOEXEC,0600);size_t n=strlen(text);CHECK(fd>=0);CHECK(write(fd,text,n)==(ssize_t)n);return fd;}
static void value(int fd,const char *expected){char b[16]={0};size_t n=strlen(expected);CHECK(pread(fd,b,sizeof b,0)==(ssize_t)n);CHECK(memcmp(b,expected,n)==0);}
static void identity(int d){
    int original=file_at(d,"old","SAFE"),current,alias;struct stat a,b;
    CHECK(renameat(d,"old",d,"saved")==0);current=file_at(d,"old","OTHER");
#if PN_MUTANT == 7
    CHECK(close(original)==0);original=openat(d,"old",O_RDONLY|O_CLOEXEC);CHECK(original>=0);
#endif
    CHECK(fstat(original,&a)==0&&fstat(current,&b)==0);CHECK(a.st_dev==b.st_dev&&a.st_ino!=b.st_ino);value(original,"SAFE");value(current,"OTHER");
#if PN_MUTANT == 8
    alias=file_at(d,"alias","SAFE");
#else
    CHECK(linkat(d,"saved",d,"alias",0)==0);alias=openat(d,"alias",O_RDONLY|O_CLOEXEC);CHECK(alias>=0);
#endif
    CHECK(fstat(alias,&b)==0);CHECK(a.st_dev==b.st_dev&&a.st_ino==b.st_ino);CHECK(unlinkat(d,"saved",0)==0);CHECK(unlinkat(d,"alias",0)==0);
    CHECK(fstat(original,&a)==0&&a.st_nlink==0);value(original,"SAFE");value(alias,"SAFE");CHECK(close(alias)==0&&close(current)==0&&close(original)==0);
}
static void anchored(int outer){
    int cwd=open(".",O_RDONLY|O_DIRECTORY|O_CLOEXEC),d,fd;char *base=getcwd(NULL,0);char absolute[4096];CHECK(base&&cwd>=0);
    CHECK(mkdirat(outer,"anchor",0700)==0);d=openat(outer,"anchor",O_RDONLY|O_DIRECTORY|O_CLOEXEC);CHECK(d>=0);fd=file_at(d,"value","SAFE");CHECK(close(fd)==0);
    CHECK(snprintf(absolute,sizeof absolute,"%s/anchor/value",base)>0);free(base);CHECK(renameat(outer,"anchor",outer,"renamed")==0);CHECK(mkdirat(outer,"anchor",0700)==0);
    CHECK(chdir("anchor")==0);
#if PN_MUTANT == 9
    fd=openat(AT_FDCWD,"value",O_RDONLY|O_CLOEXEC);
#else
    fd=openat(d,"value",O_RDONLY|O_CLOEXEC);
#endif
    CHECK(fd>=0);value(fd,"SAFE");CHECK(close(fd)==0);fd=file_at(d,"created","C");CHECK(close(fd)==0);
    /* Update only the explicit absolute spelling to the renamed directory. */
    CHECK(fchdir(cwd)==0);base=getcwd(NULL,0);CHECK(base);CHECK(snprintf(absolute,sizeof absolute,"%s/renamed/value",base)>0);free(base);
#if PN_MUTANT == 10
    fd=openat(-1,"renamed/value",O_RDONLY|O_CLOEXEC);
#else
    fd=openat(-1,absolute,O_RDONLY|O_CLOEXEC);
#endif
    CHECK(fd>=0);value(fd,"SAFE");CHECK(close(fd)==0&&close(d)==0&&close(cwd)==0);
}
static void confinement(int outer){
    int root,other,fd=-123,go[2],done[2],status;pid_t child;struct stat before;char *absolute;
    CHECK(mkdirat(outer,"root",0700)==0&&mkdirat(outer,"other",0700)==0);
    root=openat(outer,"root",O_RDONLY|O_DIRECTORY|O_CLOEXEC);other=openat(outer,"other",O_RDONLY|O_DIRECTORY|O_CLOEXEC);CHECK(root>=0&&other>=0);
    fd=file_at(root,"safe","SAFE");CHECK(close(fd)==0);fd=file_at(root,"other","OTHER");CHECK(close(fd)==0);fd=file_at(other,"value","OTHER");CHECK(close(fd)==0);
    fd=-123;CHECK(pn_open_confined(root,"safe",&fd)==0&&fd>=0);value(fd,"SAFE");CHECK(fcntl(fd,F_GETFD)&FD_CLOEXEC);CHECK(close(fd)==0);
    CHECK(symlinkat("safe",root,"selected")==0);CHECK(fstatat(root,"selected",&before,0)==0);CHECK(pipe2(go,O_CLOEXEC)==0&&pipe2(done,O_CLOEXEC)==0);
    child=fork();CHECK(child>=0);if(child==0){close(go[1]);close(done[0]);CHECK(read_byte(go[0])=='G');CHECK(unlinkat(root,"selected",0)==0);CHECK(symlinkat("other",root,"selected")==0);write_byte(done[1],'D');_exit(0);}
    CHECK(close(go[0])==0&&close(done[1])==0);write_byte(go[1],'G');CHECK(read_byte(done[0])=='D');reap(child);CHECK(close(go[1])==0&&close(done[0])==0);
    fd=openat(root,"selected",O_RDONLY|O_CLOEXEC);CHECK(fd>=0);value(fd,"OTHER");CHECK(close(fd)==0); /* Harmless flaw demonstrated. */
    fd=openat(root,"selected",O_RDONLY|O_CLOEXEC|O_NOFOLLOW);CHECK(fd<0&&errno==ELOOP);
    fd=-123;CHECK(pn_open_confined(root,"selected",&fd)==ELOOP&&fd==-123);
    CHECK(symlinkat("../other",root,"middle")==0);fd=openat(root,"middle/value",O_RDONLY|O_CLOEXEC|O_NOFOLLOW);CHECK(fd>=0);value(fd,"OTHER");CHECK(close(fd)==0);
    fd=-123;CHECK(pn_open_confined(root,"middle/value",&fd)==ELOOP&&fd==-123);
    status=pn_open_confined(root,"../other/value",&fd);CHECK(status==EXDEV&&fd==-123);
    absolute=realpath("other/value",NULL);CHECK(absolute);CHECK(pn_open_confined(root,absolute,&fd)==EXDEV&&fd==-123);free(absolute);
    CHECK(pn_open_confined(root,"missing",&fd)==ENOENT&&fd==-123);CHECK(pn_open_confined(root,"",&fd)==EINVAL&&fd==-123);
    CHECK(mkdirat(root,"directory",0700)==0);CHECK(pn_open_confined(root,"directory",&fd)==EINVAL&&fd==-123);
    CHECK(mkfifoat(root,"pipe",0600)==0);CHECK(pn_open_confined(root,"pipe",&fd)==EINVAL&&fd==-123);
    injected=ENOSYS;CHECK(pn_open_confined(root,"safe",&fd)==ENOSYS&&fd==-123);injected=EAGAIN;CHECK(pn_open_confined(root,"safe",&fd)==EAGAIN&&fd==-123);injected=0;
    /* A hard link names the same inode without traversing a symlink. It is
       deliberately permitted: path confinement is not provenance/authenticity. */
    CHECK(linkat(other,"value",root,"hard",0)==0);CHECK(pn_open_confined(root,"hard",&fd)==0);value(fd,"OTHER");CHECK(close(fd)==0&&close(other)==0&&close(root)==0);
}
static void print_info(const char *name,PNInfo p){printf("PN_META %s dev=%ju inode=%ju mode=%jo owner=%ju group=%ju links=%ju bytes=%jd blocks512=%jd preferred=%jd atime=%jd.%09ld mtime=%jd.%09ld ctime=%jd.%09ld\n",name,(uintmax_t)p.device,(uintmax_t)p.inode,(uintmax_t)p.mode,(uintmax_t)p.owner,(uintmax_t)p.group,(uintmax_t)p.links,(intmax_t)p.logical_bytes,(intmax_t)p.blocks_512,(intmax_t)p.preferred_io,(intmax_t)p.access.tv_sec,p.access.tv_nsec,(intmax_t)p.modification.tv_sec,p.modification.tv_nsec,(intmax_t)p.status_change.tv_sec,p.status_change.tv_nsec);}
static void metadata(int d){
    int fd=file_at(d,"regular","SAFE");PNInfo regular,link,directory,sparse,saved;struct stat independent;struct statfs fs;struct timespec requested[2]={{123456789,123456789},{123456790,987654321}};
    CHECK(symlinkat("regular",d,"link")==0&&mkdirat(d,"directory",0700)==0);
    CHECK(pn_info(d,"regular",true,&regular)==0);CHECK(pn_info(d,"link",false,&link)==0);CHECK(pn_info(d,"directory",false,&directory)==0);
    CHECK(S_ISREG(regular.mode)&&S_ISLNK(link.mode)&&S_ISDIR(directory.mode));CHECK(regular.logical_bytes==4&&link.logical_bytes==7);CHECK(fstat(fd,&independent)==0);
    CHECK(regular.blocks_512==independent.st_blocks&&regular.owner==geteuid()&&regular.group==independent.st_gid);CHECK(pn_info(d,"link",true,&saved)==0&&saved.inode==regular.inode&&saved.device==regular.device);
    memcpy(&saved,&regular,sizeof saved);CHECK(pn_info(d,"missing",true,&regular)==ENOENT);CHECK(memcmp(&saved,&regular,sizeof saved)==0);
    CHECK(ftruncate(fd,1024*1024)==0&&pwrite(fd,"Z",1,1024*1024-1)==1&&futimens(fd,requested)==0);CHECK(pn_info(d,"regular",true,&sparse)==0&&sparse.logical_bytes==1024*1024);
    CHECK(sparse.modification.tv_nsec>=0&&sparse.modification.tv_nsec<1000000000L);CHECK(fstatfs(fd,&fs)==0);
    print_info("regular-before",saved);print_info("symlink",link);print_info("directory",directory);print_info("sparse-after",sparse);printf("PN_FS type=%lx requested_mtime=123456790.987654321\n",(unsigned long)fs.f_type);CHECK(close(fd)==0);
}
static void policy(int d){
    unsigned installed=1234;pid_t child;int fd;PNAuthority a={true,false,false,true};struct stat s;
    CHECK(pn_creation_mode(0666,0027,false,0,&installed)==0&&installed==0640);
    CHECK(pn_creation_mode(0666,0077,true,0755,&installed)==0&&installed==0644);
    CHECK(pn_creation_mode(0600,0000,true,0777,&installed)==0&&installed==0600);
    installed=1234;CHECK(pn_creation_mode(01000,0,false,0,&installed)==EINVAL&&installed==1234);
    CHECK(pn_authorize(a,PN_READ)==EACCES&&pn_authorize(a,PN_WRITE)==EACCES&&pn_authorize(a,PN_REPLACE)==0);
    a.file_write=true;a.parent_write=false;CHECK(pn_authorize(a,PN_WRITE)==0&&pn_authorize(a,PN_REPLACE)==EACCES);a.searchable=false;CHECK(pn_authorize(a,PN_WRITE)==EACCES);CHECK(pn_authorize(a,(enum PNOperation)9)==EINVAL);
    errno=0;CHECK(fgetxattr(d,"system.posix_acl_default",NULL,0)<0&&errno==ENODATA); /* Explicit no-default-ACL precondition. */
    child=fork();CHECK(child>=0);if(child==0){umask(0027);fd=openat(d,"mode",O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC,0666);CHECK(fd>=0);CHECK(fstat(fd,&s)==0&&(s.st_mode&0777)==0640);CHECK(close(fd)==0);_exit(0);}reap(child);
    CHECK(fstatat(d,"mode",&s,0)==0);printf("PN_AUTH actual_child_mode=%jo injected_read_error=%d replacement_model=parent-write-and-search\n",(uintmax_t)(s.st_mode&0777),EACCES);
}
int main(int argc,char **argv){char local[]="./pn-owned-XXXXXX";int d,before=count_fds();CHECK(argc==2);CHECK(mkdtemp(local)&&chdir(local)==0);d=open(".",O_RDONLY|O_DIRECTORY|O_CLOEXEC);CHECK(d>=0);
    if(!strcmp(argv[1],"identity"))identity(d);else if(!strcmp(argv[1],"anchored"))anchored(d);else if(!strcmp(argv[1],"confined"))confinement(d);else if(!strcmp(argv[1],"metadata"))metadata(d);else if(!strcmp(argv[1],"policy"))policy(d);else CHECK(0);
    CHECK(close(d)==0&&count_fds()==before);puts("PN_PASS");return 0;}
