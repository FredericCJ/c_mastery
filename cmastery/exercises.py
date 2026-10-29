"""Additive exercise contracts and evidence, separate from accepted unit history.

Local receipts and hash chains detect accidental changes; they do not establish
examiner independence or resist an owner rewriting the complete local record.
"""
from __future__ import annotations

import hashlib
import json
from datetime import timedelta
from pathlib import Path, PurePosixPath
import re

from . import cli, learning

HEADINGS = ("Prerequisites", "Environment", "Problem", "Necessity", "Reasoning",
            "Deliverables", "Verification", "Rubric", "Hints", "Variants", "Security", "Validation")
FIELDS = ("schema_version", "id", "title", "project", "domain", "objectives", "families",
          "prerequisite_exercises", "prerequisite_artifacts", "profile", "environment",
          "source_records", "lab", "assessor_analysis", "status", "objective_coverage",
          "rubric", "required", "fixtures", "shared_contracts")
META = re.compile(r"\A```json\s*\n(.*?)\n```\s*\n", re.S)


def path_in_root(name, *, assessor=False):
    """Reject lexical and resolved escapes, including symlinked parents."""
    if not isinstance(name, str) or not name or "\\" in name or ":" in name:
        raise ValueError("Invalid exercise path: " + str(name))
    relative = PurePosixPath(name)
    if relative.is_absolute() or ".." in relative.parts or relative.as_posix() != name:
        raise ValueError("Exercise path escapes repository: " + name)
    if not assessor and "assessor" in {x.lower() for x in relative.parts}:
        raise ValueError("Assessor content cannot be a learner asset: " + name)
    root = cli.ROOT.resolve()
    result = root / name
    if not result.resolve().is_relative_to(root) or any(p.is_symlink() for p in [result, *result.parents] if p != root and p.is_relative_to(root)):
        raise ValueError("Exercise asset escapes repository or is a symlink: " + name)
    if not result.exists():
        raise ValueError("Missing exercise asset: " + name)
    return result


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def parse_brief(path):
    text = Path(path).read_text(encoding="utf-8-sig")
    match = META.match(text)
    if not match:
        raise ValueError(str(path) + ": missing leading fenced JSON contract")
    return json.loads(match[1]), text[match.end():]


def required_text(value, label):
    if not isinstance(value, str) or not value.strip():
        raise ValueError("Exercise needs nonempty " + label)


def delay_policy(value):
    """Honor both authored names; conflicting contracts must never bypass delay.

    delayed_transfer is the original workflow field. Modern authoring also used
    delayed_assessment for the same contract. Preserve canonical metadata and
    historical events, while applying the same gate to either spelling.
    """
    policies = []
    for name in ("delayed_transfer", "delayed_assessment"):
        if name not in value:
            continue
        policy = value[name]
        if (not isinstance(policy, dict)
                or type(policy.get("minimum_days")) is not int
                or policy["minimum_days"] < 1
                or not isinstance(policy.get("variant_ids"), list)
                or not policy["variant_ids"]
                or any(not isinstance(identifier, str) or not identifier.strip()
                       for identifier in policy["variant_ids"])
                or len(set(policy["variant_ids"])) != len(policy["variant_ids"])):
            raise ValueError("Delayed transfer needs a positive minimum_days and distinct named variants")
        policies.append(policy)
    if len(policies) == 2 and (policies[0]["minimum_days"] != policies[1]["minimum_days"]
                               or set(policies[0]["variant_ids"]) != set(policies[1]["variant_ids"])):
        raise ValueError("Conflicting delayed_transfer and delayed_assessment contracts")
    return policies[0] if policies else None


