"""Regress integrated acceptance against the actual current object-runtime schema."""
import argparse,copy,json
from common import ROOT,sha,dump
from check_expansion_campaigns import object_runtime_errors,pin_errors,execution_errors
REPORT='reviews/v5/expansion/modern_followthrough/object_runtime_current_01.json'
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',default='reviews/v5/expansion/followthrough/object_runtime_route_guard.json');a=p.parse_args()
    meta=json.loads((ROOT/'labs/object_runtime/family.json').read_text());d=json.loads((ROOT/REPORT).read_text())
    errors=pin_errors(d)+execution_errors(d)+object_runtime_errors(meta,d);controls=[]
    changes=[('seventh-profile',lambda r:r['profiles'].pop()),
      ('anchor',lambda r:r['profiles'][0]['anchors'].pop()),
      ('causal-control',lambda r:r['profiles'][0]['controls_by_anchor'].clear()),
      ('provider',lambda r:r['inspected_paths'].pop('assessor/runtime/cvm.c')),
      ('compiler-identity',lambda r:r['compiler_evidence'].update(sha256='0'*64)),
      ('host-identity',lambda r:r['host_evidence'].update(sha256='0'*64)),
      ('wrapper',lambda r:r.update(wrapper_executions=[])),
      ('false-target-execution',lambda r:r['target_audit'].update(execution='passed'))]
    for name,change in changes:
        candidate=copy.deepcopy(d);change(candidate);rejected=bool(object_runtime_errors(meta,candidate))
        controls.append(dict(name=name,rejected=rejected))
        if not rejected:errors.append('accepted '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,
        source_sha256={p:sha(ROOT/p) for p in [REPORT,'labs/object_runtime/family.json','tools/check_expansion_campaigns.py','tools/check_object_runtime_followthrough.py','tools/check_object_runtime_route.py']},
        scope='Current generic seven-profile/99-anchor producer and exact LLVM/readiness/target-audit bindings; historical417-error selected report preserved separately.')
    dump(ROOT/a.output,result);print(json.dumps(dict(passed=result['passed'],errors=errors,negative_controls=len(controls))));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
