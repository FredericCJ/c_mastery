"""Restore and boot the Linux x86-64 supervision lab as an ordinary host user."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import resource
import shutil
import subprocess
import sys
import tempfile
import urllib.error
import urllib.request

HERE = Path(__file__).resolve().parent
SNAPSHOT = "https://snapshot.ubuntu.com/ubuntu/20261120T000000Z/"
PACKAGES = {
    "kernel": {
        "version": "7.0.0-43.43",
        "archive": "pool/main/l/linux-signed/linux-image-7.0.0-43-generic_7.0.0-43.43_amd64.deb",
        "sha256": "fb44ae94b483fb2e04aa898b92863552bea8a92a6ea9b5f4610a7bd17ac5d4a1",
        "member": "boot/vmlinuz-7.0.0-43-generic",
    },
    "busybox": {
        "version": "1:1.37.0-7ubuntu1",
        "archive": "pool/main/b/busybox/busybox-static_1.37.0-7ubuntu1_amd64.deb",
        "sha256": "fd605342f62268753076aa7d9321ff098b36ba53e47434145a9aedc28fd141a4",
        "member": "usr/bin/busybox",
    },
}
CASES = {
    "namespace": ["view"],
    "pids": ["two", "three"],
    "shutdown": ["normal", "early", "startfail", "stubborn"],
    "network": ["A", "B"],
    "telemetry": ["baseline", "quota", "stalled"],
}
BOOT_IDENTITY = ("LS_BOOT kernel=" + Path(PACKAGES["kernel"]["member"]).name.removeprefix("vmlinuz-")
                 + " controllers=cpu pids")


class Unavailable(Exception):
    pass


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def package_path(cache, spec):
    return cache / Path(spec["archive"]).name


def verify_package(cache, spec):
    path = package_path(cache, spec)
    if not path.is_file():
        raise Unavailable(f"missing {path}. Run guest.py setup with this cache first")
    if digest(path) != spec["sha256"]:
        raise ValueError(f"package hash mismatch at {path}. Run guest.py setup to restore it")
    return path


def setup(cache):
    cache.mkdir(parents=True, exist_ok=True)
    for spec in PACKAGES.values():
        dest = package_path(cache, spec)
        if not dest.is_file() or digest(dest) != spec["sha256"]:
            # Concurrent restores publish only a complete package with the pinned hash.
            with tempfile.TemporaryDirectory(dir=cache) as stage:
                download = Path(stage) / dest.name
                with urllib.request.urlopen(SNAPSHOT + spec["archive"], timeout=90) as src:
                    with download.open("xb") as out:
                        shutil.copyfileobj(src, out)
                if digest(download) != spec["sha256"]:
                    raise ValueError(f"download digest differs from the pin for {dest.name}")
                os.replace(download, dest)
        print(f"{spec['sha256']}  {dest}")


def require_tool(name):
    if not shutil.which(name):
        raise Unavailable(f"missing host tool {name}")


def version(name):
    return subprocess.check_output([name, "--version"], text=True).splitlines()[0]


def checked_static(path):
    if not path.is_file():
        raise ValueError(f"missing executable {path}")
    with path.open("rb") as stream:
        header = stream.read(64)
    if (len(header) != 64 or header[:6] != b"\x7fELF\x02\x01"
            or int.from_bytes(header[16:18], "little") not in (2, 3)
            or int.from_bytes(header[18:20], "little") != 62
            or int.from_bytes(header[24:32], "little") == 0):
        raise ValueError("expected a little-endian ELF64 x86-64 executable with an entry point")
    # Dynamic PIE may have no interpreter in an unusual binary. Check DT_NEEDED too.
    result = subprocess.run(["readelf", "-hldW", path], capture_output=True, text=True,
                            env={**os.environ, "LC_ALL": "C"}, check=True)
    if ("Advanced Micro Devices X86-64" not in result.stdout
            or "INTERP" in result.stdout or "(NEEDED)" in result.stdout):
        raise ValueError("the guest program must be a statically linked x86-64 ELF executable")


def init_script(cases):
    calls = [f"/fixture {name}" for name in cases] or ["/fixture smoke"]
    return """#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
powerdown() {
    echo "LS_BOOT_END status=$1"
    /bin/busybox poweroff -f
    exit 1
}
mount -t proc proc /proc && mount -t sysfs sysfs /sys &&
    mount -t devtmpfs devtmpfs /dev || powerdown 77
