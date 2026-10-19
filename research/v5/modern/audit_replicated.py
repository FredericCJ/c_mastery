"""Keep T76 authored depth, actual anchor evidence and remaining work distinct."""
import json
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[3]))
from cmastery import exercises
from render_ad_mp import R,read,dump,write
from author_replicated import DATA

def main():
    campaign_path='reviews/v5/expansion/b07/replicated_campaign.json'
    campaign=read(campaign_path) if (R/campaign_path).exists() else {}
    experiments_path='reviews/v5/expansion/b07/replicated_experiments.json'
    experiments=read(experiments_path) if (R/experiments_path).exists() else {}
    mapping=[];gaps=[]
    for i,d in enumerate(DATA,1):
        xid=f'X-T76-{i:03}';pid=f'PR-T76-{i:02}'
        meta,body=exercises.parse_brief(R/f'curriculum/v5/exercises/{xid}.md');exercises.validate(meta,body)
        for objective,criterion,facet in zip(meta['objectives'],meta['rubric'],meta['objective_coverage']):
            mapping.append(dict(objective=objective,source_topic=meta['source_records'],teaching=f'curriculum/v5/exercises/{xid}.md',exercise=xid,learner_project=pid,learner_artifacts=facet['implementation'],assessment=criterion,facets=facet,implementation_status='Authored reference and compiling unsolved learner scaffold; author anchor evidence only when current source pins match.',executed_scope=dict(public_anchor=f'replicated.{d["case"]}',campaign=campaign_path,author_campaign_passed=campaign.get('passed',False)),independent_assessment='not-attempted; delayed variant remains required',limitations=['The complete criterion includes reasoning/transfer obligations beyond the finite anchor.','Passing author tests is neither a learner receipt nor independent reviewer evidence.']))
        gaps.append(dict(id=f'G-T76-{i:02}-DELAYED',objective_ids=meta['objectives'],kind='independent-assessment',required=True,status='not-attempted',remaining_work=d['transfer'],closure='Actual delayed unaided learner artifacts with per-objective rubric judgments and assistance disclosure; no fabricated reviewer independence.'))
        gaps.append(dict(id=f'G-T76-{i:02}-INVESTIGATION',objective_ids=meta['objectives'],kind='unimplemented-or-unvalidated-full-investigation',required=True,status='authored-not-executed-as-a-complete-investigation',remaining_work=d['evidence']+' '+d['transfer'],closure='Complete the whole stated experiment or changed-condition implementation; anchor evidence closes only its named finite checks.'))
    gaps += [dict(id='G-T76-'+edition+'-SOURCE',kind='unavailable-reference',required=True,status='unavailable',remaining_work='Inspect published ISO '+edition+' wording for any claimed edition difference; do not infer it from a compiler mode.',closure='Named actually inspected clauses and bounded edition comparison.') for edition in ['C17','C23']]
    gaps.append(dict(id='G-T76-COST-MEASUREMENTS',kind='unvalidated-experiment',required=True,status='partially-executed',remaining_work='Object sizes/static stack and dependency symbols are recorded; equivalent-contract latency, traffic, contention, durability and remote-delivery cost investigations remain learner work.',closure='Reproducible platform/workload measurements with uncertainty and correctness-equivalent alternatives.'))
    dump('research/v5/modern/replicated_depth_audit.json',dict(domain='T76',projects=9,leaf_objectives=36,baseline='No accepted project/unit identities or historical receipts changed.',expansion_reason='The modern plan named distributed/event-state mechanisms but did not establish concrete original C representations, transactional failure cuts, independent oracles or delayed assessments.',objective_map=mapping,source_scope='Eleven registered bounded primary passages; source inspection is author work, not independent review.',producer_integration=dict(executed=['persist.hamt via real dod source; event snapshots and two-update checkpoint'],required_unexecuted=['storage.pages durable log/snapshot adapters','codec.records and io.frame transport adapters','ipc.local retry/receiver integration','prior T20.03 learner artifact composition']),validation=dict(campaign=campaign_path,experiments=experiments_path,author_campaign_passed=campaign.get('passed',False),bounded_experiments_passed=experiments.get('passed',False)),comprehensive_complete=False))
    dump('research/v5/modern/replicated_gap_ledger.json',dict(domain='T76',gaps=gaps,comprehensive_complete=False,rule='Authored exercises, executed anchors, independent assessment and physically validated work are separate states.'))
    dump('research/v5/modern/replicated_remaining_work.json',dict(domain='T76',status='authored-with-source-bound-host-validation',comprehensive_complete=False,required_gap_ids=[g['id'] for g in gaps],preserved_prior_scope=['research/v5/modern/ad_mp_gap_ledger.json','research/v5/expansion/b06/gap_ledger.json'],limits='This domain does not close earlier corpus/depth gaps, durable distributed protocol investigations or learner assessment.'))
    # Apply the separately source-checked per-criterion successor evidence.
    # A missing report retains the authored-only state; a stale existing report
    # fails its guard instead of manufacturing acceptance from an old campaign.
    if (R/'research/v5/modern/replicated_build_cost_validation.json').exists():
        import subprocess
        subprocess.run([sys.executable,str(R/'tools/build_replicated_readiness.py')],check=True,cwd=R)
    print(json.dumps(dict(projects=9,leaf_mappings=len(mapping),required_gaps=len(gaps),author_campaign_passed=campaign.get('passed',False),experiments_passed=experiments.get('passed',False))))

if __name__=='__main__':main()
