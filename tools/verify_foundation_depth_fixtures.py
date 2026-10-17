"""Independent integer/wire oracle; source-only checks never claim C execution."""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import random
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def verify(driver: Path | None = None) -> dict:
    folder = ROOT / "fixtures/foundation_depth"
    data = json.loads((folder / "cases.json").read_text(encoding="utf-8"))
    assert len(data["semantic_cases"]) == 10
    assert len({item["id"] for item in data["semantic_cases"]}) == 10
    for item in data["semantic_cases"]:
        assert (folder / item["source"]).is_file(), item["source"]
    for row in data["wire_vectors"]:
        expected = struct.pack(">BBHi", 1, row["flags"], row["count"], row["value"])
        assert expected.hex() == row["hex"]
    for row in data["extent_vectors"]:
        exact = row["offset"] + row["count"] * row["width"]
        assert (exact if exact <= row["limit"] else None) == row["expected"]
    attack = data["security_observation"]
    _, _, count, value = struct.unpack(">BBHi", bytes.fromhex(attack["input_hex"]))
    assert attack["initial_counter"] + count * value == attack["flawed_counter"]
    assert attack["repaired_counter"] == attack["initial_counter"]
    result = {"fixture_vectors": 10, "semantic_sources_present": 10,
              "fixture_status": "passed", "c_execution": "not-run"}
    if driver is None:
        return result
    identity = subprocess.run([str(driver)], input="", text=True, capture_output=True, check=True)
    maximum = int(identity.stdout.strip().split()[1])
    rng = random.Random(0xB01)
    bounds = [0, 1, 2, 7, maximum // 2, maximum // 2 + 1, maximum - 1, maximum]
    requests, expected = [], []
    # Exact Python mathematical integers do not reproduce target-width overflow.
    for offset in bounds:
        for count in bounds:
            for width in bounds:
                for limit in (0, maximum // 2, maximum):
                    exact = offset + count * width
                    requests.append(f"E {offset} {count} {width} {limit}")
                    expected.append(f"E {0 if exact <= limit else 2} {exact if exact <= limit else 917}")
    for _ in range(512):
        flags, count, value = rng.randrange(4), rng.randrange(65536), rng.randrange(-2**31, 2**31)
        wire = struct.pack(">BBHi", 1, flags, count, value)
        requests.append("D " + " ".join(f"{byte:02x}" for byte in wire))
        expected.append(f"D 0 {flags} {count} {value}")
    observed = subprocess.run([str(driver)], input="\n".join(requests)+"\n", text=True, capture_output=True, check=True, timeout=20)
    lines = observed.stdout.splitlines()
    assert lines[0] == f"SIZE_MAX {maximum}"
    assert len(lines[1:]) == len(expected)
    for i, (actual, want) in enumerate(zip(lines[1:], expected)):
        assert actual == want, (requests[i], actual, want)
    result.update(c_execution="passed", size_max=maximum, differential_cases=len(expected),
                  independence="unbounded integer expression and Python struct wire specification")
    return result

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--driver", type=Path)
    args = parser.parse_args()
    print(json.dumps(verify(args.driver), indent=2))
