"""Public native composition. Every predecessor source is explicitly supplied."""
from pathlib import Path
import argparse,json,subprocess,sys
HERE=Path(__file__).resolve().parent;LABS=HERE.parents[1]
KEYS=['tcp','arp','ip','scope','stream','http','systems','ring','ring_model','phy','phy_model','ethernet','bringup','network_driver','flow']
INCLUDES=['','bringup_depth','network_followthrough/driver','network_followthrough/tcp','network_followthrough/arp','network_followthrough/ip','network_followthrough/scope','b045_systems/include','communications/include','ethernet_followthrough/ring','ethernet_followthrough/phy','ethernet_depth/include','network_followthrough/http','hosted_followthrough/http']
def main():
 p=argparse.ArgumentParser();p.add_argument('--providers',type=Path,required=True);p.add_argument('--source',type=Path,default=HERE/'learner.c');p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='cc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
 sources=json.loads(a.providers.read_text());missing=set(KEYS)-set(sources)
 if missing:p.error('missing explicit providers: '+','.join(sorted(missing)))
 for k in KEYS:
  if not Path(sources[k]).is_file():p.error('missing explicit source '+k)
 checksum=a.build/'checksum.c';subprocess.run([sys.executable,LABS/'network_followthrough/ip/checksum_slice.py',sources['systems'],checksum],check=True)
 exe=a.build/'http';subprocess.run([a.cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*['-I'+str(LABS/n) for n in INCLUDES],a.source,HERE/'tests.c',*[checksum if k=='systems' else Path(sources[k]) for k in KEYS],'-o',exe],check=True)
 return subprocess.run([exe]).returncode
if __name__=='__main__':raise SystemExit(main())
