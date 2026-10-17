"""Offline, local learning records. These records describe evidence, not certification.

The ledger is append-only and hash chained to detect accidental edits. It is not
a trusted clock, examiner, or protection against a learner rewriting all records.
"""
from __future__ import annotations

from datetime import datetime, timedelta, timezone
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import shutil
import uuid

from . import cli
from .text import normalize_prose, strip_bullet

DIMENSIONS = ("mechanism", "correctness", "memory_ownership", "resources_dependencies",
              "failure_diagnosis", "communication_transfer")
SCOPES = {"host", "model", "emulator", "physical"}
_CATALOG_CACHE = {}


def clock():
    return datetime.now(timezone.utc)


def stamp(value=None):
    return (value or clock()).astimezone(timezone.utc).isoformat()


def instant(value):
    parsed = datetime.fromisoformat(value)
    if parsed.tzinfo is None:
        raise ValueError("Timestamp must include a time zone")
    return parsed.astimezone(timezone.utc)


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(",", ":"),
                                     ensure_ascii=False).encode()).hexdigest()


def catalog_dir():
    """The edition manifest names the active catalog from schema 5 on; v4 is the default."""
    manifest = cli.ROOT / "curriculum/edition.json"
    try:
        value = cli.read(manifest) if manifest.is_file() else {}
    except (OSError, ValueError):
        value = {}
    if isinstance(value, dict) and isinstance(value.get("schema_version"), int) and value["schema_version"] >= 5:
        base = cli.ROOT / str(value.get("catalog_dir") or "")
        if value.get("catalog_dir") and (base / "units.json").is_file():
            return base
    return cli.ROOT / "curriculum/v4"


def catalog():
    base = catalog_dir()
    signature = (str(base),) + tuple((base / name).stat().st_mtime_ns for name in ("units.json", "recall.json", "artifacts.json"))
    if signature in _CATALOG_CACHE:
        return _CATALOG_CACHE[signature]
    units = cli.read(base / "units.json")
    recall = cli.read(base / "recall.json")
    artifacts = cli.read(base / "artifacts.json")
    result = {"units": {u["id"]: u for u in units["units"]}, "order": units["order"],
            "sets": {s["id"]: s for s in recall["sets"]}, "policy": recall["policy"],
            "artifacts": {a["id"]: a for a in artifacts["artifacts"]}}
    _CATALOG_CACHE.clear()
    _CATALOG_CACHE[signature] = result
    return result


def unit(identifier, project=None, family=None):
    value = catalog()["units"].get(identifier)
    if value is None:
        raise ValueError("Unknown learning unit: " + identifier)
    if project is not None and value["project"] != project:
        raise ValueError("Learning unit belongs to a different project")
    if family is not None and value.get("family") != family:
        raise ValueError("Learning unit requires family: " + str(value.get("family")))
    return value


def area(profile):
    return Path(profile).expanduser().resolve() / ".cmastery-study"


def settings(profile):
    value = cli.read(area(profile) / "profile.json")
    if value.get("schema_version") != 4:
        raise ValueError("Unsupported study profile version")
    for key in ("freshness_days", "session_minutes", "sets_per_session"):
        if type(value.get(key)) is not int or value[key] < 1:
            raise ValueError(key + " must be a positive whole number")
    if value["session_minutes"] > 15 or value["sets_per_session"] > 2:
        raise ValueError("A study session is bounded to 15 minutes and two recall sets; split a larger review")
    return value


def events(profile):
    settings(profile)
    path = area(profile) / "ledger.jsonl"
    previous = "0" * 64
    result = []
    previous_time = None
    if not path.exists():
        return result
    for line in path.read_text(encoding="utf-8").splitlines():
        item = json.loads(line)
        identity = item.pop("sha256", None)
        if item.get("previous") != previous or digest(item) != identity:
            raise ValueError("Study ledger hash chain differs; restore the original ledger")
        when = instant(item["at"])
        if previous_time is not None and when < previous_time:
            raise ValueError("Study ledger timestamps are out of order")
        if when > clock() + timedelta(minutes=5):
            raise ValueError("Study ledger contains a future timestamp; check the system clock")
        previous_time = when
        previous = identity
        item["sha256"] = identity
        result.append(item)
    return result


def append(profile, kind, **values):
    history = events(profile)
    when = clock()
    if history and when < instant(history[-1]["at"]):
        raise ValueError("Clock moved backwards; restore the clock before appending evidence")
    record = {"schema_version": 4, "id": uuid.uuid4().hex, "at": stamp(when), "kind": kind,
              "previous": history[-1]["sha256"] if history else "0" * 64, **values}
    record["sha256"] = digest(record)
    # Exclusive append lock avoids two writers producing the same previous hash.
    lock = area(profile) / "ledger.lock"
    acquired = False
    try:
        with lock.open("x", encoding="utf-8"):
            acquired = True
            current = events(profile)
            if (current[-1]["sha256"] if current else "0" * 64) != record["previous"]:
                raise ValueError("Concurrent ledger update; retry")
            with (area(profile) / "ledger.jsonl").open("a", encoding="utf-8", newline="\n") as stream:
                stream.write(json.dumps(record, ensure_ascii=False) + "\n")
    except FileExistsError as error:
        raise ValueError("Study ledger is in use; retry (a crashed writer may leave ledger.lock)") from error
    finally:
        # Do not remove a lock belonging to a different writer.
        if acquired:
            lock.unlink(missing_ok=True)
    return record


def study_init(profile):
    base = area(profile)
    if (base / "profile.json").exists():
        raise ValueError("Study profile already exists")
    data = catalog()
    policy = data["policy"]
    value = {"schema_version": 4, "created_at": stamp(), "gaps_days": policy.get("gaps_days", [1, 3, 7, 14, 30]),
             "freshness_days": policy.get("freshness_days", 14),
             "session_minutes": policy.get("session_minutes", 15), "sets_per_session": 2,
             "limits": "Local self-assessment; clock and authorship are not independently verified."}
    cli.save(base / "profile.json", value)
    append(profile, "profile-created")
    return {"profile": str(Path(profile).resolve()), "status": "initialized", "policy": value,
            "next": "study next PROFILE; after studying a unit use study next PROFILE --studied UNIT"}


