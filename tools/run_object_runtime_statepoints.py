"""Execute the explicit LLVM/x86-64 statepoint adapter; no system installation."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import resource
import shutil
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--llvm-bin', type=Path, default=Path.home()/'cmastery-modern-tools/usr/lib/llvm-21/bin')
    parser.add_argument('--report', type=Path, default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_statepoints.json')
    args = parser.parse_args()
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    build = ROOT/'build/modern_completion'/('statepoint-'+stamp)
    logs = ROOT/'reviews/v5/expansion/modern_completion/runs'/('statepoint-'+stamp)
    build.mkdir(parents=True); logs.mkdir(parents=True)
    lab = ROOT/'labs/object_runtime/statepoints'
    paths = sorted(p.relative_to(ROOT).as_posix() for p in lab.iterdir() if p.is_file())
    paths += ['assessor/object_runtime/statepoints/runtime.c', 'tools/run_object_runtime_statepoints.py']
    digest = lambda p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    pins = {p: digest(p) for p in paths}; executions = []; observations = []; passed = False

    def run(command, label, expected=0, diagnostic=None):
        start = time.monotonic()
        result = subprocess.run(list(map(str, command)), cwd=build, capture_output=True, text=True,
                                timeout=60, preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
        output = result.stdout+result.stderr
        path = logs/(f'{len(executions):03}-{label}.log'); path.write_text(output, encoding='utf8')
        executions.append({'command': list(map(str, command)), 'cwd': str(build), 'exit_code': result.returncode,
                           'seconds': time.monotonic()-start, 'stdout': output,
                           'log': path.relative_to(ROOT).as_posix(), 'log_sha256': digest(path.relative_to(ROOT))})
        assert (result.returncode == expected if expected is not None else result.returncode != 0), output[-2000:]
        if diagnostic:
            assert diagnostic in output, output[-2000:]
        return output

    try:
        opt = args.llvm_bin/'opt'; llc = args.llvm_bin/'llc'; readobj = args.llvm_bin/'llvm-readobj'
        for tool in [opt, llc, readobj, 'clang', 'gcc']:
            run([tool, '--version'], Path(tool).name+'-version')
        for mode in ['default', 'no-rematerialization']:
            directory = build/mode; directory.mkdir()
            ir = directory/'rewritten.ll'; obj = directory/'compiled.o'
            command = [opt, '-S', '-passes=rewrite-statepoints-for-gc']
            if mode != 'default': command += ['-spp-rematerialization-threshold=0']
            run(command+[lab/'source.ll', '-o', ir], mode+'-rewrite')
            text = ir.read_text(encoding='utf8')
            assert 'llvm.experimental.gc.statepoint' in text and 'llvm.experimental.gc.relocate' in text
            run([opt, '-passes=verify<safepoint-ir>', '-disable-output', ir], mode+'-verify')
            run([llc, '-O2', '-filetype=obj', ir, '-o', obj], mode+'-lower')
            run([readobj, '--stackmap', obj], mode+'-stackmap')
            run(['objdump', '-dr', obj], mode+'-assembly')
            # This control preserves ordinary IR typing while illegally retaining
            # old managed base uses after the collecting call.
            stale = directory/'stale.ll'
            mutated = text.replace('ptr addrspace(1) %base.relocated,', 'ptr addrspace(1) %base,')
            mutated = mutated.replace('load i8, ptr addrspace(1) %derived.relocated,',
                                      'load i8, ptr addrspace(1) %derived,')
            assert mutated != text
            stale.write_text(mutated, encoding='utf8')
            run([opt, '-passes=verify<safepoint-ir>', '-disable-output', stale], mode+'-stale-verifier',
                expected=None, diagnostic='Illegal use of unrelocated value found!')
            records = []
            for compiler in ['gcc', 'clang']:
                for optimization in ['-O0', '-O2', '-O3']:
                    exe = directory/(compiler+optimization)
                    common = [compiler, '-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror', optimization,
                              '-no-pie', '-I'+str(lab), lab/'driver.c', lab/'poll.S', obj,
                              '-Wl,-T,'+str(lab/'maps.ld')]
                    run(common+[ROOT/'assessor/object_runtime/statepoints/runtime.c', '-o', exe], exe.name+'-compile')
                    output = run([exe], exe.name+'-correct', diagnostic='STATEPOINT_OK polls=8')
                    for mutant in ['1', '2']:
                        run([exe, mutant], exe.name+'-mutant'+mutant, expected=1,
                            diagnostic='result==scalar+27+offset')
                    run(common+[lab/'runtime.c', '-o', str(exe)+'-learner'], exe.name+'-stub-compile')
                    run([str(exe)+'-learner'], exe.name+'-stub-reject', expected=1,
                        diagnostic='learner relocation is unimplemented')
                    records.append({'compiler': compiler, 'optimization': optimization, 'result': output.strip()})
            artifacts = []
            for path in [ir, stale, obj]:
                target = logs/(mode+'-'+path.name); shutil.copyfile(path, target)
                artifacts.append({'path': target.relative_to(ROOT).as_posix(), 'sha256': digest(target.relative_to(ROOT))})
            observations.append({'rewrite': mode, 'executions': records, 'artifacts': artifacts})
        passed = all(digest(p) == v for p, v in pins.items())
    except Exception as error:
        failure = str(error)
    report = {'schema_version': 1, 'status': 'passed' if passed else 'failed', 'passed': passed,
              'inspected_paths': pins, 'scope_source_stable': all(digest(p) == v for p, v in pins.items()),
              'observations': observations, 'executions': executions,
              'limits': ['Executed local x86-64 System V ABI adapter; LLVM nonintegral managed-pointer and statepoint semantics, not portable ISO C stack access.',
                         'Two rewrite layouts, six C compiler/optimization profiles each, eight offsets, two value mutants and compiled learner rejection per profile.',
                         'Only emitted one-function/one-call-site indirect-stack-location subset accepted. No asynchronous collector, general stack walker, physical-target or independent learner assessment claim.',
                         'Child-only core dump limit for intentionally invalid IR verifier; no host security configuration changes.']}
    if not passed: report['error'] = locals().get('failure', 'source changed during run')
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2)+'\n', encoding='utf8')
    print(json.dumps({'passed': passed, 'report': str(args.report)}))
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
