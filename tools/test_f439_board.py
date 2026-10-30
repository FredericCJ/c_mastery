"""Deterministic tests for the NUCLEO-F439ZI safety layer (tools/f439_board.py).

No test touches hardware: ELF images are synthesized in memory, board tools are
replaced by a fake executor, serial capture uses a pseudo-terminal pair and
provenance checks use a temporary Git repository.

Run: python3 tools/test_f439_board.py
Mutation run: python3 tools/test_f439_board.py --mutants [--jobs N]
(each entry of MUTANTS weakens one rule of tools/f439_board.py in an in-memory
copy; the suite must fail for every one of them; see the end of this file).
"""
from concurrent.futures import ThreadPoolExecutor
import contextlib
from pathlib import Path
import errno
import hashlib
import importlib.util
import io
import json
import os
import re
import shutil
import signal
import struct
import subprocess
import sys
import tempfile
import termios
import threading
import time
import types
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f439_board as fb  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
IDENTITY_DIR = ROOT / "reviews/v5/expansion/polish_revision/physical/identity"
SERIAL = "0669FF485753667187124334"
OPTCR1_LINE = "0x40023c18: 0fff0000 \n"

# Programs no test may start, whatever a (mutated) module under test does.
# Kept independent of fb.BOARD_TOOL_NAMES so a mutant cannot disable it.
GUARDED_PROGRAMS = {"openocd", "st-info", "st-flash", "st-util", "st-trace",
                    "STM32_Programmer_CLI", "gdb", "gdb-multiarch",
                    "arm-none-eabi-gdb", "pyocd", "dfu-util"}


def starts_board_tool(args):
    """True if a Popen of `args` would execute a guarded program."""
    program = str(args[0] if isinstance(args, (list, tuple)) else args)
    path = program if os.sep in program else shutil.which(program)
    return path is not None and \
        os.path.basename(os.path.realpath(path)) in GUARDED_PROGRAMS


class NoBoardToolPopen(subprocess.Popen):
    def __init__(self, args, *rest, **kwargs):
        if starts_board_tool(args):
            raise RuntimeError("a test tried to start a board tool: %r"
                               % (args,))
        super().__init__(args, *rest, **kwargs)


if subprocess.Popen.__name__ != "NoBoardToolPopen":  # once per process
    subprocess.Popen = NoBoardToolPopen

SHT_PROGBITS = 1
SHT_NOBITS = 8
SHF_WRITE = 1
SHF_ALLOC = 2
SHF_EXECINSTR = 4
PT_LOAD = 1


def make_elf(sections, segments, entry=0x08000101, machine=40, ei_class=1,
             ei_data=1, symbols=()):
    """Build a minimal ELF32 image.

    sections: dicts with name, type, flags, addr and either data (PROGBITS) or
    size (NOBITS). segments: dicts with section (name or None), paddr, vaddr and
    optional memsz/filesz. symbols: (name, value, size) tuples.
    """
    header_size, phent, shent = 52, 32, 40
    blobs, offsets = bytearray(), {}
    data_start = header_size + phent * len(segments)
    for section in sections:
        if section["type"] == SHT_NOBITS:
            offsets[section["name"]] = data_start + len(blobs)
            continue
        while (data_start + len(blobs)) % 4:
            blobs.append(0)
        offsets[section["name"]] = data_start + len(blobs)
        blobs += section["data"]
    strtab = bytearray(b"\0")
    symtab = bytearray(16)
    for name, value, size in symbols:
        symtab += struct.pack("<IIIBBH", len(strtab), value, size, 0x11, 0, 1)
        strtab += name.encode() + b"\0"
    names = [s["name"] for s in sections] + [".symtab", ".strtab", ".shstrtab"]
    shstrtab, name_offsets = bytearray(b"\0"), {}
    for name in names:
        name_offsets[name] = len(shstrtab)
        shstrtab += name.encode() + b"\0"
    extra = {}
    for name, blob in ((".symtab", symtab), (".strtab", strtab),
                       (".shstrtab", shstrtab)):
        while (data_start + len(blobs)) % 4:
            blobs.append(0)
        extra[name] = (data_start + len(blobs), len(blob))
        blobs += blob
    while (data_start + len(blobs)) % 4:
        blobs.append(0)
    shoff = data_start + len(blobs)
    shdrs = bytearray(shent)
    for section in sections:
        size = section.get("size", len(section.get("data", b"")))
        shdrs += struct.pack("<IIIIIIIIII", name_offsets[section["name"]],
                             section["type"], section["flags"],
                             section["addr"], offsets[section["name"]], size,
                             0, 0, 4, 0)
    strtab_index = len(sections) + 2
    shdrs += struct.pack("<IIIIIIIIII", name_offsets[".symtab"], 2, 0, 0,
                         extra[".symtab"][0], extra[".symtab"][1],
                         strtab_index, 1, 4, 16)
    shdrs += struct.pack("<IIIIIIIIII", name_offsets[".strtab"], 3, 0, 0,
                         extra[".strtab"][0], extra[".strtab"][1], 0, 0, 1, 0)
    shdrs += struct.pack("<IIIIIIIIII", name_offsets[".shstrtab"], 3, 0, 0,
                         extra[".shstrtab"][0], extra[".shstrtab"][1], 0, 0,
                         1, 0)
    phdrs = bytearray()
    by_name = {s["name"]: s for s in sections}
    for segment in segments:
        section = by_name.get(segment.get("section"))
        if section is None or section["type"] == SHT_NOBITS:
            filesz, offset = segment.get("filesz", 0), 0
        else:
            filesz = segment.get("filesz", len(section["data"]))
            offset = offsets[section["name"]]
        memsz = segment.get("memsz", filesz)
        phdrs += struct.pack("<IIIIIIII", segment.get("type", PT_LOAD), offset,
                             segment["vaddr"], segment["paddr"], filesz, memsz,
                             5, 4)
    ident = bytes([0x7F, ord("E"), ord("L"), ord("F"), ei_class, ei_data, 1]
                  ) + bytes(9)
    header = struct.pack("<16sHHIIIIIHHHHHH", ident, 2, machine, 1, entry,
                         header_size, shoff, 0x05000200, header_size, phent,
                         len(segments), shent, len(sections) + 4,
                         len(sections) + 3)
    return bytes(header + phdrs + blobs + shdrs)


def vectors(stack=0x20020000, reset=0x08000101, count=16):
    words = [stack, reset] + [reset] * (count - 2)
    return struct.pack("<%dI" % count, *words)


def good_image(**overrides):
    text = vectors() + bytes(64)
    sections = [
        dict(name=".vectors", type=SHT_PROGBITS,
             flags=SHF_ALLOC | SHF_EXECINSTR, addr=0x08000000, data=text),
        dict(name=".data", type=SHT_PROGBITS, flags=SHF_ALLOC | SHF_WRITE,
             addr=0x20000000, data=b"\x78\x56\x34\x12"),
        dict(name=".bss", type=SHT_NOBITS, flags=SHF_ALLOC | SHF_WRITE,
             addr=0x20000004, size=16),
        dict(name=".noinit", type=SHT_NOBITS, flags=SHF_ALLOC | SHF_WRITE,
             addr=0x2001F000, size=64),
    ]
    segments = [
        dict(section=".vectors", paddr=0x08000000, vaddr=0x08000000),
        dict(section=".data", paddr=0x08000000 + len(text), vaddr=0x20000000),
        dict(section=".bss", paddr=0x20000004, vaddr=0x20000004, memsz=16),
    ]
    spec = dict(sections=sections, segments=segments,
                symbols=[("bf_stage", 0x2001F000, 4)])
    spec.update(overrides)
    return make_elf(**spec)


class ElfSafetyTests(unittest.TestCase):
    def refusals(self, data):
        return fb.check_elf(data)["refusals"]

    def assert_refused(self, data, needle):
        refusals = self.refusals(data)
        self.assertTrue(any(needle in r for r in refusals),
                        "expected %r in %r" % (needle, refusals))

    def test_valid_image_passes_and_reports_erase_span(self):
        report = fb.check_elf(good_image())
        self.assertEqual(report["refusals"], [])
        self.assertTrue(report["ok"])
        self.assertEqual(report["erase_sectors"], [0])
        self.assertEqual(report["vectors"]["initial_sp"], 0x20020000)
        self.assertEqual(report["vectors"]["reset"], 0x08000101)

    def flash_blob_at(self, address, size=16):
        image = good_image()
        parsed = fb.parse_elf(image)
        self.assertTrue(parsed.segments)
        text = vectors() + bytes(64)
        return make_elf(
            sections=[
                dict(name=".vectors", type=SHT_PROGBITS,
                     flags=SHF_ALLOC | SHF_EXECINSTR, addr=0x08000000,
                     data=text),
                dict(name=".evil", type=SHT_PROGBITS, flags=SHF_ALLOC,
                     addr=address, data=bytes(size)),
            ],
            segments=[
                dict(section=".vectors", paddr=0x08000000,
                     vaddr=0x08000000),
                dict(section=".evil", paddr=address, vaddr=address),
            ])

    def test_refuses_option_bytes_bank1(self):
        self.assert_refused(self.flash_blob_at(0x1FFFC000),
                            "option-bytes-bank1")

    def test_refuses_option_bytes_bank2(self):
        self.assert_refused(self.flash_blob_at(0x1FFEC000),
                            "option-bytes-bank2")

    def test_refuses_otp(self):
        self.assert_refused(self.flash_blob_at(0x1FFF7800), "otp")

    def test_refuses_system_memory(self):
        self.assert_refused(self.flash_blob_at(0x1FFF0000), "system-memory")

    def test_refuses_loadable_content_in_sram(self):
        self.assert_refused(self.flash_blob_at(0x20000000),
                            "loadable content outside main flash")

    def test_refuses_segment_straddling_flash_end(self):
        self.assert_refused(self.flash_blob_at(0x081FFF00, 0x200),
                            "loadable content outside main flash")

    def test_refuses_address_wraparound(self):
        self.assert_refused(self.flash_blob_at(0xFFFFFF00, 0x200),
                            "wraps")

    def test_refuses_wrapping_segment_without_any_section(self):
        image = make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000,
                           data=vectors()),
                      dict(name=".pad", type=SHT_PROGBITS, flags=0, addr=0,
                           data=bytes(0x400))],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000),
                      dict(section=None, paddr=0xFFFFFF00,
                           vaddr=0xFFFFFF00, filesz=0x200)])
        self.assertTrue(any("PT_LOAD" in r and "wraps" in r
                            for r in fb.check_elf(image)["refusals"]))

    def test_refuses_alloc_section_outside_every_region(self):
        sections = [
            dict(name=".vectors", type=SHT_PROGBITS,
                 flags=SHF_ALLOC | SHF_EXECINSTR, addr=0x08000000,
                 data=vectors()),
            dict(name=".fmc", type=SHT_NOBITS, flags=SHF_ALLOC | SHF_WRITE,
                 addr=0x60000000, size=4),
        ]
        image = make_elf(sections=sections, segments=[
            dict(section=".vectors", paddr=0x08000000, vaddr=0x08000000)])
        self.assert_refused(image, "section .fmc")

    def test_refuses_option_bytes_even_as_nobits_section(self):
        sections = [
            dict(name=".vectors", type=SHT_PROGBITS,
                 flags=SHF_ALLOC | SHF_EXECINSTR, addr=0x08000000,
                 data=vectors()),
            dict(name=".opt", type=SHT_NOBITS, flags=SHF_ALLOC,
                 addr=0x1FFFC000, size=16),
        ]
        image = make_elf(sections=sections, segments=[
            dict(section=".vectors", paddr=0x08000000, vaddr=0x08000000)])
        self.assert_refused(image, "option-bytes-bank1")

    def test_refuses_nobits_in_flash(self):
        sections = [
            dict(name=".vectors", type=SHT_PROGBITS,
                 flags=SHF_ALLOC | SHF_EXECINSTR, addr=0x08000000,
                 data=vectors()),
            dict(name=".hole", type=SHT_NOBITS, flags=SHF_ALLOC | SHF_WRITE,
                 addr=0x08001000, size=16),
        ]
        image = make_elf(sections=sections, segments=[
            dict(section=".vectors", paddr=0x08000000, vaddr=0x08000000)])
        self.assert_refused(image, "section .hole")

    def test_refuses_non_arm_machine(self):
        self.assert_refused(good_image() and make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000,
                           data=vectors())],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000)], machine=62), "machine")

    def test_refuses_big_endian_and_elf64_and_garbage(self):
        image = bytearray(good_image())
        image[5] = 2
        self.assert_refused(bytes(image), "little-endian")
        image = bytearray(good_image())
        image[4] = 2
        self.assert_refused(bytes(image), "ELF32")
        self.assert_refused(b"not an elf at all" * 4, "ELF magic")
        self.assert_refused(good_image()[:40], "truncated")

    def test_refuses_truncated_program_header_table(self):
        image = good_image()
        self.assert_refused(image[:60], "truncated")

    def test_refuses_entry_outside_flash(self):
        self.assert_refused(good_image() and make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000,
                           data=vectors())],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000)], entry=0x20000001), "entry")

    def single_vector_image(self, data):
        return make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000, data=data)],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000)])

    def test_refuses_even_reset_vector(self):
        self.assert_refused(
            self.single_vector_image(vectors(reset=0x08000100)), "Thumb")

    def test_refuses_reset_vector_outside_flash(self):
        self.assert_refused(
            self.single_vector_image(vectors(reset=0x20000101)),
            "reset vector 0x20000101 outside main flash")

    def test_refuses_segment_memory_image_outside_flash_and_ram(self):
        # Load bytes in flash, but the run-time copy targets the FMC window;
        # the section is not allocated, so only the PT_LOAD rule sees it.
        image = make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000,
                           data=vectors()),
                      dict(name=".fmc", type=SHT_PROGBITS, flags=0,
                           addr=0x60000000, data=bytes(16))],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000),
                      dict(section=".fmc", paddr=0x08001000,
                           vaddr=0x60000000)])
        self.assert_refused(image, "memory image outside flash/RAM")

    def test_refuses_segment_contents_beyond_the_file(self):
        image = bytearray(good_image())
        struct.pack_into("<II", image, 52 + 16, 0x10000, 0x10000)
        self.assert_refused(bytes(image), "truncated segment contents")

    def test_refuses_initial_stack_outside_ram(self):
        self.assert_refused(
            self.single_vector_image(vectors(stack=0x08001000)),
            "initial stack")

    def test_refuses_unaligned_initial_stack(self):
        self.assert_refused(
            self.single_vector_image(vectors(stack=0x2001FFFC)),
            "8-byte")

    def test_refuses_image_without_vector_table_at_flash_base(self):
        image = make_elf(
            sections=[dict(name=".text", type=SHT_PROGBITS, flags=SHF_ALLOC,
                           addr=0x08004000, data=vectors())],
            segments=[dict(section=".text", paddr=0x08004000,
                           vaddr=0x08004000)], entry=0x08004001)
        self.assert_refused(image, "vector table")

    def test_refuses_no_loadable_content(self):
        image = make_elf(sections=[], segments=[])
        self.assert_refused(image, "no loadable content")

    def test_symbols_are_parsed(self):
        parsed = fb.parse_elf(good_image())
        self.assertEqual(parsed.symbols["bf_stage"], (0x2001F000, 4))

    def test_erase_span_crosses_into_second_sector(self):
        text = vectors() + bytes(0x4000)
        image = make_elf(
            sections=[dict(name=".vectors", type=SHT_PROGBITS,
                           flags=SHF_ALLOC, addr=0x08000000, data=text)],
            segments=[dict(section=".vectors", paddr=0x08000000,
                           vaddr=0x08000000)])
        self.assertEqual(fb.check_elf(image)["erase_sectors"], [0, 1])


