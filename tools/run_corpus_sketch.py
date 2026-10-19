"""Run six C17 sketch profiles with independent exact and numerical oracles."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
import run_expansion_family as base
from common import ROOT,sha,dump
DEFAULT='reviews/v5/expansion/corpus_sketch/current_validation.json'
def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_corpus_sketch.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    extra=[Path(__file__),ROOT/'tools/check_corpus_sketch.py',ROOT/'tools/author_corpus_sketch.py',*[ROOT/'research/v5/expansion/corpus_sketch'/name for name in ['resources.json','taxonomy_extensions.json']],*[ROOT/f'curriculum/v5/exercises/X-T15-{n:03}.md' for n in (5,6,7)]]
    for row in json.loads((ROOT/'research/v5/expansion/corpus_sketch/resources.json').read_text())['resources']:
        extra.append(ROOT/row['inspection']['download_path'])
    before={p.relative_to(ROOT).as_posix():sha(p) for p in extra}
    code=base.run('corpus_sketch',report);path=ROOT/report;data=json.loads(path.read_text());data['inspected_paths'].update(before)
    data['independent_oracles']=[]
    for execution in data['executions']:
        if execution['log'].endswith('-independent-oracle.log') and execution['exit_code']==0:
            row=json.loads((ROOT/execution['log']).read_text().strip());row['log']=execution['log'];data['independent_oracles'].append(row)
    data['scope_source_stable']=data['scope_source_stable'] and all(sha(ROOT/p)==h for p,h in before.items())
    data['passed']=not code and data['scope_source_stable'] and len(data['independent_oracles'])==6
    data['scope_input_sha256']=hashlib.sha256(json.dumps(data['inspected_paths'],sort_keys=True).encode()).hexdigest()
    data['status']='passed' if data['passed'] else 'failed'
    data['scope']='Three distinct C17 sketches, exact merge/fold/banding transfers, six compiled semantic controls and six independent finite oracles; no production hash-quality proof, board observation or learner assessment.'
    dump(path,data);print(json.dumps(dict(passed=data['passed'],commands=len(data['executions']),oracles=len(data['independent_oracles']))));return int(not data['passed'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();raise SystemExit(run(a.report))