def validate(value, body, *, projects=None, objectives=None, artifacts=None, units=None):
    """Validate specified obligations, never infer that they were performed."""
    missing = set(FIELDS) - set(value)
    if missing:
        raise ValueError("Missing exercise fields: " + ", ".join(sorted(missing)))
    identifier = value["id"]
    if not re.fullmatch(r"X-T\d{2}-\d{3}", identifier) or value["schema_version"] != 1:
        raise ValueError("Invalid exercise identity or schema")
    for key in ("title", "project", "domain", "profile"):
        required_text(value[key], key)
    if value["domain"] != identifier.split("-")[1]:
        raise ValueError("Exercise identifier must preserve its registered domain namespace")
    if type(value["required"]) is not bool:
        raise ValueError("required must be a boolean")
    if projects is not None and value["project"] not in projects:
        raise ValueError("Unknown exercise project: " + value["project"])
    if not value["families"] or not set(value["families"]) <= set("ABCDEFGHIJ"):
        raise ValueError("Exercise families must select justified A-J families")
    ids = value["objectives"]
    if not isinstance(ids, list) or not ids or len(ids) != len(set(ids)):
        raise ValueError("Exercise needs distinct objective identifiers")
    if objectives is not None and not set(ids) <= set(objectives):
        raise ValueError("Unknown objective in " + identifier)
    if len(value["families"]) != len(set(value["families"])):
        raise ValueError("Exercise family labels must be distinct")
    for key in ("prerequisite_exercises", "prerequisite_artifacts", "fixtures", "shared_contracts"):
        if not isinstance(value[key], list):
            raise ValueError(key + " must be a list")
    if artifacts is not None and not set(value["prerequisite_artifacts"]) <= set(artifacts):
        raise ValueError("Unknown prerequisite artifact in " + identifier)
    if not isinstance(value.get("prerequisite_units", []), list) or (units is not None and not set(value.get("prerequisite_units", [])) <= set(units)):
        raise ValueError("Unknown prerequisite baseline unit in " + identifier)
    headings = [s.strip() for s in re.findall(r"^##\s+(.+)$", body, re.M)]
    for required in HEADINGS:
        if not any(h == required or h.startswith(required + " ") for h in headings):
            raise ValueError(identifier + " missing substantive section: " + required)
    for heading in headings:
        section = re.search(r"^## " + re.escape(heading) + r"\s*\n(.*?)(?=^## |\Z)", body, re.M | re.S)
        if not section or len(section[1].strip()) < 20:
            raise ValueError(identifier + " empty section: " + heading)
    env = value["environment"]
    for key in ("language", "target", "toolchain", "dependencies"):
        if not env.get(key):
            raise ValueError("Environment needs " + key)
    if not value["source_records"]:
        raise ValueError("Exercise needs inspected passage references")
    for record in value["source_records"]:
        required_text(record.get("id"), "source ID")
        register = cli.read(path_in_root(record["path"]))
        rows = register.get("resources", register.get("sources", []))
        sources = [row for row in rows if row.get("id") == record["id"]]
        if len(sources) != 1 or not isinstance(sources[0].get("inspection"), dict):
            raise ValueError("Source ID needs a unique inspected record: " + record["id"])
        inspection = sources[0]["inspection"]
        if (sources[0].get("assignable") is False or inspection.get("status") in
                {"unavailable", "not-read", "not-inspected", "not-run", "planned", "discovered-only", "reference-gap"}):
            raise ValueError("Unavailable or uninspected material must remain a reference gap: " + record["id"])
        for field in ("date", "method", "scope", "limits"):
            required_text(inspection.get(field), "source inspection " + field)
        if not isinstance(record.get("passages"), list) or not record["passages"] or any(not isinstance(x, str) or not x.strip() for x in record["passages"]):
            raise ValueError("Source reference needs specific inspected passages")
        if not set(record["passages"]) <= set(sources[0].get("locators", [])):
            raise ValueError("Exercise passage is outside registered inspected source locators: " + record["id"])
    lab = value["lab"]
    if not re.fullmatch(r"[a-z][a-z0-9_]*", lab.get("family", "")):
        raise ValueError("Invalid laboratory family")
    if lab["scaffold"] != "labs/" + lab["family"]:
        raise ValueError("Scaffold must be the declared learner family")
    folder = path_in_root(lab["scaffold"])
    family = cli.read(folder / "family.json")
    declared = next((row for row in family.get("exercises", []) if row["id"] == identifier), None)
    if declared is None or not lab.get("tests") or lab["tests"] != declared.get("tests"):
        raise ValueError("Exercise tests differ from family adapter: " + identifier)
    if declared.get("project", value["project"]) != value["project"]:
        raise ValueError("Exercise family project differs")
    for relative in cli.snapshot(folder)["files"]:
        if "assessor" in {x.lower() for x in PurePosixPath(relative).parts}:
            raise ValueError("Assessor file in learner scaffold")
        path_in_root(lab["scaffold"] + "/" + relative)
    analysis = path_in_root(value["assessor_analysis"], assessor=True)
    if not analysis.is_relative_to(cli.ROOT / "assessor"):
        raise ValueError("Detailed exercise solutions belong under assessor/")
    for name in value["fixtures"] + value["shared_contracts"]:
        path_in_root(name)
    rubric = value["rubric"]
    criteria = {row["id"]: row for row in rubric}
    if not criteria or len(criteria) != len(rubric):
        raise ValueError("Distinct rubric criteria are required")
    for row in rubric:
        if row.get("objective") not in ids:
            raise ValueError("Rubric criterion needs an explicit exercise objective")
        required_text(row.get("criterion"), "rubric criterion")
        if set(row.get("levels", {})) != {"0", "1", "2", "3"}:
            raise ValueError("Every criterion needs 0-3 assessment anchors")
        for level in row["levels"].values():
            required_text(level, "rubric level")
    rows = value["objective_coverage"]
    if len(rows) != len(ids) or {row["objective"] for row in rows} != set(ids):
        raise ValueError("Every leaf objective needs one coverage record")
    anchors = {"#" + re.sub(r"[^\w\- ]", "", heading.lower()).replace(" ", "-") for heading in headings}
    deliverable_section = re.search(r"^## Deliverables\s*\n(.*?)(?=^## |\Z)", body, re.M | re.S)
    deliverables = set(re.findall(r"\bD\d+\b", deliverable_section[1] if deliverable_section else ""))
    for row in rows:
        if not row.get("teaching") or not row.get("implementation"):
            raise ValueError("Teaching and implementation/analysis facets must be explicit")
        if not set(row["teaching"]) <= anchors or not set(row["implementation"]) <= deliverables:
            raise ValueError("Coverage points to missing teaching anchors or deliverables")
        boundary = row.get("boundary", {})
        if type(boundary.get("applicable")) is not bool or (boundary["applicable"] and not boundary.get("evidence")) or (not boundary["applicable"] and not boundary.get("reason")):
            raise ValueError("Boundary coverage requires evidence or justified non-applicability")
        if boundary["applicable"] and not set(boundary["evidence"]) <= deliverables:
            raise ValueError("Boundary facet points to missing deliverables")
        independent = row.get("independent_assessment", [])
        if not independent or any(key not in criteria or criteria[key]["objective"] != row["objective"] for key in independent):
            raise ValueError("Independent assessment must identify objective-specific rubric criteria")
    status = value["status"]
    for key in ("content", "execution", "independent_review", "limitations"):
        if key not in status or not status[key]:
            raise ValueError("Status needs explicit evidence limitations and " + key)
    delay = delay_policy(value)
    if delay is not None:
        section = re.search(r"^## Variants\s*\n(.*?)(?=^## |\Z)", body, re.M | re.S)
        if section is None or any(key not in section[1] for key in delay["variant_ids"]):
            raise ValueError("Delayed variants must resolve to distinct labels in Variants")


