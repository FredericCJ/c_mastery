"""Validate checkpoint contracts, source pins, model cases and isolated exports."""
from pathlib import Path
import contextlib
import copy
import hashlib
import io
import json
import subprocess
import sys
import tempfile
from unittest.mock import patch

ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));sys.path.insert(0,str(HERE))
import check_legacy_assessments as checker
import verify_integration_fixtures as retained
import verify_legacy_checkpoint_fixtures as oracle
from common import load,read_json
from cmastery import assessments
from cards import rows
from inputs import write
from expansion_evidence import require_batch_research
from v5_common import require_domain_research


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    taxonomy,_,projects,_=load()
    objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    units={u['id']:u for u in read_json(ROOT/'curriculum/v5/units.json')['units']}
    schema=read_json(checker.BASE/'schema.json')
    expected_cards=rows();checks=[];records=[];paths=set();criteria=set();leaves=set();topics=set()
    for pid,cards in expected_cards.items():
        path=checker.BASE/(pid+'.json');data=read_json(path);records.append(data)
        errors=checker.validate(data,schema,projects,objectives,units)
        actual={c['objective'] for c in data['criteria']}
        if actual!={c['objective'] for c in cards}:errors.append('selected criterion coverage differs from individually authored cards')
        if {o.rsplit('.',1)[0] for o in actual}!=set(projects[pid]['meta']['topics']):errors.append('assigned topic lacks a composition criterion')
        if len(actual)!=len(data['criteria']):errors.append('duplicate project objective')
        for criterion in data['criteria']:
            if criterion['id'] in criteria:errors.append('duplicate criterion ID')
            if criterion['source_records']!=data['source_records']:errors.append('source mapping differs from inspected scope')
            criteria.add(criterion['id']);leaves.add(criterion['objective']);topics.add(criterion['objective'].rsplit('.',1)[0])
        if pid.startswith('PR-P4') and data['additional_required_projects']!=['PR-P3-02']:errors.append('missing full protocol prerequisite')
        if pid.startswith('PR-P5') and data['additional_required_projects']!=['PR-P1-02','PR-P3-02']:errors.append('missing bounded storage/protocol prerequisites')
        checks.append(dict(project=pid,criteria=len(data['criteria']),status='passed' if not errors else 'failed',errors=errors))
        paths.add(path);paths.add(ROOT/data['assessor_analysis'])
        paths.update(ROOT/x['path'] for x in data['baseline_binding']['files'])
        paths.update(ROOT/x['path'] for x in data['source_records'])
        paths.update(ROOT/x for x in data['shared_contracts']+data['supplied_artifacts'])

    controls=[]
    changes=[('missing-title',lambda d:d['criteria'][0].pop('title')),
             ('wrong-integration-domain',lambda d:d.update(domain='T14')),
             ('changed-baseline',lambda d:d['baseline_binding']['files'][0].update(sha256='0'*64)),
             ('foreign-objective',lambda d:d['criteria'][0].update(objective='T01.01.a')),
             ('missing-assessor-anchor',lambda d:d['criteria'][0].update(assessor_anchor='#missing')),
             ('undelayed-transfer',lambda d:d['criteria'][0]['variants'][0].update(minimum_days=0)),
             ('uninspected-passage',lambda d:d['source_records'][0].update(passages=['invented passage'])),
             ('assessor-export',lambda d:d['supplied_artifacts'].append('assessor/integration/integration.c')),
             ('traversal-export',lambda d:d['supplied_artifacts'].append('labs/integration/../../assessor/integration/integration.c'))]
    for name,change in changes:
        data=copy.deepcopy(records[0]);change(data)
        errors=checker.validate(data,schema,projects,objectives,units)
        controls.append(dict(name=name,status='passed' if errors else 'failed',observed_rejection=errors))
    original=checker.read_json
    def unavailable(path):
        value=original(path)
        if isinstance(value,dict) and 'resources' in value:
            value=copy.deepcopy(value)
            for source in value['resources']:source['inspection']['status']='unavailable'
        return value
    with patch.object(checker,'read_json',side_effect=unavailable):
        errors=checker.validate(copy.deepcopy(records[0]),schema,projects,objectives,units)
    controls.append(dict(name='explicitly-unavailable-source',status='passed' if any('uninspected' in e for e in errors) else 'failed',observed_rejection=errors))
    fixture_checks=oracle.observations(ROOT/'fixtures/legacy_checkpoints')
    fixture_controls=[]
    wrong_claims={
        'arena':('commit-failed-allocation',lambda d:d.update(final_offset=33)),
        'active-transfer':('replay-accepted-byte',lambda d:d.update(sink=[65,65,66])),
        'wire-recovery':('skip-nested-valid-frame',lambda d:d.update(recovered=d['recovered'][1:])),
        'dma-quiescence':('stale-completion-quiesces-live-generation',lambda d:d['trace'][0].update(quiescent=True)),
        'cancel-ownership':('duplicate-completion-releases-twice',lambda d:d.update(releases=2)),
        'fir-chunks':('reset-history-at-each-chunk',lambda d:d.update(outputs=['0','1','4','1','2']))}
    for path in sorted((ROOT/'fixtures/legacy_checkpoints').glob('P*.json')):
        data=read_json(path);bad=copy.deepcopy(data['expected'])
        name,change=wrong_claims[data['oracle']];change(bad)
        fixture_controls.append(dict(fixture=path.name,wrong_claim=name,rejected=oracle.compute(data['oracle'],data['input'])!=bad))
    stream=io.StringIO()
    with contextlib.redirect_stdout(stream):retained.main()
    retained_observation=dict(status='passed',output=stream.getvalue().strip(),scope='Seven retained fixture computations; six belong to these checkpoints and the unchanged seventh is the capstone recovery model. No target execution.')

    exports=[];export_execution=None
    with tempfile.TemporaryDirectory(prefix='cmastery checkpoint export ') as temp:
        temporary=Path(temp).resolve();assert not temporary.is_relative_to(ROOT.resolve())
        for data in records:
            work=temporary/data['project'];work.mkdir()
            receipt=assessments.export(ROOT,data['project'],work)
            assert receipt['learner_assessment']=='not-assessed'
            body=(work/'ASSESSMENT.md').read_text(encoding='utf-8')
            assert all(c['id'] in body and c['task'] in body and c['rubric']['2'] in body for c in data['criteria'])
            assert all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in data['supplied_artifacts']+data['shared_contracts'])
            assert not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))
            assert 'Assessor only. Authored analysis' not in body
            exports.append(dict(project=data['project'],status='passed',criteria=len(data['criteria']),all_assets_exact=True,assessor_bodies_absent=True))
            if export_execution is None:
                command=[sys.executable,str(work/'tools/verify_legacy_checkpoint_fixtures.py')]
                run=subprocess.run(command,cwd=work,capture_output=True,text=True,encoding='utf-8')
                result=json.loads(run.stdout)
                assert run.returncode==0 and result['status']=='passed'
                export_execution=dict(command=['<current-python>','<temporary-export>/tools/verify_legacy_checkpoint_fixtures.py'],exit_code=run.returncode,checks=len(result['checks']),status=result['status'],stderr=run.stderr)
    gate=read_json(HERE/'research_gate.json')
    stale=[p for p,h in gate['research_input_sha256'].items() if sha(ROOT/p)!=h]
    controls.append(dict(name='research-gate-current',status='passed' if not stale and gate['status']=='passage-inspected' else 'failed',stale_paths=stale))
    require_batch_research(HERE)
    require_domain_research(dict(domains={key:read_json(ROOT/'curriculum/v5/domains'/(key+'.json'))['domain'] for key in ['T62','T64']}))
    controls.append(dict(name='shared-batch-and-prior-domain-gates',status='passed',
                         batch_validator='tools.expansion_evidence.require_batch_research',
                         prior_domain_validator='tools.v5_common.require_domain_research for T62 and T64',
                         prior_domain_gate='research/v5/expansion/b06/research_gate.json'))
    baseline_paths=sorted({b['path'] for r in records for b in r['baseline_binding']['files'] if not b['path'].startswith(('labs/ownership/','labs/errors/'))})
    command=['git','diff','--quiet','HEAD','--']+baseline_paths
    baseline=subprocess.run(command,cwd=ROOT,capture_output=True,text=True)
    controls.append(dict(name='accepted-baseline-unchanged',status='passed' if baseline.returncode==0 else 'failed',command=command,exit_code=baseline.returncode,stderr=baseline.stderr))
    paths.update(HERE.glob('*.py'));paths.update(HERE.glob('*.json'));paths.discard(HERE/'validation.json')
    paths.update([HERE/'AUDIT.md',checker.BASE/'schema.json',ROOT/'tools/check_legacy_assessments.py',ROOT/'tools/common.py',ROOT/'cmastery/assessments.py'])
    paths.update([ROOT/'tools/expansion_evidence.py',ROOT/'tools/v5_common.py',ROOT/'research/v5/expansion/b06/research_gate.json'])
    paths.update(ROOT/'curriculum/v5/domains'/(key+'.json') for key in ['T62','T64'])
    passed=(len(records)==12 and len(criteria)==49 and len(leaves)==48 and len(topics)==36
            and all(c['status']=='passed' for c in checks+controls+exports)
            and all(c['passed'] for c in fixture_checks) and all(c['rejected'] for c in fixture_controls))
    report=dict(status='passed' if passed else 'failed',command='.conda/python.exe -X utf8 research/v5/expansion/legacy_checkpoints/validate.py',
                counts=dict(projects=len(records),criteria=len(criteria),selected_objectives=len(leaves),topics=len(topics),new_host_models=6,retained_fixture_checks=7,negative_schema_controls=10,negative_fixture_controls=6,temporary_exports=12,exported_oracle_executions=1),
                contracts=checks,controls=controls,host_models=fixture_checks,fixture_negative_controls=fixture_controls,retained_models=retained_observation,
                learner_exports=exports,exported_oracle=export_execution,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},
                limitations=['Structural/source/oracle/export checks only. No full new checkpoint C implementation, target campaign, physical measurement, learner attempt, delayed assessment or independent human review is inferred.',
                             'Host wire parsing is a whole-buffer contract oracle; partition-count arithmetic is not execution of a streaming parser.',
                             'Actual startup, IRQ/DMA, Linux concurrency, SIMD and exact uninspected primary contracts remain required in remaining_work.json.'])
    write(HERE/'validation.json',report)
    print(json.dumps({k:v for k,v in report.items() if k in ['status','command','counts','controls','exported_oracle','limitations']},ensure_ascii=False))
    return not passed


if __name__=='__main__':
    sys.stdout.reconfigure(encoding='utf-8')
    raise SystemExit(main())
