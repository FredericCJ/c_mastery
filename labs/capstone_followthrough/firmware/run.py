"""Build public firmware tests with explicitly supplied learner providers."""
import argparse,pathlib,subprocess
def main():
    p=argparse.ArgumentParser();p.add_argument('--cc',default='cc');p.add_argument('--source',action='append',required=True,type=pathlib.Path);p.add_argument('--include',action='append',required=True,type=pathlib.Path);p.add_argument('--build',required=True,type=pathlib.Path);a=p.parse_args();here=pathlib.Path(__file__).resolve().parent;a.build.mkdir(parents=True,exist_ok=True);exe=(a.build/'firmware-tests').resolve()
    subprocess.run([a.cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O0','-g',*['-I'+str(x.resolve()) for x in a.include],*[str(x.resolve()) for x in a.source],str(here/'checks.c'),str(here/'tests.c'),'-o',str(exe)],check=True)
    for case in range(8):subprocess.run([str(exe),str(case)],check=True)
if __name__=='__main__':main()
