"""Actual generated variants,type boundaries,assembly and strict component closure."""
from datetime import datetime,timezone
from pathlib import Path
import copy,json,re,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/registry_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/registry';private=ROOT/'assessor/language_portability/registry';inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.rglob('*') if p.is_file()}
 for p in ['tools/run_portability_registry_companion.py','tools/common.py','fixtures/language/PR-T06-02.json']:inputs[p]=sha(ROOT/p)
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('registry-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];external={};dependencies=[];variants=[];probes=[];targets=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd and expected==0:dep=home/(str(len(records))+'-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=60);text=p.stdout+p.stderr;log=home/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,text
  if dep:
   paths=[Path(q).resolve() for q in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if not q.is_relative_to(ROOT):external[str(q)]=sha(q)
  return p.stdout,p.stderr
 for tool in ['gcc','clang','nm','objdump','readelf','size']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
 baseline=read_json(public/'registry.json');added=copy.deepcopy(baseline);added['rows'].append(dict(id=9,name='reset',handler='reset',aliases=[]));alias=copy.deepcopy(baseline);alias['rows'][0]['aliases']=['query','state'];sparse=copy.deepcopy(baseline);sparse['rows'][0]['id']=2147483647
 cases=[('ordinary',baseline),('added',added),('reordered',dict(rows=list(reversed(baseline['rows'])))),('aliases',alias),('sparse',sparse)]
 canonical=None
 for label,value in cases:
  work=home/label;work.mkdir();inp=work/'registry.json';dump(inp,value);gen=work/'generated';execute([sys.executable,public/'generate.py',inp,gen],label+'-generate');execute([sys.executable,public/'generate.py',inp,gen,'--check'],label+'-check')
  hashes={p.name:sha(p) for p in gen.iterdir()};ordered=sorted(value['rows'],key=lambda r:r['id'])
  if label=='ordinary':canonical=hashes
  if label=='reordered':assert hashes==canonical
  source=['#include "registry.h"','#include <stdio.h>','#define C(x) do{if(!(x)){puts("LR_COMPANION_CHECK");return 1;}}while(0)','int main(void){int out=77;uint32_t id=88;const char *name="sentinel";']
  values={'start':1,'stop':4,'status':7,'reset':9}
  for i,row in enumerate(ordered):
   ident=row['id'];name=row['name'];source.append(f'C(lr_at({i},&id,&name)==0 && id=={ident}u);C(lr_dispatch(id,10,&out)==0 && out=={10+values[row["handler"]]});C(lr_probes(id)<=6);')
   for label_name in [name]+row.get('aliases',[]):source.append(f'C(lr_id("{label_name}",&id)==0 && id=={ident}u);')
   source.append(f'C(lr_name(id,&name)==0);C(name[0]==\'{name[0]}\');')
  source.append('out=77;C(lr_dispatch(4294967295u,10,&out)==-1 && out==77);C(lr_count()=='+str(len(ordered))+');return 0;}')
  test=work/'test.c';test.write_text('\n'.join(source)+'\n',encoding='utf8')
  for cc in ['gcc','clang']:
   objs=[]
   for n,src in enumerate([private/'registry.c',test]):
    obj=work/(cc+str(n)+'.o');execute([cc,'-std=c17','-O3','-Wall','-Wextra','-Werror','-pedantic-errors','-I'+str(work),'-I'+str(public),'-c',src,'-o',obj],label+'-'+cc+'-object');objs.append(obj)
   exe=work/cc;execute([cc,*objs,'-o',exe],label+'-'+cc+'-link');execute([exe],label+'-'+cc+'-execute')
  variants.append(dict(kind=label,generated=hashes,rows=len(ordered),maximum_id=max(r['id'] for r in ordered),search_entries=len(ordered),direct_index_slots=max(r['id'] for r in ordered)+1))
 # Admission failures preserve every preexisting published byte.
 invalid=[]
 for kind in ['duplicate-id','duplicate-name','missing-handler','unknown-handler','no-rows','too-many','id-range','alias-collision']:
  v=copy.deepcopy(baseline)
  if kind=='duplicate-id':v['rows'][1]['id']=7
  elif kind=='duplicate-name':v['rows'][1]['name']='status'
  elif kind=='missing-handler':del v['rows'][0]['handler']
  elif kind=='unknown-handler':v['rows'][0]['handler']='absent'
  elif kind=='no-rows':v['rows']=[]
  elif kind=='too-many':v['rows']=[dict(id=i,name='n'+str(i),handler='start') for i in range(33)]
  elif kind=='id-range':v['rows'][0]['id']=2147483648
  else:v['rows'][0]['aliases']=['start']
  inp=home/(kind+'.json');dump(inp,v);out=home/(kind+'-output');out.mkdir();(out/'sentinel').write_bytes(b'unchanged');execute([sys.executable,public/'generate.py',inp,out],kind,1);assert {p.name:p.read_bytes() for p in out.iterdir()}=={'sentinel':b'unchanged'};invalid.append(kind)
 stale=home/'stale';shutil.copytree(home/'ordinary',stale);p=stale/'generated/names.inc';p.write_text(p.read_text().replace('"start"','"wrong"'));execute([sys.executable,public/'generate.py',stale/'registry.json',stale/'generated','--check'],'stale-check',1)
 exe=stale/'wrong';execute(['gcc','-std=c17','-I'+str(stale),'-I'+str(public),private/'registry.c',stale/'test.c','-o',exe],'stale-compile');out,_=execute([exe],'stale-execute',1);assert 'LR_COMPANION_CHECK' in out
 for cc in ['gcc','clang']:
  for opt in ['O0','O3']:
   label=cc+'-'+opt;start=len(records);flags=[cc,'-std=c17','-'+opt,'-fno-inline','-Wall','-Wextra','-Werror','-pedantic-errors','-DLR_GENERIC_HEADER="'+str(private/'generic.h')+'"']
   for case in range(1,7):
    f=[*flags,'-DCASE='+str(case)];src=public/'generic_probes.c'
    if case<=4:
     out,err=execute([*f,'-fsyntax-only',src],label+'-generic-negative'+str(case),1);assert 'error:' in err;continue
    obj=home/(label+'-generic'+str(case)+'.o');exe=home/(label+'-generic'+str(case));execute([*f,'-c',src,'-o',obj],label+'-generic-object');execute([cc,obj,'-o',exe],label+'-generic-link');execute([exe],label+'-generic-run')
    if case==5:
     text,_=execute(['objdump','-dr',obj],label+'-generic-assembly');symbols,_=execute(['nm',obj],label+'-generic-symbols');assert 'lg_int' in symbols and 'lg_double' in symbols
     for function,target in [('generic_int','lg_int'),('explicit_int','lg_int'),('generic_double','lg_double'),('explicit_double','lg_double')]:
      block=text.split('<'+function+'>:',1)[1].split('\n\n',1)[0];assert target in block and not re.search(r'\s+j(?!mp\b)[a-z]+\s',block),block
   probes.append(dict(profile=label,executions=list(range(start,len(records))),negative_cases=[1,2,3,4],no_runtime_selection_branch=True,accepted_new_long=True))
   # Original x86-64 entry uses only Linux exit;component objects remain C17 freestanding.
   flags=[cc,'-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-Wall','-Wextra','-Werror','-pedantic-errors','-I'+str(public)];objs=[]
   for n,src in enumerate([private/'registry.c',private/'generic.c',public/'target.c']):
    obj=home/(label+'-strict'+str(n)+'.o');execute([*flags,'-c',src,'-o',obj],label+'-strict-object');objs.append(obj)
   exe=home/(label+'-strict');execute([cc,'-nostdlib','-static','-no-pie',*objs,public/'entry.S','-Wl,-e,_start','-o',exe],label+'-strict-link');undefined,_=execute(['nm','-u',exe],label+'-strict-undefined');assert not undefined.strip();dyn,_=execute(['readelf','-d',exe],label+'-strict-dynamic');assert 'NEEDED' not in dyn;header,_=execute(['readelf','-h',exe],label+'-strict-identity');assert 'ELF64' in header and 'X86-64' in header;execute([exe],label+'-strict-run');execute(['size',exe],label+'-strict-size');targets.append(dict(profile=label,executed=True,undefined=[],needed=[],component_library_calls=False,harness='Original x86-64 assembly/Linux exit only',iterations=32))
 fixture=read_json(ROOT/'fixtures/language/PR-T06-02.json');sorted_rows=sorted(fixture['input']['rows'],key=lambda r:r['id']);assert [r['id'] for r in sorted_rows]==fixture['expected']['ids'] and [r['name'] for r in sorted_rows]==fixture['expected']['names']
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,executions=records,compiler_dependencies=dependencies,generated_artifacts=artifacts,variants=variants,admission_rejections=invalid,stale_generated_rejected=True,probes=probes,strict_targets=targets,synthetic_fixture_checked=True,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Strict component closure is selected x86-64;Linux exit belongs only to the original observation harness.','No runtime-branch observation is bounded to the named functions/builds,not universal optimization behavior.']))
 print(json.dumps(dict(passed=True,commands=len(records),variants=len(variants),probe_profiles=len(probes),strict_profiles=len(targets))))
if __name__=='__main__':main()
