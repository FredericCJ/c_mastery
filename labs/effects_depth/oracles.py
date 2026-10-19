"""Independent finite effect/stack models; do not manufacture runtime evidence."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[2]
def closure(roots,edges):
 seen=set();pending=list(roots)
 while pending:
  node=pending.pop()
  if node not in seen:seen.add(node);pending.extend(edges.get(node,()))
 return seen
def main():
 fixture=json.loads((ROOT/'fixtures/systems/PR-T13-01.json').read_text());given=fixture['input'];base=closure(given['roots'],given['edges']);assert sorted(base)==fixture['expected']['reachable']
 changed={**given['edges'],'helper':['pool'],'component':['helper','callback'],'callback':['registered_a','registered_b']};finite=closure(given['roots'],changed);assert 'pool' in finite and 'malloc' not in finite
 changed['callback'].append('DYNAMIC_UNKNOWN');dynamic=closure(given['roots'],changed);assert 'DYNAMIC_UNKNOWN' in dynamic
 budget=json.loads((ROOT/'fixtures/systems/PR-T13-02.json').read_text());total=sum(budget['input']['parts']);assert total==48 and budget['input']['limit']-total==16
 nested=total+20+12+8;assert nested==88;separate={'task':total,'interrupt':20+12+8};assert separate=={'task':48,'interrupt':40}
 rare=total+64;assert rare==112 and rare>budget['input']['limit']
 def temp(n,width,pointer):return 2*n*pointer+width if width>32 else n*width
 values=[temp(n,w,8) for n,w in [(64,4),(300,4),(10,64)]];assert values==[256,1200,224] and temp(10,64,4)==144
 print(json.dumps({'baseline_reachable':sorted(base),'pool_without_malloc':sorted(finite),'dynamic_unresolved':sorted(dynamic),'stack_fixture':total,'nested_same_stack':nested,'separate_stack_budgets':separate,'rare_callback_total':rare,'source_array_subtotal':32+64,'unknown_callback_bound':None,'glibc_temporary8':values,'glibc_indirect4':144,'limits':'Finite graph/source arithmetic only, not observed runtime/ABI stack quantities'}))
if __name__=='__main__':main()
