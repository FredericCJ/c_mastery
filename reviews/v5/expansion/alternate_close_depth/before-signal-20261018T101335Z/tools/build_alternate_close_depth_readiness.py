"""Preserve the blocked alternate-OS execution dimension instead of closing it."""
from pathlib import Path
import copy,hashlib,json
from check_alternate_close_depth import validate,REPORT,BASE
ROOT=Path(__file__).resolve().parents[1]
def read(p):return json.loads((ROOT/p).read_text())
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def main():
 report=read(REPORT);assert not validate(report)
 guard='reviews/v5/expansion/alternate_close_depth/support_validation.json';g=read(guard);assert g['passed'] and len(g['negative_controls'])==20 and all(x['rejected'] for x in g['negative_controls'])
 for p,h in g['input_sha256'].items():assert sha(p)==h,p
 card='curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';c=next(x for x in read(card)['criteria'] if x['objective']=='T23.04.b');v=c['variants'][0]
 assert v['id']=='AX-PR-T23-02-T23.04.b-DELAYED' and v['minimum_days']==7
 artifacts=['labs/alternate_close_depth/CONTRACTS.md','labs/alternate_close_depth/QNX_TARGET.md','assessor/alternate_close_depth/close_owner.c','assessor/alternate_close_depth/ANALYSIS.md']
 blocker=dict(id='QNX8-SDK-GUEST-UNAVAILABLE',status='unavailable',required='Legitimately provisioned QNX SDP8 qcc/sysroot and disposable QNX8 guest with an authorized deployed license',observed=report['target_build'],remaining='Actually compile the target source, repair any SDK/framework incompatibility, and capture the actual libc/resource-manager EINTR retention path. A signal-caused interruption is a separate unexecuted trigger investigation.',not_substituted=['Linux host fault seam','POSIX edition model','QNX documentation inspection','Source-only or host-header compile'])
 dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:2]),('implementation','partial',artifacts),('author_validation','partial',[REPORT,guard]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-applicable',[])]}
 rows=[]
 for ident in [v['id'],'IW-e2cdc87b21fb7b31']:
  rows.append(dict(id=ident,objective='T23.04.b',parent_project='PR-T23-02',original_task=v['task'],minimum_days=7,dimensions=copy.deepcopy(dims),closure_rationale='Revised exclusive-owner implementation, independent1305-case/profile table,36 compiled controls, real Linux fd/cancellation seam and isolated export are complete. QNX8 target source/build/capture support is authored but actual SDK compilation and alternate-OS execution remain blocked and unclaimed.',artifacts=artifacts,current_reports=[REPORT,guard],external_blockers=[blocker],remaining_author_work=[blocker['remaining']],validation_method='Six actual C17 profiles and20 evidence guards; no target result inferred from host outputs.'))
 files=set(report['input_sha256'])|{REPORT,guard,card,'tools/register_alternate_close_depth.py','tools/check_alternate_close_depth.py','tools/build_alternate_close_depth_readiness.py'}
 out=dict(schema_version=1,scope='Exact delayed alternate-OS close obligation and original inventory record; deliberately partial until actual target evidence',source_sha256={p:sha(p) for p in sorted(files)},current_reports=[REPORT,guard],updates=rows,external_blockers=[blocker],summary=dict(host_commands=131,profiles=6,oracle_cases_per_profile=1305,compiled_semantic_controls=36,evidence_guards=20,actual_alternate_os_executions=0))
 path=ROOT/BASE/'completion_dispositions.json';path.write_text(json.dumps(out,indent=2)+'\n');print('PASS two exact partial dispositions; QNX SDK/guest and execution blocker preserved')
if __name__=='__main__':main()
