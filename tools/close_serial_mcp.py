"""Attach exact unchanged PR11/12 criteria with separate author/physical dimensions."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_mcp';PROJECTS=['PR-T38-11','PR-T38-12'];REPORT='research/v5/expansion/legacy_serial_mcp/validation.json';SUPPORT='research/v5/expansion/legacy_serial_mcp/completion_dispositions.json'
from research.v5.expansion.legacy_serial_mcp.author_support import attach
ARTIFACTS=['labs/serial_followthrough/mcp/CONTRACTS.md','labs/serial_followthrough/mcp/BOARD_CAPTURE.md','assessor/serial_followthrough/mcp/mcp.c','assessor/serial_followthrough/mcp/spi_adapter.c','assessor/serial_followthrough/mcp/target.c','labs/serial_followthrough/mcp/tests.c','labs/serial_followthrough/mcp/check_dossier.py','assessor/serial_followthrough/mcp/ANALYSIS.md']
REMAINING=['PR-T38-11 actual board power/pins/clock/readback/internal-loopback captures remain required and not-run.','PR-T38-12 actual compatible independent peer dual-rate exchange, waveform and changed-condition measurement remain required and not-run.','Independent original and delayed learner assessment and external review remain not-run.','Remaining T39/T40 and mixed LRUN parents remain required.']
PATTERNS={'PR-T38-11':{'T38.12.a':['.layout','.transfer','.atomic','.mutation.RAM_BUDGET','.mutation.ZERO_FD'],'T38.12.b':['.init','.failure','.spi','.atomic','.mutation.CRC','.mutation.ECC','.mutation.EARLY_READY','.mutation.CS_QUARANTINE'],'T38.12.c':['dossier.controls','.f439.route']},'PR-T38-12':{'T38.12.a':['.bench','.transfer','.receive','.mutation.PEER_SEQUENCE'],'T38.12.b':['.transfer','.receive','.atomic','.mutation.PUBLICATION','.mutation.RX_ADVANCE'],'T38.12.c':['dossier.controls']}}
WHY={'PR-T38-11':{'T38.12.a':'Actual zero-byte FD and every payload-size/DLC bound, changed FIFO layout and adjacent-storage checks.','T38.12.b':'Selected current source/errata reset/clock/RAM/FIFO/filter/readback, every before/after SPI-phase fault and no early ready; actual struct ps_spi composition.','T38.12.c':'Concrete selected board route, source/errata assumptions and rejecting dossier protocol authored; actual changed-board observation is still required.'},'PR-T38-12':{'T38.12.a':'Independent deterministic0/8/12/16/20/64-byte sequence oracle, BRS/rate changed fixtures and actual controller admission; physical peer receipt remains absent.','T38.12.b':'Application content/sequence rejects despite link success, selected bounded attempts/cancellation and FIFO/error service; physical missing-peer repeat remains absent.','T38.12.c':'Required physical gate retained, raw-file and instrument/rate/changed-condition rejecting controls; synthetic fixtures never become physical evidence.'}}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json')
def all_rows(r):
 rows=[]
 for project in PROJECTS:
  for c in read(canon(project))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(p in q['id'] for p in PATTERNS[project][c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=WHY[project][c['objective']],remaining_author_work=[],required_physical_work=REMAINING[:2],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/mcp').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/peripheral_depth/spi/spi.h','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/spi/target.c','labs/peripheral_depth/spi/CONTRACTS.md','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/serial_depth/include/serial_depth.h','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 for project in PROJECTS:
  d=read(canon(project));old=HERE/(project+'.before-mcp.json')
  if not old.exists():old.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2],source_register='research/v5/expansion/legacy_serial_mcp/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original criterion/V1 pairs: current selected-controller/struct ps_spi implementation and synthetic/struct lr_cpu verification; required physical milestones remain not-run.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS additive MCP attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_mcp.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for project in PROJECTS:
  d=read(canon(project));old=read(HERE/(project+'.before-mcp.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
 r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR11/12 six original criterion/V1 pairs, author evidence separate from physical observations',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_mcp/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact MCP dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