def ordered(values):
    by_id = {value["id"]: value for value in values}
    if len(by_id) != len(values):
        raise ValueError("Duplicate exercise identifier")
    done, visiting, order = set(), set(), []

    def visit(identifier):
        if identifier not in by_id:
            raise ValueError("Unknown prerequisite exercise: " + identifier)
        if identifier in visiting:
            raise ValueError("Exercise prerequisite cycle: " + identifier)
        if identifier in done:
            return
        visiting.add(identifier)
        for prior in by_id[identifier]["prerequisite_exercises"]:
            visit(prior)
        visiting.remove(identifier)
        done.add(identifier)
        order.append(identifier)

    for identifier in sorted(by_id):
        visit(identifier)
    return order


def build_catalog():
    base = learning.catalog_dir()
    existing = learning.catalog()
    projects = {x["id"] for x in cli.catalog()}
    objectives = {o for u in existing["units"].values() for o in u["objectives"]}
    # A newly registered leaf can be assessed by the additive exercise layer
    # without rewriting an accepted unit solely to make its identifier visible.
    for name in ("taxonomy.json", "enhancement_taxonomy.json", "performance_debugging_taxonomy.json",
                 "v2_taxonomy.json", "v3_taxonomy.json", "v5_taxonomy.json"):
        taxonomy = cli.ROOT / "curriculum" / name
        if taxonomy.is_file():
            objectives.update(o["id"] for d in cli.read(taxonomy)["domains"] for t in d["subtopics"] for o in t["objectives"])
    values = []
    for path in sorted((base / "exercises").glob("X-*.md")):
        value, body = parse_brief(path)
        if path.stem != value["id"]:
            raise ValueError("Exercise filename must match stable identifier")
        validate(value, body, projects=projects, objectives=objectives, artifacts=existing["artifacts"], units=existing["units"])
        bound = value["shared_contracts"] + value["fixtures"] + [r["path"] for r in value["source_records"]]
        values.append(dict(value, brief=path.relative_to(cli.ROOT).as_posix(), brief_sha256=sha(path),
                           bound_files={name: sha(path_in_root(name)) for name in sorted(set(bound))}))
    return {"schema_version": 1, "exercises": values, "order": ordered(values),
            "accounting": "Additive, uncalibrated effort; accepted unit hours and histories remain unchanged.",
            "limits": "Specified coverage is not executed evidence, independent review, or proof of mastery."}


