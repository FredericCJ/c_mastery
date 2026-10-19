"""Explicit local author evidence inputs, never added to a candidate archive.

Authority comes from exact active disposition rows and their embedded byte pins,
the validated resource-identity correction policy, and the exact selected POSIX
supersession export maps consumed by the build.
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
CORRECTIONS='research/v5/resource_identity_corrections.json'
CORRECTION_CONSUMERS=('tools/resource_identity_v5.py','tools/resource_inputs_v5.py')
# Exact reader edges in the two existing supersession validators. This is not
# recursive discovery of arbitrary receipt/history JSON or directory contents.
SUPERSESSION_EXPORTS={
    'research/v5/expansion/legacy_posix_context/launcher_binding_successor.json':
        ('research/v5/expansion/legacy_posix_context/exports.json',
         ('source_sha256','outputs_sha256'),'pass'),
    'research/v5/expansion/legacy_posix_binding_current/completion_dispositions.json':
        ('reviews/v5/expansion/posix_binding_current/data_exports.json',
         ('source_sha256',),'passed'),
}
SUPERSESSION_SELECTOR='tools/audit_active_disposition_pins.py'
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


def correction_capture_inputs(root,archive_names):
    """Expose only capture edges actually consumed by corrections(root).

    The policy, predecessor registry, evidence record and consumer sources must
    already ship as portable metadata. Captures remain opt-in local inputs;
    this neither registers a reading nor authorizes source redistribution.
    """
    from resource_identity_v5 import corrections
    if not (root/CORRECTIONS).exists():return []
    policy_path=bounded(root,CORRECTIONS);policy_sha=sha(policy_path)
    policy=json.loads(policy_path.read_text(encoding='utf8'))
    # Use the real consumer: malformed policy, stale predecessor/evidence and
    # changed captures fail before any dependency is admitted or staged.
    reviewed=corrections(root)
    if sha(policy_path)!=policy_sha or reviewed!={entry['id']:entry for entry in policy['corrections']}:
        raise ValueError('Resource correction policy changed during discovery')
    previous=policy['previous_registry']
    required={CORRECTIONS,previous['path'],*CORRECTION_CONSUMERS}
    required.update(entry['evidence']['path'] for entry in reviewed.values())
    for name in sorted(required):
        bounded(root,name)
        if name not in archive_names:
            raise ValueError('Resource correction authority is absent from source archive: '+name)
    consumers={name:sha(bounded(root,name)) for name in CORRECTION_CONSUMERS}
    result=[]
    for index,entry in enumerate(policy['corrections']):
        evidence=entry['evidence'];path=bounded(root,evidence['path'])
        if sha(path)!=evidence['sha256']:
            raise ValueError('Resource correction evidence changed during discovery')
        record=json.loads(path.read_text(encoding='utf8'))['records'][entry['id']]
        if 'capture' not in record:continue
        name=record['capture'];capture=bounded(root,name)
        if sha(capture)!=record['sha256']:
            raise ValueError('Resource correction capture changed during discovery: '+name)
        if name in archive_names and 'private' in Path(name).parts:
            raise ValueError('Private resource correction capture is in source archive: '+name)
        key=entry['id'].replace('~','~0').replace('/','~1')
        authority=dict(kind='validated-resource-identity-correction-capture',
            provider=CORRECTIONS,provider_sha256=policy_sha,
            pointer='/corrections/'+str(index)+'/evidence',pin_name=name,
            resource_id=entry['id'],previous_registry=previous,
            evidence=dict(path=evidence['path'],sha256=evidence['sha256'],
                record=entry['id'],pointer='/records/'+key+'/capture',
                record_sha256=hashlib.sha256(json.dumps(record,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()),
            consumer='resource_identity_v5.corrections',consumer_source_sha256=consumers)
        result.append((name,record['sha256'],authority))
    return result


def supersession_export_inputs(root,archive_names,current):
    """Bind selected export input maps without replacing supersession guards.

    Full semantic supersession validation still runs during the extracted build.
    Here the exact proof/selector/guard/successor/original-row/report chain grants
    only local byte provisioning. A new selector or reader requires review.
    """
    from audit_active_disposition_pins import SUPERSESSIONS,PROOF
    if not (root/PROOF).exists():
        if any((root/name).exists() for name in SUPERSESSION_EXPORTS):
            raise ValueError('Missing POSIX supersession verification authority')
        return []
    def require(ok,message):
        if not ok:raise ValueError(message)
    def portable(name):
        path=bounded(root,name)
        require(name in archive_names,'Supersession authority is absent from source archive: '+name)
        return path
    def document(name,digest=None):
        path=portable(name);actual=sha(path)
        require(digest is None or actual==digest,'Stale supersession authority: '+name)
        value=json.loads(path.read_text(encoding='utf8'))
        require(isinstance(value,dict),'Malformed supersession authority: '+name)
        return value,actual
    require(set(SUPERSESSIONS)==set(SUPERSESSION_EXPORTS),'Unreviewed supersession selector')
    selector_sha=sha(portable(SUPERSESSION_SELECTOR))
    proof,proof_sha=document(PROOF)
    checks=proof.get('checks')
    require(proof.get('passed') is True and isinstance(checks,list)
            and len(checks)==len(SUPERSESSIONS) and all(isinstance(x,dict) for x in checks)
            and {x.get('successor') for x in checks}==set(SUPERSESSIONS),
            'Incomplete or failed supersession verification')
    result=[]
    for index,item in enumerate(checks):
        name=item['successor'];module,function,original_paths=SUPERSESSIONS[name]
        guard='tools/'+module+'.py'
        require(item.get('guard')==module+'.'+function and item.get('errors')==[],
                'Wrong or failed supersession guard')
        require(sha(portable(guard))==item.get('guard_source_sha256'),'Stale supersession guard')
        require(isinstance(item.get('sha256'),str) and re.fullmatch('[0-9a-f]{64}',item['sha256']),
                'Missing or malformed supersession successor digest')
        value,value_sha=document(name,item.get('sha256'))
        rows=value.get('updates')
        require(isinstance(rows,list) and rows and all(isinstance(x,dict) and isinstance(x.get('id'),str) for x in rows),
                'Malformed supersession rows')
        row_hashes={row['id']:record_hash(row) for row in rows}
        require(len(row_hashes)==len(rows) and type(item.get('row_count')) is int
                and item['row_count']==len(rows),'Changed or duplicate supersession rows')
        selected=sorted(key for key,digest in row_hashes.items() if current.get(key)==digest)
        require(bool(selected),'Supersession has no exact current disposition')
        originals=item.get('originals')
        require(isinstance(originals,list) and len(originals)==len(original_paths)
                and all(isinstance(x,dict) for x in originals)
                and {x.get('path') for x in originals}==original_paths,
                'Missing or unexpected supersession predecessor')
        retained=[]
        for original in originals:
            require(isinstance(original.get('sha256'),str) and re.fullmatch('[0-9a-f]{64}',original['sha256']),
                    'Missing or malformed supersession predecessor digest')
            prior,prior_sha=document(original['path'],original.get('sha256'))
            prior_rows=prior.get('updates')
            require(isinstance(prior_rows,list) and prior_rows and all(isinstance(x,dict) and isinstance(x.get('id'),str) for x in prior_rows)
                    and len({x['id'] for x in prior_rows})==len(prior_rows),
                    'Malformed supersession predecessor rows')
            require(all(row_hashes.get(x['id'])==record_hash(x) for x in prior_rows),
                    'Original supersession disposition changed')
            retained.append(dict(path=original['path'],sha256=prior_sha))
        report_name,fields,status=SUPERSESSION_EXPORTS[name]
        reports=value.get('current_reports');source_pins=value.get('source_sha256')
        require(isinstance(reports,list) and reports.count(report_name)==1
                and isinstance(source_pins,dict) and isinstance(source_pins.get(report_name),str)
                and re.fullmatch('[0-9a-f]{64}',source_pins[report_name]),
                'Missing selected supersession export pin')
        report,report_sha=document(report_name,source_pins[report_name])
        commands=report.get('commands')
        require(report.get('status')==status and (status!='passed' or report.get('passed') is True)
                and isinstance(commands,list) and commands
                and all(isinstance(x,dict) and x.get('passed') is True for x in commands),
                'Failed or malformed selected supersession export')
        for field in fields:
            mapping=report.get(field)
            require(isinstance(mapping,dict) and bool(mapping),'Missing consumed supersession map: '+field)
            for path,digest in sorted(mapping.items()):
                require(isinstance(digest,str) and re.fullmatch('[0-9a-f]{64}',digest),
                        'Malformed selected supersession input digest')
                supplier=bounded(root,path)
                require(sha(supplier)==digest,'Stale selected supersession input: '+path)
                require(not (path in archive_names and 'private' in Path(path).parts),
                        'Private supersession input is in source archive: '+path)
                escape=lambda text:text.replace('~','~0').replace('/','~1')
                authority=dict(kind='hash-bound-posix-supersession-export-input',
                    provider=name,provider_sha256=value_sha,
                    pointer='/source_sha256/'+escape(report_name),pin_name=path,active_ids=selected,
                    verification=dict(path=PROOF,sha256=proof_sha,pointer='/checks/'+str(index)),
                    selector=dict(path=SUPERSESSION_SELECTOR,sha256=selector_sha),
                    guard=dict(path=guard,sha256=item['guard_source_sha256'],function=item['guard']),
                    originals=sorted(retained,key=lambda x:x['path']),
                    report=dict(path=report_name,sha256=report_sha,pointer='/'+field+'/'+escape(path)),
                    qualification='Retained input bytes only; the original strict guard remains mandatory and no prior execution or inspection identity is rewritten.')
                result.append((path,digest,authority))
    return result


def discover(root,identity,archive_names):
    root=root.resolve();archive_names=set(archive_names)
    active_path=bounded(root,ACTIVE);active=json.loads(active_path.read_text(encoding='utf8'))['updates']
    current={r['id']:record_hash(r) for r in active}
    if len(current)!=len(active):raise ValueError('Duplicate active disposition ID')
    values={};external=[];supplier_hashes={}
    def include(normalized,digest,authority):
        if normalized in archive_names:return
        supplier=bounded(root,normalized)
        if normalized not in supplier_hashes:supplier_hashes[normalized]=sha(supplier)
        if supplier_hashes[normalized]!=digest:raise ValueError('Stale excluded active input: '+normalized)
        entry=values.setdefault(normalized,dict(destination=normalized,supplier_relative_path=normalized,sha256=digest,bytes=supplier.stat().st_size,category=category(normalized),authority=[],non_redistributed=True))
        if entry['sha256']!=digest:raise ValueError('Conflicting excluded input identities: '+normalized)
        if authority not in entry['authority']:entry['authority'].append(authority)
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
                    include(normalized,digest,authority)
    for name,digest,authority in correction_capture_inputs(root,archive_names):
        include(name,digest,authority)
    for name,digest,authority in supersession_export_inputs(root,archive_names,current):
        include(name,digest,authority)
    for entry in values.values():entry['authority'].sort(key=lambda row:(row['provider'],row['pointer'],row['pin_name']))
    external.sort(key=lambda row:(row['path'],row['authority']['provider'],row['authority']['pointer']))
    return dict(schema_version=1,kind='qualified-local-author-environment',supplier_root=str(root),source_identity_sha256=identity['sha256'],active_dispositions_sha256=sha(active_path),archive_selection_sha256=names_hash(archive_names),inputs=[values[n] for n in sorted(values)],external_inputs_not_copied=external,limits=LIMIT)


def validate_manifest(manifest,expected):
    if manifest!=expected:raise ValueError('Author environment manifest differs from exact current input authorities, source identity or archive selection')


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
