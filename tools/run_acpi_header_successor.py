"""Execute a bounded source-comparison regression; leave original campaign intact."""
import json,os,subprocess,tempfile
from pathlib import Path
from common import ROOT,dump,sha
BASE='reviews/v5/expansion/acpi_header_successor'
def main():
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_acpi_header_successor.py',platform='wsl'),cwd=ROOT).returncode
    folder=ROOT/BASE;folder.mkdir(parents=True,exist_ok=True)
    files=[ROOT/'assessor/legacy_smp/acpi.c',ROOT/'assessor/legacy_smp/acpi_header_successor.c',ROOT/'labs/source_readings/acpi_header_66.md',Path(__file__)]
    files+=list((ROOT/'labs/smp_depth/acpi').glob('*.h'));pins={p.relative_to(ROOT).as_posix():sha(p) for p in files};commands=[]
    with tempfile.TemporaryDirectory(prefix='acpi-source-comparison-') as tmp:
        for compiler in ['gcc','clang']:
            binary=Path(tmp)/compiler
            for label,argv in [('build',[compiler,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic-errors','-I',str(ROOT/'labs/smp_depth/acpi'),str(ROOT/'assessor/legacy_smp/acpi.c'),str(ROOT/'assessor/legacy_smp/acpi_header_successor.c'),'-o',str(binary)]),('run',[str(binary)])]:
                p=subprocess.run(argv,capture_output=True,text=True,timeout=60);log=folder/(compiler+'-'+label+'.log');log.write_text(p.stdout+'\n'+p.stderr)
                passed=p.returncode==0 and(label=='build' or 'SA_HEADER_SUCCESSOR:' in p.stdout)
                commands.append(dict(command=argv,exit_code=p.returncode,passed=passed,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
    result=dict(passed=all(c['passed'] for c in commands) and all(sha(ROOT/n)==h for n,h in pins.items()),commands=commands,source_sha256=pins,scope='Actual unchanged parser plus original strict-profile source-comparison assertions. Existing root/MADT checks also execute; no new guest/physical or full-conformance claim.')
    dump(folder/'validation.json',result);print(json.dumps(result));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
