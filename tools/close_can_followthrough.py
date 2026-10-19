"""Bind exactly the verified can criteria without rewriting historical contracts."""
from pathlib import Path
import argparse,importlib.util,subprocess,json
from common import ROOT,read_json,dump,sha
from expansion_evidence import require_batch_research
from check_can_followthrough import validate,PUBLIC,OBJECTIVES

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args();folder=ROOT/'research/v5/expansion/legacy_can';require_batch_research(folder);report=read_json(ROOT/a.report);errors=validate(report)
 if errors:raise SystemExit(errors)
 guardpath='reviews/v5/expansion/legacy_can/support_validation.json';guard=read_json(ROOT/guardpath)
 assert guard['passed'] and guard['input_sha256'][a.report]==sha(ROOT/a.report) and guard['input_sha256']['tools/check_can_followthrough.py']==sha(ROOT/'tools/check_can_followthrough.py')
 canonical='curriculum/v5/expansion/legacy_assessments/PR-T24-06.json';record=read_json(ROOT/canonical);supportpath='research/v5/expansion/legacy_can/support.json';obs={(r['objective'],r['variant']):r for r in report['observations']};criteria=[];updates=[]
 for row in record['criteria']:
  oid=row['objective']
  if oid not in OBJECTIVES:continue
  teaching='labs/hosted_systems/can_depth/CAN.md';analysis='assessor/hosted_systems/can_depth/ANALYSIS.md';criteria.append(dict(id=row['id'],objective=oid,project=record['project'],original_source=canonical,ordinary=obs[oid,False],changed=obs[oid,True],teaching=teaching,assessor_analysis=analysis,receipt=a.report))
  for ident,changed in [(row['id'],False),*[(v['id'],True) for v in row['variants']]]:
   updates.append(dict(id=ident,original_source=canonical,original_objective=oid,project=record['project'],closure_rationale=obs[oid,changed]['claim'],dimensions={'teaching':dict(state='complete-for-declared-criterion',evidence=[teaching,canonical,'labs/hosted_systems/can_depth/CONTRACTS.md']),'implementation':dict(state='complete-for-declared-reference-contract',evidence=['assessor/hosted_systems/can_depth/frames.c','labs/hosted_systems/can_depth/tests.c','labs/hosted_systems/can_depth/check_bench.py','labs/hosted_systems/can_depth/BENCH.md','assessor/hosted_systems/can_depth/campaign.py',analysis]),'author_validation':dict(state='passed-with-stated-scope',evidence=[a.report,guardpath]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed; original minimum7days unchanged',evidence=[]),'physical_observation':dict(state='not-observed; exact two-node/controller/transceiver/manual/instrument prerequisites pending',evidence=[])},required_environment='Hosted C17 GCC/Clang Linux headers and binutils; original frame adapter only. No socket, interface mutation or physical connection.',validation_method='Exact frame boundaries, independent tuple oracle, compiled semantic controls, mathematical bench checks and unfilled hardware-plan rejection; real learner export.',evidence_limits=report['limits']))
 # The attachment is the canonical regeneration input; write it before applying
 # the pure hook, then pin the resulting additive project record.
 support=dict(schema_version=1,required=True,scope='One exact T24.10.c criterion and authored changed-condition bench packet; no physical observation, selected-device register configuration or independent attempt is closed',criteria=criteria,updates=updates,source_sha256={})
 dump(ROOT/supportpath,support)
 spec=importlib.util.spec_from_file_location('can_attach',folder/'author_support.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);attached=module.attach(record);assert module.attach(json.loads(json.dumps(attached)))==attached;dump(ROOT/canonical,attached)
 # Preserve the original complete criterion objects, apart from the additive
 # successor pointer. This is stronger than preserving just ID/title strings.
 baseline=json.loads(subprocess.check_output(['git','show','HEAD:'+canonical],cwd=ROOT,text=True,encoding='utf8'))
 stripped=json.loads(json.dumps(attached))
 for r in stripped['criteria']:r.pop('successor_support',None)
 for r in baseline['criteria']:r.pop('successor_support',None)
 assert stripped['criteria']==baseline['criteria'],'Original criteria changed'
 pins=[a.report,guardpath,canonical,'tools/close_can_followthrough.py','research/v5/expansion/legacy_can/author_support.py','research/v5/expansion/legacy_can/resources.json','research/v5/expansion/legacy_can/source_access.json']+PUBLIC
 support['source_sha256']={p:sha(ROOT/p) for p in pins};dump(ROOT/supportpath,support)
 dump(ROOT/'reviews/v5/expansion/legacy_can/integration.json',dict(passed=True,criteria_preserved=3,exact_changed_variants_preserved=3,pure_attachment_idempotent=True,input_sha256={p:sha(ROOT/p) for p in [canonical,supportpath,'research/v5/expansion/legacy_can/author_support.py']},scope='Original criterion bodies preserved and attachment idempotent; no learner execution claimed'))
 print(dict(criteria=1,dispositions=2,whole_topic_complete=False))
if __name__=='__main__':main()
