"""Run actual domain, frontend, frozen parser and persistent output mechanisms."""
import argparse,hashlib,json,os,subprocess
from pathlib import Path
import run_expansion_family as base
from common import ROOT,sha,dump
DEFAULT='reviews/v5/expansion/corpus_model/current_validation.json'
def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_corpus_model.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    paths=['tools/run_corpus_model.py','tools/check_corpus_model.py','tools/author_corpus_model.py',
           'curriculum/v5/exercises/X-T15-021.md','assessor/parsing_depth/pratt.c','labs/parsing_depth/parsing.h',
           'labs/parsing_depth/CONTRACTS.md','research/v5/expansion/corpus_model/resources.json',
           'research/v5/expansion/corpus_model/taxonomy_extensions.json']
    paths += [p.relative_to(ROOT).as_posix() for p in (ROOT/'build/corpus_model_sources').glob('*') if p.is_file()]
    before={p:sha(ROOT/p) for p in paths}
    assert sha(ROOT/'labs/corpus_model/provider/parsing.h')==sha(ROOT/'labs/parsing_depth/parsing.h')
    code=base.run('corpus_model',report);data=json.loads((ROOT/report).read_text());data['inspected_paths'].update(before)
    observations=[]
    for row in data['executions']:
        if row['log'].endswith('-independent-oracle.log') and row['exit_code']==0:
            value=json.loads((ROOT/row['log']).read_text());value['log']=row['log'];observations.append(value)
    data['independent_oracles']=observations
    data['scope_source_stable']=data['scope_source_stable'] and all(sha(ROOT/p)==h for p,h in before.items())
    data['passed']=not code and data['scope_source_stable'] and len(observations)==6
    data['provider_header_identity']=sha(ROOT/'labs/corpus_model/provider/parsing.h')==sha(ROOT/'labs/parsing_depth/parsing.h')
    data['scope_input_sha256']=hashlib.sha256(json.dumps(data['inspected_paths'],sort_keys=True).encode()).hexdigest()
    data['status']='passed' if data['passed'] else 'failed'
    data['scope']='Six hosted C17 profiles: persistent graph, two configuration frontends/direct API, actual frozen Pratt queries, independent graph/value histories, actual stdio REPL and retained prefix-output resumption. No physical or learner claim.'
    dump(ROOT/report,data);print(json.dumps(dict(passed=data['passed'],commands=len(data['executions']),oracles=len(observations))));return int(not data['passed'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=DEFAULT);a=p.parse_args();raise SystemExit(run(a.report))
