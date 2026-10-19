"""Independent integer/CRC and small forwarding-trace packet oracle."""
import json,pathlib,sys,zlib
da=bytes([2,0,0,0,0,2]);sa=bytes([2,0,0,0,0,1]);body=da+sa+b'\0\3'+bytes([1,2,3])+bytes(43)
expected={'stream':[sum(n+3 for n in range(257)),3,256,2],'vlan':[1501,64,18+1500+4,64-18-4,64-14-4],'frame':[3,43,60,64,zlib.crc32(body)],'bridge':[6,1,2,0,3,4],'loop':[8*2,1,4,0],'layers':[20,40,54,32+20+20,14+72+4,14+72+4+8+12,(14+72+4+8+12)*8*1000//100]}
def main():
 rows={}
 for line in pathlib.Path(sys.argv[1]).read_text().splitlines():
  if line.startswith('OBS wire_'):
   words=line.split();key=words[1][5:];assert key not in rows;rows[key]=list(map(int,words[2:]))
 wanted=set(expected) if sys.argv[2]=='all' else {sys.argv[2]};assert set(rows)==wanted
 for k,v in rows.items():assert v==expected[k],(k,v,expected[k])
 print(json.dumps(dict(status='pass',packets=len(rows),keys=sorted(rows),method='Independent Python integer arithmetic/zlib; finite trace expectations only')))
if __name__=='__main__':main()
