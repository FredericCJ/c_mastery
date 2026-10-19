"""Check additive source metadata repair against retained exact input bytes."""
import argparse,copy,json
from pathlib import Path
from repair_resource_schema_v5 import ROOT,HERE,read,sha,put,edition

def validate_change(change, current):
    expected=copy.deepcopy(change['old'])
    for field,value in change['added_fields'].items():
        if field=='edition':
            assert 'edition' not in expected and value==edition(expected)
            expected[field]=value
        elif field=='inspection.scope':
            assert not expected['inspection'].get('scope') and value==expected['locators']
            expected['inspection']['scope']=copy.deepcopy(value)
        elif field in {'inspection.limits','inspection.method'}:
            key=field.split('.')[1];assert not expected['inspection'].get(key) and isinstance(value,str) and value
            expected['inspection'][key]=value
        else:raise AssertionError('Unexpected source-field repair')
    assert current==expected,'Repair changed an unapproved source field'
    assert all(current['inspection'].get(k) for k in ('date','method','scope','limits'))
    return True

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--output',required=True);args=ap.parse_args();dest=ROOT/args.output;assert not dest.exists()
    data=read(HERE/'schema_normalization.json');history=HERE/'history_schema_01';manifest=read(history/'manifest.json')['sha256'];pins={};controls=[];registers=set()
    for path,digest in manifest.items():assert sha(history/path)==digest
    for row in data['changes']:
        path=row['path'];registers.add(path);old=next(x for x in read(history/path)['resources'] if x['id']==row['id']);assert old==row['old']
        now=next(x for x in read(ROOT/path)['resources'] if x['id']==row['id']);validate_change(row,now);pins[path]=sha(ROOT/path)
        gate=Path(path).with_name('research_gate.json').as_posix()
        for p,h in read(ROOT/gate)['research_input_sha256'].items():assert sha(ROOT/p)==h;pins[p]=h
        pins[gate]=sha(ROOT/gate)
        for p,h in now['inspection'].get('source_sha256',{}).items():assert sha(ROOT/p)==h;pins[p]=h
    sample=next(x for x in data['changes'] if 'inspection.scope' in x['added_fields'])
    for name in ('eligibility','locator','scope','date','capture','edition'):
        v=copy.deepcopy(sample['new'])
        if name=='eligibility':v['assignable']=not v['assignable']
        elif name=='locator':v['locators'].append('unread passage')
        elif name=='scope':v['inspection']['scope'].append('whole standard')
        elif name=='date':v['inspection']['date']='future'
        elif name=='capture':v['inspection']['source_sha256']={}
        else:v['edition']='published C17'
        try:validate_change(sample,v)
        except AssertionError:controls.append(dict(name=name,rejected=True))
        else:raise AssertionError('Control accepted: '+name)
    # Exact original criterion objects and baseline contracts remain untouched.
    cards=0
    for path in manifest:
        if not path.startswith('curriculum/v5/expansion/legacy_assessments/'):continue
        old=read(history/path);now=read(ROOT/path)
        if 'criteria' not in old:continue
        assert old['criteria']==now['criteria'] and old['baseline_binding']==now['baseline_binding'],path
        cards+=1
    for p in ['tools/repair_resource_schema_v5.py','tools/check_resource_schema_repair.py','research/v5/expansion/resource_merge_repair/schema_normalization.json']:
        pins[p]=sha(ROOT/p)
    result=dict(passed=True,records=len(data['changes']),registers=len(registers),original_cards_preserved=cards,negative_controls=controls,input_sha256=pins,scope='Additive canonical source metadata only; prior eligibility/locators/source inspection and execution claims unchanged. Package export and envelope refresh remains separately required.')
    put(dest,result);print(json.dumps({k:v for k,v in result.items() if k!='input_sha256'}))

if __name__=='__main__':main()
