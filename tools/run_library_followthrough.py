"""Scoped standard-library author experiments; selected modules are never whole T12."""
from datetime import datetime,timezone
from pathlib import Path
import argparse,importlib.util,json
ROOT=Path(__file__).resolve().parents[1]
def load(path,name):
 spec=importlib.util.spec_from_file_location(name,path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
helper=load(ROOT/'assessor/legacy_systems/toolchain/common.py','library_common')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--module',action='append',required=True,choices=['format','unicode','bytes','allocation','conversion','headers','scanner','fileio','mathlab','sorting','picolib','processlab','control']);p.add_argument('--report',type=Path,required=True);a=p.parse_args();stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/legacy_library';c=helper.Campaign(ROOT/'build/legacy_library'/stamp,base/'runs'/stamp/'report.json');error=None
 paths=[Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'assessor/foundations/foundation.c',ROOT/'labs/foundations/include/foundation.h']+[f for folder in ('labs/foundations/library_depth','assessor/legacy_library','research/v5/expansion/legacy_library') for f in (ROOT/folder).rglob('*') if f.is_file() and '__pycache__' not in f.parts and f.name!='support.json'];pins={f.relative_to(ROOT).as_posix():helper.sha(f) for f in paths}
 try:
  c.group='environment'
  for argv in (['gcc','--version'],['clang','--version'],['ldd','--version']):c.run(argv)
  for name in a.module:load(ROOT/'assessor/legacy_library'/(name+'.py'),'library_'+name).run(c,ROOT,a)
  if any(helper.sha(ROOT/p)!=h for p,h in pins.items()):raise RuntimeError('Sources changed during execution')
 except Exception as e:error=str(e)
 report=c.finish(pins,error);report['selected_modules']=a.module;report['topic_complete']=False;report['retained_artifacts']={f.relative_to(ROOT).as_posix():dict(sha256=helper.sha(f),bytes=f.stat().st_size) for f in c.root.rglob('*') if f.is_file()};out=(ROOT/a.report).resolve()
 if not out.is_relative_to(base.resolve()):raise ValueError('Report outside library evidence scope')
 c.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(c.output.read_bytes());print(dict(passed=report['passed'],commands=len(c.commands),observations=len(c.observations),error=error));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
