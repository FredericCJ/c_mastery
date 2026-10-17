"""Render audited T23--T29 assessments; never rewrite accepted baseline sources."""
from pathlib import Path
import copy
import hashlib
import importlib
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from technical_guidance import HINTS

DOMAINS = [f'T{n}' for n in range(23, 30)]
REGISTERS = ['research/v5/expansion/legacy_runtime/resources.json',
             'research/v5/b06_resources.json',
             'research/v5/expansion/legacy_systems/resources.json']
SOURCE_TOPICS = {
 'T23': ['FORK THREAD', 'FORK OPEN', 'OPEN', 'LSYS-WRITE', 'OPEN', 'OPEN', 'OPEN LSYS-WRITE', 'MMAP', 'MMAP', 'MMAP COND', 'FORK TERMIOS', 'B06-FSYNC LSYS-WRITE', 'TIMER', 'FORK OPEN THREAD'],
 'T24': ['SIGNAL', 'SIGNAL', 'FORK MMAP RECV', 'RECV', 'RECV LSYS-WRITE', 'RECV', 'RECV', 'B06-EPOLL', 'TIMER', 'TERMIOS', 'RECV COND', 'B06-URING B06-CANCEL B06-SENDFILE'],
 'T25': ['THREAD', 'THREAD C11-ATOMICS', 'COND', 'COND', 'COND THREAD', 'C11-ATOMICS', 'C11-ATOMICS', 'C11-ATOMICS', 'C11-ATOMICS', 'THREAD COND', 'C11-ATOMICS THREAD', 'C11-ATOMICS THREAD'],
 'T26': ['MSQUEUE', 'MSQUEUE C11-ATOMICS', 'C11-ATOMICS', 'MSQUEUE C11-ATOMICS', 'MSQUEUE C11-ATOMICS', 'MSQUEUE C11-ATOMICS', 'MSQUEUE', 'B06-KREF', 'B06-UCOPY B06-LOCKS B06-KUNIT', 'WORK B06-WORK B06-LOCKS'],
 'T27': ['RM21 CM4 B06-QEMU', 'RM21 CM4 B06-STBOOT', 'CM4 B06-AA32 LSYS-LD-KEEP', 'B06-AA32 RM21', 'LSYS-LD-LMA LSYS-GCC-LINK', 'RM21', 'LSYS-GCC-LINK LSYS-CLANG-FREE', 'LSYS-LD-LMA LSYS-LD-KEEP RM21', 'B06-BOOT RM21', 'B06-QEMU CM4'],
 'T28': ['RM21', 'RM21 CM4 LSYS-GCC-ASM', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21', 'RM21 B06-STIPC'],
 'T29': ['CM4 B06-AA32', 'CM4', 'CM4', 'CM4', 'CM4 C11-ATOMICS', 'C11-ATOMICS CM4', 'RM21', 'CM4 B06-STIPC', 'CM4', 'CM4 RM21'],
}
# Family choice follows the actual mechanism. J names the separately delayed transfer,
# not an assertion that the ordinary task supplies an independent assessment result.
TOPIC_FAMILIES = {
 'T23': ['ABCIJ','BCDGFJ','ABCDIJ','BCDFGJ','BCDEIJ','ACFIJ','ABCFIJ','ACFHIJ','BCDFGJ','BCFGIJ','BCDGFJ','ABCFGJ','ACFGHJ','AFIJ'],
 'T24': ['ACDFJ','ABCDGJ','BCFGHJ','BCFGJ','BCDFJ','BCDFJ','BCDGJ','ABCDGHJ','BCDGJ','BCDFIJ','BCGHJ','BCFGHIJ'],
 'T25': ['BCDGFJ','ABCFHJ','BCDGFJ','ABCDGFJ','ABCGJ','ACDFJ','ABCFIJ','ACDFIJ','ABCDGFJ','ADGHJ','BCFHIJ','BDFGJ'],
 'T26': ['ACDFJ','ACFHJ','ABCFHIJ','ABCDGFJ','ACDFJ','ABCDGFJ','ACFGHJ','ACDGIJ','ADFGIJ','ACDGIJ'],
 'T27': ['AFIJ','ACDIJ','BCDFIJ','ABCFHJ','BCDFIJ','BCDGFJ','BCDFGJ','BCDFIJ','BCFGJ','BDFGIJ'],
 'T28': ['ABCDIJ','BCDFIJ','BCDGJ','ABCDIJ','ABCDFHIJ','ABCFHJ','BCDGFJ','ABCDGFJ','ABCFHIJ','BCDFGJ','BCDFHIJ','BCDFGIJ'],
 'T29': ['ACDFIJ','ACDFIJ','BCDFHJ','ACDFIJ','ABCFIJ','BCDFHJ','BCDFGJ','ACDFIJ','ABCFHJ','BDFGHIJ'],
}

def read(path):
 return json.loads(path.read_text(encoding='utf-8'))

def write(path, value):
 path.parent.mkdir(parents=True, exist_ok=True)
 path.write_text(json.dumps(value, ensure_ascii=False, indent=2)+'\n', encoding='utf-8', newline='\n')

def meta(path):
 return json.loads(re.search(r'```json\s*\n(.*?)\n```', path.read_text(encoding='utf-8'), re.S)[1])

def pin(path, scope):
 return dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest(), passages=[scope])

