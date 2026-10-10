"""Compile original Thumb kernels and exercise them on declared Cortex-M4 QEMU.
No cycle, GPIO, DMA, interrupt latency or physical board claim follows.
"""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--assessor-root', type=Path)
    ap.add_argument('--report', type=Path)
    ap.add_argument('--case', choices=['all', 'lower', 'insert', 'sift'], default='all')
    ap.add_argument('--mutation', choices=['SIGNED', 'UNSTABLE', 'ABI'])
    ap.add_argument('--expect-rejection', action='store_true')
    args = ap.parse_args()
    root = Path(__file__).resolve().parents[1]
    implementation = args.assessor_root or root / 'src'
    required = ['arm-none-eabi-gcc', 'arm-none-eabi-objdump', 'qemu-system-arm']
    missing = [x for x in required if not shutil.which(x)]
    if missing:
        print('UNAVAILABLE: ' + ', '.join(missing))
        return 77
    with tempfile.TemporaryDirectory(prefix='cmastery-algorithms-thumb-') as d:
        out = Path(d)
        elf = out / 'algorithms.elf'
        cmd = ['arm-none-eabi-gcc', '-std=c17', '-mcpu=cortex-m4', '-mthumb',
               '-DALGO_THUMB_CASE=' + str(['all', 'lower', 'insert', 'sift'].index(args.case)),
               '-Os', '-ffreestanding', '-fno-builtin', '-nostdlib',
               '-I' + str(root / 'include'), str(implementation / 'thumb.S'),
               str(root / 'tests/abi_thumb.S'), str(root / 'adapters/checked_asm.c'), str(root / 'thumb/startup.c'),
               '-T' + str(root / 'thumb/link.ld'), '-Wl,-Map=' + str(out / 'algorithms.map'),
               '-lgcc', '-o', str(elf)]
        if args.mutation:
            cmd.insert(1, '-DALGO_ASM_' + args.mutation + '=1')
        subprocess.run(cmd, check=True)
        disassembly = subprocess.check_output(['arm-none-eabi-objdump', '-d', str(elf)], text=True)
        run = subprocess.run(['qemu-system-arm', '-M', 'mps2-an386', '-cpu', 'cortex-m4',
                              '-nographic', '-semihosting-config', 'enable=on,target=native',
                              '-kernel', str(elf)], text=True, capture_output=True, timeout=20)
        output = run.stdout + run.stderr
        passed = run.returncode == 0 and 'ALGO_THUMB_PASS' in output
        rejected = run.returncode != 0 and 'ALGO_THUMB_FAIL' in output
        if args.report:
            args.report.parent.mkdir(parents=True, exist_ok=True)
            args.report.with_suffix('.disassembly.txt').write_text(disassembly, encoding='utf-8')
            args.report.write_text(json.dumps({'status': 'pass' if passed else 'fail',
                'platform': 'QEMU mps2-an386 Cortex-M4', 'implementation': str(implementation),
                'mutation': args.mutation, 'case': args.case, 'compiler': subprocess.check_output([required[0], '--version'], text=True).splitlines()[0],
                'scope': 'Functional assembly, permutation/stability and base scalar ABI only',
                'stdout_stderr': output}, indent=2) + '\n', encoding='utf-8')
        print(output.strip())
        return 0 if (rejected if args.expect_rejection else passed) else 1

if __name__ == '__main__':
    raise SystemExit(main())
