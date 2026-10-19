"""Execute actual C17 t-digest mechanisms and preserve empirical limitations."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
import run_expansion_family as base
from common import ROOT,sha,dump
DEFAULT='reviews/v5/expansion/corpus_tdigest/current_validation.json'
def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_corpus_tdigest.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    extra=[Path(__file__),ROOT/'tools/check_corpus_tdigest.py',ROOT/'tools/author_corpus_tdigest.py',ROOT/'curriculum/v5/exercises/X-T15-009.md',*[ROOT/'research/v5/expansion/corpus_tdigest'/n for n in ['resources.json','taxonomy_extensions.json']]]
    for row in json.loads((ROOT/'research/v5/expansion/corpus_tdigest/resources.json').read_text())['resources']:extra.append(ROOT/row['inspection']['download_path'])
    before={p.relative_to(ROOT).as_posix():sha(p) for p in extra};code=base.run('corpus_tdigest',report);path=ROOT/report;data=json.loads(path.read_text());data['inspected_paths'].update(before)
    observations=[]
    for row in data['executions']:
        if row['log'].endswith('-independent-oracle.log') and row['exit_code']==0:
            value=json.loads((ROOT/row['log']).read_text());value['log']=row['log'];observations.append(value)
    data['independent_oracles']=observations;data['scope_source_stable']=data['scope_source_stable'] and all(sha(ROOT/p)==h for p,h in before.items());data['passed']=not code and data['scope_source_stable'] and len(observations)==6
    data['scope_input_sha256']=hashlib.sha256(json.dumps(data['inspected_paths'],sort_keys=True).encode()).hexdigest();data['status']='passed' if data['passed'] else 'failed'
    data['scope']='Six hosted C17 profiles, actual k2 merging and two-level recompression, independent centroid/mixture/raw-rank checks and explicit merge-order/large-error/rounding divergences. No universal accuracy or physical claim.'
    dump(path,data);print(json.dumps(dict(passed=data['passed'],commands=len(data['executions']),oracles=len(observations))));return int(not data['passed'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();raise SystemExit(run(a.report))
