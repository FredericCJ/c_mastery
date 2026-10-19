"""Read-only exact-ID, source, execution and dependency guard for the companion."""
import argparse
import copy
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path[:0] = [str(ROOT), str(ROOT/'tools')]
from cmastery.exercises import ordered, parse_brief
from build_modern_followthrough_support import (BASE, COMMON_LIMITS, DIMENSIONS,
    FINAL, FREE, NEW_RUNTIME, OLD_RUNTIME, aggregate, current, variant_text)
from rebind_modern_publication import criterion_text, validate_publication_proof, MANIFEST, PROOF, TOOL, HISTORY

HASHES = {}
PUBLICATION_DIFFERENCES = []
PUBLICATION_BINDINGS = {}


def sha(path):
    value = str(path)
    if value not in HASHES:
        HASHES[value] = hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
    return HASHES[value]


def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf8'))


def require(ok, reason):
    if not ok: raise ValueError(reason)


def retained_pin(path, expected):
    actual=sha(path)
    if actual==expected:return
    from check_expansion_campaigns import publication_pin
    key=(path,expected,actual)
    binding=PUBLICATION_BINDINGS.get(key)
    if binding is None:
        binding=publication_pin(path,expected,actual)
        if binding is not None:PUBLICATION_BINDINGS[key]=binding
    require(binding is not None,'stale retained source '+path)
    if binding not in PUBLICATION_DIFFERENCES:PUBLICATION_DIFFERENCES.append(binding)


def receipt(report, external):
    require(report.get('passed') is True, 'unpassed report')
    require(report.get('status','passed') == 'passed', 'report status')
    for key in ('scope_source_stable','source_stable','external_inputs_stable'):
        require(report.get(key,True) is True, 'unstable report source')
    for key in ('inspected_paths','input_sha256'):
        for path, expected in report.get(key,{}).items():
            retained_pin(path,expected)
    for path, expected in report.get('external_input_sha256',{}).items():
        require(path not in external or external[path] == expected, 'conflicting toolchain history '+path)
        external[path] = expected
    executions = report.get('executions',report.get('commands',[]))
    require(bool(executions), 'missing actual commands')
    for row in executions + report.get('wrapper_executions',[]):
        require(bool(row.get('command')), 'missing command')
        if 'log_sha256' in row:
            require(sha(row['log']) == row['log_sha256'], 'changed execution log')
    for key in ('artifacts','generated_artifacts','export_files'):
        values = report.get(key,{})
        if isinstance(values,dict):
            for path, expected in values.items():
                if isinstance(expected,str) and re.fullmatch('[0-9a-f]{64}',expected):
                    require(sha(path)==expected, 'changed generated artifact '+path)


def companion(report, prefix, rows, names):
    require(report.get('profiles') == 6 and report.get('oracle_rows_per_profile') == rows, 'companion profile/oracle count')
    require(report.get('semantic_controls') == 6 and report.get('learner_rejections') == 4 and report.get('exports') == 4, 'companion controls/exports')
    observations = report['observations']
    actual = {r['profile'] for r in observations if 'oracle_rows' in r}
    require(actual == {c+'-'+p for c in ('gcc','clang') for p in ('O0','O2','asan')}, 'compiler profile coverage')
    require(all(r['oracle_rows']==rows for r in observations if 'oracle_rows' in r), 'independent case count')
    controls = {(r['profile'],r['semantic_control']) for r in observations if 'semantic_control' in r and r.get('rejected')}
    require(controls == {(c,n) for c in ('gcc','clang') for n in names}, 'missing exact semantic control')
    require(len(report['executions']) == 38, 'actual companion command count')
    failures = [r for r in report['executions'] if r['exit_code'] == 1]
    require(len(failures) == 10, 'semantic/stub rejection count')
    for row in failures:
        text = (ROOT/row['log']).read_text(encoding='utf8')
        require(prefix in text and 'AddressSanitizer' not in text and 'runtime error:' not in text,
                'nonsemantic rejection')
    for observation in observations:
        if 'binary' in observation: require(sha(observation['binary'])==observation['sha256'], 'changed positive executable')
        if 'source' in observation: require(sha(observation['source'])==observation['sha256'], 'changed staged control')


