"""Execute original finite analysis witnesses; never a device observation."""
import argparse,copy,hashlib,json,pathlib
from fractions import Fraction
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_link_reconciliation'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def witness():
 line=lambda offset:offset//32
 packed=[line(0),line(16)];isolated=[line(0),line(32)];assert packed==[0,0] and isolated==[0,1]
 cpu=bytearray(32);device=bytearray(cpu);device[0]=9;cpu[16]=7;device[:]=cpu;assert device[0]==0
 events=['cpu-payload','cpu-length','release','doorbell','device-use','device-return','acquire','cpu-reuse'];edges=[(a,b) for a,b in zip(events,events[1:])]
 legal=lambda order:all(order.index(a)<order.index(b) for a,b in edges)
 early_doorbell=['cpu-payload','doorbell','cpu-length','release',*events[4:]]
 early_reuse=[*events[:5],'cpu-reuse','device-return','acquire'];assert legal(events) and not legal(early_doorbell) and not legal(early_reuse)
 duration=Fraction(960*729*1000000,42000000);rounded=(duration.numerator+duration.denominator-1)//duration.denominator;assert duration==Fraction(116640,7) and rounded==16663 and duration>10000
 first=200+4000;end=first+20000+200;assert first==4200 and end==24400 and end<50000
 return dict(packed_lines=packed,isolated_lines=isolated,lost_device_byte=device[0],legal_order=events,rejected_orders=[early_doorbell,early_reuse],duration_us=[duration.numerator,duration.denominator],rounded_us=rounded,latest_guard_end=end,next_turn=50000)
def expected():
 files=['curriculum/v5/expansion/link_transfer_analysis.md','tools/check_link_analysis.py','labs/ethernet_followthrough/ring/variants.h','assessor/ethernet_followthrough/ring/variants.c','labs/edu7_followthrough/physical/CONTRACTS.md','labs/edu7_followthrough/physical/tests.c']
 return dict(schema_version=1,status='pass',commands=[dict(id='finite-analysis-witnesses',command=['python3','tools/check_link_analysis.py'],passed=True)],source_sha256={n:sha(ROOT/n) for n in files},observations=witness(),limits=['Synthetic analysis models and arithmetic only; not C/device/physical execution.','No independent learner assessment or absolute mastery claim.'])
def main():
 p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);file=HERE/'analysis_validation.json';e=expected()
 if a.write:file.write_text(json.dumps(e,indent=2)+'\n',encoding='utf8')
 errors=[];d=json.loads(file.read_text(encoding='utf8'))
 if d!=e:errors.append('exact source/witness evidence differs')
 controls=[]
 if a.self_test:
  for key in ['same-line-policy','coherence-implies-order','wrong-baud','lost-physical-limit','stale-source']:
   bad=copy.deepcopy(e)
   if key=='same-line-policy':bad['observations']['isolated_lines']=[0,0]
   elif key=='coherence-implies-order':bad['observations']['rejected_orders']=[]
   elif key=='wrong-baud':bad['observations']['rounded_us']=10000
   elif key=='lost-physical-limit':bad['limits']=[]
   else:bad['source_sha256'][next(iter(bad['source_sha256']))]='0'*64
   controls.append(dict(control=key,rejected=bad!=e))
 print(json.dumps(dict(passed=not errors,negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
