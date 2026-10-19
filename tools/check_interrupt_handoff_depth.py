"""Reject missing/weak/stale handoff evidence, including the negative categories."""
from pathlib import Path
import argparse,copy,hashlib,json
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_advanced_handoff/current_validation.json'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def validate(d):
 errors=[]
 def need(x,msg):
  if not x:errors.append(msg)
 need(d.get('passed') is True,'campaign failure');rows=d.get('checks',[]);need(len(rows)==286,'campaign scope');checks={r['label']:r for r in rows};need(len(checks)==len(rows),'duplicate label')
 for key in ['input_sha256','outputs_sha256','external_input_sha256']:
  need(bool(d.get(key)),key+' absent')
  for p,h in d.get(key,{}).items():f=Path(p) if key=='external_input_sha256' else ROOT/p;need(f.is_file() and sha(f)==h,'stale '+p)
 for r in rows:
  need(r.get('passed') is True,'failed '+r['label'])
  if 'raw' in r:
   f=ROOT/r['raw'];need(f.is_file() and sha(f)==r['raw_sha256'],'raw identity');text=f.read_text(errors='replace') if f.is_file() else ''
   need(r['returncode']==r['expected'],'wrong exit');need(not r.get('marker') or r['marker'] in text,'unobserved marker');need(not r.get('empty') or not text.strip(),'nonempty closure')
 def check(label,expected=0,marker=None):
  r=checks.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'weakened expected '+label)
  if marker is not None:need(r.get('marker')==marker,'weakened marker '+label)
  return r
 mutants={1:'xi_take(&q, &out) == XI_EMPTY',2:'xi_push(&q, &r) == XI_FULL',3:'service.dropped == UINT32_MAX',4:'xi_reserve(&q) == XI_BUSY',5:'mask.value == 1',6:'mask.value == 0x40'}
 for tag in ['gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan']:
  for v in [0,1,2,3,4,5,6,'stub']:
   cmd=check(tag+' '+str(v)+' compile').get('command',[]);need('-std=c17' in cmd and '-Werror' in cmd,'C17 diagnostic profile')
   if 'asan' in tag:need('-fsanitize=address,undefined' in cmd,'sanitizer missing')
   check(tag+' '+str(v)+' execute',0 if v==0 else 1,'XI_PASS' if v==0 else ('xi_init(&q, 0)' if v=='stub' else mutants[v]))
  check(tag+' independent oracle',marker='XI_ORACLE_PASS queue=49152 mask=4096')
  for case,marker in [('threads','XI_THREADS_PASS owners=2 records=4000'),('locks','XI_LOCK_PASS observed=5000')]:check(tag+' '+case+' compile');check(tag+' '+case+' execute',marker=marker)
  cmd=check(tag+' full capacity compile').get('command',[]);need(any(x.endswith('assessor/execution/concurrent.c') for x in cmd),'actual CMX provider');check(tag+' full capacity execute',marker='XI_CMX_PASS full_capacity=1,2,4,8,16 records=1984')
  for mutant in [0,6]:
   cmd=check(tag+' DMA '+str(mutant)+' compile').get('command',[]);need(any(x.endswith('assessor/peripheral_depth/dma/dma.c') for x in cmd),'actual Pd provider');need('-DPD_MUTANT='+str(mutant) in cmd,'selected DMA mutation');check(tag+' DMA '+str(mutant)+' execute',int(bool(mutant)),'out.sequence == 42' if mutant else 'XI_DMA_PASS')
  check(tag+' wide host compile');check(tag+' wide host execute',marker='XI_WIDE_HOST_PASS records=3000 index_bits=64 capacity=3')
 for cc in ['gcc','clang']:
  for case,marker in [('threads','XI_THREADS_PASS owners=2 records=4000'),('locks','XI_LOCK_PASS observed=5000')]:
   cmd=check(cc+' tsan '+case+' compile').get('command',[]);need('-fsanitize=thread' in cmd,'TSan absent');check(cc+' tsan '+case+' execute',marker=marker)
 for m in [0,1,7,8]:
  cmd=check('GenMC '+str(m),42 if m else 0,'Error: Non-atomic race!' if m else 'Number of complete executions explored: 20').get('command',[])
  need(all(x in cmd for x in ['-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','-O0','-DXI_MUTANT='+str(m)]),'actual bounded GenMC configuration')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for instrument in [0,1]:
    label=cc+'-O'+opt+'-i'+str(instrument)+'-v0';cmd=check(label+' build',marker='XI_TARGET_BUILD_PASS').get('command',[])
    for key,value in [('--cc',cc),('--opt',opt),('--instrument',str(instrument))]:need(key in cmd and cmd[cmd.index(key)+1]==value,'wrong target configuration')
    for suffix in ['assessor/execution/support.c','assessor/interrupt_depth/frame/frame.c','assessor/interrupt_depth/handoff/wide.c']:need(any(x.endswith(suffix) for x in cmd),'missing actual '+suffix)
    r=check(label+' closure');need(r.get('empty') is True,'closure requirement lost');check(label+' assembly');check(label+' size');cmd=check(label+' execute',marker='XI_TARGET_PASS').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'virtual clock profile missing');check(label+' interval oracle',marker='"passed": true');check(label+' object audit',marker='"passed": true')
  check(cc+' widened compile');check(cc+' widened imports',marker='__atomic_load_8');check(cc+' widened store imports',marker='__atomic_store_8');check(cc+' widened link rejection',1,'undefined reference to `__atomic_load_8')
 for m,marker in [(1,'primask() == 1'),(2,'basepri() == stricter')]:check('arm-none-eabi-gcc-O0-i1-v'+str(m)+' build',marker='XI_TARGET_BUILD_PASS');check('arm-none-eabi-gcc-O0-i1-v'+str(m)+' execute',1,marker)
 for label in ['instrumentation comparison','interference model']:check(label,marker='"passed": true')
 check('public native reference',marker='XI_PASS');check('public native stub',1,'XI_CHECK:');check('public target stub build',marker='XI_TARGET_BUILD_PASS');check('public target stub execute',1,'XI_TARGET_UNIMPLEMENTED');need(checks.get('sources stable',{}).get('passed') is True,'sources changed');need(bool(d.get('limits')),'evidence limits absent')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['stale-source','stale-output','stale-tool','missing-positive','missing-mutant','missing-oracle','missing-TSan','missing-GenMC','missing-CMX','missing-DMA','missing-mask','missing-brackets','missing-comparison','missing-import','lost-closure','lost-clock','weakened-marker','compile-failure-as-kill','lost-runtime','lost-sanitizer','duplicate','lost-limits']:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind.startswith('stale-'):
    key={'stale-source':'input_sha256','stale-output':'outputs_sha256','stale-tool':'external_input_sha256'}[kind];x[key][next(iter(x[key]))]='0'*64
   elif kind.startswith('missing-'):
    label={'missing-positive':'gcc-debug 0 execute','missing-mutant':'gcc-debug 3 execute','missing-oracle':'gcc-debug independent oracle','missing-TSan':'clang tsan threads execute','missing-GenMC':'GenMC 8','missing-CMX':'gcc-debug full capacity execute','missing-DMA':'gcc-debug DMA 6 execute','missing-mask':'arm-none-eabi-gcc-O0-i1-v2 execute','missing-brackets':'clang-O3-i1-v0 interval oracle','missing-comparison':'instrumentation comparison','missing-import':'clang widened store imports'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='lost-closure':row('clang-O3-i1-v0 closure')['empty']=False
   elif kind=='lost-clock':row('clang-O3-i1-v0 execute')['command'].remove('shift=0,align=off,sleep=off')
   elif kind=='weakened-marker':row('GenMC 7')['marker']='Compilation complete'
   elif kind=='compile-failure-as-kill':row('gcc-debug 3 compile')['expected']=1
   elif kind=='lost-runtime':r=row('clang-O3-i1-v0 build');r['command']=[s for s in r['command'] if not s.endswith('assessor/execution/support.c')]
   elif kind=='lost-sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='duplicate':x['checks'][0]=x['checks'][1]
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_interrupt_handoff_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
