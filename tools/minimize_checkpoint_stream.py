"""Minimize the original skip-candidate fault using an independent P3 oracle."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]

def main(report):
    oracle_path=ROOT/'labs/checkpoint_depth/tests/check_stream.py'
    spec=importlib.util.spec_from_file_location('p3_oracle',oracle_path)
    module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
    fixture=ROOT/'fixtures/legacy_checkpoints/P3.json'
    original=bytes.fromhex(json.loads(fixture.read_text(encoding='utf-8'))['input']['wire_hex'])
    sources=[Path(__file__).resolve(),oracle_path,fixture,ROOT/'labs/checkpoint_depth/include/checkpoint_depth.h',
             ROOT/'labs/checkpoint_depth/tests/tests.c',ROOT/'assessor/checkpoint_depth/checkpoint_depth.c']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    observations=[]; probes=0
    with tempfile.TemporaryDirectory(prefix='p3-minimize-') as temporary:
        executables=[]
        for mutant in (0,3):
            executable=Path(temporary)/('p3-'+str(mutant))
            command=['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-DCP_STREAM_ONLY','-DCP_MUTANT='+str(mutant),
                '-I'+str(ROOT/'labs/checkpoint_depth/include'),str(ROOT/'labs/checkpoint_depth/tests/tests.c'),
                str(ROOT/'assessor/checkpoint_depth/checkpoint_depth.c'),
                '-ffp-contract=off','-fno-fast-math','-o',str(executable)]
            result=subprocess.run(command,capture_output=True,text=True)
            if result.returncode: raise ValueError(result.stderr)
            observations.append({'command':command,'exit_code':result.returncode})
            executables.append(executable)
        def observed(executable,data,chunk):
            result=subprocess.run([str(executable),'oracle',data.hex(),str(chunk)],capture_output=True,text=True)
            if result.returncode: raise ValueError('Crash/internal failure is not the missing-frame property: '+result.stderr)
            return result.stdout.splitlines()
        def fails(data):
            nonlocal probes
            probes+=1
            expected=module.oracle(data)
            return '7:' in expected and '7:' not in observed(executables[1],data,256)
        if not fails(original): raise ValueError('Original seed does not expose the intended missing sequence7')
        reduced=original
        # Repeated single-byte deletion establishes a local, not globally shortest, witness.
        changed=True
        while changed:
            changed=False
            for index in range(len(reduced)):
                candidate=reduced[:index]+reduced[index+1:]
                if fails(candidate): reduced=candidate;changed=True;break
        minimal=not any(fails(reduced[:i]+reduced[i+1:]) for i in range(len(reduced)))
        expected=module.oracle(reduced)
        comparisons=[]
        for chunk in (1,7,256):
            good=observed(executables[0],reduced,chunk)
            bad=observed(executables[1],reduced,chunk)
            comparisons.append({'chunk':chunk,'expected':expected,'repaired':good,'faulty':bad,
                'passed':good==expected and '7:' not in bad})
    stable=all(hashlib.sha256((ROOT/name).read_bytes()).hexdigest()==value for name,value in pins.items())
    result={'schema_version':1,'criterion':'AX-PR-P3-02-T32.06.c','passed':stable and minimal and all(x['passed'] for x in comparisons),
        'inspected_paths':pins,'source_stable':stable,'original_hex':original.hex(),'minimized_hex':reduced.hex(),
        'original_bytes':len(original),'minimized_bytes':len(reduced),'deletion_probes':probes,
        'single_byte_deletion_minimal':minimal,'builds':observations,'comparisons':comparisons,
        'scope':'Original local intentionally faulty parser; missing synthetic sequence7 only. One-byte-deletion minimality, not globally shortest witness.',
        'learner_mastery':'not-assessed','independent_review':'not-performed'}
    path=(ROOT/report).resolve()
    if not path.is_relative_to(ROOT/'reviews/v5/expansion/legacy_completion'): raise ValueError('Use a new completion report')
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({k:result[k] for k in ('passed','original_bytes','minimized_bytes','deletion_probes')}))
    return 0 if result['passed'] else 1

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
