"""Read-only exact-origin and transitive evidence audit for residual parents."""
from __future__ import annotations
import argparse
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/residual_followthrough'
DIMENSIONS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation')
HASH=re.compile(r'^[0-9a-f]{64}$')
PIN_KEYS={'source_sha256','input_sha256','inspected_paths','log_sha256','external_input_sha256','external_sha256','external_source_sha256','outputs_sha256','artifact_sha256','generated_artifacts','export_files'}

def read(path):return json.loads((ROOT/path).read_text(encoding='utf-8-sig'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def require(ok,message):
    if not ok:raise ValueError(message)

def local(path):
    path=path.replace('\\','/')
    for prefix in ('/mnt/f/Learning/Curricula/c_mastery/','F:/Learning/Curricula/c_mastery/'):
        if path.lower().startswith(prefix.lower()):path=path[len(prefix):]
    if path.startswith('/') or re.match(r'^[A-Za-z]:',path):return None
    resolved=(ROOT/path).resolve()
    require(resolved.is_relative_to(ROOT.resolve()),'escaped evidence '+path)
    return path

def pins(value,location=''):
    if isinstance(value,dict):
        for key,item in value.items():
            if key in PIN_KEYS and isinstance(item,dict):
                for path,digest in item.items():
                    if isinstance(digest,str) and HASH.fullmatch(digest):yield location+'/'+key,path,digest
            if key=='log' and isinstance(item,str) and HASH.fullmatch(str(value.get('log_sha256',''))):
                yield location+'/log',item,value['log_sha256']
            if key=='path' and isinstance(item,str) and HASH.fullmatch(str(value.get('sha256',''))):
                yield location+'/path',item,value['sha256']
            yield from pins(item,location+'/'+key)
    elif isinstance(value,list):
        for index,item in enumerate(value):yield from pins(item,location+'/'+str(index))

def candidates():
    inspection=read(BASE+'/inspection_input.json')
    paths=set()
    for row in inspection['records']:
        for child in row['candidate_children']:
            paths.update(child['provider_candidates'])
    paths.add('research/v5/expansion/legacy_network_tcp/completion_dispositions.json')
    return sorted(paths)

def evidence_audit(provider_paths):
    from build_residual_followthrough_support import current
    documents={};reports=set();missing=[];stale=[];external={};file_hash={}
    def digest(path):
        if path not in file_hash:file_hash[path]=sha(path)
        return file_hash[path]
    for path in provider_paths:
        value=read(path);documents[path]=value
        for row in value.get('updates',[]):
            for p in row.get('dimensions',{}).get('author_validation',{}).get('evidence',[]):
                if p.endswith('.json'):reports.add(current(p))
        current_reports=value.get('current_reports',[])
        reports.update(current_reports if isinstance(current_reports,(dict,list)) else [])
        for row in value.get('campaigns',[]):
            if isinstance(row,dict) and row.get('report'):reports.add(row['report'])
    # Every referenced report is inspected as a receipt, not reduced to its name.
    for path in sorted(reports):
        if not (ROOT/path).is_file():missing.append(path);continue
        documents[path]=read(path)
    for path,value in documents.items():
        for pointer,name,expected in pins(value):
            converted=local(name)
            if converted is None:
                external.setdefault(name,set()).add(expected);continue
            try:
                if not (ROOT/converted).is_file():missing.append(path+pointer+': '+converted);continue
                actual=digest(converted)
            except OSError:
                # WSL symlinks (notably the private i386 sysroot) are not readable
                # through Win32. Check their actual bytes in WSL, never skip them.
                if sys.platform!='win32':raise
                external.setdefault('/mnt/f/Learning/Curricula/c_mastery/'+converted,set()).add(expected)
                continue
            if actual!=expected:stale.append({'document':path,'pointer':pointer,'path':converted,'expected':expected,'actual':actual})
    return {'providers':provider_paths,'reports':sorted(reports),'documents':len(documents),'files':len(file_hash),
            'document_sha256':{p:sha(p) for p in sorted(documents)},
            'missing':sorted(set(missing)),'stale':stale,'external':{p:sorted(h) for p,h in sorted(external.items())},
            'report_statuses':{p:{k:documents[p].get(k) for k in ('passed','status','source_stable','scope_source_stable') if k in documents[p]} for p in sorted(reports) if p in documents}}

def canonical_children(binding):
    objectives=set(binding['links']['objectives']);topics=set()
    for origin in binding['origins']:
        body=origin['record']
        if isinstance(body,dict) and body.get('topic'):topics.add(body['topic'])
    children={}
    # Scope by exact project/domain before reading; do not derive assessment
    # coverage from the mutable implementation update register.
    for domain in binding['links']['domains']:
        for path in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-'+domain+'-*.json'):
            for criterion in read(path)['criteria']:
                objective=criterion['objective']
                if objective not in objectives and not (not objectives and any(objective.startswith(t+'.') for t in topics)):continue
                children[criterion['id']]=(path.relative_to(ROOT).as_posix(),criterion,None)
                for variant in criterion.get('variants',[]):children[variant['id']]=(path.relative_to(ROOT).as_posix(),criterion,variant)
    return children

def inspect(support,mapping):
    from build_residual_followthrough_support import original,current,selection,SUCCESSORS,TICKLESS
    policy=read(BASE+'/clause_map.json');inspection=read(BASE+'/inspection_input.json')
    require(support['report_successors']==mapping['report_successors']==SUCCESSORS,'changed metadata successor')
    require(len(support['updates'])==len(mapping['bindings'])==86,'exact reviewed parent count')
    expected={r['id'] for r in inspection['records']} - set(policy['excluded'])
    require({r['id'] for r in support['updates']}=={r['id'] for r in mapping['bindings']}==expected,'parent identity set')
    require(set(support['unmerged_obligations'])==set(policy['excluded']),'lost unresolved original obligations')
    require(set(support['source_sha256']).isdisjoint({'curriculum/v5/expansion/work_inventory.json','curriculum/v5/expansion/implementation_updates.json'}),'mutable global register pinned')
    for path,digest in support['source_sha256'].items():require(sha(path)==digest,'stale bridge source '+path)
    provider_cache={p:read(p) for p in support['providers']};updates={r['id']:r for r in support['updates']}
    unique_children=set();checked_origins=0
    for binding in mapping['bindings']:
        row=updates[binding['id']]
        original_row=next(r for r in inspection['records'] if r['id']==row['id'])
        require(row['original_origins']==binding['origins']==original_row['origins'],'lost original origin body')
        require(binding['links']==original_row['links'] and binding['requirements']==original_row['requirements'],'changed parent requirement')
        key=selection(original_row,policy['clauses'])
        require(binding['clause_key']==key and binding['clause_analysis']==row['closure_rationale']==policy['clauses'][key],'changed or unrelated clause analysis')
        for origin in binding['origins']:
            if origin['record'] is not None:
                require(original(origin)==origin['record'],'origin changed '+row['id']);checked_origins+=1
                if row['id'].startswith('IW-'):
                    body=origin['record'];identity='IW-'+hashlib.sha256((origin['path']+'\n'+json.dumps(body,sort_keys=True,ensure_ascii=False)).encode()).hexdigest()[:16]
                    require(identity==row['id'],'content-derived original ID changed')
        expected_children=canonical_children(binding)
        require(set(expected_children)=={c['id'] for c in binding['children']}==set(row['required_criteria_and_variants']),'omitted or unrelated child/variant '+row['id'])
        require(len(row['required_criteria_and_variants'])==len(expected_children),'duplicate child')
        partial=[]
        for child in binding['children']:
            source=child['provider'];require(source in provider_cache,'unregistered provider')
            require(sha(source)==child['provider_sha256'],'stale child provider')
            actual=next((r for r in provider_cache[source]['updates'] if r['id']==child['id']),None)
            require(actual is not None,'missing child provider row')
            original_source,criterion,variant=expected_children[child['id']]
            require(child['original_source']==original_source and child['original_criterion']==criterion and child['original_variant']==variant,'changed canonical criterion/variant')
            require(child['original_task']==(variant or criterion)['task'],'changed task')
            if variant:require(variant['minimum_days']>=7,'lost delayed interval')
            require(child['mechanism']==actual['closure_rationale'],'unrelated mechanism')
            require(child['checks']==actual.get('mechanism_checks',actual.get('author_validation',{}).get('checks',[])),'lost exact check selectors')
            require(child['dimension_states']=={k:actual['dimensions'][k]['state'] for k in DIMENSIONS},'changed child dimension claim')
            require(child['evidence_limits']==actual.get('evidence_limits',[]),'lost child evidence limit')
            if 'partial' in actual['dimensions']['implementation']['state']:partial.append(child['id'])
            for dimension in ('teaching','implementation','author_validation'):
                required={current(p) for p in actual['dimensions'][dimension]['evidence']}
                require(required<=set(row['dimensions'][dimension]['evidence']),'missing exact '+dimension+' evidence')
            unique_children.add(child['id'])
        require(partial==binding['partial_children']==row['partial_children'],'partial child erased')
        implementation='partial-alternate-OS-execution-pending' if partial else 'complete-for-exact-parent-author-scope'
        require(row['dimensions']['implementation']['state']==implementation,'inflated implementation')
        require(set(row['dimensions'])==set(DIMENSIONS),'dimension schema')
        for dimension in ('external_review','independent_learner','physical_observation'):
            expected_state={'external_review':'not-run','independent_learner':'not-assessed','physical_observation':'not-claimed;original-physical-obligation-retained'}[dimension]
            require(row['dimensions'][dimension]=={'state':expected_state,'evidence':[]},'invented '+dimension)
        for dimension in row['dimensions'].values():
            for path in dimension['evidence']:require((ROOT/path).is_file(),'nonfile evidence '+path)
    require(support['remaining_author_work']==[{'id':'IW-e2cdc87b21fb7b31','partial_children':['AX-PR-T23-02-T23.04.b-DELAYED']}],'lost exact alternate-OS gap')
    tickless=read(TICKLESS)
    require(tickless['passed'] is True and len(tickless['checks'])==32 and all(r['passed'] for r in tickless['checks']),'tickless current guard')
    require(any(r['name']=='original-stopped-state-defect-compiled-semantic-rejection' and r['exit_code']==1 and 'TP_CHECK:' in r['output'] for r in tickless['checks']),'missing actual old-port regression')
    for p,h in tickless['inspected_paths'].items():require(sha(p)==h,'stale tickless guard source')
    posix=read('reviews/v5/expansion/residual_followthrough/posix_binding_guard_01.json')
    require(posix['passed'] and len(posix['negative_controls'])==5 and all(r['rejected'] for r in posix['negative_controls']),'POSIX proper metadata successor guard')
    uart=read('reviews/v5/expansion/residual_followthrough/serial_uart_guard_01.json')
    require(uart['passed'] and len(uart['negative_controls'])==23 and all(r['rejected'] for r in uart['negative_controls']),'UART refreshed envelope guard')
    return {'parents':86,'original_origins':checked_origins,'distinct_exact_children':len(unique_children),'unmerged_external_or_owned_elsewhere':12,'partial_parent':1,'tickless_guard_checks':32}

def external_check(pins_value):
    expected={}
    for path,values in pins_value.items():
        require(len(values)==1,'conflicting external tool/source history '+path)
        expected[path]=values[0]
    if not expected:return {'files':0,'passed':True}
    windows={p:h for p,h in expected.items() if re.match(r'^[A-Za-z]:',p)}
    for p,h in windows.items():require(Path(p).is_file() and hashlib.sha256(Path(p).read_bytes()).hexdigest()==h,'external Windows source changed '+p)
    unix={p:h for p,h in expected.items() if p not in windows}
    code="import sys,json,pathlib,hashlib; d=json.load(sys.stdin); bad=[p for p,h in d.items() if not pathlib.Path(p).is_file() or hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()!=h]; print(json.dumps({'files':len(d),'bad':bad})); sys.exit(bool(bad))"
    command=['wsl','--cd','/tmp','python3','-c',code] if sys.platform=='win32' else [sys.executable,'-c',code]
    result=subprocess.run(command,input=json.dumps(unix),capture_output=True,text=True,timeout=60)
    require(result.returncode==0,'external identity audit '+result.stdout+result.stderr)
    return {'files':len(expected),'passed':True,'command':command,'result':json.loads(result.stdout)}

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--audit-evidence',action='store_true')
    parser.add_argument('--self-test',action='store_true')
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    destination=(ROOT/args.report).resolve()
    require(destination.is_relative_to((ROOT/'reviews/v5/expansion/residual_followthrough').resolve()),'use residual evidence directory')
    require(not destination.exists(),'preserve existing report')
    support=read(BASE+'/support.json') if (ROOT/BASE/'support.json').exists() else None
    result=evidence_audit(support['providers'] if support else candidates())
    summary={};controls=[];errors=[]
    if not args.audit_evidence:
        try:
            summary=inspect(support,read(BASE+'/evidence_map.json'))
            require(not result['missing'] and not result['stale'],'transitive evidence is stale or missing')
            require(not any(v.get('passed') is False for v in result['report_statuses'].values()),'failed original receipt')
            result['external_validation']=external_check(result['external'])
            require(all(sha(p)==h for p,h in result['document_sha256'].items()),'evidence envelope changed during guard')
        except (ValueError,KeyError,AssertionError) as error:errors.append(str(error))
        if args.self_test and not errors:
            for kind in ('lost-origin','changed-clause','lost-child','changed-task','lost-delayed','unrelated-check','wrong-provider','stale-source','invented-review','invented-learner','invented-physical','erased-partial','global-register-pin'):
                altered=deepcopy(support);mapping=read(BASE+'/evidence_map.json')
                if kind=='lost-origin':mapping['bindings'][0]['origins'].pop()
                elif kind=='changed-clause':mapping['bindings'][0]['clause_analysis']='An unrelated anchor passed.'
                elif kind=='lost-child':mapping['bindings'][0]['children'].pop()
                elif kind=='changed-task':mapping['bindings'][0]['children'][0]['original_task']='Easier task'
                elif kind=='lost-delayed':next(c for b in mapping['bindings'] for c in b['children'] if c['original_variant'])['original_variant']['minimum_days']=0
                elif kind=='unrelated-check':mapping['bindings'][0]['children'][0]['checks']=['compiler version']
                elif kind=='wrong-provider':mapping['bindings'][0]['children'][0]['provider']=support['providers'][-1]
                elif kind=='stale-source':altered['source_sha256'][next(iter(altered['source_sha256']))]='0'*64
                elif kind.startswith('invented-'):altered['updates'][0]['dimensions'][{'invented-review':'external_review','invented-learner':'independent_learner','invented-physical':'physical_observation'}[kind]]['state']='passed'
                elif kind=='erased-partial':altered['remaining_author_work']=[]
                else:altered['source_sha256']['curriculum/v5/expansion/implementation_updates.json']=sha('curriculum/v5/expansion/implementation_updates.json')
                try:inspect(altered,mapping);rejected=False
                except (ValueError,KeyError,AssertionError):rejected=True
                controls.append({'control':kind,'rejected':rejected})
                if not rejected:errors.append('accepted '+kind)
        if not errors:
            require(all(sha(p)==h for p,h in result['document_sha256'].items()),'evidence envelope changed during controls')
    result.update(schema_version=1,created_utc=datetime.now(timezone.utc).isoformat(),
                  scope='Exact original parent/child/source bridge; no new execution or external evidence inferred',
                  passed=not result['missing'] and not result['stale'] and not errors,summary=summary,negative_controls=controls,errors=errors)
    destination.parent.mkdir(parents=True,exist_ok=True)
    destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({k:result[k] for k in ('passed','documents','files')}|{'missing':len(result['missing']),'stale':len(result['stale'])}))
    return 0 if result['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
