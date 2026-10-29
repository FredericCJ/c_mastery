"""Bind each original T75 criterion and extension to its concrete, current support."""
import hashlib
import json
from pathlib import Path
from check_resilience_followthrough import CAMPAIGN,COST,SERVICE_COST,check_campaign,check_cost,check_service_cost
from resilience_cost_matrix import matrix

ROOT=Path(__file__).resolve().parents[1]
REPORTS=[CAMPAIGN,COST,SERVICE_COST]
SPECS={
1:dict(contract='SCOPE',parts=['scope'],test='foundation_test',oracle='followthrough',
       anchors=['resilience.deadline','resilience.scope_follow'],gap='IW-e9d7ccdacac08623',
       demonstration=[
           'Immutable parent owners and separate borrower acknowledgments; cancellation precedence and deadline nonextension are derived in SCOPE, including cancellation versus rollback.',
           'Actual allocated cells retain parents, refuse depth8/reference MAX/allocation cuts transactionally, and encode remaining duration plus transit bound in16 octets before rebasing to an unrelated clock.',
           'Two paused pthread borrowers survive creator release and cancellation until joined and individually acknowledged; 1364 lifetime words,44220 deadline cases and16524 budget cases compare actual C with independent models.',
           'The two-clock inequality d+P+(R-B-P)<=R is implemented and checked; the contract exposes unbounded transit/unequal clock-rate and repeated-per-hop-reset counterexamples.'],
       variant='Creator parent reference is released while a child retains the allocated cancellation cell; every cut while constructing three children is cleaned. Native completion remains separately acknowledged.',
       limits=['The equal-rate clock/transit bound is a premise; no physical clock synchronization or remote termination deadline is measured.']),
2:dict(contract='PRESSURE',parts=['pressure'],test='pressure_test',oracle='pressure',
       anchors=['resilience.pressure','resilience.pressure_owned'],gap='IW-53694610af4bcd2c',
       demonstration=[
           'Five copied/owned policies explicitly separate caller-retained BLOCK/REJECT from consumed DROP/OVERWRITE/DEGRADE and distinguish queue, transport and leased state.',
           'Actual struct rz_queue and struct msg_hub move a head only after accepted send; owned values are disposed exactly once under all five policies and closed draining.',
           'Two registered producer slots and native poll waiters are retained until terminal collection; four generation-checked credits partition queued/transport/leased/retired storage. Stale and duplicate returns cannot create credit.',
           'The same eight-offer/four-consume schedule reports accepted, rejected, discarded, coalesced and caller-pending counts for every policy;121554 independent histories and owned disposer counters check the accounting.'],
       variant='Two endpoint actors fill their outbound capacity and wait on retained sends. Forwarding alone releases no credit; explicit receive/release progress breaks the demonstrated wait-for cycle and joins both waiters.',
       limits=['Native schedules are bounded; externally serialized policy queues and mutex-protected pumps have different synchronization contracts.']),
3:dict(contract='TIMERS',parts=['timers'],test='timers_test',oracle='timers',
       anchors=['resilience.wheel','resilience.timer_structures'],gap='IW-0771cf2962ea7460',
       demonstration=[
           'Full due times, stable sequence identities, bidirectional links, heap positions and nonwrapping generations distinguish expiry from bucket coincidence.',
           'Actual sorted list, indexed heap and two-level eight-fine/eight-coarse wheel implement the same sixteen-entry stable-order/horizon/identity contract, with late processing and transactional refusal.',
           'Every full-bucket removal position and extreme generation/sequence/timestamp is checked;58821 histories and13056 due/cancel pairs compare actual C with a sorted independent oracle.',
           'Four compiler/optimization configurations compare15 equal-contract modes,420 timing samples and491520 per-advance observations; measured coarse versus fine burst ordering disproves constant expiry bookkeeping. Real CMX is imported for only the common due/cancel outcome contract.'],
       variant='A native producer admits at observed100 while processing remains0. Serialized catch-up returns original due1..8 with lateness99..92 and new due101..108 unchanged.',
       limits=['Original accepted PR-T50-05 receipts remain unchanged; retained CMX scans task slots and does not export the stronger stable event/identity contract. Host sample maxima are not physical jitter or WCET.']),
4:dict(contract='FRACTIONAL',parts=['fractional'],test='foundation_test',oracle='followthrough',
       anchors=['resilience.bucket','resilience.fractional_follow'],gap='IW-382c534856bf183c',
       demonstration=[
           'Exact T+r/q credit establishes the burst-plus-rate bound and distinguishes a policer from a waiting shaper or in-flight concurrency limit.',
           'Checked time-to-full comparison bounds multiplication below the missing credit; arbitrary idle splits preserve fractions, and denied admission commits refill without spending.',
           '51188 Fraction-based histories plus explicit numeric extrema, backward time and malformed states check refusal and cumulative arithmetic; remainder-loss and one-sided-debit controls fail.',
           '5824 paired histories stage two refills before a joint debit; the fixed/sliding/token comparison uses one arrival trace and states why their burst contracts differ.'],
       variant='864 independent migration cases export exact credit and rebase to a different origin under explicit FREEZE or trusted-duration REFILL; source retirement is an authority premise, not implied by copying.',
       limits=['No authenticated or durable authority handoff is claimed by the arithmetic snapshot. Caller serialization remains required.']),
5:dict(contract='RETRY',parts=['retry'],test='retry_test',oracle='retry',
       anchors=['resilience.retry','resilience.retry_transport'],gap='IW-1cad76fc08b89d61',
       demonstration=[
           'Original work and capped retry work are counted separately; checked exponential windows, common inherited deadline and idempotency precondition expose nested amplification and uncertain side effects.',
           'Actual deterministic PRNG rejection sampling has an explicit64-draw refusal bound and draw-counter retirement. Rzr imports retained struct net_stack retransmission and real ACK parsing, charges every dispatch and retains its scope until ACK or reset.',
           '312 random,6000 plan and10080 history cases cover draws, caps, deadline boundaries and a receiver whose effect commits while its ACK is lost; repeated sends replay the result.',
           '60 equal-budget population workloads compare no/full/equal jitter with independent and correlated seeds, reporting synchronized work and completion observations rather than claiming cryptographic randomness.'],
       variant='A Retry-After9000 at time1000 with deadline10000 refuses without advancing attempts, but an already outstanding transport accepts its later ACK and releases only after finish.',
       limits=['Established-flight fixture and synthetic loss model are not full TCP conformance or measured network behavior. PRNG uniformity is bounded-selection arithmetic, not adversarial unpredictability.']),
6:dict(contract='BREAKER',parts=['breaker'],test='breaker_test',oracle='breaker',
       anchors=['resilience.gate','resilience.breaker_window'],gap='IW-9a146fe69f867933',
       demonstration=[
           'Health observations and reserved active cost are separate invariants. Neutral outcomes do not become backend successes; permit epoch prevents stale health publication.',
           'Bounded gate admission/completion checks tenant reservation, cost, epoch/generation exhaustion, exact cooldown and overflow retirement, while old permits retain their cleanup authority.',
           '162908 structured and5000 mixed-tenant histories recompute retained lists for four sliding-window policies; both two-permit completion orders and stale/half-open transitions are explicit.',
           'Cost1/cost2 under reservation2 per tenant demonstrates isolation and idle-capacity refusal. Phase/epoch/occupancy traces compare recovery rules without equating consecutive-failure and window contracts.'],
       variant='Close requests cancellation of two tenant operations. Completing one neutrally and dropping the creator leaves the other cost and scope retained until its deliberately late completion; only then is quiescence true.',
       limits=['Backend refusal to acknowledge can retain resources indefinitely; there is no forced safe reclamation or fairness guarantee. Native owned hedge/service tests separately exercise worker/I/O lifetime.']),
7:dict(contract='HEDGE',parts=['hedge'],test='hedge_test',oracle='hedge',
       anchors=['resilience.hedge','resilience.hedge_owned'],gap='IW-6f5b1acff80493dc',
       demonstration=[
           'Post-decode acceptability and fresh scope check precede result publication. Winner, callback completion and permit/scope quiescence are separate; deduplication at the effect resource is necessary.',
           'Actual arbiter owns two tickets and response blobs, permits one result transfer, refuses stale/unstarted/duplicate completions and retains losing cleanup after result availability.',
           'Controlled pthread decoder/disposer pauses exercise both success orders, cancellation/expiry during decode, abandonment during loser cleanup and32 simultaneous races under all declared sanitizer profiles.',
           '7388 histories and16384 modeled requests distinguish independent/correlated tails and total backend work. Actual composition also measures reply3 versus8 with quiet8 and correlated dispatch8 versus4 with no winner.'],
       variant='Owned variable-size response blobs are decoded fallibly outside the mutex. Every rejected/losing blob is disposed once, including a slow failing decoder and abandonment; stale request generation cannot affect replacement.',
       limits=['Health outcome linearizes after decode before disposal; later abandonment cannot retroactively change it. Native bounded schedules and serialized oracle are not all-interleaving proof.']),
8:dict(contract='JOURNAL',parts=['journal'],test='journal_test',oracle='journal',
       anchors=['resilience.receiver','resilience.receiver_journal'],gap='IW-2de415706529a315',
       demonstration=[
           'Epoch/client/request/intent identity, finite dedup retention, modeled authentication and effect-resource fencing are explicit. Uncertain write progress poisons the instance rather than claiming rollback.',
           'Retained receiver stages checked total and records; actual64-byte journal persists body/flush/footer/flush before publishing logical state and returns original responses to equal-intent replay.',
           '2916 request/wire cases and414 independent durability cuts compare recovered dictionaries/prefixes. Native positional I/O, partial writes, fdatasync and fork exit before reply execute locally; corruption and failed recovery preserve output.',
           'Persisted issuer restart never reuses tokens; reordered clients, expired entries and divergent writable replicas expose limits. A read-only follower cannot independently admit stale effects; divergent history is rejected rather than erased.'],
       variant='One original teaching replica accepts fence4 after another commits fence5, producing incompatible totals. The repaired serialized primary enforces the fence and mirror/follower protocol has no second writer authority, including interrupted catch-up.',
       limits=['Checksums detect the stated corruption model, not malicious forgery. POSIX success is not a physical power-loss test; one serialized writer is not consensus.']),
9:dict(contract='COMPOSITION',parts=['composition','service_io','outbox','journal'],test='composition_test',oracle='composition',
       anchors=['resilience.service','resilience.service_outbox','resilience.service_io','resilience.service_transport'],gap='IW-89d3bfa32a89f5b1',
       demonstration=[
           'Actual owner/event ledger separates admission, effect commit, reply, queued/delivered/ACKed notification and late backend cleanup; each response, scope, permit, wheel event and device has an explicit owner.',
           'Real struct msg_hub, retained struct net_stack retry/ACK and retained hosted deadline/completion-byte APIs are linked. Original18-byte local framing/partial sends and durable outbox adapters fill concrete integration boundaries; no absent general reactor is relabeled.',
           '9096 independent transport histories,79896 outbox histories,11714 parser cases, native partial-I/O/cancel/fork cases and allocation/write cuts account for each effect and owner. Six queued socketpair requests additionally test two burst orders, four accepted events and delayed ACK release.',
           'Disclosed author cost decision precedes280 new host timing samples across ten workloads/four builds. All13 dimensions distinguish operation counts, virtual reply/quiet/notification ticks and measured host cost; learner independence is explicitly separate.'],
       variant='Commit/deliver before process or logical-service restart preserves event sequence and yields at-least-once redelivery until a durable ACK. Three epochs execute45 logical requests with reused IDs after quiescence and durable issuer rotation; finite journal capacity remains explicit.',
       limits=['No external exactly-once side-effect claim. Stream/ACK fixtures and local IPC are bounded teaching adapters, not deployed network or storage services.'])
}

