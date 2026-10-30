"""Execute exact original B01 verification additions without changing providers."""
from datetime import datetime,timezone
from pathlib import Path
import json,os,re,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
from c_source import replace_unique
BASE='research/v5/expansion/b01_followthrough'
REPORT=BASE+'/validation.json'
PUBLIC=ROOT/'labs/foundation_followthrough'
PRIVATE=ROOT/'assessor/foundation_followthrough'
PROVIDER=ROOT/'assessor/foundation_depth/depth.c'

def main():
    paths=[p for folder in [PUBLIC,PRIVATE] for p in folder.rglob('*') if p.is_file()]
    paths += [PROVIDER,ROOT/'labs/foundation_depth/include/depth.h',ROOT/'labs/foundation_depth/CONTRACTS.md',ROOT/'assessor/foundation_depth/transfer/comparisons.c',ROOT/'assessor/foundation_depth/transfer/unrestricted.c',ROOT/'labs/smp_depth/acpi/big_endian.S',ROOT/'tools/run_foundation_followthrough.py',ROOT/'tools/verify_foundation_followthrough.py',ROOT/'tools/common.py']
    paths += [ROOT/p for p in ['labs/foundation_depth/tests/tests.c','labs/foundation_depth/tests/oracle_driver.c','assessor/foundation_depth/semantic_answers.inc','tools/verify_foundation_depth_fixtures.py']]
    paths += [ROOT/'tools/verify_foundation_defense.py']
    paths += [p for p in (ROOT/'fixtures/foundation_depth').rglob('*') if p.is_file()]
    inputs={p.relative_to(ROOT).as_posix():sha(p) for p in paths}
    home=ROOT/'reviews/v5/expansion/b01_followthrough'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');home.mkdir(parents=True)
    for rel in inputs:
        target=home/'source_snapshot'/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,target)
    records=[];external={};dependencies=[];profiles=[];comparisons=[];targets=[];diagnostics=[];runtime=[]
    report=dict(schema_version=1,passed=False,status='failed',input_sha256=inputs,executions=records,profiles=profiles,comparisons=comparisons,targets=targets,diagnostics=diagnostics,external_input_sha256=external,compiler_dependencies=dependencies,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed')
    env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
    def execute(cmd,label,expected=0,marker=None):
        cmd=list(map(str,cmd));dep=None
        if cmd[0] in ['gcc','clang','arm-none-eabi-gcc'] and '-c' in cmd and expected==0:
            dep=home/(str(len(records))+'-'+label+'.d');cmd+=['-MD','-MF',str(dep)]
        p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180,env=env);text=p.stdout+'\n'+p.stderr
        log=home/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8')
        records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        assert p.returncode==expected and (marker is None or marker in text),text
        if marker=='FF_CHECK:':assert not any(s in text for s in ['AddressSanitizer','runtime error:'])
        if dep:
            actual=[Path(q).resolve() for q in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])]
            dependencies.append(dict(execution=len(records)-1,paths=[str(q) for q in actual]))
            for q in actual:
                if q.is_relative_to(ROOT):
                    if not q.is_relative_to(home):assert q.relative_to(ROOT).as_posix() in inputs,'unbound source '+str(q)
                else:external[str(q)]=sha(q)
        return p.stdout,p.stderr
    includes=['-I'+str(PUBLIC),'-I'+str(ROOT/'labs/foundation_depth/include')]
    def build(cc,flags,label,sources,defines=(),link=()):
        objects=[]
        for i,source in enumerate(sources):
            compile_flags=[v for v in flags if v!='-no-pie']
            obj=home/(label+'-'+str(i)+'.o');execute([cc,*compile_flags,*includes,*defines,'-c',source,'-o',obj],label+'-compile'+str(i));objects.append(obj)
        exe=home/label;execute([cc,*flags,*objects,*link,'-o',exe],label+'-link');return exe,objects
    try:
        for tool in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','nm','objdump','readelf','size','ldd']:
            path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
        external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve());execute([sys.executable,'--version'],'python-version')
        for cc in ['gcc','clang']:
            for mode,extra in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                label=cc+'-'+mode;flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*extra]
                exe,objects=build(cc,flags,label,[PUBLIC/'tests.c',PRIVATE/'follow.c',PROVIDER]);runtime.append(exe)
                anchors={}
                for case in ['interval','copy','codec','partition','lifecycle']:
                    out,_=execute([exe,case],label+'-'+case,marker='FF_OK:'+case+':');anchors[case]=out
                hist,_=build(cc,flags,label+'-histories',[PUBLIC/'history_driver.c',PROVIDER]);runtime.append(hist)
                out,_=execute([sys.executable,ROOT/'tools/verify_foundation_followthrough.py',hist],label+'-oracle');oracle=json.loads(out)
                original,_=build(cc,flags,label+'-original',[ROOT/'labs/foundation_depth/tests/tests.c',PROVIDER],['-DFD_ASSESSOR_TESTS','-I'+str(ROOT/'assessor/foundation_depth')]);runtime.append(original)
                original_anchors=['semantic','extent','cursor','codec','lifecycle','overlap','handles','defense']
                for case in original_anchors:execute([original,case],label+'-original-'+case)
                driver,_=build(cc,flags,label+'-original-oracle',[ROOT/'labs/foundation_depth/tests/oracle_driver.c',PROVIDER]);runtime.append(driver)
                out,_=execute([sys.executable,ROOT/'tools/verify_foundation_depth_fixtures.py','--driver',driver],label+'-original-oracle-run');original_oracle=json.loads(out)
                defended,_=build(cc,flags,label+'-defense',[PUBLIC/'defense_driver.c',PROVIDER]);runtime.append(defended)
                out,_=execute([sys.executable,ROOT/'tools/verify_foundation_defense.py',defended],label+'-defense-oracle');defense_oracle=json.loads(out)
                controls=[]
                for mutation,case in [(1,'interval'),(2,'interval'),(3,'copy'),(4,'codec')]:
                    mutant,_=build(cc,flags,label+'-mutant'+str(mutation),[PUBLIC/'tests.c',PRIVATE/'follow.c',PROVIDER],['-DFF_MUTATION='+str(mutation)]);runtime.append(mutant)
                    if mutation==4:execute([mutant,'roundtrip'],label+'-shared-defect-roundtrip',marker='FF_OK:roundtrip:256')
                    execute([mutant,case],label+'-mutant-reject'+str(mutation),1,'FF_CHECK:');controls.append(dict(mutation=mutation,case=case,semantic_rejection=True))
                # Exact generated source substitutions are retained and pinned;
                # canonical provider bytes are never edited.
                original=PROVIDER.read_text(encoding='utf8')
                patches=[('premature','if (slot->state != FD_INITIALIZED && slot->state != FD_PUBLISHED) return FD_STATE;','if (slot->state == FD_EMPTY) return FD_STATE;'),('failed-destructor','if (initialize(context,slot->storage)) {','if (initialize(context,slot->storage)) {\n        if (destroy) destroy(context,slot->storage);')]
                for name,old,new in patches:
                    source=home/(label+'-'+name+'.c');source.write_text(replace_unique(original,old,new),encoding='utf8')
                    mutant,_=build(cc,flags,label+'-'+name,[PUBLIC/'tests.c',PRIVATE/'follow.c',source]);runtime.append(mutant)
                    execute([mutant,'lifecycle'],label+'-'+name+'-reject',1,'FF_CHECK:');controls.append(dict(mutation=name,case='lifecycle',semantic_rejection=True,generated_provider=source.relative_to(ROOT).as_posix()))
                wrong,_=build(cc,flags,label+'-wrap',[PUBLIC/'history_driver.c',PROVIDER],['-DFD_MUTATE_HANDLES']);runtime.append(wrong)
                execute([sys.executable,ROOT/'tools/verify_foundation_followthrough.py',wrong],label+'-wrap-oracle-reject',1,'AssertionError:');controls.append(dict(mutation='generation-wrap',case='histories',semantic_rejection=True))
                for macro in ['FD_MUTATE_DEFENSE','FD_MUTATE_FLAG_AUTHORITY']:
                    mutant,_=build(cc,flags,label+'-'+macro,[PUBLIC/'defense_driver.c',PROVIDER],['-D'+macro]);runtime.append(mutant)
                    execute([sys.executable,ROOT/'tools/verify_foundation_defense.py',mutant],label+'-'+macro+'-oracle-reject',1,'AssertionError:');controls.append(dict(mutation=macro,case='defense',semantic_rejection=True))
                execute(['objdump','-dr',objects[1]],label+'-assembly');execute(['size',exe],label+'-size')
                profiles.append(dict(profile=label,anchors=anchors,oracle=oracle,controls=controls,original_anchors=original_anchors,original_oracle=original_oracle,defense_oracle=defense_oracle));print(label+' passed',flush=True)
            flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O0']
            learner,_=build(cc,flags,cc+'-learner',[PUBLIC/'tests.c',PUBLIC/'follow.c',PROVIDER]);runtime.append(learner)
            for case in ['interval','copy','codec']:execute([learner,case],cc+'-learner-'+case,1,'FF_CHECK:')
            for opt in ['O0','O2','O3']:
                label=cc+'-'+opt;flags=['-std=c17','-'+opt,'-Wall','-Wextra','-Wpedantic','-Werror']
                compare,objects=build(cc,flags,label+'-compare',[PROVIDER,ROOT/'assessor/foundation_depth/transfer/comparisons.c',ROOT/'assessor/foundation_depth/transfer/unrestricted.c']);runtime.append(compare)
                out,_=execute([compare],label+'-compare-run',marker='FD_COMPARE_OK:65542');execute(['objdump','-d',compare],label+'-compare-assembly');execute(['size',compare],label+'-compare-size')
                comparisons.append(dict(profile=label,samples=[s for s in out.splitlines() if s.startswith('FD_CPU_SAMPLE')],result=out.splitlines()[-1]))
            for case in [1,2]:
                label=cc+'-constraint'+str(case);out,err=execute([cc,'-std=c17','-pedantic-errors','-DCASE='+str(case),'-fsyntax-only',PUBLIC/'semantic_probe.c'],label+'-strict',1);assert err.strip()
                if case==1:
                    obj=home/(label+'-continued.o');_,err=execute([cc,'-std=c17','-pedantic','-Wno-error=int-conversion','-DCASE=1','-c',PUBLIC/'semantic_probe.c','-o',obj],label+'-continued');assert err.strip()
                diagnostics.append(dict(compiler=cc,case=case,diagnostic=True,continued_translation=case==1,executed=False))
            exe,_=build(cc,['-std=c17','-O2','-Wall','-Wextra','-Werror'],cc+'-defined-semantics',[PUBLIC/'semantic_probe.c']);runtime.append(exe)
            out,_=execute([exe],cc+'-defined-semantics-run');assert out in ['distinct=6,2,4 shared=5,4\n','distinct=6,2,4 shared=4,3\n']
        # Actual ABI/endian change, selected standalone image; no libc helpers.
        qemu=ROOT/'build/checkpoint-tools/qemu-user-local/usr/bin/qemu-ppc64';lld=ROOT/'build/b06-tools/root/usr/bin/ld.lld-21'
        for tool in [qemu,lld]:external[str(tool)]=sha(tool);execute([tool,'--version'],'target-tool-version')
        for opt in ['O0','O2']:
            flags=['--target=powerpc64-linux-gnu','-mabi=elfv2','-std=c17','-'+opt,'-ffreestanding','-ffunction-sections','-fdata-sections','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror']
            exe,objects=build('clang',flags,'ppc64-'+opt,[PUBLIC/'big_endian.c',PRIVATE/'follow.c',PROVIDER],link=['-nostdlib','-static','--ld-path='+str(lld),ROOT/'labs/smp_depth/acpi/big_endian.S','-Wl,--gc-sections,-e,_start'])
            h,_=execute(['readelf','-h',exe],'ppc64-'+opt+'-identity');u,_=execute(['nm','-u',exe],'ppc64-'+opt+'-undefined');d,_=execute(['readelf','-d',exe],'ppc64-'+opt+'-dynamic')
            assert 'big endian' in h and 'PowerPC64' in h and 'abiv2' in h and not u.strip() and 'NEEDED' not in d
            out,_=execute([qemu,exe],'ppc64-'+opt+'-run',marker='FF_BE_PASS');targets.append(dict(profile='ppc64-'+opt,executed=True,undefined=[],needed=[],observation=out))
        # Full component M4 link includes all functions; no section GC hides helpers.
        flags=['-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fno-unwind-tables','-fno-asynchronous-unwind-tables']
        exe,objects=build('arm-none-eabi-gcc',flags,'m4-closure',[PRIVATE/'follow.c',PROVIDER],link=['-nostdlib','-nodefaultlibs','-nostartfiles','-Wl,--no-undefined,-e,ff_interval,-Ttext=0x08000000'])
        u,_=execute(['arm-none-eabi-nm','-u',exe],'m4-undefined');assert not u.strip();symbols,_=execute(['arm-none-eabi-nm','--defined-only',exe],'m4-symbols');assert not re.search(r'\b(?:memcpy|memset|memmove|__aeabi_\w+)\b',symbols)
        targets.append(dict(profile='cortex-m4-O2',executed=False,undefined=[],needed=[],no_section_gc=True))
        for i,exe in enumerate(runtime):
            out,_=execute(['ldd',exe],'runtime'+str(i))
            for value in re.findall(r'(/[^\s()]+)',out):
                path=Path(value).resolve()
                if path.is_file():external[str(path)]=sha(path)
        report.update(source_stable=all(sha(ROOT/p)==h for p,h in inputs.items()),external_inputs_stable=all(sha(Path(p))==h for p,h in external.items()))
        assert report['source_stable'] and report['external_inputs_stable'];report.update(passed=True,status='passed')
    except Exception as error:
        report['failure']=repr(error)
    report['generated_artifacts']=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
    dump(home/'report.json',report);dump(ROOT/REPORT,report)
    print(json.dumps(dict(passed=report['passed'],commands=len(records),failure=report.get('failure'),report=REPORT)))
    return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
