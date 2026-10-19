"""Actual changed process inputs, per-object ELF comparison and strict final-image closure."""
import datetime,hashlib,json,pathlib,shutil,struct,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_checkpoint_entry';PUBLIC=ROOT/'labs/checkpoint_followthrough/entry';PRIVATE=ROOT/'assessor/checkpoint_followthrough/entry';BUILD=ROOT/'build'/('checkpoint-entry-followthrough-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PRODUCER=['assessor/checkpoint_depth/checkpoint_writer.c','labs/checkpoint_depth/include/checkpoint_writer.h','assessor/checkpoint_depth/targets/entry.S','assessor/checkpoint_depth/targets/byte_helpers.c','labs/checkpoint_depth/tests/helper_probe.c','labs/checkpoint_depth/tests/resolved_forbidden.c']
TRACER=ROOT/'build/checkpoint-tools/strace-local/usr/bin/strace';LIBRARY=ROOT/'build/checkpoint-tools/strace-local/usr/lib/x86_64-linux-gnu'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted([p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[ROOT/n for n in PRODUCER]])}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','readelf','objdump','nm']}|{TRACER,*[p for p in LIBRARY.glob('*.so*') if p.is_file()]})}
def expected_commands():
    s={'kernel','tracer.version'}
    for cc in ['gcc','clang']:
        s|={cc+'.version'}
        for opt in ['O0','O3']:
            for shape in ['fixed','pie']:
                t=cc+'.'+opt+'.'+shape
                for unit in ['entry','image','writer','helpers','aggregate']:s|={t+'.'+unit+'.compile',t+'.'+unit+'.object'}
                s|={t+'.'+n for n in ['link','audit','readelf','assembly','execute','missing-helper','forbidden.compile','forbidden.link','forbidden.audit','bad-entry.audit','bad-extent.audit','learner.compile','learner.link','learner.reject']}
                if shape=='pie':s|={t+'.relocation.compile',t+'.relocation.link',t+'.relocation.audit'}
    return s
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None
    try:
        run('kernel',['uname','-a']);run('tracer.version',['env','LD_LIBRARY_PATH='+str(LIBRARY),TRACER,'--version'])
        for cc in ['gcc','clang']:
            run(cc+'.version',[cc,'--version'])
            for opt in ['O0','O3']:
                for shape in ['fixed','pie']:
                    t=cc+'.'+opt+'.'+shape;pie=shape=='pie';flags=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-'+opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fPIE' if pie else '-fno-pie','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/checkpoint_depth/include')];objects={}
                    sources={'entry':ROOT/'assessor/checkpoint_depth/targets/entry.S','image':PRIVATE/'image.c','writer':ROOT/'assessor/checkpoint_depth/checkpoint_writer.c','helpers':ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c','aggregate':ROOT/'labs/checkpoint_depth/tests/helper_probe.c'}
                    for name,source in sources.items():
                        obj=BUILD/(t+'.'+name+'.o');objects[name]=obj;compile_flags=['-fPIE' if pie else '-fno-pie'] if name=='entry' else flags;run(t+'.'+name+'.compile',[cc,*compile_flags,'-c',source,'-o',obj]);run(t+'.'+name+'.object',[sys.executable,PUBLIC/'elf_audit.py',obj,BUILD/(t+'.'+name+'.object.json'),'object'],marker='CE_ELF_PASS')
                    link=[cc,'-nostdlib','-nostartfiles','-nodefaultlibs',*(['-static-pie'] if pie else ['-static','-no-pie']),'-Wl,-e,_start,-z,noexecstack,--no-dynamic-linker'];image=BUILD/t
                    run(t+'.link',[*link,*objects.values(),'-Wl,-Map,'+str(BUILD/(t+'.map')),'-o',image]);run(t+'.audit',[sys.executable,PUBLIC/'elf_audit.py',image,BUILD/(t+'.elf.json')],marker='CE_ELF_PASS');run(t+'.readelf',['readelf','-aW',image],marker='Program Headers:');run(t+'.assembly',['objdump','-d',image],marker='<_start>:');run(t+'.execute',[sys.executable,PUBLIC/'check_image.py',image,TRACER,LIBRARY,BUILD/(t+'.execution.json')],marker='"cases": 8')
                    run(t+'.missing-helper',[*link,*[p for n,p in objects.items() if n!='helpers'],'-o',BUILD/(t+'.missing')],1,'memcpy')
                    badobj=BUILD/(t+'.forbidden.o');run(t+'.forbidden.compile',[cc,*flags,'-c',ROOT/'labs/checkpoint_depth/tests/resolved_forbidden.c','-o',badobj]);bad=BUILD/(t+'.forbidden');run(t+'.forbidden.link',[*link,*objects.values(),badobj,'-o',bad]);run(t+'.forbidden.audit',[sys.executable,PUBLIC/'elf_audit.py',bad,BUILD/(t+'.unused.json')],41,'CE_AUDIT:forbidden resolved provider')
                    content=bytearray(image.read_bytes());struct.pack_into('<Q',content,24,0x123);bad=BUILD/(t+'.bad-entry');bad.write_bytes(content);run(t+'.bad-entry.audit',[sys.executable,PUBLIC/'elf_audit.py',bad,BUILD/(t+'.unused.json')],41,'CE_AUDIT:entry outside executable PT_LOAD')
                    content=bytearray(image.read_bytes());eh=struct.unpack_from('<16sHHIQQQIHHHHHH',content)
                    for i in range(eh[10]):
                        at=eh[5]+i*eh[9];ph=struct.unpack_from('<IIQQQQQQ',content,at)
                        if ph[0]==1 and ph[5]:struct.pack_into('<Q',content,at+40,ph[5]-1);break
                    bad=BUILD/(t+'.bad-extent');bad.write_bytes(content);run(t+'.bad-extent.audit',[sys.executable,PUBLIC/'elf_audit.py',bad,BUILD/(t+'.unused.json')],41,'CE_AUDIT:invalid load extent or writable executable')
                    stubobj=BUILD/(t+'.learner.o');run(t+'.learner.compile',[cc,*flags,'-c',PUBLIC/'learner.c','-o',stubobj]);stub=BUILD/(t+'.learner');run(t+'.learner.link',[*link,objects['entry'],stubobj,'-o',stub]);run(t+'.learner.reject',[stub],93)
                    if pie:
                        obj=BUILD/(t+'.relocation.o');run(t+'.relocation.compile',[cc,*flags,'-DCE_BAD_RELOCATION','-c',PRIVATE/'image.c','-o',obj]);bad=BUILD/(t+'.relocation');run(t+'.relocation.link',[*link,*[p for n,p in objects.items() if n!='image'],obj,'-o',bad]);run(t+'.relocation.audit',[sys.executable,PUBLIC/'elf_audit.py',bad,BUILD/(t+'.unused.json')],41,'CE_AUDIT:unhandled relocation')
        assert {q['id'] for q in h.COMMANDS}==expected_commands()
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext:failure='Scoped source/tool identity changed'
    limits=['Actual local x86-64 Linux fixed and relocation-free PIE children; C17/Profile D with explicit original helpers and CPWTransfer producer.','Changed argc, payload arguments and environment are consumed; no target ABI switch is claimed for the disjunctive delayed task.','Objects and final images are inspected separately; no dynamic relocation engine, libc startup, TLS or constructor support is supplied.','Tagged Linux source and historical AMD64 draft are bounded authorities, not the running kernel build provenance.','No external review, delayed learner attempt, physical device observation or finite-test proof of mastery.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
