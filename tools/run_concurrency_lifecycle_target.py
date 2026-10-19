"""Execute the same serialized core in isolated Cortex-M4 QEMU, without libc."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def run(report):
    base = ROOT / 'reviews/v5/expansion/legacy_concurrency_lifecycle'
    destination = (ROOT / report).resolve()
    if not destination.is_relative_to(base.resolve()):
        raise ValueError('Use concurrency lifecycle evidence directory')
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')
    logs = base / 'target_runs' / stamp
    logs.mkdir(parents=True)
    public = ROOT / 'labs/concurrency_lifecycle_depth'
    private = ROOT / 'assessor/concurrency_lifecycle_depth'
    sources = [Path(__file__).resolve(), public/'include/cl.h', public/'src/core.c',
               public/'targets/m4.c', public/'targets/m4.ld', private/'core.c']
    pins = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    result = {'schema_version': 1, 'created_utc': stamp, 'inspected_paths': pins,
              'commands': [], 'outcomes': [],
              'scope': 'Serialized core actually executed on QEMU mps2-an386 Cortex-M4. No CMX scheduler/IRQ adapter, physical board, timing or learner assessment claim.'}

    def command(argv, label, expected=0, marker=None):
        argv = list(map(str, argv))
        observed = subprocess.run(argv, capture_output=True, text=True, timeout=40)
        output = observed.stdout + observed.stderr
        path = logs / (str(len(result['commands'])).zfill(3) + '-' + label + '.log')
        path.write_text(output, encoding='utf-8', newline='\n')
        passed = observed.returncode == expected and (marker is None or marker in output)
        result['commands'].append({'command': argv, 'exit_code': observed.returncode,
                                   'log': path.relative_to(ROOT).as_posix(),
                                   'log_sha256': hashlib.sha256(path.read_bytes()).hexdigest(), 'passed': passed})
        if not passed:
            raise RuntimeError(label + ': ' + output)
        return output

    try:
        result['tools'] = {tool: command([tool, '--version'], tool).splitlines()[0]
                           for tool in ('arm-none-eabi-gcc', 'clang', 'arm-none-eabi-nm', 'arm-none-eabi-objdump', 'qemu-system-arm')}
        with tempfile.TemporaryDirectory(prefix='cl-target-') as folder:
            folder = Path(folder)
            for compiler, optimization in [('gcc', '-O0'), ('gcc', '-O3'), ('clang', '-O3')]:
                profile = compiler + optimization
                for mutant in (0, 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 'learner'):
                    name = profile + '-' + str(mutant)
                    cc = ['arm-none-eabi-gcc'] if compiler == 'gcc' else ['clang', '--target=arm-none-eabi']
                    flags = ['-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=soft', '-std=c17', optimization,
                             '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-fno-unwind-tables',
                             '-fno-asynchronous-unwind-tables', '-Wall', '-Wextra', '-Werror', '-I'+str(public/'include')]
                    if isinstance(mutant, int):
                        flags += ['-DCL_MUTANT='+str(mutant)]
                    objects = []
                    for source in (public/'src/core.c' if mutant == 'learner' else private/'core.c', public/'targets/m4.c'):
                        obj = folder / (name + '-' + source.name + '.o')
                        command([*cc, *flags, '-c', source, '-o', obj], name+'-compile-'+source.name)
                        objects.append(obj)
                    image = folder / (name+'.elf')
                    mapping = folder / (name+'.map')
                    command(['arm-none-eabi-gcc', '-mcpu=cortex-m4', '-mthumb', '-nostdlib', '-nodefaultlibs',
                             '-nostartfiles', *objects, '-T', public/'targets/m4.ld', '-Wl,-Map='+str(mapping), '-o', image], name+'-link')
                    symbols = command(['arm-none-eabi-nm', '-n', image], name+'-symbols')
                    undefined = command(['arm-none-eabi-nm', '-u', image], name+'-undefined')
                    map_text = mapping.read_text(encoding='utf-8')
                    (logs/(name+'.map')).write_text(map_text, encoding='utf-8', newline='\n')
                    if undefined.strip() or re.search(r'\b(?:memcpy|memset|malloc|free|printf|__aeabi_\w+|__atomic_\w+|__stack_chk_fail)$', symbols, re.M) or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o', map_text):
                        raise ValueError('Unexpected runtime/helper dependency: '+name)
                    command(['arm-none-eabi-objdump', '-d', image], name+'-disassembly')
                    output = command(['qemu-system-arm', '-M', 'mps2-an386', '-cpu', 'cortex-m4', '-nographic',
                                      '-monitor', 'none', '-serial', 'none', '-nic', 'none',
                                      '-semihosting-config', 'enable=on,target=native', '-kernel', image],
                                     name+'-execute', 0 if mutant == 0 else 1,
                                     'CL_M4_PASS' if mutant == 0 else 'CL_M4_CHECK')
                    result['outcomes'].append({'profile': profile, 'variant': mutant,
                                               'passed': True, 'output': output,
                                               'image_sha256': hashlib.sha256(image.read_bytes()).hexdigest(),
                                               'undefined_symbols': [], 'forbidden_helpers': []})
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        result['error'] = str(error)
    result['source_stable'] = all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest() == h for p,h in pins.items())
    result['passed'] = 'error' not in result and result['source_stable'] and len(result['outcomes']) == 36
    result['log_sha256'] = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    payload = json.dumps(result, indent=2)+'\n'
    (logs/'report.json').write_text(payload, encoding='utf-8', newline='\n')
    destination.write_text(payload, encoding='utf-8', newline='\n')
    print(json.dumps({'passed': result['passed'], 'outcomes': len(result['outcomes']), 'error': result.get('error')}))
    return 0 if result['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(run(parser.parse_args().report))
