"""Build only the reviewed additive residual parent bridge; never merge inventory."""
from __future__ import annotations
from copy import deepcopy
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
BASE='research/v5/expansion/residual_followthrough'
DIMENSIONS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation')
SUCCESSORS={'research/v5/expansion/legacy_posix_followthrough/support.json':
            'research/v5/expansion/legacy_posix_context/launcher_binding_successor.json'}
TCP='research/v5/expansion/legacy_network_tcp/completion_dispositions.json'
TICKLESS='reviews/v5/expansion/legacy_realtime_tickless/residual_guard_02.json'

def read(path):return json.loads((ROOT/path).read_text(encoding='utf-8-sig'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def write(path,value):
    (ROOT/path).write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')

def original(origin):
    value=read(origin['path'])
    for token in origin['pointer'].strip('/').split('/'):
        token=token.replace('~1','/').replace('~0','~')
        value=value[int(token)] if isinstance(value,list) else value[token]
    return value

def current(path):return SUCCESSORS.get(path,path)

def selection(row,clauses):
    if row['id'] in clauses:return row['id']
    objective=row['links']['objectives']
    if len(objective)==1 and objective[0] in clauses:return objective[0]
    for origin in row['origins']:
        body=origin['record']
        if isinstance(body,dict) and body.get('topic') in clauses:return body['topic']
    raise ValueError('No reviewed clause for '+row['id'])

def provider(child):
    paths=child['provider_candidates']
    if child['id'].startswith(('AX-PR-T41-06-','AX-PR-T41-07-')):return TCP
    if not paths:raise ValueError('No current provider '+child['id'])
    # Prefer the current package, never its byte-preserved history directory.
    return current(min(paths,key=lambda p:(len(Path(p).parts),len(p),p)))

def build():
    inspection=read(BASE+'/inspection_input.json');policy=read(BASE+'/clause_map.json')
    updates=[];bindings=[];unmerged=[];sources={BASE+'/inspection_input.json',BASE+'/clause_map.json',
        'tools/audit_residual_followthrough.py','tools/build_residual_followthrough_support.py',
        'tools/check_residual_followthrough.py','labs/residual_followthrough/README.md',
        'assessor/residual_followthrough/ANALYSIS.md',BASE+'/alternative_close_review.json'}
    used_providers=set()
    for row in inspection['records']:
        if row['id'] in policy['excluded']:
            unmerged.append({'id':row['id'],'origins':row['origins'],'reason':policy['excluded'][row['id']]});continue
        key=selection(row,policy['clauses']);clause=policy['clauses'][key]
        children=[];evidence={k:set() for k in DIMENSIONS};partial=[];limits=set()
        for candidate in row['candidate_children']:
            source=provider(candidate);value=read(source);used_providers.add(source)
            child=next(r for r in value['updates'] if r['id']==candidate['id'])
            canonical=read(candidate['original_source'])
            original_criterion=next(c for c in canonical['criteria'] if c['id']==child['id'] or any(v['id']==child['id'] for v in c.get('variants',[])))
            variant=next((v for v in original_criterion.get('variants',[]) if v['id']==child['id']),None)
            task=(variant or original_criterion)['task']
            if child.get('original_task',task)!=task:raise ValueError('Changed child task '+child['id'])
            if candidate['original_task']!=task:raise ValueError('Inspection task drift '+child['id'])
            if 'partial' in child['dimensions']['implementation']['state']:partial.append(child['id'])
            for dimension in ('teaching','implementation','author_validation'):
                evidence[dimension].update(current(p) for p in child['dimensions'][dimension]['evidence'])
            evidence['author_validation'].add(source)
            limits.update(child.get('evidence_limits',[]))
            checks=child.get('mechanism_checks',child.get('author_validation',{}).get('checks',[]))
            children.append({'id':child['id'],'provider':source,'provider_sha256':sha(source),
                'original_source':candidate['original_source'],'original_task':task,
                'original_criterion':original_criterion,'original_variant':variant,
                'mechanism':child['closure_rationale'],'checks':checks,
                'dimension_states':{k:child['dimensions'][k]['state'] for k in DIMENSIONS},
                'evidence_limits':child.get('evidence_limits',[])})
            sources.add(candidate['original_source'])
        if not children:raise ValueError('No exact constituent tasks '+row['id'])
        for origin in row['origins']:
            sources.add(origin['path'])
            if origin['record'] is not None and original(origin)!=origin['record']:raise ValueError('Origin changed '+row['id'])
        if row['id']=='G-FOLLOWTHROUGH-T52-08-STOPPED':
            evidence['author_validation'].add(TICKLESS)
            evidence['implementation'].add('assessor/legacy_realtime/tickless_port.c')
            evidence['teaching'].add('research/v5/expansion/legacy_realtime_tickless/stopped_state_correction.json')
        if key=='T23.04.b':evidence['author_validation'].add(BASE+'/alternative_close_review.json')
        evidence['teaching'].add('labs/residual_followthrough/README.md')
        evidence['implementation'].add('assessor/residual_followthrough/ANALYSIS.md')
        evidence['author_validation'].add(BASE+'/evidence_map.json')
        implementation='partial-alternate-OS-execution-pending' if partial else 'complete-for-exact-parent-author-scope'
        dimensions={k:{'state':s,'evidence':sorted(evidence[k])} for k,s in [
            ('teaching','complete-for-exact-parent-author-scope'),('implementation',implementation),
            ('author_validation','passed-existing-mechanism-receipts-currently-revalidated;external-observations-separate'),
            ('external_review','not-run'),('independent_learner','not-assessed'),
            ('physical_observation','not-claimed;original-physical-obligation-retained')]}
        binding={'id':row['id'],'origins':row['origins'],'links':row['links'],'requirements':row['requirements'],
            'clause_key':key,'clause_analysis':clause,'children':children,'partial_children':partial}
        bindings.append(binding)
        updates.append({'id':row['id'],'original_origins':row['origins'],'dimensions':dimensions,
            'closure_rationale':clause,'required_criteria_and_variants':[c['id'] for c in children],
            'partial_children':partial,'validation_method':'Exact origin/criterion/variant and transitive source/log/output binding; retain mechanism-specific commands and semantic controls from each original receipt.',
            'evidence_limits':sorted(limits|{'This bridge is an author evidence reconciliation, not a new learner attempt or independent review.',
                'An original physical/source-access clause remains open in its own dimension even when software author support is complete.'})})
    sources.update(used_providers)
    for row in updates:
        for dimension in row['dimensions'].values():sources.update(dimension['evidence'])
    sources.discard(BASE+'/evidence_map.json')
    for path in sources:
        if not (ROOT/path).is_file():raise ValueError('Evidence is not a file '+path)
    mapping={'schema_version':1,'bindings':bindings,'unmerged':unmerged,'report_successors':SUCCESSORS,
        'initial_diagnostic_only':'reviews/v5/expansion/residual_followthrough/source_audit_01.json',
        'scope':'Original origin bodies, canonical tasks and delayed variants remain unchanged; each constituent has its own mechanism and checks.'}
    write(BASE+'/evidence_map.json',mapping)
    sources.add(BASE+'/evidence_map.json')
    value={'schema_version':1,'required':True,'scope':'Exact additive residual author bridge; no global completion, physical, independent learner or source-access claim.',
        'updates':updates,'source_sha256':{p:sha(p) for p in sorted(sources)},'providers':sorted(used_providers),
        'report_successors':SUCCESSORS,'remaining_author_work':[{'id':r['id'],'partial_children':r['partial_children']} for r in bindings if r['partial_children']],
        'unmerged_obligations':[r['id'] for r in unmerged]}
    write(BASE+'/support.json',value)
    print(json.dumps({'updates':len(updates),'providers':len(used_providers),'unmerged':len(unmerged),'partial':value['remaining_author_work']}))

if __name__=='__main__':build()