IDENTITY_CMD = fb.openocd_identity_command(SERIAL)
RUN_DIR = ROOT / "build/f439/runs/x"


class CommandPolicyTests(unittest.TestCase):
    def deny(self, argv, approved=None, reads=(), run_dir=RUN_DIR):
        with self.assertRaises(fb.PolicyError, msg=repr(argv)):
            fb.check_command(argv, approved_elf=approved, serial=SERIAL,
                             run_dir=run_dir, approved_reads=reads)

    def allow(self, argv, approved=None, reads=(), run_dir=RUN_DIR):
        fb.check_command(argv, approved_elf=approved, serial=SERIAL,
                         run_dir=run_dir, approved_reads=reads)

    def openocd(self, *commands):
        return fb._with_commands(fb.openocd_base(SERIAL), list(commands))

    def prefix(self, servers=fb.SERVERS_OFF, events=fb.EVENT_OFF,
               scripts=("-s", str(fb.OPENOCD_SCRIPTS)), board=fb.BOARD_CFG,
               tool=fb.OPENOCD):
        argv = [tool, *scripts, "-f", board, "-c", "adapter serial " + SERIAL]
        return fb._with_commands(argv, [*servers, *events])

    def test_denies_option_lock_protect_and_erase_commands(self):
        self.allow(self.openocd("init", "mdw 0xe0042000", "shutdown"))
        for command in [
            "stm32f2x options_write 0 0xaaec 0 0",
            "stm32f2x options_read 0",
            "stm32f2x optcr2_write 0 0",
            "stm32f2x unlock 0",
            "stm32f2x lock 0",
            "flash protect 0 0 last on",
            "stm32f2x mass_erase 0",
            "flash erase_sector 0 0 0",
            "flash erase_address 0x08000000 0x4000",
            "flash write_image erase unlock /tmp/x.elf",
            "flash write_bank 0 x.bin",
            "mww 0x40023C14 0x0FFFAAEC",
            "mmw 0x40023C14 0 1",
            "mwb 0x20000000 1",
            "mdw 0x1FFFC000",
            "mdw 0x1FFEC000 4",
            "stm32f2x otp 1 enable",
            "load_image x.bin 0x20000000",
            "reg pc 0x08000000",
            "set RDP 1",
            "WRP",
            "BOR level",
            "halt",
            "resume",
            "reset halt",
            "reset init",
            "verify_image /tmp/x.elf",
            "gdb_port 3333",
            "bindto 0.0.0.0",
            "add_script_search_dir /tmp",
        ]:
            self.deny(self.openocd("init", command, "shutdown"))

    def test_denies_tools_outside_the_allowlist(self):
        for argv in [
            ["st-flash", "write", "x.bin", "0x08000000"],
            ["/usr/bin/st-flash", "erase"],
            ["st-flash", "--reset", "read", "x.bin", "0x1FFFC000", "16"],
            ["gdb-multiarch", "-batch", "x.elf"],
            ["STLinkUpgrade"],
            ["java", "-jar", "STLinkUpgrade.jar"],
            ["stlink-upgrade"],
            ["STM32_Programmer_CLI", "-c", "port=SWD", "-ob", "RDP=0xBB"],
            ["sh", "-c", "openocd"],
            ["/usr/bin/env", "openocd"],
            ["st-info", "--probe", "--hot-plug"],
            [fb.ST_INFO, "--probe"],
            [fb.ST_INFO, "--descr", "--hot-plug"],
            [fb.ST_INFO, "--probe", "--connect-under-reset"],
            ["openocd", *self.openocd("init", "shutdown")[1:]],
            ["/usr/local/bin/openocd", *self.openocd("init", "shutdown")[1:]],
        ]:
            self.deny(argv)

    def test_allows_the_reviewed_command_forms(self):
        self.allow([fb.ST_INFO, "--probe", "--hot-plug"])
        self.allow(IDENTITY_CMD)
        self.allow(fb.openocd_reset_command(SERIAL))
        self.allow(fb.openocd_selfcheck_command(SERIAL))
        self.allow(fb.openocd_idcode_command(SERIAL))
        elf = str(RUN_DIR / "image.elf")
        self.allow(fb.openocd_program_command(SERIAL, elf), approved=elf)
        self.allow(fb.openocd_program_command(SERIAL, elf, True),
                   approved=elf)

    def test_openocd_uses_pinned_scripts_and_closed_servers(self):
        tail = ["-c", "init", "-c", "shutdown"]
        self.allow(self.prefix() + tail)
        self.deny(self.prefix(scripts=()) + tail)
        self.deny(self.prefix(scripts=("-s", "/tmp")) + tail)
        self.deny(self.prefix(board="board/st_nucleo_f4.cfg") + tail)
        self.deny(self.prefix(board="/tmp/board/st_nucleo_f4.cfg") + tail)
        for missing in range(3):
            servers = [s for i, s in enumerate(fb.SERVERS_OFF)
                       if i != missing]
            self.deny(self.prefix(servers=servers) + tail)
        self.deny(self.prefix(servers=list(reversed(fb.SERVERS_OFF)))
                  + tail)
        self.deny(self.prefix(events=fb.EVENT_OFF[:1]) + tail)
        self.deny(self.prefix(events=()) + ["-c", "init", "-c",
                                            "mdw 0xe0042000"])

    def test_serial_is_validated_before_use(self):
        argv = self.openocd("init", "shutdown")
        for serial in ["0669ff485753667187124334", SERIAL[:-1],
                       SERIAL[:-1] + ";", SERIAL + "0", None, ""]:
            with self.assertRaises(fb.PolicyError, msg=serial):
                fb.check_command(argv, serial=serial, run_dir=RUN_DIR)
        for serial in ["0669FF48575366718712433;", "x" * 24]:
            with self.assertRaises(fb.PolicyError):
                fb.openocd_base(serial)

    def test_program_requires_the_approved_copy_inside_the_session(self):
        elf = str(RUN_DIR / "image.elf")
        other = str(RUN_DIR / "other.elf")
        command = fb.openocd_program_command(SERIAL, elf)
        self.deny(command, approved=other)
        self.deny(command, approved=None)
        self.deny(command, approved=elf, run_dir=ROOT / "build/f439/runs/y")
        outside = str(ROOT / "build/f439/images/bf-report-115200/image.elf")
        self.deny(fb.openocd_program_command(SERIAL, outside),
                  approved=outside)
        for variant in ["program {%s} verify exit" % elf,
                        "program {%s} reset exit" % elf,
                        "program {%s} preverify verify reset exit" % elf,
                        "program {%s} verify reset exit 0x08004000" % elf,
                        "program %s verify reset exit" % elf]:
            self.deny(self.openocd(variant), approved=elf)
        self.deny(self.openocd("init", "program {%s} verify reset exit"
                               % elf), approved=elf)

    def test_denylist_is_an_independent_layer(self):
        # Each word sits inside an otherwise reviewed form (an approved
        # program path inside the session), so only the denylist refuses.
        for word in ["option", "options_write", "lock", "unlock", "protect",
                     "RDP", "wrp", "BOR", "otp", "mass_erase", "mww",
                     "upgrade", "st-flash", "halt", "flash", "java", "gdb"]:
            elf = str(RUN_DIR / ("x %s y" % word) / "image.elf")
            with self.assertRaises(fb.PolicyError, msg=word):
                fb.check_command(fb.openocd_program_command(SERIAL, elf),
                                 approved_elf=elf, serial=SERIAL,
                                 run_dir=RUN_DIR)
            self.assertIsNotNone(fb.denylisted(elf), word)
        self.assertIsNone(fb.denylisted(str(RUN_DIR / "bf-clock-x.elf")))
        self.assertIsNone(fb.denylisted(" ".join(fb.SERVERS_OFF)))

    def test_word_boundaries_do_not_refuse_ordinary_names(self):
        elf = str(RUN_DIR / "bf-clock-div8" / "image.elf")
        self.allow(fb.openocd_program_command(SERIAL, elf), approved=elf)

    def test_reads_are_limited_to_registers_symbols_and_flash(self):
        for command in ["mdw 0x40004804", "mdw 0x40004800",
                        "mdw 0x50000000", "mdw 0x50001000 4",
                        "mdw 0xe000e010", "mdw 0x20000000",
                        "mdw 0x60000000", "mdw 0x1fff0000",
                        "mdh 0x1fff7800", "mdw 0x081ffffc 2",
                        "mdw 0x00000000", "mdw 0x40023874 2",
                        "mdw 0x08000000 129"]:
            self.deny(self.openocd("init", command, "shutdown"))
        self.allow(self.openocd("init", "mdw 0x40023874", "shutdown"))
        self.allow(self.openocd("init", "mdw 0x08000000 107", "shutdown"))
        self.allow(self.openocd("init", "mdw 0x2001f000", "shutdown"),
                   reads=[(0x2001F000, 4)])
        self.allow(self.openocd("init", "mdw 0x2001f000 3", "shutdown"),
                   reads=[(0x2001F000, 12)])
        self.deny(self.openocd("init", "mdw 0x2001f000 2", "shutdown"),
                  reads=[(0x2001F000, 4)])
        self.deny(self.openocd("init", "mdw 0x40004804", "shutdown"),
                  reads=[(0x40004804, 4)])

    def test_dump_image_only_reads_flash_into_the_session(self):
        good = "dump_image {%s} 0x08000000 0x4000" % (RUN_DIR /
                                                      "readback.bin")
        self.allow(self.openocd("init", good, "shutdown"))
        self.deny(self.openocd("init", "dump_image {/tmp/x.bin} 0x08000000 "
                               "0x10", "shutdown"))
        self.deny(self.openocd("init", "dump_image {%s} 0x20000000 0x100"
                               % (RUN_DIR / "r.bin"), "shutdown"))

    def test_connect_under_reset_must_precede_attach(self):
        self.deny(self.openocd("init", fb.CONNECT_UNDER_RESET,
                               "mdw 0xe0042000", "shutdown"))
        self.deny(self.openocd("mdw 0xe0042000", "init", "shutdown"))

    def test_argv_must_be_non_empty_text(self):
        self.deny([])
        self.deny([fb.ST_INFO, Path("--probe"), "--hot-plug"])

    def test_only_command_pairs_follow_the_board_file(self):
        # `-f shutdown` would load a configuration file named "shutdown".
        self.deny(self.prefix() + ["-f", "shutdown"])
        self.deny(self.prefix() + ["-c", "init", "shutdown"])

    def test_shutdown_must_be_the_final_command(self):
        self.deny(self.openocd("init", "shutdown", "mdw 0xe0042000"))

    def test_every_denylist_pattern_refuses_its_probe(self):
        # Each probe is matched by one pattern only and sits inside an
        # otherwise reviewed program path, so removing that one pattern
        # from fb.DENY_PATTERNS makes this test fail.
        for probe in DENY_PROBES:
            self.assertIsNotNone(fb.denylisted(probe), probe)
            elf = str(RUN_DIR / ("x %s y" % probe) / "image.elf")
            with self.assertRaises(fb.PolicyError, msg=probe):
                fb.check_command(fb.openocd_program_command(SERIAL, elf),
                                 approved_elf=elf, serial=SERIAL,
                                 run_dir=RUN_DIR)


# One probe per pattern of fb.DENY_PATTERNS, in the same order. r"st-flash"
# has no probe of its own: every "st-flash" text also matches r"\bflash\b"
# (see DENY_REDUNDANT).
DENY_PROBES = [
    "stm32f2x options_read 0", "stm32f2x lock 0", "stm32f2x unlock 0",
    "stm32f2x protect", "set RDP 1", "WRP", "BOR level",
    "stm32f2x optcr2_write 0 0", "stm32f2x mass_erase 0",
    "erase_sector 0 0 0", "erase_address 0x08000000 0x4000",
    "stm32f2x otp 1 enable", "mww 0x40023c14 0x0fffaaec",
    "write_bank 0 x.bin", "write_image x.elf", "fillw 0x20000000 0 4",
    "load_image x.bin 0x20000000", "reg pc 0x08000000", "stlink-upgrade",
    "dfu-util -l", "STM32_Programmer_CLI", "STM32CubeProgrammer",
    "java -jar x.jar", "mdw 0x1fffc000", "mdw 0x1ffec000", "halt",
    "resume", "step", "verify_image x.elf", "reset init", "flash info 0",
    "gdb_port 3333", "bp 0x08000000 2 hw", "wp 0x20000000 4",
    "source [find x.cfg]", "exec rm", "bindto 0.0.0.0",
    "add_script_search_dir /tmp",
]
DENY_REDUNDANT = {r"st-flash": r"\bflash\b"}


