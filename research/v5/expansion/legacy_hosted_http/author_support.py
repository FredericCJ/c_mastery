"""Pure additive source-bound attachments for all24 original T42 criteria."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from criterion_notes import NOTES
LAB=ROOT/'labs/hosted_followthrough/http';REF=ROOT/'assessor/hosted_followthrough/http';CONTRACT=LAB.relative_to(ROOT).as_posix()+'/CONTRACTS.md';SUPPORT=HERE.relative_to(ROOT).as_posix()+'/support.json';CAMPAIGN='reviews/v5/expansion/legacy_hosted_http/campaign.json';PROJECTS=[f'PR-T42-{i:02}' for i in range(1,9)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 return sorted({*[p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts],*[p.relative_to(ROOT).as_posix() for p in (ROOT/'fixtures/network_http').glob('*.json')],'labs/communications/include/com_http.h','labs/communications/include/com_net.h','labs/communications/learner/http.c','labs/communications/tests/net.c','labs/http_depth/include/http_depth.h','labs/http_depth/src/http_depth.c'})
def attach(record):
 if record['project'] not in PROJECTS:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[a for a in additions if a.get('path')!=SUPPORT];additions.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact original byte/lifetime/time/queue/socket/customTCP/16bit-width tasks and changed conditions; external dimensions separate.'});additions.sort(key=lambda x:x['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for p in public_paths():
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def prepare():
 rows=[];analysis=['# T42 assessor derivations','','Withheld from learner exports. These exact original tasks and changed conditions have mechanism-specific checks; author observation is not independent or delayed learner performance.','']
 for record in records():
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']];rows.append({'id':c['id'],'project':record['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Prediction, live-storage/ownership/byte/time contract.','Original implementation and exact changed-condition reasoning.','Discriminating raw positive/negative observations with source/tool identity.','Separate unaided minimum-seven-day observer record.']});analysis+=['## '+c['id'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','']
 assert len(rows)==24 and {r['objective'] for r in rows}==set(NOTES)
 write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'Original tasks/expected/variants unchanged; no assessor implementation exported.'});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n');print('Prepared24 original and24 changed conditions')
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_hosted_http import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];paths=[]
 for record in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(record['project']+'.json');write(path,attach(record));paths.append(path)
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']];row={'id':c['id'],'project':record['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/hosted_followthrough/http/criteria.json']},'implementation':{'state':'implemented-or-analyzed-exact-byte-lifetime-and-changed-premise','evidence':['assessor/hosted_followthrough/http/flow.c','assessor/hosted_followthrough/http/server.c','assessor/hosted_followthrough/http/ANALYSIS.md','tools/adapt_hosted_http.py',*public_paths()]},'author_validation':{'state':'passed-source-bound-native-socket-customTCP-and-AVR-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-not-required-for-loopback;customTCP-cable-and-AVR-board-separate','evidence':[]}},'required_environment':'C17 GCC15.2/Clang21.1.8 Linux;owned loopback/curl;existing com_net packet model;AVR-GCC14.3 and QEMU10.2.1 arduino-uno. No host security changes.','external_blockers':['Independent human review and minimum-seven-day unaided learner observation.'],'evidence_limits':['Finite original profile;no generalHTTP/TCP/TLS/proxy/production conformance.','Actual16bit AVR observation exercises numeral/storage-domain core only;not a complete HTTP server on2KiB SRAM.','Socket send acceptance is not peer application consumption.','Async compute order and wall rollback have explicit worked models;no external scheduler or clock adjustment is invented.','No physical cable/board observation;exact publishedC17 inaccessible.']}
   updates.append(row)
   for v in c['variants']:
    child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=variant,variant_ids=[v['id']],variant_readiness='Exact changed-condition implementation or worked premise analysis;original delayed independent gate remains separate.');updates.append(child)
 resources=json.loads((HERE/'resources.json').read_text());pins=[*paths,ROOT/CAMPAIGN,Path(__file__),HERE/'criterion_notes.py',HERE/'register_sources.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_hosted_http.py',*[ROOT/p for p in campaign['inspected_paths']]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_hosted_http.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review and original delayed learner assessment.','Physical cable/board only for separate cross-domain claims.'],'scope':'All24 exact T42 original criteria and24 exact variants, with per-mechanism checks and bounded premise analyses.'});print(json.dumps({'criteria':24,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
