"""Attach only the six exact T05.02 tasks; preserve other parent obligations."""
from common import ROOT,dump,read_json,sha
BASE='research/v5/expansion/legacy_portability_followthrough'
PUBLIC='labs/language_portability/migration'
LOCATORS=['Scope and prerequisites','Representation and operations','Six exact tasks and experiments','Verification, evidence and costs','Deliverables, rubric and hints','Status and limits']

def main():
 contract=PUBLIC+'/CONTRACTS.md'
 records=[]
 for name,edition,year,locators in [('N1570','Committee draft, April12,2011; not published C17',2011,['6.5.2.2p1-10','6.7.6.3p1-15']),('N3096','Working draft, April1,2023; not published C23',2023,['6.7.6.3p1-14'])]:
  pdf='build/portability-sources/'+name.lower()+'.pdf'
  records.append(dict(id='R-LP-'+name+'-FUNCTIONS',title=name+' function declarations and calls',url='https://www.open-std.org/jtc1/sc22/wg14/www/docs/'+name.lower()+'.pdf',authors=['ISO/IEC JTC1/SC22/WG14'],edition=edition,year=year,kind='standard-draft',access='Passage inspected',domains=['T05'],role='primary',reason='Exact bounded function declaration/call comparison; drafts are not relabeled as published editions.',prerequisites=['T02 function types'],assignable=True,locators=locators,inspection=dict(date='2026-10-18',method='Read actual public PDF passages through web document text; downloaded identical primary PDF for byte identity',scope='; '.join(locators),limits='Only named clauses inspected for this packet; no whole-document or exact published C90/C17/C23 claim.',download_path=pdf,download_sha256=sha(ROOT/pdf))))
 records.append(dict(id='R-LP-MIGRATION-CONTRACT',title='Function declarations, separate objects and typed callbacks',url=contract,authors=['Curriculum authors'],edition='Original repository contract,2026-10-18',year=2026,kind='manual',access='Passage inspected',domains=['T05'],role='primary',reason='Original checked migration, adapter and independent evidence contract.',prerequisites=['PR-T05-01'],assignable=True,locators=LOCATORS,inspection=dict(date='2026-10-18',method='Read full authored core, old producer/consumer fixtures, tests and independent oracle',scope='; '.join(LOCATORS),limits='Bounded author support; no independent learner, human or target assessment.',local_sha256=sha(ROOT/contract))))
 dump(ROOT/BASE/'migration_resources.json',dict(resources=records))
 passages={contract:LOCATORS,'assessor/language_portability/migration/migration.c':['All operation bodies and four semantic controls'],PUBLIC+'/old_sum.c':['Complete identifier-list checked sum'],PUBLIC+'/old_consumer.c':['Unprototyped admitted calls'],PUBLIC+'/promotion_consumer.c':['Float argument in each declaration mode'],PUBLIC+'/negative.c':['Four compile-only diagnostic or cast specimens']}
 dump(ROOT/BASE/'migration_research_gate.json',dict(status='passage-inspected-author-only',resources=BASE+'/migration_resources.json',input_sha256={p:sha(ROOT/p) for p in passages},passages=passages,external_source_sha256={r['inspection']['download_path']:r['inspection']['download_sha256'] for r in records if 'download_path' in r['inspection']},inspection=dict(date='2026-10-18',scope='Exact named draft and original-source passages',limits='Published C90/C17/C23 still inaccessible; no physical or human-independent review.')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T05-01.json';d=read_json(path)
 assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/PUBLIC).iterdir() if p.is_file() and p.name!='campaign.json']
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[contract]))
 binding=dict(path=BASE+'/migration_completion_dispositions.json',scope='Exactly six T05.02 ordinary/delayed tasks; other project criteria remain required. Native C17 core plus isolated C90/C23 comparison; no human/learner/physical claim.')
 for c in d['criteria']:
  if c['objective'].startswith('T05.02.'):
   c['successor_support']={**binding,'criterion_ids':[c['id']]+[v['id'] for v in c['variants']]}
 d['source_records']=[r for r in d['source_records'] if r['id'] not in {q['id'] for q in records}]+[dict(id=r['id'],path=BASE+'/migration_resources.json',passages=r['locators']) for r in records]
 dump(path,d);print(dict(attached=6,assets=len(assets)))

if __name__=='__main__':main()
