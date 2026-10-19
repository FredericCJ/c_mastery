"""Observe original uninitialized-use targets; no value/exit result is a C semantic oracle."""
import json,pathlib,subprocess,sys
debug,optimized,sanitized,out=sys.argv[1:5];observations=[]
for profile,binary in [('debug',debug),('optimized',optimized),('sanitized',sanitized)]:
    for branch in ['0','1']:
        args=[binary,branch] if profile=='sanitized' else ['valgrind','--tool=memcheck','--track-origins=yes','--error-exitcode=99','--leak-check=full',binary,branch]
        p=subprocess.run(args,capture_output=True,text=True,timeout=45);text=p.stdout+p.stderr
        observations.append(dict(profile=profile,branch='initialized' if branch=='1' else 'uninitialized',command=args,exit_code=p.returncode,stdout=p.stdout,stderr=p.stderr,uninitialized_report='uninitialised' in text,asan_report='AddressSanitizer' in text,ubsan_report='runtime error:' in text))
        if branch=='1':assert p.returncode==0 and not any(s in text for s in ['uninitialised','AddressSanitizer','runtime error:']),text
        elif profile=='debug':assert p.returncode==99 and 'Conditional jump or move depends on uninitialised value' in text and 'diagnostic.c' in text,text
        elif profile=='optimized':assert p.returncode in [0,99],text
        else:assert p.returncode in [0,1,-6],text
report=dict(status='pass',observations=observations,limits=['Only the initialized branch is a defined positive program. The uninitialized original local target is diagnostic sensitivity evidence, never a golden result or harmlessness proof.','Optimization can remove the problematic machine access before Memcheck observes it. ASan/UBSan do not promise general initialization tracking.','One expected debug Memcheck report is mandatory; other valid diagnostic outcomes are retained rather than rewritten as language semantics.'])
pathlib.Path(out).write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(status='pass',observations=len(observations),uninitialized_detected=[q['profile'] for q in observations if q['branch']=='uninitialized' and(q['uninitialized_report'] or q['asan_report'] or q['ubsan_report'])])))