def inspect(support, mapping, briefs, free, final):
    require(not validate_publication_proof(),'current publication proof')
    require(support.get('publication_successor')==dict(state='current-publication-validated;original-execution-snapshots-retained',
        new_c_execution=False,source_sha256={p:sha(p) for p in (MANIFEST,PROOF,TOOL)}),'exact publication qualifier')
    require(support['report_successors']=={OLD_RUNTIME:NEW_RUNTIME}, 'missing explicit fresh runtime campaign')
    require(support['remaining_author_work']==[], 'open author work cannot be marked ready')
    for path, digest in support['source_sha256'].items():
        require(sha(path)==digest, 'stale support input '+path)
    ids = [r['id'] for r in support['updates']]
    require(len(ids)==len(set(ids))==231, 'missing or duplicate exact obligation')
    by_id = {r['id']:r for r in support['updates']}
    readiness = {}
    for family in ('dispatch','dod','object_runtime'):
        ready_path='research/v5/modern/'+family+'_objective_readiness.json'
        for row in read(ready_path)['criteria']: readiness[row['id']]=(ready_path,row)
    expected = set()
    for binding in mapping['roots']:
        meta, body = briefs[binding['exercise']]
        require(binding['criteria']==meta['rubric'], 'changed original rubric')
        require(binding['variants']==variant_text(body), 'changed original variant')
        root = by_id[meta['id']]
        criterion_ids = [c['id'] for c in meta['rubric']]
        variant_ids = meta.get('delayed_transfer',meta.get('delayed_assessment'))['variant_ids']
        require(root['required_criteria']==criterion_ids and root['required_variants']==variant_ids, 'root omits constituent work')
        require(binding['prerequisites']==meta['prerequisite_exercises'], 'changed prerequisite binding')
        expected.update([meta['id'],*criterion_ids,*variant_ids])
        for criterion in meta['rubric']:
            row = by_id[criterion['id']]
            require(row['original_criterion']==criterion, 'changed exact criterion')
            for key in ('teaching','implementation','author_validation'):
                require(bool(row['dimensions'][key]['evidence']), 'criterion lacks '+key)
            require(meta['id'] in row['original_source'], 'wrong criterion source')
            if criterion['id'] in readiness:
                ready_path, ready=readiness[criterion['id']]
                original=criterion_text(criterion['id'],ready['criterion'])
                require(original==criterion['criterion'] and ready['objective']==criterion['objective'], 'readiness criterion drift')
                if criterion['id']=='X-T70-007-R3':
                    old_meta,_=parse_brief(ROOT/HISTORY/row['original_source'])
                    old=next(c for c in old_meta['rubric'] if c['id']==criterion['id'])
                    require(row.get('publication_predecessor_criterion')==old and row.get('publication_successor_proof')==PROOF,'retained exact predecessor criterion')
                require(ready['author_readiness']=='ready-bounded', 'partial retained criterion')
                required_teaching={p for p in ready['teaching'] if not p.startswith('assessor/')}
                require(required_teaching<=set(row['dimensions']['teaching']['evidence']), 'lost exact teaching')
                evidence=set().union(*(set(row['dimensions'][k]['evidence']) for k in ('teaching','implementation','author_validation')))
                require(set(current(ready['implementation_and_observation']))<=evidence and ready_path in evidence, 'unrelated substitute evidence')
                for p,h in ready['pins'].items(): retained_pin(p,h)
            if meta['id'] in ('X-T69-012','X-T72-010'):
                required_report=FINAL if meta['id']=='X-T69-012' else FREE
                require(required_report in row['dimensions']['author_validation']['evidence'], 'missing exact new boundary probe')
        for variant in variant_ids:
            row = by_id[variant]
            require(row['original_variants']==variant_text(body) and row['minimum_days']>=7, 'missing exact delayed obligation')
        require(root['dimensions']==aggregate([by_id[k] for k in criterion_ids+variant_ids]), 'root evidence not exact child union')
    require(len(mapping['roots'])==34, 'root count')
    origin = 'research/v5/modern/errors_ownership_remaining.json'
    registry = read(origin)['rows']
    require(len(mapping['register_aliases'])==len(registry)==20, 'modern investigation alias count')
    for original, alias in zip(registry,mapping['register_aliases']):
        ident = 'IW-'+hashlib.sha256((origin+'\n'+json.dumps(original,sort_keys=True,ensure_ascii=False)).encode()).hexdigest()[:16]
        require(alias['id']==ident and by_id[ident]['original_register_row']==original, 'register row identity')
        readiness = read(alias['readiness'])
        meta, _ = parse_brief(ROOT/'curriculum/v5/exercises'/(original['exercise']+'.md'))
        require({r['id'] for r in readiness['criteria']} == {r['id'] for r in meta['rubric']}, 'alias missing original rubric')
        for row, criterion in zip(readiness['criteria'],meta['rubric']):
            require(row['id']==criterion['id'] and row['criterion']==criterion['criterion'] and row['objective']==criterion['objective'], 'alias criterion changed')
            require(row['author_readiness']=='ready-bounded', 'unfinished alias mechanism')
        require(set(original['original_ids'])=={original['exercise'],*[r['id'] for r in meta['rubric']],*[r['id'] for r in readiness['variants']]}, 'alias delayed/rubric set')
        for path, digest in readiness['input_sha256'].items(): retained_pin(path,digest)
        for path, digest in readiness['current_reports'].items(): require(sha(path)==digest, 'stale alias report')
        expected.add(ident)
    require(set(ids)==expected, 'wrong exact original ID set')
    for row in support['updates']:
        require(set(row['dimensions'])==set(DIMENSIONS), 'dimension key set')
        require(row['dimensions']['external_review']['state']=='not-run', 'invented external review')
        require(row['dimensions']['independent_learner']['state']=='not-assessed', 'invented learner result')
        require(row['dimensions']['physical_observation']['state']=='not-claimed-separate-target-obligation', 'invented physical result')
        for dimension in row['dimensions'].values():
            for path in dimension['evidence']: require((ROOT/path).is_file(), 'non-file evidence '+path)
    values = [m for m,_ in briefs.values()]
    order = ordered(values)
    for consumer in ('X-T69-005','X-T69-010','X-T69-011'):
        require('X-T70-002' in briefs[consumer][0]['prerequisite_exercises'], 'missing producer dependency')
        require(order.index('X-T70-002')<order.index(consumer), 'consumer before producer')
    require(not any(p.startswith('X-T69-') for p in briefs['X-T70-002'][0]['prerequisite_exercises']), 'reverse prerequisite')
    producer=read('reviews/v5/expansion/modern_completion/dod_campaign.json')
    consumer=read('reviews/v5/expansion/modern_completion/dispatch_campaign.json')
    require(datetime.fromisoformat(producer['recorded_at'])<datetime.fromisoformat(consumer['recorded_at']), 'producer campaign must precede consumer')
    for p in ('assessor/dod/dod.c','labs/dod/include/dod.h'):
        require(producer['inspected_paths'][p]==consumer['inspected_paths'][p]==sha(p), 'consumer did not bind accepted current producer')
    allocator=read('research/v5/modern/ownership_allocator_audit_investigation_validation.json')
    owned=[r for r in allocator['observations'] if r.get('kind')=='actual-owned-consumer']
    require({r['profile'] for r in owned}=={c+'-'+p for c in ('gcc','clang') for p in ('debug','release','asan')}, 'missing actual owned allocator consumer profile')
    require(all(all(r.get(k) is True for k in ('zero_ID_present','create_clone_failure_sweeps','independent_backing','all_owners_drained','executed')) for r in owned), 'incomplete actual allocator composition')
    imports=[r for r in allocator['observations'] if r.get('kind')=='object-imports']
    require(imports and all(r['whole_call_graph_no_allocation_claim'] is False for r in imports), 'overstated allocator import claim')
    companion(free,'MF_CHECK:',13,('identity-shortcut','lost-rollback','overflow-publication'))
    companion(final,'MFT_CHECK:',4,('eval-failure-publication','lambda-partial-publication','application-partial-publication'))
    return dict(updates=len(ids), roots=34, criteria=143, delayed=34, register_aliases=20,
                dependency_edges_checked=3, producer_before_consumer=True,
                actual_allocator_consumer_profiles=len(owned), companion_commands=76)