def catalog():
    path = learning.catalog_dir() / "exercises.json"
    if not path.is_file():
        return {"exercises": {}, "order": []}
    value = cli.read(path)
    return {"exercises": {x["id"]: x for x in value["exercises"]}, "order": value["order"]}


def exercise(identifier, project=None, family=None):
    value = catalog()["exercises"].get(identifier)
    if value is None:
        raise ValueError("Unknown exercise: " + str(identifier))
    if project is not None and value["project"] != project:
        raise ValueError("Exercise belongs to a different project")
    if family is not None and value["lab"]["family"] != family:
        raise ValueError("Exercise requires family: " + value["lab"]["family"])
    if sha(path_in_root(value["brief"])) != value["brief_sha256"] or any(sha(path_in_root(name)) != identity for name, identity in value["bound_files"].items()):
        raise ValueError("Exercise catalog is stale; regenerate from canonical sources")
    return value


def harness(value, workspace=None):
    """Public tests/build rules are trusted evidence inputs, not learner answers."""
    folder = path_in_root(value["lab"]["scaffold"])
    records = {name: identity for name, identity in cli.snapshot(folder)["files"].items()
               if not name.startswith("src/")}
    if workspace is not None:
        for name, identity in records.items():
            path = learning.local_file(workspace, name)
            if sha(path) != identity:
                raise ValueError("Public exercise harness changed: " + name)
    return learning.digest(records)


def workspace_contract(workspace, identifier=None):
    workspace = Path(workspace).resolve()
    meta = cli.read(workspace / ".cmastery/project.json")
    identifier = identifier or meta.get("exercise")
    if meta.get("unit") or meta.get("exercise") != identifier:
        raise ValueError("Exercise check requires its explicitly initialized exercise workspace")
    value = exercise(identifier, meta["project"], meta["family"])
    if meta.get("exercise_contract") != learning.digest(value):
        raise ValueError("Initialized exercise contract changed; initialize a fresh exercise workspace")
    harness(value, workspace)
    for name in value["fixtures"] + value["shared_contracts"]:
        if sha(learning.local_file(workspace, name)) != value["bound_files"][name]:
            raise ValueError("Copied exercise fixture or shared contract changed: " + name)
    return value


def studied(profile, identifier):
    value = exercise(identifier)
    return learning.append(profile, "exercise-studied", exercise=identifier,
                           exercise_contract=learning.digest(value), objectives=value["objectives"])


