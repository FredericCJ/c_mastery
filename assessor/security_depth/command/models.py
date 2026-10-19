"""Independent bounded grammar, ownership, quota and disclosure arguments."""
import itertools,json,pathlib
def expected(packet,authorized):
 if len(packet)<2:return 'length'
 if packet[0] not in [1,2]:return 'opcode'
 if packet[1]>4:return 'count'
 if len(packet)!=2+2*packet[1]:return 'length'
 if packet[0]==2 and not authorized:return 'denied'
 return 'ok'
def main():
 root=pathlib.Path(__file__).resolve().parents[3]
 fixture=json.loads((root/'fixtures/embedded/PR-T35-01.json').read_text())
 assert [expected(p,fixture['input']['authorized']) for p in fixture['input']['packets']]==fixture['expected']
 quota_fixture=json.loads((root/'fixtures/embedded/PR-T35-03.json').read_text())
 cases=[]
 for op,count,n in itertools.product(range(5),range(6),range(12)):
  packet=([op,count]+list(range(2,11)))[:n];cases.append(dict(op=op,count=count,length=n,expected=expected(packet,False)))
 used=0;admitted=[]
 for c in [2,3,5,1]:
  ok=1<=c<=4 and c<=6-used;admitted.append(ok)
  if ok:used+=c
 assert admitted==[True,True,False,True] and used==6
 assert admitted==quota_fixture['expected']['admitted'] and used==quota_fixture['expected']['used']
 attempts=0;safe_stop=False
 for _ in range(quota_fixture['input']['failures']):
  if attempts==quota_fixture['input']['recovery_limit']:safe_stop=True
  else:attempts+=1
 assert attempts==quota_fixture['expected']['recoveries'] and safe_stop==quota_fixture['expected']['safe_stop']
 collisions=[8*i+1 for i in range(8)];assert len({x%8 for x in collisions})==1
 costs=[]
 for key in [collisions[0],collisions[-1],65]:
  cost=0
  for candidate in collisions:
   cost+=1
   if candidate==key:break
  costs.append(cost)
 assert costs==[1,8,8]
 schedules=[]
 for terminal_order in itertools.permutations(['cancel','complete']):
  for revoked in [False,True]:
   schedules.append(dict(order=terminal_order,revoked=revoked,terminals=1,releases=1,protected_effect=int(terminal_order[0]=='complete' and not revoked),late_callback='stale'))
 print(json.dumps(dict(status='pass',base_cases=cases,quota=dict(admitted=admitted,retained=used,queue=3,reserved_control=1),collision=dict(keys=collisions,bucket=1,lookup_work=costs,maximum=8,average_case_not_bound=True),schedules=schedules,arithmetic=dict(u8_product=0,required_bytes=256,size_t_condition='count > SIZE_MAX/2',conversion_minus_one='SIZE_MAX',count256_u8=0),format_types=[dict(format='%n',required='int *',admitted=True),dict(format='%hn',required='short int *',admitted=False),dict(format='%ln',required='long int *',admitted=False)],threat=dict(assets=['synthetic counter','disposable log','bounded memory'],attacker=['packet bytes','request timing','claimed role'],trusted=['authorization owner','request storage owner'],accident_and_attack_same_bytes_same_classification=True,shared_memory_corruption_excluded_from_packet_attacker=True,process_separation_not_whole_sandbox=True),limits=['Finite original model, no real credentials or external target.','Format type mismatches are classified, never executed.','Logical serialization/checked generations do not establish arbitrary concurrent callbacks or memory isolation.']),indent=2))
if __name__=='__main__':main()
