"""Unprivileged, explicit-prefix tool/source provisioning for T10 experiments."""
import argparse,hashlib,json,subprocess,tarfile,urllib.request
from pathlib import Path
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix',required=True,type=Path);a=p.parse_args();prefix=a.prefix.resolve();download=prefix/'downloads';target=prefix/'root';source=prefix/'source'
 for folder in (download,target,source):folder.mkdir(parents=True,exist_ok=True)
 commands=[]
 def run(argv,cwd=download):
  r=subprocess.run(list(map(str,argv)),cwd=cwd,capture_output=True,text=True,timeout=180);commands.append({'argv':list(map(str,argv)),'cwd':str(cwd),'exit':r.returncode,'stdout':r.stdout,'stderr':r.stderr})
  if r.returncode:raise RuntimeError(commands[-1])
 run(['apt-get','download','musl=1.2.5-3build1','musl-dev=1.2.5-3build1','musl-tools=1.2.5-3build1','strace=6.19+ds-0ubuntu5','libunwind8=1.8.3-0ubuntu1','glibc-source=2.43-2ubuntu2.4'])
 packages=sorted(download.glob('*.deb'))
 for package in packages:run(['dpkg-deb','-x',package,target])
 archives=list((target/'usr/src/glibc').glob('*.tar.*'))
 if len(archives)!=1:raise RuntimeError(('glibc source archive count',archives))
 for archive in archives:
  with tarfile.open(archive) as stream:stream.extractall(source,filter='data')
 musl=download/'musl-1.2.5.tar.gz';url='https://musl.libc.org/releases/musl-1.2.5.tar.gz'
 if not musl.exists():
  with urllib.request.urlopen(url,timeout=60) as response:musl.write_bytes(response.read())
 with tarfile.open(musl) as stream:stream.extractall(source,filter='data')
 specs=target/'usr/lib/x86_64-linux-musl/musl-gcc.specs';text=specs.read_text()
 relocated=prefix/'musl-local.specs';relocated.write_text(text.replace('/usr/lib/x86_64-linux-musl',str(target/'usr/lib/x86_64-linux-musl')).replace('/usr/include/x86_64-linux-musl',str(target/'usr/include/x86_64-linux-musl')).replace('/lib/ld-musl-x86_64.so.1',str(target/'usr/lib/x86_64-linux-musl/libc.so')))
 pins={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [*packages,*archives,musl,relocated]}
 manifest={'scope':'Downloaded packages and source extracted only under explicit prefix; no system installation or host configuration change.','commands':commands,'sha256':pins,'musl_source_url':url,'musl_specs':str(relocated),'strace':str(target/'usr/bin/strace')}
 (prefix/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(json.dumps({'passed':True,'prefix':str(prefix),'artifacts':len(pins),'musl_specs':str(relocated)}))
if __name__=='__main__':main()
