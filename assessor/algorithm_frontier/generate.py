"""Refine the exact completed T15 functions at queue/selection boundaries only."""
import hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def function(text,name):
 start=text.index(name);start=text.rfind('\n',0,start)+1;opening=text.index('{',start);depth=1;end=opening+1
 while depth:
  depth+=(text[end]=='{')-(text[end]=='}');end+=1
 return text[start:end]+'\n'
def once(text,old,new):
 assert text.count(old)==1,old
 return text.replace(old,new)
def generated():
 text=(ROOT/'assessor/algorithm_depth/graph_dp.c').read_text(encoding='utf8');base=[function(text,n) for n in ['graph_valid(','empty(','ad_bfs(','ad_dijkstra(']];bfs=base[2];weighted=base[3]
 bfs=once(bfs,'ad_bfs(','ag_refined_bfs(');bfs=once(bfs,'unsigned*queue','int*queue');bfs=once(bfs,'struct ad_traversal*out)','struct ad_traversal*out,struct ag_trace*trace)')
 bfs=once(bfs,'unsigned head=0,tail=1;queue[0]=root;','struct cd_ring ring;size_t written;int initial=(int)root;if(cd_ring_init(&ring,queue,capacity)!=CD_OK||cd_ring_push(&ring,&initial,1,0,&written)!=CD_OK)return AD_SPACE;++trace->queue_pushes;')
 bfs=once(bfs,'while(head<tail){if(t.peak<tail-head)t.peak=tail-head;unsigned v=queue[head++];','while(ring.count){if(t.peak<ring.count)t.peak=(unsigned)ring.count;ag_bfs_snapshot(trace,&t,seen,&ring);int vertex;if(cd_ring_pop(&ring,&vertex,1)!=CD_OK)return AD_FAILED;++trace->queue_pops;unsigned v=(unsigned)vertex;')
 bfs=once(bfs,'queue[tail++]=w;','int value=(int)w;if(cd_ring_push(&ring,&value,1,0,&written)!=CD_OK)return AD_SPACE;++trace->queue_pushes;')
 weighted=once(weighted,'ad_dijkstra(','ag_refined_dijkstra(');weighted=once(weighted,'unsigned previous[8])','unsigned previous[8],struct ag_workspace*workspace,struct ag_trace*trace)')
 weighted=once(weighted,'limit==UINT64_MAX','limit>65534u')
 weighted=once(weighted,'distance[start]=0;pred[start]=start;','distance[start]=0;pred[start]=start;struct cd_heap*heap=&workspace->heap;struct cd_heap_handle*handles=workspace->handles;unsigned open=1u<<start;cd_heap_init(heap,0);if(cd_heap_insert(heap,(int)start,start,&handles[start])!=CD_OK)return AD_SPACE;++trace->heap_inserts;')
 weighted=once(weighted,'unsigned v=8;for(unsigned i=0;i<g->vertices;++i)if(!(done&(1u<<i))&&distance[i]!=UINT64_MAX&&(v==8||distance[i]<distance[v]))v=i;if(v==8)break;','if(!heap->count)break;ag_weight_snapshot(trace,distance,done,heap);unsigned v;int priority;if(cd_heap_take(heap,&priority,&v)!=CD_OK)return AD_FAILED;open&=~(1u<<v);++trace->heap_takes;')
 weighted=once(weighted,'distance[w]=next;pred[w]=v;','ag_relax_snapshot(trace,e,v,w,distance[w],next);distance[w]=next;pred[w]=v;int encoded=(int)(next*8u+w);if(open&(1u<<w)){if(cd_heap_change(heap,handles[w],encoded)!=CD_OK)return AD_FAILED;++trace->heap_changes;}else{if(cd_heap_insert(heap,encoded,w,&handles[w])!=CD_OK)return AD_SPACE;open|=1u<<w;++trace->heap_inserts;}')
 weighted=once(weighted,'}return AD_OK;','}trace->heap_comparisons=heap->comparisons;return AD_OK;')
 # Mutants alter only an explicitly identified semantic premise or adaptation.
 weighted=once(weighted,'if(weight>limit-distance[v])return AD_OVERFLOW;','\n#ifndef AG_MUTANT_RANGE\nif(weight>limit-distance[v])return AD_OVERFLOW;\n#endif\n')
 weighted=once(weighted,'int encoded=(int)(next*8u+w);','int encoded=(int)(next*8u+w);\n#ifdef AG_MUTANT_TIE\nencoded=(int)(next*8u);\n#endif\n')
 weighted=once(weighted,'if(cd_heap_change(heap,handles[w],encoded)!=CD_OK)return AD_FAILED;','\n#ifndef AG_MUTANT_UPDATE\nif(cd_heap_change(heap,handles[w],encoded)!=CD_OK)return AD_FAILED;\n#endif\n')
 return '#include "frontier.h"\n'+''.join(base[:2])+bfs+weighted,{n:hashlib.sha256(v.encode()).hexdigest() for n,v in zip(['graph_valid','empty','ad_bfs','ad_dijkstra'],base)}
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True);source,pins=generated();(out/'refined.c').write_text(source,encoding='utf8');(out/'provenance.json').write_text(json.dumps(dict(provider='assessor/algorithm_depth/graph_dp.c',functions=pins,transform='Exact local substitutions at frontier operations; validator, discovery and relaxation bodies retained.'),indent=2)+'\n');print('OBS generated actual frontier refinement')
if __name__=='__main__':main()
