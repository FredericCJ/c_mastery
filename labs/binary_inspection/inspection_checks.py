"""Host and target checks for evidence submitted in the inspection workspace."""
from datetime import datetime
import importlib.util
import json
import re
import struct
import subprocess
import tempfile
import time
from pathlib import Path

from inspect_lab import LAB, digest, expect, invoke, symbol, workspace_file


def stages(row, out, compiler):
    expect(row, "object_sha256", digest(out / "driver.o"))
    expect(row, "output", invoke([out / "records"]).strip())
    names = invoke(["nm", "-u", out / "driver.o"])
    reference = row.get("unresolved_call")
    if reference != "record_score" or reference not in names.split():
        raise ValueError("Identify the unresolved cross-unit call")
    if "RECORD_WEIGHT" in (out / "record.i").read_text():
        raise ValueError("Unexpected preprocessing result")
    # Re-link independently: the missing provider must cause a linker diagnostic.
    with tempfile.TemporaryDirectory() as tmp:
        result = subprocess.run([compiler, "-pie", out / "driver.o", "-o", Path(tmp)/"missing"],
                                capture_output=True)
        if not result.returncode or reference.encode() not in result.stderr:
            raise ValueError("Could not reproduce the missing definition")
    expect(row, "missing_provider_stage", "link")


def section(image, name):
    fields = invoke(["readelf", "-SW", image])
    match = re.search(r"\[\s*\d+\]\s+" + re.escape(name) +
                      r"\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)", fields)
    if not match:
        raise ValueError(f"Missing section {name}")
    kind, address, offset, size = match.groups()
    return kind, int(address,16), int(offset,16), int(size,16)


def byte_location(image):
    _, addr, offset, size = section(image, ".text")
    pc = symbol(image, "record_score")
    if not addr <= pc <= addr + size - 8:
        raise ValueError("Function outside file-backed text")
    return pc, offset + pc - addr


def instruction_bytes(row, out):
    image = out / "records"
    pc, offset = byte_location(image)
    expect(row, "elf_sha256", digest(image))
    expect(row, "function_va", pc)
    expect(row, "file_offset", offset)
    dump = invoke(["hexdump", "-s", str(offset), "-n", "8", "-ve", '1/1 "%02x"', image])
    disasm = invoke(["objdump", "-d", "--disassemble=record_score", image])
    listing = b"".join(bytes.fromhex(m) for m in re.findall(
        r"^\s*[0-9a-f]+:\s+((?:[a-f0-9]{2} )+)", disasm, re.M))
    if listing[:8].hex() != dump:
        raise ValueError("Disassembly and file slice disagree")
    expect(row, "first_8_hex", dump)


def linking(row, out):
    listing = invoke(["readelf", "-Wr", out / "driver.o"])
    match = re.search(r"^([0-9a-f]+)\s+\S+\s+(R_\S+)\s+\S+\s+record_score\b", listing, re.M)
    if not match:
        raise ValueError("Missing cross-unit relocation")
    expect(row, "relocation_offset", int(match[1],16))
    expect(row, "relocation_type", match[2])
    expect(row, "bss_type", section(out / "records", ".bss")[0])
    expect(row, "elf_sha256", digest(out / "records"))
    elf = invoke(["readelf", "-lW", out / "records"])
    seg = next(line.split() for line in elf.splitlines() if line.strip().startswith("LOAD") and "RW" in line)
    expect(row, "rw_file_size", int(seg[4],16))
    expect(row, "rw_memory_size", int(seg[5],16))


def stopped_call(image):
    output = invoke(["gdb", "-nx", "-batch", image, "-ex", "break *record_score", "-ex", "run",
                     "-ex", 'printf "CALL=%u,%u\\n",$edi,$esi', "-ex", "disable 1", "-ex", "finish",
                     "-ex", 'printf "RETURN=%u\\n",$eax'])
    args = re.search(r"^CALL=(\d+),(\d+)$", output, re.M)
    ret = re.search(r"^RETURN=(\d+)$", output, re.M)
    if not args or not ret:
        raise ValueError("Missing GDB call or return observation")
    return [*map(int,args.groups()),int(ret[1])]


def debugger(row, out):
    image = out / "optimized"
    expect(row, "debug_args_and_return", stopped_call(out / "records"))
    expect(row, "optimized_args_and_return", stopped_call(image))
    if invoke([out/"stripped"]) != invoke([image]):
        raise ValueError("Stripped image behaves differently on the supplied records")
    annotation = json.loads(workspace_file(row["annotation"]).read_text())
    expect(annotation, "exe_sha256", digest(out/"stripped"))
    expect(annotation, "rva", symbol(image, "record_score"))
    if (any(not isinstance(annotation.get(k), str) or not annotation[k].strip()
            for k in ("name", "comment", "decompiled"))
            or annotation["name"].startswith("FUN_") or annotation["name"] not in annotation["decompiled"]):
        raise ValueError("Rename the function, add its plate comment, and export the decompiler view")


