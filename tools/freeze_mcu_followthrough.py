"""Compose immutable T50 receipts without rewriting historical attempts."""
import argparse,hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--audit',required=True);p.add_argument('--exports',required=True);a=p.parse_args()
 from research.v5.expansion.legacy_mcu_followthrough.author_support import OPEN
 components={'mechanisms':'reviews/v5/expansion/legacy_mcu_followthrough/campaign_attempt4/result.json','audit':a.audit,'exports':a.exports}
 for path in components.values():assert json.loads((ROOT/path).read_text())['passed'],path
 paths=[f for directory in ('labs/mcu_followthrough','assessor/mcu_followthrough','research/v5/expansion/legacy_mcu_followthrough') for f in (ROOT/directory).rglob('*') if f.is_file() and '__pycache__' not in f.parts and f.name!='support.json']
 paths += list((ROOT/'tools').glob('*mcu_followthrough*.py'))
 result={'schema_version':1,'passed':True,'components':{k:{'path':v,'sha256':sha(ROOT/v)} for k,v in components.items()},'source_sha256':{f.relative_to(ROOT).as_posix():sha(f) for f in paths},'coverage':{'original_criteria':36,'delayed_variants':36,'mechanism_commands':119,'supplement_commands':12,'isolated_packages':12,'export_commands':96},'open_gates':OPEN,'historical_attempts':[{'path':'reviews/v5/expansion/legacy_mcu_followthrough/campaign_attempt1/result.json','disposition':'Inconclusive original EXC_RETURN mutant deadline; no detection credit. Reserved Handler+PSP mutant in current campaign reaches exact0x72 instead.'},{'path':'reviews/v5/expansion/legacy_mcu_followthrough/export_attempt1/result.json','disposition':'Original incomplete cmx warning failure; additive unused-parameter-only scaffold registered without editing original.'},{'path':'reviews/v5/expansion/legacy_mcu_followthrough/export_attempt2/result.json','disposition':'Empty learner BSS loader failure; current general builder omits absent RAM sections.'}],'scope':'Author source-bound model/native/emulator validation; external learner,review and physical observations distinct.'}
 (ROOT/'reviews/v5/expansion/legacy_mcu_followthrough/campaign.json').write_text(json.dumps(result,indent=2)+'\n');print('Frozen campaign metadata')
if __name__=='__main__':
 import sys;sys.path.insert(0,str(ROOT));main()
