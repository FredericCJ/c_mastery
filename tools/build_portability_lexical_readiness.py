"""Map only six original stage-analysis tasks to exact current evidence."""
from common import ROOT,dump,read_json,sha
from check_portability_lexical import check,REPORT,EXPORT,BASE
def main():
 d,e=[read_json(ROOT/p) for p in [REPORT,EXPORT]];check(d,e)
 claims={
 'a':'Actual preserved physical source,preprocessed token output and execution demonstrate splice-before-comment ordering in eight GCC/Clang edition/optimization cells;private phase derivation and unfinished predictions are separate.',
 'a-DELAYED':'Same trigraph source executes under C17 and C23 modes with expected1 versus9,connected to actually inspected earlier/later phase1 draft passages and exact tool versions. Published-edition access remains explicit.',
 'b':'Actual comment-separated identifier source expands to two tokens,while VALUE remains unchanged inside the literal;runtime contents and terminators independently check the semantic prediction.',
 'b-DELAYED':'Actual macro-produced and direct adjacent literals are captured separately in preprocessing and combined in the executed object,with contents/size/terminating-zero assertions and a stage-specific private explanation.',
 'c':'Original malformed generated declaration preprocesses successfully,then receives an actual semantic diagnostic. Configuration-only repair and separate successful stages expose introduction versus diagnosis,with source snapshots retained.',
 'c-DELAYED':'Actual compiler depfiles drive initial/unchanged/header-only rounds,rehashing declared inputs. Only a.c rebuilds;preserved objects and returns15/15/19 are bound. A separate successfully linked stale-object control retains15,refuting link-success sufficiency.'}
 prefix='AX-PR-T06-01-T06.01.';reports=[REPORT,EXPORT]
 teaching=['curriculum/v5/expansion/legacy_assessments/PR-T06-01.json','labs/language_portability/lexical/CONTRACTS.md','assessor/language_portability/lexical/ANALYSIS.md']
 implementation=[p.relative_to(ROOT).as_posix() for root in ['labs/language_portability/lexical','assessor/language_portability/lexical'] for p in (ROOT/root).iterdir() if p.is_file()]+['tools/run_portability_lexical.py']
 ready=BASE+'/lexical_readiness.json';dims=dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=reports+[ready]),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run',evidence=['labs/language_portability/lexical/CONTRACTS.md']))
 rows=[dict(id=prefix+i,author_readiness='ready-bounded',demonstration=t,teaching=teaching,implementation=implementation,validation=reports,external_review='not-run',independent_learner='not-assessed',physical_observation='not-run') for i,t in claims.items()]
 inputs={**d['input_sha256'],**e['input_sha256']}
 for p in teaching+implementation+reports+[BASE+'/lexical_resources.json',BASE+'/lexical_research_gate.json','tools/check_portability_lexical.py','tools/register_portability_lexical.py','tools/build_portability_lexical_readiness.py','tools/check_replicated_replay_depth.py']:inputs[p]=sha(ROOT/p)
 dump(ROOT/ready,dict(schema_version=1,criteria=rows,input_sha256=inputs,current_reports={r:sha(ROOT/r) for r in reports},limits=['Six exact T06.01 ordinary/delayed tasks;remaining T06 tasks remain active.','Source reasoning and diagnosis laboratory,not a complete C preprocessor or fabricated stub implementation.','Private answer comparison does not establish reasoning or independent learner mastery;published editions remain inaccessible.']))
 inputs[ready]=sha(ROOT/ready);dump(ROOT/BASE/'lexical_completion_dispositions.json',dict(schema_version=1,updates=[dict(id=prefix+i,dimensions=dims,closure_rationale=t+' Bounded author support;independent learner/external review remains unassessed.') for i,t in claims.items()],current_reports=reports,input_sha256=inputs));print(dict(passed=True,updates=len(rows)))
if __name__=='__main__':main()
