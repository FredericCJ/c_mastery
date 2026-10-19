"""T09 original ELF experiments; scopes grow only when implemented and executed."""
import argparse,importlib.util,json
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def load(path,name):
    spec=importlib.util.spec_from_file_location(name,path);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
helper=load(ROOT/'assessor/legacy_systems/toolchain/common.py','elf_common')
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True,type=Path)
    p.add_argument('--module',action='append',choices=('decoder','linker','dynamic','layout','interfaces'),help='Focused diagnostic run; full closure requires every module')
    args=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/legacy_systems_elf'
    c=helper.Campaign(ROOT/'build/legacy_systems_elf'/stamp,base/'runs'/stamp/'report.json')
    paths=[Path(__file__).resolve(),ROOT/'assessor/legacy_systems/toolchain/common.py']
    paths += [ROOT/'research/v5/expansion/legacy_systems_elf'/name for name in ('resources.json','research_gate.json')]
    paths += [ROOT/'research/v5/expansion/legacy_systems/t09_cards.py',ROOT/'tools/check_systems_elf.py']
    paths += [p for folder in ('labs/machine_image/elf_depth','assessor/legacy_systems/elf_depth') for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    pins={p.relative_to(ROOT).as_posix():helper.sha(p) for p in paths};error=None
    try:
        for module in (args.module or ('decoder','linker','dynamic','layout','interfaces')):load(ROOT/'assessor/legacy_systems/elf_depth'/(module+'.py'),'elf_'+module).run(c,ROOT)
        if any(helper.sha(ROOT/p)!=h for p,h in pins.items()):raise RuntimeError('Inputs changed during run')
    except Exception as failure:error=str(failure)
    result=c.finish(pins,error)
    for file in c.root.rglob('*'):
        if file.is_file() and file.suffix in ('.ld','.map','.hex') and file.stat().st_size<100000:
            result['generated_inputs'][file.relative_to(c.root).as_posix()]={'sha256':helper.sha(file),'content':file.read_text(errors='replace')}
    result['selected_modules']=args.module or ['decoder','linker','dynamic','layout','interfaces']
    c.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');target=(ROOT/args.report).resolve()
    if not target.is_relative_to(base.resolve()):raise ValueError('Report outside ELF evidence scope')
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(c.output.read_bytes())
    print(json.dumps({'passed':result['passed'],'commands':len(c.commands),'observations':len(c.observations),'error':error}));return not result['passed']
if __name__=='__main__':raise SystemExit(main())
