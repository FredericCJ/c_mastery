"""Independent abstract oracles, not observations of a storage device/kernel."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[2]

def persistence():
    # Initial old file durable. File flush commits bytes but not its replacement name.
    states=[]
    for cut in range(5):
        visible='new' if cut>=3 else 'old'
        durable='new' if cut==4 else ('unspecified' if cut==3 else 'old')
        states.append({'cut':cut,'visible':visible,'durable':durable})
    assert states[3]['visible']=='new' and states[3]['durable']=='unspecified'
    return states

def main():
    data=json.loads((ROOT/'fixtures/systems/PR-T23-05.json').read_text())
    # Read fixture shape explicitly rather than assume target output is its oracle.
    print('PC_FIXTURE_STORAGE',json.dumps(data,sort_keys=True))
    states=persistence();print('PC_PERSISTENCE_MODEL',json.dumps(states,sort_keys=True))
    assert data['input']['stops']==list(range(5))
    assert data['input']['stages']==['write_temp','sync_temp','rename','sync_directory']
    assert data['expected']['states']==[dict(completed=s['cut'],visible=s['visible'],guaranteed_durable=s['durable']) for s in states]
    assert [(s['visible'],s['durable']) for s in states]==[('old','old'),('old','old'),('old','old'),('new','unspecified'),('new','new')]
    count=0
    for d in range(33):
        for n in range(33):
            # Integer interval cardinality is independent of the C subtraction branch.
            expected=len(range(n,d))
            assert expected==(d-n if n<d else 0)
            count+=1
    deadlines=[5,10,20];dispatch=[7,13,19]
    lateness=[len(range(d,w)) for d,w in zip(deadlines,dispatch)]
    assert lateness==[2,3,0] and sum(d<=12 for d in deadlines)==2
    # Interference consumes [20,120); no bounded-environment premise excludes it.
    interference={'release':20,'higher_priority_runs_until':120,'lateness':100}
    assert interference['higher_priority_runs_until']-interference['release']==interference['lateness']
    print('PC_ORACLE_PASS',json.dumps({'deadline_pairs':count,'interference':interference,'power_loss_executed':False}))
if __name__=='__main__':main()
