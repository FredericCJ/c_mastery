# Repository author tools

Versions are pinned in `tools/author_tools.lock.json`. Graphify and Spec Kit use
`build/author-tools/python`, with resolved packages in
`tools/author_tools_requirements.lock`. The publishing `.venv` stays independent.
OpenSpec and Beads use checksum-verified official Linux x86-64 binaries. Skills
are installed in `.agents/skills/`; no Git hooks are installed.

The requested Graphify repository contains only metadata and examples. Its
metadata attributes [Graphify-Labs/graphify](https://github.com/Graphify-Labs/graphify),
whose `graphifyy` executable distribution is installed. Both inspected commit
identities are retained. Other upstreams are [OpenSpec](https://github.com/Fission-AI/OpenSpec),
[Spec Kit](https://github.com/github/spec-kit), [Beads](https://github.com/gastownhall/beads)
(the requested URL redirects there), and [Superpowers](https://github.com/obra/superpowers).

```sh
python3 tools/install_author_tools.py --install --initialize-beads
python3 tools/author-tool graphify extract . --code-only --max-workers 2 --no-cluster --out build/author-tools
python3 tools/check_author_tools.py
python3 tools/author-tool graphify --version
python3 tools/author-tool openspec list --json
python3 tools/author-tool specify version
python3 tools/author-tool bd ready --json
```

The wrapper disables telemetry, Graphify skill auto-refresh and automatic Beads
pushes, and selects local Dolt configuration. It refuses direct `bd init`: upstream
1.3.2 attempts Git staging/commit even with `--skip-agents --skip-hooks`. The
installer initializes fresh storage outside Git with an explicit database directory
and copies it into the project. Existing Beads data is never replaced. The first
initialization exposed that upstream behavior; the commit failed, HEAD stayed
unchanged and the incidental staging was undone.

```sh
python3 tools/author-tool graphify query 'run_native_pmu' --budget 600
```

This derived AST code index runs locally without an API key or source upload.
`.graphifyignore` excludes reviews, private captures, dependencies, skills and DB
state. The graph includes assessor code, remains in ignored author build output,
and must not be included in learner packages. Initial extraction produced 59,353
nodes and 150,151 edges, with 33 partial parses and 941 files yielding no symbols.
Regenerate after source changes. It does not claim semantic coverage of curriculum
prose or proofs. Query results can be truncated; raise the budget or narrow the
query before drawing conclusions.

Spec Kit templates are bundled in its installed 1.2.5 wheel; initialization does
not fetch current upstream templates. Python packages are version-pinned, without
wheel hash pins. Native binary downloads are SHA256-verified. The supplied pins
were executed on Linux x86-64 with Python 3.14; optional author tools need no sudo.
Repository-specific restore templates live in `tools/author_tools_templates/`;
the verifier checks them against the installed configuration. Existing customized
files and live Beads storage are preserved by restoration.

OpenSpec context and Spec Kit constitution preserve existing authorization and
canonical evidence requirements. Beads starts empty as optional coordination;
entries should reference canonical obligation IDs, and closure proves no curriculum
gate. Full developer checkouts retain the project integrations. Public curriculum
source archives can restore optional author integrations with the pinned installer;
these tools are not dependencies of public curriculum validation.
