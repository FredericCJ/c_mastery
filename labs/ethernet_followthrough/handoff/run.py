"""Explicit learner providers only; never infer a private solution path."""
import argparse,json,pathlib,subprocess,sys
HERE=pathlib.Path(__file__).resolve().parent
KEYS=['tcp','arp','ip','scope','stream','http','systems','ring','ring_model','phy','phy_model','ethernet','bringup','runtime','strings','network_driver','flow','application']
INCLUDES=['labs','labs/bringup_depth','labs/network_followthrough/driver','labs/network_followthrough/tcp','labs/network_followthrough/arp','labs/network_followthrough/ip','labs/network_followthrough/scope','labs/b045_systems/include','labs/communications/include','labs/ethernet_followthrough/ring','labs/ethernet_followthrough/phy','labs/ethernet_depth/include','labs/network_followthrough/http','labs/hosted_followthrough/http']
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=pathlib.Path,required=True);p.add_argument('--providers',type=pathlib.Path,required=True);p.add_argument('--source',type=pathlib.Path,default=HERE/'learner.c');p.add_argument('--root',type=pathlib.Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--flag',action='append',default=[]);p.add_argument('--compile-only',action='store_true');a=p.parse_args()
 providers=json.loads(a.providers.read_text());missing=set(KEYS)-set(providers)
 if missing or set(providers)-set(KEYS):p.error('missing explicit providers or unknown keys: '+str(sorted(missing)))
 for n in [*providers.values(),a.source]:
  if not pathlib.Path(n).is_file():p.error('missing explicit source '+str(n))
 a.build.mkdir(parents=True,exist_ok=True);checksum=a.build/'checksum.c';subprocess.run([sys.executable,a.root/'labs/network_followthrough/ip/checksum_slice.py',providers['systems'],checksum],check=True)
 sources=[a.source,*[HERE/n for n in ['tests.c','stack.c','reassembly.c','native.c']],*[checksum if k=='systems' else pathlib.Path(providers[k]) for k in KEYS if k not in ['runtime','strings']]]
 cmd=[a.cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-fno-pie','-no-pie',*a.flag,'-I'+str(HERE),*['-I'+str(a.root/n) for n in INCLUDES],*map(str,sources),'-o',str(a.build/'test')];print(cmd,flush=True);subprocess.run(cmd,check=True)
 if not a.compile_only:
  for i in range(6):subprocess.run([a.build/'test',str(i)],check=True)
 print('EH_NATIVE_BUILD_PASS explicit_original_providers')
if __name__=='__main__':main()
