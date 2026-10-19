"""Explicit-prefix newlib packages/source; no privileged installation."""
import argparse,hashlib,json,subprocess,tarfile,urllib.request
from pathlib import Path
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--prefix',required=True,type=Path);a=p.parse_args();prefix=a.prefix.resolve();download=prefix/'downloads';target=prefix/'root';source=prefix/'source'
 for folder in (download,target,source):folder.mkdir(parents=True,exist_ok=True)
 commands=[]
 def run(argv):
  r=subprocess.run(list(map(str,argv)),cwd=download,capture_output=True,text=True,timeout=240);commands.append(dict(argv=list(map(str,argv)),cwd=str(download),exit=r.returncode,stdout=r.stdout,stderr=r.stderr))
  if r.returncode:raise RuntimeError(commands[-1])
 run(['apt-get','download','libnewlib-dev=4.6.0.20260123-1','libnewlib-arm-none-eabi=4.6.0.20260123-1'])
 packages=sorted(download.glob('*.deb'))
 for package in packages:run(['dpkg-deb','-x',package,target])
 archive=download/'newlib-4.6.0.20260123.tar.gz';url='https://sourceware.org/pub/newlib/'+archive.name
 if not archive.exists():
  with urllib.request.urlopen(url,timeout=90) as response:archive.write_bytes(response.read())
 with tarfile.open(archive) as stream:stream.extractall(source,filter='data')
 manifest=dict(scope='Private packages/source, no system install; library dependency experiment is explicitly outside strict no-library profile.',commands=commands,source_url=url,sha256={str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [*packages,archive]})
 (prefix/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');print(dict(passed=True,prefix=str(prefix),artifacts=len(manifest['sha256'])))
if __name__=='__main__':main()
