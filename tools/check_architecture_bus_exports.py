"""Test actual copied learner sources and included linker recipe in isolation."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_bus'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('architecture-bus-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(ident,args,exit=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==exit and (marker is None or marker in r.stdout);commands.append(dict(id=ident,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=exit,required_marker=marker,output=r.stdout,passed=passed));assert passed,(ident,r.stdout);return r.stdout
 assert sys.platform!='win32','Run this isolated export campaign inside WSL.'
 workspace=folder/'PR-T37-06';run('init',[sys.executable,'-m','cmastery','init','PR-T37-06',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T37-06.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 paths=['labs/architecture_bus','labs/peripheral_depth/dma','labs/architecture_bytes','labs/algorithm_wrappers','labs/containers_depth'];base=['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*['-I'+p for p in paths]];public='labs/architecture_bus/';sources=[public+'learner.c','labs/peripheral_depth/dma/dma.c','labs/architecture_bytes/placement.c',public+'ledger_objects.c',public+'tests.c']
 run('native.compile',['gcc',*base,'-DMB_EXAMPLE_MAP',*sources,public+'native.c','-o','bus-tests'],cwd=workspace);outputs[rel(workspace/'bus-tests')]=sha(workspace/'bus-tests')
 for case in ['topology','placement','lease','arbitration','bridge']:run('native.rejection.'+case,['./bus-tests',case],41,'MB_ASSERT:',workspace)
 image='bus-thumb.elf';run('thumb.compile',['arm-none-eabi-gcc',*base,'-DMB_EXAMPLE_MAP','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib',*sources,public+'thumb_start.c','labs/containers_depth/byte_helpers.c','-T',public+'model.ld','-Wl,--no-undefined','-o',image],cwd=workspace);outputs[rel(workspace/image)]=sha(workspace/image)
 run('thumb.rejection',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image],1,'MB_THUMB_FAIL',workspace)
 for mode in ['sram','ccm']:
  image='ledger-'+mode+'.elf';run('ledger.'+mode+'.compile',['arm-none-eabi-gcc',*base,'-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','labs/architecture_bytes/placement.c',public+'ledger_objects.c',public+'ledger_entry.c','-T',public+'ledger_'+mode+'.ld','-Wl,--no-undefined','-o',image],cwd=workspace);outputs[rel(workspace/image)]=sha(workspace/image);text=run('ledger.'+mode+'.symbols',['arm-none-eabi-nm','-S',image],cwd=workspace);assert 'ab_dma_candidate' in text and 'mb_dma_source' in text
 for p in [pathlib.Path(__file__),ROOT/'tools/close_architecture_bus.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['All compilation/link commands run inside the exported workspace with copied public files only; included B06 linker recipe resolves there.','Actual learner stubs compile and fail intentionally; no private Pd/new core implementation shipped.','Example header remains labeled example; real author tests use actual ELF-derived header.','No independent learner, elapsed delay or physical result inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual bus export thirteen commands')
if __name__=='__main__':main()