def recall_states(profile):
    history = events(profile)
    cfg = settings(profile)
    gaps = cfg["gaps_days"]
    if not gaps or any(type(n) is not int or n < 1 for n in gaps):
        raise ValueError("gaps_days must contain positive whole days")
    states = {}
    starts = {}
    submissions = {}
    grades = {}
    sequence = {entry["id"]: i for i, entry in enumerate(history)}
    for entry in history:
        if entry["kind"] == "recall-activated":
            states.setdefault(entry["set"], {"set": entry["set"], "activated_at": entry["at"], "stage": 0,
                "due": stamp(instant(entry["at"]) + timedelta(days=gaps[0])), "attempts": [], "objectives": {},
                "contract_objectives": {}})
        elif entry["kind"] == "recall-started":
            starts[entry["id"]] = entry
        elif entry["kind"] == "recall-submitted":
            submissions[entry["attempt"]] = entry
        elif entry["kind"] == "recall-graded":
            grades[entry["attempt"]] = entry  # Last evaluation supersedes, never deletes.
    for attempt, start in starts.items():
        state = states[start["set"]]
        state["attempts"].append(attempt)
        if attempt not in grades:
            if attempt not in submissions:
                state["open_attempt"] = attempt
            else:
                state["ungraded_attempt"] = attempt
            continue
        grade = grades[attempt]
        submitted = submissions[attempt]
        independent = not submitted["assistance"] and not grade.get("assistance")
        success = independent and all(score >= 2 for score in grade["scores"].values())
        # Successful repetitions within the same day preserve evidence, but do
        # not compress the planned spacing sequence.
        spacing_met = state["due"] is not None and instant(start["at"]) >= instant(state["due"])
        if success:
            corrective_only = state.get("needs_delayed_retry", False) and not spacing_met
            for objective, score in grade["scores"].items():
                state["objectives"][objective] = {"score": score, "at": submitted["at"],
                    "attempt": attempt, "independent": True, "retained": not corrective_only,
                    "sequence": sequence[submitted["id"]], "contract": start.get("contract")}
                state["contract_objectives"].setdefault(start.get("contract") or "", {})[objective] = state["objectives"][objective]
            if spacing_met:
                state["needs_delayed_retry"] = False
                state["stage"] += 1
                state["due"] = (stamp(instant(submitted["at"]) + timedelta(days=gaps[state["stage"]]))
                                if state["stage"] < len(gaps) else None)
        else:
            for objective, score in grade["scores"].items():
                state["objectives"][objective] = {"score": min(score, 1) if not independent else score,
                    "at": submitted["at"], "attempt": attempt, "independent": independent,
                    "retained": False, "sequence": sequence[submitted["id"]], "contract": start.get("contract")}
                state["contract_objectives"].setdefault(start.get("contract") or "", {})[objective] = state["objectives"][objective]
            state["stage"] = 0
            state["needs_delayed_retry"] = True
            state["due"] = stamp(instant(submitted["at"]) + timedelta(days=1))
        state["last_result"] = ("corrective-independent-practice" if success and corrective_only else
                                "independent-success" if success else "correction-and-delayed-retry")
    return states


def vocabulary_set_for(value):
    """The owning domain's vocabulary recall set, when the catalog has one."""
    owner = next((p.get("module") for p in cli.catalog() if p["id"] == value["project"]), None)
    key = "V-" + str(owner)
    return key if owner and key in catalog()["sets"] else None


def mark_studied(profile, identifier):
    value = unit(identifier)
    record = append(profile, "unit-studied", unit=identifier, objectives=value["objectives"])
    existing = recall_states(profile)
    keys = list(value.get("recall_sets", []))
    vocabulary = vocabulary_set_for(value)
    if vocabulary:
        keys.append(vocabulary)
    for key in keys:
        if key not in catalog()["sets"]:
            raise ValueError("Unit references missing recall set: " + key)
        if key not in existing:
            append(profile, "recall-activated", set=key, unit=identifier)
    return record


def due(profile):
    current = clock()
    result = [state for state in recall_states(profile).values()
              if state["due"] is not None and instant(state["due"]) <= current]
    return sorted(result, key=lambda s: (s["due"], s["set"]))


def recall_start(profile, identifier, contract=None):
    data = catalog()["sets"]
    states = recall_states(profile)
    if identifier not in states:
        raise ValueError("Study the related unit before activating this recall set")
    state = states[identifier]
    if state.get("open_attempt") or state.get("ungraded_attempt"):
        raise ValueError("Finish and grade the existing attempt before starting another")
    value = data[identifier]
    variant = value["variants"][len(state["attempts"]) % len(value["variants"])]
    entry = append(profile, "recall-started", set=identifier, variant=variant["id"],
                   objectives=variant["objectives"], prompt=variant["prompt"], contract=contract)
    return {"attempt": entry["id"], "set": identifier, "variant": variant["id"], "prompt": normalize_prose(variant["prompt"]),
            "objectives": variant["objectives"], "limits": "Submit your original attempt before viewing feedback."}


def find_event(profile, identifier, kind):
    result = next((e for e in events(profile) if e["id"] == identifier and e["kind"] == kind), None)
    if result is None:
        raise ValueError("Unknown " + kind + " record: " + identifier)
    return result


def recall_submit(profile, attempt, answer_file, assistance=None):
    start = find_event(profile, attempt, "recall-started")
    if any(e["kind"] == "recall-submitted" and e["attempt"] == attempt for e in events(profile)):
        raise ValueError("An original answer cannot be replaced; make a new attempt after grading")
    answer = Path(answer_file).read_text(encoding="utf-8-sig").strip()
    if not answer:
        raise ValueError("Provide an answer, including an explicit admission if you cannot recall it")
    entry = append(profile, "recall-submitted", attempt=attempt, answer=answer,
                   answer_sha256=hashlib.sha256(answer.encode()).hexdigest(), assistance=assistance or [])
    value = catalog()["sets"][start["set"]]
    variant = next(v for v in value["variants"] if v["id"] == start["variant"])
    return {"submission": entry["id"], "attempt": attempt, "checklist": variant.get("checklist", []),
            "objectives": start["objectives"], "resources": value.get("resource_ids", []),
            "next": "Grade each prompted objective 0–3 with feedback; assisted answers do not establish independent recall."}


def recall_grade(profile, attempt, assessment_file):
    start = find_event(profile, attempt, "recall-started")
    history = events(profile)
    if not any(e["kind"] == "recall-submitted" and e["attempt"] == attempt for e in history):
        raise ValueError("Submit the original answer before grading")
    assessment = cli.read(assessment_file)
    scores = assessment.get("scores", {})
    if set(scores) != set(start["objectives"]) or any(type(n) is not int or n not in range(4) for n in scores.values()):
        raise ValueError("Score exactly the objectives prompted in this attempt, using integers 0–3")
    if not isinstance(assessment.get("feedback"), str) or not assessment["feedback"].strip():
        raise ValueError("Explain the grading and correction in feedback")
    previous = [e for e in history if e["kind"] == "recall-graded" and e["attempt"] == attempt]
    entry = append(profile, "recall-graded", attempt=attempt, scores=scores, feedback=assessment["feedback"],
                   assistance=assessment.get("assistance", []), supersedes=previous[-1]["id"] if previous else None)
    return {"evaluation": entry["id"], "state": recall_states(profile)[start["set"]]}


def safe_relative(value):
    if not isinstance(value, str) or not value or "\\" in value:
        raise ValueError("Use a nonempty relative path with forward slashes")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or ":" in value or path.as_posix() != value:
        raise ValueError("Path must remain within its declared root: " + value)
    if any(p.lower() in {"assessor", ".git", ".cmastery-study"} for p in path.parts):
        raise ValueError("Assessor and private metadata cannot be exported as learner implementation")
    return value


