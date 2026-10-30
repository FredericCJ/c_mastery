"""Freeze only completed P6 producers and concrete retained-source registrations."""
import argparse,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_numeric_followthrough.author_support import OPEN,HERE,PROJECTS,public_paths,records,write
from run_numeric_followthrough import PROVIDERS,sha
from audit_numeric_followthrough import pins,fixtures
def main():
 p=argparse.ArgumentParser()
 for n in ('correctness','measurements','exports','audit'):p.add_argument('--'+n,required=True)
 a=p.parse_args();components={};paths=set(public_paths())
 for n in ('correctness','measurements','exports','audit'):
  path=getattr(a,n);d=json.loads((ROOT/path).read_text());assert d['passed'] and d['error'] is None;pins(d['source_sha256']);pins(d['artifacts']);paths.update(d['source_sha256']);components[n]=dict(path=path,sha256=sha(ROOT/path))
 prior=ROOT/HERE/'preserved_inputs.json'
 if not prior.exists():write(prior,{'records':records(),'scope':'Exact current pre-successor criterion/provider objects; historical baseline remains separately guarded.'})
 registration='research/v5/expansion/legacy_completion/resources.json';old=json.loads((ROOT/registration).read_text());resources=dict(schema_version=1,retained_registration=dict(path=registration,sha256=sha(ROOT/registration),passages=[dict(id=x['id'],url=x['url'],locators=x['locators'],scope=x.get('inspection',{}).get('scope')) for x in old['resources'] if x['id'] in ('R-LCOMP-AVX','R-LCOMP-GCC-ATTR','R-LCOMP-GCC-CPU','R-LCOMP-LINUX-CLOCK','R-LCOMP-QEMU')],scope='Preserved original selected passage registration only; no fresh complete published-standard or full-manual claim.'),frozen_provider_sha256={p:sha(ROOT/p) for p in PROVIDERS},retained_numeric_receipt=dict(path='reviews/v5/expansion/legacy_completion/checkpoint_numeric.json',sha256=sha(ROOT/'reviews/v5/expansion/legacy_completion/checkpoint_numeric.json')),oracle_scope='Actual bounded fir-chunks/Fraction and summary-arithmetic branches inspected and recomputed; host tools not target dependencies.',limits=OPEN)
 write(ROOT/HERE/'resources.json',resources)
 for base in ('assessor/numeric_followthrough',HERE):paths|={p.relative_to(ROOT).as_posix() for p in (ROOT/base).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='support.json'}
 paths|={'tools/'+p for p in ('run_numeric_followthrough.py','measure_numeric_followthrough.py','export_numeric_followthrough.py','audit_numeric_followthrough.py','close_numeric_followthrough.py','check_numeric_followthrough.py')}
 from check_numeric_followthrough import COVERAGE,validate_campaign
 result=dict(passed=True,components=components,source_sha256={p:sha(ROOT/p) for p in sorted(paths)},coverage=COVERAGE,fixtures=fixtures(),open_gates=OPEN,scope='All8 exact P6 ordinary and8 delayed tasks, with retained provider contracts, concrete companion and separate independent gates.');validate_campaign(result);dest=ROOT/'reviews/v5/expansion/legacy_numeric_followthrough/campaign.json';assert not dest.exists();write(dest,result);print(json.dumps(dict(passed=True,sources=len(paths))))
if __name__=='__main__':main()
