"""Execute original X41/X42 changed mechanisms; preserve every attempt."""
from datetime import datetime,timezone
from pathlib import Path
import json,os,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/network_http_variants'
LAB=ROOT/'labs/network_http_variants';REF=ROOT/'assessor/network_http_variants'
sys.path.insert(0,str(ROOT/'labs/network_followthrough/ip'))
from checksum_slice import emit
from adapt_hosted_http import compose
sys.path.insert(0,str(ROOT/'labs/hosted_followthrough/http'))
import parser_cases
def main():
    files=[p for folder in [LAB,REF] for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [ROOT/p for p in ['tools/run_network_http_variants.py','tools/verify_network_http_variants.py','tools/common.py','tools/adapt_hosted_http.py','tools/adapt_http_depth_baseline.py','labs/network_followthrough/ip/checksum_slice.py','assessor/b045_systems/systems.c','labs/b045_systems/include/systems.h','labs/http_depth/include/http_depth.h','assessor/http_depth/http_depth.c','labs/hosted_followthrough/http/flow.h','assessor/hosted_followthrough/http/flow.c','labs/communications/include/com_http.h','assessor/communications/http.c','labs/hosted_followthrough/http/parser_cases.py','labs/hosted_followthrough/http/probe.c','labs/hosted_followthrough/http/tests.c']]
    before={p.relative_to(ROOT).as_posix():sha(p) for p in files}
    for rel in ['labs/hosted_followthrough/http/avr_start.S','labs/hosted_followthrough/http/run_avr.py']:
        before[rel]=sha(ROOT/rel)
    home=ROOT/'reviews/v5/expansion/network_http_variants'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');home.mkdir(parents=True)
    for rel in before:
        out=home/'source_snapshot'/rel;out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,out)
    executions=[];profiles=[];deps=[];external={};report=dict(passed=False,input_sha256=before,executions=executions,profiles=profiles,compiler_dependencies=deps,external_input_sha256=external,physical='not-run',independent_assessment='not-run')
    env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
    def run(cmd,label,code=0,marker=None,stdin=None):
        cmd=list(map(str,cmd));dep=None
        if cmd[0] in ['gcc','clang','arm-none-eabi-gcc'] and '-c' in cmd and code==0:dep=home/(label+'.d');cmd+=['-MD','-MF',str(dep)]
        p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,input=stdin,env=env,timeout=180)
        text=p.stdout+'\n'+p.stderr;log=home/(str(len(executions))+'-'+label+'.log');log.write_text(text,encoding='utf8')
        executions.append(dict(command=cmd,exit_code=p.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        assert p.returncode==code and (not marker or marker in text),text
        if code==1 and marker=='NV_CHECK:':assert not any(s in text for s in ['AddressSanitizer','runtime error:'])
        if dep:
            paths=[Path(p).resolve() for p in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];deps.append(dict(execution=len(executions)-1,paths=list(map(str,paths))))
            for dependency in paths:
                if dependency.is_relative_to(ROOT):
                    if not dependency.is_relative_to(home):assert dependency.relative_to(ROOT).as_posix() in before,'unbound '+str(dependency)
                else:external[str(dependency)]=sha(dependency)
        return p.stdout
    inc=['-I'+str(p) for p in [LAB,ROOT/'labs/b045_systems/include',ROOT/'labs/http_depth/include',ROOT/'labs/hosted_followthrough/http',ROOT/'labs/communications/include']]
    def build(cc,flags,label,sources,defines=(),links=()):
        objects=[]
        for i,source in enumerate(sources):
            obj=home/(label+'-'+str(i)+'.o');run([cc,*[f for f in flags if f!='-no-pie'],*inc,*defines,'-c',source,'-o',obj],label+'-compile'+str(i));objects.append(obj)
        exe=home/label;run([cc,*flags,*objects,*links,'-Wl,--gc-sections','-o',exe],label+'-link');return exe,objects
    try:
        for tool in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','nm','objdump','size']:
            p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);run([tool,'--version'],tool+'-version')
        checksum=home/'checksum.c';report['checksum_composition']=emit(ROOT/'assessor/b045_systems/systems.c',checksum)
        parser=home/'parser.c';report['parser_composition']=compose(parser)
        expected,wire=parser_cases.batch();(home/'parser_inputs.txt').write_text(wire,encoding='utf8');dump(home/'parser_expected.json',expected)
        for cc in ['gcc','clang']:
            for name,extra in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                label=cc+'-'+name;flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-ffunction-sections','-fdata-sections',*extra]
                sources=[LAB/'tests.c',REF/'variant.c',checksum,ROOT/'assessor/http_depth/http_depth.c']
                exe,objs=build(cc,flags,label,sources)
                anchors={case:run([exe,case],label+'-'+case,marker='NV_OK:'+case) for case in ['checksum','length','date']}
                probe,_=build(cc,flags,label+'-probe',[LAB/'probe.c',REF/'variant.c',checksum])
                oracle=json.loads(run([sys.executable,ROOT/'tools/verify_network_http_variants.py',probe],label+'-oracle'))
                owned,_=build(cc,flags,label+'-owned',[LAB/'owned_tests.c',REF/'owned.c',ROOT/'assessor/hosted_followthrough/http/flow.c'],links=['-pthread'])
                anchors['owned']=run([owned],label+'-owned',marker='NV_OK:owned')
                controls=[]
                for mutation,case in [(1,'checksum'),(2,'checksum'),(3,'length'),(4,'length'),(5,'date'),(6,'date'),(7,'owned'),(8,'owned')]:
                    chosen=[LAB/'owned_tests.c',REF/'owned.c',ROOT/'assessor/hosted_followthrough/http/flow.c'] if case=='owned' else sources
                    mutant,_=build(cc,flags,label+'-mutant'+str(mutation),chosen,['-DNV_MUTATION='+str(mutation)],['-pthread'] if case=='owned' else [])
                    run([mutant]+([] if case=='owned' else [case]),label+'-reject'+str(mutation),1,'NV_CHECK:');controls.append(dict(mutation=mutation,case=case))
                # Re-execute retained concrete parser corpus and serialized
                # callback/date serializer rather than crediting their headings.
                common=[parser,ROOT/'assessor/http_depth/http_depth.c',ROOT/'assessor/hosted_followthrough/http/flow.c']
                old,_=build(cc,flags,label+'-retained',[ROOT/'labs/hosted_followthrough/http/tests.c',*common]);run([old],label+'-retained',marker='HF_PASS:all')
                parse,_=build(cc,flags,label+'-parser',[ROOT/'labs/hosted_followthrough/http/probe.c',*common]);text=run([parse],label+'-parser',stdin=wire)
                groups=parser_cases.verify(expected,[json.loads(line) for line in text.splitlines()]);assert len(groups)==144
                framing,_=build(cc,flags,label+'-framing',[LAB/'framing.c',REF/'variant.c',checksum,*common]);anchors['framing']=run([framing],label+'-framing',marker='NV_OK:framing:')
                cost,_=build(cc,flags,label+'-cost',[LAB/'cost.c',REF/'variant.c',checksum,ROOT/'assessor/http_depth/http_depth.c']);anchors['cost']=run([cost],label+'-cost',marker='NV_OK:cost:')
                run(['objdump','-dr',objs[1]],label+'-assembly');run(['size',exe],label+'-size')
                profiles.append(dict(profile=label,anchors=anchors,oracle=oracle,controls=controls,retained_parser_cases=len(groups),retained_parser_partitions=len(expected)));print(label+' passed',flush=True)
            flags=['-std=c17','-O0','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections']
            stub,_=build(cc,flags,cc+'-stub',[LAB/'tests.c',LAB/'variant.c',checksum,ROOT/'assessor/http_depth/http_depth.c'])
            for case in ['checksum','length','date']:run([stub,case],cc+'-stub-'+case,1,'NV_CHECK:')
            stub,_=build(cc,flags,cc+'-owned-stub',[LAB/'owned_tests.c',LAB/'owned.c',ROOT/'assessor/hosted_followthrough/http/flow.c'],links=['-pthread']);run([stub],cc+'-owned-stub',1,'NV_CHECK:')
            flags=['-std=c17','-O1','-g','-Wall','-Wextra','-Werror','-ffunction-sections','-fdata-sections','-fsanitize=thread','-fno-pie','-no-pie']
            tsan,_=build(cc,flags,cc+'-tsan',[LAB/'owned_tests.c',REF/'owned.c',ROOT/'assessor/hosted_followthrough/http/flow.c'],links=['-pthread']);run([tsan],cc+'-tsan',marker='NV_OK:owned')
        avr=Path('/home/frede/cmastery-http-avr/root/usr');avrcc=avr/'bin/avr-gcc'
        for tool in [avrcc,avr/'bin/avr-nm',avr/'bin/avr-size',Path('/home/frede/cmastery-b06-tools/usr/bin/qemu-system-avr')]:external[str(tool.resolve())]=sha(tool);run([tool,'--version'],'avr-version-'+tool.name)
        elf=home/'avr.elf';run([avrcc,'-mmcu=atmega328p','-std=c17','-Os','-ffreestanding','-ffunction-sections','-fdata-sections','-fno-builtin','-nostdlib','-Wall','-Wextra','-Werror',*inc,ROOT/'labs/hosted_followthrough/http/avr_start.S',LAB/'avr.c',REF/'variant.c',ROOT/'assessor/http_depth/http_depth.c','-Wl,--gc-sections,-e,_start','-lgcc','-o',elf],'avr-build')
        assert not run([avr/'bin/avr-nm','-u',elf],'avr-undefined').strip();run([avr/'bin/avr-nm',elf],'avr-symbols');run([avr/'bin/avr-size','-A',elf],'avr-size')
        report['avr']=run([sys.executable,ROOT/'labs/hosted_followthrough/http/run_avr.py','--binary',elf],'avr-run',marker='AVR_LENGTH_PASS:')
        # Complete allocation-free component link: no GC can hide helpers.
        objs=[]
        for i,source in enumerate([REF/'variant.c',checksum]):
            obj=home/('arm'+str(i)+'.o');run(['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin',*inc,'-c',source,'-o',obj],'arm-compile'+str(i));objs.append(obj)
        image=home/'arm.elf';run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nodefaultlibs','-nostartfiles',*objs,'-Wl,--no-undefined,-e,nv_sum,-Ttext=0x08000000','-o',image],'arm-link')
        assert not run(['arm-none-eabi-nm','-u',image],'arm-undefined').strip();run(['arm-none-eabi-nm','--defined-only',image],'arm-definitions')
        report['target']=dict(profile='Cortex-M4-O2',executed=False,section_gc=False,undefined=[])
        assert all(sha(ROOT/p)==h for p,h in before.items());assert all(sha(Path(p))==h for p,h in external.items())
        report.update(passed=True,source_stable=True)
    except Exception as e:report['failure']=repr(e)
    report['artifact_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in home.rglob('*') if p.is_file()}
    dump(home/'report.json',report);dump(ROOT/BASE/'validation.json',report);print(json.dumps(dict(passed=report['passed'],commands=len(executions),failure=report.get('failure'))));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