def local_file(base, relative):
    safe_relative(relative)
    root = Path(base).resolve()
    source = root / relative
    if not source.is_file() or source.is_symlink() or not source.resolve().is_relative_to(root):
        raise ValueError("Missing or escaping local evidence file: " + relative)
    for ancestor in source.parents:
        if ancestor == root:
            break
        if ancestor.is_symlink():
            raise ValueError("Symlink directories cannot provide portable evidence")
    return source


def evidence_files(workspace, names):
    if not isinstance(names, list) or not names:
        raise ValueError("Each claim needs at least one local evidence file")
    return {name: hashlib.sha256(local_file(workspace, name).read_bytes()).hexdigest() for name in names}


def verify_files(root, records):
    return all(hashlib.sha256(local_file(root, name).read_bytes()).hexdigest() == value for name, value in records.items())


def decision_seal(profile, workspace, identifier, decision_file):
    value = unit(identifier)
    workspace = Path(workspace).resolve()
    meta = cli.read(workspace / ".cmastery/project.json")
    unit(identifier, meta["project"], meta.get("family"))
    if value.get("decision_before") and any(e["kind"] == "unit-accepted" and e["unit"] == value["decision_before"] for e in events(profile)):
        raise ValueError("Seal the design before accepting the first implementation unit; use a new disclosed transfer attempt")
    decision = cli.read(decision_file)
    for key in ("requirements", "predictions", "choice", "rejected_reason", "reversal_trigger"):
        if not decision.get(key):
            raise ValueError("Decision needs " + key)
    alternatives = decision.get("alternatives", [])
    if len(alternatives) < 2 or len({json.dumps(a, sort_keys=True) for a in alternatives}) != len(alternatives):
        raise ValueError("Record at least two distinct plausible alternatives")
    spikes = evidence_files(workspace, decision.get("spike_evidence", []))
    previous = [e for e in events(profile) if e["kind"] == "decision-sealed" and e["unit"] == identifier]
    entry = append(profile, "decision-sealed", unit=identifier, unit_contract=digest(value),
                   workspace=str(workspace), source=cli.snapshot(workspace), decision=decision, spike_files=spikes,
                   supersedes=previous[-1]["id"] if previous else None)
    return {"decision": entry["id"], "unit": identifier, "sealed_at": entry["at"],
            "limits": "A timestamped local design record cannot prove when the implementation was conceived."}


def linked(workspace):
    path = Path(workspace) / ".cmastery/artifacts.json"
    return cli.read(path).get("artifacts", {}) if path.exists() else {}


def verify_bundle(bundle):
    bundle = Path(bundle).resolve()
    manifest = cli.read(bundle / "artifact.json")
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", manifest.get("cmake_target", "")):
        raise ValueError("Invalid artifact CMake target")
    if any(not re.fullmatch(r"[a-z][a-z0-9_]*", role) for role in manifest.get("adapter_roles", [])):
        raise ValueError("Invalid component adapter role")
    unsigned = dict(manifest)
    expected = unsigned.pop("sha256", None)
    if digest(unsigned) != expected or not verify_files(bundle, manifest["files"]):
        raise ValueError("Pinned artifact content or manifest changed")
    actual_files = {p.relative_to(bundle).as_posix() for p in bundle.rglob("*") if p.is_file()}
    expected_files = set(manifest["files"]) | {"artifact.json"}
    for dependency, declaration in manifest.get("dependencies", {}).items():
        if not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", dependency):
            raise ValueError("Invalid dependency artifact identity")
        nested = verify_bundle(bundle / "dependencies" / dependency)
        if nested["sha256"] != declaration["sha256"]:
            raise ValueError("Pinned dependency closure differs")
        expected_files |= {p.relative_to(bundle).as_posix() for p in (bundle / "dependencies" / dependency).rglob("*") if p.is_file()}
    if actual_files != expected_files:
        raise ValueError("Artifact contains files outside its declared closure")
    return manifest


def cmake_literal(value):
    """A literal CMake argument: no variable expansion, list or quote injection."""
    value = str(value)
    equals = "="
    while "]" + equals + "]" in value:
        equals += "="
    return "[" + equals + "[" + value + "]" + equals + "]"


