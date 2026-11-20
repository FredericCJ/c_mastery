"""Execute each selected exercise using controls from its extracted learner ZIP."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
PRIVATE = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "tools"))
from public_exports import extract, write_archive


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def archive_case(work, cache, card):
    exercise = card["id"]
    case = card["tests"][0].removeprefix("supervision.")
    area = work / exercise
    area.mkdir()
    source, archive, extracted = area / "source", area / "learner.zip", area / "extracted"
    env = {**os.environ, "PYTHONPATH": "", "PYTHONDONTWRITEBYTECODE": "1"}
    commands = []

    def run(cwd, argv, expected=0):
        cmd = [str(x) for x in argv]
        result = subprocess.run(cmd, cwd=cwd, env=env, capture_output=True, text=True, timeout=180)
        commands.append({"cwd": str(cwd).replace(str(work), "<campaign>").replace(str(ROOT), "<checkout>"),
                         "argv": [s.replace(str(work), "<campaign>").replace(str(ROOT), "<checkout>") for s in cmd],
                         "returncode": result.returncode})
        if result.returncode != expected:
            raise ValueError(f"extracted workspace {cmd}: {result.returncode}\n{result.stdout}\n{result.stderr}")
        return result.stdout

    run(ROOT, [sys.executable, "-m", "cmastery", "init", card["project"], source, "--exercise", exercise])
    names = [p.relative_to(source).as_posix() for p in source.rglob("*") if p.is_file()]
    hidden = {"assessor", "private", "reviews", "build", "__pycache__"}
    if any(hidden.intersection(Path(n).parts) for n in names):
        raise ValueError("unexpected private or generated file in initialized learner workspace")
    entries = write_archive(archive, names, root=source)
    extract(archive, extracted)
    actual = {p.relative_to(extracted).as_posix(): digest(p) for p in extracted.rglob("*") if p.is_file()}
    if entries != actual:
        raise ValueError("extracted workspace differs from initialized workspace")
    private_files = [*PRIVATE.glob("*.c"), *PRIVATE.glob("*.h")]
    if set(entries.values()).intersection(digest(path) for path in private_files):
        raise ValueError("private reference source found in learner ZIP")
    project = json.loads((extracted / ".cmastery/project.json").read_text())
    if project["assessment"] != "learner" or project["exercise"] != exercise:
        raise ValueError("extracted workspace is missing its selected exercise")
    fixture = json.loads(run(extracted, [sys.executable, "submission.py", "fixture"]))
    if fixture.get("kind") != "offline-fixture" or fixture.get("guest_execution") != "not-run":
        raise ValueError("standalone fixture failed to identify its offline scope")

    record = {"exercise": exercise, "case": case, "commands": commands, "archive_sha256": digest(archive),
              "entries": entries, "offline_fixture": fixture, "runs": {}}
    raw = {}
    for role in ("learner", "reference"):
        out = area / role
        # The private source and the test binaries both stay outside extracted/.
        configure = ["cmake", "-S", ".", "-B", out / "build", "-DCMAKE_C_COMPILER=gcc",
                     "-DCMAKE_BUILD_TYPE=Release"]
        if role == "reference":
            configure.append(f"-DCMASTERY_ASSESSOR_ROOT={PRIVATE}")
        run(extracted, configure)
        run(extracted, ["cmake", "--build", out / "build", "--parallel", "2"])
        program = out / "build/supervise"
        guest = out / "guest"
        cache_args = ["--cache", cache.resolve()] if cache else []
        command = [sys.executable, "guest.py", "run", *cache_args, "--case", case,
                   "--program", program, "--out", guest]
        stub = role == "learner"
        run(extracted, command, expected=int(stub))
        launch = json.loads((guest / "launch.json").read_text())
        serial = guest / "serial.txt"
        if (launch["status"] != ("case-failed" if stub else "completed")
                or launch["qemu_returncode"] != 0 or launch["requested"] != [case]
                or launch["sha256"]["serial"] != digest(serial) or launch["sha256"]["program"] != digest(program)
                or launch["sha256"]["runner"] != entries["guest.py"]
                or launch["sha256"]["fixture_source"] != entries["guest_fixture.c"]):
            raise ValueError(f"{exercise} {role} guest run did not bind to the exported controls")
        replay = run(extracted, [sys.executable, "submission.py", "replay", case, serial], expected=int(stub))
        facts = None if stub else json.loads(replay)["facts"]
        for cell in launch["cases"]:
            if cell["name"] != case or cell["status"] != int(stub):
                raise ValueError(f"wrong terminal status for extracted {exercise} {role}")
            if stub:
                if f"{case}/{cell['variant']} requires a learner implementation" not in serial.read_text():
                    raise ValueError(f"{exercise} starter rejection came from an unrelated failure")
            elif cell["variant"] not in facts:
                raise ValueError(f"exported replay did not score {cell}")
        record["runs"][role] = {
            "status": launch["status"], "cases": launch["cases"], "sha256": launch["sha256"],
            "grade": "expected-starter-rejection" if stub else "verified",
            "program_sha256": digest(program), "facts": facts,
            "snapshot": launch["snapshot"], "packages": launch["packages"],
        }
        raw[f"exported-{case}-{role}"] = serial
    return record, raw


def verify_archive(work, cache):
    family = json.loads((ROOT / "labs/linux_supervision/family.json").read_text())
    records, serials = {}, {}
    for card in family["exercises"]:
        record, logs = archive_case(work, cache, card)
        records[record["case"]] = record
        serials.update(logs)
    return records, serials
