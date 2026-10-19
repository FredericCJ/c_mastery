"""Pure additive exact delayed-criterion attachment and evidence producer."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
CARD='curriculum/v5/expansion/legacy_assessments/PR-T43-10.json'
OLD='research/v5/expansion/legacy_performance_followthrough/support.json'
SUPPORT='research/v5/expansion/performance_delegated/completion_dispositions.json'
ID='AX-PR-T43-10-T43.11.a-DELAYED'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
def public_paths():return sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/performance_delegated').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
def attach(record):
    if record['project']!='PR-T43-10':return record
    contract='labs/performance_delegated/CONTRACTS.md'
    if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    for p in public_paths():
        if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
    support=record.setdefault('additional_successor_support',[]);support[:]=[r for r in support if r.get('path')!=SUPPORT];support.append({'path':SUPPORT,'criterion_ids':[ID],'scope':'Exact delayed prepared-delegation observation of the existing T43 workload; historical read-only campaign and all other original dimensions retained.'});support.sort(key=lambda r:r['path'])
    source={'id':'R-PERF-DELEGATED-CGROUP','path':'research/v5/expansion/performance_delegated/resources.json','passages':json.loads((HERE/'resources.json').read_text())['resources'][0]['locators']}
    record['source_records']=[r for r in record['source_records'] if r['id']!=source['id']]+[source]
    return record
def apply():
    p=ROOT/CARD;before=p.read_bytes();history=HERE/'history';history.mkdir(exist_ok=True)
    if not (history/'original_card.json').exists():(history/'original_card.json').write_bytes(before)
    old=ROOT/OLD
    if not (history/'original_support.json').exists():(history/'original_support.json').write_bytes(old.read_bytes())
    d=json.loads(before);criteria=copy.deepcopy(d['criteria']);new=attach(d);assert new['criteria']==criteria;put(p,new);print(json.dumps({'attached':CARD,'original_criteria_unchanged':True}))
def close(campaign,exports,guard):
    sys.path.insert(0,str(ROOT/'tools'));from check_performance_delegated import check
    d=json.loads((ROOT/campaign).read_text());check(d);e=json.loads((ROOT/exports).read_text());g=json.loads((ROOT/guard).read_text());assert e['passed'] and g['passed'] and g['report_sha256']==sha(ROOT/campaign)
    old=json.loads((ROOT/OLD).read_text());historical=json.loads((HERE/'history/original_support.json').read_text());assert old['updates']==historical['updates']
    before=json.loads((HERE/'history/original_card.json').read_text());card=json.loads((ROOT/CARD).read_text());assert before['criteria']==card['criteria'] and attach(copy.deepcopy(card))==card
    criterion=next(c for c in card['criteria'] if any(v['id']==ID for v in c['variants']));variant=next(v for v in criterion['variants'] if v['id']==ID)
    row=copy.deepcopy(next(r for r in old['updates'] if r['id']==ID));prior=copy.deepcopy(row)
    row['dimensions']['teaching']={'state':'complete-for-exact-prepared-delegation-transfer','evidence':['labs/performance_delegated/CONTRACTS.md','labs/performance_delegated/fixtures.json']}
    row['dimensions']['implementation']={'state':'complete-original-workload-prepared-unprivileged-guest-adapter','evidence':['assessor/performance_delegated/delegated.c','labs/performance_followthrough/observations.c','labs/performance_delegated/guest.py']}
    row['dimensions']['author_validation']={'state':'passed-two-compiler-actual-prepared-guest-transfer-with-negative-controls','evidence':[campaign,exports,guard]}
    row.update(closure_rationale='The unchanged default T43 workload executes after exact PID placement and acknowledgment in a prepared UID1000 guest delegation. Unlimited and20ms/100ms quota trials retain raw accounting/ancestors, original ten observations, restoration and cleanup. Actual omitted-placement/restoration mutants fail. This supersedes only the old no-selected-subtree condition for this delayed author task.',required_environment='Explicit private Linux7.0.14 kernel and static BusyBox, networkless diskless QEMU TCG, GCC/Clang C17 with static Linux libraries; no WSL root or host controller/security change needed.',external_blockers=['Independent review and original minimum-seven-day unaided learner assessment remain unobserved.','Hardware PMU/physical board/counter/energy/WCET evidence remains separate.'],mechanism_checks=['exact-workload-and-source-identity','uid1000-authority-boundary','matched-workload-PID-and-sibling-supervisor','unlimited-and-CPU-quota','keyed-raw-counter-deltas','quota-restored-and-groups-removed','compiled-placement-and-restoration-controls','unsolved-exported-learner-refusal'],evidence_limits=['Actual guest-kernel enforcement is not physical PMU observation.','PSI files absent with ENOENT/null; memory.peak historical, tmpfs not physical device I/O.','Finite two-profile author execution does not assess an independent delayed learner.'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'])
    pins=set(d['input_sha256'])|set(e['source_sha256'])|{campaign,exports,guard,CARD,OLD,'tools/check_performance_delegated.py','tools/check_performance_delegated_package.py','assessor/performance_delegated/ANALYSIS.md'}
    pins.update(p.relative_to(ROOT).as_posix() for p in HERE.rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.name!='completion_dispositions.json')
    result={'schema_version':1,'required':True,'scope':'Only the original T43.11.a delayed author observation; no other criterion or external dimension promotion','updates':[row],'original_criterion':criterion,'original_variant':variant,'predecessor_row':prior,'predecessor_support':OLD,'current_reports':[campaign,exports,guard],'source_sha256':{p:sha(ROOT/p) for p in sorted(pins)},'remaining_external_work':row['external_blockers'],'new_c_execution':True,'source_records':card['source_records'][-1:]};put(ROOT/SUPPORT,result);print(json.dumps({'updates':1,'support':SUPPORT,'sha256':sha(ROOT/SUPPORT)}))
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--attach',action='store_true');p.add_argument('--campaign');p.add_argument('--exports');p.add_argument('--guard');a=p.parse_args();apply() if a.attach else close(a.campaign,a.exports,a.guard)
