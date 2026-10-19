"""Source-bound T19 criterion-specific successor experiments."""
from datetime import datetime,timezone
from pathlib import Path
import argparse,importlib.util,json
ROOT=Path(__file__).resolve().parents[1]
def load(path,name):
 spec=importlib.util.spec_from_file_location(name,path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
helper=load(ROOT/'assessor/legacy_systems/toolchain/common.py','abstractions_common')
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--module',action='append',required=True,choices=['session','registry','generic','reducer','traversal','handles','dispatch']);p.add_argument('--report',required=True);a=p.parse_args();stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/legacy_abstractions';c=helper.Campaign(ROOT/'build/legacy_abstractions'/stamp,base/'runs'/stamp/'report.json');error=None
 files=[Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'assessor/api_depth/api_depth.c',ROOT/'labs/api_depth/include/api_depth.h',ROOT/'assessor/foundations/foundation.c',ROOT/'labs/foundations/include/foundation.h',ROOT/'assessor/api_depth/contracts/ownership.c',ROOT/'assessor/api_depth/stream.c',ROOT/'labs/api_depth/stream/stream.h',ROOT/'labs/api_depth/stream/CONTRACTS.md']+[f for folder in ('labs/api_depth/abstractions','assessor/api_depth/abstractions','labs/api_depth/contracts','research/v5/expansion/legacy_abstractions') for f in (ROOT/folder).rglob('*') if f.is_file() and '__pycache__' not in f.parts and f.name!='support.json'];pins={f.relative_to(ROOT).as_posix():helper.sha(f) for f in files}
 try:
  c.group='environment'
  for argv in (['gcc','--version'],['clang','--version']):c.run(argv)
  for name in a.module:load(ROOT/'assessor/api_depth/abstractions'/(name+'.py'),'abstractions_'+name).run(c,ROOT,a)
  if any(helper.sha(ROOT/f)!=h for f,h in pins.items()):raise RuntimeError('Source changed during campaign')
 except Exception as e:error=str(e)
 report=c.finish(pins,error);report['selected_modules']=a.module;report['topic_complete']=False;report['retained_artifacts']={f.relative_to(ROOT).as_posix():dict(sha256=helper.sha(f),bytes=f.stat().st_size) for f in c.root.rglob('*') if f.is_file()};out=(ROOT/a.report).resolve()
 if not out.is_relative_to(base.resolve()):raise ValueError('Output outside abstractions evidence')
 c.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');out.parent.mkdir(parents=True,exist_ok=True);out.write_bytes(c.output.read_bytes());print(dict(passed=report['passed'],commands=len(c.commands),observations=len(c.observations),error=error));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
