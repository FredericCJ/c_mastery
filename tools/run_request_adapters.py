"""Author-reference request model and actual disposable virtio guest campaign."""
import argparse,hashlib,json,os,subprocess,sys
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/b05_depth/request_adapters';REFERENCE=ROOT/'assessor/b05_depth/request_adapters'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',type=Path);a=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    folder=ROOT/'build/b05_depth/request-campaign'/stamp;folder.mkdir(parents=True)
    evidence=ROOT/'reviews/v5/expansion/followthrough/request_adapters'/stamp;evidence.mkdir(parents=True)
    paths=[p for base in [PUBLIC,REFERENCE] for p in base.glob('*') if p.is_file()]+[Path(__file__),ROOT/'tools/check_request_adapters.py']
    report={'schema_version':1,'passed':False,'input_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in paths},'commands':[],'profiles':[],'guests':[],
      'limits':['Author reference artifacts are not learner work.','Physical device and independent reviewer/learner evidence remain separate.']}
    def run(cmd,expected=0,marker=None):
        argv=list(map(str,cmd));r=subprocess.run(argv,cwd=ROOT,text=True,capture_output=True,timeout=180,env=dict(os.environ,ASAN_OPTIONS='detect_stack_use_after_return=1'))
        ok=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr)
        report['commands'].append({'command':argv,'exit':r.returncode,'expected':expected,'marker':marker,'passed':ok,'stdout':r.stdout,'stderr':r.stderr,'environment':{'ASAN_OPTIONS':'detect_stack_use_after_return=1'}})
        if not ok:raise RuntimeError(r.stdout+r.stderr)
        return r.stdout
    try:
        flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(PUBLIC)]
        for cc in ['gcc','clang']:
            run([cc,'--version'])
            for mode,opts in [('debug',['-O0','-g']),('release',['-O2']),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all'])]:
                exe=folder/(cc+'-'+mode);run([cc,*flags,*opts,PUBLIC/'tests.c',REFERENCE/'request.c','-o',exe]);run([exe],marker='RQ_OK:196 transitions;')
                report['profiles'].append({'compiler':cc,'profile':mode,'passed':True,'transitions':196})
        for macro in ['RQ_MUTANT_CANCEL','RQ_MUTANT_RESET']:
            exe=folder/macro;run(['gcc',*flags,'-O2','-D'+macro,PUBLIC/'tests.c',REFERENCE/'request.c','-o',exe]);run([exe],1,'RQ_CHECK:transition table')
        exe=folder/'stub';run(['gcc',*flags,PUBLIC/'tests.c',PUBLIC/'request.c','-o',exe]);run([exe],1,'RQ_CHECK:admit transition')
        # Inspect actual cross-generated object, with no runtime library or GC.
        run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb',*flags,'-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-c',REFERENCE/'request.c','-o',folder/'request.o'])
        if run(['arm-none-eabi-nm','-u',folder/'request.o']).strip():raise RuntimeError('Implicit target helper')
        run(['arm-none-eabi-objdump','-dr',folder/'request.o']);report['model_target']={'target':'Cortex-M4','execution':'not-run','undefined_symbols':[],'object_sha256':sha(folder/'request.o')}
        def guest(ident,cc='gcc',opt='-O2',cpus='2',control='positive',stub=False,scenario='normal'):
            target=evidence/(ident+'.json')
            command=[sys.executable,PUBLIC/'guest.py','--kernel-source','assessor/kernel_smp','--kernel-lab','labs/kernel_smp',
              '--adapter-source',PUBLIC if stub else REFERENCE,'--build-root',folder/'guest','--compiler',cc,'--optimization='+opt,'--cpus',cpus,'--control',control,'--scenario',scenario,'--report',target]
            run(command,1 if stub else 0)
            data=json.loads(target.read_text());q=[r for r in data['commands'] if r['command'][0]=='qemu-system-x86_64' and '-machine' in r['command']]
            if stub and (len(q)!=1 or q[0]['exit']!=35 or 'RQA_CHECK:hold observer' not in q[0]['stdout']):raise RuntimeError('Wrong guest scaffold rejection')
            report['guests'].append({'id':ident,'path':target.relative_to(ROOT).as_posix(),'sha256':sha(target)})
        for cc in ['gcc','clang']:
            for opt in ['-O0','-O2']:
                for cpus in ['2','4']:guest(cc+opt+'-'+cpus,cc,opt,cpus)
        for control in ['RQ_MUTANT_CANCEL','RQ_MUTANT_RESET','RQA_MUTANT_ACK']:guest(control,control=control)
        for cc in ['gcc','clang']:guest(cc+'-device-error',cc=cc,scenario='device-error')
        guest('RQA_MUTANT_STATUS',control='RQA_MUTANT_STATUS',scenario='device-error')
        guest('learner-stub',stub=True)
        if any(sha(ROOT/p)!=h for p,h in report['input_sha256'].items()):raise RuntimeError('Inputs changed')
        report['passed']=True
        from check_request_adapters import validate
        errors=validate(report)
        if errors:raise RuntimeError(str(errors))
    except Exception as e:report.update(passed=False,error=str(e))
    target=a.report or evidence/'campaign.json';target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'passed':report['passed'],'report':str(target),'error':report.get('error')}));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
