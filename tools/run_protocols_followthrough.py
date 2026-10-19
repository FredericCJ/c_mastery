"""Required protocol variants; actual producer composition, controls and scoped evidence."""
from pathlib import Path
import argparse,subprocess,json,hashlib,datetime,os,shutil
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--rustc',default=shutil.which('rustc') or str(Path.home()/'cmastery-b06-tools/usr/bin/rustc-1.93'));ap.add_argument('--newlib-headers',type=Path,default=Path.home()/'cmastery-b06-tools/usr/include/newlib');a=ap.parse_args()
 lab=ROOT/'labs/protocols/followthrough';ref=ROOT/'assessor/protocols/followthrough';build=ROOT/'build/protocols-followthrough';build.mkdir(parents=True,exist_ok=True);rows=[]
 includes=[ROOT/'labs'/f/'include' for f in ['protocols','firmware_io','execution','adt','algorithms']]+[lab]
 imports=[ROOT/'assessor'/f/name for f,name in [('protocols','protocols.c'),('firmware_io','firmware_io.c'),('execution','cmx.c'),('adt','adt.c'),('algorithms','algo.c')]]
 env=os.environ.copy();private=Path.home()/'cmastery-b06-tools/usr';env['LD_LIBRARY_PATH']=str(private/'lib/x86_64-linux-gnu')+(':'+env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
 def run(label,cmd,negative=None,timeout=90):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout,env=env);out=p.stdout+p.stderr;ok=p.returncode==0 if negative is None else p.returncode!=0 and negative in out;row=dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:row=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(row);print(label,row['passed'],flush=True);return row
 profiles=[('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0','-g'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  for stem,tests,mutants,macro in [('protocol_follow',['driver','builder','session','decoder'],{1:'driver',5:'driver',2:'builder',6:'builder',3:'session',7:'session',4:'decoder',8:'decoder'},'PF_MUTANT'),('event_follow',['hsm','active','reactive'],{1:'hsm',4:'hsm',2:'active',5:'active',3:'reactive',6:'reactive'},'PE_MUTANT'),('system_follow',['system'],{1:'system',2:'system'},'PS_MUTANT')]:
   for m in [0,*mutants,'stub']:
    src=(lab if m=='stub' else ref)/(stem+'.c');test=lab/('tests.c' if stem=='protocol_follow' else 'event_tests.c' if stem=='event_follow' else 'system_tests.c');used_imports=imports if stem=='system_follow' else imports[:2] if stem=='protocol_follow' else imports[:1];sources=[src,*used_imports,test]
    if stem=='system_follow':sources.append(ref/'protocol_follow.c')
    exe=build/(tag+'-'+stem+'-'+str(m));cmd=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-Wno-misleading-indentation','-pthread',*flags,*['-I'+str(p) for p in includes],*sources,'-o',exe]
    if m!='stub':cmd.append('-D'+macro+'='+str(m))
    if run(tag+' '+stem+' build '+str(m),cmd)['passed']:
     for case in tests if m in [0,'stub'] else [mutants[m]]:
      run(tag+' '+case+' execute '+str(m),[exe,*([] if case=='system' else [case])],None if m==0 else ('PS_CHECK:' if case=='system' else 'PE_CHECK:' if stem=='event_follow' else 'PF_CHECK:'))
 # The actual competing-consumer adapter is checked by both supported race detectors.
 for cc in ['gcc','clang']:
  exe=build/(cc+'-thread');cmd=[cc,'-std=c17','-O1','-g','-pthread','-fsanitize=thread','-Wno-misleading-indentation',*['-I'+str(p) for p in includes],ref/'event_follow.c',imports[0],lab/'event_tests.c','-o',exe]
  if run(cc+' TSan build',cmd)['passed']:run(cc+' TSan competing consumers',[exe,'active'])
 for cc in ['gcc','clang']:
  exe=build/(cc+'-runtime-memory');r=run(cc+' declared byte runtime build',[cc,'-std=c17','-O3','-fno-builtin','-Wall','-Wextra','-Werror',lab/'runtime_mem.c',lab/'runtime_mem_test.c','-o',exe])
  if r['passed']:run(cc+' declared byte runtime boundaries',[exe])
  base=[cc,'-std=c17','-Wall','-Wextra','-Werror',*['-I'+str(p) for p in includes],'-c'];run(cc+' copied wrapper accepted',[*base,lab/'copy_token.c','-o',build/(cc+'-copy.o')]);run(cc+' incompatible wrapper diagnostic',[*base,lab/'wrong_wrapper.c','-o',build/(cc+'-wrong.o')],negative='incompatible')
 run('Rust identity',[a.rustc,'--version']);exe=build/'rust-move';positive=run('Rust owning move compile',[a.rustc,'--edition=2021',lab/'move_positive.rs','-o',exe]);
 if positive['passed']:run('Rust owning move execute',[exe])
 run('Rust moved owner diagnostic',[a.rustc,'--edition=2021',lab/'move_negative.rs','-o',build/'rust-negative'],negative='E0382')
 run('Arm compiler identity',['arm-none-eabi-gcc','--version'])
 for opt in ['-O0','-O2']:
  elf=build/('cortex-m4'+opt+'.elf');cmd=['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb',opt,'-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-nostdlib','-idirafter',a.newlib_headers,*['-I'+str(p) for p in includes],ref/'system_follow.c',ref/'protocol_follow.c',ref/'event_follow.c',lab/'irq_port.c',lab/'runtime_mem.c',*imports,ROOT/'labs/protocols/targets/runtime.c','-Wl,--gc-sections,-e,pf_system_init,-u,pf_irq_lock,-u,pf_active_step','-o',elf]
  if run('CortexM4 '+opt+' declared-runtime link',cmd)['passed']:
   r=run('CortexM4 '+opt+' undefined symbols',['arm-none-eabi-nm','-u',elf]);rows.append(dict(label='CortexM4 '+opt+' helper closure',passed=r['passed'] and not r['stdout'].strip()));r=run('CortexM4 '+opt+' disassembly',['arm-none-eabi-objdump','-d',elf]);rows.append(dict(label='CortexM4 '+opt+' saved-mask port',passed=r['passed'] and all(x in r['stdout'].lower() for x in ['primask','cpsid','dmb'])));run('CortexM4 '+opt+' code and data size',['arm-none-eabi-size',elf])
 files=[Path(__file__),*lab.glob('*'),*ref.glob('*'),*imports,ROOT/'labs/protocols/targets/runtime.c',*[p for inc in includes[:-1] for p in inc.glob('*.h')]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files if p.is_file()},checks=rows,limitations=['Source-bound author variants and actual producer references, not independently implemented learner imports','ThreadSanitizer observes only executed pthread schedules; CortexM4 IRQ port is compiled, not executed on hardware','Session crash/reconnect preserves modeled durable deduplication state; no distributed exactly-once or real persistence claim','Original delayed variants remain independent assessments; author solutions do not award learner mastery','Physical cache/DMA/reset qualification remains separate'])
 out=ROOT/'research/v5/modern/protocols_followthrough_validation.json';out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
