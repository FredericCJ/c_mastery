"""Capture owned QEMU firmware bytes using an explicitly supplied completed CMK.

Instrumentation is applied to a fresh private copy only. No assessor source is
selected implicitly. The captured root path and the hosted checked parser are
different observations; this script stops at the MADT stage, before AP startup.
"""
import argparse,hashlib,importlib.util,json,os,re,shutil
from pathlib import Path

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def prepare(lab,source,work,run_observer=None):
 work.mkdir(parents=True,exist_ok=False);copied=work/'source';shutil.copytree(source,copied)
 path=copied/'smp.c';text=path.read_text(encoding='utf-8')
 hooks={
 'static void discover(unsigned info){':'''static void sa_bytes(const char *tag,uint64_t address,const unsigned char *p,unsigned n){
 text(tag);number((unsigned)(address>>32));text(":");number((unsigned)address);text(":");number(n);text(":");
 const char hex[]="0123456789abcdef";for(unsigned i=0;i<n;i++){char c[3]={hex[p[i]>>4],hex[p[i]&15],0};text(c);}text("\\n");
}
static void discover(unsigned info){''',
 '  const unsigned char *r=table(root);unsigned z=rd32(r+4);':'''  sa_bytes("SA_RSDP:",0,rsdp,stride==4?20:36);
  const unsigned char *r=table(root);unsigned z=rd32(r+4);
  sa_bytes("SA_REGION:",root,r,z);
  for(unsigned sa_at=36;sa_at<z;sa_at+=stride){uint64_t sa_address=stride==4?rd32(r+sa_at):rd64(r+sa_at);const unsigned char *sa_t=table(sa_address);sa_bytes("SA_REGION:",sa_address,sa_t,rd32(sa_t+4));}'''}
 for old,new in hooks.items():
  if text.count(old)!=1:raise ValueError('Completed CMK discovery hook mismatch; inspect changed source before instrumentation')
  text=text.replace(old,new)
 path.write_text(text,encoding='utf-8',newline='\n')
 spec=importlib.util.spec_from_file_location('sa_cmk_guest',lab/'tools/guest.py');guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
 if run_observer is not None:guest.run=run_observer
 # Prevent unrelated ambient cross-family imports from changing this closure.
 for key in tuple(os.environ):
  if key.startswith(('CMASTERY_STORAGE_','CMASTERY_RUNTIME_')):os.environ.pop(key)
 guest.build(lab,copied,work/'guest')
 return guest,{'source_root':str(source),'instrumented_source_sha256':sha(path),'original_source_sha256':sha(source/'smp.c'),'scope':'Only serial capture hooks added to a private copy; original discovery/parser unchanged.'}

def decode(log):
 rsdp=[];regions=[]
 for kind,hi,lo,length,hexbytes in re.findall(r'SA_(RSDP|REGION):(\d+):(\d+):(\d+):([0-9a-f]+)',log):
  data=bytes.fromhex(hexbytes);address=(int(hi)<<32)|int(lo)
  if len(data)!=int(length):raise ValueError('Truncated captured table')
  if kind=='RSDP':rsdp.append(data.hex())
  else:regions.append({'address':address,'bytes':data.hex()})
 if len(rsdp)!=1 or not regions or 'PASS smp-madt' not in log or 'FAIL' in log:raise ValueError('Missing actual discovery capture')
 return {'rsdp':rsdp[0],'regions':regions}

def capture(guest,work,machine,cpus):
 if machine not in ('pc-i440fx-10.2','pc-q35-10.2') or cpus not in (1,2,4):raise ValueError('Use the bounded declared machine/CPU matrix')
 guest.MACHINE=machine;os.environ['CMASTERY_CPUS']=str(cpus);guest.stage(work/'guest','madt')
 log=(work/'guest/stage-madt.log').read_text(encoding='utf-8');record=decode(log)
 record.update(machine=machine,cpus=cpus,stage=json.loads((work/'guest/stage-madt.json').read_text(encoding='utf-8')))
 return record,log

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--lab',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--work',type=Path,required=True);a=p.parse_args()
 guest,info=prepare(a.lab.resolve(),a.source.resolve(),a.work.resolve());rows=[]
 for machine in ('pc-i440fx-10.2','pc-q35-10.2'):
  for cpus in (1,2,4):
   row,log=capture(guest,a.work.resolve(),machine,cpus);stem=machine+'-'+str(cpus);(a.work/(stem+'.log')).write_text(log,encoding='utf-8',newline='\n');rows.append(row)
 (a.work/'captures.json').write_text(json.dumps({'instrumentation':info,'captures':rows},indent=2)+'\n',encoding='utf-8',newline='\n')
