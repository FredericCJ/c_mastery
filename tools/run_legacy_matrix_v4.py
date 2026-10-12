"""Revalidate the nine retained families without rewriting v3 release evidence."""
from __future__ import annotations
import os
import subprocess
import sys
from common import ROOT, dump, read_json, sha
from v2_common import source_identity

FAMILIES=['foundations','algorithms','communications','runtime','storage','kernel','os_observatory','execution','kernel_smp']


def main(families=None, receipt='reviews/v4/legacy_matrix.json'):
    families=list(families or FAMILIES)
    folder=ROOT/'reviews/v3'
    if folder.resolve()!=ROOT.resolve()/'reviews/v3':raise ValueError('Redirected historical receipt directory')
    historic={p:p.read_bytes() for p in (ROOT/'reviews/v3').rglob('*') if p.is_file()}
    original_paths=set(historic)
    identity=source_identity()
    report_path=ROOT/receipt
    dump(report_path,{'schema_version':4,'status':'pending','passed':False,'input_sha256':identity['sha256']})
    suffix='lab_matrix_'+'_'.join(families)+'.json'
    expected=[folder/suffix]+[folder/('lab_'+family+'.json') for family in families]
    try:
        # An interrupted runner must never relabel a historical success as new.
        for path in expected:
            if not path.resolve().is_relative_to(folder.resolve()):raise ValueError('Escaping receipt path')
            if path.is_file():path.unlink()
        command=[sys.executable,str(ROOT/'tools/run_lab_matrix.py')]
        for family in families:command+=['--family',family]
        run=subprocess.run(command,cwd=ROOT)
        if run.returncode:raise ValueError('Retained-family runner exited '+str(run.returncode))
        if not all(path.is_file() for path in expected):raise ValueError('Retained-family runner omitted new receipts')
        matrix=read_json(ROOT/'reviews/v3'/suffix)
        details={f:read_json(ROOT/'reviews/v3'/('lab_'+f+'.json')) for f in families}
        if not matrix.get('passed') or not all(d.get('passed') for d in details.values()):raise ValueError('Retained-family execution failed')
        if source_identity()['sha256']!=identity['sha256'] or matrix.get('input_sha256')!=identity['sha256']:
            raise ValueError('Source changed during retained-family execution')
        inspected={}
        for family in families:
            for directory in [ROOT/'labs'/family,ROOT/'assessor'/family]:
                for p in directory.rglob('*'):
                    if p.is_file() and not {'build','__pycache__'}&set(p.relative_to(ROOT).parts):inspected[p.relative_to(ROOT).as_posix()]=sha(p)
        for p in [ROOT/'cmastery/cli.py',ROOT/'cmastery/learning.py',ROOT/'tools/run_lab_matrix.py',ROOT/'tools/run_legacy_matrix_v4.py']:inspected[p.relative_to(ROOT).as_posix()]=sha(p)
        dump(report_path,dict(matrix,schema_version=4,status='passed',details=details,inspected_paths=inspected))
    except BaseException as error:
        dump(report_path,{'schema_version':4,'status':'failed','passed':False,'input_sha256':identity['sha256'],'error':type(error).__name__+': '+str(error)})
        raise
    finally:
        for path,content in historic.items():path.write_bytes(content)
        for created in expected:
            if created not in original_paths and created.is_file():created.unlink()


if __name__=='__main__':main()
