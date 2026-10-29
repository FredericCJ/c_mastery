"""Retained exact P3 evidence plus new minimized-capacity replay and policy correction."""
import datetime,hashlib,json,pathlib,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_checkpoint_protocol';PUBLIC=ROOT/'labs/checkpoint_followthrough/protocol';PRIVATE=ROOT/'assessor/checkpoint_followthrough/protocol';BUILD=ROOT/'build'/('checkpoint-protocol-followthrough-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
RETAINED=['reviews/v5/expansion/legacy_completion/'+n for n in ['checkpoint_depth_campaign.json','checkpoint_transfer.json','p3_minimized.json']]
PROFILES=[(cc,mode,flags) for cc in ['gcc','clang'] for mode,flags in [('O0',['-O0']),('O3',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def retained():
    reports=[read(ROOT/n) for n in RETAINED]
    for d in reports:
        assert d['passed'] is True and d.get('source_stable',d.get('scope_source_stable')) is True
        for n,v in d['inspected_paths'].items():assert sha(ROOT/n)==v,('retained source changed',n)
        for n,v in d.get('log_sha256',{}).items():assert sha(ROOT/n)==v,('retained log changed',n)
    core,transfer,mini=reports;assert len(core['profiles'])==8 and all(q['passed'] for q in core['profiles'])
    for q in core['profiles']:
        for case in ['codec','stream']:assert 'checkpoint_depth.'+case in q['anchors'] and len(q['controls_by_anchor']['checkpoint_depth.'+case])==2
    for q in core['executions']:assert sha(ROOT/q['log'])==q['log_sha256']
    assert all(q['rejected'] for q in core['learner_stub_rejections'] if q['test'] in ['checkpoint_depth.codec','checkpoint_depth.stream'])
    assert len(transfer['profiles'])==6
    for profile in transfer['profiles']:
        for case,variants in [('protocol',['reference','mutant5','mutant6','learner']),('protocol16',['reference','mutant5','mutant6','learner']),('auth',['reference','mutant7','mutant8','learner']),('durable',['reference','mutant9','mutant10','learner'])]:
            selected=[q for q in profile['outcomes'] if q['case']==case];assert {q['variant'] for q in selected}==set(variants)
            for q in selected:assert q['passed'] is True and q['build_exit']==0 and q['execution_exit']==(0 if q['variant']=='reference' else 1)
    assert mini['single_byte_deletion_minimal'] and mini['deletion_probes']>=mini['minimized_bytes'] and len(mini['comparisons'])==3 and all(q['passed'] and q['repaired']==q['expected'] and '7:' not in q['faulty'] for q in mini['comparisons'])
    return {n:sha(ROOT/n) for n in RETAINED}
def pins():
    files={pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py'}
    for tree in [PUBLIC,PRIVATE]:files|={p for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    for name in RETAINED:
        files.add(ROOT/name);files|={ROOT/n for n in read(ROOT/name)['inspected_paths']}
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang']})}
def expected_commands():
    result={'kernel','gcc.version','clang.version','retained.check'}
    for cc,mode,_ in PROFILES:result|={cc+'.'+mode+'.'+n for n in ['reference.build','skip.build','replay','drop.build','drop','stub.build','stub','policy.build','policy']}
    return result
def main():
    if '--retained-check' in sys.argv:print('CPA_RETAINED_PASS '+json.dumps(retained(),sort_keys=True));return 0
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None;bindings={}
    try:
        bindings=retained();run('kernel',['uname','-a']);run('retained.check',[sys.executable,pathlib.Path(__file__),'--retained-check'],marker='CPA_RETAINED_PASS')
        for cc in ['gcc','clang']:run(cc+'.version',[cc,'--version'])
        original=read(ROOT/RETAINED[-1])['original_hex']
        for cc,mode,flags in PROFILES:
            t=cc+'.'+mode;common=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*flags,'-I'+str(PUBLIC),'-I'+str(ROOT/'labs/checkpoint_depth/include')];exes={}
            for variant,macros,source in [('reference',[],PRIVATE/'admission.c'),('skip',['-DCP_MUTANT=3'],PRIVATE/'admission.c'),('drop',['-DCPA_MUTANT_DROP'],PRIVATE/'admission.c'),('stub',[],PUBLIC/'learner.c')]:
                exe=BUILD/(t+'.'+variant);exes[variant]=exe;run(t+'.'+variant+'.build',[*common,*macros,source,PUBLIC/'replay.c',ROOT/'assessor/checkpoint_depth/checkpoint_depth.c','-o',exe])
                if variant=='skip':run(t+'.replay',[sys.executable,PUBLIC/'check_replay.py',exes['reference'],exe,ROOT/RETAINED[-1],ROOT/'labs/checkpoint_depth/tests/check_stream.py',BUILD/(t+'.replay.json')],marker='CPA_REPLAY_PASS12')
                if variant in ['drop','stub']:run(t+'.'+variant,[exe,original,'7','1','80'],41,'CPA_ASSERT:')
            exe=BUILD/(t+'.policy');run(t+'.policy.build',[*common,'-DCPX_MAX_PAYLOAD=64','-I'+str(ROOT/'labs/checkpoint_depth/transfer'),PUBLIC/'policy.c',ROOT/'assessor/checkpoint_depth/checkpoint_depth.c',ROOT/'assessor/checkpoint_depth/transfer/protocol.c','-o',exe]);run(t+'.policy',[exe],marker='CPA_POLICY_PASS original32/restricted32 refuse; configured64 accepts version1 length33')
        assert {q['id'] for q in h.COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext:failure='Scoped source/tool identity changed'
    limits=['New six-profile packet replays retained original/minimized streams through actual struct cp_parser with explicit capacity1/2 and changed chunking; physical queue storage remains two slots.','The three retained source-bound reports are checked, not re-described as newly executed; their independent oracle, mutation, protocol, authenticated-packet and durable-sink observations retain their original limits.','Configured64 acceptance of version1 length33 is a changed admission contract; original32 and compatibility-limit32 reject, as actually executed.','OpenSSL is the explicitly permitted hosted protocol primitive; CRC implementation remains first-principles, and volatile replay state is not durable authorization.','No external review, independent delayed learner result, physical power interruption or absolute mastery proof.']
    d=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,retained_reports=bindings,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=d['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
