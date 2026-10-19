"""Actual T08 typed ABI, hostile caller and unwind experiments (additive scope)."""
import argparse,hashlib,importlib.util,json,sys
from datetime import datetime,timezone
from pathlib import Path
from expansion_evidence import require_batch_research
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('abi_campaign_common',ROOT/'assessor/legacy_systems/toolchain/common.py')
helper=importlib.util.module_from_spec(spec);spec.loader.exec_module(helper)
Campaign,sha=helper.Campaign,helper.sha
PUBLIC=ROOT/'labs/machine_image/abi_depth';PRIVATE=ROOT/'assessor/legacy_systems/abi_depth'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True,type=Path);p.add_argument('--clang-linker',required=True,type=Path);args=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    base=ROOT/'reviews/v5/expansion/legacy_systems_abi';receipt=base/'runs'/stamp/'report.json'
    require_batch_research(ROOT/'research/v5/expansion/legacy_systems_abi')
    paths=[Path(__file__).resolve(),ROOT/'tools/check_systems_abi.py',ROOT/'research/v5/expansion/legacy_systems_abi/resources.json',ROOT/'research/v5/expansion/legacy_systems_abi/research_gate.json',ROOT/'assessor/legacy_systems/toolchain/common.py',ROOT/'assessor/machine_image/abi.c',ROOT/'assessor/machine_image/sum.S',ROOT/'labs/machine_image/include/machine.h']
    paths += [p for folder in (PUBLIC,PRIVATE) for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    pins={p.relative_to(ROOT).as_posix():sha(p) for p in paths}
    c=Campaign(ROOT/'build/legacy_systems_abi'/stamp,receipt);error=None
    try:
        flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/machine_image/include')]
        common=[PUBLIC/'tests.c',PUBLIC/'hostile.S',ROOT/'assessor/machine_image/abi.c',ROOT/'assessor/machine_image/sum.S']
        for cc in ('gcc','clang'):
            c.run([cc,'--version'])
            for profile,opts in [('debug',['-O0','-g']),('release',['-O2','-g','-fomit-frame-pointer']),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
                c.group=cc+'-'+profile;exe=c.root/c.group
                c.run([cc,*flags,*opts,*common,PRIVATE/'api.c',PRIVATE/'boundary.S','-o',exe])
                for case in ('values','aggregate','hostile','call8','inline','cfi'):c.run([exe,case],contains='AD_PASS')
                c.run(['objdump','-dr',exe]);c.run(['readelf','-s','-l',exe]);c.run(['readelf','--debug-dump=frames',exe])
                if profile=='release':
                    c.run(['gdb','-q','-batch','-ex','set debuginfod enabled off','-ex','set disable-randomization off','-ex','break ad_trace_leaf','-ex','run cfi','-ex','bt',exe],contains='ad_cfi',env={'DEBUGINFOD_URLS':''})
        c.group='semantic-controls'
        for macro,case,marker in [('AD_MUTANT_PRESERVED','hostile','callee preserved sentinels'),('AD_MUTANT_ALIGNMENT','call8','precall versus entry alignment'),('AD_MUTANT_EARLY','inline','early clobber additive contract')]:
            exe=c.root/macro;c.run(['gcc',*flags,'-O2','-D'+macro,*common,PRIVATE/'api.c',PRIVATE/'boundary.S','-o',exe]);c.run([exe,case],expected=1,contains='AD_CHECK:'+marker)
        exe=c.root/'learner-stub';c.run(['gcc',*flags,*common,PUBLIC/'api.c',PUBLIC/'boundary.S','-o',exe])
        for case,marker in [('values','zero extending sum'),('aggregate','integer pair return'),('hostile','nested call alignment'),('call8','eight argument sum'),('inline','early clobber additive contract'),('cfi','normal call and early return paths')]:
            c.run([exe,case],expected=1,contains='AD_CHECK:'+marker)
        c.group='public-explicit-imports'
        imports=[sys.executable,PUBLIC/'run_core.py','--sum-source',ROOT/'assessor/machine_image/sum.S','--mix-source',ROOT/'assessor/machine_image/abi.c','--machine-include',ROOT/'labs/machine_image/include']
        c.run([*imports,'--api-source',PRIVATE/'api.c','--boundary-source',PRIVATE/'boundary.S'],contains='AD_PASS')
        c.run(imports,expected='nonzero',contains='AD_CHECK:zero extending sum')
        c.group='unwind-metadata'
        for name,flags2 in [('correct',[]),('wrong',['-DAD_BAD_CFI'])]:
            obj=c.root/(name+'.o');c.run(['gcc','-g',*flags2,'-c',PRIVATE/'boundary.S','-o',obj]);frames=c.run(['readelf','--debug-dump=frames',obj])
            c.require(('DW_CFA_def_cfa_offset: 48' in frames)==(name=='correct'),'CFA matches actual push plus32-byte frame',name)
        for suffix,ordinary,variant in [
          ('02.b','Six independently varied callee-preserved register sentinels; arithmetic-correct corruption rejected','Actual nested call clobbers scratch argument registers; input lifetime preserved'),
          ('03.a','Eight arguments recorded before/after call and weighted sum204','Ninth integer plus separate double argument class executes'),
          ('03.b','Scalar, integer pair and hidden-result triple execute through compatible prototypes','Mixed integer/SSE return executes with distinct register classes'),
          ('07.b','Actual saved-RBX/32-byte frame CFI and debugger backtrace','No frame pointer; early-return state restored; wrong metadata has CFA40 rather than48'),
          ('08.a','Two-instruction read-write plus early-clobber constraints execute O0/O2 and sanitizer profiles','Equal-value loop plus explicit invalid alias allocation demonstrates missing-constraint counterexample'),
          ('08.b','Named +m and opaque-pointer memory-clobber both execute with ownership contract','Precise named-memory alternative retains explicit cc effect; broad clobber is not synchronization')]:
            c.observe('T08.'+suffix,ordinary,['core']);c.observe('T08.'+suffix,variant,['core'],True)
        for module in ('representations','frames','arm','control'):
            module_spec=importlib.util.spec_from_file_location('abi_'+module,PRIVATE/(module+'.py'))
            implementation=importlib.util.module_from_spec(module_spec);module_spec.loader.exec_module(implementation)
            if module=='frames':implementation.run(c,ROOT,args.clang_linker)
            else:implementation.run(c,ROOT)
        expected={(f'T08.{topic:02}.{leaf}',v) for topic in range(1,11) for leaf in 'abc' for v in (False,True)}
        c.require(len(c.observations)==60 and {(r['objective'],r['variant']) for r in c.observations}==expected,'all30 exact objectives and30 transfers observed',len(c.observations))
        if any(sha(ROOT/p)!=value for p,value in pins.items()):raise RuntimeError('Source changed during execution')
    except Exception as failure:error=str(failure)
    result=c.finish(pins,error);target=(ROOT/args.report).resolve()
    if not target.is_relative_to(base.resolve()):raise ValueError('Use legacy_systems_abi receipt directory')
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(receipt.read_bytes())
    print(json.dumps({'passed':result['passed'],'commands':len(result['commands']),'error':error}));return not result['passed']
if __name__=='__main__':raise SystemExit(main())
