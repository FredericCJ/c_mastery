"""Exact struct in_transfer composition, hosted backend alternation and explicit strict closure."""
import datetime,hashlib,json,pathlib,re,shutil,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_checkpoint_writer';PUBLIC=ROOT/'labs/checkpoint_followthrough/writer';PRIVATE=ROOT/'assessor/checkpoint_followthrough/writer';BUILD=ROOT/'build'/('checkpoint-writer-followthrough-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
INCLUDES=['labs/checkpoint_followthrough/writer','labs/checkpoint_depth/include','labs/integration/include','labs/foundations/include','labs/algorithms/include','labs/checkpoint_depth/transfer','labs/ownership/include','labs/errors/include']
CORE=['assessor/checkpoint_depth/checkpoint_writer.c','assessor/integration/integration.c','assessor/foundations/foundation.c','assessor/algorithms/algo.c']
EXTRA=['assessor/checkpoint_depth/transfer/adapters.c','assessor/errors/errors.c']
PROVIDER=[*CORE,*EXTRA,'assessor/checkpoint_depth/targets/entry.S','assessor/checkpoint_depth/targets/byte_helpers.c','labs/checkpoint_depth/transfer/adapters.h','labs/checkpoint_depth/include/checkpoint_writer.h','labs/errors/include/errors.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h']
PROFILES=[(cc,mode,flags) for cc in ['gcc','clang'] for mode,flags in [('O0',['-O0']),('O3',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]]
MUTANTS={'requeue':('byte',['-DCWF_MUTANT_REQUEUE'],'CWF_SINK:3:65:66:65'),'suffix':('span',['-DCWF_MUTANT_SUFFIX'],'s.transfer.accepted==3'),'short':('progress',['-DCP_MUTANT=19'],'i==3?CPW_DONE:CPW_PENDING'),'retry':('progress',['-DCP_MUTANT=20'],'t.accepted==(i<2?3:i==2?5:9)'),'cause':('progress',['-DCP_MUTANT=22'],'error.primary==5'),'product':('progress',['-DCPX_MUTANT=4'],'product.error.primary==5')}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
    files={pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in PROVIDER]}
    for d in [PUBLIC,PRIVATE,*[ROOT/n for n in ['labs/integration/include','labs/foundations/include','labs/algorithms/include']]]:files|={p for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','nm','objdump']})}
def expected_commands():
    result={'kernel','gcc.version','clang.version'}
    for cc,mode,_ in PROFILES:
        t=cc+'.'+mode;result|={t+'.'+n for n in ['build','byte','span','progress.build','progress','log.build','log','shapes','stub.build','stub.byte','stub.span']}
        for m in MUTANTS:result|={t+'.mutant.'+m+'.build',t+'.mutant.'+m}
        if mode!='asan':result|={t+'.strict.build',t+'.strict.run',t+'.strict.nm',t+'.strict.assembly'}
    return result
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None;shapes={}
    try:
        run('kernel',['uname','-a'])
        for cc in ['gcc','clang']:run(cc+'.version',[cc,'--version'])
        for cc,mode,flags in PROFILES:
            t=cc+'.'+mode;common=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections',*flags,*['-I'+str(ROOT/n) for n in INCLUDES]]
            def build(name,case,macros=(),stub=False):
                exe=BUILD/name;test=PUBLIC/('progress.c' if case=='progress' else 'tests.c');sources=[PUBLIC/'learner.c' if stub else PRIVATE/'writer.c',test,*[ROOT/n for n in CORE],*[ROOT/n for n in EXTRA if case=='progress']];run(name+'.build' if name!=t else t+'.build',[*common,*macros,*sources,'-Wl,--gc-sections','-o',exe]);return exe
            exe=build(t,'byte');run(t+'.byte',[exe],marker='CWF_BYTE_PASS');run(t+'.span',[exe,'span'],marker='CWF_SPAN_PASS')
            progress=build(t+'.progress','progress');run(t+'.progress',[progress],marker='CWF_PROGRESS_PASS');logged=build(t+'.log','progress',['-DCWF_LOG']);run(t+'.log',[logged],marker='log_records=1')
            run(t+'.shapes',['objdump','-d',progress],marker='<cpx_progress_step>:');raw=(BUILD/(t+'.shapes.txt')).read_text() if (BUILD/(t+'.shapes.txt')).exists() else (ROOT/h.COMMANDS[-1]['output']).read_text();shapes[t]={}
            for symbol in ['cpw_step','cpx_progress_step','er_transfer','cwf_error']:
                match=re.search(r'<'+symbol+r'>:\n(.*?)(?=\n\n)',raw,re.S);assert match,symbol;jumps=re.findall(r'\t(j[a-z]+)\s',match.group(1));shapes[t][symbol]=dict(static_conditional_branch_instructions=sum(j not in ['jmp','jmpq'] for j in jumps),scope='Static sites in the complete selected executable, not dynamic frequency or universal overhead.')
            stub=build(t+'.stub','byte',stub=True);run(t+'.stub.byte',[stub],41,'CWF_ASSERT:');run(t+'.stub.span',[stub,'span'],41,'CWF_ASSERT:')
            for mutant,(case,macros,marker) in MUTANTS.items():
                name=t+'.mutant.'+mutant;exe=build(name,case,macros);run(name,[exe,*(['span'] if case=='span' else [])],41,marker)
            if mode!='asan':
                exe=BUILD/(t+'.strict');strict=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-'+mode,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-ffunction-sections','-fdata-sections',*['-I'+str(ROOT/n) for n in INCLUDES],PRIVATE/'writer.c',PRIVATE/'target.c',*[ROOT/n for n in CORE],ROOT/'assessor/checkpoint_depth/targets/entry.S',ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c','-nostdlib','-nostartfiles','-nodefaultlibs','-static',*(['-no-pie'] if cc=='gcc' else []),'-Wl,-e,_start,-z,noexecstack,--gc-sections,-Map,'+str(exe)+'.map','-o',exe];run(t+'.strict.build',strict);run(t+'.strict.run',[exe]);run(t+'.strict.nm',['nm',exe],marker=' in_drain');run(t+'.strict.assembly',['objdump','-d',exe],marker='<cwf_byte_step>:')
                names=(ROOT/h.COMMANDS[-2]['output']).read_text();assert not re.search(r'\bU\b|\b(malloc|free|printf|puts|write|__libc_start_main)\b',names),names
        assert {q['id'] for q in h.COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext:failure='Scoped source/tool identity changed'
    limits=['Actual Linux x86-64 pipes/raw syscalls/libc and exact struct in_transfer composition. EINTR transition is deliberately scripted; no asynchronous interruption timing is claimed.','Six hosted profiles plus four actual strict linked native images; unused integration producer branches are removed explicitly by function-section garbage collection.','Static branch sites, actual object sizes and callback/copy counts are different observations; no timing or universal abstraction-cost claim.','Modern er_transfer comparison requires the original PR-T65-03 prerequisite, with first cause in a separate product field.','No external review, independent delayed learner assessment, physical observation or absolute mastery proof.']
    d=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,shapes=shapes,limits=limits);(HERE/'validation.json').write_text(json.dumps(d,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=d['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
