#!/bin/sh
# Explicit stages; arguments are individual paths, never a shell command string.
set -eu
compiler=$1
component=$2
public=$3
output=$4
mkdir -p "$output"
"$compiler" -std=c17 -O0 -Wall -Wextra -Wpedantic -Werror -I"$public" -c "$component" -o "$output/list.o"
"$compiler" -std=c17 -O0 -Wall -Wextra -Wpedantic -Werror -I"$public" -c "$public/main.c" -o "$output/main.o"
"$compiler" "$output/list.o" "$output/main.o" -o "$output/list"
"$output/list" 2 -5 9
