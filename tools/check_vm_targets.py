"""Validate exact environment, byte/cost, semantic and dependency evidence."""
from pathlib import Path
import argparse,copy,json
from run_vm_targets import ROOT,PROFILES,PROVIDERS,KERNEL,inputs,sha
REPORT='reviews/v5/expansion/legacy_vm_targets/current_validation.json'
def validate(d,files=True):
 errors=[]
 def need(x,m):
  if not x:errors.append(m)
 rows=d.get('checks',[]);by={r.get('label'):r for r in rows};need(d.get('passed') is True and all(r.get('passed') is True for r in rows),'failed campaign');need(len(rows)==152 and len(by)==152,'exact152 checks')
 required={'environment','adapter successor','source stability','exact fixture bytes','skip bss build','skip bss execute','implicit helper rejection','unrooted LTO link rejection','kernel frozen guard','public False','public True','missing provider refused','fixture producer build','fixture producer execute'}
 for _,_,tag in PROFILES:
  for a in [1,2]:required|={tag+' adapter'+str(a)+' '+x for x in ['build','execute','compare']}
 for opt in ['0','3']:
  for a in [1,2]:required|={'PPC64-O'+opt+' adapter'+str(a)+' '+x for x in ['build','ELF','closure','execute','compare']}
 for tag in ['arm-none-eabi-gcc-O0','arm-none-eabi-gcc-O3','clang-O0','clang-O3','arm-none-eabi-gcc-O3-LTO']:
  for a in [1,2]:required|={tag+' adapter'+str(a)+' '+x for x in ['build','execute','compare','resources','size','assembly']}
 for cc in ['gcc','clang']:
  for opt in ['O0','O2']:required|={'kernel '+cc+'-'+opt+' '+x for x in ['compare','bytes']}
 required|={'comparison control '+str(i) for i in range(5)};need(required<=set(by),'mandatory scope lost')
 need(len(d.get('external_tool_sha256',{}))==9 and bool(d.get('outputs_sha256')),'tool/output identities');need(len(d.get('limits',[]))>=5,'scope limits')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for row in rows:
  label=row['label']
  if 'command' not in row:
   need(label in {'source stability','exact fixture bytes'} or label.startswith('kernel ') and label.endswith(' bytes'),'nonexecuted claim')
   if label.endswith(' bytes') and label.startswith('kernel '):
    paths=row.get('artifacts',[]);need(len(paths)==2 and {Path(p).name for p in paths}=={'bytecode.cvm','archive-bytecode.cvm'},'kernel bytes omitted')
    if files:
     for p in paths:need((ROOT/p).read_bytes()==bytes.fromhex('43564d31010000080000000129000000160111'),'actual guest image mismatch')
   continue
  cmd=row['command'];expected=3 if label=='implicit helper rejection' else 2 if label=='missing provider refused' else 1 if label in ['skip bss execute','unrooted LTO link rejection','public True'] or label.startswith('comparison control ') else 0
  need(row.get('expected')==row.get('returncode')==expected,'exit '+label)
  marker={'skip bss execute':'VX_STARTUP_FAIL','implicit helper rejection':'VX_UNEXPLAINED_IMPORT __aeabi_uldivmod','unrooted LTO link rejection':'undefined reference to','public True':'VX_FAIL:2','missing provider refused':'--cvm','kernel frozen guard':'12 actual profiles PASS'}.get(label)
  if label.startswith('comparison control '):marker='AssertionError'
  if marker:need(row.get('marker')==marker,'control meaning '+label)
  if label.endswith(' compare'):need(row.get('marker')=='"passed": true','comparison not asserted')
  if label.endswith(' closure'):need(row.get('empty') is True,'uninspected closure')
  if label.endswith(' ELF'):need(row.get('marker')=='big endian','endian not observed')
  if label.endswith(' build') and any(label.startswith(tag+' ') for _,_,tag in PROFILES):need('-std=c17' in cmd and '-Werror' in cmd and str(ROOT/PROVIDERS['cvm']) in cmd,'native core/dialect')
  if label.startswith('arm-none-eabi-') or label.startswith('clang-O'):
   if label.endswith(' build'):
    for key in ['cvm','runtime','driver']:need('--'+key in cmd and cmd[cmd.index('--'+key)+1]==str(ROOT/PROVIDERS[key]),'target provider '+key)
    need('--poison-bss' in cmd,'startup vacuity');need(('--lto' in cmd)==('-LTO ' in label),'LTO scope')
  if files:
   raw=ROOT/row['raw'];need(sha(raw)==row.get('raw_sha256'),'raw identity');text=raw.read_text(errors='replace');need(not row.get('marker') or row['marker'] in text,'raw marker '+label);need(not row.get('empty') or not text.strip(),'nonempty closure');need(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer','VX_FORMATTER_UNSUPPORTED']),'unexpected diagnostic')
   if label.endswith(' resources'):
    dest=Path(cmd[-1]);audit=json.loads((dest/'audit.json').read_text());need(audit['passed'] and all(audit['checks'].values()) and 0<audit['conservative_bytes']<audit['reserve']==32768 and not audit['dynamic'] and not audit['undefined'],'generated resource audit');need(all((dest/(str(i)+'.o')).is_file() for i in range(6)),'missing objects')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','lost-compare','wrong-core','missing-input','missing-output','lost-limits','nonsemantic-helper','bss-not-failed','lost-lto','lost-poison','lost-endian','lost-kernel-bytes','lost-kernel-guard','ignored-closure','lost-comparison-control']:
   x=copy.deepcopy(d);pick=lambda name:next(r for r in x['checks'] if r['label']==name)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['checks']=[r for r in x['checks'] if not r['label'].startswith('clang-asan')]
   elif kind=='lost-compare':pick('gcc-debug adapter1 compare')['marker']=None
   elif kind=='wrong-core':pick('arm-none-eabi-gcc-O0 adapter1 build')['command'][pick('arm-none-eabi-gcc-O0 adapter1 build')['command'].index('--cvm')+1]='fake.c'
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='nonsemantic-helper':pick('implicit helper rejection')['marker']='compile failed'
   elif kind=='bss-not-failed':pick('skip bss execute').update(returncode=0,expected=0)
   elif kind=='lost-lto':pick('arm-none-eabi-gcc-O3-LTO adapter1 build')['command'].remove('--lto')
   elif kind=='lost-poison':pick('arm-none-eabi-gcc-O0 adapter1 build')['command'].remove('--poison-bss')
   elif kind=='lost-endian':pick('PPC64-O0 adapter1 ELF')['marker']='little endian'
   elif kind=='lost-kernel-bytes':pick('kernel gcc-O0 bytes')['artifacts']=[]
   elif kind=='lost-kernel-guard':x['checks']=[r for r in x['checks'] if r['label']!='kernel frozen guard']
   elif kind=='ignored-closure':pick('PPC64-O0 adapter1 closure')['empty']=False
   else:pick('comparison control 0')['marker']=None
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_vm_targets.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
