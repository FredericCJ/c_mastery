"""Dedicated disposable guest, no attached host USB, disk or network."""
from pathlib import Path
import argparse, hashlib, json, stat, subprocess, sys
from compression.zstd import decompress
ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / 'tools'))
from run_b06_kernel_guest import newc
HERE = Path(__file__).resolve().parent
PUBLIC = ROOT / 'labs/peripheral_depth/diagnostic/guest'
BASE = Path('/home/frede/cmastery-b06-tools')
KERNEL = BASE / 'boot/vmlinuz-7.0.0-38-generic'
HEADERS = BASE / 'usr/src/linux-headers-7.0.0-38-generic'
DUMMY = BASE / 'debug-kernel/linux-source-7.0.0/drivers/usb/gadget/udc/dummy_hcd.c'
UDC = Path('/home/frede/cmastery-diagnostic-tools/modules/usr/lib/modules/7.0.0-38-generic/kernel/drivers/usb/gadget/udc/udc-core.ko.zst')
COMPOSITE = UDC.parent.parent / 'libcomposite.ko.zst'
def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
    return [*[p for d in (HERE, PUBLIC) for p in d.rglob('*') if p.is_file() and '__pycache__' not in p.parts],
            ROOT / 'tools/run_b06_kernel_guest.py', ROOT / 'assessor/peripheral_depth/diagnostic/diagnostic.c',
            ROOT / 'labs/peripheral_depth/diagnostic/diagnostic.h', ROOT / 'assessor/foundations/foundation.c',
            ROOT / 'labs/foundations/include/foundation.h', KERNEL, BASE / 'usr/bin/busybox', UDC, COMPOSITE,
            *[HEADERS / n for n in ('.config', 'Module.symvers', 'include/generated/utsrelease.h')], DUMMY]
