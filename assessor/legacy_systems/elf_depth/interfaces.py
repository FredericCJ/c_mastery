"""Original version/size gates and real old/new ELF symbol-version consumers."""
import json


def run(c,root):
    c.group='versioned-interfaces';d=c.root/'interfaces';d.mkdir()
    def source(name,text):return c.write('interfaces/'+name,text)
    def build(name,files,flags=()):
        out=d/name;out.parent.mkdir(parents=True,exist_ok=True)
        c.run(['gcc','-std=c17','-g','-O2','-Wall','-Wextra','-Werror',*files,*flags,'-o',out]);return out
    api=source('api.c',r'''#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>
static int checked(int32_t x,int32_t *out){if(x>INT32_MAX-7)return 2;*out=x+7;return 0;}
__attribute__((visibility("default"))) int toy_apply(const void *input,size_t available,int32_t *out){
 uint32_t version,size,flags=0;int32_t value;
 if(!input||!out||available<8)return 1;
 memcpy(&version,input,4);memcpy(&size,(const unsigned char*)input+4,4);
 if(version!=1||size<12||size>available||(size>12&&size<16))return 1;
 if(size>=16){memcpy(&flags,(const unsigned char*)input+12,4);if(flags&~1u)return 1;}
 memcpy(&value,(const unsigned char*)input+8,4);
 if(flags&1u){if(value<0)return 2;value=-value;}
 return checked(value,out);}
''')
    export=source('api.map','{ global: toy_apply; local: *; };\n')
    so=build('api.so',[api],['-shared','-fPIC','-fvisibility=hidden','-Wl,--version-script='+str(export)])
    symbols=c.run(['nm','-D','--defined-only',so]);c.require(' toy_apply' in symbols and ' checked' not in symbols,'only intended operation is exported',symbols)
    probe=source('probe.c',r'''#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
int main(int argc,char **argv){if(argc!=2)return 90;void *h=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!h)return 91;
 int (*apply)(const void*,size_t,int32_t*)=(int(*)(const void*,size_t,int32_t*))dlsym(h,"toy_apply");if(!apply)return 92;
 struct {uint32_t version,size;int32_t value;uint32_t flags;} input={1,12,35,0};int32_t result=777;
 if(apply(&input,12,&result)||result!=42)return 1;
 input.size=16;if(apply(&input,sizeof input,&result)||result!=42)return 2;
 input.flags=1;if(apply(&input,sizeof input,&result)||result!=-28)return 3;
 for(int i=0;i<6;++i){input.version=i==0?9:1;input.size=i==1?7:i==2?13:i==3?20:16;input.flags=i==4?2:0;input.value=i==5?INT32_MAX:35;result=777;
  if(apply(&input,sizeof input,&result)!=(i==5?2:1)||result!=777)return 4;}
 long page=sysconf(_SC_PAGESIZE);if(page<16)return 93;
 unsigned char *mapping=mmap(NULL,(size_t)page*2,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);if(mapping==MAP_FAILED)return 94;
 if(mprotect(mapping+page,(size_t)page,PROT_NONE))return 95;
 uint32_t bad[2]={9,16};memcpy(mapping+page-8,bad,8);result=777;
 if(apply(mapping+page-8,8,&result)!=1||result!=777)return 5;
 if(munmap(mapping,(size_t)page*2)||dlclose(h))return 96;puts("ABI-GATE-PASS");return 0;}
''')
    probe_text=probe.read_text().replace('if(munmap(mapping,(size_t)page*2)||dlclose(h))return 96;puts','if(munmap(mapping,(size_t)page*2)||dlclose(h)){return 96;}puts');probe.write_text(probe_text)
    test=build('probe',[probe],['-ldl']);c.run([test,so],contains='ABI-GATE-PASS')
    # A wrong implementation that reads an optional field before the size gate
    # faults only inside this original disposable child. A crash is not ACE.
    mutant=source('api-mutant.c',api.read_text().replace('uint32_t version,size,flags=0;int32_t value;','uint32_t version,size,flags=0;int32_t value;\n volatile uint32_t premature=*(const volatile uint32_t*)((const unsigned char*)input+12);(void)premature;'))
    wrong=build('api-mutant.so',[mutant],['-shared','-fPIC']);c.run([test,wrong],expected=-11)
    # Actual GNU symbol versions, separate link-time clients and runtime providers.
    v1=source('v1.map','TOY_1 { global: toy; local: *; };\n')
    v2=source('v2.map','TOY_2 { global: toy; local: *; };\n')
    both=source('both.map','TOY_1 { global: toy; local: *; }; TOY_2 { global: toy; } TOY_1;\n')
    oldsrc=source('old.c','int toy(void){return 11;}\n')
    newsrc=source('new.c','#ifndef OLD_VALUE\n#define OLD_VALUE 11\n#endif\nint old_impl(void){return OLD_VALUE;}int new_impl(void){return 22;}\n__asm__(".symver old_impl,toy@TOY_1");\n__asm__(".symver new_impl,toy@@TOY_2");\n')
    onlynew=source('onlynew.c','int toy(void){return 22;}\n')
    providers={}
    for name,src,version,flags in [('old',oldsrc,v1,[]),('new',newsrc,both,[]),('onlynew',onlynew,v2,[]),('semantic-break',newsrc,both,['-DOLD_VALUE=22'])]:
        providers[name]=build(name+'/libtoy.so',[src],['-shared','-fPIC','-Wl,-soname,libtoy.so','-Wl,--version-script='+str(version),*flags]);c.run(['readelf','--dyn-syms','--version-info',providers[name]])
    client=source('client.c','#include <stdio.h>\n#ifndef EXPECT\n#define EXPECT 11\n#endif\nint toy(void);int main(void){int value=toy();printf("observed=%d expected=%d\\n",value,EXPECT);return value==EXPECT?0:33;}\n')
    clients={}
    for name,expect in [('old',11),('new',22)]:
        clients[name]=build('client-'+name,[client],['-DEXPECT='+str(expect),'-L'+str(providers[name].parent),'-ltoy']);c.run(['readelf','--version-info',clients[name]])
    rows=[]
    for consumer,provider,status,marker in [('old','old',0,'observed=11'),('old','new',0,'observed=11'),('new','new',0,'observed=22'),('new','old',1,"TOY_2' not found"),('old','onlynew',1,"TOY_1' not found"),('old','semantic-break',33,'observed=22 expected=11')]:
        out=c.run([clients[consumer]],env={'LD_LIBRARY_PATH':str(providers[provider].parent),'LD_PRELOAD':'','LD_BIND_NOW':'1'},expected=status,contains=marker);rows.append(dict(consumer=consumer,provider=provider,status=status,output=out))
    c.write('interfaces/version-matrix.json',json.dumps(rows,indent=2))
    # Public representation source rebuild succeeds while old offsets differ.
    public=source('public.c','#include <stddef.h>\n#include <stdio.h>\n#include <stdint.h>\nstruct Public {\n#ifdef NEW_LAYOUT\n uint64_t stamp;\n#endif\n int32_t x,y;};int main(void){struct Public x={.x=11,.y=22};printf("size=%zu x=%zu y=%zu sum=%d\\n",sizeof x,offsetof(struct Public,x),offsetof(struct Public,y),x.x+x.y);return x.x+x.y!=33;}\n')
    for name,flags,expect in [('old',[],'size=8 x=0 y=4'),('new',['-DNEW_LAYOUT'],'size=16 x=8 y=12')]:
        exe=build('layout-'+name,[public],flags);c.run([exe],contains=expect)
    # Opaque representation changes behind an unchanged ABI and provider-owned free.
    opaque=source('opaque.c',r'''#include <stdlib.h>
struct Object {
#ifdef NEW_LAYOUT
 unsigned char padding[64];unsigned long generation;
#endif
 int value;};
struct Object *object_create(int value){struct Object *p=malloc(sizeof *p);if(p)p->value=value;return p;}
int object_read(const struct Object *p){return p->value;}
void object_destroy(struct Object *p){free(p);}
''')
    opclient=source('opaque-client.c','struct Object;struct Object *object_create(int);int object_read(const struct Object*);void object_destroy(struct Object*);int main(void){struct Object *p=object_create(42);if(!p)return 2;int value=object_read(p);object_destroy(p);return value!=42;}\n')
    for name,flags in [('old',[]),('new',['-DNEW_LAYOUT'])]:build(name+'/libopaque.so',[opaque],['-shared','-fPIC','-Wl,-soname,libopaque.so',*flags])
    op=build('opaque-client',[opclient],['-L'+str(d/'old'),'-lopaque'])
    for name in ('old','new'):c.run([op],env={'LD_LIBRARY_PATH':str(d/name),'LD_PRELOAD':''})
    # Same C signature and symbol set, changed ownership detected without invalid free.
    ownership=source('ownership.c',r'''#include <stdlib.h>
static char *owned;static char borrowed[3]={'o','k',0};
char *message(void){
#ifdef BORROWED
 return borrowed;
#else
 owned=malloc(3);if(owned){owned[0]=borrowed[0];owned[1]='k';owned[2]=0;}return owned;
#endif
}
int message_release(char *p){if(p!=owned||!owned)return 1;free(owned);owned=NULL;return 0;}
''')
    ownclient=source('ownership-client.c','char *message(void);int message_release(char*);int main(void){char *p=message();if(!p)return 2;return message_release(p)?44:0;}\n')
    for name,flags in [('owned',[]),('borrowed',['-DBORROWED'])]:build(name+'/libmessage.so',[ownership],['-shared','-fPIC','-Wl,-soname,libmessage.so',*flags])
    own=build('ownership-client',[ownclient],['-L'+str(d/'owned'),'-lmessage'])
    c.run([own],env={'LD_LIBRARY_PATH':str(d/'owned'),'LD_PRELOAD':''});c.run([own],env={'LD_LIBRARY_PATH':str(d/'borrowed'),'LD_PRELOAD':''},expected=44)
    c.write('interfaces/compatibility.json',json.dumps([
      dict(change='prepend public field',source_rebuild='passes designated-field client',old_symbol_link='names unchanged',old_layout='incompatible; never executed',semantic='requires version/size rejection'),
      dict(change='opaque private layout',source_rebuild='not required',old_symbol_link='passes',old_layout='not exposed',semantic='create/read/destroy contract passed with old binary'),
      dict(change='owned return becomes borrowed',source_rebuild='same C types compile',old_symbol_link='passes',old_layout='unchanged',semantic='release obligation fails44 without unsafe free')],indent=2))
    for suffix,ordinary,variant in [
      ('11.a','Original version/size operation validates bounded prefix before later fields; guard-page mutant exposes premature access','Old12-byte caller retains defaults;16-byte optional tail changes operation under checked flags'),
      ('11.b','Actual old/new GNU-versioned clients execute supported pairs and reject missing required version','Unchanged TOY_1 name with changed11→22 behavior fails semantic compatibility despite successful resolution'),
      ('11.c','Public layout rebuild changes actual offsets; unchanged opaque client executes against two private layouts','Same C return/release types with changed ownership violate release contract without invalid-free demonstration')]:
        c.observe('T09.'+suffix,ordinary,['versioned-interfaces']);c.observe('T09.'+suffix,variant,['versioned-interfaces'],True,limitations=['Original trusted DSOs only; guard-page fault demonstrates premature read, not code execution.'])
