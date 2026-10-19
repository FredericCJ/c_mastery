"""Revalidate exact existing VM/kernel experiments; do not invent fresh execution."""
from pathlib import Path
from common import ROOT,dump,read_json,sha
from check_vm_services import validate as vm_validate,REPORT as VM
from check_kernel_followthrough import validate_guest
BASE='research/v5/expansion/legacy_storage_followthrough'
FROZEN='reviews/v5/expansion/legacy_kernel_followthrough/runtime_frozen.json'
def main():
 vm=read_json(ROOT/VM);assert not vm_validate(vm)
 frozen=read_json(ROOT/FROZEN);assert frozen['passed'] and frozen['error'] is None
 for p,h in frozen['inspected_paths'].items():assert sha(ROOT/p)==h,'stale frozen provider '+p
 inputs={**vm['input_sha256'],**frozen['inspected_paths']};reports={VM:sha(ROOT/VM),FROZEN:sha(ROOT/FROZEN)};observations=[]
 for cc in ['gcc','clang']:
  for opt,case in [('O2',i) for i in [0,30,31,32,33,34,35]]+[('O0',34)]:
   path=f'reviews/v5/expansion/legacy_kernel_followthrough/matrix_attempt4/{cc}-{opt}-case{case}-q11932-d0/result.json';r=read_json(ROOT/path);assert (r['compiler'],r['optimization'],r['case'])==(cc,opt,case);validate_guest(r)
   reports[path]=sha(ROOT/path);observations.append(dict(report=path,sha256=sha(ROOT/path),compiler=cc,optimization=opt,case=case,status='prior-actual-execution-revalidated',source_authority=FROZEN,artifact_count=len(r['artifact_sha256'])))
 for p in ['tools/bind_storage_service_providers.py','tools/check_vm_services.py','tools/run_vm_services.py','tools/check_kernel_followthrough.py','tools/freeze_kernel_followthrough.py','tools/audit_kernel_runtime_stack.py']:inputs[p]=sha(ROOT/p)
 inputs.update(reports)
 out=dict(schema_version=1,passed=True,status='current-prior-executions-revalidated',producer='tools/bind_storage_service_providers.py',input_sha256=inputs,current_reports=reports,vm_checks=203,vm_scope='Actual C17 host, M4 and big-endian provider execution; precise original report limitations retained',guest_observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['This binding revalidates prior actual executions and source/artifact identities; it does not claim a new guest run.','No broad T48 completion, physical durability, human independence or learner mastery.','Case32 without FLUSH is clean restart only.','Async queued cancellation/retention is not covered by these synchronous case bindings.'])
 dump(ROOT/BASE/'service_provider_validation.json',out);print(dict(passed=True,vm_checks=203,guest_reports=len(observations)))
if __name__=='__main__':main()
