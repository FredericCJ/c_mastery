# C mastery curriculum

Read [the complete Markdown curriculum](c_programming_mastery_curriculum.md), or serve `build/html` locally with `serve.ps1` and open `http://127.0.0.1:8766/`.

The editable teaching sources are `curriculum/modules/`, `curriculum/projects/` and `curriculum/guides/`. Supplied project data lives in `fixtures/`. `references/` records bounded source inspections; `research/` contains the public search/reflection/synthesis record. Original specification files remain unchanged.

The enhancement adds T36–T42: algorithms and data structures derived from invariants through handwritten x86-64/Thumb assembly and reusable C; computer and microcontroller architecture; serial buses, electrical interfaces, CAN/CAN FD and Ethernet; and three complete communication project families. Start with [the extension route](curriculum/guides/enhancement.md) and [the stack families](curriculum/guides/communication-stacks.md). The families comprise small milestones already charged in their module budgets. T41 reuses T42's portable HTTP implementation.

T43–T45 add complexity and performance measurement, progressive debugging, and controlled memory/storage/lifetime failure experiments. Start with [the performance and debugging route](curriculum/guides/performance-debugging.md). The addition has 36 topics, 108 objectives and 32 bounded projects, with readings from held textbooks, foundational publications and exact official documents. Its 370-hour allocation includes project work; existing implementations and the drill directory are charged once.

## Build

Use Python 3.11 or later. On Windows:

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install -r requirements-lock.txt
.\build.ps1
.\serve.ps1
```

The same Python build steps work on Linux/macOS after creating a virtual environment and installing the locked dependencies:

```sh
python tools/write_program.py
python tools/validate.py
python tools/check_fixtures.py
python tools/check_contract_guards.py
python tools/check_baseline_preservation.py
python tools/check_performance_debugging_preservation.py
python tools/publish.py
python tools/check_html.py
python tools/check_markdown.py
python -m http.server 8766 --bind 127.0.0.1 --directory build/html
```

The publication uses local assets, including MathJax. Rebuilding needs neither the Knowledge library nor the SWE graph. Research helpers are optional authoring utilities, separate from the normal build. Private book extracts/captures and the publishing virtual environment are excluded from archives.

## Verification and exports

`reviews/` contains structural, fixture, source/publication, independent-content and browser review receipts. Publishing checks do not claim that learner C implementations, physical firmware gates or formal proofs have already been completed.

The final teaching/publication receipt is `reviews/final_acceptance.json`; archive hashes and the fresh extracted-source build are recorded separately in `reviews/package_report.json`.

`tools/package.py` creates an editable source archive and a portable HTML archive, records hashes and rebuilds a fresh extracted source copy using the recorded publishing environment. The archive manifest lists exact included files. Local source changes require rebuilding and new receipts before being described as accepted.