def source_ids(topic):
 domain, number = topic.split('.')
 return [('R-'+name if name.startswith(('LSYS-', 'B06-')) else 'R-LRUN-'+name)
         for name in SOURCE_TOPICS[domain][int(number)-1].split()]

def construction_row(pid, oid, row):
 """Distinct implementation gates for the historically elective T26 projects."""
 row = copy.deepcopy(row)
 if pid == 'PR-T26-04':
  additions = {
   'a': 'Implement the fixed-storage C17 SPSC API in the retained labs/execution contract, with capacities 1,2,4,16 and INVALID for zero/non-power-of-two/>16. Exercise empty/full and FIFO over wrap; supply actual learner source and generated dependency closure. ',
   'b': 'Implement separate release/acquire publication and reuse edges with ordinary multiword slot payload. Add a controlled producer/consumer schedule at both edges, a real two-thread run and a model counterexample for weakening either edge. Keep any native racy mutant out of the correctness oracle. ',
   'c': 'Implement a reduced-width counter oracle that enumerates all reachable empty/full/wrap states for each selected capacity, then compare the actual unsigned-counter queue. Measure the actual atomic lock-free result and object helper references on each available target. ',
  }
  row['task'] = additions[oid[-1]] + 'Use the earlier analysis as a prediction, not as a substitute for these implementation artifacts. ' + row['task']
  row['expected'] += ' Construction passes only with the corresponding learner source, bounded model comparison, generated-code closure and ownership argument; the earlier awareness dossier alone fails.'
  row['variant'] = 'Independently revise the implemented queue and rerun its controls under this changed condition: '+row['variant']
 if pid == 'PR-T26-05':
  mechanism = {
   '04': 'Implement the bounded dummy-node MPMC queue with the retained four-participant/32-node contract and permitted failure statuses; preserve a finite history recorder and validate real-time FIFO ordering.',
   '05': 'Implement a tagged fixed pool with an explicit saturated-generation retirement policy, then use an 8-bit model to force every incarnation boundary without dereferencing freed memory.',
   '06': 'Implement the complete protect-publish-reread, clear, retire and scan protocol with two hazards per participant; force the removal/validation and concurrent-collector incarnation interleavings in a bounded model.',
   '07': 'Implement the externally serialized epoch state machine with explicit enter/leave/advance results; test stalled, nested-invalid, departed and saturated-epoch states. Do not silently claim concurrent metadata support.',
   '08': 'Implement the one-writer immutable-root/four-reader grace model with an explicit retained-old-version list; test stalled readers and borrowed callbacks, and reject a second writer unless an additional protocol is supplied.',
  }
  row['task'] = mechanism[oid.split('.')[1]]+' For this leaf, add an executable control or finite model witness specifically for the following obligation: '+row['task']
  row['expected'] += ' The required construction gate includes source, a criterion-specific failing control, ownership/resource accounting and a history/model witness. A checksum-only stress result or copied awareness explanation does not pass.'
  row['variant'] = 'Apply the changed condition to the learner implementation, preserving its original regression corpus: '+row['variant']
 return row

