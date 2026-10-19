"""Export-safe explicit source selection; default means learner stubs."""
import argparse,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3];LAB=Path(__file__).resolve().parent
def main():
 p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--kind',choices=['core','linux'],default='core');p.add_argument('--server-source',type=Path,default=LAB/'server_template.c');p.add_argument('--flow-source',type=Path,default=LAB/'flow.c');p.add_argument('--parser-source',type=Path,default=ROOT/'labs/communications/learner/http.c');p.add_argument('--depth-source',type=Path,default=ROOT/'labs/http_depth/src/http_depth.c');p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.parent.mkdir(parents=True,exist_ok=True)
 cmd=[a.cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-pedantic','-I'+str(LAB),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/http_depth/include'),str(a.flow_source),str(a.parser_source),str(a.depth_source)]
 if a.kind=='linux':cmd+=['-DHF_SERVER_SOURCE="'+str(a.server_source.resolve())+'"',str(LAB/'linux_tests.c')]
 else:cmd+=[str(LAB/'tests.c')]
 return subprocess.run(cmd+['-o',str(a.output)],check=False).returncode
if __name__=='__main__':raise SystemExit(main())
