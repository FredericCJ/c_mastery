"""Independent literal wire oracle and rejection of mixed provenance."""
from pathlib import Path
import argparse,copy,json,sys
ROOT=Path(__file__).resolve().parents[3];sys.path.insert(0,str(ROOT/'labs/runtime_followthrough/assessment'))
from provenance import sha,call,validate,record
p=argparse.ArgumentParser();p.add_argument('--record',type=Path,required=True);a=p.parse_args();a.record=a.record.resolve();d=json.loads(a.record.read_text());assert not validate(d)
# Derived from original format: magic,count,one descriptor,INT41,HOST1,RET.
golden=bytes.fromhex('43564d31010000080000000129000000160111')
assert Path(d['image']).read_bytes()==golden
assert d['result']==dict(status=0,value=42,fuel_left=0,host_left=0,calls=1,adapter=1)
root=a.record.parent;changed=root/'source42.cvm';changed.write_text('(fn main () (host 1 42))\n');alternate=root/'image42.cvm';row=call([d['compiler'],changed,alternate]);assert row['returncode']==0
controls=[]
for kind in ['mixed-image','mixed-source','missing-source','stale-tool','wrong-result','wrong-adapter','false-verdict']:
 x=copy.deepcopy(d)
 if kind=='mixed-image':x['image']=str(alternate);x['inputs'][str(alternate)]=sha(alternate);x['compile']['command'][-1]=str(alternate);x['run']['command'][1]=str(alternate)
 elif kind=='mixed-source':x['source']=str(changed);x['inputs'][str(changed)]=sha(changed);x['compile']['command'][1]=str(changed)
 elif kind=='missing-source':x['inputs'].pop(x['source'])
 elif kind=='stale-tool':x['inputs'][x['compiler']]='0'*64
 elif kind=='wrong-result':x['result']['value']=99;x['run']['stdout']=json.dumps(x['result'])+'\n'
 elif kind=='wrong-adapter':x['adapter']=2;x['run']['command'][-1]='2'
 else:x['compile']['stdout']='VP_COMPILE status=0 verify=7 bytes=19\n'
 errors=validate(x);assert errors,kind
 if kind in ['mixed-image','mixed-source']:assert errors==['source/image derivation mismatch'],errors
 controls.append(dict(control=kind,rejected=True,reason=errors))
other=record(Path(d['source']),Path(d['compiler']),Path(d['vm']),Path(d['build_record']),root/'changed-adapter',2);assert Path(other['image']).read_bytes()==golden and other['result']['value']==43
out=dict(passed=True,golden_hex=golden.hex(),negative_controls=controls,changed_adapter_result=other['result'],limits='Author reproduction only; independent reader/delayed assessment not performed');(root/'chain-controls.json').write_text(json.dumps(out,indent=2)+'\n');print('VP_CHAIN_TEST_PASS seven_rejections_exact19bytes_adapter42_to43')
