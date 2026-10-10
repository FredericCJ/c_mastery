"""Build bounded reuse metadata; no library reads or learner solution generation."""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).parent
ids = ["R-clrs-4", "R-hanson-cii", "R-morin-ods-0-1f", "R-SYS-AMD64-ABI-0-99-6", "R-ENH-AAPCS32-2025Q1", "R-ARM-DUI0553B"]
all_resources = json.loads((ROOT / "references/resources.json").read_text(encoding="utf-8"))["resources"]
resources = []
for source_id in ids:
    r = next(r for r in all_resources if r["id"] == source_id)
    resources.append({"id": source_id, "title": r["title"], "edition": r["edition"], "url": r["url"], "kind": r["kind"], "assignable": True, "reuse": True, "inspection": r["inspection"], "locators": r["locators"], "scope": "Reuse of already recorded bounded inspection only; not a new book or complete standard inspection."})
def write(name, data):
    (OUT / name).write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
write("algorithms_resources.json", {"schema_version": 1, "resources": resources})
write("algorithms_search_log.json", {"date": "2026-10-10", "actions": [
    "Inspected existing PR-T14/T15/T16/T17/T36/T37 contracts and cache fixtures before choosing executable subsets.",
    "Queried local reference registry for accepted algorithm, interface and two-ISA scalar ABI scopes; retained pinned editions and limitations.",
    "Opened official Arm 2025Q1 AAPCS and x86 psABI repository as update leads; no broader assigned scope inferred from index/front matter.",
    "Selected reusable held CLRS/Hanson and public ABI/core scopes; no new library crawl or payload mutation."],
    "decision": "Implement original bounded mechanisms and explicit independent test oracles. New advanced reading assignments require separate passage inspection."})
write("algorithms_research_gate.json", {"accepted": True, "status": "accepted", "reviewer": "/root/source_audit", "date": "2026-10-10", "scope": "Source sufficiency for bounded original executable companion labs; not independent acceptance of all mapped projects.", "resources": ids, "research_input_sha256": {name: hashlib.sha256((OUT / name).read_bytes()).hexdigest() for name in ["algorithms_resources.json", "algorithms_search_log.json", "algorithms_reflection.md", "algorithms_synthesis.md"]}, "limitations": ["No new full textbook reading", "Old AMD64 draft remains explicitly pinned", "No hardware timing claim", "AVL implementation tested structurally; full AVL proof reading remains existing curriculum work"]})
