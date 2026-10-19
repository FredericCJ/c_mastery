"""Reject stale, vacuous or weakened T29 frame/priority evidence."""
from pathlib import Path
import argparse,copy,hashlib,json
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_advanced_frame/current_validation.json'
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(d.get('passed') is True,'campaign failed');need(len(d.get('checks',[]))==208,'exact campaign count')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),key+' absent')
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(p)==h,'stale '+p)
 rows=d.get('checks',[]);checks={r['label']:r for r in rows};need(len(checks)==len(rows),'duplicate labels')
 for r in rows:
  need(r.get('passed') is True,'failed '+r['label'])
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(r['raw'])==r['raw_sha256'],'raw identity')
   text=p.read_text(errors='replace') if p.is_file() else ''
   need(r['returncode']==r['expected'],'wrong exit')
   need(not r.get('marker') or r['marker'] in text,'unobserved marker')
   need(not r.get('empty') or not text.strip(),'nonempty closure')
 def check(label,expected=0,marker=None):
  r=checks.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'weakened expected '+label)
  if marker is not None:need(r.get('marker')==marker,'weakened marker '+label)
  return r
 profiles=['gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan']
 mutants={1:'valid ? rc == XF_OK : rc == XF_UNSUPPORTED',2:'xf_frame(words, 8, 0x20000000u',3:'!f.fp_materialized',4:'xf_pick(cfg, irq, 3, 0x60',5:'xf_pick(cfg, irq, 3, 0x60',6:'xf_source(&irq[0], 0)'}
 for tag in profiles:
  for v in [0,1,2,3,4,5,6,'stub']:
   r=check(tag+' '+str(v)+' compile');cmd=r.get('command',[])
   need('-std=c17' in cmd and '-Werror' in cmd,'baseline flags')
   if 'asan' in tag:need('-fsanitize=address,undefined' in cmd,'sanitizer absent')
   check(tag+' '+str(v)+' execute',0 if v==0 else 1,'XF_PASS priority_cells=12288' if v==0 else ('xf_frame(words, 8' if v=='stub' else mutants[v]))
  check(tag+' independent oracle',marker='XF_ORACLE_PASS priority=12288 selection=41472')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for fp in [0,1]:
    label=cc+'-O'+opt+'-fp'+str(fp)+'-v0';r=check(label+' build',marker='XF_TARGET_BUILD_PASS');cmd=r.get('command',[])
    need('--cc' in cmd and cc in cmd and '--opt' in cmd and opt in cmd,'target toolchain cell')
    need(('--fp-callee' in cmd)==bool(fp),'FP callee transfer absent')
    need(any(s.endswith('assessor/execution/support.c') for s in cmd),'original runtime absent')
    check(label+' closure');need(checks.get(label+' closure',{}).get('empty') is True,'closure not audited')
    for suffix in ['symbols','assembly','size']:check(label+' '+suffix)
    r=check(label+' execute',marker='XF_TARGET_PASS');need('-d' in r.get('command',[]) and 'int' in r.get('command',[]),'target exception trace absent')
    check(label+' architectural oracle',marker='XF_TARGET_ORACLE_PASS cases=16 tailchains=24');check(label+' stack ledger',marker='"passed": true')
 for v,marker in [(1,'a->frame.pc >= (uint32_t)(uintptr_t)xf_interrupted_begin'),(2,'a->frame.alignment_word == pad'),(3,'r->frame.fp_materialized ==')]:
  check('arm-none-eabi-gcc-O0-fp0-v'+str(v)+' build',marker='XF_TARGET_BUILD_PASS');check('arm-none-eabi-gcc-O0-fp0-v'+str(v)+' execute',1,marker)
 check('late arrival model',marker='"passed": true');check('public native reference',marker='XF_PASS');check('public native stub',1,'XF_CHECK:');check('public target stub build',marker='XF_TARGET_BUILD_PASS');check('public target stub execute',1,'XF_TARGET_UNIMPLEMENTED');need(checks.get('sources stable',{}).get('passed') is True,'unstable sources');need(bool(d.get('limits')),'limits absent')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['stale-source','stale-output','missing-positive','missing-mutant','missing-oracle','missing-late','missing-stack','missing-fp','missing-clang','weakened-marker','compile-failure-as-kill','dropped-sanitizer','dropped-runtime','dropped-trace','nonempty-closure','lost-public-stub','duplicate','lost-limits']:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind.startswith('stale-'):key='input_sha256' if kind=='stale-source' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind.startswith('missing-') or kind=='lost-public-stub':
    label={'missing-positive':'gcc-debug 0 execute','missing-mutant':'gcc-debug 3 execute','missing-oracle':'gcc-debug independent oracle','missing-late':'late arrival model','missing-stack':'clang-O3-fp1-v0 stack ledger','missing-fp':'arm-none-eabi-gcc-O0-fp1-v0 execute','missing-clang':'clang-O3-fp0-v0 execute','lost-public-stub':'public target stub execute'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='weakened-marker':row('gcc-debug 1 execute')['marker']='XF_CHECK:'
   elif kind=='compile-failure-as-kill':row('gcc-debug 1 compile')['expected']=1
   elif kind=='dropped-sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='dropped-runtime':r=row('clang-O3-fp1-v0 build');r['command']=[s for s in r['command'] if not s.endswith('assessor/execution/support.c')]
   elif kind=='dropped-trace':row('clang-O3-fp1-v0 execute')['command'].remove('int')
   elif kind=='nonempty-closure':row('clang-O3-fp1-v0 closure')['empty']=False
   elif kind=='duplicate':x['checks'][0]=x['checks'][1]
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(p) for p in [REPORT,'tools/check_interrupt_frame_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