def security(oid):
 if oid != 'T23.05.c':
  return dict(applicable=False, scope='Original learner programs, synthetic local data, owned PTYs and emulators only. Native invalid-lifetime/race cases are analyzed or modeled unless an explicitly isolated original teaching experiment is supplied. Kernel execution is restricted to disposable VMs; no host policy changes or board flashing are performed by this authoring work.')
 return dict(applicable=True, scope='A bounded local pathname race against an original teaching reader; no external path or real data.',
  target='Original learner reader and two synthetic files in its newly owned private directory',
  environment='Unprivileged Linux child processes under a harness with a deadline and a fresh temporary directory',
  attacker_capabilities='The teaching adversary can replace the designated final-component symlink between controlled check/use stages; the delayed variant also controls one intermediate directory link.',
  isolation='Use only synthetic SAFE and OTHER teaching-file contents, exact owned paths and explicit synchronization pipes; never follow a test link outside that directory or change host permissions.',
  defect='Authority checked against one name resolution is reused after an attacker-controlled replacement; the security property is reading only the authorized teaching object.',
  exposing_transition='Pause the reader after its check, replace the designated symlink, then resume its open/read and compare the observed synthetic marker.',
  exploitability_limits='The adversary must control the relevant name component and win the scheduled interval. Demonstrating wrong-file access does not demonstrate arbitrary code execution, privilege escalation or confinement under a different resolution policy.',
  success_condition='The vulnerable original reader returns the OTHER marker instead of SAFE, confined to the two teaching files.',
  repair='Acquire and validate the intended object under one documented descriptor/resolution policy. Match final-component-only versus full-path confinement explicitly; O_NOFOLLOW on the last component alone does not solve intermediate traversal.',
  regression='Repeat both controlled schedules against flawed and repaired readers; add a preexisting link, intermediate-link variant, failed open and descriptor-cleanup checks.',
  residual_limits='The claimed repair is bounded by the selected kernel API and threat model; exact resolution API passages must be inspected before claiming stronger confinement.')

