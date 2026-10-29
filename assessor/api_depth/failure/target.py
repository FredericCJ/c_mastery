import re,hashlib
from pathlib import Path
def run(c,root,args):
 c.group='failure-target';pub=root/'labs/api_depth/failure';private=root/'assessor/api_depth/failure';headers=['taxonomy','construction','transaction','cancellation','observability','diagnostics','crash','public_error'];names=[]
 for h in headers:
  names+=re.findall(r'\b(?:enum ae_code|enum ae_outcome|struct ae_lookup|struct ae_update|void|unsigned)\s+(ae_\w+)\s*\(', (pub/(h+'.h')).read_text())
 names=sorted(set(names));c.require(len(names)>=40,'all named bounded failure-core API roots selected',names)
 entry=c.write('failure-core-entry.c',''.join('#include "'+h+'.h"\n' for h in headers)+'#include <limits.h>\n_Static_assert(CHAR_BIT==8 && UINT_MAX>=65536u,"selected octet/unsigned profile");\ntypedef void (*Root)(void);\nstatic Root volatile roots[]={'+','.join('(Root)'+name for name in names)+'};\nvolatile Root observed;void _start(void){for(unsigned i=0;i<sizeof roots/sizeof roots[0];++i)observed=roots[i];for(;;){}}\n')
 sources=[private/(m+'.c') for m in ['taxonomy','error_abi','construction','transaction','cancellation','observability','diagnostics','crash','target_helpers']]+[root/p for p in ['assessor/api_depth/abstractions/generic.c','assessor/api_depth/contracts/ownership.c','assessor/foundations/foundation.c','assessor/legacy_systems/freestanding_depth/bytes.c','assessor/legacy_systems/freestanding_depth/memory_provider.c']]+[entry]
 common=['-std=c17','-Wall','-Wextra','-Werror','-DNDEBUG','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections',*['-I'+str(root/p) for p in ['labs/api_depth/failure','labs/api_depth/abstractions','labs/api_depth/contracts','labs/api_depth/include','labs/foundations/include','labs/machine_image/freestanding_depth']]]
 for cc,opt,target in [('arm-none-eabi-gcc','-O0','m4'),('arm-none-eabi-gcc','-O2','m4'),('clang','-O2','x86_64')]:
  folder=c.root/(target+opt);folder.mkdir();objects=[];targetflags=['-mcpu=cortex-m4','-mthumb','-isystem','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib'] if target=='m4' else []
  c.run([cc,'--version'])
  for index,source in enumerate(sources):
   obj=folder/(str(index)+'-'+source.stem+'.o');c.run([cc,*common,*targetflags,opt,'-fstack-usage','-MD','-MF',str(obj)+'.d','-c',source,'-o',obj]);objects.append(obj);c.run([('arm-none-eabi-' if target=='m4' else '')+'nm','-u',obj])
  image=folder/'failure-core.elf';c.run([cc,*targetflags,'-nostdlib','-Wl,--gc-sections,-e,_start','-Wl,-Map='+str(folder/'failure-core.map'),*objects,'-o',image]);prefix='arm-none-eabi-' if target=='m4' else '';imports=c.run([prefix+'nm','-u',image]);c.require(not imports.strip(),'actual failure-core final closure empty '+target+opt,imports);symbols=c.run([prefix+'nm','--defined-only',image]);c.require(all(re.search(r'\b'+name+r'$',symbols,re.M) for name in names),'all selected roots survive linked image '+target+opt,len(names));c.run([prefix+'readelf','-d',image]);c.run([prefix+'objdump','-dr',image]);sizes=c.run([prefix+'size','-A',image]);stack={p.name:p.read_text() for p in folder.glob('*.su')};dependencies={}
  for depfile in folder.glob('*.d'):
   for token in depfile.read_text().replace('\\\n',' ').split(':',1)[1].split():
    p=Path(token)
    if p.is_file():dependencies[str(p)]=hashlib.sha256(p.read_bytes()).hexdigest()
  c.require(bool(dependencies),'actual compiler header dependencies '+target+opt,dependencies)
  c.require(len(stack)==len(sources),'actual failure-core stack records '+target+opt,dict(files=stack,sections=sizes))
