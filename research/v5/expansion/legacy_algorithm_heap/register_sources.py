"""Register the exact primary heap and reused actual ABI passage readings."""
import copy,hashlib,json,pathlib,urllib.request
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 cache=pathlib.Path('/home/frede/cmastery-algorithm-heap-sources');cache.mkdir(exist_ok=True);html=cache/'ods-heap.html';url='https://opendatastructures.org/ods-cpp/10_1_Implicit_Binary_Tree.html'
 if not html.exists():html.write_bytes(urllib.request.urlopen(url,timeout=45).read())
 loc=['10.1 accessible lines9–46: array representation, minimum ordering and upward repair','10.1 accessible lines52–82: root replacement, complete swaps and downward repair','10.1 accessible lines88–102: height bound and separate resize amortization']
 rows=[dict(id='R-AH-ODS',title='Open Data Structures: BinaryHeap implicit binary tree',authors=['Pat Morin'],edition='Online C++ edition',year=2013,kind='primary-text',url=url,access='Actual listed bounded primary HTML passages inspected',domains=['T36'],role='primary',reason='Derive child indices, moving order violation, repair direction and height cost; explicitly contrast tie and capacity contracts',prerequisites=['C object lifetimes, array bounds and binary tree relations'],locators=loc,assignable=True,inspection=dict(date='2026-10-18',method='Actual primary page prose and complete displayed add/bubbleUp/remove/trickleDown code read',scope='; '.join(loc),limits='Only listed passages; C++ source tie choice differs from scalar laboratory left-tie rule. No whole-book or ISO C/ABI proof.',external_source_sha256={str(html):sha(html)}))]
 old=ROOT/'research/v5/expansion/legacy_algorithm_abi/resources.json';canonical=[]
 for row in json.loads(old.read_text())['resources']:
  if row['id'] in ['R-AB-SYSV','R-AB-ARM']:
   new=copy.deepcopy(row);new['id']=row['id'].replace('R-AB-','R-AH-');new['reason']='Actual structured-heap kernel and complete-swap observer preservation; same bounded passage inspection as preceding ABI package';rows.append(new);canonical.append(dict(path=old.relative_to(ROOT).as_posix(),id=row['id']))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows,canonical_external_sources=canonical),indent=2)+'\n',encoding='utf8');files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__),old]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='Exact three T36.06 original and changed author tasks, not independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Exact published final C17/C23 uninspected.','Only listed primary heap/ABI passages inspected; source tie contracts differ explicitly.','Finite author execution does not establish physical observation or independent learner mastery.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
