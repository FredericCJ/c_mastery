"""Require the full source-bound lease campaign and reject weaker evidence."""
from pathlib import Path
import argparse,copy,hashlib,json
from run_interrupt_lease_depth import PROFILES,MUTANTS,COMPOSE
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_advanced_lease/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(d):
    errors=[]
    def need(x,m):
        if not x:errors.append(m)
    need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);need(len(rows)==269,'required scope');by={r['label']:r for r in rows};need(len(by)==len(rows),'duplicate label')
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
            cmd=check(tag+' '+str(v)+' compile').get('command',[]);need('-std=c17' in cmd and '-Werror' in cmd,'missing strict language profile')
            if 'asan' in tag:need('-fsanitize=address,undefined' in cmd,'missing sanitizer')
            check(tag+' '+str(v)+' execute',0 if v==0 else 1,'XD_PASS' if v==0 else ('xd_submit(&s, 32, &t) == XD_BUSY' if v=='stub' else MUTANTS[v]))
        check(tag+' probe compile');check(tag+' probe execute');check(tag+' independent oracle',marker='XD_ORACLE_PASS words=6561 transitions=26244')
        for name,defines,expected,marker in COMPOSE:
            cmd=check(tag+' composition '+name+' compile').get('command',[])
            need(all(x in cmd for x in defines),'wrong selected mutation')
            for suffix in ['assessor/peripheral_depth/dma/dma.c','assessor/firmware_lifecycle/targets/h745_cache.c','assessor/interrupt_depth/lease/bridge.c']:need(any(x.endswith(suffix) for x in cmd),'missing actual producer '+suffix)
            check(tag+' composition '+name+' execute',expected,marker)
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            label=cc+'-O'+opt;cmd=check(label+' build',marker='XD_TARGET_BUILD_PASS objects=9').get('command',[])
            for key,value in [('--cc',cc),('--opt',opt)]:need(key in cmd and cmd[cmd.index(key)+1]==value,'wrong target profile')
            need(any(x.endswith('assessor/execution/support.c') for x in cmd),'original runtime missing')
            need(check(label+' closure').get('empty') is True,'closure weakened');check(label+' assembly');check(label+' size')
            cmd=check(label+' execute',marker='XD_TARGET_PASS blocks=31 operations=128 virtual_timer=25MHz').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'virtual clock weakened')
            check(label+' object and interval audit',marker='"passed": true')
            for board in ['F439','H745']:
                name=label+' '+board;cmd=check(name+' build',marker='XD_BOARD_BUILD_PASS '+board).get('command',[]);need('--board' in cmd and cmd[cmd.index('--board')+1]==board,'board mismatch')
                need(check(name+' closure').get('empty') is True,'board closure weakened');check(name+' assembly');check(name+' size')
    check('visibility counterexamples',marker='"passed": true')
    check('wrong cache register object rejection',1,'AssertionError:')
    check('physical capture structural controls',marker='"passed": true');check('physical template unobserved rejection',1,'physical capture not observed')
    check('public native reference',marker='XD_PASS');check('public native stub',1,'XD_CHECK:');check('public target stub build',marker='XD_TARGET_BUILD_PASS objects=9');check('public target stub execute',1,'XD_TARGET_UNIMPLEMENTED')
    need(by.get('sources stable',{}).get('passed') is True,'unstable sources');need(bool(d.get('limits')),'limits absent');return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
    if a.self_test and not errors:
        for kind in ['stale-input','stale-output','missing-core','missing-mutant','missing-oracle','missing-cache','missing-reset','missing-peer','missing-closure','missing-physical-refusal','missing-address-rejection','wrong-producer','wrong-marker','compile-failure','lost-sanitizer','lost-clock','lost-runtime','lost-closure','duplicate','lost-limits']:
            x=copy.deepcopy(d)
            def row(label):return next(r for r in x['checks'] if r['label']==label)
            if kind.startswith('stale-'):
                key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
            elif kind.startswith('missing-'):
                label={'missing-core':'gcc-debug 0 execute','missing-mutant':'gcc-debug 3 execute','missing-oracle':'clang-release independent oracle','missing-cache':'gcc-debug composition stale-cache execute','missing-reset':'clang-debug composition fabricated-fence execute','missing-peer':'gcc-release composition peer-generation execute','missing-closure':'clang-O3 H745 closure','missing-physical-refusal':'physical template unobserved rejection','missing-address-rejection':'wrong cache register object rejection'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
            elif kind=='wrong-producer':r=row('gcc-debug composition reference compile');r['command']=[p for p in r['command'] if not p.endswith('assessor/peripheral_depth/dma/dma.c')]
            elif kind=='wrong-marker':row('gcc-debug composition early-free execute')['marker']='XD_COMPOSE_FAIL'
            elif kind=='compile-failure':row('gcc-debug 1 compile')['expected']=1
            elif kind=='lost-sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
            elif kind=='lost-clock':row('clang-O3 execute')['command'].remove('shift=0,align=off,sleep=off')
            elif kind=='lost-runtime':r=row('clang-O3 build');r['command']=[p for p in r['command'] if not p.endswith('assessor/execution/support.c')]
            elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
            elif kind=='duplicate':x['checks'][0]=x['checks'][1]
            else:x['limits']=[]
            rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
            if not rejected:errors.append('accepted '+kind)
    out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_interrupt_lease_depth.py','tools/run_interrupt_lease_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
