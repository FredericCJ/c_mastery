"""Execute required changed-schema, stale-consumer and layout investigations."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time
from common import ROOT,dump,sha

CHANGED=r'''#include "schema.h"
#include <stdio.h>
#include <string.h>
static unsigned calls,tags;
static int see(void *p,const struct mps_field *f,int64_t v){(void)p;++calls;tags=tags*10+f->tag;return f->tag!=4||v==15;}
static int old_equal(struct mps_record a,struct mps_record b){return a.present==b.present&&a.code==b.code&&a.value==b.value&&a.note==b.note;}
#define C(x) do{if(!(x)){fprintf(stderr,"MPS_EVOLUTION %d %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){struct mps_record a={15,0x1234,0x89abcdef,-2,15},b=a,sentinel={3,99,88,77,6};unsigned char wire[64],old[64];const struct mps_field *f;size_t n=999,visited=999;unsigned equal=99;
unsigned char expected[]={3,4,1,2,0x12,0x34,2,4,0x89,0xab,0xcd,0xef,3,5,0xff,0xff,0xff,0xfe,1,4,2,0,15};
C(mps_fields(&f)==4&&f[3].offset==offsetof(struct mps_record,quality)&&f[3].native_size==2&&f[3].width==2&&!strcmp(f[3].name,"quality"));
C(mps_validate(&a)==MPS_OK&&mps_encode(3,&a,wire,64,&n)==MPS_OK&&n==23&&!memcmp(wire,expected,23));
C(mps_decode(wire,n,&b)==MPS_OK&&b.quality==15);b.quality=14;C(old_equal(a,b));C(mps_equal(&a,&b,&equal)==MPS_OK&&!equal);
C(mps_visit(&a,see,NULL,&visited)==MPS_OK&&visited==4&&calls==4&&tags==1234);
memcpy(old,wire,64);n=999;C(mps_encode(2,&a,wire,64,&n)==MPS_RANGE&&n==999&&!memcmp(old,wire,64));
a.quality=16;C(mps_validate(&a)==MPS_RANGE&&mps_encode(3,&a,wire,64,&n)==MPS_RANGE&&n==999&&!memcmp(old,wire,64));
wire[22]=16;b=sentinel;C(mps_decode(wire,23,&b)==MPS_RANGE&&b.present==sentinel.present&&b.quality==sentinel.quality&&b.code==sentinel.code);
a.present=7;C(mps_validate(&a)==MPS_OK&&mps_encode(2,&a,wire,64,&n)==MPS_OK&&n==19);C(mps_decode(wire,n,&b)==MPS_OK&&b.quality==0&&b.present==7);
C(mps_encode(3,&b,wire,64,&n)==MPS_OK&&wire[0]==3&&n==19);MPS_LINK_GUARD();
printf("{\"record_size\":%zu,\"offsets\":[%zu,%zu,%zu,%zu],\"wire\":\"030401021234020489abcdef0305fffffffe010402000f\",\"stale_consumer_compiled\":true,\"quality_limit\":15,\"visited\":4}\n",sizeof a,f[0].offset,f[1].offset,f[2].offset,f[3].offset);return 0;}
'''

def run():
    paths=['tools/run_metaprog_schema_evolution.py','tools/common.py','labs/metaprog/schema/schema_stamp.py','labs/metaprog/schema/include/schema.h','labs/metaprog/schema/src/schema_fields.h','assessor/metaprog/schema/schema_fields.h','assessor/metaprog/schema/schema.c','labs/metaprog/schema/CMakeLists.txt','labs/metaprog/schema/tests/tests.c','labs/metaprog/schema/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc','labs/api_depth/architecture/arch.h','assessor/api_depth/architecture/core.c','labs/api_depth/architecture/CONTRACTS.md'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-schema-evolution-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,reject=False,env=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected='diagnostic' if reject else 'success'));assert (p.returncode==1 if reject else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    def preserve(p,label=None):
        dest=folder/(label or p.name);shutil.copyfile(p,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)))
    def save(p,text):p.write_text(text,encoding='utf8');preserve(p,(p.parent.name+'-' if p.name=='schema_fields.h' else '')+p.name)
    for tool in ['gcc','clang','cmake','ninja','nm','size','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    stamp=ROOT/'labs/metaprog/schema/schema_stamp.py';core=ROOT/'assessor/metaprog/schema/schema.c';codec=ROOT/'assessor/api_depth/architecture/core.c';flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror'];public=ROOT/'labs/metaprog/schema/include';api=ROOT/'labs/api_depth/architecture'
    with tempfile.TemporaryDirectory(prefix='cmastery-schema-evolution-') as name:
        tmp=Path(name);old=tmp/'old';new=tmp/'new';old.mkdir();new.mkdir();fields=(ROOT/'assessor/metaprog/schema/schema_fields.h').read_text(encoding='utf8');changed=fields.replace('#define MPS_VERSION 2','#define MPS_VERSION 3').replace('INT32_MAX,2)\n','INT32_MAX,2) \\\n X(3,quality,uint16_t,4,2,0,U16,0,15,3)\n');assert changed!=fields
        save(old/'schema_fields.h',fields);save(new/'schema_fields.h',changed)
        for directory in [old,new]:execute([sys.executable,stamp,directory/'schema_fields.h',directory/'schema_stamp.h'],directory.name+'-generate');preserve(directory/'schema_stamp.h',directory.name+'-schema_stamp.h')
        assert (old/'schema_stamp.h').read_bytes()!=(new/'schema_stamp.h').read_bytes()
        execute([sys.executable,stamp,new/'schema_fields.h',old/'schema_stamp.h','--check'],'stale-input-check','MPS_SCHEMA_STALE',True);execute([sys.executable,stamp,new/'schema_fields.h',new/'schema_stamp.h','--check'],'current-input-check');obs.append(dict(kind='source-check',stale_rejected=True,current_accepted=True,all_input_bytes=True))
        test=tmp/'changed.c';save(test,CHANGED)
        values=re.findall(r'#define MPS_SCHEMA_\d (UINT64_C\(0x[0-9a-f]+\))',(old/'schema_stamp.h').read_text());assert len(values)==4
        stale=tmp/'stale_generated.c';save(stale,'#include "schema.h"\n'+''.join(f'_Static_assert(MPS_SCHEMA_{i}=={v},"MPS_STALE_CONSUMER");\n' for i,v in enumerate(values))+'int main(void){MPS_LINK_GUARD();return 0;}\n')
        link=tmp/'link_consumer.c';save(link,'#include "schema.h"\nint main(void){MPS_LINK_GUARD();return 0;}\n')
        for cc in ['gcc','clang']:
            inc=['-I',public,'-I',new,'-I',api];oldinc=['-I',public,'-I',old,'-I',api]
            layouts=[]
            for layout,extra in [('native',[]),('packed',['-fpack-struct=1'])]:
                exe=tmp/(cc+'-'+layout);execute([cc,*flags,*extra,*inc,test,core,codec,'-o',exe],cc+'-'+layout+'-compile');row=json.loads(execute([exe],cc+'-'+layout+'-execute'));assert row['visited']==4 and row['quality_limit']==15 and row['stale_consumer_compiled'];layouts.append(row);obs.append(dict(kind='changed-schema',compiler=cc,layout=layout,executed=True,**row))
            assert layouts[0]['wire']==layouts[1]['wire'] and layouts[0]['offsets']!=layouts[1]['offsets'] and layouts[0]['record_size']!=layouts[1]['record_size'];obs.append(dict(kind='layout-comparison',compiler=cc,wire_unchanged=True,native_offsets_changed=True,extension='-fpack-struct=1'))
            execute([cc,*flags,*inc,'-c',stale,'-o',tmp/'stale.o'],cc+'-stale-static','MPS_STALE_CONSUMER',True);obs.append(dict(kind='stale-static',compiler=cc,rejected=True,hash_words=4))
            oldobj=tmp/(cc+'-old.o');execute([cc,*flags,*oldinc,'-c',link,'-o',oldobj],cc+'-old-object');execute([cc,*flags,*inc,oldobj,core,codec,'-o',tmp/'badlink'],cc+'-stale-link','mps_schema_',True);obs.append(dict(kind='stale-link',compiler=cc,rejected=True,old_object_executed=False))
            execute([cc,*flags,*inc,link,core,codec,'-o',tmp/'newlink'],cc+'-current-link');execute([tmp/'newlink'],cc+'-current-link-run');obs.append(dict(kind='current-link',compiler=cc,executed=True))
            expanded=execute([cc,*flags,*inc,'-E','-P',test],cc+'-preprocess');assert 'quality' in expanded and 'mps_schema_' in expanded;obs.append(dict(kind='preprocess',compiler=cc,bytes=len(expanded.encode()),tokens=len(re.findall(r'\w+|[^\w\s]',expanded))))
            for opt in ['0','3']:
                obj=tmp/(cc+opt+'.o');execute([cc,*flags,'-O'+opt,*inc,'-c',core,'-o',obj],cc+opt+'-compile');text=execute(['nm','-u',obj],cc+opt+'-imports');helpers=[s.split()[-1] for s in text.splitlines() if s.strip()];assert set(helpers)<={'ar_encode','ar_decode','__stack_chk_fail','memcpy','memset'};execute(['objdump','-dr',obj],cc+opt+'-disassembly');execute(['size',obj],cc+opt+'-size');obs.append(dict(kind='object',compiler=cc,optimization=opt,imports=helpers,limits='Hosted profile permits compiler-generated byte-copy/zero helpers; no freestanding runtime claim.'))
        rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust)]=sha(rust);env=dict(os.environ,LD_LIBRARY_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu');execute([rust,'--version'],'rust-version',env=env)
        rustfile=tmp/'comparison.rs';save(rustfile,'macro_rules! record { ($name:ident { $($field:ident : $ty:ty),* }) => { #[derive(PartialEq)] struct $name { $($field:$ty),* } impl $name { fn names()->Vec<&\'static str>{vec![$(stringify!($field)),*]} } }; }\nrecord!(Record{code:u16,value:u32,note:i32,quality:u16});\nfn main(){let a=Record{code:1,value:2,note:3,quality:4};let b=Record{quality:5,..a};assert!(a!=b);assert_eq!(Record::names(),vec!["code","value","note","quality"]);println!("MPS_RUST declared-macro-registry derived-field-equality");}\n')
        execute([rust,'--edition=2021',rustfile,'-o',tmp/'comparison'],'rust-compile',env=env);execute([tmp/'comparison'],'rust-run','MPS_RUST declared-macro-registry derived-field-equality',env=env);obs.append(dict(kind='language-comparison',language='Rust2021',executed=True,mechanisms=['macro_rules declared syntax repetition','derive PartialEq includes added field'],limits='No C ABI/layout, arbitrary C reflection, or equivalent hygiene guarantee.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/schema','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['All changed field objects rebuilt consistently; stale ABI object is never executed.','Fingerprint binds bytes and generator, not semantic equivalence, authenticity or malicious build control.','Packing is a GCC/Clang extension; canonical wire bytes remain separate.','Exact published C17/C23 not inspected.']);dump(ROOT/'research/v5/modern/metaprog_schema_evolution_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs),artifacts=len(artifacts))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
