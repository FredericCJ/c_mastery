"""Attach peripheral diagnosis support without rewriting original criteria."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T31-03';SUPPORT='research/v5/expansion/legacy_embedded_debug_peripheral/support.json';CONTRACT='labs/embedded_debug_depth/peripheral/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_embedded_debug_peripheral/campaign.json'
SCOPES={
'T31.05.a':'32-state diagnostic plant with five separate interventions and equal software signatures for wrong mode/route; physical discrimination plan is ready, actual pin observation pending.',
'T31.05.b':'Actual frozen UART memory-versus-line ownership and SPI receive/BSY/hold sequence; mode-scoped current errata policy with480 independent cases/profile and explicit final-item/timebound premises.',
'T31.05.c':'Checked common-clock affine interval mapping with unchanged provenance,1024 independent cases/profile, overflow refusal and complete physical acquisition/no-edge plan; no actual pin capture.',
'T31.06.a':'Actual PdTransfer12-of32 cancellation/EN/FIFO/TC/finalization/reuse trace, stale generation rejection, both cancel-before-ready and ready-before-cancel outcome orders and once-only release.',
'T31.06.b':'Actual frozen DMA core with separate CPU/backing-memory cache model, dirty-tail preservation/speculative refill/final invalidation and uncached F439 contrast; unreachable CCM admission refusal.',
'T31.06.c':'2048 independently checked halted/free-running schedules/profile, separate watchdog freeze and DMA progress, reset-before-feed ordering and explicit model rather than physical timestamp claim.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Six original criteria plus six named changes with executable diagnostic/provider models and ready physical acquisition plans; physical and independent dimensions remain pending. Historical status unchanged.'});rows.sort(key=lambda r:r['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 supplied=['fixtures/embedded/PR-T31-03.json','labs/serial_depth/include/serial_depth.h','labs/serial_depth/CONTRACTS.md',*[f'labs/peripheral_depth/{x}/{n}' for x in ('uart','spi','dma') for n in (x+'.h','CONTRACTS.md')],*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/embedded_debug_depth/peripheral').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for p in supplied:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'))
 from expansion_evidence import require_batch_research
 from check_embedded_debug_peripheral import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-criteria-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'implemented-diagnostic-models-and-actual-producer-composition-physical-plan-ready','evidence':['assessor/embedded_debug_depth/peripheral.c','assessor/embedded_debug_depth/peripheral.md','labs/embedded_debug_depth/peripheral/imports.c','labs/embedded_debug_depth/peripheral/tests.c','labs/embedded_debug_depth/peripheral/oracle.py']},'author_validation':{'state':'passed-six-host-profiles-semantic-controls-and-independent-finite-oracles','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-physical-pin-capture-silicon-errata-DMA-and-debug-freeze-unobserved','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan; explicit frozen UART/SPI/DMA/serial producers; scripted ports and independent Python finite models. Dedicated board/probe/acquisition equipment required for physical trace.',
   'external_blockers':['Independent human review and minimum-seven-day unaided learner assessment remain unattempted.','Physical edge capture, clock/probe/trigger calibration, silicon errata and watchdog/DMA debug behavior require a dedicated board and actual observations.'],
   'evidence_limits':['Five-switch plant observations are original predictions, not acquired hardware measurements.','Timestamp compatibility presumes valid common-clock calibration and provenance; neither is authenticated by the numeric API.','DMA race order is serialized adapter admission, not an inference about physical event time or concurrent callback safety.','H745-shaped cache script exercises the actual provider policy but does not execute a cache controller; F439 SRAM branch deliberately makes no irrelevant data-cache calls.','Current ES0206 passages are inspected; no old edition chronology or whole-device conformance is claimed.','Physical acquisition variants have ready procedures and model controls; no raw physical capture was produced.','Finite tests/oracles do not prove all executions or independent mastery; exact published C17 remains unavailable.']}
  updates.append(row)
  for variant in c['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=c['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Executable changed-condition model/provider controls and complete acquisition plan available; physical observation and delayed independent learner evidence remain separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_embedded_debug_peripheral.py','assessor/embedded_debug_depth/peripheral.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_embedded_debug_peripheral.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Physical calibrated pin capture and silicon watchdog/DMA/erratum observations.','Independent review and delayed learner assessment.'],'scope':'All six original criteria and six named changes under selected host/provider/model contracts; physical acquisition remains separately required, no fabricated measurements.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
