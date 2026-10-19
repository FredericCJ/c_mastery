"""Bind request successors only after current core and guest campaigns pass."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha,parse
from check_request_adapters import validate
from check_expansion_campaigns import pin_errors,execution_errors,special_errors
folder=Path(__file__).parent
campaign='reviews/v5/expansion/followthrough/request_adapter_campaign.json'
core='reviews/v5/expansion/integrated/b05_depth_campaign.json'
report=read_json(ROOT/campaign);parent=read_json(ROOT/core)
errors=validate(report)+pin_errors(parent)+execution_errors(parent)
errors+=special_errors('b05_depth',read_json(ROOT/'labs/b05_depth/family.json'),parent)
if not parent.get('passed') or parent.get('request_adapter_campaign',{}).get('sha256')!=sha(ROOT/campaign):errors.append('Parent must bind current request campaign')
if errors:raise SystemExit(str(errors))
exercise='X-T56-001';brief='curriculum/v5/exercises/'+exercise+'.md';meta,_=parse(ROOT/brief)
legacy_path='curriculum/v5/expansion/legacy_assessments/PR-T56-07.json';legacy=read_json(ROOT/legacy_path)
teaching=[brief,'labs/b05_depth/request_adapters/CONTRACTS.md','assessor/b05_depth/request_adapters/ANALYSIS.md']
implementation=['labs/b05_depth/request_adapters/guest.py','labs/b05_depth/request_adapters/user.c','labs/b05_depth/request_adapters/tests.c',
 'assessor/b05_depth/request_adapters/async.inc','assessor/b05_depth/request_adapters/request.c']
evidence=[core,campaign,'reviews/v5/expansion/followthrough/request_adapter_check.json']
rows=[];readiness=[]
def update(key,claim,limit=''):
    row={'id':key,'dimensions':{'teaching':{'state':'complete','evidence':teaching},
      'implementation':{'state':'complete','evidence':implementation},'author_validation':{'state':'passed','evidence':evidence},
      'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
      'physical_observation':{'state':'not-run; separate from the declared emulated device experiment','evidence':[]}},
      'closure_rationale':claim+' Author reference readiness only; the original seven-day independent attempt remains pending.',
      'required_environment':'C17 GCC/Clang on Linux; QEMU10.2.1 x86-64 TCG2/4CPUs with original synthetic4MiB file and modern virtio device.',
      'validation_method':'196 independent state-table cases in six host profiles, eight actual positive guest configurations, two actual error guests, four compiled kernel mutants and compiling unsolved scaffold; final ELF closure audit.',
      'evidence_limits':['No physical reset latency, real disk durability, universal interleaving proof or independent review/learner result.',limit]}
    rows.append(row)
    return row
update(exercise,'Original bounded request core plus complete reset/unknown-outcome and actual two-request virtio successor.')
for criterion in meta['rubric']:
    update(criterion['id'],criterion['criterion']+' Independent transition table, cancellation/timeout retained ownership, reset acknowledgement and exact-once matching completion assertions.')
    readiness.append({'id':criterion['id'],'objective':criterion['objective'],'exercise':exercise,'criterion':criterion['criterion'],
      'teaching':teaching,'implementation':implementation,'observations':evidence,'score':'not-assessed'})
for local,claim in [('V1','Resetting storage remains owned until trusted acknowledgement; reused request result is uncertain and generation survives.'),
                    ('V2','Explicit actual kernel imports, two DMA chains, blocked matching waiters, owner exit, two16-bit wraps, reset and actual EIO/retry.')]:
    update(exercise+'-'+local,claim)
claims={
 'T56.07.a':('Retained IRQ-and-used-progress policy has a four-case worked truth table; successor explicitly changes to used-ring polling. Actual observer hold accumulates two used entries and wakes both matching waiters.',
             'The original IRQ conjunction is a retained implementation policy, not a universal Virtio rule; changed polling policy is explicit.'),
 'T56.07.b':('Timeout and cancellation retain descriptor/buffer ownership; withheld reset observation rejects reuse, actual zero-status acknowledgement permits reinitialization with uncertain result.',
             'Withholding is an observer fault, not a claim QEMU reset was physically delayed.'),
 'T56.07.c':('Child write is copied before mutation/exit; dead-owner pending state is observed before consumption/reaping; stale ticket cannot consume reused slot;131074 real single-outstanding reads cross16-bit indices twice.',
             'Finite-generation exhaustion is injected in the independent portable table; not billions of guest admissions.')}
for criterion in legacy['criteria']:
    claim,limit=claims[criterion['objective']];update(criterion['id'],claim,limit)
    readiness.append({'id':criterion['id'],'objective':criterion['objective'],'project':legacy['project'],'criterion':criterion['task'],
       'verification_procedure':claim,'limit':limit,'teaching':teaching,'implementation':implementation,'observations':evidence,'score':'not-assessed'})
    for variant in criterion['variants']:update(variant['id'],variant['task']+' '+claim,limit)
support='research/v5/expansion/followthrough/request_objective_readiness.json'
support_data={'schema_version':1,'project':legacy['project'],'criteria':readiness,'dispositions':'research/v5/expansion/followthrough/request_adapter_dispositions.json',
 'input_sha256':{p:sha(ROOT/p) for p in sorted(set(teaching+implementation+evidence))},
 'historical_note':'Original criteria, baseline bindings and historical limitation prose remain intact. This additive source-bound successor describes current author support.'}
dump(ROOT/support,support_data)
dump(folder/'request_adapter_dispositions.json',{'schema_version':1,'updates':rows})
dump(folder/'request_adapter_gap_ledger.json',{'schema_version':1,'gaps':[{'id':exercise,'originating_record':brief,
 'requirement':'Both original required variants and exact legacy criteria; independent and physical dimensions remain separate.',
 'status':'author-implemented-validated; external-evidence-pending','support':support,'dispositions':'research/v5/expansion/followthrough/request_adapter_dispositions.json'}]})
legacy['successor_support']={'path':support,'criterion_ids':[r['id'] for r in legacy['criteria']],
 'scope':'Author-side implementation/analysis, actual guest execution and complete delayed-assessment package. No independent learner score.'}
for criterion in legacy['criteria']:criterion['successor_support']={'path':support,'criterion_id':criterion['id']}
dump(ROOT/legacy_path,legacy)
target=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(target);index={r['id']:r for r in state['updates']}
for row in rows:index[row['id']]=row
state['updates']=list(index.values());dump(target,state)
print({'closed_author_obligations':len(rows),'criterion_readiness':len(readiness),'independent_or_physical_claims':False})
