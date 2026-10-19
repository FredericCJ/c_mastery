"""Record fresh bounded ordinary-PDF inspection; never modify library or history."""
import copy,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];BASE=ROOT/'research/v5/expansion/systems_book_followthrough'
def read(p):return json.loads(p.read_text('utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,d):p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def assignments():
 registry={x['id']:x for x in read(ROOT/'references/resources.json')['resources']};out=[];pins={};books=[]
 for domain in ['T07','T08','T09','T10','T11']:
  paths=[ROOT/'curriculum/modules'/(domain+'.md'),*sorted((ROOT/'curriculum/projects').glob('PR-'+domain+'-*.md')),*sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-'+domain+'-*.json'))]
  for p in paths:
   name=p.relative_to(ROOT).as_posix();text=p.read_text('utf8');pins[name]=sha(p);ids=sorted(set(re.findall(r'R-[A-Za-z0-9-]+',text)) & registry.keys());rows=[dict(id=x,title=registry[x]['title'],kind=registry[x]['kind']) for x in ids];out.append(dict(path=name,references=rows))
   books.extend(dict(path=name,**x) for x in rows if x['kind'] in ['book','textbook'])
 return dict(scope='All five canonical modules, their24 parent project briefs and their24 exact supplemental cards; textual resource IDs resolved against current registry. Named historical TLPI assignment additionally inspected in full registered scope.',source_sha256=pins,assignments=out,book_assignments=books,limits=['Does not redefine prerequisite readings from earlier modules as newly assigned whole books.','Resource IDs in the resolved module/project/card sources are checked; no inferred assignment from bibliography membership.'])
def main():
 sys.path.insert(0,str(ROOT/'build/pdf_reader_deps'));from pypdf import PdfReader
 lookup=read(ROOT/'build/library_systems_lookup.json');old=read(ROOT/'research/contributions/systems_resources.json');old=old.get('resources',[]) if isinstance(old,dict) else old
 selected=[('R-SYS-CSAPP-GLOBAL-3E','R-SYS-BOOK-CSAPP-REINSPECT','3a8c',list(range(1,5))+list(range(707,712))+list(range(725,731))+list(range(734,738)),0),('R-SYS-TLPI-2010','R-SYS-BOOK-TLPI-REINSPECT','f68a',[5,6]+list(range(91,100))+list(range(675,680))+list(range(687,693))+list(range(1064,1068)),44)]
 resources=[];inspections=[];external={}
 for oldid,newid,prefix,pages,offset in selected:
  original=next(x for x in old if x['id']==oldid);record=next(x for x in lookup['records']['files.jsonl'] if x['file_id'].startswith(prefix));path=pathlib.Path('F:/Knowledge/Library')/record['path_relative'];assert sha(path)==record['sha256']==original['inspection']['local_sha256'];external[path.as_posix()]=sha(path);pdf=PdfReader(path)
  digests={str(n):hashlib.sha256(pdf.pages[n-1].extract_text().encode()).hexdigest() for n in pages}
  item=copy.deepcopy(original);item['id']=newid;item['inspection']={'date':'2026-10-18','method':'Read exact complete named page text extracted privately with pypdf after matching held PDF and pinned read-only catalog hashes; title/imprint checked directly','scope':'; '.join(original['locators'])+'; title/imprint pages; physical-page offset '+str(offset),'limits':'Text-only bounded author inspection, not full-book, diagram-layout, current implementation, final ISO edition, execution or independent-review evidence. Old inspection preserved.','local_document_id':record['file_id'],'local_sha256':record['sha256']};item['reason']='Versioned fresh inspection of the retained assignment, interpreted through comparison.md and actual selected implementation contracts.';resources.append(item)
  inspections.append(dict(resource_id=newid,original_resource=oldid,path=path.as_posix(),sha256=sha(path),physical_pages=pages,printed_page_offset=offset,page_text_sha256=digests,source_record_sha256=hashlib.sha256(json.dumps(original,sort_keys=True,ensure_ascii=False).encode()).hexdigest()))
 inspection=dict(schema_version=1,scope='Fresh bounded passage inspection; all library and historical source bytes unchanged',actor='/root/author_batch',date='2026-10-18',library_read_only_scope='Two exact ordinary Books records; no _old or opaque-package traversal, extraction or mutation',library_input_sha256=lookup['pins'],external_source_sha256=external,inspections=inspections,limitations=['No raw text or images committed.','No new full-book, visual-layout, final-standard or independent-review claim.','F:/Knowledge is the current location of the historical F:/Cours catalog paths.'])
 put(BASE/'inspection.json',inspection);put(BASE/'resources.json',dict(schema_version=1,resources=resources))
 put(BASE/'assignment_scope.json',assignments());files=['resources.json','inspection.json','comparison.md','assignment_scope.json'];put(BASE/'research_gate.json',dict(status='passage-inspected',domains=['T07','T08','T09','T10','T23','T24','T25'],research_input_sha256={(BASE/f).relative_to(ROOT).as_posix():sha(BASE/f) for f in files},limitations=inspection['limitations']))
 print(json.dumps(dict(resources=len(resources),physical_pages=sum(len(x['physical_pages']) for x in inspections))))
if __name__=='__main__':main()
