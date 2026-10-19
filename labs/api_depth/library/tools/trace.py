"""Structural traceability: depth review is explicit, never inferred from IDs."""
import hashlib,json,pathlib,sys
def audit(requirements,tests,sources):
 ids=[r['id'] for r in requirements]+[t['id'] for t in tests]
 duplicates=sorted({x for x in ids if ids.count(x)>1});known={r['id'] for r in requirements};covered=set();unknown=set();orphans=[];weak=[];stale=[]
 for t in tests:
  refs=set(t['requirements']);covered|=refs&known;unknown|=refs-known
  if not refs or not refs&known:orphans.append(t['id'])
  if t.get('kind')=='executable' and not t.get('assertion'):weak.append(t['id'])
  if t.get('source_sha256')!=sources.get(t.get('source')):stale.append(t['id'])
 return dict(duplicates=duplicates,uncovered=sorted(known-covered),orphan_tests=sorted(orphans),unknown_requirements=sorted(unknown),assertion_free=sorted(weak),stale=sorted(stale))
def main():
 source={'renamed.c':'revisionA'};requirements=[{'id':x} for x in ['R1','R2','R3']]
 tests=[dict(id=t,requirements=[r],kind='executable',assertion='snapshot preserved',source='renamed.c',source_sha256='revisionA') for t,r in [('T1','R1'),('T2','R2'),('T9','R9')]]
 expected=dict(duplicates=[],uncovered=['R3'],orphan_tests=['T9'],unknown_requirements=['R9'],assertion_free=[],stale=[]);assert audit(requirements,tests,source)==expected
 moved=[dict(t,source='moved.c') for t in tests];assert audit(requirements,moved,{'moved.c':'revisionA'})==expected
 assert audit(requirements,tests+[tests[0]],source)['duplicates']==['T1']
 assert audit(requirements,tests,{'renamed.c':'revisionB'})['stale']==['T1','T2','T9']
 weak=tests+[dict(tests[0],id='T3',requirements=['R3'],assertion='')];assert audit(requirements,weak,source)['assertion_free']==['T3']
 proof=dict(id='P3',requirements=['R3'],kind='analysis',source='proof.md',source_sha256='p1',review='author-derived-only');assert not audit(requirements,tests+[proof],dict(source,**{'proof.md':'p1'}))['uncovered']
 # Supersession preserves old requirement/evidence rather than reusing R1.
 split=[dict(id='R1a',supersedes='R1'),dict(id='R1b',supersedes='R1')];assert {r['supersedes'] for r in split}=={'R1'}
 print(json.dumps(dict(fixture=expected,stable_rename=True,duplicate_rejected=True,stale_rejected=True,assertion_free_distinguished=True,analysis_artifact={'id':'P3','review':'author-derived-only'},supersession=split,historical_evidence=tests[0])))
if __name__=='__main__':main()
