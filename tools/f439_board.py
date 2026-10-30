#!/usr/bin/env python3
"""Single entry point for every NUCLEO-F439ZI board action (safety layer).

The layer exists so that a reviewer can audit, in one file, everything a board
session is able to do. It enforces, in this order:

1. offline gates before any tool touches the board: this file is tracked and
   identical to HEAD; a fresh rebuild is byte-identical and every dependency
   it used is tracked and unmodified; every loadable byte lies in main flash
   0x08000000-0x081FFFFF and every allocated section in flash, SRAM or CCM
   (option bytes, OTP and system memory are refused);
2. two exclusive locks (fcntl.flock), both independent of --state: a
   host-wide one shared by every checkout, worktree and clone
   (/run/lock/cmastery-f439.lock, else /run/user/<uid>/cmastery-f439.lock)
   and build/f439/board.lock in this checkout;
3. a preflight before the session and before every tool call: the pinned
   openocd scripts and the openocd/st-info binaries match their SHA-256
   digests, nothing shadows them, and no other board tool is running;
4. tools run by absolute path in an empty sandbox directory with a scrubbed
   environment (no OPENOCD_SCRIPTS, HOME and XDG_CONFIG_HOME = sandbox), and
   every openocd argv starts with `-s <pinned scripts> -f <pinned board>`,
   the pinned adapter serial, disabled gdb/telnet/tcl servers and empty
   examine-end/reset-init handlers;
5. a command allowlist plus an independent denylist on every argv (no option,
   lock, unlock, protect, RDP/WRP/BOR, OTP, erase, memory writes, halting,
   register writes or probe-firmware tools; reads only from main flash,
   allowlisted registers and ELF symbols in RAM);
6. identity (read-only openocd attach; st-info --probe --hot-plug only in the
   `probe` action); programming of a re-hashed session copy; an independent
   flash readback compared in Python with the bytes the offline gate hashed;
   FLASH_OPTCR equal to the identity receipt and FLASH_OPTCR1 unchanged
   (across a flash, and against the baseline the first `probe` recorded for
   `probe` and `recover`);
7. bounded execution: timeouts, process groups terminated before the lock is
   released, byte and time bounds on captures, exclusive and unshared ports;
8. a raw log per step (argv, working directory, environment keys, stdout,
   stderr, SHA-256) and a SHA256SUMS file per session.

An attach is not free of side effects: the debug-port setup itself writes
Cortex-M debug registers and `reset run` resets the target; see PLAN.md.

Run `python3 tools/f439_board.py --help`. Tests: tools/test_f439_board.py.
Phase-2 procedure and predictions: reviews/v5/expansion/polish_revision/
physical/PLAN.md.
"""
from __future__ import annotations

import argparse
import array
from contextlib import contextmanager
from dataclasses import dataclass, field
from datetime import datetime, timezone
import fcntl
import hashlib
import json
import math
import os
from pathlib import Path
import re
import select
import shutil
import signal
import stat
import struct
import subprocess
import sys
import tempfile
import termios
import threading
import time

ROOT = Path(__file__).resolve().parents[1]
IDENTITY_JSON = (ROOT / "reviews/v5/expansion/polish_revision/physical/"
                 "identity/identity.json")
DEFAULT_STATE = ROOT / "build/f439"
MANIFEST_SCHEMA = "f439-image-manifest/v1"
PROBE_BASELINE = "probe-baseline.json"  # in the state directory
PROBE_BASELINE_SCHEMA = "f439-probe-baseline/v1"
PARK_IMAGE = "bf-report-115200"

MAX_CAPTURE_SECONDS = 600
MAX_CAPTURE_BYTES = 4 * 1024 * 1024
MAX_SEND_BYTES = 64
MAX_SENDS = 64
BAUDS = {9600: termios.B9600, 19200: termios.B19200, 38400: termios.B38400,
         57600: termios.B57600, 115200: termios.B115200,
         230400: termios.B230400}

TIMEOUT_ST_INFO = 20
TIMEOUT_OPENOCD = 30
TIMEOUT_PROGRAM = 180
TIMEOUT_DUMP = 180
TIMEOUT_BUILD = 120
RESET_TAIL_S = 5.0  # capture time kept after an in-capture debugger reset


class PolicyError(Exception):
    """A safety rule refused the request. Nothing further was executed."""


# --------------------------------------------------------------------------
# Memory map (RM0090 Rev21 memory map / Flash module organization tables for
# STM32F42x/43x as cited by the repository; the values also match the brief).
# --------------------------------------------------------------------------
@dataclass(frozen=True)
class Region:
    name: str
    start: int
    end: int  # exclusive

    def contains(self, start, size):
        return self.start <= start and start + size <= self.end


MAIN_FLASH = Region("main-flash", 0x08000000, 0x08200000)
SRAM = Region("sram", 0x20000000, 0x20030000)
CCM = Region("ccm", 0x10000000, 0x10010000)
RAM_REGIONS = (SRAM, CCM)
FORBIDDEN_REGIONS = (
    Region("system-memory", 0x1FFF0000, 0x1FFF7800),
    Region("otp", 0x1FFF7800, 0x1FFF7A10),
    Region("option-bytes-bank1", 0x1FFFC000, 0x1FFFC010),
    Region("option-bytes-bank2", 0x1FFEC000, 0x1FFEC010),
)
# Read-only debugger windows (mdw/mdh/dump_image). Option bytes, OTP and the
# rest of system memory are deliberately absent.
READABLE_REGIONS = (
    MAIN_FLASH, SRAM, CCM,
    Region("device-id", 0x1FFF7A10, 0x1FFF7A30),
    Region("peripherals", 0x40000000, 0x60000000),
    Region("private-peripheral-bus", 0xE0000000, 0xE0100000),
)
VECTOR_COUNT = 107  # 16 system + 91 external (STM32F42x/43x)


def _sector_map():
    """Dual-bank 2 MiB layout: 4x16K, 64K, 7x128K per 1 MiB bank."""
    sectors, number = [], 0
    for bank_base in (0x08000000, 0x08100000):
        address = bank_base
        for size in [0x4000] * 4 + [0x10000] + [0x20000] * 7:
            sectors.append((number, address, size))
            address += size
            number += 1
    return sectors


FLASH_SECTORS = _sector_map()

READ_REGISTERS = {
    "RCC_CR": 0x40023800, "RCC_PLLCFGR": 0x40023804, "RCC_CFGR": 0x40023808,
    "RCC_APB1ENR": 0x40023840, "RCC_BDCR": 0x40023870, "RCC_CSR": 0x40023874,
    "FLASH_ACR": 0x40023C00, "FLASH_OPTCR": 0x40023C14,
    "FLASH_OPTCR1": 0x40023C18, "PWR_CR": 0x40007000, "PWR_CSR": 0x40007004,
    "IWDG_PR": 0x40003004, "IWDG_RLR": 0x40003008, "IWDG_SR": 0x4000300C,
    "SYSCFG_MEMRMP": 0x40013800,
    "SCB_VTOR": 0xE000ED08, "SCB_SHCSR": 0xE000ED24, "SCB_CFSR": 0xE000ED28,
    "SCB_HFSR": 0xE000ED2C, "SCB_MMFAR": 0xE000ED34, "SCB_BFAR": 0xE000ED38,
    "SCB_SCR": 0xE000ED10, "DWT_CTRL": 0xE0001000,
    "DWT_CYCCNT": 0xE0001004, "DEMCR": 0xE000EDFC,
    "DBGMCU_IDCODE": 0xE0042000, "DBGMCU_CR": 0xE0042004,
    "DBGMCU_APB1_FZ": 0xE0042008,
}


def utc_stamp():
    return datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")


def utc_iso():
    return datetime.now(timezone.utc).isoformat()


