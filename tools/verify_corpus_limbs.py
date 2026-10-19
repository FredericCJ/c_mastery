"""Independent integer oracle for both actual compiled radix variants."""
import argparse
import importlib.util
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('limb_oracle',ROOT/'assessor/corpus_limbs/oracle.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
    if a.driver:
        rows=[oracle.run(str(Path(a.driver).with_name('limbs'+str(bits))),bits) for bits in [16,8]]
        print(json.dumps(dict(passed=True,radices=rows,total_operations=sum(r['operations'] for r in rows))))
    else:
        d=json.loads((ROOT/'fixtures/corpus_limbs/cases.json').read_text())
        assert d['multiply']['a']*d['multiply']['b']==d['multiply']['expected']
        assert divmod(d['divide']['a'],d['divide']['b'])==(d['divide']['quotient'],d['divide']['remainder'])
        print(json.dumps(dict(passed=True,independent_fixture_checks=2)))
if __name__=='__main__':main()
