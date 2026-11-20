#!/usr/bin/env bash
# Checksummed Linux x86-64 toolchain for the interactive and headless Ghidra lab.
set -euo pipefail
repo=$(cd "$(dirname "$0")/.." && pwd)
base=${1:-"$repo/build/inspection-tools"}
mkdir -p "$base"
base=$(cd "$base" && pwd)
exec 8>"$base/.install.lock"
flock 8
stage=$(mktemp -d "$base/.install.XXXXXXXX")
trap 'rm -rf "$stage"' EXIT
fetch() {
    local url=$1 name=$2 digest=$3
    if ! (cd "$base" && echo "$digest  $name" | sha256sum --check --status); then
        curl --fail --location --retry 3 "$url" -o "$stage/$name"
        (cd "$stage" && echo "$digest  $name" | sha256sum --check)
        mv "$stage/$name" "$base/$name"
    fi
}
fetch 'https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.2_build/ghidra_12.2_PUBLIC_20261016.zip' ghidra.zip 2c2cd776db3c1f6a097ba418922378ac3649cadd5e844a6f5f088ae628a7be67
fetch 'https://github.com/adoptium/temurin25-binaries/releases/download/jdk-25.0.5.1%2B2/OpenJDK25U-jdk_x64_linux_hotspot_25.0.5.1_2.tar.gz' jdk.tar.gz 760f91b22c3efc9bd1b64ff950598231be82928cc807e30691a0e7a2995a62c0
if [[ ! -d "$base/ghidra_12.2_PUBLIC" ]]; then
    unzip -q "$base/ghidra.zip" -d "$stage"
    mv "$stage/ghidra_12.2_PUBLIC" "$base"
fi
if [[ ! -d "$base/jdk25" ]]; then
    mkdir "$stage/jdk25"
    tar xf "$base/jdk.tar.gz" -C "$stage/jdk25" --strip-components=1
    mv "$stage/jdk25" "$base/"
fi
"$base/jdk25/bin/java" -version
printf 'Inspection tools ready under %s\n' "$base"
