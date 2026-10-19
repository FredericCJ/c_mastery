"""Add observation expressions to exact frozen producer functions; never rewrite policy."""
import pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
NAMES=['ring_valid','spans','cd_ring_init','cd_ring_read','cd_ring_reserve','cd_ring_commit','cd_ring_push','cd_ring_pop','vector_valid','cd_vector_init','cd_vector_reserve','cd_vector_insert','cd_vector_erase','cd_vector_shrink','cd_vector_destroy']
def function(text,name):
 at=text.index(name+'(');start=text.rfind('\n',0,at)+1;opening=text.index('{',at);depth=1;i=opening+1
 while depth:depth+=(text[i]=='{')-(text[i]=='}');i+=1
 return text[start:i]
def controls(code,counter):
 replacements=[]
 for match in re.finditer(r'\b(if|while|for)\s*\(',code):
  start=match.end();depth=1;i=start
  while depth:depth+=(code[i]=='(')-(code[i]==')');i+=1
  end=i-1;condition=code[start:end]
  if match[1]=='for':
   first=condition.index(';');last=condition.rindex(';');start+=first+1;end=start+last-first-1;condition=code[start:end]
  replacements.append((start,end,'AI_CHECK('+counter+',('+condition+'))'))
 for start,end,value in reversed(replacements):code=code[:start]+value+code[end:]
 return code
def main():
 seq=(ROOT/'assessor/containers_depth/sequence.c').read_text(encoding='utf8');linked=(ROOT/'assessor/containers_depth/transfer.c').read_text(encoding='utf8');parts=['#include "containers.h"\n#include "instrument.h"\n#define CD_MUTANT 0']
 for name in NAMES:
  code=function(seq,name)
  if 'ring' in name or name=='spans':
   code=controls(code,'ai_ring_counts')
   # Each conditional-expression decision is counted too; inner short-circuit operands stay one expression.
   for before,after in [('r->capacity?','AI_CHECK(ai_ring_counts,r->capacity)?'),('r->head>=r->capacity-r->count?','AI_CHECK(ai_ring_counts,r->head>=r->capacity-r->count)?'),('n>first?','AI_CHECK(ai_ring_counts,n>first)?'),('first?','AI_CHECK(ai_ring_counts,first)?')]:code=code.replace(before,after)
   for expression in ['s.first[i]=values[i]','s.second[i]=values[s.first_count+i]','out[i]=r->data[r->head]']:code=code.replace(expression,'(ai_ring_counts.transfers++,'+expression+')')
  parts.append(code)
 for name in ['cd_queue_init','cd_queue_push','cd_queue_pop']:
  code=controls(function(linked,name),'ai_link_counts')
  code=code.replace('q->nodes[at].live))','(ai_link_counts.slot_probes++,q->nodes[at].live)))')
  code=code.replace('q->nodes[at]=(CDQueueNode){value,-1,1}','(ai_link_counts.transfers++,q->nodes[at]=(CDQueueNode){value,-1,1})')
  code=code.replace('q->head=q->nodes[at].next','q->head=(ai_link_counts.links++,q->nodes[at].next)')
  code=code.replace('*out=value','(ai_link_counts.transfers++,*out=value)');parts.append(code)
 output='\n\n'.join(parts)+'\n';assert output.count('slot_probes++')==1 and output.count('transfers++')==5 and output.count('links++')==1
 pathlib.Path(sys.argv[1]).write_text(output,encoding='utf8');print('OBS generated source-preserving producer instrumentation')
if __name__=='__main__':main()