def prepare_component_build(workspace, output):
    """Regenerate imports from verified bundles; never execute edited metadata.

    Consumers opt into CMASTERY_COMPONENTS_FILE and the role target variables.
    Paths refer to pinned local copies, never a repository assessor directory.
    """
    workspace = Path(workspace).resolve()
    output = Path(output).resolve()
    manifests = linked_state(workspace)
    references = linked(workspace)
    lines = ["# Verified learner bundles. Generated afresh for this build."]
    roles = {}
    targets_seen = {}
    provider_sources = {}
    def inspect_closure(source):
        manifest = verify_bundle(source)
        target = manifest["cmake_target"]
        if target in targets_seen and targets_seen[target] != manifest["sha256"]:
            raise ValueError("Pinned closure gives conflicting implementations for target: " + target)
        targets_seen[target] = manifest["sha256"]
        for role in manifest.get("adapter_roles", []):
            roles.setdefault(role, [])
            if target not in roles[role]:
                roles[role].append(target)
            for name in manifest.get("sources", []):
                key = (role, name)
                identity = manifest["files"][name]
                if key in provider_sources and provider_sources[key] != identity:
                    raise ValueError("Conflicting provider snapshots for " + role + "/" + name +
                                     "; revalidate and re-export retained components from one coherent provider revision, or split the implementations into distinct source files")
                provider_sources[key] = identity
        for dependency in manifest.get("dependencies", {}):
            inspect_closure(source / "dependencies" / dependency)
    for identifier, manifest in manifests.items():
        target = manifest["cmake_target"]
        source = (workspace / references[identifier]["path"]).resolve()
        binary = output / "components" / target
        inspect_closure(source)
        lines += ["if(NOT TARGET " + target + ")", "  add_subdirectory(" + cmake_literal(source.as_posix()) +
                  " " + cmake_literal(binary.as_posix()) + ")", "endif()"]
    path = output / "verified-components.cmake"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    arguments = ["-DCMASTERY_COMPONENTS_FILE=" + str(path)]
    arguments += ["-DCMASTERY_COMPONENTS_" + role.upper() + "_TARGETS=" + ";".join(targets)
                  for role, targets in sorted(roles.items())]
    return {"arguments": arguments, "roles": roles, "sha256": {key: value["sha256"] for key, value in manifests.items()},
            "generated_file_sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


def linked_state(workspace):
    result = {}
    for identifier, link in linked(workspace).items():
        bundle = Path(workspace) / link["path"]
        safe_relative(link["path"])
        manifest = verify_bundle(bundle)
        if manifest["id"] != identifier or manifest["sha256"] != link["sha256"]:
            raise ValueError("Artifact link identity differs")
        result[identifier] = manifest
    return result


def acceptance_valid(entry):
    try:
        workspace = Path(entry["workspace"])
        if not verify_files(workspace, entry.get("source_files", {})):
            return False, "claimed component source changed"
        if digest(unit(entry["unit"])) != entry["unit_contract"]:
            return False, "unit contract changed"
        if not verify_files(workspace, entry.get("evidence_files", {})):
            return False, "claim evidence changed"
        current = linked_state(workspace)
        for identifier, identity in entry.get("imports", {}).items():
            if current.get(identifier, {}).get("sha256") != identity:
                return False, "consumed artifact changed"
        return True, None
    except (ValueError, OSError, KeyError):
        return False, "source, evidence, or artifact is unavailable"


def retain_revision(profile, workspace, evidence):
    """Deduplicate accepted source/evidence bytes independently of live files."""
    source = cli.snapshot(workspace)["files"]
    for name, identity in dict(source, **evidence).items():
        path = local_file(workspace, name)
        raw = path.read_bytes()
        if hashlib.sha256(raw).hexdigest() != identity:
            raise ValueError("Source/evidence changed while retaining accepted revision")
        target = area(profile) / "objects" / identity
        target.parent.mkdir(parents=True, exist_ok=True)
        if target.exists():
            if hashlib.sha256(target.read_bytes()).hexdigest() != identity:
                raise ValueError("Retained evidence object changed")
        else:
            target.write_bytes(raw)
    return {"source": source, "evidence": evidence}


def historical_valid(profile, entry, checked):
    if digest(unit(entry["unit"])) != entry["unit_contract"]:
        return False, "unit contract changed"
    revision = entry.get("revision")
    if revision is None:
        return acceptance_valid(entry)  # Compatible reading of earlier v4 drafts.
    for identity in set(revision["source"].values()) | set(revision["evidence"].values()):
        if not re.fullmatch(r"[0-9a-f]{64}", identity):
            return False, "invalid retained object identity"
        if identity not in checked:
            path = area(profile) / "objects" / identity
            checked[identity] = path.is_file() and hashlib.sha256(path.read_bytes()).hexdigest() == identity
        if not checked[identity]:
            return False, "accepted revision object missing or changed"
    return True, None


def accepted(profile):
    latest = {}
    for entry in events(profile):
        if entry["kind"] == "unit-accepted":
            latest[entry["unit"]] = entry
    result = {}
    checked = {}
    def evaluate(identifier, visiting):
        if identifier in result:
            return result[identifier]
        if identifier in visiting:
            return {"valid": False, "reason": "cyclic portfolio claim"}
        entry = latest[identifier]
        valid, reason = acceptance_valid(entry)
        past_valid, past_reason = historical_valid(profile, entry, checked)
        for reference in entry.get("portfolio_refs", []):
            source = latest.get(reference["unit"])
            if not source or source["id"] != reference["acceptance"]:
                valid, reason = False, "portfolio source acceptance missing or superseded"
                past_valid, past_reason = False, reason
                break
            other = evaluate(reference["unit"], visiting | {identifier})
            if not other["valid"]:
                valid, reason = False, "portfolio source evidence changed"
            if not other.get("historical_valid"):
                past_valid, past_reason = False, "portfolio accepted revision changed"
        result[identifier] = {"record": entry, "valid": valid and past_valid, "reason": reason or past_reason,
                              "historical_valid": past_valid, "historical_reason": past_reason}
        return result[identifier]
    for identifier in latest:
        evaluate(identifier, set())
    return result


def claim_constraints(required, submitted):
    """The catalog sets minimum obligations; an answer cannot weaken them."""
    def values(record, singular, plural):
        if singular in record and plural in record:
            raise ValueError("Use either " + singular + " or " + plural + " in a claim")
        value = record.get(plural, [record[singular]] if singular in record else [])
        if not isinstance(value, list):
            raise ValueError(plural + " must be a list")
        return value
    needed_targets = values(required, "target", "targets")
    declared_targets = values(submitted, "target", "targets")
    targets = sorted(set(needed_targets + declared_targets))
    if any(not isinstance(target, str) or not target for target in targets):
        raise ValueError("Claim targets must be nonempty identifiers")
    needed_cpus = required.get("cpus", [])
    declared_cpus = submitted.get("cpus", [])
    cpus = (needed_cpus if isinstance(needed_cpus, list) else [needed_cpus]) + (declared_cpus if isinstance(declared_cpus, list) else [declared_cpus])
    if any(type(count) is not int or count < 1 for count in cpus):
        raise ValueError("Claim CPU counts must be positive integers")
    interfaces = required.get("interfaces", []) + submitted.get("interfaces", [])
    if any(not isinstance(name, str) or not name for name in interfaces):
        raise ValueError("Claim interfaces must be nonempty names")
    tests = required.get("tests", []) + submitted.get("tests", [])
    return {"targets": targets, "cpus": sorted(set(cpus)), "interfaces": sorted(set(interfaces)), "tests": sorted(set(tests))}


def current_check_receipts(workspace, identifier):
    source = cli.snapshot(workspace)["sha256"]
    paths = sorted((workspace / "evidence/checks").glob("*.json"))
    latest = workspace / "evidence/latest.json"
    if latest.is_file():
        paths.append(latest)
    result = []
    for path in paths:
        record = cli.read(path)
        if (record.get("status") != "passed" or record.get("assessment") != "learner"
            or record.get("unit") != identifier or record.get("source", {}).get("sha256") != source
            or record.get("unit_contract") != digest(unit(identifier))):
            continue
        if any(not Path(p).is_dir() or cli.snapshot(Path(p))["sha256"] != h for p,h in record.get("external_sources", {}).items()):
            continue
        if record.get("component_links") is not None and record["component_links"] != {k:v["sha256"] for k,v in linked_state(workspace).items()}:
            continue
        if path == latest and record.get("receipt_path") and (workspace / record["receipt_path"]).is_file():
            continue
        result.append((path, record))
    return result


def local_observations(workspace, identifier, scope, report):
    """Collect current evidence at a real target, independently of prose claims."""
    result = []
    source = cli.snapshot(workspace)["sha256"]
    meta = cli.read(workspace / ".cmastery/project.json")
    if scope == "physical":
        for path in sorted((workspace / "evidence/physical").glob("*/receipt.json")):
            record = cli.read(path)
            if record.get("project") != meta["project"] or record.get("source_sha256") != source:
                continue
            if not any(r.get("recorded_at") == record.get("recorded_at") and r.get("status") == "recorded" and not r.get("stale") for r in report["physical_records"]):
                continue
            names = [path.relative_to(workspace).as_posix()] + [(path.parent / name).relative_to(workspace).as_posix() for name in record.get("artifacts", {})]
            result.append({"scope": scope, "target": record.get("target"), "cpu_count": record.get("cpu_count"),
                "interfaces": record.get("interfaces", []), "source_sha256": source,
                "evidence_files": evidence_files(workspace, names)})
    elif scope == "emulator":
        for path, record in current_check_receipts(workspace, identifier):
            if "emulator" not in record.get("evidence_kind", "") or not cli.emulator_test_names(record):
                continue
            result.append({"scope": scope, "target": record.get("target"), "cpu_count": record.get("cpu_count"),
                "interfaces": [], "source_sha256": source, "tests": cli.emulator_test_names(record),
                "evidence_files": evidence_files(workspace, [path.relative_to(workspace).as_posix()])})
    return result


def match_observations(scope, constraints, observations):
    chosen = []
    targets = constraints["targets"] or [None]
    cpus = constraints["cpus"] or [None]
    for target in targets:
        for cpus_required in cpus:
            candidates = [o for o in observations if o["scope"] == scope and
                          (target is None or o.get("target") == target) and
                          (cpus_required is None or o.get("cpu_count") == cpus_required) and
                          set(constraints.get("tests", [])) <= set(o.get("tests", []))]
            observed_interfaces = {name for o in candidates for name in o.get("interfaces", [])}
            if not candidates or not set(constraints["interfaces"]) <= observed_interfaces:
                raise ValueError("Required " + scope + " evidence missing for target=" + str(target or "declared target") +
                                 ", cpus=" + str(cpus_required or "unspecified") +
                                 ", interfaces=" + ",".join(constraints["interfaces"]))
            chosen.extend(candidates)
    return chosen


def freshness(profile, objectives, contract=None):
    latest = {}
    current = {}
    for state in recall_states(profile).values():
        for identifier, result in state["objectives"].items():
            if identifier not in current or result["sequence"] > current[identifier]["sequence"]:
                current[identifier] = result
        results = state["objectives"] if contract is None else state["contract_objectives"].get(contract, {})
        for identifier, result in results.items():
            if identifier not in latest or result["sequence"] > latest[identifier]["sequence"]:
                latest[identifier] = result
    threshold = clock() - timedelta(days=settings(profile)["freshness_days"])
    return [identifier for identifier in sorted(set(objectives))
            if identifier not in latest or not latest[identifier]["independent"] or latest[identifier]["score"] < 2
            or not latest[identifier].get("retained", False)
            or instant(latest[identifier]["at"]) < threshold
            or (identifier in current and current[identifier]["sequence"] > latest[identifier]["sequence"]
                and not current[identifier].get("retained", False))]


def acceptance(profile, workspace, identifier, assessment_file):
    value = unit(identifier)
    workspace = Path(workspace).expanduser().resolve()
    meta = cli.read(workspace / ".cmastery/project.json")
    unit(identifier, meta["project"], meta.get("family"))
    history = events(profile)
    if not any(e["kind"] == "unit-studied" and e["unit"] == identifier for e in history):
        raise ValueError("Record the unit as studied before requesting acceptance")
    prerequisites = accepted(profile)
    missing = [r for r in value.get("requires", []) if not prerequisites.get(r, {}).get("historical_valid")]
    if missing:
        raise ValueError("Acceptance prerequisites not current: " + ", ".join(missing))
    imports = linked_state(workspace)
    missing = set(value.get("imports", [])) - set(imports)
    if missing:
        raise ValueError("Required learner artifacts not linked: " + ", ".join(sorted(missing)))
    artifact_catalog = catalog()["artifacts"]
    for artifact in value.get("imports", []):
        contract = artifact_catalog[artifact]
        if str(imports[artifact]["contract_version"]) != str(contract["contract_version"]):
            raise ValueError("Artifact contract version differs: " + artifact)
        missing = freshness(profile, contract.get("critical_objectives", []), artifact + "@" + str(contract["contract_version"]))
        if missing:
            raise ValueError("Consumption preflight needs fresh independent recall for " + artifact + ": " + ", ".join(missing))
    missing = freshness(profile, value.get("critical_objectives", []))
    if missing:
        raise ValueError("Critical recall needs an independent preflight: " + ", ".join(missing))
    assessment = cli.read(assessment_file)
    if not assessment.get("explanation") or not assessment.get("transfer"):
        raise ValueError("Acceptance needs explanation and changed-condition transfer evidence")
    if assessment.get("independence", {}).get("unaided") is not True or assessment.get("independence", {}).get("assistance"):
        raise ValueError("Acceptance requires a disclosed independent attempt")
    full = bool(value.get("milestone") or value.get("exports"))
    if full:
        dimensions = assessment.get("dimensions", {})
        if set(dimensions) != set(DIMENSIONS) or any(type(n) is not int or n not in (2, 3) for n in dimensions.values()) or list(dimensions.values()).count(3) < 2:
            raise ValueError("Milestone rubric requires all six dimensions >=2 and at least two scores of 3")
    checks = value.get("checks", [])
    demonstrated = assessment.get("checks", [])
    if checks and (len(demonstrated) != len(checks) or any(d.get("met") is not True or d.get("check") != check for d, check in zip(demonstrated, checks))):
        raise ValueError("Assess every unit checklist item in its published order")
    files = evidence_files(workspace, assessment.get("evidence", []))
    report = cli.report(workspace)
    receipt = report["evidence"]
    if value.get("tests"):
        eligible = [(path,record) for path,record in current_check_receipts(workspace, identifier)
                    if set(value["tests"]) <= {t["name"] for t in record.get("tests", []) if not t.get("failed") and not t.get("skipped")}]
        if not eligible:
            raise ValueError("A current learner check --unit receipt is required; assessor validation cannot establish learner acceptance")
        path, receipt = eligible[-1]
        if path.name != "latest.json":
            files.update(evidence_files(workspace, [path.relative_to(workspace).as_posix()]))
    claims = assessment.get("claims", [])
    by_id = {claim["id"]: claim for claim in claims}
    required_claims = value.get("claims", []) + value.get("completion_claims", [])
    requirements = {c["id"]: c for c in required_claims if isinstance(c, dict)}
    for required in required_claims:
        key = required["id"] if isinstance(required, dict) else required
        if key not in by_id:
            raise ValueError("Missing scoped claim: " + key)
        if isinstance(required, dict) and required.get("scope") and by_id[key].get("scope") != required["scope"]:
            raise ValueError("Claim scope differs: " + key)
    verified_claims = []
    portfolio_refs = {}
    for claim in claims:
        if claim.get("scope") not in SCOPES:
            raise ValueError("Claim scope must distinguish host, model, emulator, and physical")
        files.update(evidence_files(workspace, claim.get("evidence", [])))
        required = requirements.get(claim["id"], {})
        constraints = claim_constraints(required, claim)
        source_units = required.get("source_units", [])
        observations = []
        if source_units:
            for source_unit in source_units:
                selected = prerequisites.get(source_unit)
                if not selected or not selected.get("historical_valid"):
                    raise ValueError("Portfolio needs an accepted source revision: " + source_unit)
                source_entry = selected["record"]
                portfolio_refs[source_unit] = {"unit": source_unit, "acceptance": source_entry["id"]}
                for previous_claim in source_entry.get("verified_claims", []):
                    observations += previous_claim.get("observations", [])
        elif claim["scope"] in {"physical", "emulator"}:
            observations = local_observations(workspace, identifier, claim["scope"], report)
        if claim["scope"] in {"physical", "emulator"}:
            selected_observations = match_observations(claim["scope"], constraints, observations)
            if not source_units:
                for observation in selected_observations:
                    files.update(observation["evidence_files"])
            verified_claims.append({"id": claim["id"], "scope": claim["scope"], "constraints": constraints,
                                    "observations": selected_observations, "source_units": source_units})
        else:
            if constraints["targets"] or constraints["cpus"] or constraints["interfaces"]:
                raise ValueError("Target/interface completion claims require emulator or physical evidence")
            verified_claims.append({"id": claim["id"], "scope": claim["scope"], "observations": []})
    decisions = [e for e in history if e["kind"] == "decision-sealed" and e["unit"] == identifier]
    design = None
    if value.get("decision_required"):
        if not decisions:
            raise ValueError("This boundary needs a sealed architecture decision")
        design = decisions[-1]
        if design["workspace"] != str(workspace) or design["unit_contract"] != digest(value):
            raise ValueError("Sealed design belongs to another workspace or unit contract")
        if value.get("tests") and instant(design["at"]) >= instant(receipt["recorded_at"]):
            raise ValueError("Seal the design before running the acceptance checks")
        if design["source"]["sha256"] == cli.snapshot(workspace)["sha256"]:
            raise ValueError("No implementation change is recorded after the sealed decision")
        if not verify_files(workspace, design["spike_files"]):
            raise ValueError("Preserve the original decision experiment evidence")
        files.update(design["spike_files"])
    previous = [e for e in history if e["kind"] == "unit-accepted" and e["unit"] == identifier]
    source_scope = assessment.get("source_files", [])
    if not source_scope and any(c["scope"] in {"physical", "emulator"} and not c.get("source_units") for c in verified_claims):
        source_scope = [name for name in cli.snapshot(workspace)["files"] if name not in {"UNIT.md", "PROJECT.md", "CONTRACTS.md", "COMPONENT-CONTRACTS.md"}]
    revision = retain_revision(profile, workspace, files)
    entry = append(profile, "unit-accepted", unit=identifier, workspace=str(workspace),
        source=cli.snapshot(workspace), unit_contract=digest(value), evidence_files=files,
        source_files=evidence_files(workspace, source_scope) if source_scope else {},
        revision=revision, verified_claims=verified_claims, portfolio_refs=list(portfolio_refs.values()),
        executable_receipt=receipt if value.get("tests") else None,
        imports={a: imports[a]["sha256"] for a in value.get("imports", [])},
        assessment=assessment, decision=design["id"] if design else None,
        supersedes=previous[-1]["id"] if previous else None,
        limits="Recorded self-assessment; executable, physical, explanation and independence claims remain distinct.")
    return {"acceptance": entry["id"], "unit": identifier, "status": "accepted", "project_completion": value.get("project_completion", False),
            "limits": entry["limits"]}


def study_status(profile):
    history = events(profile)
    assessed = accepted(profile)
    historical = {key for key, result in assessed.items() if result["historical_valid"]}
    data = catalog()
    core_units = {key for key, value in data["units"].items() if value.get("status", "core") != "elective"}
    core_projects = {data["units"][key]["project"] for key in core_units}
    gates = {u["project"]: key for key, u in data["units"].items() if u.get("project_completion")}
    completed = {project for project, key in gates.items() if key in historical}
    from . import exercises
    expansion = exercises.progress(profile)
    return {"profile": str(Path(profile).resolve()), "studied": sorted({e["unit"] for e in history if e["kind"] == "unit-studied"}),
            "accepted": sorted(historical),
            "current_acceptances": [key for key, result in assessed.items() if result["valid"]],
            "reopened": {key: result["reason"] for key, result in assessed.items() if not result["valid"]},
            "due_sets": len(due(profile)), "active_sets": len(recall_states(profile)),
            "expansion_completion": expansion,
            "comprehensive_complete": core_projects <= completed and core_units <= historical and expansion["complete"],
            "completion": {"projects": sorted(completed), "pending_projects": sorted(core_projects - completed),
                           "missing_completion_gates": sorted(core_projects - set(gates)),
                           "core_route_complete": core_projects <= completed and core_units <= historical,
                           "basis": "Accepted immutable revisions including required scoped observations; live source claims are listed separately."},
            "limits": "Due recall does not revoke historical implementation evidence. Only scoped acceptance consumes current prerequisites."}


def study_next(profile, studied=None, studied_exercise=None):
    if studied and studied_exercise:
        raise ValueError("Record either a studied unit or a studied exercise")
    if studied:
        mark_studied(profile, studied)
    if studied_exercise:
        from . import exercises
        exercises.studied(profile, studied_exercise)
    state = study_status(profile)
    accepted_ids = set(state["accepted"])
    data = catalog()
    available = [data["units"][key] for key in data["order"]
                 if key not in accepted_ids and set(data["units"][key].get("requires", [])) <= accepted_ids]
    cfg = settings(profile)
    return {"review_minutes": cfg["session_minutes"], "recall": due(profile)[:cfg.get("sets_per_session", 2)],
            "available_units": [{"id": u["id"], "title": u["title"], "hours": u["hours"], "project": u["project"],
                                  "decision_required": u.get("decision_required", False)} for u in available[:8]],
            "available_exercises": state["expansion_completion"]["available_exercises"],
            "limits": "Practice and check remain available even when an acceptance prerequisite needs repair."}


def study_sheet(profile, output, identifier=None, assessment_template=None):
    packet = study_next(profile)
    lines = ["# C Mastery study sheet", "", "Date / session: __________", "",
             "## Retrieval (10–15 minutes)", "", "Write the attempt before opening references or feedback.", ""]
    states = recall_states(profile)
    for state in packet["recall"]:
        value = catalog()["sets"][state["set"]]
        variant = value["variants"][len(states[state["set"]]["attempts"]) % len(value["variants"])]
        lines += ["### " + state["set"] + " / " + variant["id"], "", normalize_prose(variant["prompt"]), "",
                  "Original attempt: __________________________________________________", "",
                  "Assistance used: __________   Scores by objective: __________", "",
                  "Correction and next independent attempt: ___________________________", ""]
    chosen = unit(identifier) if identifier else (unit(packet["available_units"][0]["id"]) if packet["available_units"] else None)
    if chosen:
        lines += ["## " + chosen["id"] + " — " + normalize_prose(chosen["title"]), "", "Prediction / invariant: _________________________________", ""]
        lines += ["- [ ] " + normalize_prose(strip_bullet(step)) for step in chosen.get("steps", [])]
        lines += ["", "Evidence / changed-condition transfer: __________________", "",
                  "Architecture choice, alternatives and reversal condition: __________________", ""]
    lines += ["## Enter this record", "", "Use study next --studied UNIT after study, then recall start / submit / grade.",
              "Paper dates may be described in the answer; CLI records entry time and never backdates mastery.", ""]
    Path(output).write_text("\n".join(lines), encoding="utf-8")
    if assessment_template:
        if not chosen:
            raise ValueError("Select a unit for the assessment template")
        template = {"unit": chosen["id"], "explanation": "", "transfer": "",
                    "independence": {"unaided": False, "assistance": []}, "evidence": [], "source_files": [],
                    "checks": [{"check": check, "met": False} for check in chosen.get("checks", [])],
                    "claims": [{"id": c["id"] if isinstance(c, dict) else c,
                                "scope": c.get("scope", "host") if isinstance(c, dict) else "host",
                                **({key: c[key] for key in ('target', 'targets', 'cpus', 'interfaces', 'source_units', 'tests') if key in c} if isinstance(c, dict) else {}),
                                "evidence": []} for c in chosen.get("claims", []) + chosen.get("completion_claims", [])]}
        if chosen.get("milestone") or chosen.get("exports"):
            template["dimensions"] = {key: 0 for key in DIMENSIONS}
        cli.save(assessment_template, template)
    return {"sheet": str(Path(output).resolve()), "format": "printable Markdown", "contains_answers": False}


def artifact_export(profile, workspace, identifier, destination, declaration_file):
    workspace = Path(workspace).expanduser().resolve()
    destination = Path(destination).expanduser().resolve()
    if "assessor" in {p.lower() for p in workspace.parts}:
        raise ValueError("Assessor sources cannot be exported as learner artifacts")
    declaration = cli.read(declaration_file)
    specification = catalog()["artifacts"].get(identifier)
    if specification is None:
        raise ValueError("Unknown artifact: " + identifier)
    source_acceptance = accepted(profile).get(specification["producer_unit"])
    if not source_acceptance or not source_acceptance["valid"] or source_acceptance["record"]["workspace"] != str(workspace):
        raise ValueError("Export requires current learner acceptance of the producer unit in this workspace")
    if source_acceptance["record"]["source"]["sha256"] != cli.snapshot(workspace)["sha256"]:
        raise ValueError("Producer source changed after acceptance; revalidate before exporting")
    if str(declaration.get("contract_version")) != str(specification["contract_version"]):
        raise ValueError("Artifact contract version differs")
    target = declaration.get("cmake_target", "")
    if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", target):
        raise ValueError("Declare a portable CMake target name")
    producer_family = unit(specification["producer_unit"]).get("family")
    roles = declaration.get("adapter_roles", [producer_family] if producer_family else [])
    if not isinstance(roles, list) or any(not isinstance(role, str) or not re.fullmatch(r"[a-z][a-z0-9_]*", role) for role in roles):
        raise ValueError("Adapter roles must be explicit lowercase family identifiers")
    profiles = declaration.get("profiles", [])
    targets = declaration.get("targets", [])
    if not profiles or not targets or not set(profiles) <= set(specification["profiles"]) or not set(targets) <= set(specification["targets"]):
        raise ValueError("Export profiles and targets must be supported by the artifact contract")
    sources = declaration.get("sources", [])
    headers = declaration.get("headers", [])
    assets = declaration.get("assets", [])
    if not sources or not headers or any(not re.fullmatch(r"[A-Za-z0-9_./-]+", p) for p in sources + headers + assets):
        raise ValueError("Declare sources and public headers using portable file names")
    selected = {name: local_file(workspace, name) for name in sources + headers + assets}
    includes = declaration.get("include_dirs", [])
    if not includes:
        raise ValueError("Declare public include directories")
    for folder in includes:
        safe_relative(folder)
        if not re.fullmatch(r"[A-Za-z0-9_./-]+", folder) or not any(h.startswith(folder + "/") for h in headers):
            raise ValueError("Include directory must contain a declared public header")
    deps = linked_state(workspace)
    dependency_ids = declaration.get("dependencies", [])
    if not isinstance(dependency_ids, list) or set(dependency_ids) != set(unit(specification["producer_unit"]).get("imports", [])):
        raise ValueError("Export must declare the producer's complete imported artifact closure")
    if not set(dependency_ids) <= set(deps):
        raise ValueError("Export dependency is not linked")
    if target in {deps[d]["cmake_target"] for d in dependency_ids}:
        raise ValueError("Producer CMake target must differ from its dependencies")
    if destination == workspace or destination.is_relative_to(workspace) or workspace.is_relative_to(destination):
        raise ValueError("Export to a separate artifact directory")
    if destination.exists() and any(destination.iterdir()):
        raise ValueError("Artifact destination must be empty")
    destination.mkdir(parents=True, exist_ok=True)
    for name, source in selected.items():
        (destination / name).parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination / name)
    cmake = ["cmake_minimum_required(VERSION 3.20)", "project(" + target + " C ASM)"]
    closure = {}
    for dependency in dependency_ids:
        if not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", dependency):
            raise ValueError("Nonportable artifact identity")
        pinned = workspace / linked(workspace)[dependency]["path"]
        shutil.copytree(pinned, destination / "dependencies" / dependency)
        closure[dependency] = {"sha256": deps[dependency]["sha256"], "cmake_target": deps[dependency]["cmake_target"]}
        cmake += ["if(NOT TARGET " + deps[dependency]["cmake_target"] + ")",
                  '  add_subdirectory("dependencies/' + dependency + '")', "endif()"]
    cmake += ["add_library(" + target + " STATIC " + " ".join('"' + s + '"' for s in sources) + ")",
              "target_include_directories(" + target + " PUBLIC " + " ".join('"${CMAKE_CURRENT_SOURCE_DIR}/' + s + '"' for s in includes) + ")",
              "target_compile_features(" + target + " PUBLIC c_std_17)"]
    if dependency_ids:
        cmake += ["target_link_libraries(" + target + " PUBLIC " + " ".join(deps[d]["cmake_target"] for d in dependency_ids) + ")"]
    (destination / "CMakeLists.txt").write_text("\n".join(cmake) + "\n", encoding="utf-8")
    own = list(selected) + ["CMakeLists.txt"]
    manifest = {"schema_version": 4, "id": identifier, "contract_version": specification["contract_version"],
                "public_contract": specification["public_contract"], "cmake_target": target, "sources": sources,
                "headers": headers, "include_dirs": includes, "adapter_roles": roles,
                "profiles": profiles, "targets": targets, "dependencies": closure,
                "source_identity": cli.snapshot(workspace)["sha256"], "producer_acceptance": source_acceptance["record"]["id"],
                "claims": source_acceptance["record"]["assessment"].get("claims", []),
                "files": {name: hashlib.sha256((destination / name).read_bytes()).hexdigest() for name in own}}
    manifest["sha256"] = digest(manifest)
    cli.save(destination / "artifact.json", manifest)
    verify_bundle(destination)
    append(profile, "artifact-exported", artifact=identifier, bundle_sha256=manifest["sha256"],
           source=manifest["source_identity"], destination=str(destination))
    return {"artifact": identifier, "path": str(destination), "sha256": manifest["sha256"],
            "cmake_target": target, "limits": "Declared dependency profiles still need compiler/link and applicable target audits."}


