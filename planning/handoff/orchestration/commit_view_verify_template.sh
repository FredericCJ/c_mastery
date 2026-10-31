#!/bin/bash
V=/home/fred/dev/c_mastery/build/frozen/b9w5t; cd $V || exit 1
export TMPDIR=/home/fred/.cache/cmastery-tmp/b9w5t-run; mkdir -p $TMPDIR; export CMASTERY_TIMEOUT_SCALE=4
L=/home/fred/.cache/cmastery-tmp/orch/b9w5t-logs; mkdir -p $L; T=$(date +%s)
step() { while [ -e /home/fred/.cache/cmastery-tmp/orch/QUIET ]; do sleep 30; done; local id="$1"; shift; local s; s=$(date +%s); timeout 3600 "$@" > "$L/$id.log" 2>&1 < /dev/null; local rc=$?; echo -e "$id\t$rc\t$(( $(date +%s)-s ))s\tload=$(cut -d' ' -f1 /proc/loadavg)" | tee -a "$L/runs.tsv"; }
step regen_catalog python3 tools/author_exercises_v5.py
step regen_route python3 tools/generate_native_route.py
step style python3 tools/check_c_style.py
step security_labs python3 tools/check_security_labs.py --report "$TMPDIR/security.json"
step native_route python3 tools/check_native_route.py
step exercises python3 tools/check_exercises_v5.py
step workflow python3 tools/check_exercise_workflow.py
step public_validation python3 tools/public_validation.py --output "$TMPDIR/public_validation.json"
for t in run_thread_lifecycle run_thread_sharing run_thread_progress run_thread_waiting run_thread_atomics run_consume_index_depth run_advanced_grace run_advanced_history run_advanced_reclamation run_advanced_spsc run_capstone_linux run_checkpoint_linux_followthrough check_thread_atomics_exports check_thread_lifecycle_exports check_thread_progress_exports check_thread_sharing_exports check_thread_waiting_exports; do step $t python3 tools/$t.py; done
step run_debugging_followthrough python3 tools/run_debugging_followthrough.py --report reviews/v5/expansion/polish_revision/campaigns/b9w5t_debugging/report-$T.json
git status --porcelain -- reviews research > $L/tree_writes.txt
echo ALLDONE
