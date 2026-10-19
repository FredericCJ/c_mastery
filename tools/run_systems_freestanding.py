"""Scoped T11 producer; incomplete modules never establish whole-topic evidence."""
import argparse,importlib.util,json
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
MODULES=('core','lowering','services','instrumentation','library','closure','targets')
def load(path,name):
 s=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
helper=load(ROOT/'assessor/legacy_systems/toolchain/common.py','free_common')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True,type=Path);p.add_argument('--module',action='append',choices=('core','lowering','services','instrumentation','library','closure','targets'));a=p.parse_args();stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/legacy_systems_freestanding';c=helper.Campaign(ROOT/'build/legacy_systems_freestanding'/stamp,base/'runs'/stamp/'report.json')
 paths=[Path(__file__).resolve(),ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'research/v5/expansion/legacy_systems/t11_cards.py',ROOT/'assessor/machine_image/wide.c',ROOT/'assessor/machine_image/audit.c',ROOT/'labs/machine_image/include/machine.h',ROOT/'assessor/legacy_systems/startup_depth/decimal.c',ROOT/'labs/machine_image/startup_depth/startup.h']+[p for folder in ('labs/machine_image/freestanding_depth','assessor/legacy_systems/freestanding_depth') for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]+[ROOT/'research/v5/expansion/legacy_systems_freestanding'/name for name in ('resources.json','research_gate.json','provisioning.json','corrections.json')];pins={p.relative_to(ROOT).as_posix():helper.sha(p) for p in paths};error=None
 try:
  c.group='environment'
  for command in ('gcc','clang','arm-none-eabi-gcc','qemu-system-arm','ld','nm','objdump'):c.run([command,'--version'])
  for name in a.module or MODULES:load(ROOT/'assessor/legacy_systems/freestanding_depth'/(name+'.py'),'free_'+name).run(c,ROOT,a)
  if set(a.module or MODULES)==set(MODULES):load(ROOT/'assessor/legacy_systems/freestanding_depth/manifests.py','free_manifests').run(c,ROOT,a)
  if any(helper.sha(ROOT/p)!=h for p,h in pins.items()):raise RuntimeError('Inputs changed during run')
 except Exception as e:error=str(e)
 result=c.finish(pins,error);result['selected_modules']=a.module or list(MODULES)
 result['retained_artifacts']={p.relative_to(c.root).as_posix():dict(sha256=helper.sha(p),bytes=p.stat().st_size) for p in c.root.rglob('*') if p.is_file() and (p.suffix in ('.o','.a','.map','.su','.gcda','.json') or p.name.endswith('-gmon.out') or p.read_bytes()[:4]==b'\x7fELF')}
 c.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');out=(ROOT/a.report).resolve()
 if not out.is_relative_to(base.resolve()):raise ValueError('Report outside T11 scope')
 out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(c.output.read_bytes());print(json.dumps(dict(passed=result['passed'],commands=len(c.commands),observations=len(c.observations),error=error)));return not result['passed']
if __name__=='__main__':raise SystemExit(main())
