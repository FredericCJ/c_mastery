"""Run the original wait/IRQ teaching image in isolated Cortex-M4 QEMU only."""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PUBLIC = ROOT / 'labs/concurrency_lifecycle_depth/wait'
PRIVATE = ROOT / 'assessor/concurrency_lifecycle_depth'
HELPERS = {'memcpy', 'memset', 'memmove', '__aeabi_memclr', '__aeabi_memclr4', '__aeabi_memclr8',
           '__aeabi_memcpy', '__aeabi_memcpy4', '__aeabi_memcpy8'}


def main(report):
    destination = (ROOT / report).resolve()
    base = ROOT / 'reviews/v5/expansion/legacy_concurrency_wait'
    if not destination.is_relative_to(base.resolve()):
        raise ValueError('Use wait successor evidence directory')
    logs = base / 'target_runs' / datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs.mkdir(parents=True, exist_ok=False)
    paths = [Path(__file__).resolve(), *[PUBLIC / name for name in ('wait.h', 'wait.c', 'irq.h', 'irq.c', 'm4.c')],
             PRIVATE / 'wait.c', PRIVATE / 'wait_irq.c', ROOT / 'assessor/execution/cmx.c',
             ROOT / 'assessor/execution/support.c', ROOT / 'labs/execution/include/cmx.h',
             ROOT / 'labs/concurrency_lifecycle_depth/targets/m4.ld']
    pins = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value = {'schema_version': 1, 'created_utc': datetime.now(timezone.utc).isoformat(),
             'inspected_paths': pins, 'commands': [], 'outcomes': [],
             'scope': 'Actual CMX, generational wait wrapper and configurable IRQ0 exception execution on owned QEMU mps2-an386.',
             'limits': ['Original course memory helpers are linked explicitly and audited; this is not a helper-free image.',
                        'No libc, libgcc/compiler-rt/libatomic archive or operating system target dependency.',
                        'Logical CMX task completion, not a new context-switch implementation, physical IRQ latency or board observation.']}

    def command(argv, label, code=0, marker=None):
        argv = [str(a) for a in argv]
        result = subprocess.run(argv, capture_output=True, text=True, timeout=45)
        output = result.stdout + result.stderr
        path = logs / (str(len(value['commands'])).zfill(3) + '-' + label + '.log')
        path.write_text(output, encoding='utf-8', newline='\n')
        passed = result.returncode == code and (marker is None or marker in output)
        value['commands'].append({'command': argv, 'exit_code': result.returncode,
                                  'log': path.relative_to(ROOT).as_posix(), 'passed': passed})
        if not passed:
            raise ValueError(label + ': ' + output)
        return output

    try:
        value['toolchains'] = {name: command([name, '--version'], name).splitlines()[0]
                              for name in ('arm-none-eabi-gcc', 'clang', 'qemu-system-arm')}
        with tempfile.TemporaryDirectory(prefix='wait-irq-') as directory:
            build = Path(directory)
            for compiler, optimization in [('gcc', '-O0'), ('gcc', '-O3'), ('clang', '-O3')]:
                profile = compiler + optimization
                for variant in ('reference', 'restore-zero', 'unmasked', 'stale-predicate', 'acquisition-limit', 'irq-stub', 'wait-stub'):
                    name = profile + '-' + variant
                    cc = ['arm-none-eabi-gcc'] if compiler == 'gcc' else ['clang', '--target=arm-none-eabi']
                    flags = ['-mcpu=cortex-m4', '-mthumb', '-mfloat-abi=soft', '-std=c17', optimization,
                             '-ffreestanding', '-fno-builtin', '-fno-stack-protector', '-ffunction-sections',
                             '-fdata-sections', '-fno-unwind-tables', '-fno-asynchronous-unwind-tables',
                             '-Wall', '-Wextra', '-Werror', '-I' + str(PUBLIC), '-I' + str(ROOT / 'labs/execution/include')]
                    if compiler == 'gcc':
                        flags.append('-fno-tree-loop-distribute-patterns')
                    flags += ['-DCW_IRQ_MUTANT=' + str({'restore-zero': 1, 'unmasked': 2}.get(variant, 0)),
                              '-DCW_MUTANT=' + str({'stale-predicate': 3, 'acquisition-limit': 8}.get(variant, 0))]
                    sources = [ROOT / 'assessor/execution/cmx.c', ROOT / 'assessor/execution/support.c',
                               PUBLIC / 'wait.c' if variant == 'wait-stub' else PRIVATE / 'wait.c',
                               PUBLIC / 'irq.c' if variant == 'irq-stub' else PRIVATE / 'wait_irq.c', PUBLIC / 'm4.c']
                    objects = []
                    for index, source in enumerate(sources):
                        obj = build / (name + '-' + str(index) + '.o')
                        command([*cc, *flags, '-c', source, '-o', obj], name + '-compile-' + str(index))
                        objects.append(obj)
                    image = build / (name + '.elf')
                    mapping = build / (name + '.map')
                    command(['arm-none-eabi-gcc', '-mcpu=cortex-m4', '-mthumb', '-nostdlib', '-nodefaultlibs', '-nostartfiles',
                             *objects, '-T', ROOT / 'labs/concurrency_lifecycle_depth/targets/m4.ld', '-Wl,--gc-sections',
                             '-Wl,-Map=' + str(mapping), '-o', image], name + '-link')
                    undefined = command(['arm-none-eabi-nm', '-u', image], name + '-undefined')
                    symbols = command(['arm-none-eabi-nm', '-n', image], name + '-symbols')
                    disassembly = command(['arm-none-eabi-objdump', '-d', image], name + '-disassembly')
                    map_text = mapping.read_text(encoding='utf-8')
                    (logs / (name + '.map')).write_text(map_text, encoding='utf-8', newline='\n')
                    symbol_names = {line.split()[-1] for line in symbols.splitlines() if line.split()}
                    helpers = {s for s in symbol_names if s in HELPERS}
                    forbidden = {s for s in symbol_names if s.startswith(('__aeabi_', '__atomic_', '__sync_', '__stack_chk')) and s not in HELPERS}
                    if undefined.strip() or forbidden or re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o', map_text):
                        raise ValueError('Undeclared target runtime dependency: ' + name)
                    if not all(token in disassembly.lower() for token in ('primask', 'bkpt')):
                        raise ValueError('Missing target mask/report instruction')
                    output = command(['qemu-system-arm', '-M', 'mps2-an386', '-cpu', 'cortex-m4', '-nographic',
                        '-monitor', 'none', '-serial', 'none', '-nic', 'none', '-semihosting-config', 'enable=on,target=native',
                        '-kernel', image], name + '-execute', 0 if variant == 'reference' else 1,
                        'CW_M4_PASS' if variant == 'reference' else 'CW_M4_CHECK')
                    value['outcomes'].append({'profile': profile, 'variant': variant, 'passed': True,
                        'output': output, 'image_sha256': hashlib.sha256(image.read_bytes()).hexdigest(),
                        'undefined_symbols': [], 'linked_course_helpers': sorted(helpers), 'forbidden_helpers': []})
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        value['error'] = str(error)
    value['source_stable'] = all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == digest for p, digest in pins.items())
    value['passed'] = 'error' not in value and value['source_stable'] and len(value['outcomes']) == 21
    value['log_sha256'] = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination, logs / 'campaign.json'):
        path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps({'passed': value['passed'], 'outcomes': len(value['outcomes']), 'error': value.get('error')}))
    return 0 if value['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(main(parser.parse_args().report))
