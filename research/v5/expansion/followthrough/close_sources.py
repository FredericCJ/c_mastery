"""Record bounded source corrections without changing historical inspection claims."""
import hashlib
import json
from pathlib import Path
import sys
from datetime import datetime, timezone
ROOT=Path(__file__).resolve().parents[4]
sys.path.insert(0,str(ROOT/'tools'))
from common import load_taxonomy
import vocabulary
FOLDER=Path(__file__).parent
def read(path): return json.loads(path.read_text(encoding='utf-8'))
def dump(path,value): path.write_text(json.dumps(value,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
def name(path): return path.relative_to(ROOT).as_posix()
def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()

resource_paths=sorted(FOLDER.glob('*resources.json'))
resources={r['id']:r for r in read(ROOT/'references/resources.json')['resources']}
added=[]
for path in resource_paths:
    for row in read(path)['resources']:
        resources[row['id']]=row;added.append(row['id'])
modern_resource_paths=sorted((ROOT/'research/v5/modern').glob('*resources.json'))
for path in modern_resource_paths:
    for row in read(path)['resources']:
        if not row.get('reuse'): resources[row['id']]=row
entries,_=vocabulary.load()
errors,pending=vocabulary.validate(entries,list(resources.values()),load_taxonomy())
corrections=[]
for path in sorted(FOLDER.glob('*vocabulary_corrections.json')):
    doc=read(path)
    for index,row in enumerate(doc['changed_entries']):
        default=doc.get('inspected_source')
        if path.name=='runtime_vocabulary_corrections.json':
            default=read(FOLDER/'runtime_vocabulary_resources.json')['resources'][0]['id']
        corrections.append((path,index,row,default))
if len(corrections)!=126:
    errors.append('Expected all126 original pending authorities; observed '+str(len(corrections)))
current={row['id']:row for row in entries}
gaps=[]
for path,index,row,default in corrections:
    rid=row.get('replacement_resource',default)
    matches=[a for a in current[row['entry']]['authority'] if a['resource_id']==rid and a['locator']==row['replacement_locator'] and a.get('inspection')=='registry']
    if len(matches)!=1: errors.append('Correction lacks unique current authority: '+row['entry'])
    gaps.append({'id':'G-SOURCE-'+row['entry'],'kind':'previously-uninspected-authority',
      'scope':row['entry'],'originating_records':[name(path)+'#/changed_entries/'+str(index),row['file']],
      'prior_authority':row['prior_authority'],'requirement':'Inspect the actual bounded passage, correct the definition and retain version/semantic boundaries.',
      'status':'author-source-inspection-complete','replacement_resource':rid,'replacement_locator':row['replacement_locator'],
      'evidence':[name(path),row['file'],'research/v5/expansion/followthrough/sources.md'],
      'limits':'Source traceability is not external technical review, implementation execution or learner evidence.'})
gaps += [
 {'id':'G-B01-ISO17','kind':'unavailable-reference','scope':'Exact published ISO/IEC9899:2018 text',
  'status':'open-source-access','requirement':'Inspect the published text for exact-edition claims, or preserve explicitly bounded draft/correction support without claiming that inspection.',
  'attempts':['N2176 web open returned internal error on2026-10-17.','Authorized library catalog results did not expose a usable published text; absence is not established.'],
  'evidence':['curriculum/v5/expansion/gaps.json','research/v5/expansion/followthrough/sources.md'],
  'required_environment':'Legitimate accessible published standard or exact authorized excerpt.','limits':'Does not block original C17 exercises supported by precisely identified drafts and adopted corrections; ambiguous claims remain open.'},
 {'id':'G-SOURCE-PUBLISHED-C23','kind':'unavailable-reference','scope':'Exact published ISO/IEC9899:2024 text','status':'open-source-access',
  'requirement':'Verify any exact published-edition comparison against an accessible authorized text.','evidence':['research/v5/expansion/followthrough/sources.md'],
  'required_environment':'Legitimate accessible published standard or exact authorized excerpt.','limits':'N3096 remains a prepublication draft; no unsupported promotion to the published standard.'},
 {'id':'G-SOURCE-POSIX2024-RDLOCK','kind':'unavailable-reference','scope':'POSIX.1-2024 pthread_rwlock_rdlock exact-edition comparison','status':'open-source-access',
  'requirement':'Read the requested current-edition function contract before claiming an inspected2024 comparison.',
  'attempts':['https://pubs.opengroup.org/onlinepubs/9799919799/functions/pthread_rwlock_rdlock.html returned403 on2026-10-17.'],
  'evidence':['research/v5/expansion/followthrough/systems_vocabulary_resources.json'],
  'required_environment':'Accessible authorized POSIX2024 page.','limits':'The explicitly identified POSIX2017 synchronization contract remains usable; no blocked implementation inferred.'}]
dump(FOLDER/'source_gap_ledger.json',{'schema_version':1,'policy':'Original authority identities retained; source access, implementation and independent review are separate.','gaps':gaps})
receipt=ROOT/'reviews/v5/expansion/followthrough/vocabulary_sources.json'
inputs=resource_paths+modern_resource_paths+sorted(FOLDER.glob('*vocabulary_corrections.json'))+[FOLDER/'sources.md',Path(__file__),ROOT/'tools/vocabulary.py']+list(vocabulary.files())
dump(receipt,{'schema_version':1,'observed_utc':datetime.now(timezone.utc).isoformat(),'command':'.venv/Scripts/python.exe -X utf8 research/v5/expansion/followthrough/close_sources.py',
 'passed':not errors and not pending,'exit_status':int(bool(errors or pending)), 'corrected_authorities':len(corrections),'pending_authorities':pending,'errors':errors,
 'input_sha256':{name(p):digest(p) for p in inputs},'scope':'Validate source identities and glossary locators; actual passage inspections documented separately. No executed C, independent reviewer or physical claim.'})
if errors or pending: raise SystemExit(str({'errors':errors,'pending':pending}))
dump(FOLDER/'research_gate.json',{'schema_version':1,'batch':'FOLLOWTHROUGH','status':'passage-inspected','date':'2026-10-17',
 'scope':'Bounded same-author source inspection and correction, not independent acceptance.','resource_ids':added,
 'research_input_sha256':{name(p):digest(p) for p in resource_paths+[FOLDER/'sources.md']},
 'limitations':['Published C17/C23 and POSIX2024 access gaps remain explicitly separate.','No whole-document inspection, physical measurement or independent learner/reviewer claim.']})
updates_path=ROOT/'curriculum/v5/expansion/implementation_updates.json'
updates=read(updates_path); byid={r['id']:r for r in updates['updates']}
for gap in gaps[:126]:
    evidence=gap['evidence']
    byid[gap['id']]={'id':gap['id'],'dimensions':{
      'teaching':{'state':'complete','evidence':evidence},
      'implementation':{'state':'not-applicable-source-correction','evidence':evidence},
      'author_validation':{'state':'passed','evidence':[name(receipt)]},
      'external_review':{'state':'not-run','evidence':[]},
      'independent_learner':{'state':'not-applicable-authority-record','evidence':[]},
      'physical_observation':{'state':'not-applicable-source-correction','evidence':[]}},
      'closure_rationale':'Actual bounded passage inspection, explicit corrected authority, edition limits and glossary consistency validation.',
      'evidence_limits':['No implementation-execution, physical, independent-review or learner-mastery claim.']}
updates['updates']=list(byid.values());dump(updates_path,updates)
print({'corrected_authorities':len(corrections),'pending':len(pending),'errors':len(errors)})
