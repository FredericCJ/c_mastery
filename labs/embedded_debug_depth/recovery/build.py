"""Explicit hosted provider imports; learner-owned implementations stay separate."""
import argparse,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent
def commands(cc,flags,source,lifecycle,include,output,test='tests.c',mutant=0,lce_mutant=0,signature=None,sign_mutant=0,signature_mutant=0,crypto_root=None):
 common=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-I'+str(HERE),'-I'+str(include)];obj=Path(str(output)+'.lce.o')
 # Frozen producer has known single-line indentation diagnostics. Keep warnings
 # visible and scope their nonfatal classification to that unchanged TU only.
 compile_common=[x for x in common if x!='-no-pie']
 rows=[[*compile_common,'-Wno-error=misleading-indentation','-DLCE_MUTANT='+str(lce_mutant),'-c',lifecycle,'-o',obj]];objects=[obj]
 link=[]
 if signature:
  if crypto_root:
   common+=['-I'+str(crypto_root/'include'),'-I'+str(crypto_root/'include/x86_64-linux-gnu')];link+=['-L'+str(crypto_root/'lib/x86_64-linux-gnu'),'-Wl,-rpath,'+str(crypto_root/'lib/x86_64-linux-gnu')]
  signobj=Path(str(output)+'.signature.o');rows.append([*[x for x in common if x!='-no-pie'],'-DSIGN_MUTANT='+str(sign_mutant),'-c',signature,'-o',signobj]);objects.append(signobj)
 rows.append([*common,'-DDU_MUTANT='+str(mutant),'-DDU_SIGNATURE_MUTANT='+str(signature_mutant),source,HERE/test,*objects,*link,*(['-lcrypto'] if signature else []),'-o',output]);return [list(map(str,row)) for row in rows]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('source','lifecycle','provider-include','output'):p.add_argument('--'+n,type=Path,required=True)
 p.add_argument('--compiler',default='gcc');p.add_argument('--signature',type=Path);p.add_argument('--crypto-root',type=Path,help='Optional private OpenSSL development prefix, never a target dependency');a=p.parse_args()
 for c in commands(a.compiler,['-O0'],a.source,a.lifecycle,a.provider_include,a.output,'signature.c' if a.signature else 'tests.c',signature=a.signature,crypto_root=a.crypto_root):subprocess.run(c,check=True)
