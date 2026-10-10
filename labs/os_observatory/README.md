# OS observatory

Implement `src/oslab.c` and `src/platform.c`, starting with the selected project.
Read SPEC.md, predict the result, then run project-selected checks. The supplied
stubs compile and fail their behavioral gates. `--all-tests` is cumulative.

Typical Linux/WSL invocation from the curriculum repository:

```sh
python3 -m cmastery init PR-T49-03 ../my-os-observatory
python3 -m cmastery check ../my-os-observatory --target host
python3 ../my-os-observatory/observe.py
```

Host experiments do not establish bare-metal scheduling, cross-core correctness,
physical timing or cgroup enforcement. Complete the assigned mechanism and
changed-condition dossier as well as the finite executable anchors.
