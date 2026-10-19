"""Actual feature builds, two Python versions, migration, shared faults and rebuilds."""
from datetime import datetime,timezone
from pathlib import Path
import copy,difflib,json,re,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha,read_json
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/schema_companion_validation.json'
def main():
 public=ROOT/'labs/language_portability/schema';private=ROOT/'assessor/language_portability/schema';inputs={p.relative_to(ROOT).as_posix():sha(p) for folder in [public,private] for p in folder.rglob('*') if p.is_file()}
 for p in ['tools/run_portability_schema_companion.py','tools/common.py','tools/verify_portability_schema.py','fixtures/language/PR-T06-03.json']:inputs[p]=sha(ROOT/p)
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('schema-companion-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];external={};dependencies=[];runtime=[];variants=[];matrices=[];strict=[]
 def execute(cmd,label,expected=0,cwd=ROOT):
  cmd=list(map(str,cmd));dep=None
  if cmd[0] in ['gcc','clang'] and '-c' in cmd and expected==0:dep=home/(str(len(records))+'-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
  p=subprocess.run(cmd,cwd=cwd,text=True,capture_output=True,timeout=90);out=p.stdout+p.stderr;log=home/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,cwd=str(cwd),exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,out
  if dep:
   paths=[Path(q).resolve() for q in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in paths]))
   for q in paths:
    if not q.is_relative_to(ROOT):external[str(q)]=sha(q)
  return p.stdout,p.stderr
 for tool in ['gcc','clang','nm','readelf','size','ninja','ldd']:
  path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
 external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve());native_version=execute([sys.executable,'--version'],'native-python')[0].strip()
 windows=ROOT/'.conda/python.exe';external[str(windows)]=sha(windows);windows_version=execute([windows,'--version'],'windows-python')[0].strip();assert native_version.split()[1].split('.')[:2]!=windows_version.split()[1].split('.')[:2]
 baseline=read_json(public/'schema.json')
 def generate(label,value):
  work=home/label;work.mkdir();dump(work/'schema.json',value);execute([sys.executable,private/'generate.py',work/'schema.json',work/'generated'],label+'-generate');execute([sys.executable,private/'generate.py',work/'schema.json',work/'generated','--check'],label+'-check');return work
 cases=[('ordinary',baseline)]
 fast=copy.deepcopy(baseline);fast['internal_fast']=True;cases.append(('fast',fast))
 for kind,enabled in [('none',[]),('raw',['raw']),('framed',['framed']),('datagram',['datagram'])]:
  v=copy.deepcopy(baseline);v['features']={n:n in enabled for n in v['features']};cases.append((kind,v))
 v2=copy.deepcopy(baseline);v2['version']=2;v2['fields'].append(dict(name='reserved',offset=10,width=6,reserved=True));cases.append(('reserved-v2',v2))
 semantic=copy.deepcopy(baseline);semantic['constraints']=[dict(kind='width_at_least',field='length',other='tag')];cases.append(('semantic',semantic))
 workmap={}
 for label,value in cases:
  work=generate(label,value);workmap[label]=work;manifest=read_json(work/'generated/manifest.json');expected_masks={r['name']:(2**r['width']-1)*2**r['offset'] for r in value['fields'] if not r.get('reserved')};m=expected_masks
  text='#include "schema.h"\n#include "generated/config.h"\n#include "generated/fields.h"\n#include <stdio.h>\nint main(void){LsFields f={5,17,2},g;uint16_t w; if(ls_pack(&f,&w)||w!=653||ls_unpack(w,&g)||g.tag!=5||g.length!=17||g.flags!=2||ls_require(LS_WIRE_ID,LS_ABI_ID))return 1;printf("%u %u %u %zu\\n",(unsigned)LS_TAG_MASK,(unsigned)LS_LENGTH_MASK,(unsigned)LS_FLAGS_MASK,ls_record_size());return 0;}\n';(work/'probe.c').write_text(text)
  for cc in ['gcc','clang']:
   objs=[]
   for i,src in enumerate([private/'schema.c',work/'probe.c']):
    obj=work/(cc+str(i)+'.o');execute([cc,'-std=c17','-O3','-Wall','-Wextra','-Werror','-pedantic-errors','-I'+str(work),'-I'+str(public),'-c',src,'-o',obj],label+'-'+cc+'-object');objs.append(obj)
   exe=work/cc;execute([cc,*objs,'-o',exe],label+'-'+cc+'-link');out,_=execute([exe],label+'-'+cc+'-run');assert list(map(int,out.split()))==[m['tag'],m['length'],m['flags'],4+4*value['features']['framed']+4*value['features']['crc']];runtime.append(exe)
  variants.append(dict(kind=label,manifest=manifest,expected_masks=m))
 ordinary=read_json(workmap['ordinary']/'generated/manifest.json');fm=read_json(workmap['fast']/'generated/manifest.json');vm=read_json(workmap['reserved-v2']/'generated/manifest.json')
 assert fm['wire_identity']==vm['wire_identity']==ordinary['wire_identity'] and fm['abi_identity']==vm['abi_identity']==ordinary['abi_identity'] and len({fm['build_identity'],vm['build_identity'],ordinary['build_identity']})==3
 assert (workmap['fast']/'generated/codec.h').read_bytes()!=(workmap['ordinary']/'generated/codec.h').read_bytes()
 # Independent feature truth table: rules-only input carries no stale artifact identity.
 probe=home/'config.c';probe.write_text('#include "generated/rules.h"\nint configuration_probe;\n')
 for cc in ['gcc','clang']:
  for size in [3,4]:
   admitted=[];rejected=[]
   for bits in range(2**size):
    raw,framed,crc,datagram=[(bits>>i)&1 if i<size else 0 for i in range(4)];valid=raw+framed+datagram<=1 and (not crc or framed);flags=[f'-DLS_{n}={v}' for n,v in zip(['RAW','FRAMED','CRC','DATAGRAM'],[raw,framed,crc,datagram])]
    out,err=execute([cc,'-std=c17','-pedantic-errors','-I'+str(workmap['ordinary']),*flags,'-fsyntax-only',probe],cc+'-matrix'+str(size)+'-'+str(bits),0 if valid else 1)
    if not valid:assert ('LS_REQUIRES_crc_framed' in err if crc and not framed else 'LS_EXCLUSIVE_TRANSPORT' in err)
    (admitted if valid else rejected).append(bits)
   assert admitted==([0,1,2,6] if size==3 else [0,1,2,6,8]);matrices.append(dict(compiler=cc,features=size,admitted=admitted,rejected=rejected))
 # Override cannot change layout while keeping the generated identity.
 probe.write_text('#include "generated/config.h"\nint configuration_probe;\n')
 for cc in ['gcc','clang']:
  _,err=execute([cc,'-std=c17','-I'+str(workmap['ordinary']),'-DLS_FRAMED=0','-DLS_CRC=0','-fsyntax-only',probe],cc+'-identity-override',1);assert 'LS_CONFIGURATION_IDENTITY_framed' in err
 # Old generated v1 consumer accepts v2's zero-reserved encoding before any foreign layout use.
 consumer=home/'old.c';consumer.write_text('#include "schema.h"\n#include "generated/config.h"\n#include "generated/codec.h"\nint main(void){LsFields f={5,17,2},g;uint16_t w,again;if(ls_require(LS_WIRE_ID,LS_ABI_ID)||ls_pack(&f,&w)||lsg_unpack(w,&g)||lsg_pack(&g,&again)||again!=653)return 1;return 0;}\n')
 incompatible=home/'incompatible.c';incompatible.write_text('#include "schema.h"\n#include "generated/config.h"\nint main(void){return ls_require(LS_WIRE_ID,LS_ABI_ID)==-1?0:1;}\n')
 for cc in ['gcc','clang']:
  for label,src,view in [('old',consumer,'ordinary'),('incompatible',incompatible,'raw')]:
   obj=home/(cc+label+'.o');exe=home/(cc+label);execute([cc,'-std=c17','-Wall','-Wextra','-Werror','-I'+str(workmap[view]),'-I'+str(public),'-c',src,'-o',obj],cc+label+'-object');execute([cc,obj,workmap['reserved-v2']/(cc+'0.o'),'-o',exe],cc+label+'-link');execute([exe],cc+label+'-run');runtime.append(exe)
 # Clean paths, row/key order, and actual Python3.12 versus3.14 produce identical bytes.
 reordered=copy.deepcopy(baseline);reordered['fields'].reverse();clean=generate('different-path',reordered);win=home/'windows-generated'
 def winpath(p):return str(p).replace(str(ROOT),'F:/Learning/Curricula/c_mastery')
 execute([windows,winpath(private/'generate.py'),winpath(clean/'schema.json'),winpath(win)],'different-python-generate',cwd=clean)
 hashes=lambda folder:{p.name:sha(p) for p in folder.iterdir() if p.is_file()}
 assert hashes(win)==hashes(clean/'generated')==hashes(workmap['ordinary']/'generated')
 # Full admission happens before publication; no compiler receives invalid shift extents.
 invalid=[]
 for kind in ['overlap','duplicate','zero-width','wide','negative','count','name','reserved-v1','missing','semantic','feature','unknown-key']:
  v=copy.deepcopy(baseline)
  if kind=='overlap':v['fields'][2]['offset']=7
  elif kind=='duplicate':v['fields'][2]['name']='tag'
  elif kind=='zero-width':v['fields'][0]['width']=0
  elif kind=='wide':v['fields'][0]['width']=17
  elif kind=='negative':v['fields'][0]['offset']=-1
  elif kind=='count':v['fields']=[]
  elif kind=='name':v['fields'][0]['name']='x;evil'
  elif kind=='reserved-v1':v['fields'].append(dict(name='reserved',offset=10,width=6,reserved=True))
  elif kind=='missing':v['fields'][0]['name']='other'
  elif kind=='semantic':v['fields'][1]['width']=2;v['constraints']=semantic['constraints']
  elif kind=='feature':v['features']['raw']=True
  else:v['hidden']=1
  path=home/(kind+'.json');dump(path,v);dest=home/(kind+'-refused');dest.mkdir();(dest/'sentinel').write_bytes(b'unchanged');out,_=execute([sys.executable,private/'generate.py',path,dest],'admission-'+kind,1);assert 'LS_SCHEMA_REJECT' in out and {p.name:p.read_bytes() for p in dest.iterdir()}=={'sentinel':b'unchanged'};invalid.append(kind)
 # Two wrong generated consumers can agree; independent positional arithmetic must reject.
 shared=home/'shared-wrong';execute(['gcc','-std=c17','-DLS_MUTANT=1','-I'+str(workmap['ordinary']),'-I'+str(public),private/'schema.c',public/'tests.c','-o',shared],'shared-compile');out,_=execute([shared,'oracle'],'shared-roundtrip');assert len(out.splitlines())==1024 and all(v[:3]==v[4:] for v in [list(map(int,s.split())) for s in out.splitlines()]);execute([sys.executable,ROOT/'tools/verify_portability_schema.py','--driver',shared],'shared-independent-reject',1);runtime.append(shared)
 bad=home/'mutated';shutil.copytree(workmap['ordinary']/'generated',bad/'generated');field=bad/'generated/fields.h';field.write_text(field.read_text().replace('LS_LENGTH_MASK UINT32_C(248)','LS_LENGTH_MASK UINT32_C(240)'));exe=bad/'wrong';execute(['gcc','-std=c17','-I'+str(bad),'-I'+str(public),private/'schema.c',public/'tests.c','-o',exe],'mask-compile');out,err=execute([exe,'wire'],'mask-reject',1);assert 'LP_CHECK' in err;runtime.append(exe)
 # Actual external-generator versus X-macro dependencies and changed-row diffs.
 rebuild=[]
 for kind in ['external','xmacro']:
  work=home/('rebuild-'+kind);work.mkdir();output=work/'probe';inc=['-I'+str(work),'-I'+str(public)]
  if kind=='external':
   dump(work/'schema.json',baseline);(work/'probe.c').write_text('#include "generated/fields.h"\n#include <stdio.h>\nint main(void){printf("%u %u %u\\n",(unsigned)LS_TAG_MASK,(unsigned)LS_LENGTH_MASK,(unsigned)LS_FLAGS_MASK);return 0;}\n');rule=f'rule gen\n command = {sys.executable} {private}/generate.py schema.json generated\n';edge=f'build generated/config.h generated/rules.h generated/fields.h generated/codec.h generated/manifest.json: gen schema.json {private}/generate.py\n';prereq=' | generated/fields.h'
  else:
   shutil.copyfile(public/'xmacro.c',work/'probe.c');shutil.copyfile(public/'fields.inc',work/'fields.inc');rule='';edge='';prereq=''
  ninja=rule+f'rule cc\n command = gcc -std=c17 -MMD -MF probe.d -I. -c probe.c -o probe.o\n depfile = probe.d\n deps = gcc\nrule link\n command = gcc probe.o -o probe\n'+edge+f'build probe.o: cc probe.c{prereq}\nbuild probe: link probe.o\ndefault probe\n';(work/'build.ninja').write_text(ninja)
  initial,_=execute(['ninja','-v','-C',work],kind+'-initial');execute([output],kind+'-baseline');nochange,_=execute(['ninja','-v','-C',work],kind+'-unchanged');assert 'no work to do' in nochange
  before=(work/'generated/fields.h' if kind=='external' else work/'fields.inc').read_text()
  if kind=='external':v=copy.deepcopy(baseline);v['fields'][2]['offset']=10;dump(work/'schema.json',v)
  else:(work/'fields.inc').write_text(before.replace('flags,8,2','flags,10,2'))
  changed,_=execute(['ninja','-v','-C',work],kind+'-changed');out,_=execute([output],kind+'-changed-run');assert out=='7 248 3072\n';after=(work/'generated/fields.h' if kind=='external' else work/'fields.inc').read_text();diff=''.join(difflib.unified_diff(before.splitlines(True),after.splitlines(True)));assert diff;rebuild.append(dict(kind=kind,initial_steps=len(re.findall(r'^\[\d+/\d+\]',initial,re.M)),unchanged_steps=0,changed_steps=len(re.findall(r'^\[\d+/\d+\]',changed,re.M)),diff=diff));runtime.append(output)
  if kind=='xmacro':
   (work/'fields.inc').write_text(before+'X(tag,10,2)\n');_,err=execute(['gcc','-std=c17','-fsyntax-only',work/'probe.c'],'xmacro-duplicate',1);assert 'XM_tag' in err
   (work/'fields.inc').write_text(before.replace('length,3,5','length,3,2'));_,err=execute(['gcc','-std=c17','-DXM_SEMANTIC','-fsyntax-only',work/'probe.c'],'xmacro-semantic',1);assert 'semantic length>=tag' in err
 # Strict no-library component plus original Linux-exit observation harness.
 for cc in ['gcc','clang']:
  for opt in ['O0','O3']:
   label=cc+'-'+opt;objs=[]
   for i,src in enumerate([private/'schema.c',public/'target.c']):
    obj=home/(label+'-'+str(i)+'.o');execute([cc,'-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-Wall','-Wextra','-Werror','-pedantic-errors','-I'+str(workmap['ordinary']),'-I'+str(public),'-c',src,'-o',obj],label+'-strict-object');objs.append(obj)
   exe=home/(label+'-strict');execute([cc,'-nostdlib','-static','-no-pie',*objs,public/'entry.S','-Wl,-e,_start','-o',exe],label+'-strict-link');u,_=execute(['nm','-u',exe],label+'-undefined');d,_=execute(['readelf','-d',exe],label+'-dynamic');h,_=execute(['readelf','-h',exe],label+'-elf');assert not u.strip() and 'NEEDED' not in d and 'ELF64' in h and 'X86-64' in h;execute([exe],label+'-strict-run');execute(['size',exe],label+'-strict-size');strict.append(dict(profile=label,executed=True,undefined=[],needed=[],component_library_calls=False))
 for i,exe in enumerate(runtime):
  out,_=execute(['ldd',exe],'runtime'+str(i))
  for value in re.findall(r'(/[^\s()]+)',out):
   path=Path(value).resolve()
   if path.is_file():external[str(path)]=sha(path)
 fixture=read_json(ROOT/'fixtures/language/PR-T06-03.json');assert [(2**r['width']-1)*2**r['offset'] for r in fixture['input']['fields']]==fixture['expected']['masks'] and ordinary['enabled']==fixture['expected']['configuration']
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());extstable=all(sha(Path(p))==h for p,h in external.items());assert stable and extstable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=extstable,executions=records,compiler_dependencies=dependencies,generated_artifacts=artifacts,variants=variants,feature_matrices=matrices,admission_rejections=invalid,rebuilds=rebuild,python_versions=[native_version,windows_version],reproducible_clean_paths=True,internal_policy_identity_preserved=True,old_consumer_executed=True,incompatible_consumer_rejected=True,override_rejected=True,shared_wrong_roundtrips=1024,shared_wrong_independent_rejected=True,mask_mutant_rejected=True,strict_targets=strict,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Whole-publication I/O failure is outside the exclusive writable directory model.','Hashes identify declared profiles,not authenticity or automatic structural compatibility.','No physical board or universal Python-version portability claim.']))
 print(json.dumps(dict(passed=True,commands=len(records),variants=len(variants),feature_matrices=len(matrices),dependencies=len(dependencies))))
if __name__=='__main__':main()
