/* Original intentionally flawed target. Synthetic sentinel only, no network.
 * mismatch is executed only under CFI, never as an unchecked incompatible call. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
typedef void (*md_callback_fn)(unsigned *);
static void query(unsigned *v){*v+=1;}
static void admin(unsigned *v){*v=77;}
static void clear(unsigned *v){*v=0;}
static void wrong(unsigned v){(void)v;}
static md_callback_fn const readonly_table[3] __attribute__((used))={query,admin,clear};
int dispatch_index_authorized(unsigned request,unsigned selected,unsigned grant);
static int table_readonly(void){FILE*f=fopen("/proc/self/maps","r");if(!f)return 0;char line[512],mode[5];uintptr_t first,last,at=(uintptr_t)readonly_table;int found=0;while(fgets(line,sizeof line,f))if(sscanf(line,"%" SCNxPTR "-%" SCNxPTR " %4s",&first,&last,mode)==3&&at>=first&&at<last){found=mode[0]=='r'&&mode[1]!='w';break;}fclose(f);return found;}
int main(int argc,char **argv){if(argc==3&&(!strcmp(argv[1],"index")||!strcmp(argv[1],"index-repaired"))){char*end;unsigned long input=strtoul(argv[2],&end,10);if(*end||input>3)return 2;unsigned sentinel=91,request=0,corrupted_index=(unsigned)input;if(!table_readonly())return 3;if(corrupted_index>=3||(!strcmp(argv[1],"index-repaired")&&!dispatch_index_authorized(request,corrupted_index,0))){puts("INDEX_DENIED sentinel=91 readonly=1");return 0;}md_callback_fn const volatile*table=readonly_table;md_callback_fn f=table[corrupted_index];f(&sentinel);printf("INDEX_RESULT index=%u sentinel=%u readonly=1\n",corrupted_index,sentinel);return 0;}if(argc!=2)return 2;unsigned sentinel=91;md_callback_fn allowed[3]={query,admin,clear};md_callback_fn volatile selected=allowed[1];
 if(!strcmp(argv[1],"mismatch"))selected=(md_callback_fn)wrong;
 else if(!strcmp(argv[1],"repaired")){unsigned operation=1,grant=0;if(operation>2||(operation!=0&&!grant)){puts("DENIED sentinel=91");return 0;}selected=allowed[operation];}
 else if(strcmp(argv[1],"same-type"))return 2;
 selected(&sentinel);printf("sentinel=%u\n",sentinel);return sentinel==77?0:1;}
