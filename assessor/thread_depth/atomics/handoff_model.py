"""Finite operational slot model; not an implementation of the full C model.

Explore every transition boundary for two publications and one/two readers.
RF/SW obligations on the actual atomic mask are proved in ANALYSIS.md; this
model checks the reuse/lifetime state machine under those obligations.
"""
import collections,json

def explore(readers,early_clear=False):
    # Writer(pc,generation), current payload/mask, per-reader(pc,generation,copy).
    initial=(0,1,0,0,tuple((0,1,0) for _ in range(readers)))
    pending=collections.deque([initial]);seen={initial};edges=0;ends=0
    parent={initial:None};label={};witness=None
    while pending:
        state=pending.popleft();wp,wg,payload,mask,rs=state
        candidates=[]
        if wg<=2:
            if wp==0 and mask==0:candidates.append(('writer-check-zero',(1,wg,payload,mask,rs)))
            if wp==1:candidates.append(('writer-write',(2,wg,wg,mask,rs)))
            if wp==2:candidates.append(('writer-publish',(0,wg+1,payload,(1<<readers)-1,rs)))
        for i,(pc,g,copied) in enumerate(rs):
            if g>2:continue
            updated=list(rs);newmask=mask;newcopy=copied
            if pc==0 and mask&(1<<i):
                step='reader-observe';nextpc=1
                if early_clear:newmask&=~(1<<i)
            elif pc==1:step='reader-copy';nextpc=2;newcopy=payload
            elif pc==2:
                step='reader-retire';nextpc=0
                if not early_clear:newmask&=~(1<<i)
            else:continue
            if pc==1 and payload!=g and witness is None:
                trace=[];p=state
                while parent[p] is not None:trace.append(label[p]);p=parent[p]
                witness=list(reversed(trace))+[step+str(i)]
            updated[i]=(nextpc,g+(pc==2),newcopy)
            candidates.append((step+str(i),(wp,wg,payload,newmask,tuple(updated))))
        if wg==3 and all(g==3 for _,g,_ in rs):ends+=1
        for name,nxt in candidates:
            edges+=1
            if nxt not in seen:seen.add(nxt);parent[nxt]=state;label[nxt]=name;pending.append(nxt)
    return dict(readers=readers,states=len(seen),edges=edges,terminal_states=ends,early_clear=early_clear,bad_read_witness=witness)

def main():
    results=[explore(n,bug) for n in [1,2] for bug in [False,True]]
    for r in results:
        assert r['terminal_states']>0
        assert bool(r['bad_read_witness'])==r['early_clear'],r
    print(json.dumps(dict(status='pass',models=results,limits='Two generations, fixed enrolled readers, no cancellation. Weak-memory ordering requires the separate RF/SW proof; model is not all C executions.'),indent=2))
if __name__=='__main__':main()
