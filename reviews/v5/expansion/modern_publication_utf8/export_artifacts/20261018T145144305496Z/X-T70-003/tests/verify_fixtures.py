"""Independent bounded mathematical expectations; does not import reference C."""
import json
from pathlib import Path
ROOT = Path(__file__).resolve().parents[3]
def check(project, result):
    data=json.loads((ROOT/'fixtures/dod'/f'PR-T70-{project:02}.json').read_text(encoding='utf8'))
    assert result == data['expected_model'], (project,result,data['expected_model'])
check(1, [x+v*2 for x,v in zip([1,2,3,4],[2,-1,1,0])])
# Abstract allocation ownership, independent of byte-buffer layout.
states=[{'length':0,'owned':0}]
for length,owned in [(16,0),(16,0),(17,1),(8,0),(0,0)]:
    states.append({'length':length,'owned':owned})
check(2,states)
a,b=0xff00ff00,0x01010101
check(3,sum(min(255,((a>>(8*i))&255)+((b>>(8*i))&255))<<(8*i) for i in range(4)))
live=[11,22,33];live.remove(22)
check(4,{'logical_values':live,'issued_generations':list(range(1,4)),'retired_generation':4})
check(5,{'both_positions':[30+2,10+1],'remaining_position_only':[20]})
versions=[[0]*8];v=versions[-1].copy();v[3]=17;versions.append(v);v=v.copy();v[6]=29;versions.append(v)
check(6,{'versions':versions,'fresh_nodes':[3+1,3+1]})
versions=[{}];v={1:10};versions.append(v.copy());v[17]=20;versions.append(v.copy());v[0]=30;versions.append(v.copy());v[17]=88;versions.append(v.copy())
check(7,[{str(k):val for k,val in sorted(v.items())} for v in versions])
check(8,{'sequence':list((b'ab'+b'cd')*2),'length':8,'preserved_leaf':list(b'ab')})
print('dod independent fixture models:8 checked;0 failures')
