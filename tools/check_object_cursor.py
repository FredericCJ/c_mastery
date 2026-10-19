"""Reject missing exact tasks, stale experiments, weakened closure and invented assessment."""
import argparse,copy,functools,hashlib,json,pathlib
from close_object_cursor import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_object_cursor as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criterion/baseline identity');need(attach(copy.deepcopy(c))==c,'idempotent pure attachment');need(ARTIFACTS[0] in c['shared_contracts'] and 'labs/foundation_depth/CONTRACTS.md' in c['shared_contracts'],'explicit real shared contracts')
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==18,'exact ordinary/delayed rows and six truthful dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'actual bound artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact execution sources');need(r.get('external_sha256')==ext,'actual current tool/runtime identities');need({'tools/check_object_cursor.py','tools/close_object_cursor.py',CARD,'research/v5/expansion/legacy_object_cursor/research_gate.json'}<=set(d.get('source_sha256',{})),'current guard/canonical/source bindings')
    deps=r.get('dependency_sha256',{});need(len(deps)>10 and any('/sysroot/usr/include/' in n for n in deps),'actual i386 headers')
    for n,h in deps.items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'current dependency '+n)
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'observed expected result '+n)
    def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    def inputs(n,paths):
        for p in paths:need(any(str(x).endswith('/'+p) for x in args(n)),'actual source '+n+' '+p)
    def oracle(t):
        q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('interval_cases')==6435,'actual independent interval result '+t);inputs(t+'.oracle',['assessor/object_followthrough/cursor/oracle.py']);packet=next((pathlib.Path(n) for n in args(t+'.oracle') if n.endswith('.packets.json')),None)
        need(packet is not None and packet.is_file() and packet.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'retained raw interval packet '+t)
        if packet and packet.is_file():q=read(packet);need(q.get('returncode')==0 and len(q.get('stdout','').splitlines())==6435,'actual raw interval count '+t)
    for cc in ['gcc','clang']:
        for profile,flags in runner.PROFILES:
            t=cc+'.'+profile;need(all(f in args(t+'.compile') for f in ['-std=c17','-Werror','-Wpedantic',*flags]),'actual core profile '+t);inputs(t+'.compile',['assessor/object_followthrough/cursor/cursor.c','assessor/foundation_depth/depth.c']);inputs(t+'.stub.compile',['labs/object_followthrough/cursor/learner.c'])
            for case in runner.CASES:cmd(t+'.'+case,0,'OC_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'OC_ALLOCATED_ASSERT:' if case=='allocated' else 'OC_ASSERT:')
            oracle(t)
            for m,(_,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'OC_ASSERT:'+marker);need(('-DFD_MUTATE_CURSOR' if m=='FD_CURSOR' else '-DOC_MUTANT_'+m) in args(t+'.mutant.'+m+'.compile'),'compiled intended mutation '+t+m)
            for v,macro in [('valid',None),('abi','OC_PLUGIN_BAD_ABI'),('size','OC_PLUGIN_BAD_SIZE'),('missing','OC_PLUGIN_MISSING')]:
                inputs(t+'.plugin.'+v,['labs/object_followthrough/cursor/plugin.c']);need(all(x in args(t+'.plugin.'+v) for x in ['-fPIC','-shared',*(['-D'+macro] if macro else [])]),'actual original module '+t+v)
            for v in ['reference','failure','CLOSE','RETURN','stub']:
                tag=t+'.loader.'+v;cmd(tag,0 if v in ['reference','failure'] else 41,'OC_LOADER_PASS' if v in ['reference','failure'] else 'OC_LOADER_ASSERT:');inputs(tag+'.compile',['labs/object_followthrough/cursor/loader_learner.c'] if v=='stub' else ['assessor/object_followthrough/cursor/loader.c','assessor/object_followthrough/cursor/function_lookup.c']);need(len(args(tag))==5 and all(str(x).endswith('.so') for x in args(tag)[1:]),'four actual local modules '+tag)
                if v=='failure':need('-DOC_CLOSE_FAILURE' in args(tag+'.compile') and '-Wl,--wrap=dlclose' in args(tag+'.compile'),'actual close-failure control '+t)
                if v in ['CLOSE','RETURN']:
                    need('-DOC_MUTANT_'+v in args(tag+'.compile'),'actual loader mutant '+tag);needle='oc_close(&m)==OC_LOAD_BUSY && m.handle && a->apply(8)==11' if v=='CLOSE' else 'oc_return(&m,&a)==OC_LOAD_OK && !a && m.borrows==1';need('OC_LOADER_ASSERT:'+needle in logs.get(tag,''),'specific loader assertion '+tag)
            need('-Wpedantic' not in args(t+'.loader.reference.compile'),'loader extension boundary separate from strict core')
        for name in ['table','zero']:cmd(cc+'.negative.'+name,1,'error:');need('-fsyntax-only' in args(cc+'.negative.'+name),'compile-only invalid source')
        cmd(cc+'.negative.function',1 if cc=='gcc' else 0,'error:' if cc=='gcc' else None)
        for opt in ['O0','O3']:
            t=cc+'.arm.'+opt
            for unit in ['core','foundation','entry']:need(all(x in args(t+'.'+unit) for x in ['-'+opt,'-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-mcpu=cortex-m4','-mthumb','-c']),'actual selected strict target '+t+unit)
            inputs(t+'.foundation',['assessor/foundation_depth/depth.c']);need('-nostdlib' in args(t+'.link') and any('--gc-sections' in n for n in args(t+'.link')),'no-runtime final selected closure '+t);need(not logs.get(t+'.undefined','missing').strip(),'empty target undefined symbols '+t)
            symbols=logs.get(t+'.symbols','');need(all(n in symbols for n in ['fd_take','fd_subview','oc_previous','oc_difference','oc_encode_index','oc_target_result']) and not any(n in symbols for n in ['__aeabi_','malloc','memcpy','memmove','memset','dlopen','dlsym','__atomic_']),'actual retained helper-free image '+t);cmd(t+'.bad.link',1,'malloc');need('-DOC_TARGET_HELPER' in args(t+'.bad.entry'),'actual forbidden allocator control '+t);cmd(t+'.assembly',0,'<oc_previous>')
    oracle('i386');cmd('i386.elf',0,'elf32-i386');need('-m32' in args('i386.compile') and '--sysroot='+str(runner.SYSROOT) in args('i386.compile'),'actual narrower ABI');
    for case in runner.CASES:cmd('i386.'+case,0,'OC_'+case.upper()+'_PASS');need(args('i386.'+case) and args('i386.'+case)[0]==str(runner.LOADER),'actual narrow loader '+case)
    need(not any('analysis_only' in str(x) for q in commands.values() for x in q.get('command',[])),'invalid semantic examples never execute')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==13 and all(q.get('passed') is True for q in e['commands']),'actual exported learner rejection controls')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current public export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','actual source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'bounded source inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary bytes')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-i386','lost-loader','lost-dependencies','lost-oracle','lost-target','lost-helper-control','lost-foundation','lost-close-failure','false-clang-diagnostic','lost-remaining',*['mutant-'+m for m in runner.MUTANTS],'mutant-CLOSE','mutant-RETURN']:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier task'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):m=k[7:];tag='gcc.debug.loader.'+m if m in ['CLOSE','RETURN'] else 'gcc.debug.mutant.'+m;y['commands']=[q for q in y['commands'] if q['id']!=tag]
            elif k=='lost-i386':next(q for q in y['commands'] if q['id']=='i386.compile')['command'].remove('-m32')
            elif k=='lost-loader':next(q for q in y['commands'] if q['id']=='i386.matrix')['command'][0]='host-model'
            elif k=='lost-dependencies':y['dependency_sha256']={}
            elif k=='lost-target':next(q for q in y['commands'] if q['id']=='gcc.arm.O0.link')['command'].remove('-nostdlib')
            elif k=='lost-foundation':next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command']=[n for n in next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command'] if not n.endswith('/assessor/foundation_depth/depth.c')]
            elif k=='lost-close-failure':next(q for q in y['commands'] if q['id']=='gcc.debug.loader.failure.compile')['command'].remove('-Wl,--wrap=dlclose')
            elif k=='false-clang-diagnostic':next(q for q in y['commands'] if q['id']=='clang.negative.function')['expected_exit']=1
            elif k in ['lost-oracle','lost-helper-control']:
                tag='gcc.debug.oracle' if k=='lost-oracle' else 'gcc.arm.O0.bad.link';y['commands']=[q for q in y['commands'] if q['id']!=tag]
            else:x['remaining_parents']=[]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
