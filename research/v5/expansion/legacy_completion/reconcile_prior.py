"""Link retained depth snapshots to exact later criteria without declaring execution/mastery."""
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[4]

def main():
    sources={};by_objective=defaultdict(list);by_id={}
    def read(path):
        p=ROOT/path;sources[path]=hashlib.sha256(p.read_bytes()).hexdigest()
        return json.loads(p.read_text(encoding='utf-8'))
    for p in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json')):
        path=p.relative_to(ROOT).as_posix();value=read(path)
        for index,criterion in enumerate(value['criteria']):
            item={'criterion':criterion['id'],'project':value['project'],'path':path,
                  'pointer':'/criteria/'+str(index),'assessor_anchor':criterion['assessor_anchor'],
                  'scope':'Authored criterion/reference analysis; execution and independent assessment require their own source-bound evidence.'}
            by_id[criterion['id']]=item
            if value['domain']!='INTEGRATION':by_objective[criterion['objective']].append(item)
    rows=[]
    for path,key in [('curriculum/v5/expansion/b02_depth_disposition.json','objectives'),
                     ('curriculum/v5/expansion/b03_depth_disposition.json','objectives'),
                     ('curriculum/v5/expansion/b04_depth_audit.json','leaves'),
                     ('curriculum/v5/expansion/b05_depth_audit.json','leaves'),
                     ('research/v5/expansion/legacy_checkpoints/objective_dispositions.json','rows')]:
        value=read(path)
        for index,old in enumerate(value[key]):
            objective=old['objective'];integration=key=='rows'
            if integration and old.get('criterion'):
                successors=[by_id[old['criterion']]]
                disposition='selected-checkpoint-criterion-authored'
            else:
                successors=by_objective[objective]
                disposition='retained-domain-obligation-not-checkpoint-coverage' if integration else 'later-domain-criterion-authored'
            if not successors:raise ValueError('Missing exact successor '+path+':'+objective)
            rows.append({'original_source':path,'original_pointer':'/'+key+'/'+str(index),
                         'original_id':objective,'original_project':old.get('project'),
                         'original_disposition':old.get('disposition',old.get('depth_review')),
                         'current_authoring_disposition':disposition,'successors':successors,
                         'implementation_execution':'not-inferred-from-this-mapping',
                         'learner_assessment':'not-inferred','external_or_physical':'not-inferred'})
    result={'schema_version':1,'purpose':'Successor links for historical depth snapshots. Original files are preserved byte-for-byte.',
            'method':'Exact objective/criterion ID resolution only; no new passage inspection, runtime execution or coverage acceptance.',
            'source_sha256':sources,'counts':dict(Counter(x['original_source'] for x in rows)),'rows':rows}
    path=ROOT/'research/v5/expansion/legacy_completion/prior_disposition_successors.json'
    path.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'rows':len(rows),'counts':result['counts']}))

if __name__=='__main__':main()
