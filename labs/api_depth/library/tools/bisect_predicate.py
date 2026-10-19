"""Translate only this exact teaching failure; unbuildable is125, harness is200."""
import json,pathlib,subprocess,sys
repo,out,provider,header,test=map(pathlib.Path,sys.argv[1:]);out.mkdir(parents=True,exist_ok=True)
mode=subprocess.run(['git','show','BISECT_HEAD:mode.h'],cwd=repo,stdout=subprocess.PIPE,check=True).stdout
(out/'mode.h').write_bytes(mode)
p=subprocess.run(['gcc','-std=c17','-I'+str(header),'-include',str(out/'mode.h'),str(provider),str(test),'-o',str(out/'case')],stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
if p.returncode:code=125;result=p.stdout.decode(errors='replace')
else:
 p=subprocess.run([str(out/'case'),'2'],stdout=subprocess.PIPE,stderr=subprocess.STDOUT);result=p.stdout.decode(errors='replace');code=0 if p.returncode==0 else 1 if p.returncode==17 and 'SAME_FAILURE_REJECTED_APPEND_LENGTH' in result else 200
with (out/'predicate.jsonl').open('a') as f:f.write(json.dumps(dict(mode=mode.decode(),compiler_or_test_output=result,translated_status=code))+'\n')
print(result,end='');raise SystemExit(code)