def sha256_file(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def _hex32(value):
    return "none" if value is None else "0x%08x" % value


# --------------------------------------------------------------------------
# ELF32 parsing (independent of binutils; readelf is a second check at build)
# --------------------------------------------------------------------------
class ElfError(ValueError):
    pass


@dataclass
class Segment:
    type: int
    offset: int
    vaddr: int
    paddr: int
    filesz: int
    memsz: int
    flags: int


@dataclass
class Section:
    name: str
    type: int
    flags: int
    addr: int
    offset: int
    size: int


@dataclass
class ElfImage:
    entry: int
    machine: int
    segments: list = field(default_factory=list)
    sections: list = field(default_factory=list)
    symbols: dict = field(default_factory=dict)
    data: bytes = b""

    def read_flash(self, address, size):
        """Bytes at a load (physical) address, from PT_LOAD file contents."""
        for seg in self.segments:
            if seg.type == 1 and seg.filesz and \
                    seg.paddr <= address and address + size <= \
                    seg.paddr + seg.filesz:
                start = seg.offset + address - seg.paddr
                return self.data[start:start + size]
        return None


def parse_elf(data):
    if len(data) < 52:
        raise ElfError("truncated ELF header")
    if data[:4] != b"\x7fELF":
        raise ElfError("missing ELF magic")
    if data[4] != 1:
        raise ElfError("not ELF32")
    if data[5] != 1:
        raise ElfError("not little-endian")
    (_, _, machine, _, entry, phoff, shoff, _, _, phentsize, phnum,
     shentsize, shnum, shstrndx) = struct.unpack_from("<16sHHIIIIIHHHHHH",
                                                      data, 0)
    image = ElfImage(entry=entry, machine=machine, data=bytes(data))
    if phnum and (phentsize != 32 or phoff + 32 * phnum > len(data)):
        raise ElfError("truncated program header table")
    for i in range(phnum):
        fields = struct.unpack_from("<IIIIIIII", data, phoff + 32 * i)
        segment = Segment(*fields[:7])
        if segment.filesz and segment.offset + segment.filesz > len(data):
            raise ElfError("truncated segment contents")
        image.segments.append(segment)
    if shnum:
        if shentsize != 40 or shoff + 40 * shnum > len(data):
            raise ElfError("truncated section header table")
        raw = [struct.unpack_from("<IIIIIIIIII", data, shoff + 40 * i)
               for i in range(shnum)]
        if shstrndx >= shnum:
            raise ElfError("bad section-name table index")
        names = raw[shstrndx]

        def string(table, offset):
            start = table[4] + offset
            if start >= len(data):
                raise ElfError("truncated string table")
            end = data.find(b"\0", start)
            return data[start:end if end >= 0 else len(data)].decode(
                "ascii", "replace")

        for entry_fields in raw[1:]:
            image.sections.append(Section(
                string(names, entry_fields[0]), entry_fields[1],
                entry_fields[2], entry_fields[3], entry_fields[4],
                entry_fields[5]))
        for entry_fields in raw:
            if entry_fields[1] != 2 or entry_fields[6] >= shnum:
                continue
            strtab = raw[entry_fields[6]]
            for offset in range(entry_fields[4],
                                entry_fields[4] + entry_fields[5], 16):
                if offset + 16 > len(data):
                    raise ElfError("truncated symbol table")
                st_name, value, size, _, _, _ = struct.unpack_from(
                    "<IIIBBH", data, offset)
                if st_name:
                    image.symbols[string(strtab, st_name)] = (value, size)
    return image


def _label(start, size):
    hits = [r.name for r in FORBIDDEN_REGIONS
            if start < r.end and start + max(size, 1) > r.start]
    return (" [" + ", ".join(hits) + "]") if hits else ""


def check_elf(data):
    """Return a report whose `refusals` list is empty only for a safe image."""
    refusals, loads = [], []
    report = dict(refusals=refusals, loads=loads, erase_sectors=[],
                  vectors={}, ok=False)
    try:
        image = parse_elf(data)
    except ElfError as error:
        refusals.append("ELF: " + str(error))
        return report
    if image.machine != 40:
        refusals.append("ELF machine %d is not ARM (40)" % image.machine)
    for seg in image.segments:
        if seg.type != 1:
            continue
        for name, start, size in (("physical", seg.paddr, seg.filesz),
                                  ("virtual", seg.vaddr, seg.memsz)):
            if start + size > 0x100000000:
                refusals.append("PT_LOAD %s range 0x%08x+0x%x wraps"
                                % (name, start, size))
        if seg.filesz:
            loads.append(dict(paddr=seg.paddr, vaddr=seg.vaddr,
                              filesz=seg.filesz, memsz=seg.memsz))
            if not MAIN_FLASH.contains(seg.paddr, seg.filesz):
                refusals.append(
                    "loadable content outside main flash: 0x%08x+0x%x%s"
                    % (seg.paddr, seg.filesz, _label(seg.paddr, seg.filesz)))
        if seg.memsz and not any(
                r.contains(seg.vaddr, seg.memsz)
                for r in (MAIN_FLASH, *RAM_REGIONS)):
            refusals.append("PT_LOAD memory image outside flash/RAM: "
                            "0x%08x+0x%x%s" % (seg.vaddr, seg.memsz,
                                               _label(seg.vaddr, seg.memsz)))
    for sec in image.sections:
        if not sec.flags & 2 or (sec.size == 0 and sec.type == 8):
            continue
        if sec.addr + sec.size > 0x100000000:
            refusals.append("section %s wraps the address space" % sec.name)
            continue
        allowed = (RAM_REGIONS if sec.type == 8
                   else (MAIN_FLASH, *RAM_REGIONS))
        if not any(r.contains(sec.addr, sec.size) for r in allowed):
            refusals.append("section %s at 0x%08x+0x%x outside %s%s"
                            % (sec.name, sec.addr, sec.size,
                               "RAM" if sec.type == 8 else "flash/RAM",
                               _label(sec.addr, sec.size)))
    if not loads:
        refusals.append("no loadable content")
    if not MAIN_FLASH.contains(image.entry & ~1, 2):
        refusals.append("entry point 0x%08x outside main flash"
                        % image.entry)
    words = image.read_flash(MAIN_FLASH.start, 8)
    if words is None:
        refusals.append("no vector table at 0x08000000")
    else:
        stack, reset = struct.unpack("<II", words)
        report["vectors"] = dict(initial_sp=stack, reset=reset)
        if not any(r.start < stack <= r.end for r in RAM_REGIONS):
            refusals.append("initial stack 0x%08x outside SRAM/CCM" % stack)
        if stack % 8:
            refusals.append("initial stack 0x%08x not 8-byte aligned"
                            % stack)
        if not reset & 1:
            refusals.append("reset vector 0x%08x lacks the Thumb bit"
                            % reset)
        if not MAIN_FLASH.contains(reset & ~1, 2):
            refusals.append("reset vector 0x%08x outside main flash"
                            % reset)
    erase = set()
    for load in loads:
        for number, start, size in FLASH_SECTORS:
            if load["paddr"] < start + size and \
                    load["paddr"] + load["filesz"] > start:
                erase.add(number)
    report["erase_sectors"] = sorted(erase)
    report["ok"] = not refusals
    return report


# --------------------------------------------------------------------------
# Board tools: absolute paths, pinned scripts, scrubbed environment.
# openocd resolves `source [find X]` through the current directory, `-s`
# directories, OPENOCD_SCRIPTS, $XDG_CONFIG_HOME/openocd (or
# ~/.config/openocd), ~/.openocd, then $pkgdatadir/site and scripts
# (openocd.info, "Configuration Basics"). Tools therefore run in an empty
# sandbox directory with HOME and XDG_CONFIG_HOME pointing at it, without
# OPENOCD_SCRIPTS, and with `-s` naming the pinned script library first.
# --------------------------------------------------------------------------
OPENOCD = "/usr/bin/openocd"
ST_INFO = "/usr/bin/st-info"
OPENOCD_SCRIPTS = Path("/usr/share/openocd/scripts")
BOARD_CFG = str(OPENOCD_SCRIPTS / "board/st_nucleo_f4.cfg")
# Transitive closure of `source [find ...]` from BOARD_CFG, plus the two
# binaries (openocd 0.12.0-3build3, stlink-tools 1.8.0-1.1build1).
PINNED_FILES = {
    str(OPENOCD_SCRIPTS / "board/st_nucleo_f4.cfg"):
        "6bf23bf3f78cbef1f50c49d79aca41132da3c3040abf739002ab6190f90e8503",
    str(OPENOCD_SCRIPTS / "interface/stlink.cfg"):
        "b082525a7e5e6828d6b052be4a587e6a9af47c5a97836aa3d816577686dbbeea",
    str(OPENOCD_SCRIPTS / "target/stm32f4x.cfg"):
        "abaf2f78e389e1c17c73a4e32187b846130facf54e6ccbee2e08140a99279539",
    str(OPENOCD_SCRIPTS / "target/swj-dp.tcl"):
        "aad132008735bbafea3d18a304632c638f0fb99bfc19530c9f577eda2b10410a",
    str(OPENOCD_SCRIPTS / "mem_helper.tcl"):
        "01794a37e9a8bdc20e2304ee64e9d70549c0872f5f6c40fa032a165bcbba1fd2",
    OPENOCD:
        "bb51f408ad164a0cb523258780642b90e9337e3dc8ac006fd2403fcf726aa351",
    ST_INFO:
        "4cc67601ca76eda678ec9fbd2f23a9017ed36a9481d861c6a44ff54d8d1fe9bd",
}
SHADOW_DIRS = (Path("/usr/share/openocd/site"),)
LOCK_PATH = ROOT / "build/f439/board.lock"  # this checkout; not --state
# The host-wide lock is what keeps a second checkout, worktree or clone away
# from the board. /run/lock is the system lock directory (hier(7), /var/lock;
# Debian tmpfiles.d: `d /run/lock 1777 root root`, legacy.conf links
# /var/lock to it): one path for every user, shell and login session for the
# whole boot. The fallback /run/user/<uid> is the logind runtime directory
# that $XDG_RUNTIME_DIR names; it is derived from the uid because
# pam_systemd(8) leaves the variable unset for a user who is not the
# session's original user and removes the directory at the final logout.
# The first candidate that exists is used; one that exists but is unsafe is
# refused, never skipped, so two processes cannot settle on different files.
# The checkout lock stays as a second lock (a sandbox may give /run a
# private mount; the checkout is still shared).
HOST_LOCK_NAME = "cmastery-f439.lock"
HOST_LOCK_DIRS = ("/run/lock", "/run/user/{uid}")
SELF_FILES = ("tools/f439_board.py",)
SERIAL_RE = re.compile(r"[0-9A-F]{24}")
LABEL_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]{0,63}")
BOARD_TOOL_NAMES = {"openocd", "st-info", "st-flash", "st-util", "st-trace",
                    "STM32_Programmer_CLI"}


def require_serial(serial):
    if not isinstance(serial, str) or not SERIAL_RE.fullmatch(serial):
        raise PolicyError("ST-LINK serial must be 24 uppercase hex digits")
    return serial


def verify_tooling(pins=None, scripts_root=None, shadow_dirs=None):
    """Refusals unless every pinned file matches and none is shadowed."""
    pins = PINNED_FILES if pins is None else pins
    scripts_root = Path(OPENOCD_SCRIPTS if scripts_root is None
                        else scripts_root)
    shadow_dirs = SHADOW_DIRS if shadow_dirs is None else shadow_dirs
    refusals = []
    for name, digest in sorted(pins.items()):
        path = Path(name)
        if not path.is_file():
            refusals.append("pinned file missing: " + name)
            continue
        if sha256_file(path) != digest:
            refusals.append("pinned file changed: " + name)
        if path.is_relative_to(scripts_root):
            relative = path.relative_to(scripts_root)
            for shadow in shadow_dirs:
                if (Path(shadow) / relative).exists():
                    refusals.append("shadowing copy of %s in %s"
                                    % (relative, shadow))
    return refusals


def board_tool_processes(proc=Path("/proc")):
    """Other processes whose program name is a board tool."""
    found = []
    for entry in proc.iterdir():
        if not entry.name.isdigit() or int(entry.name) == os.getpid():
            continue
        try:
            argv = [a.decode(errors="replace") for a in
                    (entry / "cmdline").read_bytes().split(b"\0") if a]
        except OSError:
            continue
        if argv and os.path.basename(argv[0]) in BOARD_TOOL_NAMES:
            found.append(dict(pid=int(entry.name), argv=argv))
    return found


def tty_holders(device, proc=Path("/proc")):
    """Other processes with `device` open (only processes this user can
    inspect are visible; a root process holding the port is not)."""
    real = os.path.realpath(device)
    holders = []
    for entry in proc.iterdir():
        if not entry.name.isdigit() or int(entry.name) == os.getpid():
            continue
        try:
            fds = list((entry / "fd").iterdir())
        except OSError:
            continue
        for fd in fds:
            try:
                if os.path.realpath(fd) == real:
                    holders.append(int(entry.name))
                    break
            except OSError:
                continue
    return holders


def tool_environment(sandbox):
    sandbox = str(sandbox)
    return {"PATH": "/usr/bin:/bin", "LC_ALL": "C", "HOME": sandbox,
            "XDG_CONFIG_HOME": sandbox}


# --------------------------------------------------------------------------
# Command policy: allowlist of reviewed forms, then an independent denylist.
# --------------------------------------------------------------------------
DENY_PATTERNS = [
    r"option", r"\block\b", r"\bunlock", r"protect", r"\brdp\b", r"\bwrp\b",
    r"\bbor\b", r"optcr", r"mass_erase", r"erase_sector", r"erase_address",
    r"\botp\b", r"\b(mww|mwh|mwb|mwd|mmw)\b", r"write_bank", r"write_image",
    r"\bfill[bhwd]\b", r"load_image", r"\breg\b", r"upgrade", r"\bdfu\b",
    r"st-flash", r"programmer_cli", r"cubeprog", r"\bjava\b", r"1fffc0",
    r"1ffec0", r"\bhalt\b", r"\bresume\b", r"\bstep\b", r"verify_image",
    r"reset (halt|init)", r"\bflash\b", r"\bgdb(?!_port disabled\b)",
    r"\bbp\b", r"\bwp\b", r"\bsource\b", r"\bexec\b", r"bindto",
    r"search_dir",
]
SERVERS_OFF = ("gdb_port disabled", "telnet_port disabled",
               "tcl_port disabled")
EVENT_OFF = ("stm32f4x.cpu configure -event examine-end {}",
             "stm32f4x.cpu configure -event reset-init {}")
CONNECT_UNDER_RESET = "reset_config srst_only srst_nogate connect_assert_srst"
SELF_CHECK = (
    'echo "F439_EXAMINE_END=<[stm32f4x.cpu cget -event examine-end]>"',
    'echo "F439_RESET_INIT=<[stm32f4x.cpu cget -event reset-init]>"')
IDENTITY_READS = ["mdw 0xe0042000", "mdw 0x1fff7a10 3", "mdh 0x1fff7a22",
                  "mdw 0x40023c14", "mdw 0x40023c18", "mdw 0xe0042004",
                  "mdw 0xe0042008", "mdw 0x40023874"]
IDCODE_READ = "mdw 0xe0042000"


def _check_read(command, approved_reads):
    """mdw/mdh: main flash, an allowlisted register, or an ELF symbol."""
    match = re.fullmatch(r"(mdw|mdh) 0x([0-9a-f]{1,8})(?: ([1-9][0-9]{0,2}))?",
                         command)
    if not match:
        return False
    if command in IDENTITY_READS:
        return True
    width = 4 if match[1] == "mdw" else 2
    count, address = int(match[3] or 1), int(match[2], 16)
    size = width * count
    if count <= 128 and MAIN_FLASH.contains(address, size):
        return True
    if match[1] == "mdw" and count == 1 and \
            address in READ_REGISTERS.values():
        return True
    for start, length in approved_reads:
        if count <= 128 and any(r.contains(start, length)
                                for r in RAM_REGIONS) and \
                start <= address and address + size <= start + length:
            return True
    raise PolicyError("read outside the reviewed registers, symbols and "
                      "flash: " + command)


