"""Reject stale, missing or semantically weakened network-scope evidence."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
from run_network_scope_depth import PROFILES,MUTANTS,COMPOSE,PROVIDERS,TARGET_NEGATIVE,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_network_scope/current_validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def validate(d):
    errors=[]
    def need(x,m):
        if not x:errors.append(m)
    need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);need(len(rows)==269,'required269 checks');by={r['label']:r for r in rows};need(len(by)==len(rows),'duplicate labels')
    need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure')
    for key in ['input_sha256','outputs_sha256']:
        need(bool(d.get(key)),key+' absent')
        for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
    need(len(d.get('external_tool_sha256',{}))==7,'exact seven tool binaries')
    for p,h in d.get('external_tool_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'external tool '+p)
    logs={}
    for r in rows:
        need(r.get('passed') is True,'failed '+r['label'])
        if 'raw' in r:
            p=ROOT/r['raw'];need(p.is_file() and sha(p)==r['raw_sha256'],'raw identity');t=data(p).decode('utf8',errors='replace') if p.is_file() else '';logs[r['label']]=t
            need(r.get('returncode')==r.get('expected'),'exit status');need(not r.get('marker') or r['marker'] in t,'result marker');need(not r.get('empty') or not t.strip(),'runtime closure')
    def check(label,expected=0,marker=None):
        r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'expectation '+label)
        if marker is not None:need(r.get('marker')==marker,'marker '+label)
        return r
    for cc,opt,tag in PROFILES:
        for v in [0,1,2,3,4,5,6,'stub']:
            cmd=check(tag+' '+str(v)+' compile').get('command',[]);need('-std=c17' in cmd and '-Werror' in cmd and opt in cmd,'strict profile');need('-DNW_MUTANT='+str(0 if v=='stub' else v) in cmd,'compiled mutant')
            check(tag+' '+str(v)+' execute',0 if v==0 else 1,'NW_PASS' if v==0 else ('NW_CHECK nw_lengths' if v=='stub' else MUTANTS[v]))
        check(tag+' ledger compile');check(tag+' ledger execute');check(tag+' ledger oracle',marker='NW_LEDGER_ORACLE_PASS cases=180')
        for name,defines,expected,marker in COMPOSE:
            cmd=check(tag+' composition '+name+' compile').get('command',[]);need(all(x in cmd for x in defines),'selected composition mutation')
            for v in ['http','net']:need(any(x.endswith(PROVIDERS[v]) for x in cmd),'actual provider '+v)
            check(tag+' composition '+name+' execute',expected,marker)
        for v in [0,1,'stub']:
            cmd=check(tag+' pair '+str(v)+' compile').get('command',[]);need('-DNW_PAIR_MUTANT='+str(0 if v=='stub' else v) in cmd,'pair mutant')
            for k in ['http','net']:need(any(x.endswith(PROVIDERS[k]) for x in cmd),'actual pair provider '+k)
            check(tag+' pair '+str(v)+' execute',0 if v==0 else 1,'NW_PAIR_PASS actual_handshakes=2' if v==0 else ('NW_PAIR nw_pair_init' if v=='stub' else 'pair.flows[i].state == NET_SYN_RECEIVED'))
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            tag=cc+'-O'+opt;cmd=check(tag+' build',marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime').get('command',[])
            for k,v in [('--cc',cc),('--opt',opt)]:need(k in cmd and cmd[cmd.index(k)+1]==v,'target profile')
            for k,v in PROVIDERS.items():need('--'+k in cmd and any(x.endswith(v) for x in cmd),'target provider '+k)
            for k in ['pair','pair-test','strings']:need('--'+k in cmd,'actual target composition '+k)
            need(check(tag+' closure').get('empty') is True,'empty closure');check(tag+' assembly');check(tag+' size')
            for suffix in [' resources',' f439 resources']:
                check(tag+suffix,marker='"passed": true');r=json.loads(logs.get(tag+suffix,'{}'));need(r.get('passed') is True and r.get('physical_observation') is False and 1024<r.get('conservative_stack_bytes',0)<=32768 and r.get('stack_reserve')==32768 and bool(r.get('frames')),'actual resource bound '+tag)
            check(tag+' f439 link');need(check(tag+' f439 closure').get('empty') is True,'F439 closure');check(tag+' missing strings rejected',1,'strlen')
            cmd=check(tag+' execute',marker='NW_TARGET_PASS dma_reserved=12384 connections=2 segments_each=3 physical_dma=unobserved').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'execution model')
            text=logs.get(tag+' execute','');need('NW_PAIR_PASS actual_handshakes=2' in text and 'target_three_segment_budget_cases=18' in text and 'NW_LAYOUT ' in text,'actual target scope')
            build=Path(by.get(tag+' build',{}).get('command',[''])[-1]) # derive from --build, never infer a replacement backend
            args=by.get(tag+' build',{}).get('command',[])
            if '--build' in args:
                build=Path(args[args.index('--build')+1]);p=build/'external-inputs.json';need(p.is_file(),'target compiler/header identity')
                if p.is_file():
                    dependencies=json.loads(data(p));need(any(n.endswith('/stdint.h') for n in dependencies),'actual standard declarations')
                    for n,h in dependencies.items():q=Path(n);need(q.is_file() and sha(q)==h,'target dependency '+n)
    check('CCM bad placement links');check('CCM resource rejection',1,'DMA reachability/size')
    for variant,marker in TARGET_NEGATIVE.items():check('target '+variant+' build',marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime');check('target '+variant+' execute',1,marker)
    check('target stack build',marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime');check('target stack rejection',1,'stack reserve exceeded')
    check('public native reference',marker='NW_PASS');check('public native stub',1,'NW_CHECK');need(by.get('sources stable',{}).get('passed') is True,'source stability');need(len(d.get('limits',[]))>=7,'evidence limits');return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
    if a.self_test and not errors:
        kinds=['stale-input','stale-output','lost-core','lost-mutant','lost-oracle','lost-parser','lost-pair','lost-target-pair','lost-closure','lost-CCM','lost-stack','lost-runtime-rejection','lost-sanitizer','compile-failure','wrong-marker','wrong-provider','wrong-target','missing-header-identity','duplicate','erased-limits']
        for kind in kinds:
            x=copy.deepcopy(d)
            def row(label):return next(r for r in x['checks'] if r['label']==label)
            if kind.startswith('stale-'):
                key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
            elif kind in ['lost-core','lost-mutant','lost-oracle','lost-parser','lost-pair','lost-CCM','lost-stack','lost-runtime-rejection']:
                label={'lost-core':'gcc-debug 0 execute','lost-mutant':'gcc-debug 3 execute','lost-oracle':'clang-release ledger oracle','lost-parser':'gcc-debug composition stream execute','lost-pair':'gcc-debug pair 1 execute','lost-CCM':'CCM resource rejection','lost-stack':'target stack rejection','lost-runtime-rejection':'clang-O3 missing strings rejected'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
            elif kind=='lost-target-pair':row('clang-O3 build')['command'].remove('--pair')
            elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
            elif kind=='lost-sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
            elif kind=='compile-failure':row('gcc-debug 1 compile')['expected']=1
            elif kind=='wrong-marker':row('target pair execute')['marker']='unrelated assertion'
            elif kind=='wrong-provider':r=row('gcc-debug composition reference compile');r['command']=[s for s in r['command'] if not s.endswith(PROVIDERS['http'])]
            elif kind=='wrong-target':row('clang-O3 execute')['command'].remove('shift=0,align=off,sleep=off')
            elif kind=='missing-header-identity':x['external_tool_sha256']={}
            elif kind=='duplicate':x['checks'][0]=x['checks'][1]
            else:x['limits']=[]
            rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
            if not rejected:errors.append('accepted '+kind)
    out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_network_scope_depth.py','tools/run_network_scope_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