def target_observation(image):
    with tempfile.TemporaryDirectory() as tmp:
        sock = Path(tmp)/"arm.sock"
        args = ["qemu-system-arm", "-M", "mps2-an386", "-cpu", "cortex-m4", "-display","none",
                "-serial","none","-monitor","none","-kernel",image,"-S","-gdb",
                f"unix:{sock},server=on,wait=off"]
        with subprocess.Popen(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL) as proc:
            try:
                for _ in range(200):
                    if sock.exists() or proc.poll() is not None:
                        break
                    time.sleep(.02)
                output = invoke(["gdb-multiarch","-nx","-batch",image,"-ex",f"target remote {sock}",
                                 "-ex",'printf "RESET=%u,%u,%u\\n",$sp,$pc,($xpsr >> 24) & 1',
                                 "-ex","break *result_ready","-ex","continue",
                                 "-ex",'printf "RESULT=%u,%u,%u\\n",initial_size,record_calls,observed'])
            finally:
                proc.terminate()
        reset = re.search(r"^RESET=(\d+),(\d+),(\d+)$", output, re.M)
        result = re.search(r"^RESULT=(\d+),(\d+),(\d+)$", output, re.M)
        if not reset or not result:
            raise ValueError("Missing reset or post-startup observation")
        return tuple(map(int, reset.groups())), list(map(int, result.groups()))


def cortex(row, out):
    image = out / "target.elf"
    expect(row, "elf_sha256", digest(image))
    if invoke(["arm-none-eabi-nm","-u",image]).strip():
        raise ValueError("Undefined symbol in freestanding image")
    _, _, offset, size = section(image, ".vectors")
    if size < 8:
        raise ValueError("Incomplete vector table")
    initial_sp, reset_entry = struct.unpack_from("<II", image.read_bytes(), offset)
    observed_reset, values = target_observation(image)
    pc = symbol(image, "reset", "arm-none-eabi-")
    if reset_entry != (pc | 1) or observed_reset != (initial_sp, pc, 1):
        raise ValueError("Image and observed reset state disagree")
    expect(row,"initial_sp",initial_sp)
    expect(row,"reset_vector",reset_entry)
    expect(row,"post_startup",values)


def verify_count_stream(document):
    """Check the independently captured command outputs against every judged row."""
    request = document["request"]
    trials = []
    for attempt in document["calibration"]["attempts"]:
        pair = attempt["trials"]
        if [r["variant"] for r in pair] != [0, 1] or any(r["repetitions"] != attempt["repetitions"] for r in pair):
            raise ValueError("Calibration trials do not describe both requested variants")
        trials.extend(pair)
    rows = document["rows"]
    pairs = request["pairs"]
    if type(pairs) is not int or pairs <= 0 or len(rows) != 2 * pairs:
        raise ValueError("Requested pairs disagree with counting rows")
    for index, sample in enumerate(rows):
        pair, order = divmod(index, 2)
        for key, expected in (("pair", pair), ("order", order), ("variant", order ^ (pair & 1))):
            expect(sample, key, expected)

    affinity = document["host"]["affinity"]
    cpu = document["pmu_domain"]["cpu"]
    if not affinity or (cpu is not None and affinity != [cpu]):
        raise ValueError("PMU placement disagrees with recorded affinity")
    samples = trials + rows
    if len(document["commands"]) != len(samples):
        raise ValueError("Missing or extra raw native collector outputs")
    for sample, command in zip(samples, document["commands"]):
        expect(command, "exit_code", 0)
        expected = ["count", request["family"], request["n"], sample["variant"], sample["repetitions"]]
        expect(command, "argv", expected)
        raw = json.loads(command["stdout"])
        if raw != {k: v for k, v in sample.items() if k not in ("pair", "order")}:
            raise ValueError("Judged count disagrees with original collector output")
        for key, value in (("kind", "count"), ("family", request["family"]), ("n", request["n"]),
                           ("scope", "calling-thread-user"), ("acquisition", "strict-group"),
                           ("strict_group", True), ("status", "measured"), ("errno", 0), ("cleanup_errno", 0)):
            expect(raw, key, value)
        if not (0 < raw["running"] <= raw["enabled"] and raw["raw_instructions"] > 0 and raw["raw_cycles"] > 0
                and raw["instruction_id"] != raw["cycle_id"] and raw["instruction_id"] > 0 and raw["cycle_id"] > 0):
            raise ValueError("Missing scheduled user instructions/cycles or event identities")
        if raw["cpu_before"] not in affinity or raw["cpu_after"] not in affinity:
            raise ValueError("Collector CPU endpoints fall outside the recorded domain")


