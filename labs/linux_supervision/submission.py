"""Run a fresh Linux guest exercise or check an offline observation."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

from observations import VARIANTS, verify

LAB = Path(__file__).resolve().parent


def digest(path):
    with path.open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def write_json(path, data):
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf8")


def read_json(path):
    return json.loads(path.read_text(encoding="utf8"))


def corpus_fixture():
    relative = Path("fixtures/linux_supervision/workloads.json")
    for base in (LAB, LAB.parents[1]):
        path = base / relative
        if path.is_file():
            return read_json(path)
    raise ValueError("Exported workload fixture missing")


def check_fixture():
    fixture = corpus_fixture()
    network, telemetry = fixture["network"], fixture["telemetry"]
    if len(network["request"]) != 8 or sum(network["request"]).to_bytes(4, "big").hex() != "00000283":
        raise ValueError("T24 loopback checksum fixture disagrees")
    frames, width = telemetry["frames"], telemetry["request_octets"]
    total = sum((frame + k) & 255 for frame in range(frames) for k in range(width))
    if (frames, width, telemetry["reply_octets"], total) != (8192, 8, 4, 8355840):
        raise ValueError("Telemetry byte ledger disagrees")
    print(json.dumps({"kind": "offline-fixture", "network_reply_sum": 643, "frames": frames,
                      "input_bytes": frames*width, "checksum": total, "guest_execution": "not-run"}))


def answer_for(case, grading):
    path = LAB / "answers" / (case + ".json")
    if not path.is_file():
        raise ValueError(f"prediction required in answers/{case}.json")
    answer = read_json(path)
    if not isinstance(answer, dict) or not isinstance(answer.get("prediction"), str) or not answer["prediction"].strip():
        raise ValueError(f"prediction required in answers/{case}.json")
    if grading and (not isinstance(answer.get("explanation"), str) or not answer["explanation"].strip()):
        raise ValueError(f"explanation required in answers/{case}.json")
    return answer


def first_prediction(case, answer):
    # Keep the first prediction for each version of the learner's source. A hard link
    # installs a fully written record without replacing another concurrent attempt.
    source_root = LAB / "src"
    sources = {p.relative_to(source_root).as_posix(): digest(p)
               for p in sorted(source_root.rglob("*")) if p.is_file()}
    identity = hashlib.sha256(json.dumps(sources, sort_keys=True).encode()).hexdigest()
    directory = LAB/".cmastery"/"supervision-predictions"/case
    directory.mkdir(exist_ok=True, parents=True)
    target = directory/(identity+".json")
    if not target.exists():
        with tempfile.NamedTemporaryFile(dir=directory, mode="w", encoding="utf8", delete=False) as temp:
            json.dump({"case": case, "prediction": answer["prediction"], "implementation": sources,
                       "recorded_utc": datetime.now(timezone.utc).isoformat()}, temp)
        try:
            try:
                os.link(temp.name, target)
            except FileExistsError:
                pass
        finally:
            os.unlink(temp.name)
    stored = read_json(target)
    if stored["prediction"] != answer["prediction"]:
        raise ValueError("The original prediction for these sources changed; keep it and discuss corrections in explanation")
    return stored


def run(args):
    grading = args.action == "check"
    answer = answer_for(args.case, grading)
    stored = first_prediction(args.case, answer)
    args.out.mkdir(parents=True, exist_ok=grading)
    if grading:
        # CTest reuses the target directory, including on a failed run's retry.
        args.out = Path(tempfile.mkdtemp(prefix="attempt-", dir=args.out))
    write_json(args.out/"prediction.json", stored)
    image = args.program.resolve(strict=True)
    image_hash = digest(image)
    command = [sys.executable, str(LAB/"guest.py"), "run", "--program", str(image),
               "--case", args.case, "--out", str(args.out/"guest")]
    result = subprocess.run(command)
    launch = read_json(args.out/"guest/launch.json")
    if result.returncode == 77 and launch["status"] == "unavailable":
        print("Supervision unavailable: " + launch.get("reason", "guest capability unavailable"), file=sys.stderr)
        return 77
    if result.returncode or launch["status"] != "completed":
        raise ValueError("Guest execution failed; inspect guest/serial.txt and guest/launch.json")
    serial = args.out/"guest/serial.txt"
    if digest(serial) != launch["sha256"]["serial"] or image_hash != launch["sha256"]["program"] or digest(image) != image_hash:
        raise ValueError("Observed executable or serial capture changed")
    text = serial.read_bytes().decode("utf8", errors="replace")
    facts = verify(args.case, text)
    if grading and args.case == "telemetry":
        diagnoses = {key: row["cause"] for key, row in facts.items()}
        if answer.get("diagnosis") != diagnoses:
            raise ValueError("Submitted diagnosis disagrees with the observed CPU and pipe signals")
    report = {"case": args.case, "mode": args.action, "prediction": stored, "facts": facts,
              "explanation": answer.get("explanation"), "diagnosis": answer.get("diagnosis"),
              "sha256": launch["sha256"], "serial_file": "guest/serial.txt",
              "serial": text, "execution": "qemu-linux-guest", "independent_assessment": "not-assessed"}
    write_json(args.out/"result.json", report)
    print(f"Verified {args.case} in the guest. Observations: {args.out / 'result.json'}")
    return 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="action", required=True)
    for command in ("observe", "check"):
        p = sub.add_parser(command)
        p.add_argument("case", choices=VARIANTS)
        p.add_argument("--program", required=True, type=Path)
        p.add_argument("--out", required=True, type=Path)
    sub.add_parser("fixture")
    p = sub.add_parser("replay")
    p.add_argument("case", choices=VARIANTS)
    p.add_argument("serial", type=Path)
    args = parser.parse_args()
    if args.action == "fixture":
        check_fixture()
    elif args.action == "replay":
        facts = verify(args.case, args.serial.read_text())
        print(json.dumps({"kind": "transcript-replay", "guest_execution": "not-run", "facts": facts}, indent=2))
    else:
        return run(args)
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, KeyError, TypeError, ValueError) as error:
        sys.exit(f"Supervision check: {error}")