def assessment_template(value):
    return {"exercise": value["id"], "exercise_contract": learning.digest(value),
            "explanation": "", "transfer": "", "transfer_variant": "", "evidence": [],
            "independence": {"unaided": False, "assistance": []},
            "dimensions": {key: 0 for key in learning.DIMENSIONS},
            "criteria": [{"id": row["id"], "objective": row["objective"], "score": 0,
                          "reason": "", "evidence": []} for row in value["rubric"]],
            "limits": "Self-assessment records claims; a CLI cannot certify independent reasoning."}


def learner_brief(value, *, inline_shared=False):
    """Render the substantive contract and objective-specific rubric without answers."""
    _, body = parse_brief(path_in_root(value["brief"]))
    for name in value["shared_contracts"]:
        body = body.replace("](" + Path(name).name + ")", "](" + name + ")")
    lines = [body.rstrip(), "", "## Objective-specific assessment anchors", ""]
    for row in value["rubric"]:
        lines += ["### " + row["id"] + " / " + row["objective"], "", row["criterion"], ""]
        lines += ["- " + level + ": " + row["levels"][level] for level in ("0", "1", "2", "3")]
        lines.append("")
    lines += ["## Traceability and contract selection", "", "Project: " + value["project"],
              "", "Required earlier exercises: " + (", ".join(value["prerequisite_exercises"]) or "none"),
              "", "Required baseline units: " + (", ".join(value.get("prerequisite_units", [])) or "none"),
              "", "Required artifact imports: " + (", ".join(value["prerequisite_artifacts"]) or "none"), ""]
    for coverage in value["objective_coverage"]:
        if coverage.get("assessed_claim"):
            lines += ["- " + coverage["objective"] + ": " + coverage["assessed_claim"] +
                      (" Scope: " + coverage["scope"] if coverage.get("scope") else "")]
    lines.append("")
    for row in value["source_records"]:
        register = cli.read(path_in_root(row["path"]))
        source = next(r for r in register.get("resources", register.get("sources", [])) if r["id"] == row["id"])
        label = "[" + row["id"] + "](" + source["url"] + ")" if source.get("url") else row["id"]
        lines += ["- " + label + ": " + "; ".join(row["passages"])]
    delay = delay_policy(value)
    if delay:
        lines += ["", "Delayed transfer: wait at least " + str(delay["minimum_days"]) +
                  " days after the first recorded study of this contract; select " + ", ".join(delay["variant_ids"]) + "."]
    lines += ["", "Contract SHA-256: " + learning.digest(value), ""]
    if inline_shared:
        for name in value["shared_contracts"]:
            lines += ["", "---", "", path_in_root(name).read_text(encoding="utf-8-sig"), ""]
    return "\n".join(lines)


def sheet(profile, output, identifier, template=None):
    learning.settings(profile)
    value = exercise(identifier)
    text = "# Exercise study sheet\n\n" + learner_brief(value, inline_shared=True) + "\n\n## Attempt record\n\n"
    text += "Prediction, invariant, evidence paths, assistance, and changed-condition reasoning: __________\n\n"
    text += "Record study with `study next PROFILE --studied-exercise " + identifier + "`. Tests and independent assessment remain separate.\n"
    Path(output).write_text(text, encoding="utf-8")
    if template:
        cli.save(template, assessment_template(value))
    return {"sheet": str(Path(output).resolve()), "exercise": identifier, "contains_answers": False}


def current_receipts(workspace, value):
    workspace = Path(workspace).resolve()
    source = cli.snapshot(workspace)
    linked = {k: v["sha256"] for k, v in learning.linked_state(workspace).items()}
    result = []
    for path in sorted((workspace / "evidence/checks").glob("*.json")):
        receipt = cli.read(path)
        observed = {t["name"] for t in receipt.get("tests", []) if not t.get("failed") and not t.get("skipped")}
        steps = receipt.get("steps", [])
        if (receipt.get("assessment") != "learner" or receipt.get("status") != "passed"
            or receipt.get("exercise") != value["id"] or receipt.get("exercise_contract") != learning.digest(value)
            or receipt.get("source") != source or receipt.get("exercise_harness") != harness(value, workspace)
            or receipt.get("component_links", {}) != linked or not set(value["lab"]["tests"]) <= observed
            or len(steps) != 3 or any(step.get("exit_code") != 0 for step in steps)
            or not receipt.get("environment") or receipt.get("project") != value["project"]):
            continue
        if any(not Path(p).is_dir() or cli.snapshot(p)["sha256"] != h for p, h in receipt.get("external_sources", {}).items()):
            continue
        result.append((path, receipt))
    return result


