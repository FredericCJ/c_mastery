"""Guard exact reuse of prior VM and synchronous kernel executions."""
import copy,json
from check_replicated_replay_depth import ROOT,sha
REPORT='research/v5/expansion/legacy_storage_followthrough/service_provider_validation.json'
def check(d,files=True):
 assert d['passed'] and d['status']=='current-prior-executions-revalidated' and d['vm_checks']==203
 assert d['input_sha256'] and d['current_reports'] and len(d['guest_observations'])==16
 expected={(c,'O2',i) for c in ['gcc','clang'] for i in [0,30,31,32,33,34,35]}|{(c,'O0',34) for c in ['gcc','clang']}
 assert {(r['compiler'],r['optimization'],r['case']) for r in d['guest_observations']}==expected
 assert all(r['status']=='prior-actual-execution-revalidated' and r['artifact_count']>0 and r['source_authority']=='reviews/v5/expansion/legacy_kernel_followthrough/runtime_frozen.json' and d['current_reports'][r['report']]==r['sha256'] for r in d['guest_observations'])
 assert d['physical_validation']=='not-run' and d['independent_review']=='not-run' and d['learner_mastery']=='not-assessed'
 if files:
  from check_vm_services import validate as vm_validate,REPORT as VM
  from check_kernel_followthrough import validate_guest
  for p,h in {**d['input_sha256'],**d['current_reports']}.items():assert sha(ROOT/p)==h,'stale provider '+p
  assert not vm_validate(json.loads((ROOT/VM).read_text()))
  for r in d['guest_observations']:validate_guest(json.loads((ROOT/r['report']).read_text()))
 return True
def selftest(d):
 edits=[lambda a:a.update(passed=False),lambda a:a.update(vm_checks=1),lambda a:a['guest_observations'].pop(),lambda a:a['guest_observations'][0].update(status='new-execution'),lambda a:a['guest_observations'][0].update(case=99),lambda a:a['guest_observations'][0].update(artifact_count=0),lambda a:a.update(current_reports={}),lambda a:a.update(physical_validation='passed')]
 for edit in edits:
  a=copy.deepcopy(d);edit(a)
  try:check(a,False)
  except (AssertionError,KeyError):continue
  raise AssertionError('accepted unsupported provider reuse')
 return len(edits)