def main():
 resources = {}
 for path in REGISTERS:
  for value in read(ROOT/path)['resources']:
   assert value['id'] not in resources
   resources[value['id']] = (path, value)
 units = read(ROOT/'curriculum/v4/units.json')['units']
 taxonomy = read(ROOT/'curriculum/taxonomy.json')['domains']
 total = 0
 for domain in DOMAINS:
  cards = importlib.import_module(domain.lower()+'_cards').rows()
  tax = next(d for d in taxonomy if d['id'] == domain)
  leaves = {o['id']: o.get('text', o.get('title')) for t in tax['subtopics'] for o in t['objectives']}
  assert all(leaves.values()), 'taxonomy objective label unavailable'
  audit, authored, seen = [], [], set()
  embedded = int(domain[1:]) >= 27
  for project in sorted((ROOT/'curriculum/projects').glob('PR-'+domain+'-*.md')):
   m = meta(project); pid = m['id']
   objectives = {oid: title for oid,title in leaves.items() if oid.rsplit('.',1)[0] in m['topics']}
   assert set(objectives) <= set(cards)
   pu = [u for u in units if u['project'] == pid]
   tests = sorted({t for u in pu for t in u.get('tests', [])})
   module = ROOT/'curriculum/modules'/(domain+'.md')
   fixture = ROOT/m['fixtures'][0]
   oracle_path = 'tools/verify_embedded_fixtures.py' if embedded else 'tools/verify_systems_fixtures.py'
   bindings = [pin(module,'All assigned topic teaching and prerequisite/reading metadata'),
               pin(project,'Metadata, Contract, Steps, Failure cases, Acceptance, Hints and changed-condition work'),
               pin(fixture,'Complete synthetic inputs and expected outputs'),
               pin(ROOT/oracle_path,'Exact branches used by the assigned synthetic fixture; not target execution')]
   lab = 'firmware_io' if embedded else ('execution' if domain=='T26' else 'hosted_systems')
   bindings.append(pin(ROOT/'labs'/lab/'README.md','Named implementation contracts and explicit execution/physical limitations'))
   if lab != 'execution':
    bindings.append(pin(ROOT/'labs'/lab/'CONTRACTS.md','All operation contracts, preconditions, failure and ownership boundaries'))
   ids = sorted({sid for topic in m['topics'] for sid in source_ids(topic)})
   sources = [dict(id=i,path=resources[i][0],passages=resources[i][1]['locators']) for i in ids]
   criteria = []
   analysis = ['# '+pid+' — supplemental assessor analysis','','Assessor only. Authored technical arguments do not assert a learner attempt, independent human review or fresh execution of the complete project. The original implementation contract remains binding.','']
   for oid,title in objectives.items():
    row = construction_row(pid,oid,cards[oid]); topic=oid.rsplit('.',1)[0]
    cid='AX-'+pid+'-'+oid; aid=oid.lower().replace('.','')
    families=list(dict.fromkeys(TOPIC_FAMILIES[domain][int(oid.split('.')[1])-1]))
    if oid != 'T23.05.c' and 'E' in families: families.remove('E')
    if pid in ['PR-T26-04','PR-T26-05'] and 'B' not in families: families.insert(1,'B')
    expected=row['expected']; variant=row['variant']; task=row['task']
    criterion=dict(id=cid,title=title,objective=oid,families=families,source_records=[s for s in sources if s['id'] in source_ids(topic)],
     task=task,purpose=row['misconception'],
     prerequisites=dict(knowledge=['Retained '+topic+' teaching passage and parent prerequisites','Active C17 semantics kept separate from POSIX/Linux, ISA, ABI and selected board contracts'],artifacts=[pid+' learner-owned implementation or diagnostic dossier','Predicted original fixture result before inspecting its expected data']),
     reasoning=[row['misconception']+' Refute this explanation with the smallest relevant state or execution.','State valid inputs, operation pre/postconditions, object ownership/lifetime and failure/publication boundaries for the specified mechanism.','Account for resource bounds and progress/complexity under the declared participant, storage and target assumptions. Name genuinely inapplicable costs instead of inventing them.'],
     deliverables=dict(D1='Pre-experiment prediction and causal argument addressing: '+row['misconception'],D2=task,D3='Exact commands or derivation steps, original inputs, observed outputs, a minimized disagreement and an independent control for this leaf. Retain unsupported-tool/target outcomes separately.',D4=variant),
     verification=dict(procedure='Predict the specified result first; execute or derive the concrete task within the shared and parent contracts. Retain its ordinary case, boundary/failure case and a criterion-specific falsification control. Compare to the independent expected argument below, not only to the old synthetic fixture. Record implementation/target versions and all source dependencies.',expected=expected,**{'pass':'Every task clause and the expected causal argument must be supported by the actual learner artifact/trace. A matching number with the stated misconception, missing ownership boundary or unsupported target claim fails this objective.'},limits='Authored per-leaf work is unattempted. Named laboratory anchors cover only their explicit cases. Newly executed lifecycle tests are assessor-reference evidence, not learner completion or the full project.'),
     rubric={'0':'Missing/incorrect artifact, violated containment/precondition, or a false semantic claim.','1':'Reproduces the ordinary case but cannot reject this leaf’s stated misconception or establish its boundary.','2':expected+' Supplies the criterion-specific evidence and a distinct falsification control.','3':variant+' Derives and verifies the changed obligations independently after the recorded delay.'},
     hints=HINTS[topic],variants=[dict(id=cid+'-DELAYED',minimum_days=7,task=variant)],assessor_anchor='#'+aid,
     facet_scope=dict(teaching='Retained '+topic+' teaching passage; inspected source passages listed separately.',implementation_or_analysis='Retain the parent construction contract and add this explicit investigation. '+('Required implementation gate despite historical elective metadata.' if pid in ['PR-T26-04','PR-T26-05'] else 'No new accepted unit or historical hour is counted.'),boundary_adversarial='The concrete altered, invalid, stalled, exhausted or failure state in this task has a separate expected result and evidence obligation.',independent_assessment='New per-objective level2 rubric and seven-day changed-condition level3 variant; no attempt or reviewer independence claimed.',remaining='Learner artifact, criterion evidence review and delayed attempt remain required. Unavailable source, hardware, guest and broader experiment gaps remain open.'),security=security(oid))
    criteria.append(criterion);seen.add(oid);authored.append(dict(project=pid,objective=oid,criterion=cid))
    analysis += ['<a id="'+aid+'"></a>','## '+oid+' — '+title,'',expected,'','Reject: '+row['misconception'],'','Evidence discriminator: locate the first task-specific state, return value, ownership transition or artifact that would differ under the rejected explanation. Require that observation or a complete finite derivation; reject a generic checksum, sanitizer absence or source heading as a substitute.','', 'Delayed assessor check: '+variant,'']
   anchors=[]
   if pid=='PR-T23-01':
    anchors=[dict(criterion='AX-'+pid+'-'+o,test='posix_lifecycle_depth.launch',scope=s) for o,s in [('T23.02.a','Original exit0/127, missing image ENOENT, self-SIGTERM, owned descriptor cleanup and exact reap; no forced EINTR.'),('T23.02.b','Close-on-exec channel and two parent acquisition failures; short error records and premature pre-exec death remain authored, unexecuted obligations.')]]
   if pid=='PR-T25-01':
    anchors=[dict(criterion='AX-'+pid+'-T25.01.a',test='posix_lifecycle_depth.pool',scope='All successful-prefix creation/failure cases for0..4 actual pthreads, result sum and invalid-input preservation; no detached/cancellation/join-failure recovery claim.')]
   execution=dict(kind='retained partial anchors plus additive lifecycle laboratory' if anchors else 'retained implementation/analysis contracts and bounded partial anchors',scope='Only the named cases count as executed support. The full criterion and independent transfer require separate learner evidence.',tests=tests+[a['test'] for a in anchors],fresh_execution='Source-pinned posix_lifecycle_depth campaign only; the complete supplemental criteria are unattempted.' if anchors else 'not-run in this depth-authoring wave')
   if anchors: execution.update(criterion_anchors=anchors,campaign='reviews/v5/expansion/legacy_runtime/posix_lifecycle_depth_campaign.json')
   gaps=['Published C17/C23 texts and private TLPI/CS:APP/book passages are not newly inspected; N1570 and listed primary passages have only their recorded scope.','Historical reading metadata is not promoted to a new source-inspection claim.']
   if embedded: gaps += ['Selected current device datasheet, exact board schematic/revision, errata and full vendor startup pair still require inspection; RM0090 Rev21 is historical, and MPS2 is a different target.','Physical electrical behavior, peripheral timing, cache maintenance and reset/boot observations remain unvalidated; this authoring task performs no flashing.']
   else: gaps += ['Exact POSIX/Linux interface passages not listed here—including detailed signal restart/allowlist, pathname confinement and mmap fault rules—remain required before stronger claims.']
   if domain=='T26': gaps += ['Full hazard/epoch/RCU proof literature and the exact selected LKMM/RCU passages require separate inspection beyond corrected queue pseudocode and retained implementation contracts.','Disposable-VM kernel execution remains required but unavailable in this wave; host models are not guest evidence.']
   target = 'Dedicated MPS2 emulator or explicitly identified STM32 teaching board; host register models are separate from target behavior.' if embedded else 'Unprivileged Linux x86-64 for original hosted experiments; pure C memory models and restricted-profile target code retain their own contracts. Kernel execution only in a disposable VM.'
   supplied=[project.relative_to(ROOT).as_posix(),fixture.relative_to(ROOT).as_posix(),oracle_path,'labs/'+lab+'/README.md']
   contracts=['curriculum/v5/expansion/legacy_assessments/shared-contract.md']
   if anchors:
    supplied += ['labs/posix_lifecycle_depth/include/posix_lifecycle.h','labs/posix_lifecycle_depth/src/posix_lifecycle.c','fixtures/posix_lifecycle_depth/cases.json']
    contracts += ['labs/posix_lifecycle_depth/CONTRACTS.md']
   record=dict(schema_version=1,project=pid,domain=domain,title=m['title']+' — explicit depth assessments',required=True,shared_contracts=contracts,
    baseline_binding=dict(files=bindings,teaching=[dict(path=module.relative_to(ROOT).as_posix(),anchors=m['topics'])],units=[u['id'] for u in pu],inspection_limits=['All assigned topic teaching, project contract/steps/failure/acceptance and complete fixture inspected.','Unit objective/test links and first substantive steps inspected; no complete historical learner portfolio review.','Selected baseline laboratory contracts/tests and assessor functions inspected; exact scopes are in the audit, not inferred from titles.']),
    environment=dict(language='Active C17. N1570 is a C11 draft comparison, not a substitute citation for C17/C23. GNU/POSIX/Linux, ABI, ISA and board rules are labeled separately.',target=target,profile=m['profile'],dependencies='Retain the parent profile. Python, host fixture tools and hosted verification harnesses are distinct from restricted learner/target closures. No library may implement the mechanism being taught.',toolchain=['Record GCC/Clang, build tools, target triple, optimization and generated helper/link dependencies.','Device/VM/tool unavailability is a missing observation, not a passing result.']),source_records=sources,reference_gaps=gaps,supplied_artifacts=supplied,execution_support=execution,criteria=criteria,assessor_analysis='assessor/legacy_depth/'+pid+'.md',
    status=dict(content='authored',execution='Only the explicitly linked lifecycle assessor anchors were newly executed; supplemental learner investigations remain unattempted.',independent_review='not-run',learner_assessment='not-assessed',limitations=['Finite tests do not prove absolute mastery.','Accepted source bytes, IDs, units, historical hours and receipts remain unchanged.','Historical optional labels do not exclude required expansion work.']))
   f=read(fixture);kind=f.get('oracle_type',f.get('input',{}).get('kind','declared model'))
   audit.append(dict(project=pid,objectives=list(objectives),baseline_bindings=bindings,teaching='Substantive retained topic passages',construction='Concrete retained implementation contract; historically elective T26 construction made required in additive supplement',boundary='Named bounded baseline cases plus broader authored project obligations',independent_assessment_before='General acceptance/transfer and topic checklist, lacking explicit per-leaf four-level rubric',expansion='Distinct leaf investigations, misconception controls, two hints, expected arguments and delayed variants',tests=tests,fixture_limit=kind+' is only its declared synthetic model; no platform observation inferred.'))
   # Publish the audit before the supplement, preserving authoring order.
   write(HERE/(domain+'_audit.json'),dict(domain=domain,status='author passage/depth audit',projects=audit,limitations=['No independent human review or fresh full baseline campaign.']))
   write(ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json'),record)
   ap=ROOT/record['assessor_analysis'];ap.parent.mkdir(parents=True,exist_ok=True)
   ap.write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n')
  assert seen==set(cards),(domain,set(cards)-seen)
  write(HERE/(domain+'_remaining.json'),dict(domain=domain,required=True,unique_objectives=len(seen),authored_criteria=len(authored),pending=[dict(**x,work='Perform the criterion-specific learner implementation/analysis, review its evidence and complete the delayed transfer.',status='unattempted') for x in authored],external=['Reference and target gaps in each supplement remain required.','No physical, guest-kernel or complete portfolio validation inferred.']))
  total+=len(authored)
  print(domain,len(audit),'projects',len(seen),'unique objectives',len(authored),'criteria')
 assert total==258,total

if __name__=='__main__':
 main()
