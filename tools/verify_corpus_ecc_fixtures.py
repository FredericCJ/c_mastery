"""Independent parity-equation codebook, fixture and optional C-driver comparison."""
import argparse
import importlib.util
import json
import pathlib
import subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('ecc_oracle',ROOT/'assessor/corpus_ecc/oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
    fixture=json.loads((ROOT/'fixtures/corpus_ecc/cases.json').read_text(encoding='utf8'))
    for row in fixture['codebooks']:
        n=row['n'];by_data={oracle.data(n,w):w for w in oracle.codebook(n)}
        assert [by_data[x] for x in row['data']]==row['words']
    if a.driver:result=oracle.verify(subprocess.check_output([a.driver,'oracle'],text=True))
    else:result={'passed':True,'fixture':'independent parity-equation codebook','physical_channel':'not-observed'}
    print(json.dumps(result))
if __name__=='__main__':main()