def _inside(path, run_dir):
    path = Path(path)
    return run_dir is not None and path.is_absolute() and \
        path.resolve().is_relative_to(Path(run_dir).resolve())


def _check_openocd_command(command, approved_elf, run_dir, approved_reads):
    if command in SELF_CHECK or command in (
            "init", "shutdown", "reset run", CONNECT_UNDER_RESET):
        return
    if _check_read(command, approved_reads):
        return
    match = re.fullmatch(r"dump_image \{([^{}]+)\} 0x([0-9a-f]{1,8}) "
                         r"0x([0-9a-f]{1,8})", command)
    if match:
        address, size = int(match[2], 16), int(match[3], 16)
        if not _inside(match[1], run_dir):
            raise PolicyError("dump_image outside the session directory")
        if size == 0 or not MAIN_FLASH.contains(address, size):
            raise PolicyError("dump_image reads main flash only")
        return
    match = re.fullmatch(r"program \{([^{}]+)\} verify reset exit", command)
    if match:
        if approved_elf is None or not _inside(match[1], run_dir) or \
                Path(match[1]).resolve() != Path(approved_elf).resolve():
            raise PolicyError("program target is not the approved session "
                              "copy of the ELF")
        return
    raise PolicyError("openocd command not in the reviewed set: " + command)


def denylisted(text):
    """Return the first denylist pattern found in text, else None."""
    for pattern in DENY_PATTERNS:
        if re.search(pattern, text, re.IGNORECASE):
            return pattern
    return None


def check_command(argv, *, approved_elf=None, serial=None, run_dir=None,
                  approved_reads=()):
    """Raise PolicyError unless argv is one of the reviewed command forms."""
    if not argv or not all(isinstance(a, str) for a in argv):
        raise PolicyError("empty or non-text argv")
    require_serial(serial)
    joined = " ".join(argv)
    pattern = denylisted(joined)
    if pattern:
        raise PolicyError("denylisted pattern %r in: %s" % (pattern, joined))
    tool = argv[0]
    if tool == ST_INFO:
        if argv != [ST_INFO, "--probe", "--hot-plug"]:
            raise PolicyError("only `st-info --probe --hot-plug` is reviewed")
        return
    if tool != OPENOCD:
        raise PolicyError("tool not in the reviewed set: " + tool)
    if argv[1:5] != ["-s", str(OPENOCD_SCRIPTS), "-f", BOARD_CFG]:
        raise PolicyError("openocd must search only the pinned scripts and "
                          "load " + BOARD_CFG)
    rest = argv[5:]
    if len(rest) % 2 or any(rest[i] != "-c" for i in range(0, len(rest), 2)):
        raise PolicyError("openocd arguments after the board must be -c pairs")
    commands = rest[1::2]
    if commands[:6] != ["adapter serial " + serial, *SERVERS_OFF,
                        *EVENT_OFF]:
        raise PolicyError("openocd must pin the adapter serial, disable the "
                          "gdb/telnet/tcl servers and the examine-end and "
                          "reset-init handlers before anything else")
    body = commands[6:]
    for command in body:
        _check_openocd_command(command, approved_elf, run_dir,
                               approved_reads)
    if body[:1] == [CONNECT_UNDER_RESET]:
        body = body[1:]
    if CONNECT_UNDER_RESET in body:
        raise PolicyError("reset configuration must precede attach")
    if any(c.startswith("program ") for c in body):
        if len(body) != 1:
            raise PolicyError("program must be the only target command")
        return
    access = ("mdw", "mdh", "dump_image", "reset run")
    if any(c.startswith(access) for c in body):
        if "init" not in body or any(
                c.startswith(access) for c in body[:body.index("init")]):
            raise PolicyError("target access before init")
    if "shutdown" in body and body[-1] != "shutdown":
        raise PolicyError("shutdown must be the final command")


def openocd_base(serial):
    argv = [OPENOCD, "-s", str(OPENOCD_SCRIPTS), "-f", BOARD_CFG]
    return _with_commands(argv, ["adapter serial " + require_serial(serial),
                                 *SERVERS_OFF, *EVENT_OFF])


def _with_commands(argv, commands):
    for command in commands:
        argv += ["-c", command]
    return argv


def openocd_identity_command(serial, extra=()):
    return _with_commands(openocd_base(serial),
                          ["init", *IDENTITY_READS, *extra, "shutdown"])


def openocd_idcode_command(serial):
    """Connect under reset; read only the debug-domain DBGMCU_IDCODE."""
    return _with_commands(openocd_base(serial), [
        CONNECT_UNDER_RESET, "init", IDCODE_READ, "shutdown"])


def openocd_program_command(serial, elf, connect_under_reset=False):
    argv = openocd_base(serial)
    if connect_under_reset:
        argv += ["-c", CONNECT_UNDER_RESET]
    return argv + ["-c", "program {%s} verify reset exit" % Path(elf)]


def openocd_reset_command(serial):
    return _with_commands(openocd_base(serial), ["init", "reset run",
                                                 "shutdown"])


def openocd_selfcheck_command(serial):
    """No `init`: openocd parses its configuration and exits."""
    return _with_commands(openocd_base(serial), [*SELF_CHECK, "shutdown"])


# --------------------------------------------------------------------------
# Lock, bounded execution and logs
# --------------------------------------------------------------------------
def host_lock_path(candidates=None):
    """The host-wide lock file in the first existing candidate directory,
    which must be a real directory owned by root or this user and writable
    by others only if it is root-owned and sticky (like /run/lock)."""
    uid = os.geteuid()
    if candidates is None:
        candidates = [d.format(uid=uid) for d in HOST_LOCK_DIRS]
    candidates = [Path(c) for c in candidates]
    for directory in candidates:
        try:
            info = os.lstat(directory)
        except FileNotFoundError:
            continue
        except OSError as error:
            raise PolicyError("host lock directory unusable: %s (%s)"
                              % (directory, error.strerror)) from None
        shared = info.st_mode & 0o022
        sticky_root = info.st_uid == 0 and info.st_mode & stat.S_ISVTX
        if not stat.S_ISDIR(info.st_mode) or info.st_uid not in (0, uid) \
                or (shared and not sticky_root):
            raise PolicyError(
                "host lock directory unusable: %s must be a directory (not "
                "a symlink) owned by root or uid %d, writable by others "
                "only if root-owned and sticky" % (directory, uid))
        return directory / HOST_LOCK_NAME
    raise PolicyError("no host lock directory among: %s"
                      % ", ".join(map(str, candidates)))


class BoardLock:
    """Exclusive, non-blocking flock(2) on a private lock file.

    The final path component is opened without following a symlink
    (open(2) O_NOFOLLOW); the file must be a single-link regular file of
    this user that group and others cannot write, and still be the file at
    `path` once locked (a lock on a replaced inode would exclude nobody).
    """

    def __init__(self, path, holder):
        self.path, self.holder, self.fd = Path(path), holder, None

    def __enter__(self):
        self.path.parent.mkdir(parents=True, exist_ok=True)
        try:
            fd = os.open(self.path, os.O_RDWR | os.O_CREAT | os.O_NOFOLLOW |
                         os.O_CLOEXEC, 0o600)
        except OSError as error:
            raise PolicyError("board lock unusable: %s (%s)"
                              % (self.path, error.strerror)) from None
        try:
            self._lock(fd)
        except BaseException:
            os.close(fd)
            raise
        self.fd = fd
        return self

    def _lock(self, fd):
        info = os.fstat(fd)
        if not stat.S_ISREG(info.st_mode) or info.st_uid != os.geteuid() \
                or info.st_nlink != 1 or info.st_mode & 0o022:
            raise PolicyError("board lock unusable: %s is not a single-link "
                              "regular file of this user closed to group and "
                              "other writes" % self.path)
        try:
            fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
        except BlockingIOError:
            holder = os.pread(fd, 4096, 0).decode(errors="replace").strip()
            raise PolicyError("board lock held: %s (%s)" % (
                self.path, holder or "no holder record")) from None
        try:
            now = os.stat(self.path, follow_symlinks=False)
        except OSError:
            now = None
        if now is None or (now.st_dev, now.st_ino) != (info.st_dev,
                                                       info.st_ino):
            raise PolicyError("board lock replaced while locking: %s"
                              % self.path)
        os.ftruncate(fd, 0)
        os.pwrite(fd, json.dumps(dict(
            pid=os.getpid(), holder=self.holder, root=str(ROOT),
            utc=utc_iso())).encode() + b"\n", 0)

    def __exit__(self, *exc):
        if self.fd is not None:
            fcntl.flock(self.fd, fcntl.LOCK_UN)
            os.close(self.fd)
        self.fd = None
        return False


_LIVE = set()
_LIVE_LOCK = threading.Lock()


def live_children():
    with _LIVE_LOCK:
        return [p.pid for p in _LIVE if p.poll() is None]


def _stop(process):
    """SIGTERM the child's process group, then SIGKILL after 2 s."""
    if process.poll() is not None:
        return
    for sig, wait in ((signal.SIGTERM, 2), (signal.SIGKILL, 5)):
        try:
            os.killpg(process.pid, sig)
        except OSError:
            pass
        try:
            process.wait(wait)
            return
        except subprocess.TimeoutExpired:
            continue


def terminate_live_children():
    with _LIVE_LOCK:
        processes = list(_LIVE)
    for process in processes:
        _stop(process)


def run_bounded(argv, *, timeout_s, log_dir, label, cwd=None, env=None):
    """Run argv with a hard timeout in its own process group; write
    stdout/stderr/record files. A child never outlives this call."""
    log_dir = Path(log_dir)
    log_dir.mkdir(parents=True, exist_ok=True)
    start_utc, started = utc_iso(), time.monotonic()
    env = dict(env) if env is not None else dict(os.environ, LC_ALL="C")
    stdout = stderr = b""
    rc, timed_out = None, False
    try:
        process = subprocess.Popen(argv, cwd=cwd, env=env,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   start_new_session=True)
    except OSError as error:
        process, stderr = None, str(error).encode()
    if process is not None:
        with _LIVE_LOCK:
            _LIVE.add(process)
        try:
            try:
                stdout, stderr = process.communicate(timeout=timeout_s)
                rc = process.returncode
            except subprocess.TimeoutExpired:
                timed_out = True
                _stop(process)
                stdout, stderr = process.communicate()
        finally:
            _stop(process)
            with _LIVE_LOCK:
                _LIVE.discard(process)
    record = dict(argv=list(argv), cwd=str(cwd) if cwd else None,
                  env_keys=sorted(env), timeout_s=timeout_s,
                  start_utc=start_utc,
                  duration_s=round(time.monotonic() - started, 6),
                  returncode=rc, timed_out=timed_out,
                  stdout=label + ".stdout", stderr=label + ".stderr",
                  stdout_sha256=hashlib.sha256(stdout).hexdigest(),
                  stderr_sha256=hashlib.sha256(stderr).hexdigest(),
                  record=label + ".json")
    (log_dir / record["stdout"]).write_bytes(stdout)
    (log_dir / record["stderr"]).write_bytes(stderr)
    (log_dir / record["record"]).write_text(json.dumps(record, indent=1) +
                                            "\n")
    return record


def write_sums(directory):
    directory = Path(directory)
    lines = ["%s  %s" % (sha256_file(p), p.relative_to(directory).as_posix())
             for p in sorted(directory.rglob("*"))
             if p.is_file() and p.name != "SHA256SUMS"]
    (directory / "SHA256SUMS").write_text("\n".join(lines) + "\n")


def journal_window(start_epoch, end_epoch, log_dir):
    """ModemManager journal entries around a capture (host log, read-only)."""
    argv = ["journalctl", "-u", "ModemManager.service", "--since",
            "@%d" % math.floor(start_epoch - 1), "--until",
            "@%d" % math.ceil(end_epoch + 1), "--no-pager", "-o",
            "short-iso-precise"]
    record = run_bounded(argv, timeout_s=20, log_dir=log_dir,
                         label="modemmanager-journal")
    text = (Path(log_dir) / record["stdout"]).read_text(errors="replace")
    entries = [l for l in text.splitlines()
               if l.strip() and not l.startswith("-- ")]
    return dict(status="recorded" if record["returncode"] == 0
                else "unavailable", returncode=record["returncode"],
                entries=len(entries), file=record["stdout"],
                sha256=record["stdout_sha256"], argv=argv)


