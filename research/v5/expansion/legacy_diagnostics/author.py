"""Render reviewed, individually authored assessment cards; never edit historical sources."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[4]/'tools'))
from common import ROOT,dump,read_json,sha,load,write
folder=ROOT/'research/v5/expansion/legacy_diagnostics'
resources=read_json(folder/'resources.json')['resources']
security_scopes=read_json(folder/'security.json') if (folder/'security.json').exists() else {}
taxonomy,modules,projects,_=load()
objective={o['id']:o['text'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
units=read_json(ROOT/'curriculum/v5/units.json')['units']
support={}
for f in (ROOT/'labs').glob('*/family.json'):
 for r in read_json(f).get('projects',[]):
  if isinstance(r,dict):support.setdefault(r.get('id'),[]).append((f.parent.name,r))
base=ROOT/'curriculum/v5/expansion/legacy_assessments'

def author(domain, rows):
    records={}
    for line in rows.strip().splitlines():
        fields=line.split('|')
        assert len(fields)==8,(len(fields),line)
        suffix,fams,task,misconception,expected,variant,hint,analysis=fields
        oid=domain+'.'+suffix
        assert oid in objective,oid
        records[oid]=(fams,task,misconception,expected,variant,hint,analysis)
    assert set(records)=={k for k in objective if k.startswith(domain+'.')}, (domain,len(records))
    for pid,p in projects.items():
        if p['meta']['module']!=domain:continue
        meta=p['meta'];keys=[k for k in records if k.rsplit('.',1)[0] in meta['topics']]
        inspected=[f'curriculum/modules/{domain}.md',f'curriculum/projects/{pid}.md',*meta['fixtures']]
        bindings=[dict(path=name,sha256=sha(ROOT/name),passages=['Teaching text; project Contract, Inputs, Steps and Hints; complete fixture data']) for name in inspected]
        sources=[dict(id=r['id'],path='research/v5/expansion/legacy_diagnostics/resources.json',passages=r['locators']) for r in resources if domain in r['domains']]
        anchors=support.get(pid,[])
        s={'tests':sorted({t for _,row in anchors for t in row.get('tests',[])}),'scope':'; '.join(name+': '+row.get('scope','Declared anchor only') for name,row in anchors),'remaining_evidence':'Complete criterion-specific C, diagnosis and changed-condition evidence remain required.'}
        data=dict(schema_version=1,project=pid,domain=domain,title=meta['title']+' - explicit objective assessment',shared_contracts=['curriculum/v5/expansion/legacy_assessments/shared-contract.md'],baseline_binding=dict(files=bindings,teaching=[dict(path=inspected[0],anchors=meta['topics'])],units=[u['id'] for u in units if u['project']==pid],inspection_limits=['Unit identity/mapping inspected; this wave does not claim fresh reading of every unit body.','Named anchor implementations are separately scoped; a project family binding alone is not full exercise execution.','No fresh execution of the legacy implementation campaign in this audit.']),environment=dict(language='ISO C17 active baseline; C11 draft support must retain its edition limits',target='Parent target/profile: hosted Linux or original disposable QEMU kernel/Cortex-M teaching firmware; physical routes require their own evidence',profile=meta['profile'],dependencies='Parent boundary applies. C components use permitted local primitives; hosted verification and packaging tools are separate. Audit generated helpers and link closure before any strict freestanding claim.',toolchain=['Record actual GCC/Clang, CMake, Python and Git versions when used.','NDEBUG on/off and O0/O2 for relevant semantic/diagnostic comparisons.']),source_records=sources,reference_gaps=['Exact published C17 not inspected','Only registered bounded source passages are claimed; historical book inspections remain separate','Hardware, foreign ABI and dependency license claims require their exact selected authorities'],supplied_artifacts=inspected,execution_support=dict(kind='Retained partial anchors and criterion-specific new assessment tasks',scope=s.get('scope','Project contract and mathematical fixture only; no named C anchor')+' '+s.get('remaining_evidence','Complete learner implementation/analysis and independent verification remain required.'),tests=s.get('tests',[]),fresh_execution='not-run for these supplemental tasks'),criteria=[],assessor_analysis='assessor/legacy_depth/'+pid+'.md',status=dict(content='authored additive assessment',execution='not-run for supplemental tasks',independent_review='not-run',learner_assessment='not-assessed',limitations=['Baseline text, IDs, hours and historical receipts preserved.','Finite controls do not prove absolute mastery.','Each exact changed-condition experiment remains required.']))
        if pid in {'PR-T18-01', 'PR-T19-01'}:
            data['supplied_artifacts'].append('fixtures/api_depth/contracts.json')
        solution=['# '+pid+' supplemental assessor analysis','','Assessor-only; author reasoning, not independent or learner evidence.','']
        for oid in keys:
            fams,task,misconception,expected,variant,hint,analysis=records[oid]
            cid='AX-'+pid+'-'+oid;aid=oid.lower().replace('.','')
            criterion=dict(id=cid,title=objective[oid],objective=oid,families=list(fams),task=task,purpose=misconception,prerequisites=dict(knowledge=[domain+' parent teaching and exact project contract','Representation, lifetime and failure guarantees of imported artifacts'],artifacts=[pid+' learner-owned implementation/dossier and recorded initial fixture prediction']),reasoning=['Derive the stated result from pre/postconditions; name the first state at which the incorrect explanation fails.',misconception,'Account for ownership, failure preservation and resource/complexity bounds where the task changes them; justify genuine non-applicability.'],deliverables=dict(D1='Prediction and causal explanation addressing: '+misconception,D2=task,D3='Independent counterexample and source-bound evidence for: '+expected,D4=variant),verification=dict(procedure='Predict before execution. Retain complete observable state, not just return codes. Compare the stated expected relation with the original fixture and independently constructed cases. Preserve exact commands/tool/source identities for experiments; reason about undefined cases without executing them as expected-output tests.',expected=expected,**{'pass':'All stated relations hold with a correct causal argument; a matching output with false lifetime/language reasoning fails.','limits':'This criterion is newly authored. The listed legacy anchor does not assess all of its obligations; no fresh result is inferred.'}),rubric={'0':'Missing or incorrect contract/result.','1':'Reproduces a nominal example but cannot diagnose the stated misconception.','2':expected+' Explains the mechanism and supplies independent evidence.','3':variant+' Derives changed invariants without copying the previous solution.'},hints=[hint,'Write the earliest changed ownership, state, dependency or observation explicitly; compare that point before comparing final outputs.'],variants=[dict(id=cid+'-DELAYED',minimum_days=7,task=variant)],assessor_anchor='#'+aid,facet_scope=dict(teaching='Retain inspected '+oid.rsplit('.',1)[0]+' teaching and the bound project contract.',implementation_or_analysis='Existing learner artifact plus the exact new task; no unrelated test is promoted to whole-leaf coverage.',boundary_adversarial='The named misconception, independent counterexample and changed assumption are explicit assessment obligations.',independent_assessment='Objective-specific 0-3 criterion and separate assessor analysis authored; not administered.',remaining='Execute or independently review all task-specific evidence and delayed transfer; exact reference/target gaps remain.'),security=dict(applicable=False,scope='Original local learner components and synthetic fixtures only. This is a contract/diagnostic assessment, not an exploit demonstration; shared containment applies.'))
            if oid.rsplit('.',1)[0] in security_scopes:
                criterion['security']=security_scopes[oid.rsplit('.',1)[0]]
            data['criteria'].append(criterion)
            solution+=['<a id="'+aid+'"></a>','## '+oid,'',analysis,'','Expected relation: '+expected,'','Withheld transfer discriminator: '+variant,'']
        if pid == 'PR-T19-04':
            import runpy
            attach = runpy.run_path(str(ROOT/'research/v5/expansion/legacy_stream_followthrough/attach.py'))['attach']
            data = attach(data)
        if domain == 'T46':
            import runpy
            attach_vm = runpy.run_path(str(ROOT/'research/v5/expansion/legacy_vm_representation/author_support.py'))['attach']
            data = attach_vm(data)
        dump(base/(pid+'.json'),data);write(ROOT/data['assessor_analysis'],'\n'.join(solution))


if __name__=='__main__':
    for domain in sys.argv[1:]:
        author(domain,(folder/(domain+'.cards')).read_text(encoding='utf-8'))