def external_check(pins):
    code = ('import json,sys,hashlib,pathlib; d=json.load(sys.stdin); '
            'bad=[p for p,h in d.items() if not pathlib.Path(p).is_file() or hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()!=h]; '
            'print(json.dumps(dict(checked=len(d),mismatches=bad))); sys.exit(bool(bad))')
    command = ['wsl','-e','python3','-c',code] if os.name=='nt' else [sys.executable,'-c',code]
    process = subprocess.run(command,input=json.dumps(pins),capture_output=True,text=True,timeout=60)
    require(process.returncode==0, 'changed observed external toolchain: '+process.stdout+process.stderr)
    return json.loads(process.stdout)


def main():
    parser=argparse.ArgumentParser(); parser.add_argument('--self-test',action='store_true'); parser.add_argument('--report',required=True)
    args=parser.parse_args()
    support=read(BASE+'support.json'); mapping=read(BASE+'evidence_map.json')
    briefs={p.stem:parse_brief(p) for p in (ROOT/'curriculum/v5/exercises').glob('X-*.md')}
    free,final=read(FREE),read(FINAL)
    result=inspect(support,mapping,briefs,free,final)
    external={}
    for report in support['current_reports']: receipt(read(report),external)
    # Reuse the existing T74 guard against a new, explicitly substituted view.
    # The historical readiness and campaign files themselves are never rewritten.
    import check_object_runtime_followthrough as runtime_guard
    data=read('research/v5/modern/object_runtime_objective_readiness.json')
    for row in data['criteria']:
        row['implementation_and_observation']=[NEW_RUNTIME if p==OLD_RUNTIME else p for p in row['implementation_and_observation']]
        if OLD_RUNTIME in row['pins']:
            del row['pins'][OLD_RUNTIME]; row['pins'][NEW_RUNTIME]=sha(NEW_RUNTIME)
    result['runtime_guard']=runtime_guard.inspect(data,read(NEW_RUNTIME),read('reviews/v5/expansion/modern_completion/object_runtime_statepoints.json'))
    result['external_toolchain']=external_check(external)
    rejected=0
    if args.self_test:
        for case in range(16):
            s,m,b,f,t=copy.deepcopy((support,mapping,briefs,free,final))
            if case==0: s['updates'].pop()
            elif case==1: next(r for r in s['updates'] if 'original_criterion' in r)['original_criterion']['criterion']+=' invented'
            elif case==2: next(r for r in s['updates'] if 'original_variants' in r)['original_variants']+=' weakened'
            elif case==3: s['updates'][0]['dimensions']['independent_learner']['state']='passed'
            elif case==4: s['updates'][0]['dimensions']['physical_observation']['state']='passed'
            elif case==5: s['report_successors']={}
            elif case==6: s['source_sha256'][next(iter(s['source_sha256']))]='0'*64
            elif case==7: b['X-T69-005'][0]['prerequisite_exercises'].remove('X-T70-002')
            elif case==8: f['observations']=[r for r in f['observations'] if r.get('semantic_control')!='lost-rollback']
            elif case==9: t['oracle_rows_per_profile']=1
            elif case==10: s['updates'][0]['dimensions']['author_validation']['evidence']=['assessor/modern_followthrough']
            elif case==11: m['register_aliases'][0]['id']='IW-invented'
            elif case==12: next(r for r in s['updates'] if r['id']=='X-T69-001-R2')['dimensions']['implementation']['evidence']=['assessor/higher_order/free.c']
            elif case==13: next(r for r in s['updates'] if r['id']=='X-T72-010-R3')['dimensions']['author_validation']['evidence'].remove(FREE)
            elif case==14: s.pop('publication_successor')
            else: next(r for r in s['updates'] if r['id']=='X-T70-007-R3')['publication_predecessor_criterion']['criterion']+=' invented'
            try: inspect(s,m,b,f,t)
            except (ValueError,KeyError): rejected+=1
            else: raise ValueError('Accepted guard negative '+str(case))
    require(not validate_publication_proof(),'publication changed during audit')
    for p in (MANIFEST,PROOF,TOOL):
        require(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==sha(p),'publication identity changed during audit')
    result.update(passed=True,status='passed',source_sha256=sha(BASE+'support.json'),
                  current_reports=len(support['current_reports']), source_files=len(support['source_sha256']),
                  negative_controls_rejected=rejected,
                  original_execution_reports_rewritten=False,
                  whole_source_execution_current=not PUBLICATION_DIFFERENCES,
                  historical_publication_differences=PUBLICATION_DIFFERENCES,
                  limits=COMMON_LIMITS)
    output=ROOT/args.report; output.parent.mkdir(parents=True,exist_ok=True)
    output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result))
    return 0


if __name__=='__main__':
    raise SystemExit(main())
