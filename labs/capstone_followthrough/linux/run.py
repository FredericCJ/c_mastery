"""Build public capstone tests with explicitly named learner providers only."""
import argparse,pathlib,subprocess
def main():
    p=argparse.ArgumentParser();p.add_argument('--mode',choices=['core','linux','modern','composition'],default='core');p.add_argument('--cc',default='cc');p.add_argument('--source',action='append',required=True,type=pathlib.Path);p.add_argument('--include',action='append',required=True,type=pathlib.Path);p.add_argument('--build',required=True,type=pathlib.Path);a=p.parse_args();here=pathlib.Path(__file__).resolve().parent
    tests={'core':(here.parent/'core/tests.c',['framing','progress','pressure','terminal']),'linux':(here/'linux_tests.c',['fairness','duplicate','service','file']),'modern':(here/'modern_tests.c',['result','slabs','receipts','cleanup']),'composition':(here/'composition.c',['p5','coroutine'])};source,cases=tests[a.mode];a.build.mkdir(parents=True,exist_ok=True);exe=a.build/a.mode
    args=[a.cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O0','-g','-pthread','-ffunction-sections','-fdata-sections',*['-I'+str(x.resolve()) for x in a.include],*[str(x.resolve()) for x in a.source],str(source),'-Wl,--gc-sections','-o',str(exe.resolve())];subprocess.run(args,check=True)
    for case in cases:subprocess.run([str(exe.resolve()),case],check=True)
if __name__=='__main__':main()
