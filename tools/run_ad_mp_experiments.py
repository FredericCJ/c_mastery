"""Record exact diagnostic, generated-table and resource observations.

This author campaign does not assert independent learner assessment or close
the written delayed/composition experiments.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import tempfile
from datetime import datetime,timezone

ROOT=Path(__file__).resolve().parents[1]

def main():
    if os.name=='nt':raise SystemExit('Run under Linux/WSL with GCC and Clang.')
    records=[]
    paths=[Path(__file__)]
    for family in ['adt','metaprog']:
        for base in ['labs','assessor','fixtures']:
            paths.extend(p for p in (ROOT/base/family).rglob('*') if p.is_file())
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)}
    def run(command,expect=0,diagnostic=None):
        value=subprocess.run([str(v) for v in command],cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
        output=value.stdout+value.stderr
        passed=(value.returncode==expect if expect==0 else value.returncode!=0) and (diagnostic is None or re.search(diagnostic,output,re.I) is not None)
        records.append(dict(command=[str(v) for v in command],exit_code=value.returncode,stdout=value.stdout,stderr=value.stderr,expected='success' if expect==0 else 'named diagnostic',diagnostic=diagnostic,passed=passed))
        return value
    with tempfile.TemporaryDirectory(prefix='cm-admp-experiments-') as tmp:
        tmp=Path(tmp)
        for family,marker,cases in [
            ('adt','ADT_CHECK',['sums','quantities','flags','text','brands','views','arena','optics','evolution','packet']),
            ('metaprog','MP_CHECK',['mono','constraints','policies','variadic','generic','reflection','laws','formatting','hygiene','buildgraph'])]:
            for number in range(1,21):
                target=tmp/(family+'-mutant-'+str(number))
                if run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-DMOD_MUTATION='+str(number),'-I'+str(ROOT/'labs'/family/'include'),ROOT/'assessor'/family/(family+'.c'),ROOT/'labs'/family/'tests/tests.c','-o',target]).returncode==0:
                    run([target,cases[(number-1)//2]],1,marker)
        for compiler in ['gcc','clang']:
            run([compiler,'--version'])
            common=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only']
            run(common+['-Ilabs/adt/include','labs/adt/tests/probes/positive.c'])
            run(common+['-Wswitch-enum','-Ilabs/adt/include','labs/adt/tests/probes/missing_case.c'],1,'AD_PAIR.*not handled|enumeration value.*AD_PAIR')
            run(common+['-Ilabs/adt/include','labs/adt/tests/probes/wrong_quantity.c'],1,'incompatible|ADMillis.*ADSeconds')
            run(common+['-Ilabs/adt/include','labs/adt/tests/probes/wrong_brand.c'],1,'incompatible|ADNodeIndex.*ADBrand')
            run(common+['-Ilabs/metaprog/include','labs/metaprog/tests/probes/constant_capacity.c'])
            run(common+['-Ilabs/metaprog/include','-DPROBE_CAPACITY=3','labs/metaprog/tests/probes/constant_capacity.c'],1,'static assertion|static_assert')
            run(common+['-Ilabs/metaprog/include','labs/metaprog/tests/probes/wrong_argument.c'],1,'_Generic|generic association')
            run(common+['-Ilabs/metaprog/include','labs/metaprog/tests/probes/const_write.c'],1,'read-only|const-qualified')
            # An implementation comparison; unavailable final C23 text remains a reference gap.
            run([compiler,'-std=c2x','-Wall','-Wextra','-Wpedantic','-Werror','-fsyntax-only','labs/metaprog/tests/probes/vaopt_c23.c'])
            driver=tmp/(compiler+'-driver')
            compiled=run([compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Ilabs/metaprog/include','assessor/metaprog/metaprog.c','labs/metaprog/tests/oracle_driver.c','-o',driver])
            if compiled.returncode==0:
                emitted=subprocess.run([str(driver)],input='E\n',text=True,capture_output=True)
                records.append(dict(command=[str(driver)],stdin='E\n',exit_code=emitted.returncode,stdout=emitted.stdout,stderr=emitted.stderr,passed=emitted.returncode==0))
                generated=tmp/(compiler+'-generated.c');generated.write_text(emitted.stdout,encoding='utf-8',newline='\n')
                consumer=tmp/(compiler+'-consumer.c');consumer.write_text('#include <stdint.h>\nextern const uint64_t generated_keys[];\nint main(void){return !(generated_keys[0]==0 && generated_keys[1]==UINT64_C(0x123456789abcdef0) && generated_keys[2]==UINT64_MAX);}\n',encoding='utf-8',newline='\n')
                binary=tmp/(compiler+'-generated-test')
                if run([compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',generated,consumer,'-o',binary]).returncode==0:run([binary])
            for family in ['adt','metaprog']:
                obj=tmp/(compiler+'-'+family+'.o')
                run([compiler,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-fstack-usage','-ffreestanding','-fno-builtin','-I'+str(ROOT/'labs'/family/'include'),'-c',ROOT/'assessor'/family/(family+'.c'),'-o',obj])
                run(['size',obj]);run(['nm','-u',obj])
                stack=obj.with_suffix('.su')
                records.append(dict(observation='compiler stack-use report',compiler=compiler,family=family,path=str(stack),output=stack.read_text(encoding='utf-8') if stack.exists() else None,passed=stack.exists(),limit='Compiler static report, not measured maximum runtime stack on arbitrary targets.'))
        stable=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==sha for p,sha in pins.items())
    passed=stable and all(row['passed'] for row in records)
    report=dict(recorded_at=datetime.now(timezone.utc).isoformat(),passed=passed,checks=len(records),inspected_paths=pins,scope_source_stable=stable,executions=records,limitations=['Compiler diagnostics demonstrate these invocations, not arbitrary implementations.','C2x behavior does not close unavailable final C23 source inspection.','Resource reports and undefined-symbol observations do not establish target execution or timing.','Actual learner artifact composition, independent transfer and delayed assessments remain required.'])
    target=ROOT/'reviews/v5/expansion/b07/ad_mp_experiments.json';target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps(dict(passed=passed,checks=len(records),failed=[r for r in records if not r['passed']],report=target.relative_to(ROOT).as_posix())))
    return 0 if passed else 1

if __name__=='__main__':raise SystemExit(main())
