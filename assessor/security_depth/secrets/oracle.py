"""Independent byte equality/work model and trusted-bridge tag oracle."""
import hashlib,hmac,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[3]
def main():
 f=json.loads((ROOT/'fixtures/embedded/PR-T35-05.json').read_text());t=f['input']['token'];cs=f['input']['candidates'];early=[next((i+1 for i,(a,b) in enumerate(zip(t,c)) if a!=b),4) for c in cs];expected=dict(equal=[t==c for c in cs],early_work=early,fixed_work=[4]*3);assert expected==f['expected']
 if len(sys.argv)>1:
  lines=pathlib.Path(sys.argv[1]).read_text().splitlines();tags=[x.split() for x in lines if x.startswith('TAG ')];assert len(tags)==65
  for _,length,tag in tags:
   n=int(length);assert tag==hmac.new(bytes(t),bytes(range(n)),hashlib.sha256).hexdigest()
 print(json.dumps(dict(status='pass',fixture=expected,provider_vectors=65 if len(sys.argv)>1 else 0,limits='Logical count and selected vectors only; no timing or deployment certification.')))
if __name__=='__main__':main()
