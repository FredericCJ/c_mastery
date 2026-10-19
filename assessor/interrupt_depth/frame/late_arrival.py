"""Architectural event model; no assertion of physical recognition time."""
import copy,json

def trace(events,wrong_context=False,wrong_tail=False):
    pending=[];active=[];frames=[];out=[];stacking=None;history=[]
    priority={'A':6,'B':5,'C':6}
    for kind,arg in events:
        if kind=='pend':
            if arg not in pending:pending.append(arg)
        elif kind=='begin':
            assert stacking is None and pending
            stacking=active[-1] if active else 'thread'
            frames.append(stacking)
        elif kind=='enter':
            assert stacking is not None and pending
            winner=min(pending,key=lambda k:(priority[k],k));pending.remove(winner)
            if wrong_context and winner=='B' and stacking=='thread':frames[-1]='A'
            active.append(winner);stacking=None
        elif kind=='return':
            assert active and active[-1]==arg and stacking is None
            active.pop();return_context=frames[-1]
            eligible=[p for p in pending if return_context=='thread' or priority[p]<priority[return_context]]
            if eligible:
                winner=min(eligible,key=lambda k:(priority[k],k));pending.remove(winner);active.append(winner)
                if wrong_tail:frames.append(return_context)
            else:frames.pop()
        else:raise AssertionError('unknown phase')
        out.append(dict(event=[kind,arg],active=list(active),pending=sorted(pending),saved_contexts=list(frames)))
        history.append(len(frames))
    assert not frames and not active and not pending and stacking is None
    return out,max(history)

LATE=[('pend','A'),('begin',None),('pend','B'),('enter',None),('return','B'),('pend','C'),('return','A'),('return','C')]
NORMAL=[('pend','A'),('begin',None),('enter',None),('pend','B'),('pend','C'),('begin',None),('enter',None),('return','B'),('return','A'),('return','C')]

def verify(which,rows,depth):
    entered=[r['active'][-1] for r in rows if r['event'][0]=='enter']
    assert entered==(['B'] if which=='late' else ['A','B'])
    assert depth==(1 if which=='late' else 2)
    if which=='late':
        assert rows[3]['saved_contexts']==['thread'] and rows[4]['active']==['A']
        assert rows[4]['saved_contexts']==['thread'] and rows[6]['active']==['C']
    else:
        assert rows[6]['saved_contexts']==['thread','A'] and rows[7]['active']==['A']
        assert rows[7]['pending']==['C'] and rows[8]['active']==['C']

def main():
    results={}
    for name,events in [('late',LATE),('normal',NORMAL)]:
        r,d=trace(events);verify(name,r,d);results[name]=dict(events=r,max_hardware_frames=d)
    controls=[]
    for kind in ['late-saves-A','tailchain-pushes']:
        try:
            r,d=trace(LATE,kind=='late-saves-A',kind=='tailchain-pushes');verify('late',r,d)
        except AssertionError:controls.append(kind)
    assert len(controls)==2
    print(json.dumps(dict(passed=True,scope='explicit phase model, not hardware late-arrival timing',traces=results,rejected=controls),indent=2))
if __name__=='__main__':main()
