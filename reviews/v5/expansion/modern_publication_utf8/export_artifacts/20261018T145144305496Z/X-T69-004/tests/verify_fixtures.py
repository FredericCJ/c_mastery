"""Independent expected values and transition traces; no assessor C imports."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
def check(n,value):
    f=json.loads((ROOT/'fixtures/dispatch'/f'PR-T69-{n:02}.json').read_text(encoding='utf8'))
    assert value==f['expected_model'],(n,value,f['expected_model'])
check(1,{'value':7,'failed_output':99})
check(2,{'value':17,'failed_output':99})
acc=10
for v in [3,2]:acc-=v
check(3,{'subtract':acc,'sum':sum([10,3,2]),'empty':7})
capture=3;mutable=[]
for arg in [2,2]:capture+=arg;mutable.append(capture)
check(4,{'shared':[3+2,3+2],'mutable':mutable,'once':1+2,'attempt_consumes_on_range_failure':True})
owners={'source'};owners.remove('source');owners.add('destination');destroyed=len(owners);owners.clear()
check(5,{'result':7+3,'destroyed':destroyed,'remaining_owners':len(owners)})
check(6,{'before':10-2,'reversed':'unsupported','after':10+2,'final_epoch':3})
items=[1,2,3];before=sum(items);items.remove(2);middle=sum(items);items.clear()
check(7,[before,middle,sum(items)])
check(8,{'capacity':2,'success':7+7,'reads_before_second_failure':2,'failure_output':99})
rights=set(['read','add']);attenuated=rights&set(['read']);counter=0
if 'add' in rights:counter+=5
check(9,{'attenuated':sorted(attenuated),'counter':counter,'amplification_allowed':set(['read','add'])<=attenuated})
check(10,{'admitted_abi':1,'rejected_abi':9,'result':17+3,'borrowed_close':'busy'})
state=7;generation=1;state*=2;generation+=1
check(11,{'committed_state':state,'generation':generation,'failed_migration_preserves':600})
print('dispatch independent fixture models:11 checked;0 failures')
