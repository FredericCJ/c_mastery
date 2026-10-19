"""Explicit local author evidence inputs, never added to a candidate archive.

Authority comes from exact active disposition rows and their embedded byte pins.
This is environment provisioning, not a new source inspection or execution.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil

from common import ROOT, dump
from audit_current_envelopes import historical
from audit_active_disposition_pins import record_hash
from check_residual_followthrough import pins, local

ACTIVE='curriculum/v5/expansion/implementation_updates.json'
LIMIT='Qualified local author reproduction: the ZIP alone does not contain these required author evidence inputs. No redistribution, new inspection, new mechanism execution, release acceptance or external observation is inferred.'


def sha(path):
    with path.open('rb') as stream:return hashlib.file_digest(stream,'sha256').hexdigest()


def names_hash(names):
    return hashlib.sha256(json.dumps(sorted(names),separators=(',',':')).encode()).hexdigest()


def bounded(root,name,exists=True):
    if not isinstance(name,str) or '\\' in name or not name or name.startswith('/') or ':' in name:
        raise ValueError('Author input must be an exact relative POSIX path: '+str(name))
    parts=name.split('/')
    if any(p in ('','.','..') for p in parts):raise ValueError('Unsafe author input path: '+name)
    path=root/name
    if path.resolve()!=root.resolve()/name or (exists and not path.is_file()):
        raise ValueError('Missing or redirected author input: '+name)
    return path


def category(name):
    if 'private' in Path(name).parts:return 'private-research-capture'
    if name.startswith('build/source-gap-final/'):return 'private-primary-source-capture'
    if name.startswith('build/'):return 'retained-author-build-or-observation-artifact'
    return 'other-excluded-local-author-evidence'


def discover(root,identity,archive_names):
    root=root.resolve();archive_names=set(archive_names)
    active_path=bounded(root,ACTIVE);active=json.loads(active_path.read_text(encoding='utf8'))['updates']
    current={r['id']:record_hash(r) for r in active}
    if len(current)!=len(active):raise ValueError('Duplicate active disposition ID')
    values={};external=[];supplier_hashes={}
    for area in ('research/v5/expansion','research/v5/modern'):
        for directory,dirs,files in os.walk(root/area):
            dirs[:]=sorted(d for d in dirs if not historical(d))
            for filename in sorted(files):
                if historical(filename) or not filename.endswith('.json'):continue
                path=Path(directory)/filename;relative=path.relative_to(root).as_posix()
                data=json.loads(path.read_text(encoding='utf8'))
                if not isinstance(data,dict) or not isinstance(data.get('updates'),list):continue
                selected=sorted(r['id'] for r in data['updates'] if isinstance(r,dict) and r.get('id') in current and record_hash(r)==current[r['id']])
                if not selected:continue
                provider_hash=sha(bounded(root,relative))
                for pointer,name,digest in pins(data):
                    normalized=local(name)
                    authority=dict(provider=relative,provider_sha256=provider_hash,pointer=pointer,pin_name=name,active_ids=selected)
                    if normalized is None:
                        external.append(dict(path=name,sha256=digest,authority=authority,copied=False));continue
                    if normalized in archive_names:continue
                    supplier=bounded(root,normalized)
                    if normalized not in supplier_hashes:supplier_hashes[normalized]=sha(supplier)
                    if supplier_hashes[normalized]!=digest:raise ValueError('Stale excluded active input: '+normalized)
                    entry=values.setdefault(normalized,dict(destination=normalized,supplier_relative_path=normalized,sha256=digest,bytes=supplier.stat().st_size,category=category(normalized),authority=[],non_redistributed=True))
                    if entry['sha256']!=digest:raise ValueError('Conflicting excluded input identities: '+normalized)
                    if authority not in entry['authority']:entry['authority'].append(authority)
    for entry in values.values():entry['authority'].sort(key=lambda row:(row['provider'],row['pointer'],row['pin_name']))
    external.sort(key=lambda row:(row['path'],row['authority']['provider'],row['authority']['pointer']))
    return dict(schema_version=1,kind='qualified-local-author-environment',supplier_root=str(root),source_identity_sha256=identity['sha256'],active_dispositions_sha256=sha(active_path),archive_selection_sha256=names_hash(archive_names),inputs=[values[n] for n in sorted(values)],external_inputs_not_copied=external,limits=LIMIT)


def validate_manifest(manifest,expected):
    if manifest!=expected:raise ValueError('Author environment manifest differs from exact current active pins, source identity or archive selection')


def require_opt_in(expected,manifest):
    if manifest is None:
        if expected['inputs']:raise ValueError('Source ZIP alone lacks '+str(len(expected['inputs']))+' required local author evidence inputs; supply an exact --author-environment-manifest to request qualified local reproduction')
    else:validate_manifest(manifest,expected)


def stage(root,extracted,manifest,expected,archive_names):
    validate_manifest(manifest,expected)
    root=root.resolve();extracted=extracted.absolute();archive_names=set(archive_names)
    if not extracted.is_dir() or extracted.resolve()!=extracted:raise ValueError('Extracted candidate must be an existing unredirected directory')
    planned=[]
    for row in manifest['inputs']:
        name=row['destination']
        if name!=row['supplier_relative_path'] or name in archive_names or row['non_redistributed'] is not True:
            raise ValueError('Author input overlaps archive or changes supplier identity: '+name)
        supplier=bounded(root,name);destination=bounded(extracted,name,exists=False)
        if destination.exists() or destination.is_symlink():raise ValueError('Author staging would overwrite candidate member: '+name)
        if sha(supplier)!=row['sha256'] or supplier.stat().st_size!=row['bytes']:raise ValueError('Author supplier changed: '+name)
        planned.append((supplier,destination,row))
    # Admission of every path precedes writes. No directory traversal/copy or
    # external absolute supplier is permitted; each destination is fresh.
    for supplier,destination,row in planned:
        destination.parent.mkdir(parents=True,exist_ok=True)
        bounded(extracted,row['destination'],exists=False)
        with supplier.open('rb') as source,destination.open('xb') as target:shutil.copyfileobj(source,target,length=1024*1024)
        if sha(destination)!=row['sha256']:raise ValueError('Staged author bytes differ: '+row['destination'])
    verify_staged(root,extracted,manifest)
    return dict(provisioned=True,files=len(planned),bytes=sum(row['bytes'] for _,_,row in planned),manifest_sha256=hashlib.sha256(json.dumps(manifest,sort_keys=True,separators=(',',':')).encode()).hexdigest(),archive_self_contained=not planned,external_absolute_paths_copied=False,new_source_inspection=False,new_mechanism_execution=False,limits=LIMIT)


def verify_staged(root,extracted,manifest):
    for row in manifest['inputs']:
        for folder in (root,extracted):
            path=bounded(folder,row['destination'])
            if sha(path)!=row['sha256'] or path.stat().st_size!=row['bytes']:raise ValueError('Author environment input changed: '+row['destination'])


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--manifest',type=Path,required=True);args=parser.parse_args()
    destination=(ROOT/args.manifest).absolute()
    if destination.exists() or destination.resolve()!=destination or not destination.is_relative_to((ROOT/'reviews').resolve()):raise ValueError('Use a fresh unredirected manifest under reviews')
    from package_v4 import source_files
    from v2_common import source_identity
    from v5_common import RECEIPTS
    identity=source_identity();names={p.relative_to(ROOT).as_posix() for p in source_files(identity,RECEIPTS)}
    # This new environment manifest will itself be selected for the archive.
    names.add(destination.relative_to(ROOT).as_posix())
    value=discover(ROOT,identity,names)
    if source_identity()!=identity:raise ValueError('Source changed during environment discovery')
    dump(destination,value);print(json.dumps(dict(manifest=str(destination),files=len(value['inputs']),bytes=sum(r['bytes'] for r in value['inputs']),archive_self_contained=False,limits=LIMIT)))


if __name__=='__main__':main()
