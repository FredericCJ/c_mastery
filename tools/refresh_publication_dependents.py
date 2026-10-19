"""Refresh only model/B01/hosted metadata after the shared source guard freezes.

Original C campaigns and export observations are validated and retained. This
does not regenerate cards/catalogs, merge inventory, or run the full build.
"""
import concurrent.futures
from datetime import datetime,timezone
import json
import os
import subprocess
import sys
from common import ROOT,dump,sha


def main():
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    base=ROOT/'reviews/v5/expansion/followthrough'/('publication_dependents_'+stamp)
    base.mkdir(exist_ok=False)
    scopes={
      'model':dict(support='research/v5/expansion/corpus_model/completion_dispositions.json',
        preserve=['research/v5/expansion/corpus_model/research_gate.json','reviews/v5/expansion/corpus_model/guard.json'],
        commands=[['tools/check_corpus_model.py','--self-test','--output','reviews/v5/expansion/corpus_model/guard.json'],['tools/close_corpus_model.py']],
        guard='reviews/v5/expansion/corpus_model/guard.json'),
      'foundation':dict(support='research/v5/expansion/b01_followthrough/completion_dispositions.json',
        preserve=['research/v5/expansion/b01_followthrough/readiness.json'],
        commands=[['tools/build_foundation_followthrough_readiness.py'],['tools/check_foundation_followthrough.py','--self-test']],
        guard=(base/'foundation_guard.json').relative_to(ROOT).as_posix()),
      'hosted':dict(support='research/v5/expansion/legacy_hosted_parent_reconciliation/completion_dispositions.json',
        preserve=[],commands=[['tools/reconcile_hosted_parent_gaps.py','--self-test','--report',(base/'hosted_guard.json').relative_to(ROOT).as_posix()]],
        guard=(base/'hosted_guard.json').relative_to(ROOT).as_posix()),
    }
    preserved=[]
    for scope,spec in scopes.items():
        for name in [spec['support'],*spec['preserve']]:
            source=ROOT/name;target=base/'history'/name
            target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(source.read_bytes())
            assert sha(source)==sha(target)
            preserved.append(dict(path=name,sha256=sha(source),preserved=target.relative_to(ROOT).as_posix()))
    dump(base/'preservation.json',dict(files=preserved))
    def run(item):
        scope,spec=item;records=[]
        for index,args in enumerate(spec['commands']):
            command=([sys.executable,'-X','utf8',*args] if scope!='foundation' or os.name!='nt'
                     else ['wsl','--cd','/mnt/'+ROOT.drive[0].lower()+ROOT.as_posix()[2:],'python3',*args])
            process=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='utf8',timeout=900)
            log=base/(scope+'-'+str(index)+'.log');log.write_text(process.stdout+'\n'+process.stderr,encoding='utf8',newline='\n')
            records.append(dict(command=command,exit_code=process.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
            print(scope,index,process.returncode,flush=True)
            if process.returncode:return dict(name=scope,passed=False,executions=records)
            if scope=='foundation' and index==1:dump(ROOT/spec['guard'],json.loads(process.stdout))
        packet=json.loads((ROOT/spec['support']).read_text(encoding='utf8'))
        return dict(name=scope,passed=True,executions=records,support=spec['support'],support_sha256=sha(ROOT/spec['support']),
                    guard=spec['guard'],guard_sha256=sha(ROOT/spec['guard']),rows=len(packet['updates']))
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as pool:rows=list(pool.map(run,scopes.items()))
    result=dict(passed=all(r['passed'] for r in rows),packages=rows,new_c_execution=False,
                source_sha256={'tools/refresh_publication_dependents.py':sha(ROOT/'tools/refresh_publication_dependents.py')})
    dump(base/'result.json',result);print(base.relative_to(ROOT).as_posix(),flush=True)
    return int(not result['passed'])


if __name__=='__main__':raise SystemExit(main())
