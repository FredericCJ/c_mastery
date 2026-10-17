"""Check original additive fixture mathematics, never learner C acceptance."""
import json
from itertools import groupby
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def check():
    checked = []
    for project in ("PR-T14-09", "PR-T15-10", "PR-T15-08"):
        path = ROOT / "fixtures" / "legacy_depth" / (project + ".json")
        data = json.loads(path.read_text(encoding="utf-8"))
        source, expected = data["input"], data["expected"]
        if source["kind"] == "membership_filter":
            bits = {h for key in source["keys"] for h in (key % 8, (3 * key + 1) % 8)}
            positive = [key for key in source["queries"]
                        if {key % 8, (3 * key + 1) % 8} <= bits]
            result = {"set_bits": sorted(bits), "positive_queries": positive,
                      "false_positive_queries": [key for key in positive if key not in source["keys"]]}
        elif source["kind"] == "rle":
            encoded = []
            for case in source["cases"]:
                pairs = []
                for value, run in groupby(case):
                    count = sum(1 for _ in run)
                    full, remainder = divmod(count, 255)
                    pairs += [255, value] * full
                    if remainder:
                        pairs += [remainder, value]
                encoded.append(pairs)
            invalid = []
            for pair_stream in source["invalid_encodings"]:
                invalid.append("invalid" if len(pair_stream) % 2 or
                               any(count == 0 for count in pair_stream[::2]) else "valid")
            result = {"encoded": encoded, "invalid_status": invalid}
        else:
            # Polynomial long division over GF(2), independent of bytewise C update loops.
            message = int.from_bytes(bytes(source["bytes"]), "big") << 8
            divisor = (1 << 8) | source["polynomial"]
            while message.bit_length() >= divisor.bit_length():
                message ^= divisor << (message.bit_length() - divisor.bit_length())
            result = {"crc": message}
        assert result == expected, (project, result, expected)
        checked.append(project)
    return {"scope": "Exact additive fixture mathematics only; no C or learner execution",
            "checked": checked, "passed": len(checked)}


if __name__ == "__main__":
    print(json.dumps(check(), indent=2))
