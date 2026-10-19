"""Execute P4 reference variants and cross-build (never flash) the capture adapter."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,struct,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/checkpoint_firmware_followthrough';REF=ROOT/'assessor/checkpoint_firmware_followthrough';BASE=ROOT/'labs/checkpoint_depth';OLD=ROOT/'assessor/checkpoint_depth'
CORE=OLD/'checkpoint_firmware.c';PARSER=OLD/'checkpoint_depth.c';HALF=OLD/'transfer/halfduplex.c';RUNTIME=ROOT/'assessor/execution/support.c'
REPORT='reviews/v5/expansion/legacy_checkpoint_firmware/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('p4_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():
 files={Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',CORE,PARSER,HALF,RUNTIME,OLD/'targets/f439.c',ROOT/'tools/run_checkpoint_firmware.py',ROOT/'tools/verify_checkpoint_timing.py',ROOT/'fixtures/legacy_checkpoints/transfer/P4_timing.json',BASE/'tests/firmware.c',BASE/'src/checkpoint_firmware.c',BASE/'transfer/halfduplex_tests.c'}
 for base in [LAB,REF,BASE/'include',BASE/'transfer']:
  files.update(p for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts and (base in [LAB,REF] or p.suffix=='.h'))
 return sorted(files)
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/checkpoint-firmware-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;deps={};board=[];old_report=None
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  c.run(['uname','-a'])
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(BASE/'include'),'-I'+str(BASE/'transfer')]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   for name,sources,marker in [('ordinary',[BASE/'tests/firmware.c',REF/'ordinary.c'],'P4_ORDINARY_PASS'),('variants',[REF/'variants.c'],'P4_VARIANTS_PASS histories=13122'),('halfduplex',[HALF,BASE/'transfer/halfduplex_tests.c'],'CPX_PASS TXE/TC distinction')]:
    c.group=tag+':'+name;exe=w/(tag+'-'+name);c.run([cc,*flags,CORE,PARSER,*sources,'-o',exe]);text=c.run([exe],contains=marker,env=env);c.require('runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,'defined successful reference')
   for v in [33,34,35,36,37,38,'stub']:
    c.group=tag+':core-'+str(v);exe=w/(tag+'-core-'+str(v));src=BASE/'src/checkpoint_firmware.c' if v=='stub' else CORE;c.run([cc,*flags,'-DCP_MUTANT='+str(0 if v=='stub' else v),src,PARSER,BASE/'tests/firmware.c',REF/'ordinary.c','-o',exe]);c.run([exe],expected=1,contains='CP_CHECK:',env=env)
   for v in [11,12]:
    c.group=tag+':half-'+str(v);exe=w/(tag+'-half-'+str(v));c.run([cc,*flags,'-DCPX_MUTANT='+str(v),CORE,PARSER,HALF,BASE/'transfer/halfduplex_tests.c','-o',exe]);c.run([exe],expected=1,contains='CPX_CHECK:',env=env)
  for name,cmd in [('capture',[sys.executable,LAB/'check_capture.py','--self-test']),('decoder',[sys.executable,REF/'capture_tests.py',LAB]),('timing',[sys.executable,REF/'timing.py']),('retained-timing',[sys.executable,ROOT/'tools/verify_checkpoint_timing.py'])]:
   c.group=name;text=c.run(cmd,contains='"passed": true',cwd=ROOT);d=json.loads(text);c.write(name+'.json',json.dumps(d,indent=2)+'\n');c.require(d['passed'],'independent '+name)
  for cc in ['gcc','clang']:
   for bad in [0,1]:
    c.group=cc+':register-'+str(bad);exe=w/(cc+'-register-'+str(bad));c.run([cc,'-std=c17','-O2','-Wall','-Wextra','-Werror','-DP4_BAD_STATUS_MASK='+str(bad),REF/'register_tests.c','-o',exe]);c.run([exe],expected=bad,contains='P4_STATUS_REJECT reserved/TC bits' if bad else 'P4_STATUS_PASS states=1024 reserved_zero')
  def build(cc,opt,mode,trace,ring,offset,ccm=False):
   name=f'{cc}-O{opt}-mode{mode}-trace{trace}-ring{ring}-off{offset:x}'+('-ccm' if ccm else '');c.group='board:'+name;dest=w/name
   cmd=[sys.executable,LAB/'target_build.py','--build',dest,'--driver',REF/'f439_capture.c','--core',CORE,'--parser',PARSER,'--halfduplex',HALF,'--runtime',RUNTIME,'--core-include',BASE/'include','--transfer-include',BASE/'transfer','--cc','arm-none-eabi-gcc' if cc=='gcc' else cc,'--opt',str(opt),'--mode',str(mode),'--trace',str(trace),'--ring',str(ring),'--offset',hex(offset)]+(['--dma-ccm'] if ccm else [])
   c.run(cmd,contains='P4_CAPTURE_BUILD_PASS explicit_objects=6 no_archives execution=not-run');text=c.run([sys.executable,REF/'audit_target.py',dest],expected=int(ccm),contains='"passed": '+('false' if ccm else 'true'));audit=json.loads(text);c.require(audit['dma_reachable']!=ccm and audit['reset_vector_layout'] and not audit['unresolved'],'actual board layout/closure');deps.update(json.loads((dest/'build.json').read_text())['input_sha256']);board.append(dict(name=name,selected=json.loads((dest/'build.json').read_text())['selected'],audit=audit));return dest
  first=None
  for cc in ['gcc','clang']:
   for opt in [0,3]:
    for trace in [0,1]:
     dest=build(cc,opt,1,trace,4,0)
     if first is None:first=dest
   build(cc,3,0,1,2,0x4000);build(cc,3,2,1,4,0x4000)
  build('gcc',3,1,1,4,0,True)
  c.group='bad-vector';bad=w/'bad-vector';shutil.copytree(first,bad);p=bad/'image.elf';data=bytearray(p.read_bytes());h=struct.unpack_from('<16sHHIIIIIHHHHHH',data);sections=[struct.unpack_from('<10I',data,h[6]+i*h[11]) for i in range(h[12])];names=sections[h[13]];strings=data[names[4]:names[4]+names[5]];v=next(s for s in sections if strings[s[0]:].split(b'\0',1)[0]==b'.vectors');struct.pack_into('<I',data,v[4]+22*4,0);p.write_bytes(data);text=c.run([sys.executable,REF/'audit_target.py',bad],expected=1,contains='"passed": false');c.require(not json.loads(text)['reset_vector_layout'],'actual wrong IRQ vector rejected')
  c.group='emulated-reset';old_report='reviews/v5/expansion/legacy_completion/p4_reset_'+w.name+'.json';c.run([sys.executable,ROOT/'tools/run_checkpoint_firmware.py','--report',old_report],contains='"passed": true',cwd=ROOT,timeout=240);d=json.loads((ROOT/old_report).read_text());c.require(d['passed'] and len(d['profiles'])==10 and len(d['negative_controls'])==13,'actual poisoned reset, relocation and negative guests');c.write('reset-report.json',json.dumps(d,indent=2)+'\n')
  c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(board_images=board,reset_report=old_report,dependency_sha256=deps,external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(native_profiles=6,core_mutants=6,half_mutants=2,fifo_histories=13122,failed_abort_attempts=100,board_images=13,valid_board_images=12,capture_negatives=20,decoder_negatives=5),limits=d['limits']+['F439 capture adapter only cross-built/audited; no board run, electrical timing, trace, or physical fault injection.','MPS2 actual poisoned reset/PendSV runs are a different peripheral environment, not F439 DMA emulation.','Native variants include exact declared failed-ack/counter-cut models and two separate SPSC providers, not a new MPSC algorithm.','Unit-step/arrival-ledger synthetic predictions are not physical latency or WCET.','Strict selected images contain six explicit original objects, no runtime archives; host verification dependencies are separate.','Capture checker verifies consistency/identity, not authenticity.','External review, independent learner and seven-day assessment remain absent.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