def assess(value, assessment, workspace=None, retained=None):
    def evidence(names):
        if retained is None:
            return learning.evidence_files(workspace, names)
        if not isinstance(names, list) or not names:
            raise ValueError("Each criterion needs retained local evidence")
        for name in names:
            learning.safe_relative(name)
            if name not in retained:
                raise ValueError("Criterion evidence is absent from accepted revision")
        return {name: retained[name] for name in names}

    if assessment.get("exercise") != value["id"] or assessment.get("exercise_contract") != learning.digest(value):
        raise ValueError("Assessment exercise contract differs")
    for key in ("explanation", "transfer", "transfer_variant"):
        required_text(assessment.get(key), key)
    delay = delay_policy(value)
    if delay and assessment["transfer_variant"] not in delay["variant_ids"]:
        raise ValueError("Select a declared delayed transfer variant")
    independence = assessment.get("independence", {})
    if independence.get("unaided") is not True or independence.get("assistance") != []:
        raise ValueError("Acceptance requires a disclosed independent transfer attempt")
    dimensions = assessment.get("dimensions", {})
    if set(dimensions) != set(learning.DIMENSIONS) or any(type(n) is not int or n not in (2, 3) for n in dimensions.values()) or list(dimensions.values()).count(3) < 2:
        raise ValueError("Exercise rubric requires six dimensions >=2 and at least two scores of 3")
    criteria = assessment.get("criteria", [])
    if len(criteria) != len(value["rubric"]) or {x.get("id") for x in criteria} != {x["id"] for x in value["rubric"]}:
        raise ValueError("Assess every objective-specific exercise criterion exactly once")
    files = evidence(assessment.get("evidence", []))
    required = {x["id"]: x for x in value["rubric"]}
    for row in criteria:
        if row.get("objective") != required[row["id"]]["objective"] or type(row.get("score")) is not int or row["score"] not in (2, 3):
            raise ValueError("Criterion objective differs or performance is below independent understanding")
        required_text(row.get("reason"), "criterion reasoning")
        files.update(evidence(row.get("evidence", [])))
    return files


