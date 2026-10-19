"""Exact retained object tasks, real evidence and false-closure controls."""
import argparse,copy,functools,hashlib,json,pathlib
from close_object_atlas import ROOT,HERE,CARD,original,rows,ARTIFACTS,attach
import run_object_atlas as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original task/baseline preservation');need(attach(copy.deepcopy(c))==c,'idempotent actual attachment');need(ARTIFACTS[0] in c['shared_contracts'],'required public contract')
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==18,'all exact ordinary/delayed tasks and truthful dimensions');need(d.get('remaining_parents')==['Other T03 projects remain required and receive separate exact implementation evidence.','Exact published C17/C23, external review, independent learner delayed assessment and physical observations remain open.'],'remaining required scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound actual artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact scoped code closure');need(r.get('external_sha256')==external,'current actual tool/runtime identities')
    deps=r.get('dependency_sha256',{});need(len(deps)>10 and any('/sysroot/usr/include/' in p for p in deps),'actual i386 header dependencies')
    for n,h in deps.items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'current dependency '+n)
    need({'tools/check_object_atlas.py','tools/close_object_atlas.py',CARD,'research/v5/expansion/legacy_object_atlas/research_gate.json'}<=set(d.get('source_sha256',{})),'canonical/guard/source bindings')
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual observation '+n)
    def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def inputs(n,paths):
        args=commands.get(n,{}).get('command',[])
        for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
    def oracle(tag):
        obj=json.loads(logs.get(tag+'.oracle','{}'));need(obj.get('status')=='pass' and obj.get('placement_cases')==3690 and obj.get('canonical')=='7afffffffe3ff8000000000000','independent oracle result '+tag);inputs(tag+'.oracle',['assessor/object_followthrough/atlas/oracle.py']);args=commands.get(tag+'.oracle',{}).get('command',[]);packet=next((pathlib.Path(n) for n in args if n.endswith('.packets.json')),None)
        need(packet is not None and packet.is_file() and packet.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'actual retained oracle packets '+tag)
        if packet and packet.is_file():
            q=read(packet);need(len(q.get('packets',[]))==2 and all(x.get('returncode')==0 for x in q['packets']) and len(q['packets'][0]['stdout'].splitlines())==3690 and 'OA_CANONICAL:7afffffffe3ff8000000000000' in q['packets'][1]['stdout'],'actual finite table and encoding observations '+tag)
    for cc in ['gcc','clang']:
        for profile,flags in runner.PROFILES:
            t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and '-Werror' in args and all(f in args for f in flags),'actual C17 profile '+t);inputs(t+'.compile',['assessor/object_followthrough/atlas/atlas.c']);inputs(t+'.stub.compile',['labs/object_followthrough/atlas/learner.c'])
            for case in runner.CASES:cmd(t+'.'+case,0,'OA_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'OA_ASSERT:')
            oracle(t)
            for m,(_,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'OA_ASSERT:'+marker);need('-DOA_MUTANT_'+m in commands.get(t+'.mutant.'+m+'.compile',{}).get('command',[]),'compiled intended defect '+t+m)
    oracle('gcc.lto');need('-flto' in commands.get('gcc.lto.compile',{}).get('command',[]),'actual LTO oracle')
    oracle('i386');need('-m32' in commands.get('i386.compile',{}).get('command',[]) and '--sysroot='+str(runner.SYSROOT) in commands.get('i386.compile',{}).get('command',[]),'actual narrower ABI');cmd('i386.elf',0,'elf32-i386');need('model:8,8,8,53' in logs.get('gcc.debug.representation','') and 'model:4,4,8,53' in logs.get('i386.representation',''),'observed LP64/ILP32 model change')
    for case in runner.CASES:cmd('i386.'+case,0,'OA_'+case.upper()+'_PASS');need(commands.get('i386.'+case,{}).get('command',[None])[0]==str(runner.LOADER),'actual selected i386 loader '+case)
    cmd('decode.assembly',0,'<oa_decode_u32>');need('--disassemble=oa_decode_u32' in commands.get('decode.assembly',{}).get('command',[]),'actual bounded decoder assembly')
    for profile in ['debug','optimized','sanitized','tsan']:
        t='gcc.tls.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need(args and args[0]=='gcc' and '-std=gnu17' in args and '-pthread' in args,'declared actual GCC TLS contract '+t);inputs(t+'.compile',['labs/object_followthrough/atlas/tls.c']);cmd(t+'.run',0,'OA_TLS_PASS:')
        if profile=='tsan':need('-fsanitize=thread' in args,'actual TSan instrumentation')
        for m in ['GLOBAL','NOWRITE']:cmd(t+'.mutant.'+m,41,'OA_TLS_ASSERT:');need('-DOA_TLS_'+m in commands.get(t+'.mutant.'+m+'.compile',{}).get('command',[]),'actual TLS control '+t+m)
    need(not any(any(str(x).endswith('/analysis_only.c') for x in q.get('command',[])) for q in commands.values()),'invalid examples remain analysis only')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual public export controls')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        ins=s['inspection'];need(ins.get('scope')=='; '.join(s['locators']) and ins.get('passages')==s['locators'] and ins.get('method'),'bounded actual source inspection');cap=ins.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'actual primary source capture')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();external=runner.external();errors=validate(d,r,pins,external);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-i386','lost-loader','lost-dependencies','lost-tls','lost-tsan','lost-oracle','lost-assembly','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier replacement'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutant.'+k[7:]]
            elif k=='lost-i386':next(q for q in y['commands'] if q['id']=='i386.compile')['command'].remove('-m32')
            elif k=='lost-loader':next(q for q in y['commands'] if q['id']=='i386.lifetime')['command'][0]='host-model'
            elif k=='lost-dependencies':y['dependency_sha256']={}
            elif k=='lost-tsan':next(q for q in y['commands'] if q['id']=='gcc.tls.tsan.compile')['command'].remove('-fsanitize=thread')
            elif k in ['lost-tls','lost-oracle','lost-assembly']:
                tag={'lost-tls':'gcc.tls.debug.run','lost-oracle':'gcc.debug.oracle','lost-assembly':'decode.assembly'}[k];y['commands']=[q for q in y['commands'] if q['id']!=tag]
            else:x['remaining_parents']=[]
            rejected=bool(validate(x,y,pins,external));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=external:errors.append('sources changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
