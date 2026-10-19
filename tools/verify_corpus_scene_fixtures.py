"""Independent original geometry fixture and actual driver oracle."""
import argparse
import json
import subprocess
import sys
import importlib.util
import math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
 d=json.loads((ROOT/'fixtures/corpus_scene/cases.json').read_text(encoding='utf8'))
 parent=d['parent'];child=d['child']+[1]
 assert [sum(a*b for a,b in zip(row,child)) for row in parent]==d['rotated_child_world']
 spec=importlib.util.spec_from_file_location('independent',ROOT/'assessor/corpus_scene/oracle.py');model=importlib.util.module_from_spec(spec);spec.loader.exec_module(model)
 path=[tuple(p) for p in d['turning_path']];polys=[[tuple(p) for p in row] for row in d['corridor']]
 assert all(model.visible(a,b,polys) for a,b in zip(path,path[1:]))
 assert math.isclose(sum(math.dist(a,b) for a,b in zip(path,path[1:])),model.shortest(path[0],path[-1],polys))
 if a.driver:
  r=subprocess.run([sys.executable,ROOT/'assessor/corpus_scene/oracle.py',a.driver],text=True,capture_output=True,check=True);print(r.stdout,end='')
 else:print(json.dumps(dict(passed=True,fixture='hand-derived affine and corridor anchors')))
if __name__=='__main__':main()