def artifact_link(profile, workspace, bundle, profile_name, target):
    workspace = Path(workspace).expanduser().resolve()
    bundle = Path(bundle).expanduser().resolve()
    manifest = verify_bundle(bundle)
    identifier = manifest["id"]
    if not re.fullmatch(r"[A-Za-z0-9_][A-Za-z0-9_.-]*", identifier):
        raise ValueError("Invalid artifact identity")
    specification = catalog()["artifacts"].get(identifier)
    if specification is None or str(manifest["contract_version"]) != str(specification["contract_version"]):
        raise ValueError("Artifact does not match the current catalog contract")
    if profile_name not in manifest["profiles"] or target not in manifest["targets"]:
        raise ValueError("Artifact does not support requested dependency profile/target")
    meta = cli.read(workspace / ".cmastery/project.json")
    if meta.get("unit"):
        consumer = unit(meta["unit"])
        if identifier not in consumer.get("imports", []):
            raise ValueError("Current unit does not declare this artifact import")
    destination = workspace / "components" / (identifier + "-" + manifest["sha256"][:16])
    if destination.exists():
        if verify_bundle(destination)["sha256"] != manifest["sha256"]:
            raise ValueError("Existing component location has different content")
    else:
        shutil.copytree(bundle, destination)
    references = linked(workspace)
    previous = references.get(identifier)
    if any(key != identifier and link["cmake_target"] == manifest["cmake_target"] for key, link in references.items()):
        raise ValueError("Two component identities cannot declare the same CMake target")
    references[identifier] = {"path": destination.relative_to(workspace).as_posix(), "sha256": manifest["sha256"],
                              "profile": profile_name, "target": target, "cmake_target": manifest["cmake_target"]}
    cli.save(workspace / ".cmastery/artifacts.json", {"schema_version": 4, "artifacts": references})
    # Consumer owns its adapter. This file imports only learner component targets.
    lines = ["# Generated pinned learner imports; include from your CMakeLists.txt."]
    for link in references.values():
        lines += ["if(NOT TARGET " + link["cmake_target"] + ")",
                  '  add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../' + link["path"] +
                  '" "${CMAKE_CURRENT_BINARY_DIR}/components/' + link["cmake_target"] + '")', "endif()"]
    (workspace / ".cmastery/components.cmake").write_text("\n".join(lines) + "\n", encoding="utf-8")
    append(profile, "artifact-linked", artifact=identifier, workspace=str(workspace), bundle_sha256=manifest["sha256"],
           replaces=previous["sha256"] if previous else None, dependency_profile=profile_name, target=target)
    return {"artifact": identifier, "sha256": manifest["sha256"], "path": str(destination),
            "adapter": "include(.cmastery/components.cmake) then target_link_libraries(YOUR_TARGET PRIVATE " + manifest["cmake_target"] + ")",
            "recall_contract": identifier + "@" + str(manifest["contract_version"])}


