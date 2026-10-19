"""Check workload equality independently of timing; no speed threshold."""
import argparse
import json
import subprocess

MODES = ('select_shared','select_registered','deque_sc','deque_weak','deque_mutex',
         'sequence_sc','sequence_weak','sequence_mutex','transaction','transaction_mutex',
         'snapshot_pins','snapshot_copy','stream_item','stream_batch16','observer',
         'representation','fairness','operation_owned','actor_owned','supervisor_tree','thread_stack',
         'deque_sc4','deque_weak4','deque_mutex4','snapshot_pins4','snapshot_copy4')


def validate(row,n):
    mode=row['mode']
    if mode=='representation':
        assert len(row['types'])>=30
        assert all(v['bytes']>0 and v['align']>0 for v in row['types'].values())
        return
    if mode=='fairness':
        assert row==dict(mode='fairness',rounds=n,a_attempts=n,a_success=0,b_success=n)
        return
    if mode=='thread_stack':
        assert len(row['stack_bytes'])==len(row['guard_bytes'])==8
        assert all(v>0 for v in row['stack_bytes']) and all(v>=0 for v in row['guard_bytes'])
        assert row['spawn_to_ready_ns']>=0 and row['cancel_to_join_ns']>=0
        return
    threads=row['threads'];success=[t['success'] for t in threads]
    assert len(threads)==(5 if mode in ('deque_sc4','deque_weak4','deque_mutex4') else 3) and row['elapsed_ns']>=0
    assert all(t['retries']>=0 and all(v>=0 for v in t['latency_ns']) for t in threads)
    if mode.startswith('select_'):
        assert success==[n,n,2*n]
        assert threads[2]['checksum']==2*n*(2*n-1)//2
    elif mode.startswith('deque_'):
        assert success[0]==n and sum(success[1:])==n
        assert sum(t['checksum'] for t in threads)==n*(n-1)//2
    elif mode.startswith('sequence_'):
        assert success[0]==n and success[1]+threads[1]['retries']==n and success[2]==0
        assert all(len(threads[i]['latency_ns'])==n for i in (0,1))
    elif mode in ('transaction','transaction_mutex'):
        assert success==[n,n,0]
        if mode=='transaction':assert row['extra']>=2*n
    else:
        assert success==[n,0,0]
        if mode.startswith('snapshot_'):
            assert threads[0]['checksum']==(64*n*(n+1)+48*n if mode.endswith('4') else 8*n*(n+1)//2)
            assert row['extra']==(5 if mode=='snapshot_pins4' else 2 if mode=='snapshot_pins' else 0)
        elif mode.startswith('stream_'):
            assert threads[0]['checksum']==n*(n-1)//2
            assert row['extra']==(n if mode=='stream_item' else (n+15)//16)
        elif mode in ('operation_owned','actor_owned'):
            assert threads[0]['checksum']==n*(n-1)//2 and row['extra']==n
        elif mode=='supervisor_tree':
            assert threads[0]['checksum']==4*n and row['extra']==16+2*n
    if not mode.startswith('sequence_'):
        assert [len(t['latency_ns']) for t in threads]==success


def main():
    parser=argparse.ArgumentParser();parser.add_argument('driver');parser.add_argument('n',type=int,nargs='?',default=128)
    args=parser.parse_args()
    for mode in MODES:
        result=subprocess.run([args.driver,mode,str(args.n)],capture_output=True,text=True,timeout=30)
        if result.returncode:
            print(result.stdout+result.stderr);return result.returncode
        row=json.loads(result.stdout);assert row['mode']==mode;validate(row,args.n)
    print('Equal workload, conserved ownership/value counts and coherent pairs checked for all26 modes')
    return 0


if __name__=='__main__':raise SystemExit(main())