# --------------------------------------------------------------------------
# Serial capture (bounded, raw, timestamped)
# --------------------------------------------------------------------------
TIOCGICOUNT = 0x545D
TIOCGEXCL = 0x80045440  # _IOR('T', 0x40, int), asm-generic/ioctls.h
ICOUNT_FIELDS = ("cts", "dsr", "rng", "dcd", "rx", "tx", "frame", "overrun",
                 "parity", "brk", "buf_overrun")


def _icount(fd):
    try:
        buffer = array.array("i", [0] * 20)
        fcntl.ioctl(fd, TIOCGICOUNT, buffer, True)
        return dict(zip(ICOUNT_FIELDS, buffer.tolist()))
    except OSError as error:
        return dict(unsupported=str(error))


def require_label(label):
    if not isinstance(label, str) or not LABEL_RE.fullmatch(label):
        raise PolicyError("label must match %s" % LABEL_RE.pattern)
    return label


def _exclusive(fd):
    """Put the tty in exclusive mode and confirm it took effect."""
    try:
        fcntl.ioctl(fd, termios.TIOCEXCL)
        state = array.array("i", [0])
        fcntl.ioctl(fd, TIOCGEXCL, state, True)
        return state[0] != 0
    except OSError:
        return False


def _release(fd):
    """Leave exclusive mode (TIOCNXCL) and close the port."""
    try:
        fcntl.ioctl(fd, termios.TIOCNXCL)
    except OSError:
        pass
    os.close(fd)


def capture_serial(device, *, seconds, max_bytes, baud, parity, out_dir,
                   sends=(), label="capture", on_start=None):
    """Record raw bytes from `device` until a time or byte bound is reached.

    Refused if another visible process has the port open or exclusive mode
    cannot be confirmed. sends: (offset_seconds, bytes) written at that
    offset. Writes <label>.bin, <label>.index.jsonl and <label>.json.
    """
    require_label(label)
    if not 0 < seconds <= MAX_CAPTURE_SECONDS:
        raise PolicyError("capture seconds must be in (0, %d]"
                          % MAX_CAPTURE_SECONDS)
    if not 0 < max_bytes <= MAX_CAPTURE_BYTES:
        raise PolicyError("capture bytes must be in (0, %d]"
                          % MAX_CAPTURE_BYTES)
    if baud not in BAUDS or parity not in ("none", "even", "odd"):
        raise PolicyError("unsupported line setting")
    sends = sorted((float(t), bytes(b)) for t, b in sends)
    if len(sends) > MAX_SENDS or any(
            not 0 <= t <= seconds or not 0 < len(b) <= MAX_SEND_BYTES
            for t, b in sends):
        raise PolicyError("send schedule outside the reviewed bounds")
    holders = tty_holders(device)
    if holders:
        raise PolicyError("serial port already open by pid %s" % holders)
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    if not _exclusive(fd):
        _release(fd)
        raise PolicyError("exclusive access to the serial port failed")
    raw_path = out_dir / (label + ".bin")
    index_path = out_dir / (label + ".index.jsonl")
    try:
        attrs = termios.tcgetattr(fd)
        cflag = termios.CS8 | termios.CREAD | termios.CLOCAL
        iflag = 0
        if parity != "none":
            cflag |= termios.PARENB | (termios.PARODD if parity == "odd"
                                       else 0)
            iflag |= termios.INPCK | termios.PARMRK
        attrs[0], attrs[1], attrs[2], attrs[3] = iflag, 0, cflag, 0
        attrs[4] = attrs[5] = BAUDS[baud]
        attrs[6][termios.VMIN], attrs[6][termios.VTIME] = 0, 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        termios.tcflush(fd, termios.TCIOFLUSH)
        applied = termios.tcgetattr(fd)
        icount_before = _icount(fd)
        start_utc, start_epoch = utc_iso(), time.time()
        start_ns = time.monotonic_ns()
        if on_start is not None:
            on_start(start_ns)
        total, stop_reason, pending = 0, "time-bound", list(sends)
        with open(raw_path, "wb") as raw, open(index_path, "w") as index:
            while True:
                now_ns = time.monotonic_ns() - start_ns
                while pending and pending[0][0] * 1e9 <= now_ns:
                    offset, data = pending.pop(0)
                    before = time.monotonic_ns() - start_ns
                    written = os.write(fd, data)
                    after = time.monotonic_ns() - start_ns
                    index.write(json.dumps(dict(
                        dir="tx", t_ns=before, t_end_ns=after,
                        scheduled_s=offset, hex=data[:written].hex()))
                        + "\n")
                    now_ns = after
                if now_ns >= seconds * 1e9:
                    break
                wait_ns = seconds * 1e9 - now_ns
                if pending:
                    wait_ns = min(wait_ns, pending[0][0] * 1e9 - now_ns)
                ready, _, _ = select.select([fd], [], [],
                                            max(wait_ns, 0) / 1e9)
                if not ready:
                    continue
                try:
                    chunk = os.read(fd, min(4096, max_bytes - total))
                except BlockingIOError:
                    continue
                stamp = time.monotonic_ns() - start_ns
                if not chunk:
                    continue
                raw.write(chunk)
                index.write(json.dumps(dict(dir="rx", t_ns=stamp,
                                            offset=total, len=len(chunk)))
                            + "\n")
                total += len(chunk)
                if total >= max_bytes:
                    stop_reason = "byte-bound"
                    break
        end_epoch = time.time()
        icount_after = _icount(fd)
        final = termios.tcgetattr(fd)
    finally:
        _release(fd)
    summary = dict(device=str(device), start_utc=start_utc,
                   start_epoch=start_epoch, end_epoch=end_epoch,
                   start_monotonic_ns=start_ns, seconds=seconds,
                   max_bytes=max_bytes, baud=baud, parity=parity,
                   termios=dict(iflag=applied[0], oflag=applied[1],
                                cflag=applied[2], lflag=applied[3],
                                ispeed=applied[4], ospeed=applied[5]),
                   termios_unchanged=final == applied,
                   icount_before=icount_before, icount_after=icount_after,
                   exclusive=True, bytes=total, stop_reason=stop_reason,
                   raw=raw_path.name, raw_sha256=sha256_file(raw_path),
                   index=index_path.name,
                   index_sha256=sha256_file(index_path),
                   sends=[dict(offset_s=t, hex=b.hex()) for t, b in sends])
    (out_dir / (label + ".json")).write_text(json.dumps(summary, indent=1) +
                                             "\n")
    return summary


def serial_device_for(serial):
    """udev by-id link for the ST-LINK virtual COM port (interface 02)."""
    return Path("/dev/serial/by-id/"
                "usb-STMicroelectronics_STM32_STLink_%s-if02"
                % require_serial(serial))


# --------------------------------------------------------------------------
# Identity
# --------------------------------------------------------------------------
def parse_st_info(text):
    def find(pattern):
        match = re.search(pattern, text)
        return match[1] if match else None
    count = find(r"Found (\d+) stlink programmers")
    flash = find(r"flash:\s+(\d+)")
    return dict(programmers=int(count) if count else None,
                version=find(r"version:\s+(\S+)"),
                serial=find(r"serial:\s+([0-9A-Fa-f]+)"),
                flash_bytes=int(flash) if flash else None,
                chipid=find(r"chipid:\s+(0x[0-9a-fA-F]+)"))


def parse_openocd_reads(text):
    """mdw/mdh output as blocks [start, width, values]. openocd 0.12 prints
    eight words (32 bytes) per line, each line prefixed by its address, so
    a line continuing the previous one at the same width is merged."""
    blocks = []
    for match in re.finditer(r"^0x([0-9a-f]{8}):((?: [0-9a-f]{4,8})+)\s*$",
                             text, re.MULTILINE):
        tokens = match[2].split()
        if len({len(t) for t in tokens}) != 1:
            continue
        address, width = int(match[1], 16), len(tokens[0]) // 2
        values = [int(t, 16) for t in tokens]
        if blocks:
            start, last_width, last_values = blocks[-1]
            if last_width == width and \
                    start + width * len(last_values) == address:
                last_values.extend(values)
                continue
        blocks.append([address, width, values])
    return blocks