def main():
    p = argparse.ArgumentParser()
    p.add_argument('--directory', type=Path, required=True)
    p.add_argument('--mutant', type=int, choices=[0, 1, 2], default=0)
    p.add_argument('--stub', action='store_true')
    a = p.parse_args()
    if a.stub and a.mutant:
        p.error('stub and semantic mutant are distinct controls')
    w = a.directory.resolve()
    w.mkdir(parents=True, exist_ok=True)
    records = []
    pins = {str(p): sha(p) for p in inputs()}
    def run(label, cmd, marker=None, timeout=180):
        cmd = list(map(str, cmd))
        log = w / (label + '.log')
        with log.open('w') as f:
            try:
                code = subprocess.run(cmd, cwd=ROOT, stdout=f, stderr=subprocess.STDOUT, timeout=timeout).returncode
            except (OSError, subprocess.TimeoutExpired) as ex:
                f.write(str(ex)); code = 124
        out = log.read_text(errors='replace')
        ok = code == 0 and (not marker or marker in out)
        records.append(dict(label=label, command=cmd, returncode=code, passed=ok, marker=marker, stdout=out))
        print(label, ok, flush=True)
        return ok
    for tool in ['gcc', 'gcc-15', 'make', 'qemu-system-x86_64', 'nm', 'readelf']:
        run(tool + '-version', [tool, '--version'])
    ok = run('modules', [sys.executable, HERE / 'build.py', '--directory', w / 'module', '--mutant', a.mutant, *(['--stub'] if a.stub else [])])
    ok = run('probe', ['gcc', '-std=c17', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-static',
                      '-I' + str(ROOT / 'labs/peripheral_depth/diagnostic'), '-I' + str(ROOT / 'labs/foundations/include'),
                      HERE / 'probe.c', ROOT / 'assessor/peripheral_depth/diagnostic/diagnostic.c',
                      ROOT / 'assessor/foundations/foundation.c', '-o', w / 'probe']) and ok
    if ok:
        for module in ['dummy_hcd.ko', 'dg_gadget_module.ko']:
            for name, cmd, marker in [('elf', ['readelf', '-hW'], 'Advanced Micro Devices X86-64'),
                                      ('imports', ['nm', '-u'], None), ('modinfo', ['readelf', '-p', '.modinfo'], 'vermagic=7.0.0-38-generic')]:
                ok = run(module + '-' + name, [*cmd, w / 'module' / module], marker) and ok
    if ok:
        common = """#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
fail(){ echo DG_GUEST_FAIL; dmesg; poweroff -f; }
uname -a
insmod /udc-core.ko || fail
insmod /libcomposite.ko || fail
insmod /dummy_hcd.ko is_high_speed=0 || fail
"""
        if a.stub:
            body = """insmod /dg_gadget_module.ko
if /probe; then fail; fi
dmesg | grep DG_GADGET_STUB || fail
echo DG_GUEST_STUB_REJECTED
# Registration may remain while the gadget's bind callback rejects it.
if grep -q '^dg_gadget_module ' /proc/modules; then rmmod dg_gadget_module || fail; fi
"""
        else:
            body = """insmod /dg_gadget_module.ko || fail
/probe || fail
rmmod dg_gadget_module || fail
"""
        tail = """rmmod dummy_hcd || fail
rmmod libcomposite || fail
rmmod udc_core || fail
dmesg
if dmesg | grep -E 'BUG:|WARNING:|KASAN:|possible circular locking|scheduling while atomic'; then fail; fi
echo DG_GUEST_FINISHED
poweroff -f
"""
        entries = [('bin', stat.S_IFDIR | 0o755, b''),
                   ('bin/busybox', stat.S_IFREG | 0o755, (BASE / 'usr/bin/busybox').read_bytes()),
                   ('init', stat.S_IFREG | 0o755, (common + body + tail).encode()),
                   ('udc-core.ko', stat.S_IFREG | 0o600, decompress(UDC.read_bytes())),
                   ('libcomposite.ko', stat.S_IFREG | 0o600, decompress(COMPOSITE.read_bytes())),
                   ('probe', stat.S_IFREG | 0o755, (w / 'probe').read_bytes())]
        entries += [(name, stat.S_IFREG | 0o600, (w / 'module' / name).read_bytes()) for name in ['dummy_hcd.ko', 'dg_gadget_module.ko']]
        (w / 'initramfs.cpio').write_bytes(newc(entries))
        run('guest', ['qemu-system-x86_64', '-accel', 'tcg', '-machine', 'q35', '-cpu', 'max', '-smp', '2', '-m', '384M',
                      '-nodefaults', '-display', 'none', '-serial', 'stdio', '-monitor', 'none', '-nic', 'none', '-snapshot',
                      '-no-reboot', '-kernel', KERNEL, '-initrd', w / 'initramfs.cpio',
                      '-append', 'console=ttyS0 rdinit=/init panic=-1 quiet'], 'DG_GUEST_CHECK:' if a.mutant else 'DG_GUEST_FINISHED', 300)
    guest = next((r['stdout'] for r in records if r['label'] == 'guest'), '')
    passed = all(r['passed'] for r in records) and len(records) == 15
    passed = passed and not any(s in guest for s in ['BUG:', 'Oops:', 'KASAN:', 'scheduling while atomic', 'WARNING:'])
    if a.stub:
        passed = passed and 'DG_GUEST_STUB_REJECTED' in guest and 'DG_GADGET_STUB' in guest and 'DG_GUEST_PASS' not in guest and 'DG_GUEST_FAIL' not in guest
    elif a.mutant:
        expected = {1: 'DG_GUEST_COUNTERS 33 4 33 33 0 0', 2: 'DG_GUEST_REPLAY accepted=35 rejected=1'}[a.mutant]
        passed = passed and expected in guest and 'DG_GUEST_FAIL' in guest and 'DG_GUEST_PASS' not in guest
    else:
        passed = passed and 'DG_GUEST_PASS' in guest and 'DG_GUEST_FAIL' not in guest
    stable = pins == {str(p): sha(p) for p in inputs()}
    build = json.loads((w / 'module/build.json').read_text()) if (w / 'module/build.json').is_file() else {}
    copied_current = bool(build.get('input_sha256')) and all(sha(Path(p)) == h for p, h in build.get('input_sha256', {}).items())
    passed = passed and stable and copied_current
    report = dict(passed=passed, mutant=a.mutant, learner_stub=a.stub, checks=records, sources_stable=stable,
                  copied_inputs_current=copied_current, public_build=build, input_sha256=pins,
                  output_sha256={str(p): sha(p) for p in w.rglob('*') if p.is_file() and p.name != 'report.json'},
                  limits=['Actual selected Linux host/gadget/dummy controller, no physical USB.',
                          'Kernel module depends on selected Linux API; portable core profile is audited separately.',
                          'Bounded two-struct lr_cpu guest observations; no claim of all possible callback interleavings.',
                          'No external reviewer or learner attempt.'])
    (w / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    print('guest passed', passed)
    return int(not passed)
if __name__ == '__main__':
    raise SystemExit(main())
