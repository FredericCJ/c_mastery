"""One-time immutable v2 identity snapshot from the accepted Git commit."""
import hashlib
import json
import subprocess
from common import ROOT, dump

BASE='40bf1b2'
def git(*args): return subprocess.check_output(['git',*args],cwd=ROOT).decode('utf-8-sig')
def main():
    target=ROOT/'curriculum/baseline_v2.json'
    if target.exists(): raise SystemExit('Baseline exists; do not regenerate historical identities')
    domains=[]
    for name in ['taxonomy.json','enhancement_taxonomy.json','performance_debugging_taxonomy.json','v2_taxonomy.json']:
        domains+=json.loads(git('show',BASE+':curriculum/'+name))['domains']
    objectives={o['id']:o['text'] for d in domains for t in d['subtopics'] for o in t['objectives']}
    paths=git('ls-tree','-r','--name-only',BASE,'--','curriculum/projects').splitlines()
    projects={}
    for path in paths:
        raw=subprocess.check_output(['git','show',BASE+':'+path],cwd=ROOT)
        projects[path.rsplit('/',1)[-1].removesuffix('.md')]=hashlib.sha256(raw).hexdigest()
    baseline={'commit':git('rev-parse',BASE).strip(),'domains':[d['id'] for d in domains],
              'topics':[t['id'] for d in domains for t in d['subtopics']],
              'objectives':objectives,'projects':projects}
    dump(target,baseline)
    dump(ROOT/'curriculum/migration_v3.json',{'baseline_commit':baseline['commit'],
        'objectives':[{'old_id':key,'new_ids':[key],'disposition':'retained','old_text':value,
                       'rationale':'Original assessable identity retained; new OS route adds separate advanced claims.'}
                      for key,value in objectives.items()],
        'projects':[{'old_id':key,'new_ids':[key],'disposition':'retained','baseline_sha256':value,
                     'rationale':'Accepted project contract preserved; advanced implementation work has new project IDs.'}
                    for key,value in projects.items()]})
    print(hashlib.sha256(target.read_bytes()).hexdigest())
if __name__=='__main__': main()
