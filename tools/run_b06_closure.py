"""Source-bound cross-target atomic width/ISA/optimization dependency experiments."""
from pathlib import Path
import subprocess,json,hashlib,datetime,os,re
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/architecture_ports/targets';src=lab/'closure_probe.c';build=ROOT/'build/b06-closure';build.mkdir(parents=True,exist_ok=True);private=Path.home()/'cmastery-b06-tools';lld=private/'usr/bin/ld.lld-21';rows=[];observations=[];artifacts={};env=dict(os.environ);env['LD_LIBRARY_PATH']=str(private/'usr/lib/x86_64-linux-gnu')+':'+env.get('LD_LIBRARY_PATH','')
 def run(label,cmd,expected=0):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60,env=env);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=(p.returncode==0 if expected==0 else p.returncode!=0)));return p
 run('compiler identity',['clang','--version']);run('linker identity',[lld,'--version'])
 for arch,target,flags,prefix in [('rv64i','riscv64-none-elf',['-march=rv64i','-mabi=lp64'],'riscv64'),('rv64ima','riscv64-none-elf',['-march=rv64ima','-mabi=lp64'],'riscv64'),('armv8','aarch64-none-elf',['-march=armv8-a','-mno-outline-atomics'],'aarch64'),('armv8-lse','aarch64-none-elf',['-march=armv8.1-a+lse','-mno-outline-atomics'],'aarch64')]:
  dump=private/('usr/bin/'+prefix+'-linux-gnu-objdump');run(arch+' disassembler identity',[dump,'--version'])
  for opt in ['-O0','-O2','-Os']:
   for width in [32,64,128]:
    name=arch+opt[1:]+'w'+str(width);obj=build/(name+'.o');asm=build/(name+'.s');exe=build/(name+'.elf');common=['clang','--target='+target,*flags,'-std=c17',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-DATOMIC_WIDTH='+str(width)]
    if run(name+' compile',common+['-fstack-usage','-c',src,'-o',obj]).returncode:continue
    p=run(name+' undefined helpers',['nm','-u',obj]);helpers=sorted(re.findall(r' U (\S+)',p.stdout));expected_helpers=(['__atomic_fetch_add_'+str(width//8),'__udivdi3'] if arch=='rv64i' else ['__atomic_fetch_add_16'] if arch=='rv64ima' and width==128 else []);rows.append(dict(label=name+' independently predicted helper set',passed=helpers==sorted(expected_helpers),expected=expected_helpers,observed=helpers));run(name+' compiler assembly',common+['-S',src,'-o',asm]);p=run(name+' object disassembly',[dump,'-dr',obj]);dis=build/(name+'.dis');dis.write_text(p.stdout);p=run(name+' section byte sizes',['size','-A',obj]);sizes=p.stdout;su=obj.with_suffix('.su');stack=su.read_text();rows.append(dict(label=name+' stack report',stdout=stack,passed='closure_step' in stack and '_start' in stack))
    # Deliberately supply no compiler runtime: a missing helper must prevent closure.
    p=run(name+' no-runtime final link',[lld,'-e','_start','-Ttext=0x400000',obj,'-Map='+str(build/(name+'.map')),'-o',exe],expected=1 if helpers else 0)
    rows.append(dict(label=name+' precise unresolved dependency control',passed=(all(h in p.stderr for h in helpers) if helpers else not p.stderr.strip()),required_helpers=helpers))
    if not helpers:
     p=run(name+' final undefined references',['nm','-u',exe]);rows.append(dict(label=name+' final helper closure',passed=not p.stdout.strip()))
    observations.append(dict(configuration=name,target=target,flags=common[2:],atomic_width=width,helpers=helpers,static_frames=stack,section_sizes=sizes,closed=not helpers,disassembly=dis.relative_to(ROOT).as_posix()))
    for f in [obj,asm,dis,su]:artifacts[f.relative_to(ROOT).as_posix()]=hashlib.sha256(f.read_bytes()).hexdigest()
 files=[Path(__file__),src,lab/'CLOSURE.md',ROOT/'research/v5/expansion/b06/architecture_codegen_review.md'];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},generated_artifact_sha256=artifacts,checks=rows,observations=observations,limitations=['Object and final-link evidence, not reset execution of these probe images; actual privileged execution is in the separate ABI guest receipt','Static frames exclude runtime helpers and asynchronous exception frames; unresolved helpers deliberately have no invented implementations','128-bit integer spelling is a compiler extension; absence of helper calls is not a lock-free progress or whole-program concurrency proof','Compiler release, target ISA and flags pin this matrix; no cross-compiler universal code-size claim']);(ROOT/'research/v5/expansion/b06/closure_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
