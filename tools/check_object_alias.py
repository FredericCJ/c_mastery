"""Verify exact alias tasks, original imports, finite experiments and honest evidence dimensions."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_object_alias import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_object_alias as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criteria and baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent pure attachment');need(ARTIFACTS[0] in c['shared_contracts'] and 'labs/foundation_depth/CONTRACTS.md' in c['shared_contracts'],'actual shared contracts')
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==24,'exact ordinary/delayed tasks and honest six dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact execution sources');need(r.get('external_sha256')==ext,'actual tool/runtime identities');need({'tools/check_object_alias.py','tools/close_object_alias.py',CARD,'research/v5/expansion/legacy_object_alias/research_gate.json'}<=set(d.get('source_sha256',{})),'current guard/canonical/source bindings')
    deps=r.get('dependency_sha256',{});need(len(deps)>10 and any('/sysroot/usr/include/' in n for n in deps),'actual narrow headers')
    for n,h in deps.items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual dependency '+n)
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
    def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    def inputs(n,paths):
        for p in paths:need(any(str(x).endswith('/'+p) for x in args(n)),'actual input '+n+' '+p)
    def oracle(t):
        q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('range_cases')==6048,'independent range count '+t);inputs(t+'.oracle',['assessor/object_followthrough/alias/oracle.py']);packet=next((pathlib.Path(n) for n in args(t+'.oracle') if n.endswith('.packets.json')),None)
        need(packet is not None and packet.is_file() and packet.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'retained raw range packet '+t)
        if packet and packet.is_file():q=read(packet);need(q.get('returncode')==0 and len(q.get('stdout','').splitlines())==6048,'actual raw range count '+t)
    for cc in ['gcc','clang']:
        for profile,flags in runner.PROFILES:
            t=cc+'.'+profile;need(all(f in args(t+'.compile') for f in ['-std=c17','-Werror','-Wpedantic',*flags]),'actual core profile '+t);inputs(t+'.compile',['assessor/object_followthrough/alias/alias.c','assessor/foundation_depth/depth.c']);inputs(t+'.stub.compile',['labs/object_followthrough/alias/learner.c','labs/foundation_depth/src/depth.c'])
            for case in runner.CASES:cmd(t+'.'+case,0,'OS_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'OS_ASSERT:')
            oracle(t)
            for m,(_,define,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'OS_ASSERT:'+marker);need('-D'+define in args(t+'.mutant.'+m+'.compile'),'actual intended mutation '+t+m)
            inputs(t+'.extension.compile',['labs/object_followthrough/alias/extension.c']);cmd(t+'.extension',0,'OS_EXTENSION_PASS')
            for variant in ['safe','unsafe']:
                n=t+'.wrap.'+variant;inputs(n+'.compile',['assessor/foundation_depth/depth.c','labs/object_followthrough/alias/wrap.c']);need(('-DFD_MUTATE_HANDLES' in args(n+'.compile'))==(variant=='unsafe'),'actual wrap policy '+n);cmd(n,3 if variant=='safe' else 0,'OS_WRAP_REFUSED' if variant=='safe' else 'OS_WRAP_COLLISION')
            if profile in ['strict','relaxed']:
                for unit in ['bench.core','bench.entry']:need(all(x in args(t+'.'+unit) for x in flags) and '-c' in args(t+'.'+unit),'actual timed optimization '+t+unit)
                inputs(t+'.bench.core',['assessor/object_followthrough/alias/alias.c']);inputs(t+'.bench.entry',['labs/object_followthrough/alias/bench.c']);need(all('-flto' not in str(x) for n in ['bench.core','bench.entry','bench.link'] for x in args(t+'.'+n)),'separate retained function calls '+t)
                assembly=logs.get(t+'.assembly','');need(all(re.search(r'call\s+[^\n]*<'+name+r'>',assembly) for name in ['os_transform','os_ordinary']),'actual two measured calls '+t);need(all(name in logs.get(t+'.sizes','') for name in ['os_transform','os_ordinary']),'both actual code sizes '+t);need('vectoriz' in logs.get(t+'.bench.core',''),'actual emitted vectorization diagnostics '+t)
                inputs(t+'.measure',['assessor/object_followthrough/alias/measure.py']);q=json.loads(logs.get(t+'.measure','{}'));need(q.get('samples')==240 and q.get('groups')==16,'actual measured cell count '+t);packet=next((pathlib.Path(n) for n in args(t+'.measure') if n.endswith('.timing.json')),None);need(packet is not None and packet.is_file() and packet.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'retained raw timing '+t)
                if packet and packet.is_file():q=read(packet);need(q.get('status')=='pass' and q.get('samples')==240 and len(q.get('raw_stdout','').splitlines())==240 and len(q.get('groups',[]))==16,'actual raw timing cells '+t)
        for opt in ['O0','O3']:
            t=cc+'.arm.'+opt
            for unit in ['core','foundation','entry']:need(all(x in args(t+'.'+unit) for x in ['-'+opt,'-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-mcpu=cortex-m4','-mthumb','-c']),'actual strict target '+t+unit)
            inputs(t+'.foundation',['assessor/foundation_depth/depth.c']);need('-nostdlib' in args(t+'.link') and any('--gc-sections' in n for n in args(t+'.link')),'no-runtime final closure '+t);need(not logs.get(t+'.undefined','missing').strip(),'empty undefined symbols '+t)
            symbols=logs.get(t+'.symbols','');need(all(n in symbols for n in ['fd_slot_initialize','fd_move','fd_pool_release','os_wide_reset','os_clone','os_transform','os_target_result']) and not any(n in symbols for n in ['__aeabi_','malloc','memcpy','memmove','memset','__atomic_']),'actual retained helper-free image '+t);cmd(t+'.bad.link',1,'malloc');need('-DOS_TARGET_HELPER' in args(t+'.bad.entry'),'actual forbidden helper control '+t);cmd(t+'.assembly',0,'<os_clone>')
    oracle('i386');cmd('i386.elf',0,'elf32-i386');need('-m32' in args('i386.compile') and '--sysroot='+str(runner.SYSROOT) in args('i386.compile'),'actual narrower ABI')
    for case in runner.CASES:cmd('i386.'+case,0,'OS_'+case.upper()+'_PASS');need(args('i386.'+case) and args('i386.'+case)[0]==str(runner.LOADER),'actual narrower loader '+case)
    need('pointer=8,handle=24' in logs.get('gcc.debug.identity','') and 'pointer=4,handle=12' in logs.get('i386.identity',''),'observed distinct handle costs');need(not any('analysis_only' in str(x) for q in commands.values() for x in q.get('command',[])),'invalid examples remain analysis-only')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual exported learner rejection')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current public export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','actual source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    resources=read(HERE/'resources.json')['resources'];need({'R-OS-N1570','R-OS-N2219','R-OS-N2263','R-OS-GCC-ALIAS','R-OS-GCC-PTR','R-OS-CLANG'}<={s['id'] for s in resources},'actual distinct authority sources')
    for s in resources:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'bounded source inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary bytes')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-i386','lost-loader','lost-dependencies','lost-oracle','lost-target','lost-helper-control','lost-foundation','lost-public-foundation','lost-extension','lost-wrap-control','lost-strict-mode','lost-measure','lost-assembly','lost-analysis-scope','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier task'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutant.'+k[7:]]
            elif k=='lost-i386':next(q for q in y['commands'] if q['id']=='i386.compile')['command'].remove('-m32')
            elif k=='lost-loader':next(q for q in y['commands'] if q['id']=='i386.identity')['command'][0]='host-model'
            elif k=='lost-dependencies':y['dependency_sha256']={}
            elif k=='lost-target':next(q for q in y['commands'] if q['id']=='gcc.arm.O0.link')['command'].remove('-nostdlib')
            elif k in ['lost-foundation','lost-public-foundation']:
                q=next(q for q in y['commands'] if q['id']==('gcc.debug.compile' if k=='lost-foundation' else 'gcc.debug.stub.compile'));q['command']=[n for n in q['command'] if not n.endswith('/assessor/foundation_depth/depth.c' if k=='lost-foundation' else '/labs/foundation_depth/src/depth.c')]
            elif k=='lost-strict-mode':next(q for q in y['commands'] if q['id']=='gcc.strict.compile')['command'].remove('-fstrict-aliasing')
            elif k=='lost-analysis-scope':x['updates'][20]['author_validation']['analysis_scope']='Compiler run proves every proposal claim'
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:
                tags={'lost-oracle':'gcc.debug.oracle','lost-helper-control':'gcc.arm.O0.bad.link','lost-extension':'clang.strict.extension','lost-wrap-control':'gcc.debug.wrap.unsafe','lost-measure':'clang.relaxed.measure','lost-assembly':'gcc.strict.assembly'};y['commands']=[q for q in y['commands'] if q['id']!=tags[k]]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
