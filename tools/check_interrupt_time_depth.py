"""Reject missing, stale, vacuous or weakened timestamp evidence."""
from pathlib import Path
import argparse,copy,hashlib,json
from run_interrupt_time_depth import PROFILES,MUTANTS,COMPOSE,PROVIDERS,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_advanced_time/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(d):
    errors=[]
    def need(x,m):
        if not x:errors.append(m)
    need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);need(len(rows)==229,'required scope');by={r['label']:r for r in rows};need(len(by)==len(rows),'duplicate label')
    need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
    for key in ['input_sha256','outputs_sha256']:
        need(bool(d.get(key)),key+' absent')
        for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
    for r in rows:
        need(r.get('passed') is True,'failed '+r['label'])
        if 'raw' in r:
            f=ROOT/r['raw'];need(f.is_file() and sha(f)==r['raw_sha256'],'raw identity');text=f.read_text(errors='replace') if f.is_file() else ''
            need(r.get('expected')==r.get('returncode'),'wrong return');need(not r.get('marker') or r['marker'] in text,'missing result');need(not r.get('empty') or not text.strip(),'nonempty closure')
    def check(label,expected=0,marker=None):
        r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'wrong expectation '+label)
        if marker is not None:need(r.get('marker')==marker,'wrong marker '+label)
        return r
    for cc,opt,tag in PROFILES:
        for v in [0,1,2,3,4,5,'stub']:
            cmd=check(tag+' '+str(v)+' compile').get('command',[]);need('-std=c17' in cmd and '-Werror' in cmd and opt in cmd,'strict profile')
            check(tag+' '+str(v)+' execute',0 if v==0 else 1,'XK_PASS' if v==0 else ('XK_CHECK xk_snapshot' if v=='stub' else MUTANTS[v]))
        check(tag+' probe compile');check(tag+' probe execute');r=check(tag+' independent oracle',marker='XK_ORACLE_PASS')
        if r.get('raw'):need('snapshot=2100 calibration=4096 sequence=400 order=1296 event=360' in (ROOT/r['raw']).read_text(),'oracle scope')
        for name,defines,expected,marker in COMPOSE:
            cmd=check(tag+' composition '+name+' compile').get('command',[]);need(all(x in cmd for x in defines),'selected mutation')
            for k in ['handoff','frame','lease','checkpoint','correlation']:need(any(x.endswith(PROVIDERS[k]) for x in cmd),'actual producer '+k)
            check(tag+' composition '+name+' execute',expected,marker)
        check(tag+' threads compile');check(tag+' threads execute',marker='XK_THREADS_PASS writes=5000')
    for cc in ['gcc','clang']:
        need('-fsanitize=thread' in check(cc+' tsan compile').get('command',[]),'TSan required');check(cc+' tsan execute',marker='XK_THREADS_PASS writes=5000')
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            label=cc+'-O'+opt;cmd=check(label+' build',marker='XK_TARGET_BUILD_PASS explicit_objects=13 original_division_helper').get('command',[])
            for k,v in [('--cc',cc),('--opt',opt)]:need(k in cmd and cmd[cmd.index(k)+1]==v,'target profile')
            for k,v in PROVIDERS.items():need('--'+k in cmd and any(x.endswith(v) for x in cmd),'target actual provider '+k)
            need(check(label+' closure').get('empty') is True,'target closure');check(label+' symbols',marker='__aeabi_uldivmod');check(label+' assembly');check(label+' size')
            cmd=check(label+' execute',marker='XK_TARGET_PASS actual_irq=7 queued=4 dropped=3 odd_snapshot_refused=1').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'virtual clock')
            check(label+' interval oracle',marker='"passed": true');check(label+' missing helper rejected',1,'__aeabi_uldivmod')
    for v,marker in [('clock','!cpf_clock_read(&clock, 2, &sample)'),('queue','xi_reserve(&q) == XI_FULL'),('interval','touching == XK_AMBIGUOUS'),('stub','XK_TARGET_UNIMPLEMENTED')]:
        check('target '+v+' build',marker='XK_TARGET_BUILD_PASS explicit_objects=13 original_division_helper');check('target '+v+' execute',1,marker)
    check('public native reference',marker='XK_PASS');check('public native stub',1,'XK_CHECK');check('physical capture structural controls',marker='"physical_claim": false');check('physical template unobserved rejection',1,'physical capture not observed')
    need(by.get('sources stable',{}).get('passed') is True,'unstable sources');need(len(d.get('limits',[]))>=7,'limits absent');return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
    if a.self_test and not errors:
        for kind in ['stale-input','stale-output','missing-core','missing-mutant','missing-oracle','missing-clock','missing-queue','missing-provenance','missing-TSan','missing-helper-rejection','missing-touch','missing-physical-refusal','wrong-producer','wrong-marker','compile-failure','lost-sanitizer','lost-virtual-clock','lost-runtime','lost-closure','duplicate','lost-limits','missing-provider-include']:
            x=copy.deepcopy(d)
            def row(label):return next(r for r in x['checks'] if r['label']==label)
            if kind.startswith('stale-'):
                key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
            elif kind=='missing-provider-include':x['input_sha256'].pop('assessor/interrupt_depth/handoff/queue.c')
            elif kind.startswith('missing-'):
                label={'missing-core':'gcc-debug 0 execute','missing-mutant':'gcc-debug 3 execute','missing-oracle':'clang-release independent oracle','missing-clock':'gcc-debug composition clock execute','missing-queue':'gcc-debug composition queue execute','missing-provenance':'gcc-debug composition provenance execute','missing-TSan':'clang tsan execute','missing-helper-rejection':'clang-O3 missing helper rejected','missing-touch':'target interval execute','missing-physical-refusal':'physical template unobserved rejection'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
            elif kind=='wrong-producer':r=row('gcc-debug composition reference compile');r['command']=[p for p in r['command'] if not p.endswith(PROVIDERS['checkpoint'])]
            elif kind=='wrong-marker':row('gcc-debug composition queue execute')['marker']='XK_INTEGRATE_FAIL'
            elif kind=='compile-failure':row('gcc-debug 1 compile')['expected']=1
            elif kind=='lost-sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
            elif kind=='lost-virtual-clock':row('clang-O3 execute')['command'].remove('shift=0,align=off,sleep=off')
            elif kind=='lost-runtime':r=row('clang-O3 build');r['command']=[p for p in r['command'] if not p.endswith(PROVIDERS['divide'])]
            elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
            elif kind=='duplicate':x['checks'][0]=x['checks'][1]
            else:x['limits']=[]
            rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
            if not rejected:errors.append('accepted '+kind)
    out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_interrupt_time_depth.py','tools/run_interrupt_time_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
