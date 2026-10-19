"""Independent scalar fixture and actual driver validation."""
import argparse
import json
import pathlib
import subprocess
import sys
ROOT=pathlib.Path(__file__).resolve().parents[1]
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
    d=json.loads((ROOT/'fixtures/corpus_spatial/cases.json').read_text(encoding='utf8'))
    for r in d['floor_cells']:assert r['x']//r['width']==r['cell']
    r=d['weighted_nearest'];values=[sum(w*(x-y)**2 for w,x,y in zip(r['weights'],q,r['query'])) for q in r['points']]
    assert min((v,i) for i,v in enumerate(values))==(r['squared_distance'],r['id'])
    r=d['closed_contact'];assert all(r['a'][0][k]<=r['b'][1][k] and r['b'][0][k]<=r['a'][1][k] for k in range(2))==r['overlap']
    if a.driver:
        result=subprocess.run([sys.executable,ROOT/'assessor/corpus_spatial/oracle.py',a.driver],capture_output=True,text=True,check=True)
        print(result.stdout,end='')
    else:print(json.dumps(dict(passed=True,fixture='independent integer geometry')))
if __name__=='__main__':main()
