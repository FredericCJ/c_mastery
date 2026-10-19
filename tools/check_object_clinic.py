"""Strict current evidence and exact-task guard for the memory-clinic successor."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_object_clinic import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_object_clinic as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criterion/baseline identities');need(attach(copy.deepcopy(c))==c,'pure idempotent attachment');need(ARTIFACTS[0] in c['shared_contracts'],'actual public shared contract');need(d.get('updates')==rows(r) and len(d.get('updates',[]))==6,'exact original/delayed tasks and dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'actual execution sources');need(r.get('external_sha256')==ext,'actual tool/runtime identities');need({'tools/check_object_clinic.py','tools/close_object_clinic.py',CARD,'research/v5/expansion/legacy_object_clinic/research_gate.json'}<=set(d.get('source_sha256',{})),'current guard/canonical/source bindings')
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
    def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    def inputs(n,paths):
        for p in paths:need(any(str(x).endswith('/'+p) for x in args(n)),'actual input '+n+' '+p)
    def packet(n,suffix):
        p=next((pathlib.Path(x) for x in args(n) if x.endswith(suffix)),None);need(p is not None and p.is_file() and p.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'actual retained packet '+n);return read(p) if p and p.is_file() else {}
    for cc in ['gcc','clang']:
        for profile,flags in runner.PROFILES:
            t=cc+'.'+profile;need(all(x in args(t+'.compile') for x in ['-std=c17','-Werror','-Wpedantic',*flags]),'actual profile '+t);inputs(t+'.compile',['assessor/object_followthrough/clinic/clinic.c','assessor/foundation_depth/depth.c']);inputs(t+'.stub.compile',['labs/object_followthrough/clinic/learner.c','labs/foundation_depth/src/depth.c']);inputs(t+'.shared.compile',['assessor/object_followthrough/clinic/shared.c','labs/object_followthrough/clinic/shared_tests.c']);need('-pthread' in args(t+'.shared.compile'),'actual pthread adapter '+t);inputs(t+'.shared.stub.compile',['labs/object_followthrough/clinic/shared_learner.c'])
            for case in runner.CORE:cmd(t+'.'+case,0,'MC_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'MC_ASSERT:')
            for case in runner.SHARED:cmd(t+'.shared.'+case,0,'MC_SHARED_'+case.upper()+'_PASS');cmd(t+'.shared.stub.'+case,41,'MC_SHARED_ASSERT:')
            n=t+'.history.oracle';cmd(n,0,'"observations": 324');inputs(n,['assessor/object_followthrough/clinic/oracle.py']);q=packet(n,'.history.json');need(q.get('returncode')==0 and len(q.get('stdout','').splitlines())==324,'actual 81-history finite oracle '+n)
            for m,(_,macro,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'MC_ASSERT:'+marker);need('-D'+macro in args(t+'.mutant.'+m+'.compile'),'actual semantic mutant '+t+m)
            cmd(t+'.shared.mutant.RETAIN',41,'MC_SHARED_ASSERT:mc_shared_retain(&state,&extra)==0 && state.references==2');need('-DMC_MUTANT_RETAIN' in args(t+'.shared.mutant.RETAIN.compile'),'actual retain mutant '+t)
        for case in runner.CORE:cmd(cc+'.debug.memcheck.'+case,0,'ERROR SUMMARY: 0 errors');need(bool(args(cc+'.debug.memcheck.'+case)) and args(cc+'.debug.memcheck.'+case)[0]=='valgrind','actual memory-check positive '+cc+case)
        for p in ['O0','O3']:
            t=cc+'.defects.'+p;inputs(t+'.compile',['labs/object_followthrough/clinic/defects.c']);need(all(x in args(t+'.compile') for x in ['-'+p,'-fsanitize=address,undefined','-fno-builtin-memcpy']),'actual diagnostic profile '+t);cmd(t+'.observe',0,'"diagnostic_cases": 5');cmd(t+'.assembly',0,'<main>:');q=packet(t+'.observe','.json');obs=q.get('observations',[]);need(q.get('status')=='pass' and len(obs)==5 and {x.get('case') for x in obs}=={'escape','release','access','overlap','overlap-indirect'},'actual diagnostic coverage '+t)
            need(q.get('environment',{}).get('ASAN_OPTIONS')=='detect_stack_use_after_return=1:detect_leaks=1:halt_on_error=1:exitcode=86','actual stack-return instrumentation '+t)
            for v in obs:
                text=v.get('stdout','')+v.get('stderr','');found='AddressSanitizer' in text and v.get('diagnostic','NEVER') in text;miss=cc=='gcc' and p=='O3' and v.get('case')=='overlap';need(v.get('detected')==found and v.get('expected_detection')== (not miss),'actual diagnostic classification '+t+v.get('case',''));need((v.get('exit_code')==0 and not text) if miss else (v.get('exit_code')!=0 and found),'actual diagnostic or explicitly observed miss '+t+v.get('case',''))
            if cc=='gcc' and p=='O3':need('expect-direct-overlap-miss' in args(t+'.observe'),'explicit selected missing detection');need(bool(re.search(r'call\s+\*%[a-z0-9]+',logs.get(t+'.assembly','').split('<main>:')[-1].split('<_start>:')[0])),'retained indirect boundary in main assembly')
        t=cc+'.tsan';inputs(t+'.shared.compile',['assessor/object_followthrough/clinic/shared.c']);need('-fsanitize=thread' in args(t+'.shared.compile') and '-pthread' in args(t+'.shared.compile'),'actual race instrumentation '+cc)
        for case in runner.SHARED:cmd(t+'.shared.'+case,0,'MC_SHARED_'+case.upper()+'_PASS')
        need('-fsanitize=thread' in args(t+'.race.compile') and '-DMC_MUTANT_RACE' in args(t+'.race.compile'),'actual removed-access-lock control '+cc);cmd(t+'.race.serial',0,'MC_SHARED_SERIAL_PASS');cmd(t+'.race.parallel',66,'WARNING: ThreadSanitizer: data race')
        if cc=='gcc':need('mc_shared_add' in logs.get(t+'.race.parallel',''),'actual payload race attribution gcc')
        else:
            cmd(t+'.race.symbols',0,'mc_shared_add');offsets=set(re.findall(r'clang\.tsan\.race\+(0x[0-9a-f]+)',logs.get(t+'.race.parallel','')));need(len(offsets)>=2 and offsets<=set(args(t+'.race.symbols')),'actual observed offsets symbolized');need('assessor/object_followthrough/clinic/shared.c:' in logs.get(t+'.race.symbols',''),'actual payload source attribution clang')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual isolated exported stub rejection')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    records=read(HERE/'resources.json')['resources'];need({'R-MC-N1570','R-MC-ASAN','R-MC-TSAN','R-MC-POSIX-MUTEX','R-MC-LOCAL'}=={s['id'] for s in records},'source authority identities')
    for s in records:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'bounded actual source scope');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary bytes')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-history','lost-shared-late','lost-tsan','lost-race','lost-symbols','lost-serial','lost-diagnostic','lost-assembly','lost-miss','lost-retain','lost-memcheck','lost-scope','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier task'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutant.'+k[7:]]
            elif k=='lost-tsan':next(q for q in y['commands'] if q['id']=='gcc.tsan.shared.compile')['command'].remove('-fsanitize=thread')
            elif k=='lost-miss':next(q for q in y['commands'] if q['id']=='gcc.defects.O3.observe')['command'].remove('expect-direct-overlap-miss')
            elif k=='lost-scope':x['updates'][0]['author_validation']['diagnostic_scope']='Undefined value proves semantics'
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:
                tags={'lost-history':'gcc.debug.history.oracle','lost-shared-late':'gcc.sanitized.shared.late','lost-race':'gcc.tsan.race.parallel','lost-symbols':'clang.tsan.race.symbols','lost-serial':'gcc.tsan.race.serial','lost-diagnostic':'gcc.defects.O3.observe','lost-assembly':'gcc.defects.O3.assembly','lost-retain':'gcc.debug.shared.mutant.RETAIN','lost-memcheck':'gcc.debug.memcheck.unique'};y['commands']=[q for q in y['commands'] if q['id']!=tags[k]]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
