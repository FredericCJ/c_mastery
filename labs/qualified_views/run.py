"""Compile a strict component with an explicit original runtime provider."""
import argparse
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cc", default="gcc")
    parser.add_argument("--source", type=Path)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    args = parser.parse_args()

    lab = Path(__file__).resolve().parent
    args.build.mkdir(parents=True, exist_ok=True)
    exe = args.build / "views-contract"
    command = [
        args.cc, "-std=c17", "-O0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
        "-ffreestanding", "-fno-builtin",
        "-I" + str(lab),
        str(args.source or lab / "learner.c"),
        str(args.runtime),
        str(lab / "tests.c"),
        "-o", str(exe),
    ]
    subprocess.run(command, check=True)
    return subprocess.run([str(exe)]).returncode


if __name__ == "__main__":
    raise SystemExit(main())
