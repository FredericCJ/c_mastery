"""Fresh checks of the inspection scaffold. Generated answers exist only in temporary author workspaces."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def execute(args, cwd, *, ok=True, env=None, timeout=60):
    result = subprocess.run(list(map(str, args)), cwd=cwd, capture_output=True, text=True,
                            timeout=timeout, env=env)
    if (result.returncode == 0) != ok:
        raise ValueError(f"{args[0]} return {result.returncode}: {result.stdout[-1400:]} {result.stderr[-1400:]}")
    return result.stdout if ok else result.stderr


def dump(path, data):
    path.write_text(json.dumps(data, indent=2) + "\n")


def nm(image, name, prefix=""):
    names = execute([prefix + "nm", "-n", image], image.parent)
    return int(next(line.split()[0] for line in names.splitlines() if line.endswith(" "+name)), 16)


def address_fields(image, section):
    headers = execute(["objdump", "-h", image], image.parent)
    fields = next(line.split() for line in headers.splitlines()
                  if len(line.split()) > 5 and line.split()[1] == section)
    return int(fields[3], 16), int(fields[5], 16)


def annotate(work, out, install):
    scripts = work / "reference-scripts"
    scripts.mkdir(exist_ok=True)
    shutil.copy(ROOT / "assessor/binary_inspection/AnnotateReference.java", scripts)
    shutil.copy(work / "ghidra/ExportInspection.java", scripts)
    env = dict(os.environ, JAVA_HOME=str(install/"jdk25"),
               PATH=str(install/"jdk25/bin") + os.pathsep + os.environ["PATH"])
    ghidra = install / "ghidra_12.2_PUBLIC/support/analyzeHeadless"
    rva = hex(nm(out/"optimized", "record_score"))
    export = work / "answers/annotation.json"
    run = execute([ghidra, work, "author-project", "-import", out/"stripped", "-scriptPath",
                   scripts, "-postScript", "AnnotateReference.java", rva,
                   "-postScript", "ExportInspection.java", export, rva, "-deleteProject"],
                  work, env=env, timeout=180)
    if not export.exists():
        raise ValueError("Ghidra annotation export failed:\n" + run[-5000:])
    return export.relative_to(work).as_posix()


def exercise(work, compiler, case, fields, checks, *, bad_key, bad_value):
    path = work / "answers" / (case + ".json")
    path.unlink(missing_ok=True)
    command = [sys.executable, "inspect_lab.py", "check", case, "--cc", compiler]
    rejection = execute(command, work, ok=False)
    if "Prediction and observations required" not in rejection:
        raise ValueError("Missing submission was rejected for an unrelated reason")
    response = dict(prediction="Author fixture prediction.", explanation="Author fixture comparison.", **fields)
    dump(path, response)
    execute(command, work)
    dump(path, dict(response, **{bad_key: bad_value}))
    if "Evidence disagrees" not in execute(command, work, ok=False):
        raise ValueError("Incorrect observation was rejected for an unrelated reason")
    checks.append({"compiler": compiler, "case": case, "fresh_positive": True,
                   "empty_submission_rejected": True, "changed_field_rejected": bad_key})


def verify(args):
    checks = []
    builds = {}
    assets = ROOT / "labs/binary_inspection"
    inputs = {p.relative_to(ROOT).as_posix(): sha(p) for p in assets.rglob("*")
              if p.is_file() and not {"build", "__pycache__"}.intersection(p.parts)}
    inputs[Path(__file__).relative_to(ROOT).as_posix()] = sha(Path(__file__))
    if args.full:
        for name in ("assessor/binary_inspection/AnnotateReference.java", "assessor/binary_inspection/test_live_pmu.py",
                     "tools/install_inspection_tools.sh", "reviews/v5/expansion/main_integration/pmu/layout-count.json"):
            inputs[name] = sha(ROOT/name)
    with tempfile.TemporaryDirectory(prefix="cmastery-inspection-") as folder:
        work = Path(folder) / "workspace"
        shutil.copytree(assets, work, ignore=shutil.ignore_patterns("build", "__pycache__"))
        (work/"answers").mkdir()
        fixture = Path("fixtures/binary_inspection/records.json")
        (work/fixture).parent.mkdir(parents=True)
        shutil.copy(ROOT/fixture, work/fixture)
        inputs[fixture.as_posix()] = sha(ROOT/fixture)
        pmu = work/"labs/performance_followthrough"
        pmu.mkdir(parents=True)
        for name in ("performance_followthrough/native.py", "support/measurement_gates.py",
                     "performance_followthrough/dossiers/replay/layout.json"):
            src = ROOT/"labs"/name
            dst = work/"labs"/name
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy(src, dst)
            inputs[src.relative_to(ROOT).as_posix()] = sha(src)
        for compiler in ("gcc", "clang"):
            out = Path(execute([sys.executable, "inspect_lab.py", "prepare", "--cc", compiler], work).strip())
            manifest = json.loads((out/"manifest.json").read_text())
            builds[compiler] = {"compiler": manifest["compiler"], "products": manifest["products"]}
            image = out / "records"
            common = {"elf_sha256": sha(image)}
            exercise(work, compiler, "stages", {"object_sha256": sha(out/"driver.o"),
                     "output": "total=72 calls=3", "unresolved_call": "record_score",
                     "missing_provider_stage": "link"}, checks, bad_key="output", bad_value="total=0")
            pc = nm(image, "record_score")
            addr, start = address_fields(image, ".text")
            offset = start + pc - addr
            exercise(work, compiler, "bytes", dict(common, function_va=pc, file_offset=offset,
                     first_8_hex=image.read_bytes()[offset:offset+8].hex()), checks,
                     bad_key="file_offset", bad_value=offset+1)
            reloc = execute(["objdump", "-r", out/"driver.o"], work)
            match = re.search(r"^([0-9a-f]+)\s+(R_\S+)\s+record_score\b", reloc, re.M)
            seg = next(line.split() for line in execute(["readelf", "-Wl", image], work).splitlines()
                       if line.strip().startswith("LOAD") and "RW" in line)
            exercise(work, compiler, "linking", dict(common, relocation_offset=int(match[1], 16),
                     relocation_type=match[2], bss_type="NOBITS", rw_file_size=int(seg[4], 16),
                     rw_memory_size=int(seg[5], 16)), checks, bad_key="bss_type", bad_value="PROGBITS")
            replay = pmu/"dossiers/replay/layout.json"
            exercise(work, compiler, "replay", dict(report_sha256=sha(replay), pairs=31,
                     interpretation="supported", wrapped_sample={"physical_header_offset": 12,
                     "next_tail": 40, "kernel_loss_without_count": "unknown"}), checks,
                     bad_key="pairs", bad_value=30)
            if args.full:
                relative = annotate(work, out, args.tools)
                annotation = json.loads((work/relative).read_text())
                builds[compiler]["ghidra"] = {"imported_sha256": annotation["exe_sha256"],
                                             "function_rva": annotation["rva"],
                                             "decompiler_export_sha256": sha(work/relative)}
                exercise(work, compiler, "debugger", dict(debug_args_and_return=[3,5,35],
                         optimized_args_and_return=[3,5,35], annotation=relative), checks,
                         bad_key="optimized_args_and_return", bad_value=[3,5,0])
        if args.full:
            out = Path(execute([sys.executable, "inspect_lab.py", "prepare", "--target", "cortex"], work).strip())
            manifest = json.loads((out/"manifest.json").read_text())
            builds["cortex"] = {"compiler": manifest["compiler"], "products": manifest["products"]}
            image = out/"target.elf"
            headers = execute(["arm-none-eabi-objdump", "-h", image], work)
            offset = int(next(line.split()[5] for line in headers.splitlines() if ".vectors " in line), 16)
            stack, vector = struct.unpack_from("<II", image.read_bytes(), offset)
            exercise(work, "gcc", "cortex", dict(elf_sha256=sha(image), initial_sp=stack,
                     reset_vector=vector, post_startup=[5,1,35]), checks, bad_key="reset_vector",
                     bad_value=vector ^ 1)
        # Historical author data cannot satisfy the native learner gate.
        relative = "labs/performance_followthrough/dossiers/replay/layout.json"
        dump(work/"answers/live.json", dict(prediction="test", explanation="test", report=relative,
             report_sha256=sha(work/relative), pairs=31, interpretation="supported"))
        if "provenance" not in execute([sys.executable,"inspect_lab.py","check","live","--cc","gcc"], work, ok=False):
            raise ValueError("Offline report did not fail the live provenance gate")
        checks.append({"case": "live", "historical_report_rejected": True, "learner_run": "not-assessed"})
    if args.full:
        execute([sys.executable, ROOT/"assessor/binary_inspection/test_live_pmu.py"], ROOT)
        checks.append({"case": "live", "author_report_and_component_identity_tests": "passed"})
    return {"passed": True, "builds": builds, "profile": "full" if args.full else "public", "inputs": inputs,
            "checks": checks, "independent_learner": "not-assessed", "gui_interaction": "not-assessed"}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--full", action="store_true", help="also use real GDB, Ghidra headless and QEMU")
    parser.add_argument("--tools", type=Path, default=ROOT/"build/inspection-tools")
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    result = verify(args)
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        dump(args.report, result)
    print(f"{result['profile']}: {len(result['checks'])} source-bound inspection cases passed")

