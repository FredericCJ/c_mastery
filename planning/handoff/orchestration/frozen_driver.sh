#!/bin/bash
# usage: frozen_driver.sh COMMIT LABEL PRODUCER_LIST
# Runs campaign producers in a detached worktree of COMMIT so concurrent edits
# in the main tree cannot change the sources a run binds. Receipts are copied
# back to reviews/v5/expansion/polish_revision/campaigns/<LABEL>_<id>/.
# PRODUCER_LIST lines: <id> <python args...>. TMPDIR lives on disk, outside both trees.
set -u
MAIN=/home/fred/dev/c_mastery; COMMIT="$1"; LABEL="$2"; LIST="$3"
TREE=$MAIN/build/frozen/$LABEL; LOGS=$MAIN/build/campaign-logs/$LABEL
C=reviews/v5/expansion/polish_revision/campaigns
cd "$MAIN" || exit 1
[ -e "$TREE" ] || git worktree add --detach "$TREE" "$COMMIT" >/dev/null 2>&1 || exit 1
mkdir -p "$LOGS"; export CMASTERY_TIMEOUT_SCALE=4; export TMPDIR=/home/fred/.cache/cmastery-tmp/campaign-$LABEL; mkdir -p "$TMPDIR"
[ -e "$TREE/build/author-tools" ] || { mkdir -p "$TREE/build"; ln -s "$MAIN/build/author-tools" "$TREE/build/author-tools"; }
cd "$TREE" || exit 1
while read -r id args; do
  [ -z "$id" ] && continue
  out="$C/${LABEL}_$id"; [ -e "$MAIN/$out" ] && { echo "SKIP_EXISTING $id"; continue; }
  mkdir -p "$out"; start=$(date +%s); load0=$(cut -d' ' -f1 /proc/loadavg)
  timeout 2700 env CMASTERY_V5_RECEIPTS_DIR="$out" python3 $args > "$LOGS/$id.log" 2>&1 < /dev/null; rc=$?
  git status --porcelain -- reviews research > "$out/tree_writes.txt"
  mkdir -p "$MAIN/$out"; cp -a "$out/." "$MAIN/$out/"; cp "$LOGS/$id.log" "$MAIN/$out/producer.log"
  printf '%s\t%s\t%s\t%s\t%s\t%s\n' "$id" "$rc" "$(( $(date +%s) - start ))" "$COMMIT" "load0=$load0" "python3 $args" >> "$LOGS/runs.tsv"
  git checkout -q -- reviews research 2>/dev/null; git clean -qfd -- reviews research 2>/dev/null
  rm -rf --one-file-system "${TMPDIR:?}"/* 2>/dev/null
  echo "DONE $id rc=$rc"
done < "$LIST"
echo ALLDONE
