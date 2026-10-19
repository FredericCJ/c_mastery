"""Independent widened mathematics, finite counts and selected branch bytes."""
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def oracle(path,case):
 if case in ['x86','thumb']:
  raw=path.read_bytes();codes=[0x73,0x76,0x7d,0x7e,0x75] if case=='x86' else [0xd2,0xd9,0xda,0xdd,0xd1];offsets=[]
  for code in codes:
   pattern=bytes([code,3]) if case=='x86' else bytes([1,code]);assert raw.count(pattern)==1,(case,code,raw.hex());at=raw.index(pattern);target=at+(2+3 if case=='x86' else 4+2);assert target<len(raw);offsets.append(dict(at=at,target=target,condition=code))
  return dict(status='pass',selected_branches=offsets,encoding_scope='Only five inspected unsigned/signed/equality short branches, not a general decoder')
 rows={}
 for text in path.read_text().splitlines():
  line=text.split()
  if line and line[0]!='OBS':rows.setdefault(line[0],[]).append(list(map(int,line[1:])))
 signed=lambda x,n:x-(1<<n) if x>=(1<<(n-1)) else x
 if case=='truth':
  expected=[[a,b,c,(a+b+c)%2,(a+b+c)//2] for a in range(2) for b in range(2) for c in range(2)];assert rows['SLICE']==expected;assert expected==json.loads((ROOT/'fixtures/algorithms/PR-T37-01.json').read_text())['expected']['full_adder_rows'];return dict(status='pass',packets=8)
 if case=='exhaustive':
  actual=rows['ALU'];assert len(actual)==5120;seen=set();totals={}
  for width,op,extra,a,b,value,z,n,c,cf,v in actual:
   key=(width,op,extra,a,b);assert key not in seen;seen.add(key);size=1<<width;mat=a-b-extra if op else a+b+extra;sm=signed(a,width)-signed(b,width)-extra if op else signed(a,width)+signed(b,width)+extra;ev=mat%size;ec=int(mat>=0) if op else int(mat>=size);ecf=1-ec if op else ec;overflow=int(not -size//2<=sm<size//2);assert [value,z,n,c,cf,v]==[ev,int(ev==0),int(ev>=size//2),ec,ecf,overflow];t=totals.setdefault((width,op,extra),[0,0]);t[0]+=ecf;t[1]+=overflow
  assert rows['COUNTS']==[[*key,*totals[key]] for key in sorted(totals)];return dict(status='pass',packets=5120,counts=rows['COUNTS'])
 if case=='branches':
  actual=rows['BRANCH'];assert len(actual)==1280;seen=set()
  for width,a,b,relation,native in actual:
   assert (width,a,b) not in seen;seen.add((width,a,b));sa,sb=signed(a,width),signed(b,width);expected=int(a<b)|(int(a>b)<<1)|(int(sa<sb)<<2)|(int(sa>sb)<<3)|(int(a==b)<<4);size=1<<width;v=int(not -size//2<=sa-sb<size//2);n=int((a-b)%size>=size//2);assert relation==expected and native==expected|(int(a<b)<<5)|(v<<6)|(n<<7)
  return dict(status='pass',packets=1280,architecture='actual native x86-64 flags; target Arm checks independently inside C harness')
 raise AssertionError(case)
if __name__=='__main__':print(json.dumps(oracle(pathlib.Path(sys.argv[1]),sys.argv[2])))
