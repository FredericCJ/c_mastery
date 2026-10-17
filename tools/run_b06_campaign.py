"""Execute B06 cores, semantic mutants and learner-stub rejection on Linux.

All generated binaries live in a temporary directory. This author campaign is
not learner acceptance, kernel/board execution, or independent review.
"""
from pathlib import Path
import argparse,datetime,hashlib,json,os,platform,subprocess,tempfile
ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--output',type=Path,default=ROOT/'research/v5/expansion/b06/core_validation.json');parser.add_argument('--family');args=parser.parse_args()
    records=[];sources={};profiles=[('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O2']),('clang-ubsan','clang',['-O1','-g','-fsanitize=undefined','-fno-sanitize-recover=all'])]
    def run(command,kind,expected=0,marker=None):
        result=subprocess.run([str(x) for x in command],cwd=ROOT,text=True,capture_output=True,timeout=120)
        passed=result.returncode==expected and (marker is None or marker in result.stderr)
        records.append(dict(kind=kind,command=[str(x) for x in command],returncode=result.returncode,stdout=result.stdout,stderr=result.stderr,passed=passed,expected_returncode=expected,required_stderr=marker))
        return passed
    families=[json.loads(p.read_text(encoding='utf-8'))['domain']['family'] for p in sorted((ROOT/'curriculum/v5/domains').glob('T*.json')) if 57<=int(p.stem[1:])<=64]
    if args.family:families=[family for family in families if family==args.family]
    with tempfile.TemporaryDirectory(prefix='cmastery-b06-') as td:
        work=Path(td)
        for label,compiler,flags in profiles:
            run([compiler,'--version'],'compiler-version')
            for family in families:
                lab=ROOT/'labs'/family;reference=ROOT/'assessor'/family/'core.c';harness=lab/'tests/tests.c'
                data=json.loads((lab/'family.json').read_text(encoding='utf-8'));slugs=[t.split('.')[-1] for t in data['tests']]
                for path in [reference,harness,lab/'include/core.h',lab/'src/core.c',lab/'family.json']:
                    sources[path.relative_to(ROOT).as_posix()]=sha(path)
                base=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic',*flags,'-I',lab/'include']
                executable=work/(label+'-'+family)
                if run([*base,reference,harness,'-lm','-o',executable],label+':reference-build'):
                    for slug in slugs:run([executable,slug],label+':'+family+'.'+slug)
                for n,slug in enumerate(slugs,1):
                    mutant=work/(label+'-'+family+'-bad')
                    if run([*base,'-DB06_MUTATION='+str(n),reference,harness,'-lm','-o',mutant],label+':mutant-build'):
                        run([mutant,slug],label+':'+family+'.mutation.'+slug,1,'B06_CHECK')
                if label=='gcc-debug':
                    stub=work/(family+'-learner-stub')
                    if run([*base,lab/'src/core.c',harness,'-lm','-o',stub],'learner-stub-build'):
                        for slug in slugs:run([stub,slug],'learner-stub-rejection:'+family+'.'+slug,1,'B06_CHECK')
    sources['tools/run_b06_campaign.py']=sha(Path(__file__))
    report=dict(passed=bool(families) and all(r['passed'] for r in records),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),platform=platform.platform(),profiles=[p[0] for p in profiles],families=families,input_sha256=sources,checks=records,limits=['Author reference implementation campaign, not learner acceptance','No independent human reviewer','No guest kernel, physical board, exhaustive C semantics or performance claim','Target adapters and solver/fuzz campaigns have separate evidence'])
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':report['passed'],'checks':len(records),'failures':[dict(kind=r['kind'],stderr=r['stderr'][-1200:],returncode=r['returncode']) for r in records if not r['passed']],'output':str(args.output)},indent=2))
    return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
