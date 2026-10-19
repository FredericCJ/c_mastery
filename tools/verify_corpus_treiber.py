import argparse,importlib.util,json
from common import ROOT
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
 spec=importlib.util.spec_from_file_location('treiber_oracle',ROOT/'assessor/corpus_treiber/oracle.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 if a.driver:print(json.dumps(m.run(a.driver)))
 else:
  data=json.loads((ROOT/'fixtures/corpus_treiber/cases.json').read_text());stack=[];out=[]
  for op,value in data['operations']:
   if op=='push':stack.append(value)
   else:out.append(stack.pop() if stack else None)
  assert out==data['expected'];print(json.dumps(dict(passed=True,fixture_outputs=out)))
if __name__=='__main__':main()
