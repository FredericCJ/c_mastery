"""T10 scoped producer: add observations only for actually implemented experiments."""
import argparse,importlib.util,json
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def load(path,name):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
helper=load(ROOT/'assessor/legacy_systems/toolchain/common.py','startup_common')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True,type=Path);p.add_argument('--module',action='append',choices=('core','entry','runtime'));p.add_argument('--tool-prefix',type=Path,default=Path('/home/frede/cmastery-startup-tools'));a=p.parse_args()
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/legacy_systems_startup';c=helper.Campaign(ROOT/'build/legacy_systems_startup'/stamp,base/'runs'/stamp/'report.json')
 paths=[Path(__file__).resolve(),ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'research/v5/expansion/legacy_systems/t10_cards.py',ROOT/'tools/check_systems_startup.py',ROOT/'research/v5/expansion/legacy_systems/provision_startup.py']
 paths += [ROOT/'research/v5/expansion/legacy_systems_startup'/name for name in ('resources.json','research_gate.json','provisioning.json')]
 paths += [p for folder in ('labs/machine_image/startup_depth','assessor/legacy_systems/startup_depth') for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 pins={p.relative_to(ROOT).as_posix():helper.sha(p) for p in paths};error=None
 try:
  from expansion_evidence import require_batch_research
  require_batch_research(ROOT/'research/v5/expansion/legacy_systems_startup')
  c.group='toolchain-identity'
  for tool in ('gcc','clang','ld','readelf','objdump','gdb','arm-none-eabi-gcc'):c.run([tool,'--version'])
  c.run(['ldd','--version']);c.run(['dpkg-query','-W','libc6','libc6-dev']);c.run(['uname','-a'])
  for module in a.module or ('core','entry','runtime'):load(ROOT/'assessor/legacy_systems/startup_depth'/(module+'.py'),'startup_'+module).run(c,ROOT,a)
  if any(helper.sha(ROOT/p)!=h for p,h in pins.items()):raise RuntimeError('Inputs changed during run')
 except Exception as e:error=str(e)
 result=c.finish(pins,error);result['selected_modules']=a.module or ['core','entry','runtime']
 # Preserve exact generated debugger commands, link origins and resource data.
 for path in c.root.rglob('*'):
  if path.is_file() and path.suffix in ('.gdb','.map','.su','.trace','.json') and path.stat().st_size<8000000:
   result['generated_inputs'][path.relative_to(c.root).as_posix()]={'sha256':helper.sha(path),'content':path.read_text(errors='replace')}
 result['tool_provisioning']=json.loads((a.tool_prefix/'manifest.json').read_text())
 c.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
 target=(ROOT/a.report).resolve()
 if not target.is_relative_to(base.resolve()):raise ValueError('Report outside startup scope')
 target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(c.output.read_bytes());print(json.dumps({'passed':result['passed'],'commands':len(c.commands),'observations':len(c.observations),'error':error}));return not result['passed']
if __name__=='__main__':raise SystemExit(main())
