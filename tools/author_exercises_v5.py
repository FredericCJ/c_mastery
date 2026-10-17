"""Derive the additive exercise catalog from complete canonical Markdown briefs."""
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from cmastery import cli, exercises, learning
from common import dump


def main():
    value = exercises.build_catalog()
    dump(learning.catalog_dir() / "exercises.json", value)
    print("Authored additive exercise catalog: " + str(len(value["exercises"])) + " exercises")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
