"""Check exact capstone MCU tasks, actual compiler/guest controls and limits."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_capstone_firmware import ROOT,HERE,PROJECTS,card,original,rows,REMAINING,attach
import run_capstone_firmware as runner
original=functools.lru_cache(None)(original)
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
    errors=[]
    def need(ok,message):
        if not ok:errors.append(message)
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==24,'exact twelve ordinary/delayed pairs');need(d.get('remaining_parents')==REMAINING,'remaining evidence limits')
    for p in PROJECTS:
        c=read(ROOT/card(p));old=original(p);need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'preserved '+p);need(attach(copy.deepcopy(c))==c,'idempotent '+p)
    for x,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for k in keys:
            need(bool(x.get(k)),'nonempty '+k)
            for n,h in x.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact scoped source inputs');need(r.get('external_sha256')==external,'actual tool/header identities')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need(n in d.get('source_sha256',{}),'bound row artifact '+n)
    cmds={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(cmds)==len(r.get('commands',[])) and set(cmds)==runner.expected_commands() and all(q.get('passed') is True for q in cmds.values()),'exact executed command inventory')
    logs={}
    for n,q in cmds.items():
        p=ROOT/q.get('output','missing');t=data(p).decode('utf8') if p.is_file() else '';logs[n]=t;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in t),'actual result '+n)
    def args(n):return cmds.get(n,{}).get('command',[])
    def result(n,code,marker=None):
        q=cmds.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    for cc,mode,flags in runner.PROFILES:
        t=cc+'.'+mode
        for name,macro,case in [('authority','-DCF_MUTANT=1',0),('lease','-DCF_MUTANT=2',1),('watchdog','-DCF_MUTANT=3',2),('trust','-DCF_MUTANT=4',3),('status','-DCF_MUTANT=5',7)]:
            need(macro in args(t+'.'+name+'.build'),'actual mutant '+t+name);result(t+'.'+name+'.'+str(case),41,'CAP_FIRMWARE_ASSERT')
        for name in ['reference','authority','lease','watchdog','trust','status','stub','transfer','representation']:need(all(f in args(t+'.'+name+'.build') for f in flags+['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic']),'actual profile '+t+name)
        need(any(a.endswith('/learner.c') for a in args(t+'.stub.build')),'actual unfinished source '+t);result(t+'.stub.0',41,'CAP_FIRMWARE_ASSERT')
        for case in range(8):result(t+'.reference.'+str(case),0,'CAP_FIRMWARE_PASS')
        result(t+'.transfer',0,'CAP_MCU_TRANSFER_PASS');result(t+'.representation',0,'CAP_MCU_RESULT');need('failure-callbacks=0' in logs.get(t+'.representation',''),'actual result semantics '+t)
    for cc in ['gcc','clang']:
        result(cc+'.result-default-reject',1,'setjmp.h');need('-isystem' in args(cc+'.result-header') and str(runner.NEWLIB) in args(cc+'.result-header'),'explicit target headers '+cc);need('longjmp' in logs.get(cc+'.result-full.undefined',''),'actual whole-result closure '+cc)
        for opt in ['O0','O3']:
            for variant in ['normal','reorder','copy','bss','vector','mpu','f439']:
                t=cc+'.'+opt+'.'+variant;linkargs=args(t+'.link');need(all(f in linkargs for f in ['-nostdlib','-nostartfiles','-nodefaultlibs']),'strict image '+t);need(not logs.get(t+'.undefined','').strip(),'closed image '+t)
                for i in range(12):need(all(f in args(t+'.compile.'+str(i)) for f in ['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fstack-usage']),'actual target compile '+t+str(i))
                macro={'copy':'-DCF_BAD_COPY','bss':'-DCF_NO_BSS','vector':'-DCF_WRONG_VECTOR','mpu':'-DCF_NO_MPU'}.get(variant)
                if macro:need(all(macro in args(t+'.compile.'+str(i)) for i in range(12)),'intended target control '+t)
                p=pathlib.Path(linkargs[-1]) if linkargs else None;need(p is not None and p.is_file(),'actual ELF '+t)
                if p is not None and p.is_file():
                    folder=p.parent;budget=read(folder/'budget.json');frames=budget['static_frames'];need(frames and all(isinstance(f['bytes'],int) and f['bytes']>=0 for f in frames),'actual frame records '+t);need(budget['conservative_stack']==3*sum(f['bytes'] for f in frames)+200 and budget['total']==budget['static_ram_including_padding']+budget['conservative_stack'] and budget['total']<65536 and budget['admit4096']==(budget['total']<=4096) and budget['admit2048']==(budget['total']<=2048) and budget['fp_two_extended_delta']==144,'honest selected budget '+t)
                    image=(folder/'image.bin').read_bytes();vector=int.from_bytes(image[4:8],'little');need(int.from_bytes(image[:4],'little')==0x20010000,'selected stack '+t);need(vector==0 if variant=='vector' else bool(vector&1),'actual vector control '+t)
                    maptext=(folder/'image.map').read_text();need(not re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',maptext),'final helper allowlist '+t)
                if variant=='f439':
                    need(any(a.endswith('/f439_adapter.c') for a in args(t+'.compile.6')) and any(a.endswith('/peripheral_depth/dma/target.c') for a in args(t+'.compile.7')),'actual imported board composition '+t);need(' cap_board' in logs.get(t+'.symbols','') and ' pd_target_io' in logs.get(t+'.symbols',''),'retained actual port '+t);need(t+'.run' not in cmds,'no false F439 execution '+t)
                elif variant=='vector':need(t+'.run' not in cmds,'invalid vector not executed '+t)
                else:
                    result(t+'.run',41 if variant in ['copy','bss','mpu'] else 0,'CAP_MCU_MPU_REJECT' if variant=='mpu' else 'CAP_MCU_STARTUP_REJECT' if variant in ['copy','bss'] else 'CAP_MCU_PASS');need(sum('loader,file=' in a for a in args(t+'.run'))==2 and 'mps2-an386' in args(t+'.run'),'actual poisoned selected guest '+t)
    for t in ['gcc.O0.normal','gcc.O0.f439']:
        result(t+'.release',0,'CAP_MCU_EVIDENCE_PASS');need('categories8 timing6 stale-bundle debug-identity physical-pending' in logs.get(t+'.release',''),'claim and release negative controls '+t)
        command=args(t+'.debug.binary');p=pathlib.Path(command[-1]) if command else None;need(p is not None and p.is_file(),'actual stripped bytes '+t)
        if p is not None and p.is_file():need(data(p)==data(p.parent/'image.bin'),'stripped load-byte equivalence '+t);need(read(p.parent/'release.json')['physical_observation']=='not-run','build is not deployed identity '+t)
    for row in d.get('updates',[]):need(bool(row.get('author_validation',{}).get('checks')) and set(row.get('author_validation',{}).get('checks',[]))<=set(cmds),'objective-specific executed checks')
    timing=read(ROOT/'fixtures/legacy_capstones/firmware_timing_matrix.json')['rows'];need(len(timing)==3,'three concrete timing rows')
    for q,ticks,hz in zip(timing,[12,12,65548],[16000000,8000000,16000000]):need(q.get('expected_ticks')==ticks and q.get('expected_nanoseconds')==ticks*1000000000//hz and q.get('measured_maximum') is None and all(q.get(k) for k in ['requirement','analytical_assumptions','timer_resolution','clock_configuration','interference','physical_gate']),'exact six-column timing/physical distinction')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==44 and all(q.get('passed') is True for q in e['commands']),'four actual learner exports')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','actual source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('passages')==s['locators'] and i.get('scope')=='; '.join(s['locators']),'actual exact source scope');need(sha(ROOT/i['capture']['path'])==i['capture']['sha256'],'captured source')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'inspected input '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        names=['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-checks','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','stale-tool','lost-authority','lost-lease','lost-watchdog','lost-trust','lost-register','lost-startup','lost-mpu','lost-board','lost-reorder','lost-transfer','lost-release','lost-dependency','wrong-mutation','wrong-target-control','lost-remaining']
        for name in names:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if name=='missing-criterion':x['updates'].pop(0)
            elif name=='missing-delayed':x['updates'].pop()
            elif name=='changed-task':x['updates'][0]['original_task']='placeholder'
            elif name=='changed-delay':x['updates'][1]['minimum_days']=0
            elif name=='wrong-checks':x['updates'][0]['author_validation']['checks']=[]
            elif name.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[name]]['state']='passed'
            elif name.startswith('stale-'):k={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-tool':'external_sha256'}[name];y[k][next(iter(y[k]))]='0'*64
            elif name=='wrong-mutation':next(q for q in y['commands'] if q['id']=='gcc.O0.authority.build')['command'].remove('-DCF_MUTANT=1')
            elif name=='wrong-target-control':next(q for q in y['commands'] if q['id']=='gcc.O0.mpu.compile.7')['command'].remove('-DCF_NO_MPU')
            elif name=='lost-remaining':x['remaining_parents']=[]
            else:
                missing={'lost-authority':'gcc.O0.authority.0','lost-lease':'gcc.O0.lease.1','lost-watchdog':'gcc.O0.watchdog.2','lost-trust':'gcc.O0.trust.3','lost-register':'gcc.O0.status.7','lost-startup':'gcc.O0.copy.run','lost-mpu':'gcc.O0.mpu.run','lost-board':'gcc.O0.f439.compile.7','lost-reorder':'clang.O3.reorder.run','lost-transfer':'clang.asan.transfer','lost-release':'gcc.O0.f439.release','lost-dependency':'gcc.result-full.undefined'}[name];y['commands']=[q for q in y['commands'] if q['id']!=missing]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=name,rejected=rejected))
            if not rejected:errors.append(name+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('inputs changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
