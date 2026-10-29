"""Exact retained 64-slot AVL bodies, separate counted/free-list variants."""
import pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2];sys.path.insert(0,str(ROOT/'tools'))
from c_source import find_unique,replace_unique
def change(s,a,b):
 return replace_unique(s,a,b)
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);s=(ROOT/'assessor/algorithms/algo.c').read_text(encoding='utf8');s=s[find_unique(s,'void algo_avl_init(')[0]:find_unique(s,'int algo_sets_init(')[0]];header='#include "tree.h"\nvoid *memset(void*,int,size_t);\n';(out/'baseline.c').write_text(header+s,encoding='utf8')
 s=change(s,'static void update(algo_avl *t, int p) {','static void update(algo_avl *t, int p) {\n  if(at_work)++at_work->height_updates;')
 for name in ['left','right']:s=change(s,'static int rotate_'+name+'(algo_avl *t, int p) {','static int rotate_'+name+'(algo_avl *t, int p) {\n  if(at_work)++at_work->rotations;')
 s=change(s,'static int balance(algo_avl *t, int p) {','static int balance(algo_avl *t, int p) {\n  if(at_work)++at_work->balances;')
 for predicate in ['d > 1','d < -1']:s=change(s,'if ('+predicate+') {','if ('+predicate+') {\n#ifdef AT_MUTANT_REPAIR\n    return p;\n#endif\n    if(at_work)at_work->repair_roots[at_work->repair_count++]=p;')
 s=change(s,'while (p >= 0) {','while (p >= 0) {\n    if(at_work){++at_work->search_nodes;++at_work->key_predicates;}')
 s=change(s,'p = key < t->nodes[p].key ?','if(at_work)++at_work->key_predicates;\n    p = key < t->nodes[p].key ?')
 allocation='int q = 0;\n  while (q < 64 && t->nodes[q].live)\n    ++q;\n  if (q == 64)\n    return ALGO_FULL;'
 counted=change(s,allocation,'int q = 0;\n  while(q<64){if(at_work)++at_work->allocation_checks;if(!t->nodes[q].live)break;++q;}\n  if(q==64)return ALGO_FULL;')
 free=change(s,allocation,'if(!at_pool->free_count)return ALGO_FULL;\n  if(at_work)++at_work->allocation_checks;\n  int q=(int)at_pool->free_slots[--at_pool->free_count];')
 free=change(free,'n->live = 0;','n->live = 0;\n#ifndef AT_MUTANT_FREE\n    at_pool->free_slots[at_pool->free_count++]=(unsigned)p;\n#endif')
 for name,body in [('observed',counted),('free',free)]:
  body=body.replace('algo_avl_','at_'+name+'_');(out/(name+'.c')).write_text(header+body,encoding='utf8')
 print('OBS generated preserved AVL bodies with counters and separate free-list allocation/release variant')
if __name__=='__main__':main()