class LockAndBoundTests(unittest.TestCase):
    def test_lock_refuses_when_held(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "board.lock"
            with fb.BoardLock(path, "first"):
                with self.assertRaises(fb.PolicyError):
                    with fb.BoardLock(path, "second"):
                        pass
            with fb.BoardLock(path, "third"):
                pass

    def test_lock_refuses_a_symlink_a_hard_link_or_a_shared_file(self):
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "real.lock"
            target.write_text("")
            target.chmod(0o600)
            link = Path(tmp) / "link.lock"
            link.symlink_to(target)
            hard = Path(tmp) / "hard.lock"
            os.link(target, hard)
            shared = Path(tmp) / "shared.lock"
            shared.write_text("")
            shared.chmod(0o666)
            fifo = Path(tmp) / "fifo.lock"
            os.mkfifo(fifo, 0o600)
            for path in (link, hard, shared, fifo):
                with self.assertRaises(fb.PolicyError, msg=path.name):
                    with fb.BoardLock(path, "x"):
                        pass
            hard.unlink()
            with fb.BoardLock(target, "single link, private"):
                pass

    def test_lock_never_creates_a_file_through_a_dangling_symlink(self):
        with tempfile.TemporaryDirectory() as tmp:
            elsewhere = Path(tmp) / "elsewhere" / "planted"
            elsewhere.parent.mkdir()
            link = Path(tmp) / "board.lock"
            link.symlink_to(elsewhere)
            with self.assertRaises(fb.PolicyError):
                with fb.BoardLock(link, "x"):
                    pass
            self.assertFalse(elsewhere.exists())

    def test_lock_refuses_a_file_replaced_while_locking(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "board.lock"
            real = fb.fcntl.flock

            def flock(fd, operation):
                if operation & fb.fcntl.LOCK_EX:
                    path.unlink()
                    path.write_text("")
                return real(fd, operation)

            with mock.patch.object(fb.fcntl, "flock", flock):
                with self.assertRaises(fb.PolicyError) as caught:
                    with fb.BoardLock(path, "x"):
                        pass
            self.assertIn("replaced", str(caught.exception))

    def test_lock_records_and_reports_its_holder(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / "board.lock"
            with fb.BoardLock(path, "first"):
                record = json.loads(path.read_text())
                self.assertEqual(
                    (record["holder"], record["pid"], record["root"]),
                    ("first", os.getpid(), str(fb.ROOT)))
                with self.assertRaises(fb.PolicyError) as caught:
                    with fb.BoardLock(path, "second"):
                        pass
                self.assertIn('"holder": "first"', str(caught.exception))

    def test_run_bounded_times_out_and_logs_exact_command(self):
        with tempfile.TemporaryDirectory() as tmp:
            argv = [sys.executable, "-c",
                    "import time,sys; print('start', flush=True); "
                    "time.sleep(5)"]
            started = time.monotonic()
            record = fb.run_bounded(argv, timeout_s=0.5, log_dir=Path(tmp),
                                    label="sleeper")
            self.assertLess(time.monotonic() - started, 4.0)
            self.assertTrue(record["timed_out"])
            self.assertIsNone(record["returncode"])
            self.assertEqual(record["argv"], argv)
            for stream in ("stdout", "stderr"):
                path = Path(tmp) / record[stream]
                self.assertEqual(
                    hashlib.sha256(path.read_bytes()).hexdigest(),
                    record[stream + "_sha256"])
            saved = json.loads((Path(tmp) / record["record"]).read_text())
            self.assertEqual(saved["argv"], argv)

    def test_run_bounded_records_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            record = fb.run_bounded([sys.executable, "-c", "print('ok')"],
                                    timeout_s=10, log_dir=Path(tmp),
                                    label="ok")
            self.assertEqual(record["returncode"], 0)
            self.assertFalse(record["timed_out"])
            self.assertEqual((Path(tmp) / record["stdout"]).read_text(),
                             "ok\n")


class HostLockTests(unittest.TestCase):
    """The board lock that every checkout, worktree and clone shares."""

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)

    def tearDown(self):
        for path in self.root.iterdir():
            if path.is_dir() and not path.is_symlink():
                path.chmod(0o700)
        self.tmp.cleanup()

    def directory(self, name, mode=0o700):
        path = self.root / name
        path.mkdir()
        path.chmod(mode)
        return path

    def test_the_first_existing_candidate_directory_is_used(self):
        runtime = self.directory("runtime")
        self.assertEqual(
            fb.host_lock_path([self.root / "missing", runtime]),
            runtime / fb.HOST_LOCK_NAME)
        self.assertEqual(fb.host_lock_path([self.directory("open", 0o755)]),
                         self.root / "open" / fb.HOST_LOCK_NAME)

    def test_an_unusable_existing_candidate_is_refused_not_skipped(self):
        runtime = self.directory("runtime")
        link = self.root / "link"
        link.symlink_to(runtime)
        plain = self.root / "plain"
        plain.write_text("")
        plain.chmod(0o600)
        bad = [self.directory("group", 0o770), self.directory("other", 0o707),
               self.directory("sticky-not-root", 0o1777), link, plain]
        for candidate in bad:
            with self.assertRaises(fb.PolicyError, msg=candidate.name):
                fb.host_lock_path([candidate, runtime])

    def test_a_directory_of_another_user_is_refused(self):
        foreign = self.directory("foreign", 0o755)
        real = fb.os.lstat

        def lstat(path, *args, **kwargs):
            info = real(path, *args, **kwargs)
            if Path(path) != foreign:
                return info
            fields = list(info[:10])
            fields[4] = os.geteuid() + 1  # st_uid: neither root nor us
            return os.stat_result(fields)

        with mock.patch.object(fb.os, "lstat", lstat):
            with self.assertRaises(fb.PolicyError):
                fb.host_lock_path([foreign, self.directory("runtime")])

    def test_no_candidate_directory_is_refused(self):
        with self.assertRaises(fb.PolicyError):
            fb.host_lock_path([self.root / "a", self.root / "b"])

    def test_default_lock_is_in_the_system_lock_directory(self):
        self.assertEqual(fb.HOST_LOCK_DIRS, ("/run/lock", "/run/user/{uid}"))
        if not Path("/run/lock").is_dir():
            self.skipTest("no /run/lock on this host")
        self.assertEqual(fb.host_lock_path(),
                         Path("/run/lock") / fb.HOST_LOCK_NAME)


class SerialCaptureTests(unittest.TestCase):
    def setUp(self):
        self.master, self.slave = os.openpty()
        self.device = os.ttyname(self.slave)
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        os.close(self.master)
        os.close(self.slave)
        self.tmp.cleanup()

    def test_capture_stops_at_the_byte_bound(self):
        payload = bytes(range(256)) * 8

        def writer():
            time.sleep(0.3)
            os.write(self.master, payload)

        thread = threading.Thread(target=writer)
        thread.start()
        summary = fb.capture_serial(self.device, seconds=5, max_bytes=1000,
                                    baud=115200, parity="none",
                                    out_dir=Path(self.tmp.name))
        thread.join()
        raw = (Path(self.tmp.name) / summary["raw"]).read_bytes()
        self.assertEqual(len(raw), 1000)
        self.assertEqual(summary["bytes"], 1000)
        self.assertEqual(summary["stop_reason"], "byte-bound")
        self.assertTrue(summary["exclusive"])
        self.assertEqual(hashlib.sha256(raw).hexdigest(),
                         summary["raw_sha256"])

    def test_capture_stops_at_the_time_bound(self):
        started = time.monotonic()
        summary = fb.capture_serial(self.device, seconds=0.3, max_bytes=10,
                                    baud=115200, parity="none",
                                    out_dir=Path(self.tmp.name))
        self.assertLess(time.monotonic() - started, 3.0)
        self.assertEqual(summary["bytes"], 0)
        self.assertEqual(summary["stop_reason"], "time-bound")

    def test_capture_sends_scheduled_bytes_and_logs_them(self):
        summary = fb.capture_serial(self.device, seconds=0.4, max_bytes=10,
                                    baud=115200, parity="even",
                                    out_dir=Path(self.tmp.name),
                                    sends=[(0.05, b"\x00A\xff")])
        received = os.read(self.master, 16)
        self.assertEqual(received, b"\x00A\xff")
        index = [json.loads(line) for line in
                 (Path(self.tmp.name) / summary["index"]).read_text()
                 .splitlines()]
        self.assertTrue(any(e["dir"] == "tx" and e["hex"] == "0041ff"
                            for e in index))

    def test_capture_holds_exclusive_access(self):
        import errno
        outcome = {}

        def second_opener():
            time.sleep(0.25)
            try:
                fd = os.open(self.device, os.O_RDWR | os.O_NOCTTY)
                os.close(fd)
                outcome["errno"] = 0
            except OSError as error:
                outcome["errno"] = error.errno

        thread = threading.Thread(target=second_opener)
        thread.start()
        fb.capture_serial(self.device, seconds=0.6, max_bytes=10,
                          baud=115200, parity="none",
                          out_dir=Path(self.tmp.name))
        thread.join()
        self.assertEqual(outcome["errno"], errno.EBUSY)

    def test_capture_rejects_unbounded_requests(self):
        self.assertEqual((fb.MAX_CAPTURE_SECONDS, fb.MAX_CAPTURE_BYTES),
                         (600, 4 * 1024 * 1024))
        saved = fb.MAX_CAPTURE_SECONDS, fb.MAX_CAPTURE_BYTES
        # Small limits keep a regression (a missing bound) from blocking.
        fb.MAX_CAPTURE_SECONDS, fb.MAX_CAPTURE_BYTES = 0.2, 4
        try:
            for kwargs in [dict(seconds=0, max_bytes=2),
                           dict(seconds=0.1, max_bytes=0),
                           dict(seconds=0.3, max_bytes=2),
                           dict(seconds=0.1, max_bytes=5)]:
                with self.assertRaises(fb.PolicyError, msg=kwargs):
                    fb.capture_serial(self.device, baud=115200,
                                      parity="none",
                                      out_dir=Path(self.tmp.name), **kwargs)
        finally:
            fb.MAX_CAPTURE_SECONDS, fb.MAX_CAPTURE_BYTES = saved
        with self.assertRaises(fb.PolicyError):
            fb.capture_serial(self.device, seconds=1, max_bytes=10,
                              baud=12345, parity="none",
                              out_dir=Path(self.tmp.name))
        with self.assertRaises(fb.PolicyError):
            fb.capture_serial(self.device, seconds=1, max_bytes=10,
                              baud=115200, parity="none",
                              out_dir=Path(self.tmp.name),
                              sends=[(0.1, bytes(fb.MAX_SEND_BYTES + 1))])


class IdentityTests(unittest.TestCase):
    def setUp(self):
        self.expected = fb.load_expected_identity()
        self.st_info = (IDENTITY_DIR / "st-info-probe.txt").read_text()
        self.openocd = (IDENTITY_DIR / "openocd-identity.txt").read_text()

    def test_receipt_outputs_match_the_receipt(self):
        observed = fb.identity_from_outputs(self.st_info, self.openocd)
        self.assertEqual(fb.identity_mismatches(observed, self.expected), [])

    def test_mismatches_are_reported(self):
        cases = [
            (self.st_info.replace(SERIAL, "0669FF485753667187120000"),
             self.openocd, "serial"),
            (self.st_info.replace("Found 1 stlink", "Found 2 stlink"),
             self.openocd, "programmers"),
            (self.st_info, self.openocd.replace("20036419", "20036413"),
             "DEV_ID"),
            (self.st_info, self.openocd.replace("3133510a", "3133510b"),
             "UID"),
            (self.st_info, self.openocd.replace("0fffaaed", "0fffbbed"),
             "FLASH_OPTCR"),
            (self.st_info, self.openocd.replace("0x1fff7a22: 0800",
                                                "0x1fff7a22: 0400"),
             "flash size"),
            (self.st_info, self.openocd.replace("0xe0042000: 20036419", ""),
             "DEV_ID"),
            (self.st_info, self.openocd.replace("20036419", "10036419"),
             "REV_ID"),
            (re.sub(r"chipid:\s+0x419", "chipid: 0x413", self.st_info),
             self.openocd, "chipid"),
            (re.sub(r"flash:\s+2097152", "flash: 1048576", self.st_info),
             self.openocd, "flash bytes"),
        ]
        for st_info, openocd, needle in cases:
            observed = fb.identity_from_outputs(st_info, openocd)
            mismatches = fb.identity_mismatches(observed, self.expected)
            self.assertTrue(any(needle in m for m in mismatches),
                            (needle, mismatches))




class ToolingTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        root = Path(self.tmp.name)
        self.scripts = root / "scripts"
        self.site = root / "site"
        self.pins = {}
        for name, text in [("board/b.cfg", "source [find target/t.cfg]\n"),
                           ("target/t.cfg", "set x 1\n")]:
            path = self.scripts / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text)
            self.pins[str(path)] = hashlib.sha256(text.encode()).hexdigest()

    def tearDown(self):
        self.tmp.cleanup()

    def check(self):
        return fb.verify_tooling(self.pins, self.scripts, [self.site])

    def test_matching_pins_pass(self):
        self.assertEqual(self.check(), [])

    def test_changed_or_missing_pinned_file_is_refused(self):
        (self.scripts / "target/t.cfg").write_text("mww 0x40023c08 1\n")
        self.assertTrue(any("t.cfg" in r for r in self.check()))
        (self.scripts / "target/t.cfg").unlink()
        self.assertTrue(any("t.cfg" in r for r in self.check()))

    def test_a_shadowing_copy_is_refused(self):
        shadow = self.site / "target/t.cfg"
        shadow.parent.mkdir(parents=True)
        shadow.write_text("set x 1\n")
        self.assertTrue(any("shadow" in r for r in self.check()))

    def test_real_pins_cover_the_sourced_closure(self):
        names = {Path(p).relative_to(fb.OPENOCD_SCRIPTS).as_posix()
                 for p in fb.PINNED_FILES if p.startswith(
                     str(fb.OPENOCD_SCRIPTS))}
        self.assertEqual(names, {"board/st_nucleo_f4.cfg",
                                 "interface/stlink.cfg",
                                 "target/stm32f4x.cfg",
                                 "target/swj-dp.tcl", "mem_helper.tcl"})
        self.assertIn(fb.OPENOCD, fb.PINNED_FILES)
        self.assertIn(fb.ST_INFO, fb.PINNED_FILES)
        if not Path(fb.BOARD_CFG).is_file():
            self.skipTest("openocd scripts not installed here")
        for name in names:
            text = (fb.OPENOCD_SCRIPTS / name).read_text()
            for found in re.findall(r"source \[find ([^\]]+)\]", text):
                self.assertIn(found, names, name)


class ProcessTests(unittest.TestCase):
    def test_no_test_can_start_a_board_tool(self):
        self.assertEqual(subprocess.Popen.__name__, "NoBoardToolPopen")
        with tempfile.TemporaryDirectory() as tmp:
            fake = Path(tmp) / "st-flash"
            fake.write_text("")  # not executable: nothing could run anyway
            self.assertTrue(starts_board_tool([str(fake), "erase"]))
            with self.assertRaises(RuntimeError):
                subprocess.Popen([str(fake), "erase"])
            alias = Path(tmp) / "openocd"
            alias.symlink_to(sys.executable)
            self.assertFalse(starts_board_tool([str(alias)]))
        for name in (fb.OPENOCD, "st-info"):
            if shutil.which(name):
                self.assertTrue(starts_board_tool([name, "--version"]))

    def test_scanner_finds_a_board_tool_by_program_name(self):
        with tempfile.TemporaryDirectory() as tmp:
            fake = Path(tmp) / "openocd"
            fake.symlink_to(sys.executable)
            child = subprocess.Popen([str(fake), "-c",
                                      "import time; time.sleep(20)"])
            try:
                found = []
                for _ in range(50):
                    found = [p for p in fb.board_tool_processes()
                             if p["pid"] == child.pid]
                    if found:
                        break
                    time.sleep(0.05)
                self.assertTrue(found)
            finally:
                child.kill()
                child.wait()

    def test_live_children_are_terminated(self):
        with tempfile.TemporaryDirectory() as tmp:
            result = {}

            def worker():
                result.update(fb.run_bounded(
                    [sys.executable, "-c", "import time; time.sleep(30)"],
                    timeout_s=60, log_dir=Path(tmp), label="long"))

            thread = threading.Thread(target=worker)
            started = time.monotonic()
            thread.start()
            for _ in range(100):
                if fb.live_children():
                    break
                time.sleep(0.05)
            self.assertTrue(fb.live_children())
            fb.terminate_live_children()
            thread.join(10)
            self.assertFalse(thread.is_alive())
            self.assertLess(time.monotonic() - started, 10)
            self.assertNotEqual(result["returncode"], 0)
            self.assertEqual(fb.live_children(), [])


class CaptureGuardTests(unittest.TestCase):
    def setUp(self):
        self.master, self.slave = os.openpty()
        self.device = os.ttyname(self.slave)
        self.tmp = tempfile.TemporaryDirectory()

    def tearDown(self):
        os.close(self.master)
        os.close(self.slave)
        self.tmp.cleanup()

    def capture(self, **kwargs):
        args = dict(seconds=0.5, max_bytes=10, baud=115200, parity="none",
                    out_dir=Path(self.tmp.name))
        args.update(kwargs)
        return fb.capture_serial(self.device, **args)

    def test_refuses_a_port_another_process_holds_open(self):
        holder = subprocess.Popen(
            [sys.executable, "-c",
             "import os,sys,time; fd=os.open(sys.argv[1], os.O_RDWR|"
             "os.O_NOCTTY); print('open', flush=True); time.sleep(20)",
             self.device], stdout=subprocess.PIPE, text=True)
        try:
            self.assertEqual(holder.stdout.readline().strip(), "open")
            with self.assertRaises(fb.PolicyError):
                self.capture()
        finally:
            holder.kill()
            holder.wait()
            holder.stdout.close()

    def test_termios_change_during_capture_is_detected(self):
        self.assertTrue(self.capture()["termios_unchanged"])

        def changer():
            time.sleep(0.2)
            attrs = termios.tcgetattr(self.slave)
            attrs[4] = attrs[5] = termios.B9600
            termios.tcsetattr(self.slave, termios.TCSANOW, attrs)

        thread = threading.Thread(target=changer)
        thread.start()
        summary = self.capture()
        thread.join()
        self.assertFalse(summary["termios_unchanged"])

    def fake_gexcl(self, value=0, error=None):
        """TIOCEXCL really applied; the TIOCGEXCL answer is substituted."""
        real = fb.fcntl.ioctl

        def ioctl(fd, request, *args):
            if request == fb.TIOCGEXCL:
                if error is not None:
                    raise OSError(error, os.strerror(error))
                args[0][0] = value
                return 0
            return real(fd, request, *args)

        return mock.patch.object(fb.fcntl, "ioctl", ioctl)

    def test_exclusive_mode_must_be_confirmed_by_tiocgexcl(self):
        for patch in (self.fake_gexcl(0), self.fake_gexcl(error=errno.ENOTTY)):
            with patch, self.assertRaises(fb.PolicyError) as caught:
                self.capture(label="unconfirmed")
            self.assertIn("exclusive", str(caught.exception))
            self.assertFalse(
                (Path(self.tmp.name) / "unconfirmed.bin").exists())
        # A refused attempt leaves the port usable (TIOCNXCL on refusal).
        self.assertTrue(self.capture(label="after", seconds=0.1)["exclusive"])

    def test_send_schedule_is_bounded(self):
        for sends in ([(0.0, b"x")] * (fb.MAX_SENDS + 1), [(0.6, b"x")],
                      [(-0.1, b"x")], [(0.1, b"")]):
            with self.assertRaises(fb.PolicyError, msg=len(sends)):
                self.capture(seconds=0.5, sends=sends)

    def test_capture_records_wall_clock_window(self):
        summary = self.capture(seconds=0.2)
        self.assertLessEqual(summary["start_epoch"], summary["end_epoch"])
        self.assertGreater(summary["end_epoch"] - summary["start_epoch"], 0.1)

    def test_labels_are_restricted(self):
        for label in ["../x", "a/b", "", ".hidden", "x y", "a" * 65,
                      "café"]:
            with self.assertRaises(fb.PolicyError, msg=label):
                self.capture(label=label)


class ReadParsingTests(unittest.TestCase):
    def test_multi_line_mdw_output_is_merged(self):
        words = list(range(0x1000, 0x1000 + 107))
        lines = []
        for offset in range(0, 107, 8):
            chunk = words[offset:offset + 8]
            lines.append("0x%08x: %s " % (0x08000000 + 4 * offset,
                                          " ".join("%08x" % w
                                                   for w in chunk)))
        text = "\n".join(["0xe0042000: 20036419 ", *lines,
                          "0x1fff7a10: 004c0025 3133510a 36393739 "]) + "\n"
        blocks = fb.parse_openocd_reads(text)
        self.assertEqual(fb.read_words(blocks, 0x08000000, 107), words)
        self.assertEqual(fb.read_words(blocks, 0x08000020, 2), words[8:10])
        self.assertEqual(fb.read_words(blocks, 0xE0042000, 1), [0x20036419])
        self.assertIsNone(fb.read_words(blocks, 0x08000000, 108))
        self.assertIsNone(fb.read_words(blocks, 0xE0042004, 1))

    def test_halfword_lines_merge_only_with_halfwords(self):
        text = "0x1fff7a22: 0800 \n0x1fff7a24: 0001 \n"
        blocks = fb.parse_openocd_reads(text)
        self.assertEqual(fb.read_words(blocks, 0x1FFF7A22, 2, width=2),
                         [0x800, 0x1])


class SerialValidationTests(unittest.TestCase):
    def test_receipt_serial_must_be_24_uppercase_hex(self):
        with tempfile.TemporaryDirectory() as tmp:
            receipt = json.loads((IDENTITY_DIR / "identity.json")
                                 .read_text())
            receipt["observed"]["stlink_serial"] = SERIAL + "\"; mww 0 0"
            path = Path(tmp) / "identity.json"
            path.write_text(json.dumps(receipt))
            with self.assertRaises(fb.PolicyError):
                fb.load_expected_identity(path)

    def test_board_refuses_a_bad_expected_serial(self):
        expected = dict(fb.load_expected_identity(), serial="bad")
        with self.assertRaises(fb.PolicyError):
            fb.Board(expected=expected)

    def test_cli_lock_path_ignores_state(self):
        board = fb.Board(Path("/tmp/somewhere-else"), dry_run=True)
        self.assertEqual(board.lock_path, fb.LOCK_PATH)
        self.assertIs(board.host_lock, fb.host_lock_path)


GOOD_TEXT = vectors() + bytes(64)
GOOD_DATA = b"\x78\x56\x34\x12"
# Flash contents of good_image(), written out independently of the module:
# .vectors/.text at 0x08000000 followed by the .data load image.
GOOD_FLASH = GOOD_TEXT + GOOD_DATA
SELFCHECK_OK = "F439_EXAMINE_END=<>\nF439_RESET_INIT=<>\n"


class FakeExecutor:
    """Canned tool output; records argv, cwd and env of every call.

    openocd_outputs / openocd_rcs: optional lists consumed by successive
    non-program openocd calls (defaults: the given text, return code 0).
    readback: bytes written by a dump_image command.
    """

    def __init__(self, st_info, openocd, program_rc=0, verified=True,
                 openocd_outputs=None, readback=GOOD_FLASH, on_call=None,
                 openocd_rcs=None):
        self.calls, self.contexts = [], []
        self.on_call = on_call or (lambda argv: None)
        self.st_info, self.openocd = st_info, openocd
        self.program_rc, self.verified = program_rc, verified
        self.outputs = list(openocd_outputs or [])
        self.rcs = list(openocd_rcs or [])
        self.readback = readback

    def __call__(self, argv, *, timeout_s, log_dir, label, cwd=None,
                 env=None):
        self.calls.append(list(argv))
        self.on_call(argv)
        self.contexts.append(dict(cwd=cwd, env=dict(env or {}),
                                  cwd_entries=os.listdir(cwd)
                                  if cwd else None))
        out, rc = "", 0
        if argv[0] == fb.ST_INFO:
            out = self.st_info
        elif any(a.startswith("program ") for a in argv):
            out = ("** Programming Finished **\n** Verify Started **\n"
                   + ("** Verified OK **\n" if self.verified else "")
                   + "** Resetting Target **\n")
            rc = self.program_rc
        else:
            out = self.outputs.pop(0) if self.outputs else self.openocd
            rc = self.rcs.pop(0) if self.rcs else 0
            for arg in argv:
                match = re.fullmatch(r"dump_image \{(.+)\} 0x[0-9a-f]+ "
                                     r"0x([0-9a-f]+)", arg)
                if match:
                    Path(match[1]).write_bytes(
                        self.readback[:int(match[2], 16)])
        log_dir.mkdir(parents=True, exist_ok=True)
        path = log_dir / (label + ".stdout")
        path.write_text(out)
        err = log_dir / (label + ".stderr")
        err.write_text("")
        return dict(argv=list(argv), returncode=rc, timed_out=False,
                    stdout=path.name, stderr=err.name,
                    stdout_sha256=hashlib.sha256(out.encode()).hexdigest(),
                    stderr_sha256=hashlib.sha256(b"").hexdigest(),
                    record=None, timeout_s=timeout_s)


def git(repo, *args):
    subprocess.run(["git", *args], cwd=repo, check=True,
                   capture_output=True)


def is_program(argv):
    return any(a.startswith("program ") for a in argv)


class ProvenanceAndBoardTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.repo = Path(self.tmp.name) / "repo"
        (self.repo / "tools").mkdir(parents=True)
        git(self.repo, "init", "-q")
        git(self.repo, "config", "user.email", "t@example.invalid")
        git(self.repo, "config", "user.name", "t")
        (self.repo / "src.c").write_text("int x;\n")
        (self.repo / "tools/f439_board.py").write_text("# layer\n")
        git(self.repo, "add", "src.c", "tools/f439_board.py")
        git(self.repo, "commit", "-q", "-m", "init")
        self.image_dir = Path(self.tmp.name) / "images" / "bf-test"
        self.image_dir.mkdir(parents=True)
        self.elf = good_image()
        (self.image_dir / "image.elf").write_bytes(self.elf)
        self.sources = {"src.c": hashlib.sha256(b"int x;\n").hexdigest()}
        self.manifest = dict(
            schema=fb.MANIFEST_SCHEMA, image="bf-test", elf="image.elf",
            elf_sha256=hashlib.sha256(self.elf).hexdigest(),
            sources=dict(self.sources))
        (self.image_dir / "manifest.json").write_text(
            json.dumps(self.manifest))
        self.st_info = (IDENTITY_DIR / "st-info-probe.txt").read_text()
        # The receipt predates the FLASH_OPTCR1 read that every identity
        # attach now makes; the fake adds that line.
        self.receipt = (IDENTITY_DIR / "openocd-identity.txt").read_text()
        self.openocd = self.receipt + OPTCR1_LINE
        self.lock = Path(self.tmp.name) / "lock" / "board.lock"
        self.host_lock = Path(self.tmp.name) / "host" / fb.HOST_LOCK_NAME
        self.state = Path(self.tmp.name) / "state"
        self.journal_calls = []

    def tearDown(self):
        self.tmp.cleanup()

    def rebuilt(self, sha=None, sources=None):
        return dict(elf_sha256=sha or self.manifest["elf_sha256"],
                    sources=self.sources if sources is None else sources)

    def board(self, executor, rebuilt=None, dry_run=False, scan=None,
              lock=None, state=None):
        result = rebuilt or self.rebuilt()

        def journal(start, end, log_dir):
            self.journal_calls.append((start, end))
            return dict(lines=0, status="fake")

        return fb.Board(state or self.state, repo=self.repo,
                        executor=executor, rebuilder=lambda name: result,
                        dry_run=dry_run, lock_path=lock or self.lock,
                        host_lock=lambda: self.host_lock,
                        process_scan=scan or (lambda: []),
                        verify_tools=lambda: [], journal=journal)

    def only_run(self, tag):
        runs = sorted(self.state.glob("runs/*-%s/run.json" % tag))
        self.assertEqual(len(runs), 1, runs)
        return json.loads(runs[0].read_text())

    def record_baseline(self, line=OPTCR1_LINE):
        self.board(FakeExecutor(self.st_info, self.receipt + line)).probe()

    # provenance --------------------------------------------------------
    def test_sources_tracked_and_clean_pass(self):
        self.assertEqual(fb.verify_sources(self.sources, self.repo), [])

    def test_sources_modified_untracked_or_outside_are_refused(self):
        (self.repo / "src.c").write_text("int y;\n")
        self.assertTrue(fb.verify_sources(self.sources, self.repo))
        git(self.repo, "checkout", "--", "src.c")
        (self.repo / "new.c").write_text("int z;\n")
        sources = dict(self.sources,
                       **{"new.c": hashlib.sha256(b"int z;\n").hexdigest()})
        self.assertTrue(any("tracked" in r for r in
                            fb.verify_sources(sources, self.repo)))
        self.assertTrue(fb.verify_sources({"../outside.c": "0" * 64},
                                          self.repo))

    def test_a_source_that_differs_from_the_built_bytes_is_refused(self):
        # Identical to HEAD, but not the bytes the build hashed.
        refusals = fb.verify_sources({"src.c": "0" * 64}, self.repo)
        self.assertTrue(any("changed since build" in r for r in refusals),
                        refusals)

    def test_paths_outside_the_repository_are_named(self):
        for path in ["../outside.c", "/etc/hostname", "tools/../../x.c"]:
            refusals = fb.verify_sources({path: "0" * 64}, self.repo)
            self.assertTrue(any("outside the repository" in r
                                for r in refusals), (path, refusals))

    def test_provenance_requires_the_schema_and_a_dependency_set(self):
        manifest = dict(self.manifest, schema="f439-image-manifest/v0")
        self.assertTrue(any("schema" in r for r in fb.verify_provenance(
            manifest, self.repo, self.image_dir, self.rebuilt())))
        refusals = fb.verify_provenance(
            dict(self.manifest, sources={}), self.repo, self.image_dir,
            self.rebuilt(sources={}))
        self.assertTrue(any("no sources" in r for r in refusals), refusals)

    def test_flash_refuses_a_manifest_for_another_image(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).flash("bf-other", self.image_dir)
        self.assertIn("different image", str(caught.exception))
        self.assertEqual(executor.calls, [])

    def test_gate_programs_and_compares_the_bytes_it_hashed(self):
        # The ELF is swapped after every hash in the offline gate and put
        # back before the session copy: the readback must be compared with
        # the hashed bytes, not with a later read of the file.
        elf = self.image_dir / "image.elf"
        evil = self.elf.replace(GOOD_DATA, b"\0\0\0\0", 1)
        self.assertNotEqual(evil, self.elf)
        real = fb.verify_provenance

        def provenance(*args):
            refusals = real(*args)
            elf.write_bytes(evil)
            return refusals

        executor = FakeExecutor(self.st_info, self.openocd,
                                on_call=lambda argv: elf.write_bytes(self.elf))
        with mock.patch.object(fb, "verify_provenance", provenance):
            result = self.board(executor).flash("bf-test", self.image_dir)
        run = json.loads((Path(result["run_dir"]) / "run.json").read_text())
        self.assertEqual(run["status"], "completed")
        self.assertIs(run["readback_match"], True)

    def test_provenance_uses_the_rebuild_dependency_set(self):
        self.assertEqual(fb.verify_provenance(self.manifest, self.repo,
                                              self.image_dir,
                                              self.rebuilt()), [])
        (self.repo / "extra.h").write_text("#define X 1\n")
        deps = dict(self.sources, **{"extra.h": hashlib.sha256(
            b"#define X 1\n").hexdigest()})
        refusals = fb.verify_provenance(self.manifest, self.repo,
                                        self.image_dir,
                                        self.rebuilt(sources=deps))
        self.assertTrue(any("dependency" in r for r in refusals))
        self.assertTrue(any("extra.h" in r and "tracked" in r
                            for r in refusals))
        refusals = fb.verify_provenance(self.manifest, self.repo,
                                        self.image_dir,
                                        self.rebuilt(sha="1" * 64))
        self.assertTrue(any("rebuild" in r for r in refusals))
        manifest = dict(self.manifest, elf_sha256="0" * 64)
        self.assertTrue(any("ELF" in r for r in fb.verify_provenance(
            manifest, self.repo, self.image_dir,
            self.rebuilt(sha="0" * 64))))

    def test_flash_refuses_a_modified_safety_layer(self):
        (self.repo / "tools/f439_board.py").write_text("# edited\n")
        executor = FakeExecutor(self.st_info, self.openocd)
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertIn("tools/f439_board.py", str(caught.exception))
        self.assertEqual(executor.calls, [])

    def test_flash_refuses_bad_provenance_without_touching_tools(self):
        (self.repo / "src.c").write_text("int y;\n")
        executor = FakeExecutor(self.st_info, self.openocd)
        with self.assertRaises(fb.PolicyError):
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertEqual(executor.calls, [])

    def test_flash_refuses_irreproducible_build_without_touching_tools(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        with self.assertRaises(fb.PolicyError):
            self.board(executor, rebuilt=self.rebuilt(sha="1" * 64)).flash(
                "bf-test", self.image_dir)
        self.assertEqual(executor.calls, [])

    def test_flash_refuses_unsafe_elf_without_touching_tools(self):
        bad = make_elf(
            sections=[dict(name=".opt", type=SHT_PROGBITS, flags=SHF_ALLOC,
                           addr=0x1FFFC000, data=bytes(16))],
            segments=[dict(section=".opt", paddr=0x1FFFC000,
                           vaddr=0x1FFFC000)])
        (self.image_dir / "image.elf").write_bytes(bad)
        sha = hashlib.sha256(bad).hexdigest()
        (self.image_dir / "manifest.json").write_text(json.dumps(
            dict(self.manifest, elf_sha256=sha)))
        executor = FakeExecutor(self.st_info, self.openocd)
        with self.assertRaises(fb.PolicyError):
            self.board(executor, rebuilt=self.rebuilt(sha=sha)).flash(
                "bf-test", self.image_dir)
        self.assertEqual(executor.calls, [])

    # environment, lock and other processes -------------------------------
    def test_tools_run_in_an_empty_sandbox_with_a_scrubbed_environment(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        saved = os.environ.get("OPENOCD_SCRIPTS")
        os.environ["OPENOCD_SCRIPTS"] = "/tmp/evil"
        try:
            self.board(executor).flash("bf-test", self.image_dir)
        finally:
            if saved is None:
                os.environ.pop("OPENOCD_SCRIPTS")
            else:
                os.environ["OPENOCD_SCRIPTS"] = saved
        self.assertTrue(executor.contexts)
        for context in executor.contexts:
            self.assertEqual(context["cwd_entries"], [])
            env = context["env"]
            self.assertEqual(set(env), {"PATH", "LC_ALL", "HOME",
                                        "XDG_CONFIG_HOME"})
            self.assertEqual(env["HOME"], context["cwd"])
            self.assertEqual(env["XDG_CONFIG_HOME"], context["cwd"])
            self.assertEqual(env["PATH"], "/usr/bin:/bin")
        self.assertFalse(Path(executor.contexts[0]["cwd"]).exists())

    def test_tool_pin_mismatch_refuses_before_any_tool(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor)
        board.verify_tools = lambda: ["pinned file changed: x"]
        with self.assertRaises(fb.PolicyError):
            board.reset_run()
        self.assertEqual(executor.calls, [])

    def test_running_board_tool_refuses_before_any_tool(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor, scan=lambda: [
            dict(pid=4242, argv=["/usr/bin/openocd"])])
        with self.assertRaises(fb.PolicyError) as caught:
            board.reset_run()
        self.assertIn("4242", str(caught.exception))
        self.assertEqual(executor.calls, [])

    def test_preflight_runs_again_before_every_tool(self):
        # Clean at session start; a board tool (or a changed pin) appears
        # once the first tool has run. The next tool must not start.
        for problem in ("scan", "pins"):
            executor = FakeExecutor(self.st_info, self.openocd)
            other = [dict(pid=4242, argv=["/usr/bin/st-flash", "write"])]
            board = self.board(executor, scan=lambda: other
                               if executor.calls and problem == "scan"
                               else [])
            board.verify_tools = lambda: ["pinned file changed: x"] \
                if executor.calls and problem == "pins" else []
            with self.assertRaises(fb.PolicyError, msg=problem) as caught:
                board.flash("bf-test", self.image_dir)
            self.assertIn("4242" if problem == "scan" else "pinned",
                          str(caught.exception))
            self.assertEqual(len(executor.calls), 1, problem)
            self.assertFalse(any(is_program(c) for c in executor.calls))

    def test_a_file_left_in_the_sandbox_refuses_the_next_tool(self):
        fake = FakeExecutor(self.st_info, self.openocd)

        def planting(argv, **kwargs):
            if not fake.calls:
                self.addCleanup(shutil.rmtree, kwargs["cwd"], True)
            record = fake(argv, **kwargs)
            (Path(kwargs["cwd"]) / "openocd.cfg").write_text(
                "mww 0x40023c08 1\n")
            return record

        with self.assertRaises(fb.PolicyError) as caught:
            self.board(planting).flash("bf-test", self.image_dir)
        self.assertIn("sandbox is not empty", str(caught.exception))
        self.assertEqual(len(fake.calls), 1)
        self.assertFalse(any(is_program(c) for c in fake.calls))
        run = self.only_run("program-bf-test")
        self.assertEqual(run["sandbox_leftovers"], ["openocd.cfg"])

    def test_session_refuses_an_unreviewed_command_before_running_it(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor)
        for argv in (["/usr/bin/st-flash", "erase"],
                     fb.openocd_identity_command(
                         SERIAL, ["mdw 0x40004804"])):
            with self.assertRaises(fb.PolicyError, msg=argv):
                with board.session("unreviewed") as session:
                    session.run(argv, timeout_s=1, label="x")
        self.assertEqual(executor.calls, [])

    def test_failed_reset_is_refused(self):
        executor = FakeExecutor(self.st_info, self.openocd,
                                openocd_rcs=[0, 1])
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).reset_run()
        self.assertIn("reset failed", str(caught.exception))
        self.assertEqual(self.only_run("reset")["status"],
                         "refused-or-failed")

    def test_identity_read_failure_refuses(self):
        executor = FakeExecutor(self.st_info, self.openocd, openocd_rcs=[1])
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).reset_run()
        self.assertIn("identity read failed", str(caught.exception))
        self.assertEqual(len(executor.calls), 1)

    def test_host_lock_is_shared_by_every_checkout(self):
        first = self.board(FakeExecutor(self.st_info, self.openocd))
        executor = FakeExecutor(self.st_info, self.openocd)
        other = self.board(executor,
                           lock=Path(self.tmp.name) / "other/board.lock",
                           state=Path(self.tmp.name) / "other-state")
        with first.session("outer"):
            with self.assertRaises(fb.PolicyError) as caught:
                other.reset_run()
        self.assertIn("board lock held: %s" % self.host_lock,
                      str(caught.exception))
        self.assertEqual(executor.calls, [])
        other.reset_run()
        self.assertEqual(len(executor.calls), 2)

    def test_lock_is_held_for_the_whole_session(self):
        board = self.board(FakeExecutor(self.st_info, self.openocd))
        with board.session("outer"):
            for path in (self.lock, self.host_lock):
                with self.assertRaises(fb.PolicyError):
                    with fb.BoardLock(path, "intruder"):
                        pass

    def test_session_exit_terminates_children(self):
        board = self.board(FakeExecutor(self.st_info, self.openocd))
        result = {}
        with board.session("children") as session:
            def worker():
                result.update(fb.run_bounded(
                    [sys.executable, "-c", "import time; time.sleep(30)"],
                    timeout_s=60, log_dir=session.dir, label="orphan"))

            thread = threading.Thread(target=worker)
            thread.start()
            for _ in range(100):
                if fb.live_children():
                    break
                time.sleep(0.05)
        thread.join(10)
        self.assertFalse(thread.is_alive())
        self.assertEqual(fb.live_children(), [])

    # flash ----------------------------------------------------------------
    def test_flash_happy_path(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        result = self.board(executor).flash("bf-test", self.image_dir)
        run_dir = Path(result["run_dir"])
        self.assertEqual([c[0] for c in executor.calls], [fb.OPENOCD] * 3)
        programs = [c for c in executor.calls if is_program(c)]
        self.assertEqual(len(programs), 1)
        self.assertEqual(executor.calls.index(programs[0]), 1)
        self.assertIn("program {%s} verify reset exit"
                      % (run_dir / "image.elf"), programs[0])
        self.assertEqual(hashlib.sha256((run_dir / "image.elf")
                                        .read_bytes()).hexdigest(),
                         self.manifest["elf_sha256"])
        dump = [a for a in executor.calls[2] if a.startswith("dump_image")]
        self.assertEqual(dump, ["dump_image {%s} 0x08000000 0x%x"
                                % (run_dir / "readback.bin",
                                   len(GOOD_FLASH))])
        run = json.loads((run_dir / "run.json").read_text())
        self.assertEqual((run["action"], run["status"]),
                         ("flash", "completed"))
        self.assertIs(run["readback_match"], True)
        self.assertIn("run.json", (run_dir / "SHA256SUMS").read_text())

    def test_flash_refuses_an_elf_swapped_after_the_gate(self):
        def tamper(argv):
            (self.image_dir / "image.elf").write_bytes(self.elf + b"x")

        executor = FakeExecutor(self.st_info, self.openocd, on_call=tamper)
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertIn("session copy", str(caught.exception))
        self.assertFalse(any(is_program(c) for c in executor.calls))

    def test_flash_readback_mismatch_is_a_failure(self):
        corrupt = bytearray(GOOD_FLASH)
        corrupt[0x80] ^= 0xFF  # first .data load byte
        executor = FakeExecutor(self.st_info, self.openocd,
                                readback=bytes(corrupt))
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertIn("readback", str(caught.exception))

    def test_flash_refuses_identity_mismatch_before_programming(self):
        executor = FakeExecutor(self.st_info, self.openocd.replace(
            "3133510a", "3133510b"))
        with self.assertRaises(fb.PolicyError):
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertFalse(any(is_program(c) for c in executor.calls))

    def test_flash_detects_an_optcr1_change(self):
        before = self.receipt + "0x40023c18: 0fff0000 \n"
        after = self.receipt + "0x40023c18: 0ffe0000 \n"
        executor = FakeExecutor(self.st_info, self.openocd,
                                openocd_outputs=[before, after])
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertIn("OPTCR1", str(caught.exception))

    def test_flash_failure_is_recorded_and_not_retried(self):
        executor = FakeExecutor(self.st_info, self.openocd, program_rc=1)
        with self.assertRaises(fb.PolicyError):
            self.board(executor).flash("bf-test", self.image_dir)
        self.assertEqual(len([c for c in executor.calls if is_program(c)]),
                         1)

    def test_flash_without_verification_banner_is_a_failure(self):
        executor = FakeExecutor(self.st_info, self.openocd, verified=False)
        with self.assertRaises(fb.PolicyError):
            self.board(executor).flash("bf-test", self.image_dir)

    def test_dry_run_executes_nothing(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        result = self.board(executor, dry_run=True).flash("bf-test",
                                                          self.image_dir)
        self.assertEqual(executor.calls, [])
        self.assertTrue(any(is_program(c) for c in result["planned"]))

    # recover --------------------------------------------------------------
    def test_recover_reads_idcode_under_reset_then_programs(self):
        self.record_baseline()
        executor = FakeExecutor(self.st_info, self.openocd)
        result = self.board(executor).recover("bf-test", self.image_dir)
        self.assertNotIn(fb.ST_INFO, [c[0] for c in executor.calls])
        first, second, third = executor.calls
        self.assertIn(fb.CONNECT_UNDER_RESET, first)
        self.assertEqual([a for a in first if a.startswith("md")],
                         ["mdw 0xe0042000"])
        self.assertTrue(is_program(second))
        self.assertIn(fb.CONNECT_UNDER_RESET, second)
        self.assertNotIn(fb.CONNECT_UNDER_RESET, third)
        self.assertIn("mdw 0x40023c14", third)
        self.assertIn("mdw 0x40023c18", third)
        run = json.loads((Path(result["run_dir"]) / "run.json").read_text())
        self.assertEqual(run["status"], "completed")
        self.assertEqual(run["optcr1_baseline"]["optcr1"], 0x0FFF0000)
        self.assertEqual(run["identity_after"]["optcr1"], 0x0FFF0000)

    def test_recover_refuses_a_foreign_idcode(self):
        self.record_baseline()
        for old, new in (("20036419", "20036413"), ("20036419", "10036419")):
            executor = FakeExecutor(self.st_info,
                                    self.openocd.replace(old, new))
            with self.assertRaises(fb.PolicyError, msg=new):
                self.board(executor).recover("bf-test", self.image_dir)
            self.assertEqual(len(executor.calls), 1)
            self.assertFalse(any(is_program(c) for c in executor.calls))

    def test_recover_refuses_an_idcode_read_failure(self):
        self.record_baseline()
        executor = FakeExecutor(self.st_info, self.openocd, openocd_rcs=[1])
        with self.assertRaises(fb.PolicyError):
            self.board(executor).recover("bf-test", self.image_dir)
        self.assertEqual(len(executor.calls), 1)

    def test_recover_refuses_without_a_probe_baseline(self):
        good = dict(schema=fb.PROBE_BASELINE_SCHEMA, serial=SERIAL,
                    optcr1=0x0FFF0000)
        for text in (None, "{", "[]",
                     json.dumps(dict(good, schema="f439-probe-baseline/v0")),
                     json.dumps(dict(good, serial="0" * 24)),
                     json.dumps(dict(good, optcr1="0x0fff0000"))):
            if text is not None:
                self.state.mkdir(parents=True, exist_ok=True)
                (self.state / fb.PROBE_BASELINE).write_text(text)
            executor = FakeExecutor(self.st_info, self.openocd)
            with self.assertRaises(fb.PolicyError, msg=text) as caught:
                self.board(executor).recover("bf-test", self.image_dir)
            self.assertIn("baseline", str(caught.exception))
            self.assertEqual(executor.calls, [], text)

    def test_recover_refuses_an_optcr1_change(self):
        self.record_baseline()
        executor = FakeExecutor(self.st_info,
                                self.receipt + "0x40023c18: 0ffe0000 \n")
        with self.assertRaises(fb.PolicyError) as caught:
            self.board(executor).recover("bf-test", self.image_dir)
        self.assertIn("FLASH_OPTCR1", str(caught.exception))
        self.assertIn("0x0fff0000 -> 0x0ffe0000", str(caught.exception))

    # probe and selfcheck -------------------------------------------------
    def test_probe_records_the_optcr1_baseline_once(self):
        self.record_baseline()
        path = self.state / fb.PROBE_BASELINE
        saved = json.loads(path.read_text())
        self.assertEqual((saved["schema"], saved["serial"], saved["optcr1"],
                          saved["optcr"]),
                         (fb.PROBE_BASELINE_SCHEMA, SERIAL, 0x0FFF0000,
                          0x0FFFAAED))
        before = path.read_bytes()
        self.record_baseline()
        self.assertEqual(path.read_bytes(), before)

    def test_probe_refuses_an_optcr1_that_differs_from_the_baseline(self):
        self.record_baseline()
        path = self.state / fb.PROBE_BASELINE
        before = path.read_bytes()
        with self.assertRaises(fb.PolicyError) as caught:
            self.record_baseline("0x40023c18: 0ffe0000 \n")
        self.assertIn("FLASH_OPTCR1", str(caught.exception))
        self.assertEqual(path.read_bytes(), before)

    def test_probe_refuses_a_missing_optcr1_read(self):
        with self.assertRaises(fb.PolicyError) as caught:
            self.record_baseline("")
        self.assertIn("FLASH_OPTCR1", str(caught.exception))
        self.assertFalse((self.state / fb.PROBE_BASELINE).exists())

    def test_selfcheck_requires_openocd_success(self):
        executor = FakeExecutor(self.st_info, SELFCHECK_OK, openocd_rcs=[1])
        with self.assertRaises(fb.PolicyError):
            self.board(executor).selfcheck()
    def test_probe_uses_hot_plug_st_info_then_openocd(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        self.board(executor).probe()
        self.assertEqual(executor.calls[0],
                         [fb.ST_INFO, "--probe", "--hot-plug"])
        self.assertEqual(executor.calls[1][0], fb.OPENOCD)

    def test_selfcheck_requires_both_empty_markers(self):
        for output, ok in [(SELFCHECK_OK, True),
                           ("", False),
                           ("F439_EXAMINE_END=<>\n", False),
                           ("F439_EXAMINE_END=<\n\tmmw 0xE0042004 7 0\n>\n"
                            "F439_RESET_INIT=<>\n", False)]:
            executor = FakeExecutor(self.st_info, output)
            board = self.board(executor)
            if ok:
                board.selfcheck()
            else:
                with self.assertRaises(fb.PolicyError, msg=output):
                    board.selfcheck()
            self.assertNotIn("init", executor.calls[0])

    # read -----------------------------------------------------------------
    def test_read_resolves_symbols_and_refuses_unknown_names(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor)
        board.read("bf-test", self.image_dir, symbols=["bf_stage"],
                   registers=["RCC_CSR"])
        reads = [c for c in executor.calls if "mdw 0x2001f000" in c]
        self.assertEqual(len(reads), 1)
        self.assertIn("mdw 0x40023874", reads[0])
        self.assertFalse(any(a.startswith(("reset", "halt", "program"))
                             for a in reads[0]))
        for kwargs in [dict(symbols=["no_such"]),
                       dict(registers=["FLASH_KEYR"]),
                       dict(registers=["USART3_SR"]),
                       dict(registers=["SYST_CSR"]),
                       dict(vectors=fb.VECTOR_COUNT + 1)]:
            with self.assertRaises(fb.PolicyError, msg=kwargs):
                board.read("bf-test", self.image_dir, **kwargs)

    def test_read_vectors_compares_the_eight_word_lines(self):
        words = struct.unpack("<16I", vectors())
        lines = ["0x%08x: %s " % (0x08000000 + 32 * i,
                                  " ".join("%08x" % w
                                           for w in words[8 * i:8 * i + 8]))
                 for i in range(2)]
        table = "\n".join(lines) + "\n"
        for text, expected in [(table, True),
                               (table.replace("20020000", "20020008"),
                                False)]:
            executor = FakeExecutor(self.st_info, self.openocd + text)
            result = self.board(executor).read("bf-test", self.image_dir,
                                               vectors=16)
            run = json.loads((Path(result["run_dir"]) / "run.json")
                             .read_text())
            self.assertIs(run["vectors_match_elf"], expected)

    # capture --------------------------------------------------------------
    def test_capture_with_reset_checks_identity_then_resets_once(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        master, slave = os.openpty()
        saved, fb.RESET_TAIL_S = fb.RESET_TAIL_S, 0.3
        try:
            result = self.board(executor).capture(
                "boot", seconds=0.6, max_bytes=100, baud=115200,
                reset_at=0.1, device=os.ttyname(slave),
                allow_any_device=True)
        finally:
            fb.RESET_TAIL_S = saved
            os.close(master)
            os.close(slave)
        self.assertEqual([c[0] for c in executor.calls], [fb.OPENOCD] * 2)
        self.assertIn("reset run", executor.calls[1])
        offset = result["summary"]["reset"]["start_offset_s"]
        self.assertGreaterEqual(offset, 0.1)
        self.assertLess(offset, 0.6)
        self.assertEqual(len(self.journal_calls), 1)
        start, end = self.journal_calls[0]
        self.assertLessEqual(start, end)
        self.assertEqual(result["summary"]["modemmanager_journal"]["status"],
                         "fake")

    def test_capture_reset_offset_is_bounded(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor)
        master, slave = os.openpty()
        try:
            with self.assertRaises(fb.PolicyError) as caught:
                board.capture("boot", seconds=2, max_bytes=10, baud=115200,
                              reset_at=1.9, device=os.ttyname(slave),
                              allow_any_device=True)
        finally:
            os.close(master)
            os.close(slave)
        self.assertIn("reset offset", str(caught.exception))
        self.assertEqual(executor.calls, [])

    def test_capture_label_is_validated_before_anything(self):
        executor = FakeExecutor(self.st_info, self.openocd)
        board = self.board(executor)
        with self.assertRaises(fb.PolicyError) as caught:
            board.capture("../../escape", seconds=1, max_bytes=10,
                          baud=115200, device="/dev/null",
                          allow_any_device=True)
        self.assertIn("label", str(caught.exception))
        self.assertEqual(executor.calls, [])

    @contextlib.contextmanager
    def pty(self):
        master, slave = os.openpty()
        try:
            yield master, slave, os.ttyname(slave)
        finally:
            os.close(master)
            os.close(slave)

    def test_capture_refuses_while_another_board_tool_runs(self):
        # A capture without a reset runs no tool, so only the preflight at
        # session start can see the other tool.
        board = self.board(FakeExecutor(self.st_info, self.openocd),
                           scan=lambda: [dict(pid=4242,
                                              argv=["/usr/bin/openocd"])])
        with self.pty() as (_, _, device):
            with self.assertRaises(fb.PolicyError) as caught:
                board.capture("cap", seconds=0.3, max_bytes=10, baud=115200,
                              device=device, allow_any_device=True)
        self.assertIn("4242", str(caught.exception))
        self.assertEqual(list(self.state.rglob("cap.bin")), [])

    def test_capture_requires_a_ttyacm_device(self):
        board = self.board(FakeExecutor(self.st_info, self.openocd))
        with self.pty() as (_, _, device):
            with self.assertRaises(fb.PolicyError) as caught:
                board.capture("cap", seconds=0.3, max_bytes=10, baud=115200,
                              device=device)
        self.assertIn("ttyACM", str(caught.exception))
        self.assertFalse((self.state / "runs").exists())

    def test_capture_refuses_and_records_a_termios_change(self):
        board = self.board(FakeExecutor(self.st_info, self.openocd))
        with self.pty() as (_, slave, device):
            def changer():
                time.sleep(0.2)
                attrs = termios.tcgetattr(slave)
                attrs[4] = attrs[5] = termios.B9600
                termios.tcsetattr(slave, termios.TCSANOW, attrs)

            thread = threading.Thread(target=changer)
            thread.start()
            try:
                with self.assertRaises(fb.PolicyError) as caught:
                    board.capture("cap", seconds=0.5, max_bytes=10,
                                  baud=115200, device=device,
                                  allow_any_device=True)
            finally:
                thread.join()
        self.assertIn("line settings changed", str(caught.exception))
        run = self.only_run("capture-cap")
        self.assertEqual(run["status"], "termios-changed")
        self.assertIs(run["capture"]["termios_unchanged"], False)
        self.assertIn("line settings changed", run["error"])

    def test_capture_refuses_and_records_a_failed_reset(self):
        executor = FakeExecutor(self.st_info, self.openocd,
                                openocd_rcs=[0, 1])
        saved, fb.RESET_TAIL_S = fb.RESET_TAIL_S, 0.3
        try:
            with self.pty() as (_, _, device):
                with self.assertRaises(fb.PolicyError) as caught:
                    self.board(executor).capture(
                        "boot", seconds=0.6, max_bytes=100, baud=115200,
                        reset_at=0.1, device=device, allow_any_device=True)
        finally:
            fb.RESET_TAIL_S = saved
        self.assertIn("reset during capture failed", str(caught.exception))
        run = self.only_run("capture-boot")
        self.assertEqual(run["status"], "reset-failed")
        self.assertEqual(run["capture"]["reset"]["returncode"], 1)


def write_capture(directory, chunks, label="cap", sends=()):
    """chunks: (t_seconds, bytes); sends: (t_seconds, bytes)."""
    directory = Path(directory)
    raw, index, offset = bytearray(), [], 0
    for t, data in chunks:
        index.append(dict(dir="rx", t_ns=int(t * 1e9), offset=offset,
                          len=len(data)))
        raw += data
        offset += len(data)
    for t, data in sends:
        index.append(dict(dir="tx", t_ns=int(t * 1e9),
                          t_end_ns=int(t * 1e9), hex=data.hex()))
    index.sort(key=lambda e: e["t_ns"])
    (directory / (label + ".bin")).write_bytes(bytes(raw))
    (directory / (label + ".index.jsonl")).write_text(
        "".join(json.dumps(e) + "\n" for e in index))
    return directory / (label + ".bin")


class AnalysisTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.dir = Path(self.tmp.name)

    def tearDown(self):
        self.tmp.cleanup()

    def test_lines_take_the_arrival_time_of_their_last_byte(self):
        raw = write_capture(self.dir, [(1.0, b"junk\xffBF1 R seq=1 csr=0x0c"),
                                       (1.5, b"000000\r\nBF1 R seq=2\r\n")])
        lines = fb.capture_lines(raw)
        records = [l for l in lines if l["tag"] == "R"]
        self.assertEqual([r["fields"]["seq"] for r in records], [1, 2])
        self.assertEqual(records[0]["fields"]["csr"], 0x0C000000)
        self.assertAlmostEqual(records[0]["t_s"], 1.5)

    def test_tick_fit_recovers_a_known_clock(self):
        period = 16_000_000 / 15_920_000  # HSI 0.5 % slow
        chunks = [(0.010 + n * period + (0.0007 if n % 3 else 0.0),
                   ("BF1 T n=%d cyc=0x%08x d=%d skipped=0\r\n"
                    % (n, (n * 16_000_000) & 0xFFFFFFFF,
                       16_000_000)).encode()) for n in range(1, 61)]
        fit = fb.fit_ticks(fb.capture_lines(write_capture(self.dir, chunks)),
                           reload=15_999_999, divider=1)
        self.assertEqual(fit["ticks"], 60)
        self.assertAlmostEqual(fit["derived_core_hz"], 15_920_000, delta=200)
        self.assertLess(fit["residual_max_s"], 0.001)
        self.assertEqual(fit["cyccnt_delta_min"], 16_000_000)
        self.assertEqual(fit["clean_ticks"], 30)
        self.assertEqual(fit["kind"], "derived-estimate")

    def test_starvation_interval_runs_from_last_feed_to_boot(self):
        lines = [(5.5, "BF1 W win=11 feed=0 starving=1"),
                 (6.0, "BF1 W win=12 feed=1 starving=1"),
                 (9.0, "BF1 W win=18 feed=1 starving=1"),
                 (9.6, "BF1 BOOT mode=2 csr=0x24000000")]
        raw = write_capture(self.dir, [(t, (l + "\r\n").encode())
                                       for t, l in lines])
        result = fb.starvation_intervals(fb.capture_lines(raw))
        self.assertEqual(len(result), 1)
        self.assertAlmostEqual(result[0]["interval_s"], 4.1)
        self.assertTrue(result[0]["iwdgrstf"])
        self.assertAlmostEqual(result[0]["derived_lsi_hz"],
                               512 * 256 / 4.1, places=3)

    def test_stop_cycles_pair_enter_wake_byte_and_woke_line(self):
        raw = write_capture(
            self.dir,
            [(1.0, b"BF1 S ENTER n=1 cyc=0x00000000\r\n"),
             (3.6, b"BF1 S WOKE n=1 entered=0 finished=0 d=1200 sr=0x0\r\n")],
            sends=[(3.5, b"\x00")])
        cycles = fb.stop_cycles(fb.capture_lines(raw), raw)
        self.assertEqual(len(cycles), 1)
        self.assertAlmostEqual(cycles[0]["host_stop_s"], 2.5)
        self.assertEqual(cycles[0]["target_cycles"], 1200)
        self.assertLess(cycles[0]["cycles_per_host_s"], 1000)
        self.assertTrue(cycles[0]["woke_after_wake_byte"])
        self.assertTrue(cycles[0]["valid"])

    def test_stop_cycle_that_never_slept_is_flagged(self):
        raw = write_capture(
            self.dir,
            [(1.0, b"BF1 S ENTER n=1 cyc=0x00000000\r\n"),
             (1.01, b"BF1 S WOKE n=1 entered=3 finished=3 d=90\r\n")],
            sends=[(3.5, b"\x00")])
        cycles = fb.stop_cycles(fb.capture_lines(raw), raw)
        self.assertFalse(cycles[0]["woke_after_wake_byte"])
        self.assertFalse(cycles[0]["valid"])
        self.assertIn("not entered", cycles[0]["status"])

    def test_probing_is_confirmed_stopped_only_by_a_final_probing_zero(self):
        on = "BF1 E STATE probing=1 dbg=0x00000001"
        off = "BF1 E STATE probing=0 dbg=0x00000001"
        probe = "BF1 E probe=0 wfi=0 dbg=0x00000000 restored=0x00000001"
        tick = "BF1 T n=9 cyc=0x00000000 d=16000000 skipped=0"
        cases = [([on, probe, off, tick], True),
                 ([on, probe, off, tick, off], True),
                 ([on, probe, tick], False),
                 ([tick, tick], False),
                 ([on, off, probe], False),
                 ([on, off, on], False)]
        for number, (lines, stopped) in enumerate(cases):
            raw = write_capture(self.dir, [(1.0 + i, (l + "\r\n").encode())
                                           for i, l in enumerate(lines)],
                                label="p%d" % number)
            result = fb.probing_state(fb.capture_lines(raw))
            self.assertIs(result["stopped"], stopped, lines)
            self.assertEqual(result["kind"], "raw")
            with contextlib.redirect_stdout(io.StringIO()) as out:
                code = fb.main(["analyze", "probing", str(raw)])
            self.assertEqual(code, 0 if stopped else 1, lines)
            self.assertIs(json.loads(out.getvalue())["stopped"], stopped)

    def test_uart_summary_lists_every_rx_line(self):
        raw = write_capture(self.dir, [
            (1.0, b"BF1 RX v=0x00000000 e=0x00000000 cyc=0x1\r\n"),
            (1.1, b"BF1 RX v=0x00000041 e=0x00000000 cyc=0x2\r\n"),
            (1.2, b"BF1 RX v=0x00000054 e=0x00000000 cyc=0x3\r\n"),
            (1.3, b"BF1 RX v=0x000000ff e=0x00000008 cyc=0x4\r\n")])
        rx = fb.uart_summary(raw)["rx"]
        self.assertEqual([(r["v"], r["e"]) for r in rx],
                         [(0, 0), (0x41, 0), (0x54, 0), (0xFF, 8)])

    def test_uart_summary_counts_frames_and_bursts(self):
        burst = b"U" * 11520
        raw = write_capture(self.dir, [(0.5, b"BF1 R seq=1\r\n"),
                                       (1.0, burst[:6000]),
                                       (2.0, burst[6000:]),
                                       (2.1, b"BF1 BURST bytes=11520 "
                                             b"cycles=16000000\r\n")])
        summary = fb.uart_summary(raw)
        self.assertEqual(summary["bf1_lines"], 2)
        self.assertEqual(summary["burst"]["bytes"], 11520)
        self.assertAlmostEqual(summary["burst"]["host_span_s"], 1.0)
        self.assertEqual(summary["burst"]["target_cycles"], 16000000)


class RegistryTests(unittest.TestCase):
    def test_every_image_source_exists(self):
        for name, spec in fb.IMAGES.items():
            for source in fb.image_sources(name):
                self.assertTrue((ROOT / source).is_file(), (name, source))

    def test_park_image_is_registered(self):
        self.assertIn(fb.PARK_IMAGE, fb.IMAGES)

    def test_build_commands_use_relative_paths_only(self):
        for name in fb.IMAGES:
            for argv in fb.build_commands(name, Path("build/x") / name):
                for arg in argv:
                    self.assertFalse(str(ROOT) in arg, (name, arg))


# --------------------------------------------------------------------------
# Mutation run. Each mutant weakens one rule of tools/f439_board.py by an
# exact text replacement; the text must occur exactly once (else the mutant
# is reported STALE). A child process executes the mutated source as module
# f439_board (its __file__ stays the real path, so ROOT and the identity
# receipt resolve), imports this file afresh and runs the whole suite with
# failfast. A mutant is killed when the suite fails or exceeds
# MUTANT_TIMEOUT_S. The unmutated source must pass through the same path
# first. tools/f439_board.py itself is never written.
# Not mutated, with the reason: the readelf/nm build cross-checks (need the
# toolchain); the section wrap check (a wrapping section also fails the
# region check); the gate's ELF digest (verify_provenance repeats it) and
# the check_elf of the session copy (its digest equals the checked bytes);
# the owner part of the lock-file check (needs a second user); the 128-word
# symbol limit (_check_read refuses the same reads); r"st-flash" (see
# DENY_REDUNDANT).
# --------------------------------------------------------------------------
FB_PATH = Path(fb.__file__).resolve()
MUTANT_TIMEOUT_S = 120
CAPTURE_TERMIOS_REFUSAL = '''\
            if not summary["termios_unchanged"]:
                session.finish("termios-changed", capture=summary)
                raise PolicyError("line settings changed during the capture "
                                  "(another process?); data kept in %s"
                                  % session.dir)
'''
MUTANTS = [
    # ELF parsing and check_elf
    ("elf-magic", 'if data[:4] != b"\\x7fELF":', "if False:"),
    ("elf-class", "if data[4] != 1:", "if False:"),
    ("elf-endian", "if data[5] != 1:", "if False:"),
    ("elf-segment-in-file",
     "if segment.filesz and segment.offset + segment.filesz > len(data):",
     "if False:"),
    ("elf-machine", "if image.machine != 40:", "if False:"),
    ("elf-load-wrap", "if start + size > 0x100000000:", "if False:"),
    ("elf-load-in-main-flash",
     "if not MAIN_FLASH.contains(seg.paddr, seg.filesz):", "if False:"),
    ("elf-memory-image-region", "if seg.memsz and not any(",
     "if False and not any("),
    ("elf-section-region",
     "if not any(r.contains(sec.addr, sec.size) for r in allowed):",
     "if False:"),
    ("elf-nobits-ram-only", "allowed = (RAM_REGIONS if sec.type == 8",
     "allowed = ((MAIN_FLASH, *RAM_REGIONS) if sec.type == 8"),
    ("elf-loadable-required", 'refusals.append("no loadable content")',
     "pass"),
    ("elf-entry-in-flash", "if not MAIN_FLASH.contains(image.entry & ~1, 2):",
     "if False:"),
    ("elf-vector-table", 'refusals.append("no vector table at 0x08000000")',
     "pass"),
    ("elf-stack-in-ram",
     "if not any(r.start < stack <= r.end for r in RAM_REGIONS):",
     "if False:"),
    ("elf-stack-aligned", "if stack % 8:", "if False:"),
    ("elf-reset-thumb", "if not reset & 1:", "if False:"),
    ("elf-reset-in-flash", "if not MAIN_FLASH.contains(reset & ~1, 2):",
     "if False:"),
    ("region-end-bound",
     "return self.start <= start and start + size <= self.end",
     "return self.start <= start"),
    # pinned tools, other processes, environment
    ("pins-digest", "if sha256_file(path) != digest:", "if False:"),
    ("pins-missing", 'refusals.append("pinned file missing: " + name)',
     "pass"),
    ("pins-shadow", "if (Path(shadow) / relative).exists():", "if False:"),
    ("scan-tool-names",
     "if argv and os.path.basename(argv[0]) in BOARD_TOOL_NAMES:",
     "if False:"),
    ("preflight-scan", "        if others:\n", "        if False:\n"),
    ("preflight-raise", 'raise PolicyError("; ".join(refusals))', "pass"),
    ("tty-holders", "if os.path.realpath(fd) == real:", "if False:"),
    ("env-path", '"PATH": "/usr/bin:/bin"', '"PATH": os.environ["PATH"]'),
    ("env-home", '"HOME": sandbox,', ""),
    ("env-xdg-config", '"XDG_CONFIG_HOME": sandbox}', "}"),
    ("sandbox-cwd", "cwd=str(self.sandbox), env=tool_environment(",
     "cwd=None, env=tool_environment("),
    # session: command check, per-run preflight and sandbox, locks
    ("run-check-command", "        check_command(argv, approved_elf=",
     "        (lambda *a, **k: None)(argv, approved_elf="),
    ("run-preflight",
     "        self.board.preflight()\n        if os.listdir(self.sandbox):",
     "        if os.listdir(self.sandbox):"),
    ("run-sandbox-empty",
     "        if os.listdir(self.sandbox):\n            raise PolicyError("
     "\"tool sandbox is not empty: %s\" % self.sandbox)\n", ""),
    ("session-preflight", "                self.preflight()\n"
     "                yield session", "                yield session"),
    ("session-host-lock",
     "with BoardLock(host, action), BoardLock(self.lock_path, action):",
     "with BoardLock(self.lock_path, action):"),
    ("session-checkout-lock",
     "with BoardLock(host, action), BoardLock(self.lock_path, action):",
     "with BoardLock(host, action):"),
    ("session-default-host-lock", "host_lock=host_lock_path,",
     "host_lock=lambda: LOCK_PATH,"),
    ("session-terminates-children",
     "            finally:\n                terminate_live_children()\n",
     "            finally:\n                pass\n"),
    ("session-keeps-refusal-status",
     'if session.status in ("started", "completed")', "if True"),
    ("lock-taken", "fcntl.flock(fd, fcntl.LOCK_EX | fcntl.LOCK_NB)", "pass"),
    ("lock-nofollow", "os.O_RDWR | os.O_CREAT | os.O_NOFOLLOW |",
     "os.O_RDWR | os.O_CREAT |"),
    ("lock-regular-file",
     "if not stat.S_ISREG(info.st_mode) or info.st_uid != os.geteuid() \\",
     "if False \\"),
    ("lock-single-link", "or info.st_nlink != 1 or info.st_mode & 0o022:",
     "or info.st_mode & 0o022:"),
    ("lock-not-shared", "or info.st_nlink != 1 or info.st_mode & 0o022:",
     "or info.st_nlink != 1:"),
    ("lock-not-replaced",
     "if now is None or (now.st_dev, now.st_ino) != (info.st_dev,",
     "if now is None and (now.st_dev, now.st_ino) != (info.st_dev,"),
    ("hostdir-real-owned",
     "if not stat.S_ISDIR(info.st_mode) or info.st_uid not in (0, uid) \\",
     "if False \\"),
    ("hostdir-not-shared", "                or (shared and not sticky_root):",
     "                or False:"),
    ("hostdir-sticky-root",
     "sticky_root = info.st_uid == 0 and info.st_mode & stat.S_ISVTX",
     "sticky_root = info.st_mode & stat.S_ISVTX"),
    ("hostdir-order", 'HOST_LOCK_DIRS = ("/run/lock", "/run/user/{uid}")',
     'HOST_LOCK_DIRS = ("/run/user/{uid}", "/run/lock")'),
    # command policy
    ("argv-text",
     "if not argv or not all(isinstance(a, str) for a in argv):",
     "if not argv:"),
    ("deny-applied", '    if pattern:\n        raise PolicyError("denylisted',
     '    if False:\n        raise PolicyError("denylisted'),
    ("st-info-args", 'if argv != [ST_INFO, "--probe", "--hot-plug"]:',
     "if False:"),
    ("tool-allowlist", "    if tool != OPENOCD:\n", "    if False:\n"),
    ("openocd-prefix",
     'if argv[1:5] != ["-s", str(OPENOCD_SCRIPTS), "-f", BOARD_CFG]:',
     "if False:"),
    ("openocd-c-pairs",
     'if len(rest) % 2 or any(rest[i] != "-c" for i in range(0, len(rest), '
     '2)):', "if False:"),
    ("openocd-header", "    if commands[:6] != [",
     "    if False and commands[:6] != ["),
    ("openocd-body-allowlist", "    for command in body:\n",
     "    for command in body[:0]:\n"),
    ("read-flash-count",
     "if count <= 128 and MAIN_FLASH.contains(address, size):",
     "if MAIN_FLASH.contains(address, size):"),
    ("read-register-single-word", 'if match[1] == "mdw" and count == 1 and \\',
     'if match[1] == "mdw" and \\'),
    ("read-symbol-in-ram", "for r in RAM_REGIONS) and \\",
     "for r in READABLE_REGIONS) and \\"),
    ("read-symbol-bound",
     "start <= address and address + size <= start + length:",
     "start <= address:"),
    ("dump-in-session", "if not _inside(match[1], run_dir):", "if False:"),
    ("dump-flash-only",
     "if size == 0 or not MAIN_FLASH.contains(address, size):", "if False:"),
    ("program-in-session",
     "if approved_elf is None or not _inside(match[1], run_dir) or \\",
     "if approved_elf is None or \\"),
    ("program-approved-copy",
     "Path(match[1]).resolve() != Path(approved_elf).resolve():", "False:"),
    ("program-exact-form",
     're.fullmatch(r"program \\{([^{}]+)\\} verify reset exit", command)',
     're.match(r"program \\{([^{}]+)\\} verify reset exit", command)'),
    ("program-alone", "if len(body) != 1:", "if False:"),
    ("reset-config-first", "if CONNECT_UNDER_RESET in body:", "if False:"),
    ("access-after-init", 'raise PolicyError("target access before init")',
     "pass"),
    ("shutdown-last", 'if "shutdown" in body and body[-1] != "shutdown":',
     "if False:"),
    ("serial-format",
     "if not isinstance(serial, str) or not SERIAL_RE.fullmatch(serial):",
     "if not isinstance(serial, str):"),
    ("label-format",
     "if not isinstance(label, str) or not LABEL_RE.fullmatch(label):",
     "if not isinstance(label, str):"),
    # identity
    ("identity-refusal", '        if problems:\n            raise PolicyError('
     '"identity mismatch: "', '        if False:\n            raise '
     'PolicyError("identity mismatch: "'),
    ("identity-read-failed",
     'problems.append("openocd identity read failed")', "pass"),
    ("identity-programmers",
     'compare("ST-LINK programmers", observed["programmers"], 1)', "pass"),
    ("identity-serial", 'compare("ST-LINK serial", observed["stlink_serial"],',
     'compare("ST-LINK serial", expected["serial"],'),
    ("identity-chipid",
     'compare("st-info chipid", observed["st_info_chipid"],',
     'compare("st-info chipid", "0x%03x" % expected["dev_id"],'),
    ("identity-flash-bytes", 'compare("st-info flash bytes", '
     'observed["st_info_flash_bytes"],',
     'compare("st-info flash bytes", expected["flash_kib"] * 1024,'),
    ("identity-dev-id", 'compare("DBGMCU_IDCODE DEV_ID", observed["dev_id"],',
     'compare("DBGMCU_IDCODE DEV_ID", expected["dev_id"],'),
    ("identity-rev-id", 'compare("DBGMCU_IDCODE REV_ID", observed["rev_id"],',
     'compare("DBGMCU_IDCODE REV_ID", expected["rev_id"],'),
    ("identity-uid", 'compare("UID", observed["uid"], expected["uid"], hexs)',
     "pass"),
    ("identity-flash-kib", 'compare("flash size KiB", observed["flash_kib"], '
     'expected["flash_kib"])', "pass"),
    ("identity-optcr",
     'compare("FLASH_OPTCR", observed["optcr"], expected["optcr"], hexs)',
     "pass"),
    # offline gate, programming, readback, option registers
    ("gate-image-name", 'if manifest.get("image") != name:', "if False:"),
    ("gate-elf-check", 'refusals += check_elf(data)["refusals"]', "pass"),
    ("gate-self-file",
     "refusals += verify_sources({p: None for p in SELF_FILES}, self.repo)",
     "pass"),
    ("gate-provenance", "        if not refusals:\n            refusals += "
     "verify_provenance(", "        if False:\n            refusals += "
     "verify_provenance("),
    ("gate-hashed-bytes", "return elf.resolve(), manifest, parse_elf(data)",
     "return elf.resolve(), manifest, parse_elf(elf.read_bytes())"),
    ("copy-rehash", 'if sha256_file(copy) != manifest["elf_sha256"] or \\',
     'if False and sha256_file(copy) != manifest["elf_sha256"] or \\'),
    ("program-returncode",
     'record["timed_out"] or record["returncode"] != 0 or',
     'record["timed_out"] or'),
    ("program-verified", '"** Verified OK **" not in session.text(record)',
     "False"),
    ("readback-compare", "if len(data) != size or differences:", "if False:"),
    ("flash-optcr1",
     'if not self.dry_run and before["optcr1"] != after["optcr1"]:',
     "if False:"),
    ("recover-dev-id", 'idcode & 0xFFF != self.expected["dev_id"] or \\',
     "False or \\"),
    ("recover-rev-id", 'idcode >> 16 != self.expected["rev_id"]:', "False:"),
    ("recover-idcode-returncode",
     'if record["returncode"] != 0 or idcode is None or \\',
     "if idcode is None or \\"),
    ("recover-baseline-required", "        baseline = self.optcr1_baseline()\n"
     "        with self.session(\"recover\"", "        baseline = dict("
     "optcr1=None)\n        with self.session(\"recover\""),
    ("recover-optcr1",
     'if not self.dry_run and after["optcr1"] != baseline["optcr1"]:',
     "if False:"),
    ("baseline-schema", 'record.get("schema") != PROBE_BASELINE_SCHEMA or \\',
     "False or \\"),
    ("baseline-serial", 'record.get("serial") != self.serial or \\',
     "False or \\"),
    ("probe-baseline-compare", 'if baseline["optcr1"] != observed["optcr1"]:',
     "if False:"),
    ("probe-optcr1-read", 'if observed["optcr1"] is None:\n            raise',
     'if False:\n            raise'),
    ("probe-records-baseline", "baseline = None if self.dry_run else \\\n"
     "                self._check_baseline(observed, session)",
     "baseline = None"),
    ("selfcheck-returncode", 'if record["returncode"] != 0 or \\\n'
     '                    "F439_EXAMINE_END', 'if False or \\\n'
     '                    "F439_EXAMINE_END'),
    ("selfcheck-examine-end", '"F439_EXAMINE_END=<>" not in lines or \\',
     "False or \\"),
    ("selfcheck-reset-init", '"F439_RESET_INIT=<>" not in lines:', "False:"),
    ("reset-returncode",
     'if not self.dry_run and record["returncode"] != 0:\n'
     '                raise PolicyError("reset failed")',
     'if False:\n                raise PolicyError("reset failed")'),
    # capture
    ("capture-reset-offset",
     "0 <= reset_at and reset_at + RESET_TAIL_S <= seconds):",
     "0 <= reset_at):"),
    ("capture-ttyacm", "if not allow_any_device and \\", "if False and \\"),
    ("capture-reset-failed", 'if reset.get("returncode") != 0:', "if False:"),
    ("capture-termios-refusal", CAPTURE_TERMIOS_REFUSAL, ""),
    ("capture-holders",
     '    if holders:\n        raise PolicyError("serial port already open',
     '    if False:\n        raise PolicyError("serial port already open'),
    ("capture-tiocgexcl", '        state = array.array("i", [0])\n'
     "        fcntl.ioctl(fd, TIOCGEXCL, state, True)\n"
     "        return state[0] != 0\n", "        return True\n"),
    ("capture-exclusive-refusal", "    if not _exclusive(fd):\n",
     "    if False:\n"),
    ("capture-release-on-refusal",
     "    if not _exclusive(fd):\n        _release(fd)\n",
     "    if not _exclusive(fd):\n        os.close(fd)\n"),
    ("capture-seconds-bound", "if not 0 < seconds <= MAX_CAPTURE_SECONDS:",
     "if not 0 < seconds:"),
    ("capture-bytes-bound", "if not 0 < max_bytes <= MAX_CAPTURE_BYTES:",
     "if not 0 < max_bytes:"),
    ("capture-line-setting",
     'if baud not in BAUDS or parity not in ("none", "even", "odd"):',
     "if False:"),
    ("capture-send-count", "if len(sends) > MAX_SENDS or any(", "if any("),
    ("capture-send-offset",
     "not 0 <= t <= seconds or not 0 < len(b) <= MAX_SEND_BYTES",
     "not 0 < len(b) <= MAX_SEND_BYTES"),
    ("capture-send-size",
     "not 0 <= t <= seconds or not 0 < len(b) <= MAX_SEND_BYTES",
     "not 0 <= t <= seconds or not 0 < len(b)"),
    ("capture-read-bound",
     "chunk = os.read(fd, min(4096, max_bytes - total))",
     "chunk = os.read(fd, 4096)"),
    ("capture-byte-stop", "                if total >= max_bytes:\n",
     "                if False:\n"),
    ("capture-time-stop", "                if now_ns >= seconds * 1e9:\n"
     "                    break\n", ""),
    # bounded execution
    ("run-timeout", "stdout, stderr = process.communicate(timeout=timeout_s)",
     "stdout, stderr = process.communicate()"),
    ("run-own-group", "start_new_session=True)", "start_new_session=False)"),
    ("terminate-children", "    for process in processes:\n        _stop(",
     "    for process in processes:\n        (lambda p: None)("),
    # provenance
    ("sources-inside-repository",
     'if candidate.is_absolute() or ".." in candidate.parts or \\',
     "if False and \\"),
    ("sources-built-bytes", "if not full.is_file() or (digest is not None and",
     "if not full.is_file() or (False and"),
    ("sources-tracked",
     'if _git(repo, "ls-files", "--error-unmatch", "--", path).returncode:',
     "if False:"),
    ("sources-clean",
     'elif _git(repo, "status", "--porcelain", "--", path).stdout.strip():',
     "elif False:"),
    ("provenance-schema", 'if manifest.get("schema") != MANIFEST_SCHEMA:',
     "if False:"),
    ("provenance-elf-digest", 'if not elf.is_file() or sha256_file(elf) != '
     'manifest.get("elf_sha256"):', "if False:"),
    ("provenance-rebuild-identical",
     'if rebuilt.get("elf_sha256") != manifest.get("elf_sha256"):',
     "if False:"),
    ("provenance-dependency-set",
     'if rebuilt.get("sources") != manifest.get("sources"):', "if False:"),
    ("provenance-sources-required", "    if not sources:\n        refusals.",
     "    if False:\n        refusals."),
    ("provenance-sources-checked",
     "    refusals += verify_sources(sources, repo)\n", ""),
    # read action
    ("read-vector-count", "if not 0 < vectors <= VECTOR_COUNT:", "if False:"),
    ("read-symbol-known", "if symbol not in image.symbols:", "if False:"),
    ("read-register-allowlist", "if register not in READ_REGISTERS:",
     "if False:"),
    # pre-registered analysis that gates the next attach
    ("probing-final-zero", "stopped=final == 0 and not late)",
     "stopped=not late)"),
    ("probing-no-later-probe", "stopped=final == 0 and not late)",
     "stopped=final == 0)"),
    ("probing-exit-code", 'return 1 if args.kind == "probing" and not '
     'result["stopped"] \\', 'return 0 if args.kind == "probing" and not '
     'result["stopped"] \\'),
]
MUTANTS += [("deny %s" % pattern, 'r"%s"' % pattern, 'r"(?!)"')
            for pattern in fb.DENY_PATTERNS if pattern not in DENY_REDUNDANT]


def run_one_mutant(index):
    """Child process. Exit 0: suite passed; 1: suite failed; 2: stale
    mutant; 3: the mutated module does not load."""
    source = FB_PATH.read_text()
    if index >= 0:
        _, old, new = MUTANTS[index]
        if source.count(old) != 1:
            print("STALE: text occurs %d times" % source.count(old))
            return 2
        source = source.replace(old, new)
    module = types.ModuleType("f439_board")
    module.__file__ = str(FB_PATH)
    sys.modules["f439_board"] = module
    try:
        exec(compile(source, str(FB_PATH), "exec"), module.__dict__)
    except Exception as error:  # a broken mutant, not a kill
        print("LOAD-ERROR: %r" % error)
        return 3
    spec = importlib.util.spec_from_file_location(
        "f439_board_mutant_tests", Path(__file__).resolve())
    tests = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = tests
    spec.loader.exec_module(tests)
    result = unittest.TextTestRunner(stream=io.StringIO(), failfast=True,
                                     verbosity=0).run(
        unittest.defaultTestLoader.loadTestsFromModule(tests))
    for test, _ in result.failures + result.errors:
        print("FAILED-TEST: %s" % test.id().split(".", 1)[-1])
    print("RAN: %d" % result.testsRun)
    return 0 if result.wasSuccessful() else 1


def _child(index):
    process = subprocess.Popen(
        [sys.executable, str(Path(__file__).resolve()), "--run-mutant",
         str(index)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, start_new_session=True)
    try:
        out, _ = process.communicate(timeout=MUTANT_TIMEOUT_S)
        return process.returncode, out
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        out, _ = process.communicate()
        return "timeout", out


def run_mutants(jobs=3, only=None):
    names = [name for name, _, _ in MUTANTS]
    duplicates = sorted({n for n in names if names.count(n) > 1})
    if duplicates:
        print("duplicate mutant names: %s" % duplicates)
        return 1
    rc, out = _child(-1)
    print("baseline (unmutated): %s" % ("passed" if rc == 0 else
                                         "FAILED %s\n%s" % (rc, out)))
    if rc != 0:
        return 1
    chosen = [i for i, name in enumerate(names) if only is None or
              only in name]
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        outcomes = list(pool.map(_child, chosen))
    killed, problems = 0, []
    for index, (rc, out) in zip(chosen, outcomes):
        failed = [l.split(": ", 1)[1] for l in out.splitlines()
                  if l.startswith("FAILED-TEST: ")]
        status = {0: "SURVIVED", 1: "killed", "timeout": "killed",
                  2: "STALE", 3: "LOAD-ERROR"}.get(rc, "ERROR %s" % rc)
        detail = "timeout %d s" % MUTANT_TIMEOUT_S if rc == "timeout" \
            else (failed[0] if failed else
                  " ".join(out.strip().splitlines()[-1:]))
        print("%3d %-10s %-34s %s" % (index, status, names[index], detail))
        if status == "killed":
            killed += 1
        else:
            problems.append(names[index])
    print("killed %d/%d mutants of %s%s" % (
        killed, len(chosen), FB_PATH.relative_to(ROOT),
        "; not killed: " + ", ".join(problems) if problems else ""))
    return 0 if killed == len(chosen) else 1


if __name__ == "__main__":
    if sys.argv[1:2] == ["--run-mutant"]:
        sys.exit(run_one_mutant(int(sys.argv[2])))
    if sys.argv[1:2] == ["--mutants"]:
        options = dict(zip(sys.argv[2::2], sys.argv[3::2]))
        sys.exit(run_mutants(int(options.get("--jobs", 3)),
                             options.get("--only")))
    unittest.main(verbosity=1)
