"""Changed-PHY/routing transfer and wrong-layer refusals, with synthetic data only."""
import copy,importlib.util,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3];p=ROOT/'labs/peripheral_depth/diagnostic/evidence.py'
spec=importlib.util.spec_from_file_location('dg_evidence',p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
current={k:k+'-v1' for k in set().union(*(r[2] for r in m.CLAIMS.values()))}
capture=dict(source_passages='Synthetic source identity for checker only',raw_capture_sha256='a'*64,instrument_setup='Synthetic 100MHz/1GSps placeholder, not actual measurement',uncertainty='Synthetic +/-1us example',reset_state='Synthetic reset asserted',isolation='Synthetic bench-only route',failure_control='Synthetic held-reset trace')
record=dict(record_kind='synthetic-fixture',evidence=[dict(id=key,claim=key,column=c,environment=sorted(env)[0],dependencies={k:current[k] for k in dep},observation='Synthetic '+key+' observation',capture=copy.deepcopy(capture)) for key,(c,env,dep) in m.CLAIMS.items()])
r=m.inspect(record,current);assert r['passed'] and len(r['still_applicable'])==7 and not r['physical_claims_requiring_authenticity_review'] and not r['physical_validation']
changed=dict(current,phy='replacement-PHY-v2');r=m.inspect(record,changed);assert r['passed'] and {x['id'] for x in r['invalidated']}=={'mdio_board','phy_link'}
changed=dict(current,routing='new-board-routing');r=m.inspect(record,changed);assert {x['id'] for x in r['invalidated']}=={'dma_board','mdio_board','phy_link'}
controls=[]
for kind in ['wrong-column','host-phy','missing-dependency','missing-capture','missing-raw','duplicate','unknown','missing-observation','empty','missing-column']:
 d=copy.deepcopy(record);row=next(x for x in d['evidence'] if x['id']=='phy_link')
 if kind=='wrong-column':row['column']='application'
 elif kind=='host-phy':row['environment']='host'
 elif kind=='missing-dependency':row['dependencies'].pop('phy')
 elif kind=='missing-capture':row['capture']={}
 elif kind=='missing-raw':row['capture']['raw_capture_sha256']=''
 elif kind=='duplicate':d['evidence'].append(copy.deepcopy(row))
 elif kind=='unknown':row['claim']='universal_conformance'
 elif kind=='missing-observation':row['observation']=''
 elif kind=='empty':d['evidence']=[]
 else:d['evidence']=[r for r in d['evidence'] if r['column']!='application']
 errors=m.inspect(d,current)['errors'];assert errors,kind;controls.append(dict(kind=kind,errors=errors))
print(json.dumps(dict(passed=True,synthetic_only=True,claims=7,controls=controls),indent=2))
