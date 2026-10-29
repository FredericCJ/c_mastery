"""Fresh C17 hosted tag/reservation/frame/rollback/teardown mechanism campaign."""
import argparse,hashlib,json,os,subprocess,sys
from pathlib import Path
from c_source import find_all
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/kernel_followthrough/contracts';REF=ROOT/'assessor/kernel_followthrough/contracts'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);commands=[];checks=[];before={p.relative_to(ROOT).as_posix():sha(p) for p in [*LAB.glob('*'),*REF.glob('*'),Path(__file__),ROOT/'assessor/kernel/model.c',ROOT/'labs/kernel/include/cmk_contract.h',ROOT/'assessor/memory_followthrough/byte_support.c'] if p.is_file()}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:exitcode=86',UBSAN_OPTIONS='halt_on_error=1:exitcode=86')
 def run(label,cmd,expected=0):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,env=env,capture_output=True,text=True,timeout=60);path=work/(label+'.txt');path.write_text(r.stdout+r.stderr);commands.append({'label':label,'command':cmd,'exit_code':r.returncode,'expected_exit_code':expected,'output':path.relative_to(ROOT).as_posix()});assert r.returncode==expected,(label,r.stdout,r.stderr);return r.stdout+r.stderr
 def check(label,value):checks.append({'name':label,'passed':bool(value)});assert value,label
 includes=['-I'+str(LAB),'-I'+str(ROOT/'labs/kernel/include')];files=[LAB/'tests.c',REF/'contracts.c',ROOT/'assessor/kernel/model.c'];groups=('tags','reservations','frames','allocation','lifecycle')
 try:
  for cc in ('gcc','clang'):
   run(cc+'-version',[cc,'--version'])
   for profile,flags in [('O0',['-O0','-g']),('O2',['-O2','-g','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-no-pie'])]:
    label=cc+'-'+profile;exe=work/label;run(label+'-build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*flags,*includes,*files,'-o',exe])
    for group in groups:check(label+'-'+group,'KF_PASS:'+group in run(label+'-'+group,[exe,group]))
   for profile,extra in [('O0',['-O0','-g']),('O2',['-O2','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-no-pie'])]:
    label=cc+'-boundaries-'+profile;exe=work/label;run(label+'-build',[cc,'-std=c17','-Wall','-Wextra','-Werror',*extra,*includes,LAB/'boundaries.c',*files[1:],'-o',exe])
    for group in ('permissions','scheduling','images','archives','handles','lines'):check(label+'-'+group,'KF_PASS:'+group in run(label+'-'+group,[exe,group]))
   for group in ('permissions','scheduling','images','archives','handles','lines'):check(cc+'-boundaries-memcheck-'+group,'ERROR SUMMARY: 0 errors' in run(cc+'-boundaries-memcheck-'+group,['valgrind','--leak-check=full','--error-exitcode=89',work/(cc+'-boundaries-O0'),group]))
   for group in groups:check(cc+'-memcheck-'+group,'ERROR SUMMARY: 0 errors' in run(cc+'-memcheck-'+group,['valgrind','--leak-check=full','--error-exitcode=89',work/(cc+'-O0'),group]))
   shared=work/(cc+'.so');run(cc+'-shared',[cc,'-std=c17','-O2','-shared','-fPIC',*includes,*files[1:],'-o',shared]);check(cc+'-independent-set-oracle',json.loads(run(cc+'-oracle',[sys.executable,LAB/'oracle.py',shared]))['cases']==128)
   objects=[]
   for name,source in [('contracts',REF/'contracts.c'),('retained',ROOT/'assessor/kernel/model.c'),('bytes',ROOT/'assessor/memory_followthrough/byte_support.c')]:
    obj=work/(cc+'-'+name+'.o');run(cc+'-'+name+'-freestanding',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin',*includes,'-c',source,'-o',obj]);objects.append(obj)
   closed=work/(cc+'-closed.o');run(cc+'-close',['ld','-r',*objects,'-o',closed]);check(cc+'-no-runtime-imports',not run(cc+'-undefined',['nm','-u',closed]).strip())
   original=(REF/'contracts.c').read_text();mutations=[('optional','tags','if (type == 1) {','if (0) {'),('reserved','reservations','if (type != 1 && base < hi && lo < end)','if (0 && base < hi && lo < end)'),('rollback','allocation','(void)cmk_frame_put(&p->frames, s->pages[first + i].frame);','/* leak allocated frame */'),('handoff','lifecycle','slot == c->cr3_owner ||','')]
   for name,group,old,new in mutations:
    spans=find_all(original,old);assert len(spans)==(2 if name=='handoff' else 1),(name,len(spans));source=work/(cc+'-'+name+'.c');source.write_text(original[:spans[0][0]]+new+original[spans[0][1]:]);exe=work/(cc+'-'+name);run(cc+'-'+name+'-build',[cc,'-std=c17','-O2','-Wall','-Wextra',*includes,LAB/'tests.c',source,files[2],'-o',exe]);check(cc+'-'+name+'-killed','KF_CHECK:' in run(cc+'-'+name,[exe,group],1))
  exe=work/'stub';run('stub-build',['gcc','-std=c17','-O2',*includes,LAB/'tests.c',LAB/'contracts.c',ROOT/'assessor/kernel/model.c','-o',exe]);check('stub-incomplete','KF_CHECK:' in run('stub',[exe,'tags'],1));check('source-stable',all(sha(ROOT/p)==h for p,h in before.items()))
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 result={'passed':passed,'error':error,'commands':commands,'checks':checks,'inspected_paths':before,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Hosted C and finite set models;actual guest allocation/teardown is a separate campaign.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(commands),'checks':len(checks)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);raise SystemExit(main(p.parse_args().work))
