"""Canonical additive T27 attachments and exact original criterion preservation."""
from pathlib import Path
import json,sys,hashlib,subprocess
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/bringup_depth').rglob('*')) if p.is_file() and '__pycache__' not in str(p)]
before={};mapping=[]
for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T27-*.json')):
 d=read(path);baseline=json.loads(subprocess.check_output(['git','show','HEAD:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf8'));assert d['criteria']==baseline['criteria'];before[d['project']]=hashlib.sha256(json.dumps(d['criteria'],sort_keys=True).encode()).hexdigest()
 write(HERE/('attachment_'+d['project']+'.json'),dict(project=d['project'],domain='T27',key='bringup_successor',supplied_artifacts=public,shared_contracts=['labs/bringup_depth/CONTRACTS.md','labs/bringup_depth/BOARD_CAPTURE.md'],source_register='research/v5/expansion/legacy_advanced_bringup/resources.json',support='research/v5/expansion/legacy_advanced_bringup/completion_dispositions.json',criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original T27 criteria, separate host/model/MPS2/cross-build/physical dimensions; no other project closed by association'))
 attached=attach(d);assert attach(json.loads(json.dumps(attached)))==attached;assert attached['criteria']==baseline['criteria'];write(path,attached)
 for c in d['criteria']:
  topic=c['objective'].split('.')[1];modes={'01':['authority ledger','source access','original/cross images'],'02':['boot-offset','software-reset','watchdog-cross','handoff Boolean model'],'03':['independent image','eight actual ELF mutations','ram-vectors','boot-offset'],'04':['stack argument','guard','stack-overlap','stack-align','fp'],'05':['memory byte oracle','DMA inaccessible bank','independent image'],'06':['clock tests','fp disassembly/attributes','crystal/bypass source analysis'],'07':['hook DAG/failure tests','forbidden helper link','BU_RETURN_PASS'],'08':['vendor missing/repaired copy','vendor and original image/maps/closure'],'09':['retained cuts','generation model','handoff Boolean model','schema migration'],'10':['ram-markers','bad-lma','bad-zero','two-defects','board capture rejection']}[topic]
  mapping.append(dict(project=d['project'],criterion=c['id'],objective=c['objective'],original_task=c['task'],delayed=c['variants'][0],mechanism_evidence=modes,teaching='labs/bringup_depth/CONTRACTS.md',analysis='assessor/bringup_depth/ANALYSIS.md',learner_artifacts=['bringup.c','target/reset.S','identity/source dossier','ELF/map/closure dossier','board_capture.json'],independent_assessment='Original criterion rubric and minimum7days unchanged; no attempt yet'))
write(HERE/'objective_mapping.json',dict(scope='30 original criteria and30 changed tasks, no additional hours or accepted identity',criteria=mapping))
write(ROOT/'reviews/v5/expansion/legacy_advanced_bringup/integration.json',dict(passed=True,criteria=30,delayed=30,original_criteria_hashes=before,pure_hook_idempotent=True,public=public,limits='Additive canonical attachment only; passing campaigns and physical evidence tracked separately'))
print(dict(projects=len(before),criteria=len(mapping),public=len(public)))
