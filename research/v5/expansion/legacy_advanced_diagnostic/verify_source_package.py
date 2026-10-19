"""Compare inspected source bytes with the selected distribution source package."""
from pathlib import Path
import hashlib,json,subprocess,tarfile
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
base=Path('/home/frede/cmastery-diagnostic-tools');deb=base/'packages/linux-source-7.0.0_7.0.0-38.38_all.deb';dest=base/'source-package';dest.mkdir(exist_ok=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
expected='997a13a0c7fd0172a93b0c1ae9a4ffc7d3cbbf91d2fdd91d85334193580687de'
assert sha(deb)==expected
commands=[]
for cmd in [['dpkg-deb','-f',str(deb),'Package','Version','Source'],['dpkg-deb','-x',str(deb),str(dest)]]:
 r=subprocess.run(cmd,capture_output=True,text=True);commands.append(dict(command=cmd,returncode=r.returncode,stdout=r.stdout,stderr=r.stderr));assert r.returncode==0
archive=dest/'usr/src/linux-source-7.0.0.tar.bz2';original=Path('/home/frede/cmastery-b06-tools/usr/src/linux-source-7.0.0.tar.bz2')
assert sha(archive)==sha(original)
rel=['drivers/usb/gadget/udc/core.c','drivers/usb/gadget/udc/dummy_hcd.c','drivers/usb/gadget/epautoconf.c','drivers/usb/core/devio.c','Makefile'];want={'linux-source-7.0.0/'+p:p for p in rel};rows=[]
with tarfile.open(archive,'r:bz2') as t:
 for item in t:
  if item.name in want:
   local=Path('/home/frede/cmastery-b06-tools/debug-kernel')/item.name;data=t.extractfile(item).read();digest=hashlib.sha256(data).hexdigest();rows.append(dict(member=item.name,inspected=str(local),package_sha256=digest,inspected_sha256=sha(local),equal=digest==sha(local)))
  if len(rows)==len(want):break
passed=len(rows)==len(want) and all(r['equal'] for r in rows)
out=dict(passed=passed,package=dict(path=str(deb),version='7.0.0-38.38',sha256=sha(deb),expected_apt_sha256=expected),archive=dict(path=str(archive),sha256=sha(archive),prior_archive=str(original)),commands=commands,comparisons=rows,limits=['Source package and inspected selected source bytes agree; upstream Makefile7.0.14 and Ubuntu release7.0.0-38 are different naming schemes.','This does not independently reproduce the distributed kernel binary.'])
(HERE/'kernel_source_identity.json').write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=passed,files=len(rows))));raise SystemExit(not passed)
