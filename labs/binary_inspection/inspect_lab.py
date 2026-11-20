"""Prepare source-bound inspection artifacts or check a learner's observations."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

LAB = Path(__file__).resolve().parent
STAGES = ("stages", "bytes", "linking", "debugger", "cortex", "replay", "live")


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def invoke(argv, *, cwd=LAB, timeout=45):
    return subprocess.run(list(map(str, argv)), cwd=cwd, env={**os.environ, "LC_ALL": "C"},
                          check=True, text=True, capture_output=True, timeout=timeout).stdout


def inputs(target):
    paths = [LAB / "record.h", LAB / "record.c"]
    paths += sorted((LAB / "arm").glob("*")) if target == "cortex" else [LAB / "driver.c"]
    return {p.relative_to(LAB).as_posix(): digest(p) for p in paths}


def recipe(output, cc, target):
    commands = []

    def build(*argv):
        commands.append([str(a) for a in argv])

    if target == "cortex":
        cc = "arm-none-eabi-gcc"
        build(cc, "-std=c17", "-Og", "-g", "-mcpu=cortex-m4", "-mthumb", "-ffreestanding",
              "-fno-builtin", "-nostdlib", "-I.", "-Tarm/image.ld", "arm/entry.S", "arm/target.c",
              "record.c", "-o", output / "target.elf")
        products = ["target.elf"]
    else:
        cc = shutil.which(cc) or cc
        flags = ["-std=c17", "-Og", "-g", "-fPIE"]
        for unit in ("driver", "record"):
            build(cc, *flags, "-E", unit + ".c", "-o", output / (unit + ".i"))
            build(cc, *flags, "-S", output / (unit + ".i"), "-o", output / (unit + ".s"))
            build(cc, "-c", output / (unit + ".s"), "-o", output / (unit + ".o"))
        build(cc, "-pie", output / "driver.o", output / "record.o", "-o", output / "records")
        build(cc, "-std=c17", "-O2", "-g", "-fPIE", "-pie", "driver.c", "record.c",
              "-o", output / "optimized")
        build("strip", "--strip-all", "-o", output / "stripped", output / "optimized")
        products = [u + ext for u in ("driver", "record") for ext in (".i", ".s", ".o")]
        products += ["records", "optimized", "stripped"]
    return cc, commands, products


def prepare(output, cc, target):
    output.mkdir(parents=True, exist_ok=True)
    cc, commands, products = recipe(output, cc, target)
    for command in commands:
        invoke(command)
    manifest = {"target": target, "inputs": inputs(target), "compiler": invoke([cc, "--version"]).splitlines()[0],
                "commands": commands, "products": {n: digest(output / n) for n in products}}
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest


def symbol(image, name, prefix=""):
    for line in invoke([prefix + "nm", "-n", image]).splitlines():
        fields = line.split()
        if len(fields) == 3 and fields[2] == name:
            return int(fields[0], 16)
    raise ValueError(f"Missing symbol {name} in {image.name}")


def workspace_file(relative):
    name = Path(relative)
    path = LAB / name
    if name.is_absolute() or ".." in name.parts or path.resolve() != path or not path.is_file():
        raise ValueError(f"Expected a regular workspace file: {relative}")
    return path


def expect(row, key, value):
    # Python equality treats 1, 1.0 and True as equal, including inside arrays.
    if key not in row or json.dumps(row[key], sort_keys=True) != json.dumps(value, sort_keys=True):
        raise ValueError(f"Evidence disagrees at {key}")


def checked_manifest(output, compiler, target):
    manifest = json.loads((output / "manifest.json").read_text())
    cc, commands, products = recipe(output, compiler, target)
    expect(manifest, "target", target)
    expect(manifest, "compiler", invoke([cc, "--version"]).splitlines()[0])
    expect(manifest, "commands", commands)
    expect(manifest, "inputs", inputs(target))
    expect(manifest, "products", {name: digest(output / name) for name in products})
    # A matching self-reported hash cannot tie a substituted binary to these inputs.
    with tempfile.TemporaryDirectory() as folder:
        rebuilt = prepare(Path(folder), compiler, target)
    expect(manifest, "products", rebuilt["products"])
    return manifest


def check(case, output, compiler, receipt=None):
    from inspection_checks import CHECKS
    response = LAB / "answers" / (case + ".json")
    if not response.exists():
        raise ValueError(f"Prediction and observations required in answers/{case}.json")
    row = json.loads(response.read_text())
    for name in ("prediction", "explanation"):
        if not isinstance(row.get(name), str) or not row[name].strip():
            raise ValueError(f"Missing {name}")
    if case in ("replay", "live"):
        CHECKS[case](row, output)
        evidence = {"report_sha256": row["report_sha256"]}
        if case == "live":
            evidence["measured_image_sha256"] = digest(workspace_file(row["measured_image"]))
    else:
        target = "cortex" if case == "cortex" else "host"
        manifest = checked_manifest(output, compiler, target)
        if case == "stages":
            CHECKS[case](row, output, compiler)
        else:
            CHECKS[case](row, output)
        evidence = {key: manifest[key] for key in ("target", "compiler", "inputs", "products")}
        evidence["build_steps"] = [
            {"tool": Path(cmd[0]).name, "flags": [arg for arg in cmd[1:] if arg.startswith("-") and arg != "-o"]}
            for cmd in manifest["commands"]]
    if receipt:
        receipt.parent.mkdir(parents=True, exist_ok=True)
        receipt.write_text(json.dumps({"stage": case, **evidence}, indent=2) + "\n")
    print(f"Verified {case} observations against source-bound artifacts. Written reasoning requires review.")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    cmd = parser.add_subparsers(dest="action", required=True)
    build = cmd.add_parser("prepare")
    build.add_argument("--target", choices=("host","cortex"), default="host")
    verify = cmd.add_parser("check")
    verify.add_argument("stage", choices=STAGES)
    verify.add_argument("--receipt", type=Path)
    for p in (build, verify):
        p.add_argument("--cc", default="gcc")
    args = parser.parse_args()
    target = args.target if args.action == "prepare" else ("cortex" if args.stage == "cortex" else "host")
    name = Path(shutil.which(args.cc) or args.cc).resolve().name
    output = LAB / "build/inspection" / ("cortex" if target == "cortex" else name)
    if args.action == "prepare":
        prepare(output, args.cc, target)
        print(output)
    else:
        check(args.stage, output, args.cc, args.receipt)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        sys.exit(f"Inspection check: {error}")
