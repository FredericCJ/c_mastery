"""Execute all checkpoint standalone exports and bounded core object dependency audits."""
import argparse
from contextlib import nullcontext
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, project_assets
from check_exercise_integration import check_exported_stub

def main(report):
    root = cli.ROOT
    destination = (root/report).resolve()
    if not destination.is_relative_to(root/'reviews/v5/expansion/legacy_completion'):
        raise ValueError('Choose a new legacy_completion report, never historical evidence')
    paths = [Path(__file__).resolve(), root/'tools/check_exercise_integration.py']
    paths += [root/'cmastery'/name for name in ('cli.py','project_assets.py','exercises.py','assessments.py','learning.py')]
    paths += [p for name in ('labs/checkpoint_depth', 'assessor/checkpoint_depth')
              for p in (root/name).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    pins = {p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    projects = [f'PR-P{phase}-{part:02d}' for phase in range(1,7) for part in (1,2)]
    declared = {project:project_assets.prepare(root,project) for project in projects}
    for project in projects:
        for name in ['curriculum/projects/'+project+'.md','curriculum/v5/expansion/legacy_assessments/'+project+'.json',
                     *[row['path'] for row in declared[project]]]:
            pins[name]=hashlib.sha256((root/name).read_bytes()).hexdigest()
    observations = []; outputs = {}
    # Retain this new run's raw workspace and object artifacts; old runs stay intact.
    with nullcontext(tempfile.mkdtemp(prefix='checkpoint-reference-export-',dir=root/'build')) as temporary:
        tmp = Path(temporary)
        for project in projects:
            workspace = tmp/project
            initialized=cli.instantiate(project, workspace, family_name='checkpoint_depth')
            if initialized.get('project_assets',[])!=declared[project]:raise ValueError('Project asset declaration differs: '+project)
            for asset in declared[project]:
                exported=workspace/asset['path'];actual=hashlib.sha256(exported.read_bytes()).hexdigest()
                if actual!=asset['sha256']:raise ValueError('Project asset copy differs: '+asset['path'])
                outputs[exported.relative_to(root).as_posix()]=actual
            files = list(workspace.rglob('*'))
            leaked = [p for p in files if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
            exact_stub = all((workspace/'src'/p.name).read_bytes()==p.read_bytes() for p in (root/'labs/checkpoint_depth/src').glob('*.c'))
            result = check_exported_stub(workspace, tmp)
            observations.append({'kind':'standalone-export','project':project,
                'passed':not leaked and exact_stub and result['status']=='passed',
                'learner_stub_unchanged':exact_stub,'assessor_files':len(leaked),'execution':result,
                'workspace':workspace.relative_to(root).as_posix(),'project_assets':declared[project]})
        for compiler,source in ((cc,name) for cc in ('gcc','clang') for name in ('checkpoint_depth.c','checkpoint_memory.c')):
            output = tmp/(compiler+'-'+source+'.o')
            command = [compiler,'-std=c17','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector',
                       '-fno-pie','-I'+str(root/'labs/checkpoint_depth/include'),'-c',
                       str(root/'assessor/checkpoint_depth'/source),'-o',str(output)]
            build = subprocess.run(command,capture_output=True,text=True)
            undefined = subprocess.run(['nm','-u',str(output)],capture_output=True,text=True) if build.returncode==0 else None
            version = subprocess.run([compiler,'--version'],capture_output=True,text=True)
            target = subprocess.run([compiler,'-dumpmachine'],capture_output=True,text=True)
            observations.append({'kind':'freestanding-object','source':source,'compiler':compiler,'version':version.stdout.splitlines()[0],
                'target':target.stdout.strip(),'command':command,'compile_exit':build.returncode,
                'compile_output':build.stdout+build.stderr,'undefined_exit':None if undefined is None else undefined.returncode,
                'undefined_symbols':None if undefined is None else undefined.stdout,
                'passed':build.returncode==0 and undefined is not None and undefined.returncode==0 and not undefined.stdout.strip(),
                'scope':'Selected core object only; no target execution, full image/startup or physical evidence.'})
    stable = all(hashlib.sha256((root/p).read_bytes()).hexdigest()==value for p,value in pins.items())
    result = {'schema_version':1,'recorded_at':datetime.now(timezone.utc).isoformat(),
        'passed':stable and all(o['passed'] for o in observations),'source_stable':stable,
        'inspected_paths':pins,'outputs_sha256':outputs,'checks':observations,'learner_mastery':'not-assessed','physical_validation':'not-run'}
    destination.parent.mkdir(parents=True,exist_ok=True)
    destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'checks':len(observations),'report':report}))
    return 0 if result['passed'] else 1

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