def register(sub):
    study = sub.add_parser("study", help="Offline learning sessions and printable packets")
    study_sub = study.add_subparsers(dest="action", required=True)
    for action in ("init", "next", "status", "sheet"):
        parser = study_sub.add_parser(action)
        parser.add_argument("profile")
        if action == "next":
            selection = parser.add_mutually_exclusive_group()
            selection.add_argument("--studied", help="Record completed study and activate linked recall")
            selection.add_argument("--studied-exercise", help="Record exercise study separately from accepted units")
        if action == "sheet":
            parser.add_argument("output")
            selection = parser.add_mutually_exclusive_group()
            selection.add_argument("--unit")
            selection.add_argument("--exercise")
            parser.add_argument("--assessment-template", help="Write an unaccepted JSON assessment template for the selected unit")
    recall = sub.add_parser("recall", help="Original attempts, objective-specific feedback, and spaced retries")
    recall_sub = recall.add_subparsers(dest="action", required=True)
    for action in ("due", "start", "submit", "grade"):
        parser = recall_sub.add_parser(action)
        parser.add_argument("profile")
        if action == "start":
            parser.add_argument("set")
            parser.add_argument("--contract", help="Consumed artifact contract ID@VERSION for a scoped preflight")
        elif action in ("submit", "grade"):
            parser.add_argument("attempt")
            parser.add_argument("file", help="Original answer text or objective-score JSON")
            if action == "submit":
                parser.add_argument("--assistance", action="append", default=[])
    decision = sub.add_parser("decision", help="Seal an architecture choice before implementation checks")
    decision_sub = decision.add_subparsers(dest="action", required=True)
    parser = decision_sub.add_parser("seal")
    parser.add_argument("profile"); parser.add_argument("workspace"); parser.add_argument("unit"); parser.add_argument("file")
    artifact = sub.add_parser("artifact", help="Export and pin relocatable learner components")
    artifact_sub = artifact.add_subparsers(dest="action", required=True)
    parser = artifact_sub.add_parser("export")
    parser.add_argument("profile"); parser.add_argument("workspace"); parser.add_argument("artifact"); parser.add_argument("destination")
    parser.add_argument("--manifest", required=True)
    parser = artifact_sub.add_parser("link")
    parser.add_argument("profile"); parser.add_argument("workspace"); parser.add_argument("bundle")
    parser.add_argument("--profile", dest="dependency_profile", required=True); parser.add_argument("--target", required=True)
    parser = sub.add_parser("accept", help="Record scoped learner acceptance without conflating evidence types")
    parser.add_argument("profile"); parser.add_argument("workspace")
    selection = parser.add_mutually_exclusive_group(required=True)
    selection.add_argument("--unit")
    selection.add_argument("--exercise")
    parser.add_argument("--assessment", required=True)