def accepted(profile):
    history = learning.events(profile)
    latest = {row["exercise"]: row for row in history if row["kind"] == "exercise-accepted"}
    result = {}
    baseline = None

    def inspect(identifier, visiting):
        nonlocal baseline
        if identifier in result:
            return result[identifier]
        entry = latest[identifier]
        valid, reason = True, None
        try:
            if identifier in visiting:
                raise ValueError("Cyclic exercise acceptance")
            value = exercise(identifier)
            if entry.get("exercise_contract") != learning.digest(value):
                raise ValueError("Exercise contract changed")
            revision = entry.get("revision", {})
            if not revision.get("source") or not revision.get("evidence"):
                raise ValueError("Retained exercise evidence missing")
            assess(value, entry.get("assessment", {}), retained=revision["evidence"])
            study = next((row for row in history if row["id"] == entry.get("study_event")), None)
            if not study or study["kind"] != "exercise-studied" or study.get("exercise") != identifier or study.get("exercise_contract") != entry["exercise_contract"]:
                raise ValueError("Original current-contract study binding missing")
            minimum = (delay_policy(value) or {}).get("minimum_days", 0)
            if learning.instant(entry["at"]) - learning.instant(study["at"]) < timedelta(days=minimum):
                raise ValueError("Delayed transfer acceptance predates the required interval")
            if set(entry.get("prerequisites", {})) != set(value["prerequisite_exercises"]):
                raise ValueError("Prerequisite acceptance binding missing")
            for prior, receipt_id in entry.get("prerequisites", {}).items():
                if prior not in latest or latest[prior]["id"] != receipt_id or not inspect(prior, visiting | {identifier})["historical_valid"]:
                    raise ValueError("Prerequisite accepted revision missing or superseded")
            unit_refs = entry.get("unit_prerequisites", {})
            if set(unit_refs) != set(value.get("prerequisite_units", [])):
                raise ValueError("Baseline prerequisite acceptance binding missing")
            if unit_refs and baseline is None:
                baseline = learning.accepted(profile)
            for key, receipt_id in unit_refs.items():
                if not baseline.get(key, {}).get("historical_valid") or baseline[key]["record"]["id"] != receipt_id:
                    raise ValueError("Baseline prerequisite accepted revision missing or superseded")
            for identity in set(revision["source"].values()) | set(revision["evidence"].values()):
                if not re.fullmatch(r"[0-9a-f]{64}", identity) or sha(learning.area(profile) / "objects" / identity) != identity:
                    raise ValueError("Retained exercise object changed")
            receipt = entry.get("executable_receipt", {})
            observed = {t["name"] for t in receipt.get("tests", []) if not t.get("failed") and not t.get("skipped")}
            steps = receipt.get("steps", [])
            if (receipt.get("exercise") != identifier or receipt.get("project") != value["project"]
                or receipt.get("exercise_contract") != learning.digest(value) or receipt.get("assessment") != "learner"
                or receipt.get("status") != "passed" or receipt.get("source", {}).get("files") != revision["source"]
                or receipt.get("exercise_harness") != harness(value) or not receipt.get("environment")
                or not set(value["lab"]["tests"]) <= observed or len(steps) != 3 or any(s.get("exit_code") != 0 for s in steps)):
                raise ValueError("Retained learner execution binding missing")
            if set(entry.get("imports", {})) != set(value["prerequisite_artifacts"]) or any(receipt.get("component_links", {}).get(key) != identity for key, identity in entry.get("imports", {}).items()):
                raise ValueError("Retained artifact import binding missing")
            receipt_hash = revision["evidence"].get(receipt.get("receipt_path"))
            if not receipt_hash or cli.read(learning.area(profile) / "objects" / receipt_hash) != receipt:
                raise ValueError("Accepted executable receipt differs from retained evidence")
        except (OSError, ValueError, KeyError) as error:
            valid, reason = False, str(error)
        current = valid
        if valid:
            try:
                current = cli.snapshot(entry["workspace"])["files"] == entry["revision"]["source"] and learning.verify_files(entry["workspace"], entry["revision"]["evidence"])
                selected = entry.get("active_specification")
                if selected:
                    from . import route
                    current = current and selected["sha256"] == route.specification_identity(selected["id"])
                links = learning.linked_state(entry["workspace"])
                current = current and all(links.get(key, {}).get("sha256") == identity for key, identity in entry.get("imports", {}).items())
            except (OSError, ValueError):
                current = False
        result[identifier] = {"historical_valid": valid, "valid": current, "reason": reason if not valid else (None if current else "Live source or evidence changed"), "record": entry}
        return result[identifier]

    for identifier in latest:
        inspect(identifier, set())
    return result