def read_words(blocks, address, count, width=4):
    """`count` values of `width` bytes at `address`, or None if absent."""
    for start, block_width, values in blocks:
        offset = address - start
        if block_width == width and offset >= 0 and offset % width == 0 \
                and offset // width + count <= len(values):
            return values[offset // width:offset // width + count]
    return None


def _first(values):
    return values[0] if values else None


def identity_from_outputs(st_info_text, openocd_text):
    st = parse_st_info(st_info_text)
    blocks = parse_openocd_reads(openocd_text)
    idcode = _first(read_words(blocks, 0xE0042000, 1))
    version = re.search(r"STLINK (V\S+)", openocd_text)
    voltage = re.search(r"Target voltage: ([0-9.]+)", openocd_text)
    return dict(programmers=st["programmers"], stlink_serial=st["serial"],
                st_info_version=st["version"], st_info_chipid=st["chipid"],
                st_info_flash_bytes=st["flash_bytes"],
                idcode=idcode,
                dev_id=None if idcode is None else idcode & 0xFFF,
                rev_id=None if idcode is None else idcode >> 16,
                uid=read_words(blocks, 0x1FFF7A10, 3),
                flash_kib=_first(read_words(blocks, 0x1FFF7A22, 1, 2)),
                optcr=_first(read_words(blocks, 0x40023C14, 1)),
                optcr1=_first(read_words(blocks, 0x40023C18, 1)),
                dbgmcu_cr=_first(read_words(blocks, 0xE0042004, 1)),
                dbgmcu_apb1_fz=_first(read_words(blocks, 0xE0042008, 1)),
                rcc_csr=_first(read_words(blocks, 0x40023874, 1)),
                openocd_stlink_version=version[1] if version else None,
                target_voltage_v=float(voltage[1]) if voltage else None)


def load_expected_identity(path=IDENTITY_JSON):
    observed = json.loads(Path(path).read_text())["observed"]
    words = [re.match(r"0x([0-9A-Fa-f]{8})", observed[key])
             for key in ("DBGMCU_IDCODE", "FLASH_OPTCR")]
    if not all(words):
        raise PolicyError("identity receipt is not parsable: %s" % path)
    idcode, optcr = (int(match[1], 16) for match in words)
    return dict(serial=require_serial(observed["stlink_serial"]),
                dev_id=idcode & 0xFFF, rev_id=idcode >> 16,
                uid=[int(w, 16) for w in observed["UID_96"].split()],
                flash_kib=int(observed["flash_size_kib"]), optcr=optcr)


def identity_mismatches(observed, expected, check_probe=True):
    problems = []

    def compare(label, seen, want, fmt=str):
        if seen != want:
            problems.append("%s: expected %s, observed %s"
                            % (label, fmt(want),
                               "missing" if seen is None else fmt(seen)))

    def hexs(value):
        return " ".join("%08x" % v for v in value) if isinstance(
            value, list) else "0x%x" % value

    if check_probe:
        compare("ST-LINK programmers", observed["programmers"], 1)
        compare("ST-LINK serial", observed["stlink_serial"],
                expected["serial"])
        compare("st-info chipid", observed["st_info_chipid"],
                "0x%03x" % expected["dev_id"])
        compare("st-info flash bytes", observed["st_info_flash_bytes"],
                expected["flash_kib"] * 1024)
    compare("DBGMCU_IDCODE DEV_ID", observed["dev_id"], expected["dev_id"],
            hexs)
    compare("DBGMCU_IDCODE REV_ID", observed["rev_id"], expected["rev_id"],
            hexs)
    compare("UID", observed["uid"], expected["uid"], hexs)
    compare("flash size KiB", observed["flash_kib"], expected["flash_kib"])
    compare("FLASH_OPTCR", observed["optcr"], expected["optcr"], hexs)
    return problems


# --------------------------------------------------------------------------
# Image registry and reproducible builds
# --------------------------------------------------------------------------
BU_LAB = "labs/bringup_depth"
BU_REF = "assessor/bringup_depth/target"
BF = "assessor/board_f439"
TARGET_FLAGS = ["-std=c17", "-O2", "-mcpu=cortex-m4", "-mthumb",
                "-mfloat-abi=soft", "-ffreestanding", "-fno-builtin",
                "-fno-stack-protector", "-ffunction-sections",
                "-fdata-sections", "-Wall", "-Wextra", "-Werror"]
# House firmware profile (curriculum/guides/c-style.md, Tier C) for the new
# board harness sources; reused assessor sources keep their own flag set.
HARNESS_STRICT = ["-Wconversion", "-Wsign-conversion", "-Wpedantic"]
HARNESS_INCLUDES = ["-I" + BF, "-Ilabs/peripheral_depth/uart",
                    "-Ilabs/serial_depth/include",
                    "-Ilabs/constrained_depth/supervision"]
HARNESS_OWN = [BF + "/startup.S", BF + "/bf_main.c", BF + "/bf_report.c",
               BF + "/bf_hw.c"]
HARNESS_REUSED = ["assessor/peripheral_depth/uart/uart.c",
                  "assessor/peripheral_depth/uart/target.c",
                  "assessor/serial_depth/serial_depth.c",
                  "assessor/legacy_constrained/supervision.c",
                  "assessor/legacy_constrained/power.c",
                  "assessor/legacy_constrained/power_f439.c",
                  "assessor/execution/support.c"]


def _bu(defines, experiment, purpose):
    return dict(kind="bringup", defines=defines, experiment=experiment,
                purpose=purpose)


def _bf(defines, experiment, purpose):
    return dict(kind="harness", defines=defines, experiment=experiment,
                purpose=purpose)


IMAGES = {
    "bu-cold-cross": _bu({"BU_F439": "1"}, "E2",
                         "existing T27 cold image; retained stage via SWD"),
    "bu-software-reset-cross": _bu(
        {"BU_F439": "1", "BU_SOFTWARE_RESET": "1"}, "E2",
        "existing T27 image, AIRCR software-reset path on F439"),
    "bu-watchdog-cross": _bu({"BU_F439": "1", "BU_WATCHDOG": "1"}, "E3",
                             "existing BOARD_CAPTURE.md watchdog-cross image"),
    "bu-guard-cross": _bu({"BU_F439": "1", "BU_GUARD_FAULT": "1"}, "X1",
                          "existing MPU stack-guard MemManage path on F439"),
    "bf-report-115200": _bf({"BF_MODE": "1", "BF_BAUD": "115200"}, "E1,E2",
                            "USART3 report/echo/burst at 115200 8N1; park"),
    "bf-report-57600": _bf({"BF_MODE": "1", "BF_BAUD": "57600"}, "E1",
                           "same report at 57600 8N1 (changed condition)"),
    "bf-iwdg-fed": _bf({"BF_MODE": "2", "BF_STARVE_AFTER": "0"}, "E3",
                       "progress-epoch IWDG feeding, never starved"),
    "bf-iwdg-starve": _bf({"BF_MODE": "2", "BF_STARVE_AFTER": "10"}, "E3",
                          "task 1 stops progressing after 10 windows"),
    "bf-clock-core": _bf({"BF_MODE": "3", "BF_SYSTICK_DIV8": "0"}, "E4",
                         "SysTick on the processor clock vs DWT CYCCNT"),
    "bf-clock-div8": _bf({"BF_MODE": "3", "BF_SYSTICK_DIV8": "1"}, "E4",
                         "SysTick on HCLK/8 (changed condition)"),
    "bf-stop-nodebug": _bf({"BF_MODE": "4", "BF_STOP_DEBUG": "0"}, "E4",
                           "Stop via cs_f439_stop, DBGMCU_CR=0, EXTI9 wake"),
    "bf-stop-debug": _bf({"BF_MODE": "4", "BF_STOP_DEBUG": "1"}, "E4",
                         "Stop with DBG_STOP=1 (changed condition)"),
}


def image_sources(name):
    spec = IMAGES[name]
    if spec["kind"] == "bringup":
        return [BU_LAB + "/target/image.ld.in", BU_REF + "/reset.S",
                BU_REF + "/guest.c", BU_LAB + "/bringup.h"]
    return [BF + "/board.ld", *HARNESS_OWN, *HARNESS_REUSED]


def _objects(name, out_dir):
    spec = IMAGES[name]
    sources = ([BU_REF + "/reset.S", BU_REF + "/guest.c"]
               if spec["kind"] == "bringup" else HARNESS_OWN + HARNESS_REUSED)
    return [(s, (Path(out_dir) / ("%02d-%s.o" % (i, Path(s).stem)))
             .as_posix()) for i, s in enumerate(sources)]


def build_commands(name, out_dir):
    """Compile and link argv lists, relative to ROOT (run with cwd=ROOT)."""
    spec, out = IMAGES[name], Path(out_dir)
    defines = ["-D%s=%s" % kv for kv in sorted(spec["defines"].items())]
    commands = []
    for source, obj in _objects(name, out):
        flags = list(TARGET_FLAGS)
        if spec["kind"] == "bringup":
            flags += ["-I" + BU_LAB]
        else:
            flags += HARNESS_INCLUDES
            if source in HARNESS_OWN and source.endswith(".c"):
                flags += HARNESS_STRICT
        commands.append(["arm-none-eabi-gcc", *flags, *defines, "-MMD", "-MF",
                         obj[:-2] + ".d", "-c", source, "-o", obj])
    script = (out / "image.ld").as_posix() if spec["kind"] == "bringup" \
        else BF + "/board.ld"
    commands.append(["arm-none-eabi-ld", "--gc-sections", "-z", "noexecstack",
                     "-T", script,
                     "-Map=" + (out / "image.map").as_posix(),
                     *[o for _, o in _objects(name, out)], "-o",
                     (out / "image.elf").as_posix()])
    return commands


def _bringup_script():
    template = (ROOT / BU_LAB / "target/image.ld.in").read_text()
    return template.replace("@CODE@", "0x08000000").replace(
        "@BANK@", "0x10000000")


def _depfile_sources(path):
    text = Path(path).read_text().replace("\\\n", " ")
    _, _, deps = text.partition(":")
    return [d for d in deps.split() if not d.startswith("/")]


def readelf_cross_check(elf, log_dir):
    """Second, binutils-based check of the same address rules."""
    record = run_bounded(["arm-none-eabi-readelf", "-lSW", str(elf)],
                         timeout_s=TIMEOUT_BUILD, log_dir=log_dir,
                         label="readelf")
    text = (Path(log_dir) / record["stdout"]).read_text()
    problems = []
    for match in re.finditer(r"^\s*LOAD\s+0x([0-9a-f]+) 0x([0-9a-f]+) "
                             r"0x([0-9a-f]+) 0x([0-9a-f]+) 0x([0-9a-f]+)",
                             text, re.MULTILINE):
        paddr, filesz = int(match[3], 16), int(match[4], 16)
        if filesz and not MAIN_FLASH.contains(paddr, filesz):
            problems.append("readelf LOAD 0x%08x+0x%x outside main flash"
                            % (paddr, filesz))
    sections = re.findall(r"^\s*\[\s*\d+\]\s+(\S+)\s+(\S+)\s+([0-9a-f]{8}) "
                          r"[0-9a-f]+ ([0-9a-f]+) [0-9a-f]+ +([A-Z]*)",
                          text, re.MULTILINE)
    for name, kind, addr, size, flags in sections:
        if "A" not in flags or not int(size, 16):
            continue
        allowed = RAM_REGIONS if kind == "NOBITS" else (MAIN_FLASH,
                                                        *RAM_REGIONS)
        if not any(r.contains(int(addr, 16), int(size, 16))
                   for r in allowed):
            problems.append("readelf section %s outside allowed regions"
                            % name)
    if record["returncode"] != 0 or not sections:
        problems.append("readelf failed or reported no sections")
    return problems


def build_image(name, build_root, *, log=print):
    """Build one image under build_root/name; return its manifest dict."""
    out = Path(build_root) / name
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)
    logs = out / "logs"
    rel_out = os.path.relpath(out, ROOT)
    generated = {}
    if IMAGES[name]["kind"] == "bringup":
        (out / "image.ld").write_text(_bringup_script())
        generated["image.ld"] = sha256_file(out / "image.ld")
    steps = []
    for i, argv in enumerate(build_commands(name, rel_out)):
        record = run_bounded(argv, timeout_s=TIMEOUT_BUILD, log_dir=logs,
                             label="%02d-%s" % (i, argv[0]), cwd=ROOT)
        steps.append(record)
        if record["returncode"] != 0:
            raise PolicyError("build step failed: %s (see %s)"
                              % (" ".join(argv), logs / record["stderr"]))
    deps = set(image_sources(name))
    for dep in out.glob("*.d"):
        deps.update(_depfile_sources(dep))
    sources = {p: sha256_file(ROOT / p) for p in sorted(deps)}
    elf = out / "image.elf"
    report = check_elf(elf.read_bytes())
    cross = readelf_cross_check(elf, logs)
    extra = {}
    for label, argv in [("nm-undefined", ["arm-none-eabi-nm", "-u", elf]),
                        ("nm", ["arm-none-eabi-nm", "-n", elf]),
                        ("objdump-h", ["arm-none-eabi-objdump", "-h", elf]),
                        ("size", ["arm-none-eabi-size", "-A", elf])]:
        record = run_bounded([str(a) for a in argv], timeout_s=TIMEOUT_BUILD,
                             log_dir=logs, label=label)
        extra[label] = dict(returncode=record["returncode"],
                            stdout_sha256=record["stdout_sha256"])
    undefined = (logs / "nm-undefined.stdout").read_text().strip()
    if undefined:
        cross.append("unresolved symbols: " + undefined.replace("\n", ", "))
    if IMAGES[name]["kind"] == "bringup":
        record = run_bounded([sys.executable, BU_LAB + "/check_image.py",
                              os.path.relpath(elf, ROOT), "--code",
                              "0x08000000", "--bank", "0x10000000"],
                             timeout_s=TIMEOUT_BUILD, log_dir=logs,
                             label="check_image", cwd=ROOT)
        extra["check_image"] = dict(returncode=record["returncode"],
                                    stdout_sha256=record["stdout_sha256"])
        if record["returncode"] != 0:
            cross.append("labs/bringup_depth/check_image.py rejected image")
    parsed = parse_elf(elf.read_bytes())
    manifest = dict(
        schema=MANIFEST_SCHEMA, image=name, spec=IMAGES[name],
        elf="image.elf", elf_sha256=sha256_file(elf), sources=sources,
        generated=generated, commands=[s["argv"] for s in steps],
        toolchain=_toolchain(), elf_check=report, readelf_check=cross,
        binutils=extra, symbols={k: list(v) for k, v in
                                 sorted(parsed.symbols.items())},
        built_utc=utc_iso())
    (out / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
    if not report["ok"] or cross:
        raise PolicyError("image %s failed safety checks: %s"
                          % (name, report["refusals"] + cross))
    log("built %s %s sectors=%s" % (name, manifest["elf_sha256"],
                                    report["erase_sectors"]))
    return manifest


def _toolchain():
    versions = {}
    for tool in ("arm-none-eabi-gcc", "arm-none-eabi-ld"):
        try:
            versions[tool] = subprocess.run(
                [tool, "--version"], capture_output=True, text=True,
                timeout=20).stdout.splitlines()[0]
        except (OSError, subprocess.TimeoutExpired, IndexError):
            versions[tool] = None
    return versions


def rebuild(name, build_root):
    """Fresh build; returns the ELF digest and the dependency set the
    compiler actually used (-MMD), which the provenance gate checks."""
    manifest = build_image(name, build_root, log=lambda *_: None)
    return dict(elf_sha256=manifest["elf_sha256"],
                sources=manifest["sources"])


# --------------------------------------------------------------------------
# Provenance
# --------------------------------------------------------------------------
def _git(repo, *args):
    return subprocess.run(["git", *args], cwd=repo, capture_output=True,
                          text=True, timeout=60)


def verify_sources(sources, repo):
    """Each path inside the repository, tracked, identical to HEAD and (when
    a digest is given) unchanged since it was hashed."""
    refusals = []
    repo = Path(repo).resolve()
    for path, digest in sorted(sources.items()):
        candidate = Path(path)
        full = (repo / candidate).resolve()
        if candidate.is_absolute() or ".." in candidate.parts or \
                not full.is_relative_to(repo):
            refusals.append("source outside the repository: " + path)
            continue
        if not full.is_file() or (digest is not None and
                                  sha256_file(full) != digest):
            refusals.append("source changed since build: " + path)
        if _git(repo, "ls-files", "--error-unmatch", "--", path).returncode:
            refusals.append("source not tracked by Git: " + path)
        elif _git(repo, "status", "--porcelain", "--", path).stdout.strip():
            refusals.append("source differs from HEAD: " + path)
    return refusals


def verify_provenance(manifest, repo, image_dir, rebuilt):
    """`rebuilt` = {elf_sha256, sources} from a fresh build; its dependency
    set (from the compiler's -MMD files) is the one that must be clean."""
    refusals = []
    if manifest.get("schema") != MANIFEST_SCHEMA:
        refusals.append("manifest schema is not " + MANIFEST_SCHEMA)
    elf = Path(image_dir) / str(manifest.get("elf", ""))
    if not elf.is_file() or sha256_file(elf) != manifest.get("elf_sha256"):
        refusals.append("ELF digest differs from its build manifest")
    if rebuilt.get("elf_sha256") != manifest.get("elf_sha256"):
        refusals.append("rebuild from the working tree is not identical")
    if rebuilt.get("sources") != manifest.get("sources"):
        refusals.append("manifest dependency list differs from the rebuild")
    sources = rebuilt.get("sources") or {}
    if not sources:
        refusals.append("rebuild reported no sources")
    refusals += verify_sources(sources, repo)
    return refusals


def load_extent(image):
    loads = [s for s in image.segments if s.type == 1 and s.filesz]
    start = min(s.paddr for s in loads)
    return start, max(s.paddr + s.filesz for s in loads) - start


def compare_readback(image, start, data):
    """Segment byte ranges of the ELF that differ from flash readback."""
    differences = []
    for seg in image.segments:
        if seg.type != 1 or not seg.filesz:
            continue
        want = image.data[seg.offset:seg.offset + seg.filesz]
        got = data[seg.paddr - start:seg.paddr - start + seg.filesz]
        if got != want:
            differences.append("0x%08x+0x%x" % (seg.paddr, seg.filesz))
    return differences


# --------------------------------------------------------------------------
# Board sessions
# --------------------------------------------------------------------------
DIR_TAGS = {"flash": "program"}  # keep the denylisted word out of paths


class Session:
    def __init__(self, board, action, subject=None):
        self.board, self.action, self.subject = board, action, subject
        tag = DIR_TAGS.get(action, action) + ("-" + subject if subject
                                              else "")
        self.dir = board.state / "runs" / ("%s-%s" % (utc_stamp(), tag))
        self.steps, self.planned, self.status = [], [], "started"
        self.sandbox, self.details, self.locks = None, {}, []

    def run(self, argv, *, timeout_s, label, approved_elf=None,
            approved_reads=()):
        check_command(argv, approved_elf=approved_elf,
                      serial=self.board.serial, run_dir=self.dir,
                      approved_reads=approved_reads)
        if self.board.dry_run:
            self.planned.append(list(argv))
            return None
        self.board.preflight()
        if os.listdir(self.sandbox):
            raise PolicyError("tool sandbox is not empty: %s" % self.sandbox)
        record = self.board.executor(
            argv, timeout_s=timeout_s, log_dir=self.dir,
            label="%02d-%s" % (len(self.steps), label),
            cwd=str(self.sandbox), env=tool_environment(self.sandbox))
        self.steps.append(record)
        return record

    def text(self, record):
        return (self.dir / record["stdout"]).read_text(errors="replace") + \
            (self.dir / record["stderr"]).read_text(errors="replace")

    def finish(self, status, **details):
        self.status, self.details = status, details
        if self.board.dry_run:
            return
        self.dir.mkdir(parents=True, exist_ok=True)
        leftovers = sorted(os.listdir(self.sandbox)) \
            if self.sandbox and self.sandbox.is_dir() else []
        (self.dir / "run.json").write_text(json.dumps(dict(
            action=self.action, subject=self.subject, status=status,
            utc=utc_iso(), perf_event_paranoid=_paranoid(),
            locks=self.locks, sandbox_leftovers=leftovers,
            steps=self.steps,
            head=_git(self.board.repo, "rev-parse", "HEAD").stdout.strip(),
            **details), indent=1, default=str) + "\n")
        write_sums(self.dir)


def _paranoid():
    try:
        return Path("/proc/sys/kernel/perf_event_paranoid").read_text().strip()
    except OSError:
        return None


class Board:
    def __init__(self, state_dir=DEFAULT_STATE, *, repo=ROOT,
                 executor=run_bounded, rebuilder=None, dry_run=False,
                 expected=None, lock_path=LOCK_PATH,
                 host_lock=host_lock_path,
                 process_scan=board_tool_processes,
                 verify_tools=verify_tooling, journal=journal_window):
        self.state = Path(state_dir)
        self.repo = Path(repo)
        self.executor = executor
        self.dry_run = dry_run
        self.expected = expected or load_expected_identity()
        self.serial = require_serial(self.expected["serial"])
        self.lock_path = Path(lock_path)
        self.host_lock = host_lock  # resolved per session
        self.process_scan = process_scan
        self.verify_tools = verify_tools
        self.journal = journal
        self.rebuilder = rebuilder or (lambda name: rebuild(
            name, self.state / "verify" / utc_stamp()))

    def preflight(self):
        refusals = list(self.verify_tools())
        others = self.process_scan()
        if others:
            refusals.append("board tool already running: %s" % ", ".join(
                "pid %d %s" % (p["pid"], " ".join(p["argv"][:2]))
                for p in others))
        if refusals:
            raise PolicyError("; ".join(refusals))

    @contextmanager
    def session(self, action, subject=None):
        session = Session(self, action, subject)
        if self.dry_run:
            yield session
            return
        host = Path(self.host_lock())
        with BoardLock(host, action), BoardLock(self.lock_path, action):
            session.locks = [str(host), str(self.lock_path)]
            session.sandbox = Path(tempfile.mkdtemp(prefix="f439-sandbox-"))
            try:
                self.preflight()
                yield session
            except BaseException as error:
                terminate_live_children()
                # Keep a status the action already recorded (for example
                # termios-changed) together with its details.
                session.finish("refused-or-failed"
                               if session.status in ("started", "completed")
                               else session.status,
                               **dict(session.details, error=str(error)))
                raise
            finally:
                terminate_live_children()
                try:
                    session.sandbox.rmdir()
                except OSError:
                    pass  # not empty: kept and listed in run.json
            if session.status == "started":
                session.finish("completed")

    def _identity(self, session, extra=(), probe=False, approved_reads=()):
        """One read-only openocd attach (and st-info --hot-plug if probe).
        Nothing is written to target RAM or peripherals by these commands;
        the attach itself still programs debug registers (see PLAN.md)."""
        probed = session.run([ST_INFO, "--probe", "--hot-plug"],
                             timeout_s=TIMEOUT_ST_INFO,
                             label="st-info-probe") if probe else None
        read = session.run(openocd_identity_command(self.serial, extra),
                           timeout_s=TIMEOUT_DUMP if any(
                               e.startswith("dump_image") for e in extra)
                           else TIMEOUT_OPENOCD, label="identity",
                           approved_reads=approved_reads)
        if self.dry_run:
            return None, None
        observed = identity_from_outputs(
            session.text(probed) if probed else "", session.text(read))
        problems = identity_mismatches(observed, self.expected,
                                       check_probe=probe)
        if read["timed_out"] or read["returncode"] != 0:
            problems.append("openocd identity read failed")
        if problems:
            raise PolicyError("identity mismatch: " + "; ".join(problems))
        return observed, session.text(read)

    def offline_gate(self, name, image_dir):
        image_dir = Path(image_dir)
        manifest = json.loads((image_dir / "manifest.json").read_text())
        if manifest.get("image") != name:
            raise PolicyError("manifest describes a different image")
        elf = image_dir / str(manifest.get("elf", ""))
        refusals = []
        # Read once: the digest, the ELF check and the image returned for
        # the readback comparison all use these bytes, so a file swapped
        # after hashing cannot produce a false readback alarm.
        data = elf.read_bytes() if elf.is_file() else None
        if data is None or hashlib.sha256(data).hexdigest() != manifest.get(
                "elf_sha256"):
            refusals.append("ELF digest differs from its build manifest")
        else:
            refusals += check_elf(data)["refusals"]
        refusals += verify_sources({p: None for p in SELF_FILES}, self.repo)
        if not refusals:
            refusals += verify_provenance(manifest, self.repo, image_dir,
                                          self.rebuilder(name))
        if refusals:
            raise PolicyError("offline gate refused %s: %s"
                              % (name, "; ".join(refusals)))
        return elf.resolve(), manifest, parse_elf(data)

    def _session_copy(self, session, source, manifest):
        """Copy the gated ELF into the locked session and re-check it."""
        copy = session.dir / "image.elf"
        if self.dry_run:
            return copy
        session.dir.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, copy)
        if sha256_file(copy) != manifest["elf_sha256"] or \
                check_elf(copy.read_bytes())["refusals"]:
            raise PolicyError("session copy of the ELF failed re-checking")
        return copy

    def _program(self, session, copy, connect_under_reset=False):
        record = session.run(openocd_program_command(
            self.serial, copy, connect_under_reset),
            timeout_s=TIMEOUT_PROGRAM, label="program", approved_elf=copy)
        if not self.dry_run and (
                record["timed_out"] or record["returncode"] != 0 or
                "** Verified OK **" not in session.text(record)):
            raise PolicyError("program/verify failed; do not retry, run "
                              "`recover` after review")

    def _readback(self, session, image):
        """Full identity attach plus a flash dump compared in Python with the
        ELF load bytes, independently of openocd's own verify."""
        start, size = load_extent(image)
        target = session.dir / "readback.bin"
        observed, _ = self._identity(session, extra=[
            "dump_image {%s} 0x%08x 0x%x" % (target, start, size)])
        if self.dry_run:
            return None, None
        data = target.read_bytes() if target.is_file() else b""
        differences = compare_readback(image, start, data)
        if len(data) != size or differences:
            raise PolicyError("flash readback differs from the ELF load "
                              "image: %s" % (differences or "short read"))
        return observed, dict(start=start, size=size,
                              sha256=hashlib.sha256(data).hexdigest())

    def flash(self, name, image_dir):
        source, manifest, image = self.offline_gate(name, image_dir)
        with self.session("flash", name) as session:
            before, _ = self._identity(session)
            copy = self._session_copy(session, source, manifest)
            self._program(session, copy)
            after, readback = self._readback(session, image)
            if not self.dry_run and before["optcr1"] != after["optcr1"]:
                raise PolicyError("FLASH_OPTCR1 changed: %s -> %s" % (
                    _hex32(before["optcr1"]), _hex32(after["optcr1"])))
            session.finish("completed", image=name,
                           elf_sha256=manifest["elf_sha256"],
                           identity_before=before, identity_after=after,
                           readback=readback, readback_match=True)
            return dict(run_dir=str(session.dir), planned=session.planned)

    def optcr1_baseline(self):
        """FLASH_OPTCR1 recorded by the first `probe` in this state
        directory (the identity receipt predates the OPTCR1 read)."""
        path = self.state / PROBE_BASELINE
        try:
            record = json.loads(path.read_text())
        except (OSError, ValueError) as error:
            raise PolicyError("no usable FLASH_OPTCR1 baseline %s (%s); run "
                              "`probe` first" % (path, error)) from None
        if not isinstance(record, dict) or \
                record.get("schema") != PROBE_BASELINE_SCHEMA or \
                record.get("serial") != self.serial or \
                type(record.get("optcr1")) is not int:
            raise PolicyError("FLASH_OPTCR1 baseline %s is not a probe "
                              "record for this board" % path)
        return record

    def _check_baseline(self, observed, session):
        """First probe: record FLASH_OPTCR1. Later probes: refuse a change.
        A recorded baseline is never overwritten."""
        if observed["optcr1"] is None:
            raise PolicyError("FLASH_OPTCR1 was not read; no baseline")
        path = self.state / PROBE_BASELINE
        if path.exists() or path.is_symlink():
            baseline = self.optcr1_baseline()
            if baseline["optcr1"] != observed["optcr1"]:
                raise PolicyError(
                    "FLASH_OPTCR1 differs from the baseline recorded by probe "
                    "at %s: %s -> %s" % (baseline.get("utc"),
                                         _hex32(baseline["optcr1"]),
                                         _hex32(observed["optcr1"])))
            return baseline
        baseline = dict(schema=PROBE_BASELINE_SCHEMA, serial=self.serial,
                        optcr1=observed["optcr1"], optcr=observed["optcr"],
                        utc=utc_iso(), run_dir=str(session.dir))
        path.parent.mkdir(parents=True, exist_ok=True)
        with open(path, "x") as handle:
            handle.write(json.dumps(baseline, indent=1) + "\n")
        return baseline

    def recover(self, name, image_dir):
        """For a target that cannot be attached normally (Stop or WFI with
        the debug clocks off): connect under reset and check only the
        debug-domain DBGMCU_IDCODE, program, then run the full identity and
        compare FLASH_OPTCR1 with the baseline `probe` recorded."""
        source, manifest, image = self.offline_gate(name, image_dir)
        baseline = self.optcr1_baseline()
        with self.session("recover", name) as session:
            record = session.run(openocd_idcode_command(self.serial),
                                 timeout_s=TIMEOUT_OPENOCD, label="idcode")
            if not self.dry_run:
                idcode = _first(read_words(parse_openocd_reads(
                    session.text(record)), 0xE0042000, 1))
                if record["returncode"] != 0 or idcode is None or \
                        idcode & 0xFFF != self.expected["dev_id"] or \
                        idcode >> 16 != self.expected["rev_id"]:
                    raise PolicyError("DBGMCU_IDCODE under reset does not "
                                      "match the identity receipt")
            copy = self._session_copy(session, source, manifest)
            self._program(session, copy, connect_under_reset=True)
            after, readback = self._readback(session, image)
            if not self.dry_run and after["optcr1"] != baseline["optcr1"]:
                raise PolicyError(
                    "FLASH_OPTCR1 changed: %s -> %s (baseline recorded by "
                    "probe at %s)" % (_hex32(baseline["optcr1"]),
                                      _hex32(after["optcr1"]),
                                      baseline.get("utc")))
            session.finish("completed", image=name,
                           elf_sha256=manifest["elf_sha256"],
                           identity_after=after, readback=readback,
                           readback_match=True, optcr1_baseline=baseline)
            return dict(run_dir=str(session.dir), planned=session.planned)

    def probe(self):
        with self.session("probe") as session:
            observed, _ = self._identity(session, probe=True)
            baseline = None if self.dry_run else \
                self._check_baseline(observed, session)
            session.finish("completed", identity=observed,
                           optcr1_baseline=baseline)
            return dict(run_dir=str(session.dir), planned=session.planned)

    def read(self, name, image_dir, symbols=(), registers=(), vectors=0):
        """SWD reads without reset or halt: allowlisted registers, ELF
        symbols in RAM or flash, and optionally N vector words compared
        with the ELF."""
        image = parse_elf((Path(image_dir) / "image.elf").read_bytes())
        reads, approved = [], []
        if vectors:
            if not 0 < vectors <= VECTOR_COUNT:
                raise PolicyError("vector count outside 1..%d" % VECTOR_COUNT)
            reads.append("mdw 0x08000000 %d" % vectors)
        for symbol in symbols:
            if symbol not in image.symbols:
                raise PolicyError("unknown symbol: " + symbol)
            address, size = image.symbols[symbol]
            words = max(1, (size + 3) // 4)
            if words > 128:
                raise PolicyError("symbol too large to read: " + symbol)
            approved.append((address, 4 * words))
            reads.append("mdw 0x%08x %d" % (address, words) if words > 1
                         else "mdw 0x%08x" % address)
        for register in registers:
            if register not in READ_REGISTERS:
                raise PolicyError("register not in the read allowlist: "
                                  + register)
            reads.append("mdw 0x%08x" % READ_REGISTERS[register])
        with self.session("read", name) as session:
            observed, text = self._identity(session, extra=reads,
                                            approved_reads=approved)
            blocks = parse_openocd_reads(text or "")
            match = None
            if vectors and not self.dry_run:
                expected = image.read_flash(MAIN_FLASH.start, 4 * vectors)
                seen = read_words(blocks, MAIN_FLASH.start, vectors)
                match = (expected is not None and seen is not None and
                         list(struct.unpack("<%dI" % vectors, expected)) ==
                         seen)
            session.finish("completed", image=name, symbols=list(symbols),
                           registers=list(registers), identity=observed,
                           blocks=blocks, vectors_match_elf=match)
            return dict(run_dir=str(session.dir), planned=session.planned)

    def reset_run(self):
        with self.session("reset") as session:
            self._identity(session)
            record = session.run(openocd_reset_command(self.serial),
                                 timeout_s=TIMEOUT_OPENOCD, label="reset-run")
            if not self.dry_run and record["returncode"] != 0:
                raise PolicyError("reset failed")
            return dict(run_dir=str(session.dir), planned=session.planned)

    def dump_flash(self, size=0x4000):
        with self.session("dump") as session:
            self._identity(session, extra=[
                "dump_image {%s} 0x08000000 0x%x"
                % (session.dir / "readback.bin", size)])
            return dict(run_dir=str(session.dir), planned=session.planned)

    def selfcheck(self):
        """openocd parses its configuration without `init` and prints both
        event-handler bodies between markers; both must be empty."""
        argv = openocd_selfcheck_command(self.serial)
        with self.session("selfcheck") as session:
            record = session.run(argv, timeout_s=TIMEOUT_OPENOCD,
                                 label="selfcheck")
            if self.dry_run:
                return dict(planned=session.planned)
            lines = session.text(record).splitlines()
            if record["returncode"] != 0 or \
                    "F439_EXAMINE_END=<>" not in lines or \
                    "F439_RESET_INIT=<>" not in lines:
                raise PolicyError("event-handler override not confirmed; "
                                  "see %s" % session.dir)
            session.finish("completed", lines=lines)
            return dict(run_dir=str(session.dir), output=lines)

    def capture(self, label, *, seconds, max_bytes, baud, parity="none",
                sends=(), device=None, reset_at=None, allow_any_device=False):
        """Bounded VCP capture; optionally one debugger `reset run` at
        `reset_at` seconds after the capture starts (identity checked first).
        allow_any_device exists for tests with a pseudo-terminal only."""
        require_label(label)
        device = Path(device) if device else serial_device_for(self.serial)
        if reset_at is not None and not (
                0 <= reset_at and reset_at + RESET_TAIL_S <= seconds):
            raise PolicyError("reset offset must leave >= %.1f s of capture"
                              % RESET_TAIL_S)
        if self.dry_run:
            planned = [dict(capture=str(device), seconds=seconds,
                            max_bytes=max_bytes, baud=baud, parity=parity,
                            sends=[(t, b.hex()) for t, b in sends])]
            with self.session("capture", label) as session:
                if reset_at is not None:
                    self._identity(session)
                    session.run(openocd_reset_command(self.serial),
                                timeout_s=TIMEOUT_OPENOCD, label="reset-run")
                return dict(planned=planned + session.planned)
        real = Path(os.path.realpath(device))
        if not allow_any_device and \
                not re.fullmatch(r"/dev/ttyACM\d+", str(real)):
            raise PolicyError("serial device does not resolve to ttyACM: "
                              + str(real))
        with self.session("capture", label) as session:
            session.dir.mkdir(parents=True, exist_ok=True)
            reset, threads = {}, []
            if reset_at is not None:
                self._identity(session)

            def start_reset(start_ns):
                def worker():
                    delay = start_ns / 1e9 + reset_at - time.monotonic()
                    time.sleep(max(0.0, delay))
                    reset["start_offset_s"] = \
                        (time.monotonic_ns() - start_ns) / 1e9
                    try:
                        record = session.run(
                            openocd_reset_command(self.serial),
                            timeout_s=TIMEOUT_OPENOCD, label="reset-run")
                        reset["returncode"] = record["returncode"]
                        reset["timed_out"] = record["timed_out"]
                    except PolicyError as error:
                        reset["refused"] = str(error)
                    reset["end_offset_s"] = \
                        (time.monotonic_ns() - start_ns) / 1e9
                thread = threading.Thread(target=worker, daemon=True)
                thread.start()
                threads.append(thread)

            try:
                summary = capture_serial(
                    device, seconds=seconds, max_bytes=max_bytes, baud=baud,
                    parity=parity, out_dir=session.dir, sends=sends,
                    label=label,
                    on_start=start_reset if reset_at is not None else None)
            finally:
                for thread in threads:
                    thread.join(TIMEOUT_OPENOCD + 5)
            summary["modemmanager_journal"] = self.journal(
                summary["start_epoch"], summary["end_epoch"], session.dir)
            if reset_at is not None:
                summary["reset"] = reset
                if reset.get("returncode") != 0:
                    session.finish("reset-failed", capture=summary)
                    raise PolicyError("debugger reset during capture failed")
            if not summary["termios_unchanged"]:
                session.finish("termios-changed", capture=summary)
                raise PolicyError("line settings changed during the capture "
                                  "(another process?); data kept in %s"
                                  % session.dir)
            session.finish("completed", capture=summary,
                           resolved_device=str(real))
            return dict(run_dir=str(session.dir), summary=summary)


# --------------------------------------------------------------------------
# Pre-registered analysis. Every result is a derived estimate computed from
# the raw capture; it never replaces the raw bytes and their timestamps.
# --------------------------------------------------------------------------
FIELD = re.compile(r"([a-z_0-9]+)=(0x[0-9a-fA-F]+|[0-9]+)")


def capture_events(raw_path):
    raw_path = Path(raw_path)
    index_path = raw_path.with_name(raw_path.name[:-len(".bin")] +
                                    ".index.jsonl")
    rx, tx = [], []
    for line in index_path.read_text().splitlines():
        event = json.loads(line)
        (rx if event["dir"] == "rx" else tx).append(event)
    return raw_path.read_bytes(), rx, tx


def capture_lines(raw_path):
    """BF1 records with the host arrival time of each line's final byte."""
    data, rx, _ = capture_events(raw_path)
    ends = [(e["offset"] + e["len"], e["t_ns"]) for e in rx]
    lines, start, chunk = [], 0, 0
    for match in re.finditer(rb"\r\n", data):
        while chunk < len(ends) and ends[chunk][0] < match.end():
            chunk += 1
        text = data[start:match.start()]
        start = match.end()
        at = text.rfind(b"BF1 ")
        if at < 0 or chunk >= len(ends):
            continue
        body = text[at + 4:].decode("ascii", "replace")
        tokens = body.split(" ")
        tag = []
        for token in tokens:
            if "=" in token or not token.isupper():
                break
            tag.append(token)
        fields = {k: int(v, 0) for k, v in FIELD.findall(body)}
        lines.append(dict(t_s=ends[chunk][1] / 1e9, tag=" ".join(tag),
                          fields=fields, text=body))
    return lines


def fit_ticks(lines, *, reload, divider, probe_every=2):
    """Least-squares host seconds per SysTick tick -> core clock estimate.

    Model: tick period = divider * (reload + 1) / f_core. Host timestamps
    include UART and USB delivery jitter; residuals bound that jitter.
    """
    ticks = [l for l in lines if l["tag"] == "T" and "n" in l["fields"]]
    xs = [l["fields"]["n"] for l in ticks]
    ys = [l["t_s"] for l in ticks]
    count = len(xs)
    if count < 3:
        return dict(kind="derived-estimate", ticks=count,
                    status="inconclusive: fewer than 3 tick lines")
    mean_x, mean_y = sum(xs) / count, sum(ys) / count
    sxx = sum((x - mean_x) ** 2 for x in xs)
    slope = sum((x - mean_x) * (y - mean_y) for x, y in zip(xs, ys)) / sxx
    residuals = [y - (mean_y + slope * (x - mean_x)) for x, y in zip(xs, ys)]
    # A sleep probe runs after each tick whose number is a multiple of
    # probe_every, so the next interval may contain sleep. "Clean" deltas
    # come from intervals that ended on a probe-free multiple.
    usable = [l for l in ticks[1:] if l["fields"].get("skipped") == 0]
    deltas = [l["fields"]["d"] for l in usable
              if l["fields"]["n"] % probe_every == 0]
    after_probe = [l["fields"]["d"] for l in usable
                   if l["fields"]["n"] % probe_every != 0]
    return dict(kind="derived-estimate", ticks=count, slope_s_per_tick=slope,
                derived_core_hz=divider * (reload + 1) / slope,
                residual_rms_s=(sum(r * r for r in residuals) / count) ** 0.5,
                residual_max_s=max(abs(r) for r in residuals),
                clean_ticks=len(deltas),
                cyccnt_delta_min=min(deltas) if deltas else None,
                cyccnt_delta_max=max(deltas) if deltas else None,
                after_probe_delta_min=min(after_probe) if after_probe
                else None,
                after_probe_delta_max=max(after_probe) if after_probe
                else None,
                model="period = %d*(%d+1)/f_core" % (divider, reload))


def starvation_intervals(lines, *, reload=511, prescaler=256):
    """Last fed window (feed=0, CS_OK) to the next BOOT line."""
    results, last_fed, starving = [], None, 0
    for line in lines:
        if line["tag"] == "W" and "feed" in line["fields"]:
            if line["fields"]["feed"] == 0:
                last_fed, starving = line, 0
            else:
                starving += 1
        elif line["tag"] == "BOOT" and last_fed is not None:
            interval = line["t_s"] - last_fed["t_s"]
            csr = line["fields"].get("csr", 0)
            results.append(dict(
                kind="derived-estimate", last_fed_window=last_fed["fields"]
                .get("win"), unfed_windows=starving, interval_s=interval,
                csr=csr, iwdgrstf=bool(csr >> 29 & 1),
                derived_lsi_hz=(reload + 1) * prescaler / interval,
                model="timeout = (RLR+1)*prescaler/f_LSI; interval also "
                      "contains line-print and USB delivery offsets"))
            last_fed, starving = None, 0
    return results


def stop_cycles(lines, raw_path):
    """Pair each S ENTER line with the next wake byte and its S WOKE line."""
    _, _, tx = capture_events(raw_path)
    woke = {l["fields"].get("n"): l for l in lines if l["tag"] == "S WOKE"}
    results = []
    for enter in (l for l in lines if l["tag"] == "S ENTER"):
        number = enter["fields"].get("n")
        wake = next((e for e in tx if e["t_ns"] / 1e9 > enter["t_s"]), None)
        report = woke.get(number)
        if wake is None or report is None:
            results.append(dict(n=number, status="inconclusive: no wake "
                                "byte or no WOKE line"))
            continue
        host = wake["t_ns"] / 1e9 - enter["t_s"]
        cycles = report["fields"].get("d")
        entered = report["fields"].get("entered")
        finished = report["fields"].get("finished")
        after = report["t_s"] > wake["t_ns"] / 1e9
        # P4.8 is interpreted only for cycles that satisfy P4.7.
        valid = entered in (0, 255) and finished in (0, 255) and after
        status = "valid" if valid else (
            "inconclusive: stop not entered (entered=%s finished=%s)"
            % (entered, finished) if entered not in (0, 255) or
            finished not in (0, 255) else
            "inconclusive: WOKE line arrived before the wake byte")
        results.append(dict(kind="derived-estimate", n=number,
                            valid=valid, status=status,
                            woke_after_wake_byte=after,
                            host_stop_s=host, target_cycles=cycles,
                            cycles_per_host_s=cycles / host if host > 0
                            else None, entered=entered, finished=finished))
    return results


def probing_state(lines):
    """Clock image: `E STATE` lines in arrival order (raw values).

    `stopped` holds only if the last `E STATE` line reports probing=0 and no
    sleep-probe `E` line follows it, i.e. the image itself said it stopped
    probing (DLE `Q`) and a later attach cannot meet a core asleep with
    DBG_SLEEP cleared. No `E STATE` line at all is not a confirmation.
    """
    states = [i for i, line in enumerate(lines) if line["tag"] == "E STATE"]
    final = lines[states[-1]]["fields"].get("probing") if states else None
    late = [line for line in lines[states[-1] + 1:] if line["tag"] == "E"] \
        if states else []
    return dict(kind="raw", states=[dict(t_s=lines[i]["t_s"],
                                         probing=lines[i]["fields"]
                                         .get("probing"),
                                         dbg=lines[i]["fields"].get("dbg"))
                                    for i in states],
                final_probing=final, probes_after_final_state=len(late),
                stopped=final == 0 and not late)


def uart_summary(raw_path):
    data, rx, _ = capture_events(raw_path)
    lines = capture_lines(raw_path)
    summary = dict(kind="raw-count", bytes=len(data), bf1_lines=len(lines),
                   tags=sorted({l["tag"] for l in lines}),
                   rx=[dict(t_s=l["t_s"], v=l["fields"].get("v"),
                            e=l["fields"].get("e"))
                       for l in lines if l["tag"] == "RX"],
                   printable_fraction=(sum(32 <= b < 127 or b in (10, 13)
                                           for b in data) / len(data))
                   if data else None)
    runs = list(re.finditer(rb"U{1024,}", data))
    if runs:
        run = max(runs, key=lambda m: m.end() - m.start())

        def time_at(offset):
            return next(e["t_ns"] for e in rx
                        if e["offset"] <= offset < e["offset"] + e["len"])

        reported = [l for l in lines if l["tag"] == "BURST"]
        summary["burst"] = dict(
            kind="derived-estimate", bytes=run.end() - run.start(),
            host_span_s=(time_at(run.end() - 1) - time_at(run.start())) / 1e9,
            target_cycles=reported[0]["fields"].get("cycles")
            if reported else None)
    return summary


# --------------------------------------------------------------------------
# Command line
# --------------------------------------------------------------------------
def _parse_send(text):
    offset, _, payload = text.partition(":")
    return float(offset), bytes.fromhex(payload)


def _stop_on_signals():
    """SIGTERM/SIGHUP unwind like Ctrl-C, so session cleanup terminates
    openocd children before the lock is released."""
    def handler(signum, frame):
        raise SystemExit(128 + signum)
    for sig in (signal.SIGTERM, signal.SIGHUP):
        signal.signal(sig, handler)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--state", type=Path, default=DEFAULT_STATE,
                        help="lock, runs and verification builds")
    parser.add_argument("--images", type=Path,
                        default=DEFAULT_STATE / "images")
    parser.add_argument("--dry-run", action="store_true",
                        help="check policy and print commands only")
    sub = parser.add_subparsers(dest="command", required=True)
    build = sub.add_parser("build", help="offline: build and check images")
    build.add_argument("names", nargs="*")
    check = sub.add_parser("check-elf", help="offline: ELF safety check")
    check.add_argument("elf", type=Path)
    sub.add_parser("list", help="offline: list registered images")
    sub.add_parser("selfcheck-openocd",
                   help="parse openocd configuration without init")
    sub.add_parser("probe", help="identity: st-info --hot-plug + openocd")
    flash = sub.add_parser("flash", help="program one registered image")
    flash.add_argument("name", choices=sorted(IMAGES))
    read = sub.add_parser("read", help="read symbols/registers, no reset")
    read.add_argument("name", choices=sorted(IMAGES))
    read.add_argument("--symbol", action="append", default=[])
    read.add_argument("--register", action="append", default=[])
    read.add_argument("--vectors", type=int, default=0, metavar="WORDS",
                      help="read WORDS at 0x08000000 and compare with ELF")
    sub.add_parser("reset", help="debugger reset (reset run)")
    sub.add_parser("dump-flash", help="read the first 16 KiB of flash")
    capture = sub.add_parser("capture", help="bounded VCP capture")
    capture.add_argument("label")
    capture.add_argument("--seconds", type=float, required=True)
    capture.add_argument("--max-bytes", type=int, required=True)
    capture.add_argument("--baud", type=int, default=115200)
    capture.add_argument("--parity", default="none",
                         choices=["none", "even", "odd"])
    capture.add_argument("--send", action="append", default=[],
                         type=_parse_send, metavar="SECONDS:HEX")
    capture.add_argument("--reset-at", type=float, default=None,
                         help="one debugger `reset run` at this offset")
    analyze = sub.add_parser(
        "analyze", help="offline: derived estimates; `probing` exits 1 "
        "unless the last E STATE line reports probing=0")
    analyze.add_argument("kind", choices=["lines", "ticks", "starvation",
                                          "stop", "uart", "probing"])
    analyze.add_argument("capture", type=Path, help="<label>.bin")
    analyze.add_argument("--reload", type=int, default=15_999_999)
    analyze.add_argument("--divider", type=int, default=1)
    sub.add_parser("park", help="flash the park image (known state)")
    sub.add_parser("recover", help="connect under reset, then park")
    args = parser.parse_args(argv)
    try:
        if args.command == "build":
            names = args.names or sorted(IMAGES)
            for name in names:
                build_image(name, args.images)
            return 0
        if args.command == "check-elf":
            report = check_elf(args.elf.read_bytes())
            print(json.dumps(report, indent=1))
            return 0 if report["ok"] else 1
        if args.command == "analyze":
            lines = capture_lines(args.capture)
            result = {"lines": lambda: lines,
                      "ticks": lambda: fit_ticks(lines, reload=args.reload,
                                                 divider=args.divider),
                      "starvation": lambda: starvation_intervals(lines),
                      "stop": lambda: stop_cycles(lines, args.capture),
                      "uart": lambda: uart_summary(args.capture),
                      "probing": lambda: probing_state(lines)}[args.kind]()
            print(json.dumps(result, indent=1))
            return 1 if args.kind == "probing" and not result["stopped"] \
                else 0
        if args.command == "list":
            for name, spec in IMAGES.items():
                print("%-26s %-6s %s" % (name, spec["experiment"],
                                         spec["purpose"]))
            return 0
        _stop_on_signals()
        board = Board(args.state, dry_run=args.dry_run)
        if args.command == "selfcheck-openocd":
            result = board.selfcheck()
        elif args.command == "probe":
            result = board.probe()
        elif args.command == "flash":
            result = board.flash(args.name, args.images / args.name)
        elif args.command == "read":
            result = board.read(args.name, args.images / args.name,
                                args.symbol, args.register, args.vectors)
        elif args.command == "reset":
            result = board.reset_run()
        elif args.command == "dump-flash":
            result = board.dump_flash()
        elif args.command == "capture":
            result = board.capture(args.label, seconds=args.seconds,
                                   max_bytes=args.max_bytes, baud=args.baud,
                                   parity=args.parity, sends=args.send,
                                   reset_at=args.reset_at)
        elif args.command == "park":
            result = board.flash(PARK_IMAGE, args.images / PARK_IMAGE)
        else:
            result = board.recover(PARK_IMAGE, args.images / PARK_IMAGE)
        print(json.dumps(result, indent=1, default=str))
        return 0
    except (PolicyError, OSError, ValueError) as error:
        print("REFUSED: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
