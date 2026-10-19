"""Derive the explicit CST5 teaching fork; fail if the reviewed parent changes."""
from pathlib import Path
import hashlib
ROOT=Path(__file__).resolve().parents[1]
def main(check=False):
 source=ROOT/'assessor/storage_followthrough/tree/tree.c'
 assert hashlib.sha256(source.read_bytes()).hexdigest()=='9e91c830850cc96d5d2198ebce8260d302ccb4dbe0de768c53a32b0e3d614fc6'
 assert hashlib.sha256((ROOT/'labs/storage_followthrough/tree/include/store.h').read_bytes()).hexdigest()=='3aad559b49bf270c7cc3b16e0d0ab61224724efecb57c6e9cdca2e2c61149585'
 s=source.read_text(encoding='utf8')
 # Canonical output is compiled and reviewed separately; these exact edits
 # preserve an auditable derivation instead of modifying the frozen provider.
 def change(old,new,count=1):
  nonlocal s
  assert s.count(old)==count,(old,s.count(old),count)
  s=s.replace(old,new)
 change('#include "tree.h"','#include "refs.h"')
 change('#define MAGIC (TREE_CAPACITY==7u ? UINT32_C(0x43535432) : UINT32_C(0x43535434))','#define MAGIC UINT32_C(0x43535435)\n#ifndef REF_MUTANT\n#define REF_MUTANT 0\n#endif')
 change('/* Additive capacity/path-repair teaching fork; original provider unchanged. */','/* Separate CST5 incarnation-tagged page-reference teaching fork. */')
 change('  seal(b);\n}\nstatic void encode(store *s,','  wr64(b+768,n->incarnation);\n  wr64(b+776,n->next_incarnation);\n  if(!n->leaf)for(i=0;i<=n->n&&i<9;i++)wr64(b+784+8*i,n->child_incarnation[i]);\n  seal(b);\n}\nstatic void encode(store *s,')
 change('    wr32(b + 24, s->used);','    wr32(b + 24, s->used);\n    wr64(b+32,s->root_incarnation);\n    for(unsigned j=0;j<32;j++)wr64(b+64+8*j,REF_MUTANT==3&&!(s->used&(UINT32_C(1)<<j))?0:s->incarnations[j]);')
 change('  n->next = rd32(b + 24);','  n->next = rd32(b + 24);\n  n->incarnation=rd64(b+768);n->next_incarnation=rd64(b+776);\n  for(unsigned j=0;j<9;j++)n->child_incarnation[j]=rd64(b+784+8*j);\n  if(!n->incarnation||(n->next?!n->next_incarnation:n->next_incarnation!=0))return STORE_CORRUPT;')
 change('      if (!n->child[i] || n->child[i] >= STORE_DATA_PAGES)','      if (!n->child[i] || n->child[i] >= STORE_DATA_PAGES || !n->child_incarnation[i])')
 change('  for (i = 1; i < n->n; i++)\n','  if(!n->leaf&&(n->next||n->next_incarnation))return STORE_CORRUPT;\n  for(i=n->leaf?0u:n->n+1u;i<9;i++)if(n->child_incarnation[i])return STORE_CORRUPT;\n  for (i = 1; i < n->n; i++)\n')
 old='''static int node(store *s, uint32_t id, store_node *n) {
  uint8_t b[STORE_PAGE_SIZE];
  int r;
  if (!id || id >= STORE_DATA_PAGES || !(s->used & (UINT32_C(1) << id)))
    return STORE_CORRUPT;
  if (s->active) {
    *n = s->scratch[id];
    return STORE_OK;
  }
  r = cached(s, id, b);
  return r ? r : decode(s, id, b, n);
}'''
 new='''static int reference_ok(store *s,uint32_t id,uint64_t incarnation){
  return id>0&&id<32&&(s->used&(UINT32_C(1)<<id))&&incarnation&&
    (REF_MUTANT==1||s->incarnations[id]==incarnation);
}
static int node(store *s,uint32_t id,store_node *out){
  uint8_t bytes[4096];store_node n;int r=STORE_OK;
  if(!id||id>=32||!(s->used&(UINT32_C(1)<<id)))return STORE_CORRUPT;
  if(s->active)n=s->scratch[id];else{r=cached(s,id,bytes);if(!r)r=decode(s,id,bytes,&n);}
  if(r)return r;
  if(!reference_ok(s,id,n.incarnation)||(id==s->root&&!reference_ok(s,id,s->root_incarnation)))return STORE_CORRUPT;
  if(n.leaf){if(n.next&&!reference_ok(s,n.next,n.next_incarnation))return STORE_CORRUPT;}
  else for(unsigned i=0;i<=n.n;i++)if(!reference_ok(s,n.child[i],n.child_incarnation[i]))return STORE_CORRUPT;
  *out=n;return STORE_OK;
}'''
 change(old,new)
 change('  s->used = rd32(b + 24);','  s->used = rd32(b + 24);\n  s->root_incarnation=rd64(b+32);\n  for(unsigned j=0;j<32;j++)s->incarnations[j]=rd64(b+64+8*j);\n  if(s->incarnations[0])return STORE_CORRUPT;')
 change('  s.scratch[1].leaf = 1;','  s.scratch[1].leaf = 1;\n  s.incarnations[1]=s.root_incarnation=s.scratch[1].incarnation=1;')
 change('  s->saved_used = s->used;','  s->saved_used = s->used;\n  s->saved_root_incarnation=s->root_incarnation;\n  memcpy(s->saved_incarnations,s->incarnations,sizeof s->incarnations);')
 change('    s->used = s->saved_used;','    s->used = s->saved_used;\n    s->root_incarnation=s->saved_root_incarnation;\n    memcpy(s->incarnations,s->saved_incarnations,sizeof s->incarnations);')
 change('    if (!(s->used & (UINT32_C(1) << i))) {','    if (!(s->used & (UINT32_C(1) << i)) && (REF_MUTANT==2||s->incarnations[i]<UINT64_MAX)) {')
 change('      s->scratch[i].leaf = (uint8_t)leaf;','      s->scratch[i].leaf = (uint8_t)leaf;\n      if(s->incarnations[i]==UINT64_MAX)s->incarnations[i]=1;else s->incarnations[i]++;\n      s->scratch[i].incarnation=s->incarnations[i];')
 change('      n->child[j + 1] = n->child[j];','      n->child[j + 1] = n->child[j];\n      n->child_incarnation[j+1]=n->child_incarnation[j];')
 change('    n->child[i + 1] = sub.right;','    n->child[i + 1] = sub.right;\n    n->child_incarnation[i+1]=s->incarnations[sub.right];')
 change('    q->next = n->next;\n    n->next = right;','    q->next = n->next;\n    q->next_incarnation=n->next_incarnation;\n    n->next = right;')
 change('    n->next = right;','    n->next = right;\n    n->next_incarnation=s->incarnations[right];')
 change('    for (j = 0; j < HALF; j++)\n      q->child[j] = n->child[j + HALF+1];','    for (j = 0; j < HALF; j++){\n      q->child[j] = n->child[j + HALF+1];\n      q->child_incarnation[j]=n->child_incarnation[j+HALF+1];\n    }')
 change('      s->scratch[root].child[1] = sp.right;','      s->scratch[root].child[1] = sp.right;\n      s->scratch[root].child_incarnation[0]=s->incarnations[s->root];\n      s->scratch[root].child_incarnation[1]=s->incarnations[sp.right];')
 change('      s->root = root;','      s->root = root;\n      s->root_incarnation=s->incarnations[root];')
 change('    p->child[j + 1] = p->child[j + 2];','    p->child[j + 1] = p->child[j + 2];\n    p->child_incarnation[j+1]=p->child_incarnation[j+2];')
 change('        for (j = n->n + 1; j > 0; j--)\n          n->child[j] = n->child[j - 1];','        for (j = n->n + 1; j > 0; j--){\n          n->child[j] = n->child[j - 1];\n          n->child_incarnation[j]=n->child_incarnation[j-1];\n        }')
 change('        n->child[0] = q->child[q->n];','        n->child[0] = q->child[q->n];\n        n->child_incarnation[0]=q->child_incarnation[q->n];')
 change('        n->child[n->n + 1] = q->child[0];','        n->child[n->n + 1] = q->child[0];\n        n->child_incarnation[n->n+1]=q->child_incarnation[0];')
 change('        for (j = 0; j < q->n; j++)\n          q->child[j] = q->child[j + 1];','        for (j = 0; j < q->n; j++){\n          q->child[j] = q->child[j + 1];\n          q->child_incarnation[j]=q->child_incarnation[j+1];\n        }')
 change('        q->child[q->n] = n->child[0];','        q->child[q->n] = n->child[0];\n        q->child_incarnation[q->n]=n->child_incarnation[0];')
 change('        if (!n->leaf)\n          q->child[q->n + 1] = n->child[j + 1];','        if (!n->leaf){\n          q->child[q->n + 1] = n->child[j + 1];\n          q->child_incarnation[q->n+1]=n->child_incarnation[j+1];\n        }')
 change('      if (n->leaf&&TREE_MUTANT!=3)\n        q->next = n->next;','      if (n->leaf&&TREE_MUTANT!=3){\n        q->next = n->next;\n        q->next_incarnation=n->next_incarnation;\n      }')
 change('        n->child[n->n] = q->child[0];','        n->child[n->n] = q->child[0];\n        n->child_incarnation[n->n]=q->child_incarnation[0];')
 change('        if (!n->leaf)\n          n->child[n->n + 1] = q->child[j + 1];','        if (!n->leaf){\n          n->child[n->n + 1] = q->child[j + 1];\n          n->child_incarnation[n->n+1]=q->child_incarnation[j+1];\n        }')
 change('      if (n->leaf&&TREE_MUTANT!=3)\n        n->next = q->next;','      if (n->leaf&&TREE_MUTANT!=3){\n        n->next = q->next;\n        n->next_incarnation=q->next_incarnation;\n      }')
 change('    s->root = n->child[0];','    s->root = n->child[0];\n    s->root_incarnation=n->child_incarnation[0];')
 s+='''
int store_ref_make(store *s,uint32_t id,StoreRef *out){
 if(!s||!out||s->poisoned||s->active)return STORE_STATE;
 store_node n;int r=node(s,id,&n);if(r)return r;
 *out=(StoreRef){s,id,n.incarnation};return STORE_OK;
}
int store_ref_read(StoreRef ref,store_node *out){
 store *s=ref.owner;if(!s||!out||s->poisoned||s->active)return STORE_STATE;
 if(!reference_ok(s,ref.id,ref.incarnation))return STORE_STALE;
 store_node n;int r=node(s,ref.id,&n);if(r)return r;*out=n;return STORE_OK;
}
'''
 public=ROOT/'labs/storage_followthrough/references';private=ROOT/'assessor/storage_followthrough/references'
 if not check:(public/'include').mkdir(parents=True,exist_ok=True);private.mkdir(parents=True,exist_ok=True)
 h=(ROOT/'labs/storage_followthrough/tree/include/store.h').read_text(encoding='utf8').replace('STORE_UNIMPLEMENTED = 8','STORE_UNIMPLEMENTED = 8, STORE_STALE = 9').replace('TREE_CAPACITY==3u || TREE_CAPACITY==7u','TREE_CAPACITY==7u').replace('two derived odd capacities','CST5 capacity7 only')
 h=h.replace('  uint32_t next, child[9];','  uint32_t next, child[9];\n  uint64_t incarnation,next_incarnation,child_incarnation[9];').replace('  uint8_t touched_ids[31], touched_count;','  uint8_t touched_ids[31], touched_count;\n  uint64_t root_incarnation,saved_root_incarnation,incarnations[32],saved_incarnations[32];')
 def emit(path,text):
  data=text.encode('utf8')
  if check:assert path.read_bytes()==data,str(path)
  else:path.write_bytes(data)
 emit(public/'include/store.h',h);emit(private/'refs.c',s)
 emit(public/'include/tree.h',(ROOT/'labs/storage_followthrough/tree/include/tree.h').read_text(encoding='utf8'))
 emit(public/'tree_cases.c',(ROOT/'labs/storage_followthrough/tree/tests.c').read_text(encoding='utf8'))
 stub=(ROOT/'labs/storage_followthrough/tree/tree.c').read_text(encoding='utf8').replace('#include "store.h"','#include "refs.h"')
 stub+='\nint store_ref_make(store *s,uint32_t id,StoreRef *out){(void)s;(void)id;(void)out;return STORE_UNIMPLEMENTED;}\nint store_ref_read(StoreRef ref,store_node *out){(void)ref;(void)out;return STORE_UNIMPLEMENTED;}\n'
 emit(public/'refs.c',stub)
 print(dict(parent_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),output_sha256=hashlib.sha256((private/'refs.c').read_bytes()).hexdigest()))
if __name__=='__main__':
 import argparse
 p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');main(p.parse_args().check)
