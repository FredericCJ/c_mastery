"""Independent finite host oracles for the new capstone composition fixtures.

These models do not run the learner service, firmware, epoll or physical board.
Python/zlib is a host verifier, never a dependency of the learner C core.
"""
import hashlib
import json
from pathlib import Path
import platform
import zlib

ROOT = Path(__file__).resolve().parents[2]


def decode(buffer):
    emitted = []
    while buffer:
        if buffer[0] != 0xA5:
            del buffer[0]
            continue
        if len(buffer) < 6:
            break
        n = int.from_bytes(buffer[4:6], "little")
        if buffer[1] != 1 or n > 32:
            del buffer[0]
            continue
        size = 10 + n
        if len(buffer) < size:
            break
        candidate = bytes(buffer[:size])
        expected = zlib.crc32(candidate[1:-4])
        if int.from_bytes(candidate[-4:], "little") != expected:
            del buffer[0]
            continue
        emitted.append((int.from_bytes(candidate[2:4], "little"), candidate[6:-4]))
        del buffer[:size]
    return emitted


def frame(sequence, payload):
    core = bytes([1]) + sequence.to_bytes(2, "little") + len(payload).to_bytes(2, "little") + payload
    return bytes([0xA5]) + core + zlib.crc32(core).to_bytes(4, "little")


def main():
    fixture_path = ROOT / "fixtures/legacy_capstones/boundaries.json"
    value = json.loads(fixture_path.read_text(encoding="utf-8"))
    wire_path = ROOT / "fixtures/integration/wire.json"
    wire = json.loads(wire_path.read_text(encoding="utf-8"))
    supplied = bytes.fromhex(wire["expected"]["frame_hex"])
    assert frame(wire["input"]["sequence"], bytes.fromhex(wire["input"]["payload_hex"])) == supplied
    split_cases = 0
    for sequence, payload in [(258, b"\x00A\xff"), (0, b""), (65535, bytes(range(32)))]:
        original = frame(sequence, payload)
        for split in range(len(original) + 1):
            pending = bytearray(original[:split])
            result = decode(pending)
            pending += original[split:]
            result += decode(pending)
            assert result == [(sequence, payload)] and not pending
            split_cases += 1
    corrupt = bytearray(supplied)
    corrupt[-1] ^= 1
    stream = bytearray(bytes(corrupt) + supplied)
    assert decode(stream) == [(258, b"\x00A\xff")]

    output = value["output"]
    data, cursor, delivered, history = bytes.fromhex(output["bytes_hex"]), 0, bytearray(), []
    for result in output["script"]:
        if isinstance(result, int) and result > 0:
            assert result <= len(data) - cursor
            delivered += data[cursor:cursor + result]
            cursor += result
        history.append(cursor)
    assert history == output["expected_cursor"]
    assert delivered.hex() == output["expected_delivered_hex"]

    edge = value["edge_ready"]
    remaining, work = edge["available_bytes"], []
    while remaining:
        amount = min(remaining, edge["per_turn_budget"])
        work.append(amount)
        remaining -= amount
    assert work == edge["expected_work"]
    stale = edge["stale_event"]
    assert (stale["event_generation"] == stale["live_generation"]) == stale["expected_dispatch"]
    ledger = value["ownership"]
    assert len(ledger["accepted"]) == len(ledger["delivered"]) + len(ledger["discarded"]) + ledger["queued"] + ledger["active"]
    assert sorted(ledger["accepted"]) == sorted(ledger["delivered"] + ledger["discarded"])
    assert len(set(ledger["delivered"] + ledger["discarded"])) == ledger["expected_releases"]

    budget = value["firmware_budget"]
    total = sum(budget["entries"].values())
    assert total == budget["expected_total"]
    assert budget["limit_bytes"] - total == budget["expected_remaining"]
    assert (total + budget["extra_nested_frame"] <= budget["tighter_limit"]) == budget["expected_extra_fits_tighter"]
    watch = value["watchdog"]
    feeds = [(mask & watch["required_mask"]) == watch["required_mask"] for mask in watch["epoch_observations"]]
    assert feeds == watch["expected_feed"]
    update = value["update"]
    selected = ["B" if cut == len(update["steps"]) else "A" for cut in update["cuts"]]
    assert selected == update["expected_selected"]
    assert update["untrusted_candidate_expected"] == "A"

    # Small deliberate faulty model controls verify that the exact assertions
    # discriminate these misconceptions; these are not compiled C mutant tests.
    controls = {
        "replay accepted prefix": bytes(data[:2] + data) != delivered,
        "sleep after ET budget": work[:1] != edge["expected_work"],
        "dispatch by fd only": True != stale["expected_dispatch"],
        "ignore exception storage": total - budget["entries"]["nested_exception_allowance"] != budget["expected_total"],
        "feed on any progress": [bool(x) for x in watch["epoch_observations"]] != watch["expected_feed"],
        "select before commit": ["B" if cut else "A" for cut in update["cuts"]] != selected,
    }
    assert all(controls.values())
    paths = [fixture_path, wire_path, Path(__file__).resolve(),
             ROOT / "curriculum/v5/expansion/legacy_assessments/capstone-contract.md"]
    report = {"status": "passed", "command": ".conda/python.exe assessor/legacy_depth/verify_capstone_boundaries.py",
              "python": platform.python_version(), "zlib_runtime": zlib.ZLIB_RUNTIME_VERSION,
              "wire_two_chunk_cases": split_cases, "corrupt_then_valid_resynchronization": "passed",
              "finite_boundary_groups": ["output prefixes", "ET scheduling/stale generation", "ownership conservation",
                                         "firmware budget", "watchdog epochs", "update cuts"],
              "faulty_model_controls": controls,
              "source_sha256": {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
              "limitations": ["Finite independent host models only; no learner C service, firmware or native epoll execution.",
                              "No actual cryptography, flash atomicity, board timing, hardware protection or learner assessment established."]}
    dest = ROOT / "reviews/v5/expansion/legacy_capstones/boundary_oracles.json"
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: report[k] for k in ["status", "wire_two_chunk_cases", "finite_boundary_groups", "faulty_model_controls"]}))


if __name__ == "__main__":
    main()
