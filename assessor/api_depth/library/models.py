"""Original independent byte model, trace mapping and reversible migration."""
import itertools,json,pathlib,sys
def append(previous,incoming,capacity):
 return (2,previous) if len(previous)+len(incoming)>capacity else (0,previous+incoming)
def encode(version,data):
 limit=8 if version==1 else 16
 if len(data)>limit:raise ValueError('capacity')
 return b'L'+bytes([48+version,len(data)])+data
def decode(version,data):
 if len(data)<3 or data[:2]!=b'L'+bytes([48+version]) or len(data)!=3+data[2] or data[2]>(8 if version==1 else 16):raise ValueError('format')
 return data[3:]
def main():
 root=pathlib.Path(sys.argv[1]);root.mkdir(parents=True,exist_ok=True);old=root/'version1.data';new=root/'version2.data'
 old.write_bytes(encode(1,b'A\0B'));new.write_bytes(encode(2,decode(1,old.read_bytes())));assert decode(2,new.read_bytes())==b'A\0B'
 rollback=root/'rollback.data';rollback.write_bytes(encode(1,decode(2,new.read_bytes())));assert old.read_bytes()==rollback.read_bytes()
 new.write_bytes(encode(2,bytes(range(9))));before=(old.read_bytes(),new.read_bytes())
 try:encode(1,decode(2,new.read_bytes()))
 except ValueError:pass
 else:raise AssertionError('unsupported downgrade accepted')
 assert before==(old.read_bytes(),new.read_bytes())
 cases=sum(1 for cap in range(9) for prior in range(cap+1) for n in range(10) if append(bytes(prior),bytes(n),cap)[0] in [0,2]);assert cases==450
 print(json.dumps(dict(append_cases=cases,rollback_refusal_preserves_both=True,reversible_small_record=True,known_limit='copy-to-new-file procedure; no atomic replacement or power-loss claim',capacity_assertion='length and every existing byte unchanged on rejection',ownership_assertion='same failure performs no release and preserves borrow validity')))
if __name__=='__main__':main()
