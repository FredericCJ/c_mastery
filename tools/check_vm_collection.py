"""Reject stale, partial, vacuous, mis-scoped or nonsemantic T46.07/08 evidence."""
from pathlib import Path
import argparse,copy,hashlib,json
from run_vm_collection import ROOT,LAB,REF,PROFILES,PROVIDERS,MUTANTS,MARKER,STUB,inputs
REPORT='reviews/v5/expansion/legacy_vm_collection/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(d,files=True):
 errors=[]
 def need(x,msg):
  if not x:errors.append(msg)
 need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);by={r.get('label'):r for r in rows};need(len(rows)==221 and len(by)==221,'exact221checks')
 need(all(r.get('passed') is True for r in rows),'failed check')
 required={'independent graph corpus','environment','source stability','public False','public True','missing provider refused'}
 for _,_,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:required|={tag+' '+str(v)+' compile',tag+' '+str(v)+' execute'}
  for label in ['oracle compile','independent oracle']:required.add(tag+' '+label)
 for opt in ['0','3']:
  for v in [0,*MUTANTS,'stub']:
   for label in ['build','ELF','closure','execute']:required.add('PPC64-O'+opt+' '+str(v)+' '+label)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for label in ['build','execute','closure','size','assembly','resources']:required.add(cc+'-O'+opt+' '+label)
 for v in [2,3,5,6,'stub']:required|={'target '+str(v)+' build','target '+str(v)+' execute'}
 need(required<=set(by),'mandatory scope omitted');need(len(d.get('limits',[]))>=5,'evidence limits absent')
 need({p.relative_to(ROOT).as_posix() for p in inputs()}==set(d.get('input_sha256',{})),'input closure changed')
 need(bool(d.get('outputs_sha256')) and len(d.get('external_tool_sha256',{}))==9,'outputs/tool identities absent')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for row in rows:
  if 'command' not in row:need(row.get('label')=='source stability','nonexecuted claim');continue
  label=row['label'];cmd=row['command'];need(row.get('returncode')==row.get('expected'),'exit mismatch '+label)
  negative=label in ['public True','missing provider refused'] or label.endswith(' execute') and any(label.endswith(' '+str(v)+' execute') for v in [*MUTANTS,'stub'])
  expected=2 if label=='missing provider refused' else int(negative)
  need(row.get('expected')==expected,'weakened expected exit '+label)
  if files:
   raw=ROOT/row['raw'];need(sha(raw)==row.get('raw_sha256'),'raw identity '+label);text=raw.read_text(errors='replace')
   need(not row.get('marker') or row['marker'] in text,'semantic marker '+label)
   need(not row.get('empty') or not text.strip(),'nonempty closure '+label)
   need(not any(v in text for v in ['runtime error:','ERROR: AddressSanitizer','VG_FORMATTER_UNSUPPORTED']),'unexpected diagnostic '+label)
   if label.endswith(' 0 execute') or label in [cc+'-O'+opt+' execute' for cc in ['arm-none-eabi-gcc','clang'] for opt in ['0','3']]:
    for prefix in ['VG_IDENTITY_PASS','VG_BARRIER_PASS','VG_ALLOCATION_PASS','VG_TRANSFER_PASS','VG_BASELINE_PASS']:
     need(prefix in text,'missing mechanism execution '+prefix)
  if label.endswith(' closure'):need(row.get('empty') is True,'closure not inspected '+label)
  if label.endswith(' independent oracle'):need(row.get('marker')=='VG_ORACLE_PASS cases=32768' and bool(row.get('stdin')),'independent oracle missing')
  if label.startswith('PPC64') and label.endswith(' ELF'):need(row.get('marker')=='big endian','wrong endian observation')
  if label.endswith(' compile') and any(label.startswith(tag+' ') for _,_,tag in PROFILES):
   need('-std=c17' in cmd and '-Werror' in cmd,'native dialect/diagnostics');need(str(ROOT/PROVIDERS['cvm']) in cmd or label.endswith('helper compile'),'actual CVM provider')
  if (label.startswith('arm-none-eabi-gcc-') or label.startswith('clang-O') or label.startswith('target ')) and label.endswith(' build'):
   for key,value in PROVIDERS.items():need('--'+key in cmd and cmd[cmd.index('--'+key)+1]==str(ROOT/value),'explicit target provider '+key)
   need(row.get('marker')=='VG_TARGET_BUILD_PASS explicit_objects=12 original_limb_division','target provider closure');need('--headers' in cmd,'explicit header profile')
  if negative and label.endswith(' execute'):
   variant=label.split()[-2];need(row.get('marker')==(STUB if variant=='stub' else MUTANTS.get(int(variant))),'semantic mutation reason')
  if files and label.endswith(' resources'):
   dest=Path(cmd[-1]);audit=json.loads((dest/'resource-audit.json').read_text());need(audit['passed'] and 0<audit['conservative_bytes']<audit['reserve']==32768 and not audit['undefined'] and not audit['dynamic'],'stack/closure scope')
   port=json.loads((dest/'registration_port.port.json').read_text());need(port['id']=='R-VG-ORVM-ENUM-2026-01' and port['source']==str(ROOT/PROVIDERS['registrations']) and port['source_sha256']==sha(ROOT/PROVIDERS['registrations']) and port['output_sha256']==sha(dest/'registration_port.c'),'explicit source successor')
   for i in range(12):need((dest/(str(i)+'.o')).is_file(),'missing generated object')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  kinds=['failed','missing-profile','failed-mutant','weakened-exit','missing-marker','missing-input','missing-output','wrong-dialect','wrong-provider','weakened-oracle','wrong-endian','ignored-closure','lost-limits','lost-stability','missing-port']
  for kind in kinds:
   x=copy.deepcopy(d);rows=x['checks'];select=lambda label:next(r for r in rows if r['label']==label)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['checks']=[r for r in rows if not r['label'].startswith('clang-asan')]
   elif kind=='failed-mutant':select('gcc-debug 1 compile').update(returncode=1,expected=1)
   elif kind=='weakened-exit':select('gcc-debug 1 execute').update(returncode=0,expected=0)
   elif kind=='missing-marker':select('gcc-debug 1 execute')['marker']=None
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='wrong-dialect':select('gcc-debug 0 compile')['command'].remove('-std=c17')
   elif kind=='wrong-provider':select('arm-none-eabi-gcc-O0 build')['command'][select('arm-none-eabi-gcc-O0 build')['command'].index('--cvm')+1]='fake.c'
   elif kind=='weakened-oracle':select('gcc-debug independent oracle')['marker']='VG_ORACLE_PASS cases=1'
   elif kind=='wrong-endian':select('PPC64-O0 0 ELF')['marker']='little endian'
   elif kind=='ignored-closure':select('arm-none-eabi-gcc-O0 closure')['empty']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='lost-stability':select('source stability')['passed']=False
   elif kind=='missing-port':select('arm-none-eabi-gcc-O0 build')['command'].remove('--headers')
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_vm_collection.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
