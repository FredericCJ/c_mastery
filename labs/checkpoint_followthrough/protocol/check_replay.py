"""The original minimized wrong-output property under changed chunks/capacity."""
import argparse,hashlib,importlib.util,json,pathlib,re,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[3]
def load_oracle(path):
    spec=importlib.util.spec_from_file_location('independent_p3_oracle',path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module.oracle
def main():
    a=argparse.ArgumentParser();a.add_argument('reference');a.add_argument('faulty');a.add_argument('receipt');a.add_argument('oracle');a.add_argument('output');p=a.parse_args();old=json.loads(pathlib.Path(p.receipt).read_text());oracle=load_oracle(pathlib.Path(p.oracle));assert old['passed'] and old['single_byte_deletion_minimal'];observations=[]
    for seed in ['original_hex','minimized_hex']:
        data=bytes.fromhex(old[seed]);expected=oracle(data);assert '7:' in expected
        for chunk in [1,7,256]:
            for capacity in [1,2]:
                prefix=[str(900+i)+':' for i in range(capacity)];results=[]
                for executable in [p.reference,p.faulty]:
                    args=[executable,data.hex(),str(chunk),str(capacity),'80'];q=subprocess.run(args,capture_output=True,text=True,timeout=10);assert q.returncode==0,(args,q.returncode,q.stdout,q.stderr);m=re.fullmatch(r'CPA_OBS capacity=(\d+) peak=(\d+) refused=(\d+) turns=(\d+) bound=(\d+) bytes=(\d+)\n',q.stderr);assert m,q.stderr;values=list(map(int,m.groups()));assert values[0]==capacity and values[1]==capacity and values[3]<=values[4] and values[5]==len(data);results.append(dict(command=args,exit_code=q.returncode,frames=q.stdout.splitlines(),capacity=values[0],peak=values[1],refusals=values[2],turns=values[3],bound=values[4]))
                good,bad=results;assert good['frames']==prefix+expected and good['refusals']>0;assert bad['frames'][:capacity]==prefix and '7:' not in bad['frames'];observations.append(dict(seed=seed,chunk=chunk,capacity=capacity,expected=prefix+expected,reference=good,faulty=bad,property='missing nested synthetic sequence7, with normal child exit; not a crash/timeout'))
    report=dict(status='pass',observations=observations,receipt_sha256=hashlib.sha256(pathlib.Path(p.receipt).read_bytes()).hexdigest(),limits=['Admission capacity1/2 changes allowed occupancy, not the fixed two-element struct cp_queue physical array.','Original retained deletion-minimal witness is replayed unchanged; no new global-minimality claim.']);pathlib.Path(p.output).write_text(json.dumps(report,indent=2)+'\n');print('CPA_REPLAY_PASS12 paired original/minimized changed-schedule cells')
if __name__=='__main__':main()
