"""Run independently derived fixture oracles and record their actual outcomes."""
import json
from pathlib import Path
import subprocess
import sys
from common import ROOT, dump, load

def required_oracles(root=ROOT):
    names={'verify_language_fixtures.py','verify_systems_fixtures.py','verify_embedded_fixtures.py','verify_integration_fixtures.py'}
    if (root/'curriculum/enhancement_taxonomy.json').exists():
        names.update({'verify_algorithms_fixtures.py','verify_communications_fixtures.py','verify_http_fixtures.py','verify_networking_fixtures.py'})
    if (root/'curriculum/performance_debugging_taxonomy.json').exists():
        names.update({'verify_performance_fixtures.py','verify_debugging_fixtures.py','verify_memory_lab_fixtures.py'})
    if (root/'curriculum/v2_taxonomy.json').exists():
        names.update({'verify_runtime_fixtures.py','verify_storage_fixtures.py','verify_kernel_fixtures.py'})
    return names

def check_oracle_presence(scripts,root=ROOT):
    missing=required_oracles(root)-{p.name for p in scripts}
    if missing: raise ValueError('Missing required fixture oracles: '+str(sorted(missing)))

def main():
    # A failed catalog/presence check must not leave an earlier success current.
    dump(ROOT/'reviews/fixture_checks.json',{'passed':False,'stage':'started','checks':[]})
    scripts=sorted((ROOT/'tools').glob('verify_*_fixtures.py'))
    check_oracle_presence(scripts)
    _,_,projects,_=load()
    referenced={fixture for p in projects.values() for fixture in p['meta']['fixtures']}
    actual={p.relative_to(ROOT).as_posix() for p in (ROOT/'fixtures').rglob('*') if p.is_file()}
    if referenced!=actual:
        raise ValueError('Fixture catalog mismatch: missing='+str(sorted(referenced-actual))+' unreferenced='+str(sorted(actual-referenced)))
    records=[]
    for path in scripts:
        result=subprocess.run([sys.executable,'-X','utf8',str(path)],capture_output=True,text=True,encoding='utf-8',cwd=ROOT)
        record={'script':path.name,'exit_code':result.returncode,'output':result.stdout.strip(),'errors':result.stderr.strip()}
        records.append(record)
        print(path.name, result.returncode, result.stdout.strip())
    passed=all(r['exit_code']==0 for r in records)
    dump(ROOT/'reviews/fixture_checks.json',{'passed':passed,'project_contracts':len(projects),'unique_referenced_fixtures':len(referenced),'file_correspondence':True,'checks':records})
    if not passed: raise SystemExit(1)

if __name__=='__main__': main()
