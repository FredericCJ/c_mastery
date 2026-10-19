"""Verify independent streaming event fixtures for both actual text capacities."""
import argparse
import importlib.util
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('event_oracle',ROOT/'assessor/corpus_events/oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path);a=p.parse_args()
    fixture=json.loads((ROOT/'fixtures/corpus_events/cases.json').read_text())
    assert oracle.expected(fixture['document'])==[(k,d,bytes.fromhex(b)) for k,d,b in fixture['coalesced_events']]
    rows=[]
    if a.driver:
        for chunk in [16,1]:rows.append(dict(chunk=chunk,cases=oracle.run(a.driver.parent/('events'+str(chunk)))))
    print(json.dumps(dict(passed=True,oracles=rows,total_cases=sum(x['cases'] for x in rows))))
if __name__=='__main__':main()
