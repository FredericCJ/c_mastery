"""Bind actual selected exercise/project exports while their workspace is live.

The whole catalog is checked for mid-run changes, then replaced by the exact
canonical selected brief and all bound inputs. This is a dependency correction,
not permission to re-date an old execution or ignore a changed selected contract.
"""
import json
import sys
from pathlib import Path
from envelope_export_inputs import bind_project_exports
from common import sha

def bind_completed_exports(root,before,records):
    root=Path(root)
    for path,digest in before.items():
        assert sha(root/path)==digest, ('changed during export',path)
    sys.path.insert(0,str(root))
    from cmastery.exercises import parse_brief
    commands=[dict(row,returncode=row.get('returncode',row.get('exit_code'))) for row in records]
    pins={root/p for p in before};errors=[]
    bind_project_exports(root,pins,commands,errors)
    assert not errors,errors
    selected=[]
    for row in commands:
        cmd=row.get('command',[])
        if len(cmd)<6 or cmd[1:4]!=['-m','cmastery','init'] or row['returncode']!=0 or '--exercise' not in cmd:continue
        ident=cmd[cmd.index('--exercise')+1];brief=root/'curriculum/v5/exercises'/f'{ident}.md'
        meta,_=parse_brief(brief)
        catalog=json.loads((root/'curriculum/v5/exercises.json').read_text(encoding='utf8'))
        entries=[r for r in catalog['exercises'] if r['id']==ident]
        assert len(entries)==1,('selected exercise missing/duplicate',ident)
        entry=entries[0]
        assert entry['brief_sha256']==sha(brief) and all(entry[k]==v for k,v in meta.items()),('selected canonical mismatch',ident)
        assert meta['project']==cmd[4],('selected project mismatch',ident)
        text=''.join((Path(cmd[5])/n).read_text(encoding='utf8') for n in ('EXERCISE.md','ACTIVE.md') if (Path(cmd[5])/n).is_file())
        assert all(r['criterion'] in text for r in meta['rubric']),('exported rubric mismatch',ident)
        pins.add(brief)
        for path,digest in entry['bound_files'].items():
            assert sha(root/path)==digest,('selected bound input mismatch',path)
            pins.add(root/path)
        selected.append(ident)
    if 'curriculum/v5/exercises.json' in before:
        assert selected,'full catalog dependency cannot be removed without an actual selected export'
        pins.remove(root/'curriculum/v5/exercises.json')
    pins.add(root/'tools/selected_export_bindings.py')
    before.clear()
    before.update({p.relative_to(root).as_posix():sha(p) for p in sorted(pins)})
    return selected
