"""Run mailbox cases with explicit completed CMX, receipt and mailbox imports."""
import argparse
from pathlib import Path
import subprocess
import tempfile
HERE=Path(__file__).resolve().parent


def command(compiler,flags,execution,execution_include,receipts,receipt_include,extension,output,mutant=0):
    return [compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-I'+str(HERE),'-I'+str(execution_include),'-I'+str(receipt_include),
            str(execution),str(receipts),str(extension),str(HERE/'mailbox_tests.c'),'-DCDMB_MUTANT='+str(mutant),'-o',str(output)]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('execution-source','execution-include','receipt-source','receipt-include','mailbox-source'):
        parser.add_argument('--'+name,required=True,type=Path)
    parser.add_argument('--compiler',default='cc')
    parser.add_argument('--case',required=True,choices=('transport','generations','clients','unknown','receipts','restart'))
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='mailbox-depth-') as directory:
        binary=Path(directory)/'mailbox'
        build=subprocess.run(command(args.compiler,['-O0','-g'],args.execution_source,args.execution_include,args.receipt_source,args.receipt_include,args.mailbox_source,binary))
        if build.returncode:return build.returncode
        return subprocess.run([str(binary),args.case]).returncode


if __name__=='__main__':raise SystemExit(main())
