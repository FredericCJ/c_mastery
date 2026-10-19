"""Export explicitly declared public project inputs without private solutions."""
from pathlib import Path, PurePosixPath
import hashlib
import json
import re
import shutil


def prepare(root, project):
    root=Path(root).resolve()
    brief=root/'curriculum/projects'/(project+'.md')
    text=brief.read_text(encoding='utf-8-sig')
    match=re.match(r'\A```json\s*\n(.*?)\n```',text,re.S)
    if match is None:
        if text.startswith('```json'):raise ValueError('Malformed project asset contract')
        return []
    data=json.loads(match[1])
    if data.get('id')!=project:raise ValueError('Project asset identity differs')
    files={}
    for key,areas in [('shared_contracts',{'curriculum','labs'}),('supplied_artifacts',{'curriculum','labs','fixtures','tools'})]:
        values=data.get(key,[])
        if not isinstance(values,list):raise ValueError('Project assets must be a list: '+key)
        for name in values:
            if not isinstance(name,str):raise ValueError('Invalid public project asset')
            rel=PurePosixPath(name)
            if (not rel.parts or rel.is_absolute() or rel.as_posix()!=name or '\\' in name or ':' in name
                    or '..' in rel.parts or rel.parts[0] not in areas or 'assessor' in {p.casefold() for p in rel.parts}):
                raise ValueError('Invalid public project asset: '+name)
            source=root/Path(*rel.parts)
            if (not source.is_file() or not source.resolve().is_relative_to(root/rel.parts[0])
                    or any(p.is_symlink() for p in [source,*source.parents] if p!=root and p.is_relative_to(root))):
                raise ValueError('Missing or unsafe public project asset: '+name)
            files[name]={'path':name,'sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
    return [files[name] for name in sorted(files)]


def export(root,destination,records):
    root=Path(root).resolve();destination=Path(destination).resolve();copies=[]
    for row in records:
        source=root/row['path'];target=destination/row['path']
        if (not target.resolve().is_relative_to(destination)
                or any(p.is_symlink() for p in [target,*target.parents] if p!=destination and p.is_relative_to(destination))):
            raise ValueError('Unsafe project asset destination')
        if hashlib.sha256(source.read_bytes()).hexdigest()!=row['sha256']:
            raise ValueError('Project asset changed during initialization')
        if target.exists() and (not target.is_file() or hashlib.sha256(target.read_bytes()).hexdigest()!=row['sha256']):
            raise ValueError('Project asset would overwrite different learner input')
        copies.append((source,target))
    # Complete every check before copying any of this declared input set.
    for source,target in copies:
        target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,target)
    return records
