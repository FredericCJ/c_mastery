"""Execute all ADT cases, retaining both original assertion dialects."""
import argparse,json,os,subprocess
from datetime import datetime,timezone
from pathlib import Path
from common import ROOT,sha,dump
from run_expansion_family import run
from check_adt_marker_contract import SOURCES,corrected,validate
if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        raise SystemExit(subprocess.run(python_command('tools/run_adt.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode)
    target=ROOT/args.report
    raw=target.parent/('adt_generic_'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')+'.json')
    before={s:sha(ROOT/s) for s in SOURCES}
    run('adt',str(raw))
    result=corrected(json.loads(raw.read_text()))
    result['generic_predecessor']=dict(path=raw.relative_to(ROOT).as_posix(),sha256=sha(raw))
    result['inspected_paths'].update(before)
    errors=validate(result)
    if errors:print(json.dumps(errors))
    dump(target,result)
    raise SystemExit(int(bool(errors)))
