"""Keep early storage gates independent of later transaction/recovery APIs."""
import json
import re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
rows={
 1:(['codec','initial_image'],['format','corruption']),
 2:(['cache'],['corruption']),
 3:(['lookup'],['tree','differential']),
 4:(['insertion'],['tree','differential']),
 5:(['deletion'],['tree','differential']),
 6:(['transaction'],['format','capacity']),
 7:(['publish'],['crash']),
 8:(['checkpoint_order'],['checkpoint']),
 9:(['recovery','corruption','format','tree','capacity'],[]),
 10:(['crash','checkpoint','recovery','differential'],[]),
 11:(['file','corruption'],[]),12:(['vm'],[])}
assessor={7:['storage.mutation_no_commit_flush'],8:['storage.mutation_early_reclaim'],10:['storage.mutation_bad_separator']}
def main():
 family=json.loads((ROOT/'labs/storage/family.json').read_text(encoding='utf-8'))
 for i,(immediate,deferred) in rows.items():
  pid=f'PR-T47-{i:02}';tests=['storage.'+x for x in immediate];later=['storage.'+x for x in deferred]
  path=ROOT/f'curriculum/projects/{pid}.md';text=path.read_text(encoding='utf-8');match=re.match(r'^```json\s*(.*?)\s*```',text,re.S)
  meta=json.loads(match[1]);old=meta['tests'];meta['tests']=tests;meta['deferred_tests']=later;meta['assessor_tests']=assessor.get(i,[])
  body=text[match.end():].replace('`'+', '.join(old)+'`','`'+', '.join(tests)+'`')
  body=re.sub(r'\n## Progressive execution\n.*?(?=\n## )','',body,flags=re.S)
  paragraph='\n## Progressive execution\n\nThe immediate learner gate is '+', '.join('`'+x+'`'for x in tests)+'. '
  if i<=8:
   paragraph+='Early tests inject declared valid component states; they do not require a completed opener, recovery engine or later mutation API. '
  if later:
   paragraph+='Defer '+', '.join('`'+x+'`'for x in later)+' until PR-T47-09/10 integrates opening, durable publication and recovery. '
  if i in assessor:
   paragraph+='The separately named assessor mutation controls require explicit assessor source and are not learner checkpoint prerequisites. '
  paragraph+='Passes cover their finite mechanism only; retain the separate invariant, failure and transfer evidence.\n'
  body=body.replace('\n## Steps\n',paragraph+'\n## Steps\n',1)
  path.write_text('```json\n'+json.dumps(meta,indent=2)+'\n```'+body,encoding='utf-8')
  fixture_path=ROOT/f'fixtures/storage/{pid}.json';fixture=json.loads(fixture_path.read_text(encoding='utf-8'));fixture['tests']=tests;fixture['deferred_tests']=later
  fixture_path.write_text(json.dumps(fixture,indent=2)+'\n',encoding='utf-8')
  entry=next(p for p in family['projects']if p['id']==pid);entry.update(tests=tests,deferred_tests=later,assessor_tests=assessor.get(i,[]),scope='Immediate finite mechanism gate; deferred integration and independent objective evidence remain required.')
 family['tests']=['storage.'+x for x in ['codec','initial_image','cache','lookup','insertion','deletion','transaction','publish','checkpoint_order','format','tree','differential','crash','checkpoint','recovery','corruption','capacity','file']]
 family['assessor_tests']=['storage.mutation_no_commit_flush','storage.mutation_early_reclaim','storage.mutation_bad_separator']
 (ROOT/'labs/storage/family.json').write_text(json.dumps(family,indent=2)+'\n',encoding='utf-8')
if __name__=='__main__':main()
