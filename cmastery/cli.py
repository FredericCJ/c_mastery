"""Discover, instantiate and exercise explicitly scoped learner laboratories."""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
TOOLS = {
    "gcc": ["gcc", "--version"], "clang": ["clang", "--version"],
    "cmake": ["cmake", "--version"], "ninja": ["ninja", "--version"],
    "gdb": ["gdb", "--version"], "valgrind": ["valgrind", "--version"],
    "arm_compiler": ["arm-none-eabi-gcc", "--version"],
    "qemu_arm": ["qemu-system-arm", "--version"],
    "qemu_x86": ["qemu-system-x86_64", "--version"],
    "grub": ["grub-mkrescue", "--version"],
    "perf": ["perf", "--version"],
}

def read(path):
    return json.loads(Path(path).read_text(encoding="utf-8-sig"))

def save(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")

def now():
    return datetime.now(timezone.utc).isoformat()

def invoke(command, cwd=None, timeout=120):
    try:
        proc = subprocess.run(command, cwd=cwd, capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=timeout)
        return {"command": command, "exit_code": proc.returncode,
                "stdout": proc.stdout, "stderr": proc.stderr}
    except FileNotFoundError as error:
        return {"command": command, "exit_code": None, "unavailable": str(error)}
    except subprocess.TimeoutExpired as error:
        def decode(value):
            return value.decode("utf-8", "replace") if isinstance(value, bytes) else value or ""
        return {"command": command, "exit_code": None, "timed_out": True,
                "stdout": decode(error.stdout), "stderr": decode(error.stderr)}

def doctor():
    capabilities = {}
    for name, command in TOOLS.items():
        path = shutil.which(command[0])
        record = invoke(command, timeout=15) if path else {"exit_code": None}
        lines = (record.get("stdout", "") + record.get("stderr", "")).splitlines()
        capabilities[name] = {"available": record["exit_code"] == 0,
                              "path": path, "version": lines[0] if lines else None}
    cgroup = Path("/sys/fs/cgroup/cgroup.controllers")
    pmu = Path("/proc/sys/kernel/perf_event_paranoid")
    return {"schema_version": 2, "recorded_at": now(), "system": platform.platform(),
            "python": platform.python_version(), "machine": platform.machine(),
            "capabilities": capabilities,
            "cgroup_v2": cgroup.read_text().strip() if cgroup.is_file() else None,
            "perf_event_paranoid": pmu.read_text().strip() if pmu.is_file() else None,
            "physical_measurements": "not-run",
            "limits": "Tool discovery does not establish sanitizer, PMU, emulator or hardware execution.",
            "host_route": "Run C laboratories inside Linux/WSL; publishing Python is independent."}

def catalog():
    path = ROOT / "curriculum/project_graph.json"
    return read(path).get("projects", []) if path.exists() else []

def families():
    result = {}
    for path in sorted((ROOT / "labs").glob("*/family.json")):
        meta = read(path)
        result[path.parent.name] = {"path": path.parent, "meta": meta}
    return result

def locate(project):
    known = {item["id"]: item for item in catalog()}
    if project not in known:
        raise ValueError("Unknown project: " + project)
    item = known[project]
    family = item.get("lab_family")
    candidates = families()
    if family not in candidates:
        for key, value in candidates.items():
            entries = value["meta"].get("projects", {})
            ids = entries.keys() if isinstance(entries, dict) else [
                entry if isinstance(entry, str) else entry.get("id") for entry in entries]
            if project in ids:
                family = key
                break
    return item, candidates.get(family)

def snapshot(folder):
    records = {}
    for path in sorted(Path(folder).rglob("*")):
        relative = path.relative_to(folder)
        if not path.is_file() or set(relative.parts) & {"build", ".cmastery", "evidence", ".git", "__pycache__"}:
            continue
        records[relative.as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    identity = hashlib.sha256(json.dumps(records, sort_keys=True).encode()).hexdigest()
    return {"sha256": identity, "files": records}

def instantiate(project, destination, family_name=None):
    item, family = locate(project)
    if family_name:
        candidates=families()
        if family_name not in candidates: raise ValueError('Unknown laboratory family: '+family_name)
        candidate=candidates[family_name]
        entries=candidate['meta'].get('projects',[])
        ids=set(entries) if isinstance(entries,dict) else {p if isinstance(p,str) else p['id'] for p in entries}
        if project not in ids: raise ValueError('Family does not support project: '+family_name+' / '+project)
        family=candidate
    destination = Path(destination).expanduser().resolve()
    protected = [ROOT / name for name in ('curriculum', 'labs', 'assessor', 'tools', 'site', 'research', 'references', 'fixtures', 'cmastery')]
    if destination == ROOT or destination in ROOT.parents or any(destination.is_relative_to(p) for p in protected):
        raise ValueError("Choose a separate, empty learner directory.")
    if destination.exists() and any(destination.iterdir()):
        raise ValueError("Learner destination is not empty: " + str(destination))
    destination.mkdir(parents=True, exist_ok=True)
    if family:
        shutil.copytree(family["path"], destination, dirs_exist_ok=True,
                        ignore=shutil.ignore_patterns("build", "__pycache__", "*.pyc"))
    brief = ROOT / "curriculum/projects" / (project + ".md")
    shutil.copy2(brief, destination / "PROJECT.md")
    # Local contract links resolve in the exported workspace as well.
    guide = ROOT / 'curriculum/guides/component-contracts.md'
    shutil.copy2(guide, destination / 'CONTRACTS.md')
    text = (destination / 'PROJECT.md').read_text(encoding='utf-8')
    (destination / 'PROJECT.md').write_text(text.replace('../guides/component-contracts.md', 'CONTRACTS.md'), encoding='utf-8')
    for fixture in item.get("fixtures", []):
        source = (ROOT / fixture).resolve()
        if not source.is_relative_to(ROOT / "fixtures"):
            raise ValueError("Invalid fixture path")
        target = destination / fixture
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    record = {"schema_version": 2, "project": project,
              "family": family["path"].name if family else None,
              "created_at": now(), "source_catalog": item,
              "assessment": "learner", "evidence_status": "not-run"}
    save(destination / ".cmastery/project.json", record)
    save(destination / ".cmastery/prediction.json", {
        "project": project, "predictions": [], "invariant": "",
        "competing_causes": [], "assistance": [], "sealed_at": None})
    return {"project": project, "workspace": str(destination),
            "family": record["family"], "status": "not-run",
            "next": "Implement the assigned boundary, then run cmastery check.",
            "limits": "Instantiation does not establish implementation or mastery."}

def advance(workspace, project):
    """Select another project in the same family without replacing learner code."""
    workspace=Path(workspace).expanduser().resolve()
    meta=read(workspace/'.cmastery/project.json')
    item,_=locate(project)
    if project==meta['project']: raise ValueError('Project already selected: '+project)
    local_family=read(workspace/'family.json')
    entries=local_family.get('projects',[])
    ids=set(entries) if isinstance(entries,dict) else {p if isinstance(p,str) else p['id'] for p in entries}
    if project not in ids or meta['family'] not in item.get('lab_families',[item.get('lab_family')]):
        raise ValueError('Advance requires a project supported by the same exported laboratory family')
    # Validate incoming assets before archiving or changing selection.
    brief=ROOT/'curriculum/projects'/(project+'.md')
    text=brief.read_text(encoding='utf-8-sig').replace('../guides/component-contracts.md','CONTRACTS.md')
    fixtures=[]
    for fixture in item.get('fixtures',[]):
        source=(ROOT/fixture).resolve()
        if not source.is_relative_to(ROOT/'fixtures') or not source.is_file():
            raise ValueError('Invalid fixture path: '+fixture)
        fixtures.append((source,workspace/fixture))
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    history=workspace/'evidence/history'/(stamp+'-'+meta['project'])
    history.mkdir(parents=True,exist_ok=False)
    save(history/'source-snapshot.json',snapshot(workspace))
    for source,name in [(workspace/'.cmastery/project.json','project.json'),
                        (workspace/'.cmastery/prediction.json','prediction.json'),
                        (workspace/'PROJECT.md','PROJECT.md'),
                        (workspace/'evidence/latest.json','latest.json')]:
        if source.is_file(): shutil.copy2(source,history/name)
    for source,target in fixtures:
        if not target.exists():
            target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,target)
    (workspace/'PROJECT.md').write_text(text,encoding='utf-8')
    meta.update(project=project,source_catalog=item,selected_at=now(),evidence_status='not-run')
    save(workspace/'.cmastery/project.json',meta)
    save(workspace/'.cmastery/prediction.json',{'project':project,'predictions':[],
         'invariant':'','competing_causes':[],'assistance':[],'sealed_at':None})
    (workspace/'evidence/latest.json').unlink(missing_ok=True)
    return {'project':project,'workspace':str(workspace),'family':meta['family'],
            'status':'not-run','archived_evidence':str(history),
            'limits':'Selection preserves learner sources and prior evidence; it does not accept prerequisites or mastery.'}

def run_checks(workspace, compiler, profile, assessor=False, timeout=180, all_tests=False, dependencies=None):
    workspace = Path(workspace).resolve()
    meta = read(workspace / ".cmastery/project.json")
    before = snapshot(workspace)
    dependencies = dependencies or {}
    family_meta = read(workspace / 'family.json') if (workspace / 'family.json').is_file() else {}
    entries = family_meta.get('projects', [])
    if isinstance(entries, dict):
        project_contract = entries.get(meta['project'], {})
    else:
        project_contract = next((p for p in entries if isinstance(p, dict) and p['id'] == meta['project']), {})
    selected = project_contract.get('tests', [])
    env = doctor()
    record = {"schema_version": 2, "project": meta["project"], "family": meta["family"],
              "recorded_at": now(), "evidence_kind": "host-execution",
              "assessment": "assessor-validation" if assessor else "learner",
              "profile": profile, "compiler": compiler, "source": before,
              "environment": env, "status": "not-run", "steps": [],
              "mastery": "requires separate explanation, transfer and applicable physical evidence"}
    record['project_contract'] = project_contract
    record['required_tests'] = selected
    unavailable = [tool for tool in (compiler, "cmake", "ninja")
                   if not env["capabilities"][tool]["available"]]
    if unavailable or not (workspace / "CMakeLists.txt").is_file() or (not selected and not all_tests):
        record.update(status="unavailable", reason="Missing: " + ", ".join(unavailable or ["executable project adapter"]))
    else:
        # A fresh cache prevents removed dependency options from silently
        # retaining implementations that the new evidence record does not pin.
        run_id=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
        output = workspace / "build" / (compiler + "-" + profile + ("-assessor" if assessor else "-learner")) / run_id
        flags = "-Wall -Wextra -Wpedantic"
        flags += {"debug": " -O0 -g", "release": " -O2 -g",
                  "asan": " -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer"}[profile]
        configure = ["cmake", "-S", str(workspace), "-B", str(output), "-G", "Ninja",
                     "-DCMAKE_C_COMPILER=" + compiler, "-DCMAKE_C_STANDARD=17",
                     "-DCMAKE_C_FLAGS=" + flags]
        if assessor:
            reference = ROOT / "assessor" / meta["family"]
            if not reference.is_dir():
                raise ValueError("Assessor implementation unavailable for " + str(meta["family"]))
            configure.append("-DCMASTERY_ASSESSOR_ROOT=" + str(reference))
            record["assessor_source"] = snapshot(reference)
            record.setdefault('external_sources', {})[str(reference)] = record['assessor_source']['sha256']
            if meta['family'] in {'storage', 'kernel'}:
                dependencies = dict(dependencies)
                dependencies.setdefault('runtime', ROOT / 'labs/runtime')
                if meta['family'] == 'kernel': dependencies.setdefault('storage', ROOT / 'labs/storage')
        for family, folder in dependencies.items():
            if family not in {'runtime', 'storage'}: raise ValueError('Unsupported dependency: '+family)
            dependency = Path(folder).expanduser().resolve()
            if not (dependency / 'family.json').is_file(): raise ValueError('Missing dependency family: '+str(dependency))
            implementation = ROOT / 'assessor' / family if assessor else dependency / 'src'
            root_argument = implementation if meta['family']=='kernel' else dependency
            configure.append('-DCMASTERY_'+family.upper()+'_ROOT='+str(root_argument))
            if meta['family']=='kernel':
                configure.append('-DCMASTERY_'+family.upper()+'_INCLUDE='+str(dependency/'include'))
                if family=='runtime': configure.append('-DCMASTERY_RUNTIME_ADAPTERS='+str(dependency/'adapters'))
            record.setdefault('dependencies', {})[family] = snapshot(dependency)
            record.setdefault('external_sources', {})[str(dependency)] = record['dependencies'][family]['sha256']
            if assessor:
                ref = ROOT / 'assessor' / family
                configure.append('-DCMASTERY_'+family.upper()+'_ASSESSOR_ROOT='+str(ref))
                record.setdefault('dependency_assessors', {})[family] = snapshot(ref)
                record.setdefault('external_sources', {})[str(ref)] = record['dependency_assessors'][family]['sha256']
        test_command = ["ctest", "--test-dir", str(output), "--output-on-failure", "--no-tests=error", '--output-junit', str(output/'results.xml')]
        if not all_tests: test_command += ['-R', '^('+'|'.join(re.escape(t) for t in selected)+')$']
        commands = [configure, ["cmake", "--build", str(output)], test_command]
        for command in commands:
            result = invoke(command, cwd=workspace, timeout=timeout)
            record["steps"].append(result)
            if result["exit_code"] != 0:
                record["status"] = "failed"
                break
        else:
            record["status"] = "passed"
        junit = output/'results.xml'
        if len(record['steps']) == 3 and junit.is_file():
            cases = ET.parse(junit).getroot().findall('.//testcase')
            record['tests'] = [{'name': c.get('name'), 'status': c.get('status'), 'skipped': c.find('skipped') is not None, 'failed': c.find('failure') is not None} for c in cases]
            missing = set(selected) - {c.get('name') for c in cases}
            if not all_tests and missing: record.update(status='unavailable', reason='Required tests not registered: '+', '.join(sorted(missing)))
            if any(c.find('skipped') is not None for c in cases): record.update(status='unavailable', reason='A selected test was skipped')
            if not cases: record.update(status='unavailable', reason='No executed test evidence')
        elif record['status']=='passed':
            record.update(status='failed', reason='CTest returned success without its required test report')
        record["test_scope"] = "Complete family suite" if all_tests else "Project-selected executable anchors; see project_contract for limits"
        record['evidence_kind'] = 'host-and-emulator-execution' if any('guest' in c.get('name','') for c in record.get('tests',[])) else 'host-execution'
        target_files = [p for p in (output/'guest').glob('*') if p.is_file() and p.suffix in {'.json','.log'}]
        if target_files:
            record['target_evidence'] = {p.name:p.read_text(encoding='utf-8',errors='replace') for p in target_files}
            record['target_limits'] = 'Host compiler/profile flags do not instrument the freestanding guest. Guest compiler, flags, images and machine are recorded in build.json.'
    if snapshot(workspace)["sha256"] != before["sha256"]:
        record.update(status="failed", reason="Source changed during the run")
    if any(not Path(p).is_dir() or snapshot(Path(p))['sha256'] != digest for p,digest in record.get('external_sources', {}).items()):
        record.update(status='failed', reason='Dependency or assessor source changed during the run')
    save(workspace / "evidence/latest.json", record)
    return record

def report(workspace):
    workspace = Path(workspace).resolve()
    meta = read(workspace / ".cmastery/project.json")
    path = workspace / "evidence/latest.json"
    record = read(path) if path.exists() else {"status": "not-run"}
    if record.get('project',meta['project']) != meta['project']:
        record=dict(record,status='not-run',stale=True,reason='Evidence belongs to a different project')
    if record.get("source", {}).get("sha256") not in (None, snapshot(workspace)["sha256"]):
        record = dict(record, status="not-run", stale=True, reason="Workspace source changed after evidence")
    if any(not Path(p).is_dir() or snapshot(Path(p))['sha256'] != digest for p,digest in record.get('external_sources', {}).items()):
        record = dict(record, status='not-run', stale=True, reason='Dependency or assessor source changed after evidence')
    return {"project": meta["project"], "family": meta["family"], "evidence": record,
            "unresolved_claims": meta["source_catalog"].get("manual_evidence", [])}

def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("doctor")
    listing = sub.add_parser("list")
    listing.add_argument("--module")
    initialize = sub.add_parser("init")
    initialize.add_argument("project")
    initialize.add_argument("destination")
    initialize.add_argument('--family', help='Select an additional family listed for this project')
    advancing=sub.add_parser('advance',help='Select the next same-family project, preserving code and archiving evidence')
    advancing.add_argument('workspace')
    advancing.add_argument('project')
    checking = sub.add_parser("check")
    checking.add_argument("workspace")
    checking.add_argument("--compiler", choices=["gcc", "clang"], default="gcc")
    checking.add_argument("--profile", choices=["debug", "release", "asan"], default="debug")
    checking.add_argument("--assessor", action="store_true")
    checking.add_argument("--timeout", type=int, default=180)
    checking.add_argument('--all-tests', action='store_true', help='Run all family tests for integration/assessor review')
    checking.add_argument('--dependency', action='append', default=[], metavar='FAMILY=PATH', help='Explicit completed runtime or storage workspace')
    reporting = sub.add_parser("report")
    reporting.add_argument("workspace")
    args = parser.parse_args(argv)
    try:
        if args.command == "doctor":
            result = doctor()
        elif args.command == "list":
            result = [{"id": p["id"], "title": p["title"], "module": p["module"],
                       "lab_family": p.get("lab_family"), "lab_families":p.get('lab_families',[]), "requires": p.get("requires", [])}
                      for p in catalog() if not args.module or p["module"] == args.module]
        elif args.command == "init":
            result = instantiate(args.project, args.destination, args.family)
        elif args.command == 'advance':
            result = advance(args.workspace,args.project)
        elif args.command == "check":
            deps = dict(item.split('=', 1) for item in args.dependency)
            result = run_checks(args.workspace, args.compiler, args.profile, args.assessor, args.timeout, args.all_tests, deps)
        else:
            result = report(args.workspace)
        print(json.dumps(result, indent=2, ensure_ascii=False))
        return 1 if isinstance(result, dict) and result.get("status") in {"failed", "unavailable"} else 0
    except (OSError, ValueError, KeyError) as error:
        print(json.dumps({"status": "failed", "error": str(error)}))
        return 2
