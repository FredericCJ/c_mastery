"""Run actual T07 reference build experiments, preserving failed attempts."""
import argparse, hashlib, json, sys
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
PRIVATE=ROOT/'assessor/legacy_systems/toolchain'
sys.path.insert(0,str(PRIVATE))
from common import Campaign,sha

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True,type=Path)
    parser.add_argument('--part',choices=['stages','all'],default='all')
    parser.add_argument('--clang-linker',default='ld.lld')
    parser.add_argument('--llvm-nm',default='llvm-nm')
    args=parser.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    base=ROOT/'reviews/v5/expansion/legacy_systems_toolchain'
    receipt=base/'runs'/stamp/'report.json'
    public=ROOT/'labs/foundations/toolchain'
    paths=[Path(__file__).resolve(),ROOT/'tools/check_systems_toolchain.py',
        *[p for folder in (PRIVATE,public) for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
    pins={p.relative_to(ROOT).as_posix():sha(p) for p in paths}
    campaign=Campaign(ROOT/'build/legacy_systems_toolchain'/stamp,receipt)
    campaign.clang_linker=args.clang_linker
    campaign.llvm_nm=args.llvm_nm
    error=None
    try:
        campaign.group='tool-identities'
        for tool in ['gcc','clang','make','cmake','ninja','ar','ld','arm-none-eabi-gcc',args.clang_linker,args.llvm_nm]:
            campaign.run([tool,'--version'])
        campaign.run(['make','-C',public/'seed'],expected='nonzero',contains='LEARNER_BUILD_REQUIRED')
        import stages
        stages.execute(campaign)
        if args.part=='all':
            import graphs,packages,provenance
            for module in (graphs,packages,provenance):module.execute(campaign)
        if any(sha(ROOT/p)!=value for p,value in pins.items()):raise RuntimeError('Source changed during execution')
    except Exception as failure:error=str(failure)
    result=campaign.finish(pins,error)
    target=(ROOT/args.report).resolve()
    if not target.is_relative_to(base.resolve()):raise ValueError('Report must be in legacy_systems_toolchain evidence directory')
    target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(receipt.read_bytes())
    print(json.dumps({'passed':result['passed'],'commands':len(result['commands']),'observations':len(result['observations']),'error':error}))
    return not result['passed']
if __name__=='__main__':raise SystemExit(main())