def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def sha(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def dump(path,value):(ROOT/path).write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def metadata(path):return json.loads((ROOT/path).read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0])

def main():
    campaign,cost,service=map(read,REPORTS);check_campaign(campaign);check_cost(cost);check_service_cost(service)
    report_pins={p:sha(p) for p in REPORTS};criteria=[];variants=[];updates=[]
    readiness='research/v5/modern/resilience_objective_readiness.json'
    cost_matrix='research/v5/modern/resilience_cost_matrix.json'
    abi_probe='research/v5/modern/resilience_data_model_probe.json'
    probe=read(abi_probe)
    for p,h in probe['input_sha256'].items():assert sha(p)==h,('stale ABI availability probe',p)
    assert len(probe['observations'])==2
    assert all(r['status']=='unavailable' and r['exit_code']!=0 for r in probe['observations'])
    dump(cost_matrix,matrix(SPECS,service,SERVICE_COST,COST,abi_probe))
    remaining_path='curriculum/v5/expansion/b07_t75_remaining.json';remaining=read(remaining_path)
    all_inputs={}
    for data,key in [(campaign,'inspected_paths'),(cost,'input_sha256'),(service,'input_sha256')]:
        for p,h in data[key].items():
            assert p not in all_inputs or all_inputs[p]==h,('conflicting current inputs',p)
            all_inputs[p]=h
    for p in [cost_matrix,abi_probe,'tools/probe_resilience_data_model.py','tools/resilience_cost_matrix.py',
              'research/v5/modern/resilience_service_cost_analysis.md',
              'research/v5/modern/resilience_service_decision.json']:
        all_inputs[p]=sha(p)
    for number,spec in SPECS.items():
        exercise=f'X-T75-{number:03d}';brief=f'curriculum/v5/exercises/{exercise}.md';meta=metadata(brief)
        teaching=[brief,meta['assessor_analysis'],'labs/resilience/CONTRACTS.md',f'labs/resilience/followthrough/{spec["contract"]}.md','labs/resilience/followthrough/COST.md',cost_matrix]
        implementation=['assessor/resilience/resilience.c','labs/resilience/tests/tests.c',f'labs/resilience/followthrough/{spec["test"]}.c',f'tools/verify_resilience_{spec["oracle"]}.py']
        implementation += [f'assessor/resilience/followthrough/{part}.c' for part in spec['parts']]
        if number==9:implementation += ['labs/resilience/followthrough/service_io_test.c','labs/resilience/followthrough/outbox_test.c','tools/verify_resilience_outbox.py','tools/verify_resilience_service_io.py']
        paths=teaching+implementation+REPORTS+[abi_probe]
        current_pins={p:sha(p) for p in paths};all_inputs.update(current_pins)
        limits=spec['limits']+['Finite supplied contracts and available host profiles only; no arbitrary C execution, physical validation or learner mastery claim.',
                             'Published ISO C17 access remains unavailable; inspected drafts are not relabeled. Other ABI/physical assumptions require their own evidence.']
        for rubric,demonstration in zip(meta['rubric'],spec['demonstration']):
            criteria.append({**rubric,'author_readiness':'ready-bounded','demonstration':demonstration,'teaching':teaching,
                             'implementation_and_observation':implementation+REPORTS,'anchors':spec['anchors'],'pins':current_pins,'limits':limits,
                             'external_review':'not-run','independent_learner':'not-assessed','physical_evidence':'not-claimed'})
        variants.append({'id':exercise+'-DELAYED','author_support':'implemented-and-host-validated','demonstration':spec['variant'],
                         'evidence':paths,'pins':current_pins,'independent_learner':'not-assessed','limits':limits})
        for identity in [exercise,exercise+'-DELAYED',spec['gap']]+[r['id'] for r in meta['rubric']]:
            demonstration=spec['variant'] if identity.endswith('-DELAYED') else next((r['demonstration'] for r in criteria if r['id']==identity),' '.join(spec['demonstration']))
            updates.append({'id':identity,'dimensions':{
                'teaching':{'state':'complete','evidence':teaching},
                'implementation':{'state':'complete','evidence':implementation},
                'author_validation':{'state':'passed','evidence':REPORTS+[readiness]},
                'external_review':{'state':'not-run','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-applicable-host-contract','evidence':[teaching[3]]}},
                'closure_rationale':demonstration+' Exact external/physical/edition limits remain in the criterion readiness map.'})
        old=next(row for row in remaining['required_unexecuted_extensions'] if row['exercise']==exercise)
        old.update(id=spec['gap'],status='implemented-and-author-validated; independent learner assessment not-run',
                   evidence=[readiness,*REPORTS],remaining_external=['Independent delayed learner attempt and per-objective judgment','Physical evidence only for an explicitly selected physical extension'])
    assert len(criteria)==36 and len(variants)==9 and len(updates)==63
    remaining.update(status='required-author-successors-implemented-and-source-validated',comprehensive_complete=False,
                     field_name_note='required_unexecuted_extensions retains its original name/order for stable inventory origins; current per-row status is authoritative.',
                     remaining_scope='Independent/delayed learner performance, non-author review, unavailable published-edition passages and any selected physical claims remain separate. The nine named author implementation extensions now have actual support.')
    dump(remaining_path,remaining);all_inputs[remaining_path]=sha(remaining_path)
    value={'schema_version':1,'domain':'T75','criteria':criteria,'variants':variants,'current_reports':REPORTS,
           'report_sha256':report_pins,'source_sha256':all_inputs,'guard':'tools/check_resilience_followthrough.py',
           'limits':['This is exact author-support readiness, not a whole-curriculum completion or learner mastery receipt.','Bounded author-team source reviews are not independent human assessment.']}
    dump(readiness,value)
    all_inputs[readiness]=sha(readiness);all_inputs['tools/build_resilience_readiness.py']=sha('tools/build_resilience_readiness.py')
    all_inputs['tools/check_resilience_followthrough.py']=sha('tools/check_resilience_followthrough.py')
    dump('research/v5/modern/resilience_completion_dispositions.json',{'schema_version':1,'domain':'T75','updates':updates,
         'current_reports':REPORTS,'input_sha256':dict(sorted(all_inputs.items())),
         'scope':'63 exact original IDs: nine exercises,36 rubric criteria,nine delayed variants and nine previously hidden extension records. Author execution is separate from independent assessment.'})
    print(json.dumps({'passed':True,'criteria':36,'variants':9,'updates':63}))

if __name__=='__main__':main()