def verify_live_build(document, image, native):
    manifest = json.loads(workspace_file((image.parent / "build.json").relative_to(LAB)).read_text())
    expect(manifest, "kind", "native-build")
    expect(manifest, "provenance", "LEARNER")
    expect(manifest, "instrumented", False)
    if datetime.fromisoformat(document["created_utc"]) < datetime.fromisoformat(manifest["created_utc"]):
        raise ValueError("Observation predates its measured build")
    if Path(manifest["binary"]) != image or native.instrumented(image):
        raise ValueError("Use the original uninstrumented current-workspace collector")
    for key in ("binary_sha256", "components", "flags"):
        expect(document["build"], key, manifest[key])
    expect(manifest, "binary_sha256", digest(image))
    roles = [part["role"] for part in manifest["components"]]
    if roles.count("pmu") != 1 or len(set(roles)) != len(roles):
        raise ValueError("Expected one learner PMU component and unique component roles")
    for part in manifest["components"]:
        expect(part, "label", "LEARNER")
        source = workspace_file(Path(part["path"]).relative_to(LAB))
        expect(part, "sha256", digest(source))
    expected_files = {p.relative_to(native.LABS).as_posix() for p in [*native.SUPPLIED_COLLECTOR,
                                                                       native.HERE / "pmu_domain.h"]}
    adapters = manifest["supplied_adapters"]
    if len(adapters) != len(expected_files) or {part["file"] for part in adapters} != expected_files:
        raise ValueError("Collector manifest does not identify its supplied adapters")
    for part in adapters:
        expect(part, "label", "SUPPLIED")
        expect(part, "sha256", digest(workspace_file(Path("labs") / part["file"])))
    available = [f for f in manifest["families"] if f["family"] == document["request"]["family"] and f["available"]]
    if len(available) != 1:
        raise ValueError("Measured workload family missing from this executable")


def pmu_result(row, out, *, live):
    folder = LAB / "labs/performance_followthrough"
    path = workspace_file(row["report"]) if live else folder / "dossiers/replay/layout.json"
    document = json.loads(path.read_text())
    if document.get("kind") != "native-run":
        raise ValueError("Counting report required")
    loader = importlib.util.spec_from_file_location("learner_native", folder / "native.py")
    native = importlib.util.module_from_spec(loader)
    loader.loader.exec_module(native)
    _, gates = native.recheck(document)
    expect(row, "report_sha256", digest(path))
    expect(row, "pairs", gates["pairs"])
    expect(row, "interpretation", gates["gates"]["interpretation"])
    if live:
        expect(document, "provenance", "LEARNER")
        if gates["gates"]["acquisition"] != "valid" or not document.get("host"):
            raise ValueError("Native hardware acquisition remains pending")
        image = workspace_file(row["measured_image"])
        verify_live_build(document, image, native)
        verify_count_stream(document)
        host = native.host_facts()
        for key in ("cpu_model", "kernel", "machine"):
            expect(document["host"], key, host[key])
        domain = native.select_core(document["pmu_domain"]["cpu"], set(document["host"]["affinity"]))
        expect(document, "pmu_domain", domain)
        listing = invoke(["objdump","-d",image])
        if not row.get("measured_function") or ("<" + row["measured_function"] + ">:") not in listing:
            raise ValueError("Locate the observed function in the measured binary")
    else:
        ring = row.get("wrapped_sample", {})
        case = json.loads(workspace_file("fixtures/binary_inspection/records.json").read_text())["ring"]
        if not 0 < case["record_size"] <= case["head"] - case["tail"] <= case["capacity"]:
            raise ValueError("Fixture must describe one available complete record")
        expect(ring, "physical_header_offset", case["tail"] % case["capacity"])
        expect(ring, "next_tail", case["tail"] + case["record_size"])
        expect(ring, "kernel_loss_without_count", "unknown")


CHECKS = {"stages": stages, "bytes": instruction_bytes, "linking": linking,
          "debugger": debugger, "cortex": cortex,
          "replay": lambda row,out: pmu_result(row,out,live=False),
          "live": lambda row,out: pmu_result(row,out,live=True)}