mount -t cgroup2 cgroup2 /sys/fs/cgroup || powerdown 77
echo '+cpu +pids' > /sys/fs/cgroup/cgroup.subtree_control || powerdown 77
mkdir /sys/fs/cgroup/lab || powerdown 1
echo '+cpu +pids' > /sys/fs/cgroup/lab/cgroup.subtree_control || powerdown 77
echo outer > /run/supervision-marker || powerdown 1
echo "LS_BOOT kernel=$(uname -r) controllers=$(cat /sys/fs/cgroup/lab/cgroup.subtree_control)"
result=0
""" + "\n".join(line + ''' || {
    rc=$?
    [ "$rc" = 1 ] || powerdown "$rc"
    result=1
}''' for line in calls) + """
rmdir /sys/fs/cgroup/lab || powerdown 1
powerdown "$result"
"""


def build_archive(work, busybox, program, cases, record):
    staging = work / "root"
    staging.mkdir()
    entries = ["bin", "dev", "proc", "sys", "run", "tmp", "bin/busybox", "fixture", "init"]
    for directory in entries[:6]:
        (staging / directory).mkdir(mode=0o755)
    shutil.copyfile(busybox, staging / "bin/busybox")
    compiler = ["cc", "-std=c17", "-O2", "-Wall", "-Wextra", "-Werror", "-static",
                str(HERE / "guest_fixture.c"), "-o", str(staging / "fixture")]
    subprocess.run(compiler, check=True)
    record["fixture_compile"] = compiler
    record["sha256"]["fixture_source"] = digest(HERE / "guest_fixture.c")
    record["sha256"]["fixture_header"] = digest(HERE / "supervision.h")
    record["sha256"]["fixture"] = digest(staging / "fixture")
    (staging / "init").write_text(init_script(cases))
    if program:
        checked_static(program)
        shutil.copyfile(program, staging / "supervise")
        record["sha256"]["program"] = digest(staging / "supervise")
        entries.append("supervise")
    for name in entries:
        (staging / name).chmod(0o1777 if name == "tmp" else 0o755)
        os.utime(staging / name, (0, 0))
    image = work / "supervision.cpio"
    with image.open("xb") as out:
        subprocess.run(["cpio", "-o", "--quiet", "-H", "newc", "--owner=0:0", "--reproducible"],
                       cwd=staging, input=("\n".join(entries) + "\n").encode(), stdout=out, check=True)
    record["sha256"]["initramfs"] = digest(image)
    return image


def command(kernel, initramfs):
    return ["qemu-system-x86_64", "-machine", "q35", "-accel", "tcg", "-cpu", "max",
            "-smp", "1", "-m", "384M", "-nodefaults", "-display", "none",
            "-monitor", "none", "-serial", "stdio", "-nic", "none", "-no-reboot",
            "-kernel", str(kernel), "-initrd", str(initramfs),
            "-append", "console=ttyS0 rdinit=/init panic=-1 quiet"]


def classify(lines, cases, record):
    events = [line for line in lines if line.startswith("LS_")]
    witnesses = {}
    for line in lines:
        witness = re.match(r"LS_(INNER_FIX|OUTER|CLEAN|CASE_END) name=(\w+) variant=(\w+) ", line)
        if witness:
            event, name, variant = witness.groups()
            witnesses.setdefault((name, variant), []).append(event)
        match = re.fullmatch(r"LS_CASE_END name=(\w+) variant=(\w+) status=(\d+)", line)
        if match:
            name, variant, status = match.groups()
            record["cases"].append({"name": name, "variant": variant, "status": int(status)})
    endings = [line for line in lines if line.startswith("LS_BOOT_END ")]
    if record["qemu_returncode"] != 0 or len(endings) != 1:
        raise ValueError("QEMU or the guest did not finish normally. See serial.txt")
    if events[-1] != endings[0]:
        raise ValueError("guest reported observations after powering down")
    if endings == ["LS_BOOT_END status=77"]:
        raise Unavailable("guest capabilities unavailable. See serial.txt")
    beginnings = [line for line in events if line.startswith("LS_BOOT ")]
    if beginnings != [BOOT_IDENTITY] or events[0] != BOOT_IDENTITY:
        raise ValueError("pinned guest identity or controllers missing. See serial.txt")
    if endings not in (["LS_BOOT_END status=0"], ["LS_BOOT_END status=1"]):
        raise ValueError("guest fixture failed. See serial.txt")
    expected = [(name, variant) for name in cases for variant in CASES[name]]
    actual = [(row["name"], row["variant"]) for row in record["cases"]]
    if actual != expected:
        raise ValueError("incomplete or unexpected guest cases. See serial.txt")
    sequence = ["INNER_FIX", "OUTER", "CLEAN"]
    required = ({key: [*sequence, "CASE_END"] for key in expected} if cases
                else {("smoke", "probe"): sequence})
    if witnesses != required:
        raise ValueError("missing, duplicate or unordered fixture observations. See serial.txt")
    if any(case["status"] != 0 for case in record["cases"]) or any(
            line.startswith("LS_CONTROL_FAIL ") for line in lines):
        return "case-failed"
    if endings != ["LS_BOOT_END status=0"]:
        raise ValueError("guest fixture failed. See serial.txt")
    if not cases and not any(line.startswith("LS_CLEAN name=smoke variant=probe pids_current=0 ")
                             and " removed=1 " in line for line in lines):
        raise ValueError("smoke fixture did not exercise and release its namespaces and cgroup")
    return "completed"


def run(args):
    args.out.mkdir(parents=True, exist_ok=False)
    serial = args.out / "serial.txt"
    serial.touch()
    cases = [] if args.action == "smoke" else (args.case or list(CASES))
    record = {"status": "harness-error", "requested": cases, "cases": [],
              "sha256": {"runner": digest(HERE / "guest.py")},
              "snapshot": SNAPSHOT, "packages": PACKAGES, "tool_versions": {},
              "command": [], "qemu_returncode": None}
    try:
        if len(set(cases)) != len(cases):
            raise ValueError("a case may be requested once per boot")
        for tool in ("cc", "cpio", "dpkg-deb", "qemu-system-x86_64", "readelf"):
            require_tool(tool)
            record["tool_versions"][tool] = version(tool)
        with tempfile.TemporaryDirectory(prefix="cmastery-guest-") as tmp:
            work = Path(tmp)
            assets = {}
            for key, spec in PACKAGES.items():
                package = verify_package(args.cache, spec)
                subprocess.run(["dpkg-deb", "-x", package, work / "packages"], check=True)
                assets[key] = work / "packages" / spec["member"]
                record["sha256"][key] = digest(assets[key])
            image = build_archive(work, assets["busybox"], getattr(args, "program", None),
                                  cases, record)
            record["command"] = command(assets["kernel"], image)
            record["timeout_seconds"] = 45 + len(cases) * 45
            limits = [8 * 1024 * 1024, *resource.getrlimit(resource.RLIMIT_FSIZE)]
            record["serial_limit_bytes"] = min(n for n in limits if n != resource.RLIM_INFINITY)
            with serial.open("wb") as capture, subprocess.Popen(
                    record["command"], stdin=subprocess.DEVNULL, stdout=capture,
                    stderr=subprocess.STDOUT) as emulator:
                try:
                    ceiling = record["serial_limit_bytes"]
                    resource.prlimit(emulator.pid, resource.RLIMIT_FSIZE, (ceiling, ceiling))
                    record["qemu_returncode"] = emulator.wait(timeout=record["timeout_seconds"])
                finally:
                    if emulator.poll() is None:
                        emulator.kill()
                        emulator.wait()
        lines = serial.read_bytes().decode("utf-8", errors="replace").splitlines()
        record["status"] = classify(lines, cases, record)
    except Unavailable as exc:
        record.update(status="unavailable", reason=str(exc))
    except (ValueError, OSError, subprocess.SubprocessError) as exc:
        record.update(status="harness-error", reason=str(exc))
    record["sha256"]["serial"] = digest(serial)
    (args.out / "launch.json").write_text(json.dumps(record, indent=2) + "\n")
    print(f"{record['status']} {args.out / 'launch.json'}")
    if "reason" in record:
        print(record["reason"], file=sys.stderr)
    return {"completed": 0, "unavailable": 77}.get(record["status"], 1)


def main():
    cache_home = Path(os.environ.get("XDG_CACHE_HOME", Path.home() / ".cache"))
    default = Path(os.environ.get("CMASTERY_LINUX_GUEST_CACHE", cache_home / "cmastery/linux-supervision"))
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    for name in ("setup", "smoke", "run"):
        p = sub.add_parser(name)
        p.add_argument("--cache", type=Path, default=default)
        if name != "setup":
            p.add_argument("--out", required=True, type=Path, help="fresh output directory")
        if name == "run":
            p.add_argument("--program", required=True, type=Path)
            p.add_argument("--case", choices=CASES, action="append")
    args = parser.parse_args()
    try:
        if args.action == "setup":
            setup(args.cache)
            return 0
        return run(args)
    except (OSError, ValueError, urllib.error.URLError) as exc:
        parser.exit(1, f"{exc}\n")


if __name__ == "__main__":
    raise SystemExit(main())