def dispatch(args):
    if args.command == "study":
        if args.action == "init": return study_init(args.profile)
        if args.action == "next": return study_next(args.profile, args.studied, args.studied_exercise)
        if args.action == "status": return study_status(args.profile)
        if args.exercise:
            from . import exercises
            return exercises.sheet(args.profile, args.output, args.exercise, args.assessment_template)
        return study_sheet(args.profile, args.output, args.unit, args.assessment_template)
    if args.command == "recall":
        if args.action == "due": return {"due": due(args.profile)}
        if args.action == "start": return recall_start(args.profile, args.set, args.contract)
        if args.action == "submit": return recall_submit(args.profile, args.attempt, args.file, args.assistance)
        return recall_grade(args.profile, args.attempt, args.file)
    if args.command == "decision": return decision_seal(args.profile, args.workspace, args.unit, args.file)
    if args.command == "artifact":
        if args.action == "export": return artifact_export(args.profile, args.workspace, args.artifact, args.destination, args.manifest)
        return artifact_link(args.profile, args.workspace, args.bundle, args.dependency_profile, args.target)
    if args.exercise:
        from . import exercises
        return exercises.acceptance(args.profile, args.workspace, args.exercise, args.assessment)
    return acceptance(args.profile, args.workspace, args.unit, args.assessment)
