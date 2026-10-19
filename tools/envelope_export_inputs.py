"""Bind and verify the actual selected project frontmatter inputs after CLI init."""
import hashlib
import sys
from pathlib import Path


def bind_project_exports(root,pins,commands,errors):
    root=Path(root).resolve()
    sys.path.insert(0,str(root))
    from cmastery.project_assets import prepare
    pins.update(root/path for path in ['tools/envelope_export_inputs.py','cmastery/cli.py',
        'cmastery/assessments.py','cmastery/exercises.py','cmastery/learning.py',
        'cmastery/project_assets.py'])
    selected=[]
    for row in commands:
        command=row.get('command',[])
        if len(command)<6 or command[1:4]!=['-m','cmastery','init']:continue
        project,destination=command[4],Path(command[5])
        if row.get('returncode')!=0:continue
        brief=root/'curriculum/projects'/(project+'.md')
        pins.add(brief)
        assets=prepare(root,project)
        for asset in assets:
            pins.add(root/asset['path'])
            target=destination/asset['path']
            if not target.is_file() or hashlib.sha256(target.read_bytes()).hexdigest()!=asset['sha256']:
                errors.append('selected project asset missing/changed '+project+' '+asset['path'])
        selected.append(project)
    if not selected:errors.append('no successful canonical project export to bind')
    return selected
