"""Check the exact fresh source identities and bounded inspection assertions."""
import argparse,copy,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];BASE=ROOT/'research/v5/expansion/systems_book_followthrough'
def read(p):return json.loads(p.read_text('utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def require(ok,message):
 if not ok:raise ValueError(message)
def inspect(i,r,g):
 from record_systems_books import assignments
 scope=read(BASE/'assignment_scope.json');require(scope==assignments(),'complete canonical assignment scope');require(len(scope['assignments'])==53 and len(scope['book_assignments'])==1 and scope['book_assignments'][0]['id']=='R-SYS-CSAPP-GLOBAL-3E','actual book assignment identity')
 original=read(ROOT/'research/contributions/systems_resources.json');original=original.get('resources',[]) if isinstance(original,dict) else original
 require(len(i['inspections'])==len(r['resources'])==2,'two exact held assignments')
 expected={'R-SYS-CSAPP-GLOBAL-3E':list(range(1,5))+list(range(707,712))+list(range(725,731))+list(range(734,738)),'R-SYS-TLPI-2010':[5,6]+list(range(91,100))+list(range(675,680))+list(range(687,693))+list(range(1064,1068))}
 for x in i['inspections']:
  old=next(t for t in original if t['id']==x['original_resource']);fresh=next(t for t in r['resources'] if t['id']==x['resource_id']);p=pathlib.Path(x['path'])
  require(p.is_relative_to(pathlib.Path('F:/Knowledge/Library/Books')) and p.is_file(),'ordinary selected book path')
  require(sha(p)==x['sha256']==old['inspection']['local_sha256']==fresh['inspection']['local_sha256'],'source identity')
  require(hashlib.sha256(json.dumps(old,sort_keys=True,ensure_ascii=False).encode()).hexdigest()==x['source_record_sha256'],'historical record identity')
  require(x['physical_pages']==expected[x['original_resource']] and set(x['page_text_sha256'])==set(map(str,x['physical_pages'])),'exact bounded inspected pages')
  require(fresh['locators']==old['locators'] and fresh['inspection']['date']=='2026-10-18','exact locators/date')
  require(fresh['inspection']['limits']=='Text-only bounded author inspection, not full-book, diagram-layout, current implementation, final ISO edition, execution or independent-review evidence. Old inspection preserved.','scope limits')
 for p,h in i['library_input_sha256'].items():
  path=pathlib.Path(p);path=path if path.is_absolute() else pathlib.Path('F:/Knowledge')/path;require(path.is_file() and sha(path)==h,'library pointer/manifest pin')
 require(g['status']=='passage-inspected','gate state')
 for p,h in g['research_input_sha256'].items():require(sha(ROOT/p)==h,'research gate pin')
 return {'resources':2,'physical_pages':45,'source_files':2,'historical_records_preserved':True}
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');ap.add_argument('--report',required=True);args=ap.parse_args();dest=ROOT/args.report;require(dest.resolve().is_relative_to((ROOT/'reviews/v5/expansion/systems_book_followthrough').resolve()) and not dest.exists(),'new scoped report required');i=read(BASE/'inspection.json');r=read(BASE/'resources.json');g=read(BASE/'research_gate.json');summary=inspect(i,r,g);controls=[]
 if args.self_test:
  for name in ['wrong-bytes','wrong-pages','missing-book','wrong-locator','inflated-inspection','changed-history','stale-library','stale-gate']:
   a,b,c=copy.deepcopy((i,r,g))
   if name=='wrong-bytes':a['inspections'][0]['sha256']='0'*64
   elif name=='wrong-pages':a['inspections'][0]['physical_pages'].append(738)
   elif name=='missing-book':a['inspections'].pop()
   elif name=='wrong-locator':b['resources'][0]['locators']=['entire book']
   elif name=='inflated-inspection':b['resources'][0]['inspection']['limits']='independent mastery established'
   elif name=='changed-history':a['inspections'][0]['source_record_sha256']='0'*64
   elif name=='stale-library':a['library_input_sha256'][next(iter(a['library_input_sha256']))]='0'*64
   else:c['research_input_sha256'][next(iter(c['research_input_sha256']))]='0'*64
   try:inspect(a,b,c);rejected=False
   except ValueError:rejected=True
   require(rejected,'accepted false source record '+name);controls.append(dict(control=name,rejected=True))
 paths=['tools/record_systems_books.py','tools/check_systems_books.py','research/contributions/systems_resources.json',*[p.relative_to(ROOT).as_posix() for p in BASE.glob('*') if p.is_file()]];report=dict(schema_version=1,passed=True,scope='Exact source identity and bounded inspection guard, not execution or independent content review',summary=summary,negative_controls=controls,source_sha256={p:sha(ROOT/p) for p in paths},external_source_sha256=i['external_source_sha256']);dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(summary|dict(controls=len(controls),passed=True)))
if __name__=='__main__':main()
