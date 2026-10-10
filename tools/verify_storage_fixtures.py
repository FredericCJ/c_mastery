"""Check the storage contract index. Actual C evidence is a separate runner result."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
def main():
    files=sorted((ROOT/'fixtures/storage').glob('PR-T47-*.json'))
    assert len(files)==12
    family=json.loads((ROOT/'labs/storage/family.json').read_text(encoding='utf-8'))
    mapped={p['id']:p for p in family['projects']}
    for file in files:
        x=json.loads(file.read_text(encoding='utf-8'))
        assert x['project']==file.stem and x['project'] in mapped
        assert x['page_bytes']==4096 and x['image_pages']==2+2*x['data_pages']+1==67
        assert x['expected']['empty_root']==1 and x['expected']['empty_allocation_mask']==3
        assert x['expected']['old_map']=={'1':10,'2':20} and x['expected']['new_map']=={'1':11,'3':30}
        assert x['tests']==mapped[x['project']]['tests']
        assert x['expected']['physical_power_cut_proven'] is False
    print('12 storage contract fixtures valid; no C execution or hardware result is inferred')
if __name__=='__main__':main()
