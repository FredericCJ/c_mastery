"""Original ASan sensitivity targets only; retain findings without undefined-output oracles."""
import json,os,pathlib,subprocess,sys
binary,out=sys.argv[1:3];missing_overlap=len(sys.argv)==4 and sys.argv[3]=='expect-direct-overlap-miss';env=os.environ.copy();env.update(ASAN_OPTIONS='detect_stack_use_after_return=1:detect_leaks=1:halt_on_error=1:exitcode=86',UBSAN_OPTIONS='halt_on_error=1');observations=[]
for case,expected in [('escape','stack-use-after-return'),('release','double-free'),('access','heap-use-after-free'),('overlap','memcpy-param-overlap'),('overlap-indirect','memcpy-param-overlap')]:
    p=subprocess.run([binary,case],env=env,capture_output=True,text=True,timeout=30);text=p.stdout+p.stderr
    found='AddressSanitizer' in text and expected in text;miss=missing_overlap and case=='overlap'
    observations.append(dict(case=case,command=[binary,case],exit_code=p.returncode,diagnostic=expected,detected=found,expected_detection=not miss,stdout=p.stdout,stderr=p.stderr))
    assert (p.returncode==0 and not text) if miss else (p.returncode!=0 and found),(case,p.returncode,text)
report=dict(status='pass',observations=observations,environment={k:env[k] for k in ['ASAN_OPTIONS','UBSAN_OPTIONS']},limits=['Four original defects plus a second indirect-call version of overlap run in isolated child processes; no undefined-value oracle.','Selected optimized direct-copy diagnostic miss is contrasted with retained indirect-call detection; no missing report proves safety.','Nonzero diagnostic termination demonstrates neither exploitability nor all-path correctness. Corrected implementations are tested separately.'])
pathlib.Path(out).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(status='pass',diagnostic_cases=len(observations))))
