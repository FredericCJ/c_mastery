"""Depfile-driven build: source unchanged, resource content identity drives rebuild."""
import argparse,hashlib,json,shlex,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('compiler');p.add_argument('folder');a=p.parse_args();d=Path(a.folder);manifest=d/'manifest.json'
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
old=json.loads(manifest.read_text()) if manifest.exists() else None
changed=not old or old['compiler']!=sha(a.compiler) or any(not Path(k).is_file() or sha(k)!=v for k,v in old['inputs'].items())
command=[a.compiler,'-std=c23','-pedantic-errors','-MD','-MF',str(d/'embed.d'),str(d/'embed.c'),'-o',str(d/'embed')]
if changed:
 subprocess.run(command,check=True)
 deps=shlex.split((d/'embed.d').read_text().replace('\\\n',' ').split(':',1)[1]);assert str(d/'resource.bin') in deps
 state=dict(inputs={str(Path(k).resolve()):sha(k) for k in deps},compiler=sha(a.compiler),binary=sha(d/'embed'));manifest.write_text(json.dumps(state,sort_keys=True))
print(json.dumps(dict(rebuilt=bool(changed),compile_command=command if changed else None,output=subprocess.check_output([str(d/'embed')],text=True).strip())))