def acceptance(profile, workspace, identifier, assessment_file):
    workspace = Path(workspace).expanduser().resolve()
    value = workspace_contract(workspace, identifier)
    contract = learning.digest(value)
    history = learning.events(profile)
    studies = [row for row in history if row["kind"] == "exercise-studied" and row["exercise"] == identifier and row["exercise_contract"] == contract]
    if not studies:
        raise ValueError("Record the current exercise as studied before acceptance")
    study = studies[0]
    delay = (delay_policy(value) or {}).get("minimum_days", 0)
    earliest = learning.instant(study["at"]) + timedelta(days=delay)
    if learning.clock() < earliest:
        raise ValueError("Delayed independent transfer pending until " + learning.stamp(earliest) + "; practice and checks remain available")
    previous = accepted(profile)
    missing = [key for key in value["prerequisite_exercises"] if not previous.get(key, {}).get("historical_valid")]
    if missing:
        raise ValueError("Missing accepted exercise prerequisites: " + ", ".join(missing))
    from . import route
    route.require_ready(profile, identifier, workspace)
    baseline = learning.accepted(profile) if value.get("prerequisite_units") else {}
    missing = [key for key in value.get("prerequisite_units", []) if not baseline.get(key, {}).get("historical_valid")]
    if missing:
        raise ValueError("Missing accepted baseline unit prerequisites: " + ", ".join(missing))
    imports = learning.linked_state(workspace)
    if not set(value["prerequisite_artifacts"]) <= set(imports):
        raise ValueError("Missing required learner artifact imports")
    for key in value["prerequisite_artifacts"]:
        artifact = learning.catalog()["artifacts"][key]
        if str(imports[key]["contract_version"]) != str(artifact["contract_version"]):
            raise ValueError("Imported artifact contract changed: " + key)
        missing = learning.freshness(profile, artifact.get("critical_objectives", []), key + "@" + str(artifact["contract_version"]))
        if missing:
            raise ValueError("Imported artifact needs independent recall: " + key)
    assessment = cli.read(assessment_file)
    files = assess(value, assessment, workspace)
    receipts = current_receipts(workspace, value)
    if not receipts:
        raise ValueError("Current successful learner check --exercise receipt is required")
    path, receipt = receipts[-1]
    files.update(learning.evidence_files(workspace, [path.relative_to(workspace).as_posix()]))
    revision = learning.retain_revision(profile, workspace, files)
    record = learning.append(profile, "exercise-accepted", exercise=identifier, exercise_contract=contract,
        workspace=str(workspace), revision=revision, assessment=assessment, executable_receipt=receipt,
        active_specification=cli.read(workspace/".cmastery/project.json").get("active_specification"),
        study_event=study["id"], independent_assessment="self-reported unaided transfer; reviewer independence not established",
        prerequisites={key: previous[key]["record"]["id"] for key in value["prerequisite_exercises"]},
        unit_prerequisites={key: baseline[key]["record"]["id"] for key in value.get("prerequisite_units", [])},
        imports={key: imports[key]["sha256"] for key in value["prerequisite_artifacts"]},
        supersedes=previous.get(identifier, {}).get("record", {}).get("id"),
        limits="Local self-assessment; execution, reasoning, claimed independence and physical evidence remain distinct. No certification or absolute mastery claim.")
    return {"exercise": identifier, "acceptance": record["id"], "status": "accepted", "limits": record["limits"]}


def progress(profile):
    data = catalog()
    assessed = accepted(profile)
    historical = {key for key, row in assessed.items() if row["historical_valid"]}
    required = {key for key, value in data["exercises"].items() if value["required"]}
    pending_batches = []
    edition = cli.ROOT / "curriculum/edition.json"
    manifest = cli.read(edition) if edition.is_file() else {}
    expansion = manifest.get("exercise_expansion", {})
    remaining = expansion.get("remaining_work")
    if remaining:
        try:
            work = cli.read(path_in_root(remaining))
            available_statuses = {"authored", "validated", "delivered", "complete"}
            pending_batches = [row["id"] for row in work["batches"] if row.get("required", True) and row.get("status") not in available_statuses]
        except (OSError, ValueError, KeyError):
            pending_batches = ["remaining-work-manifest-unavailable"]
    missing_authorship = max(0, expansion.get("required_exercises", len(required)) - len(required))
    available = [data["exercises"][key] for key in data["order"] if key not in historical and set(data["exercises"][key]["prerequisite_exercises"]) <= historical]
    return {"studied": sorted({row["exercise"] for row in learning.events(profile) if row["kind"] == "exercise-studied"}),
            "accepted": sorted(historical), "pending": sorted(required - historical),
            "current_acceptances": sorted(key for key, row in assessed.items() if row["valid"]),
            "reopened": {key: row["reason"] for key, row in assessed.items() if not row["valid"]},
            "current_exercises_complete": required <= historical,
            "complete": required <= historical and not pending_batches and not missing_authorship,
            "pending_batches": pending_batches, "missing_required_specifications": missing_authorship,
            "required_count": len(required),
            "available_exercises": [{key: value[key] for key in ("id", "title", "project")} for value in available[:8]],
            "independent_assessment": "Recorded acceptances are self-reported independent attempts; independent reviewer status is not inferred.",
            "limits": "Authored exercise completion and pending curriculum-wide batches are separate; neither proves absolute mastery."}
