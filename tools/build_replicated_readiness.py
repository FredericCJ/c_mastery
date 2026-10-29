"""Bind each original T76 criterion to current, specifically scoped author support."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PREFIX = 'research/v5/modern/'
READINESS = PREFIX + 'replicated_objective_readiness.json'
COST = PREFIX + 'replicated_cost_matrix.json'
MODULES = ['ordinary_depth', 'durable_current', 'outbox', 'clocks', 'hashing',
           'membership', 'deltas', 'sequence_depth', 'merkle_repair',
           'wire_depth', 'replay_depth']
SPECS = {
1: dict(module='followthrough', contract='DURABLE', report='durable_current', demonstrations=[
    'The admitted event prefix, immutable HAMT roots, current dictionary and validated snapshot are distinct. QUERY_RESULT records the original answer; the supplied counterexample shows re-querying99 instead of recorded9 changes semantics.',
    'The actual retained64-node HAMT consumes three cells per append and48 for sixteen. Count/root publication follows allocation and durable commit; failed preparation restores the used/count authority, with unreachable scratch and diagnostic counters explicitly excluded. Published arenas cannot relocate; the serialized successor explains why concurrent publication needs a separate release/acquire design.',
    'The retained log and its two semantic controls run in ordinary_depth. The actual codec/store adapter compares709 wire rows,3976 prefix queries,192 states,64 reopens,617 append admissions and64 snapshots, then executes417 append and417 recovery partial-I/O/flush cuts per profile. Compiler dependency closures bind actual HAMT, codec and page-store providers.',
    'Versioned UUID/schema/position records and historical-position snapshots survive every139502 append and139502 recovery byte/flush prefix in the stated model. Outbox process exits distinguish durable effect acceptance from lost reply; naive unrelated effect replay duplicates the synthetic counter.'],
    extras=['outbox'], limits=['The persistence-prefix model excludes arbitrary unordered sector subsets, checksum collision and dishonest flush; no physical power-loss observation.']),
2: dict(module='outbox', contract='OUTBOX', report='outbox', demonstrations=[
    'Query-model shape is separate from delivery. A retained log identity and serialized projection admit an event once; the outbox explicitly demonstrates that a nontransactional external counter may execute twice.',
    'Retained projection stores the exact applied prefix and checked int64 sum, with whole-state mutex ownership. The successor compares original historical key/value/kind and total; duplicate identity cannot silently refer to another log or revised content.',
    'Actual pthread projection schedules, original duplicate/gap controls and GCC/Clang TSan execute in ordinary_depth. Outbox independent transition model covers13417 rows and148 persistence cuts; no-interleaving or arbitrary-effect theorem is inferred.',
    'Three distinct actual page-store logs implement source/inbox/acknowledgment. Original local processes use actual framed transport, partial file I/O and fdatasync; exits after source commit, receiver commit before reply and sender before acknowledgment preserve retry identity. A separate external-effect experiment visibly duplicates.'],
    extras=['followthrough'], limits=['Trusted UUID/content correlation is not cryptographic authentication; the receiver state is atomic only within its actual store transaction.']),
3: dict(module='clocks', contract='CLOCKS', report='clocks', demonstrations=[
    'Scalar, vector and HLC contracts are separately derived; graph reachability, rather than timestamp order, supplies the causal oracle. Logical order does not establish physical recency.',
    'Concurrent events with ordered scalar clocks and incomparable vectors are explicit. All HLC equality branches and full-width overflow are tested; the wall/skew premise is retained in the closeness proof and WAIT never fabricates a later physical instant.',
    'The retained clock controls and exhaustive small HLC cases execute alongside128 independent histories,6144 events,144384 reachability pairs,40903 scalar nonconverse pairs and128 wire cases. Unsigned extrema and transactional outputs are distinct tests.',
    'Durable replacement retains old incarnation components and publishes the new identity only after commit;148 cuts exercise reopen. Finite identity/counter exhaustion refuses, while HLC logical exhaustion can wait only below physical UINT64_MAX. Actual native sizes, identity-bearing wire growth and bounded-skew analysis are supplied.'],
    extras=[], limits=['Clock-error bounds require the stated rate/skew/monotonicity premises; native execution does not establish physical clock synchronization.']),
4: dict(module='membership', contract='MEMBERSHIP', report='membership', demonstrations=[
    'Unique writer identity and componentwise max establish the state-join model. Register clock-skew counterexample shows chosen total order differs from real-time recency; operation retransmission is distinguished from state retransmission.',
    'The compatible-state law proof is accompanied by262144 PN triples, six merge orders and1000 compatible register triples. Forked shared identity loses an increment, unchecked wrap violates inflation, and equal-event conflicting payload exposes why the register join is not a total associative operation over invalid histories.',
    'Retained counters and tie-break controls execute with6108 independent register traces. Duplicate/reordered/full-state retransmission and explicit wrap/fork/additive-merge counterexamples are observed; named semantic controls compile successfully before normal rejection.',
    'Six retained identity components, canonical160-byte snapshots and64-byte HMAC admission packets implement replacement/corruption detection. Synthetic forged high component and authenticated but over-cap component distinguish authentication from authorization;12416 model rows,4368 small-state laws and1280 bit mutations exercise the policy. State-versus-event growth uses equal writer/retry premises.'],
    extras=['hashing'], limits=['Finite trusted permit/key authority is an original local teaching policy, not Byzantine consensus or general membership reconfiguration.']),
5: dict(module='deltas', contract='DELTAS', report='deltas', demonstrations=[
    'Exact unique tags and retained tombstones implement add-wins observed removal. The teaching contrasts a self-contained tag delta with an arbitrary partial operation and explains why collection needs a separate stability argument.',
    'The mathematical adds-minus-removed abstraction proves concurrent unseen adds survive and known removals cannot resurrect. Interval admission requires the actual incorporated causal base; a large sequence number alone is not an acknowledgment proof.',
    'Retained set controls and independent two-writer traces include duplicate and remove-before-add delivery. Successor model has3104 transitions,9312 snapshots and413 causal-base checks, with explicit24-tag/four-interval capacity outcomes.',
    'Three replicas, disconnect/restart, durable incarnation/counter identities, retained intervals and full-state fallback are implemented.222 persistence cuts per profile exercise epoch/state publication. Premature tombstone collection is falsified; the implemented policy retains tombstones and quantifies finite growth instead of claiming an unavailable stable frontier.'],
    extras=[], limits=['No tombstone garbage collection, asynchronous delivery liveness or hostile-peer wire protocol is claimed.']),
6: dict(module='sequence_depth', contract='SEQUENCE', report='sequence_depth', demonstrations=[
    'Stable node identity is separated from visible position; the parent-tree abstraction and central OT context protocol have distinct correctness premises. A two-insert diamond does not establish an arbitrary replicated OT protocol.',
    'Parent ordering prevents cycles and canonical child ordering preserves rendering under admitted arrival orders; deleted anchors retain descendants. The separate OT server retains transformed context/log state and each of three clients has at most one pending request.',
    'Retained rendering/deleted-subtree controls and recursive oracle run alongside9312 successor tree rows and15456 OT rows. The concrete three-operation context-blind result abx differs from corrected axb; capacity, duplicate, missing dependency and failure publication are assessed separately.',
    'Early-delete queue versus tombstone placeholder, Unicode scalar payload and368-byte snapshots are implemented. Compaction requires identical three-replica state plus drained transport before nonwrapping epoch change; old messages reject. State-vector/context-controlled OT is exercised beyond pairwise transformation.'],
    extras=[], limits=['Unicode scalars are not grapheme normalization; compaction requires explicit quiescence and the central finite OT protocol is not arbitrary distributed OT.']),
7: dict(module='merkle_repair', contract='MERKLE', report='merkle_repair', demonstrations=[
    'Ordered domain-separated leaves and explicit trusted domain/count/root distinguish content commitment from sender authentication, availability and semantic validity. Both raw RFC-style tree and original envelope are specified.',
    'First-principles streaming SHA uses defined uint32 operations, explicit endian conversion and complete length/padding semantics, with16-word and64-word schedules. Non-power-of-two shape and position/range/count checks are derived before proof admission.',
    'Independent hashlib/HMAC comparisons cover1786 messages,7144 digests and33411 two-piece splits, including padding boundaries and RFC vectors. Retained and new compiled controls reject semantic defects. Actual assembly/stack/link observations and equivalent-root selective/full-node/direct-leaf experiments charge client hashes and framed traffic.',
    'Inclusion proofs verify exact index/count/path and fail on malformed, truncated or extra siblings. Actual struct rf_parser frames original request/reply packets;2311 proof/path rows and2090 repair rows include malicious missing/inconsistent peer data without partial target publication. Chunking/domain changes alter identity as specified.'],
    extras=['hashing'], limits=['Finite experiments do not prove collision resistance, constant-time behavior, root provenance or peer availability.']),
8: dict(module='wire_depth', contract='WIRE', report='wire_depth', demonstrations=[
    'Grammar acceptance and canonical identity are separate; arithmetic ZigZag includes INT32_MIN without signed left-shift assumptions. Versioned RPW1 is selected explicitly rather than silently changing the old codec.',
    'Minimal five-byte varint bounds and sign-mask arithmetic are justified; sorted canonical field maps reject duplicates. Decoder/encoder idempotence is scoped to admitted values, with unknown fields retained rather than reconstructed from only known fields.',
    'Retained overlong/ZigZag controls and independent original codec comparisons execute. Successor oracle covers6102 rows:2225 admitted,1283 budget refusals,2082 malformed and512 required-capability rejections; truncation, ordering and output publication have separate tests.',
    'Owned incremental stream composes actual framing and enforces global bytes/fields/arena/depth budgets, critical/advisory negotiation and explicit terminator/EOF/cancel phases. Unknown opaque values round-trip exactly;2112 independent bit-normalization/UTF8 cases state signed-zero, NaN and exact-scalar policy without claiming NFC or a new float wire schema.'],
    extras=[], limits=['Externally serialized finite stream; no arbitrary recursive schema, Unicode normalization or concurrent feed claim.']),
9: dict(module='replay_depth', contract='REPLAY', report='replay_depth', demonstrations=[
    'Actual T20 struct ar_effects reducer separates complete state/input from pending effects and adapters. Speculative presentation notices replace ranges; only confirmed immutable input prefixes may publish effect intents. Ordinary C does not enforce purity.',
    'Unsigned step arithmetic and explicit random seed make deterministic state complete. Original checkpoint tests preserve old roots after failed second HAMT update. Successor copies/stages state, refuses score/revision overflow and keeps pending effect identity stable under later corrections.',
    'Original9841 movement histories and controls execute; the successor independent oracle compares10609 histories and319681 intermediate snapshots. Synthetic idempotent inbox and duplicate external-effect counterexample,296 persistence cuts and hidden-time undefined-symbol negative control establish named boundaries only.',
    'Two actual local processes send delayed corrections through actual frame parsing; bounded eight-tick rollback retains known/predicted state and replacement notices. T20 reducer/page-store composition survives process death between sink commit and acknowledgment. GCC/Clang host profiles and actual PPC64 ELFv2 big-endian O0/O3 executions compare canonical full-state hashes; a wrong-endian control rejects.'],
    extras=['followthrough'], limits=['No physical power-loss or exactly-once arbitrary device effect; big-endian execution covers the pure core/codec, separately from hosted POSIX persistence.'])
}

def read(p): return json.loads((ROOT / p).read_text(encoding='utf8'))
def sha(p): return hashlib.sha256((ROOT / p).read_bytes()).hexdigest()
def dump(p, x): (ROOT / p).write_text(json.dumps(x, indent=2, ensure_ascii=False)+'\n', encoding='utf8')
def metadata(p): return json.loads((ROOT/p).read_text(encoding='utf8').split('```json\n', 1)[1].split('\n```', 1)[0])

def main():
    reports = [PREFIX+'replicated_'+m+'_validation.json' for m in MODULES]
    reports += [PREFIX+'replicated_replay_target_validation.json', PREFIX+'replicated_build_cost_validation.json']
    guards = []
    for module in MODULES:
        command=[sys.executable, str(ROOT/f'tools/check_replicated_{module}.py'), '--self-test']
        p=subprocess.run(command, cwd=ROOT, text=True, capture_output=True, timeout=180)
        assert p.returncode==0, (module, p.stdout, p.stderr)
        guards.append(dict(command=command,exit_code=p.returncode,result=json.loads(p.stdout)))
        print(module+' evidence current',flush=True)
    current={p:sha(p) for p in reports};inputs={}
    for p in reports:
        d=read(p);assert d['passed'] and d['source_stable']
        for key,value in d['input_sha256'].items():
            assert sha(key)==value,('stale report',p,key)
            assert key not in inputs or inputs[key]==value,('conflicting source',key)
            inputs[key]=value
    cost=read(COST);assert len(cost['mechanisms'])==9
    for p,h in cost['current_reports'].items():assert sha(p)==h,('stale cost report',p)
    for row in cost['mechanisms']:assert len(row['dimensions'])==13 and len(row['build_observations'])==2
    criteria=[];variants=[];updates=[]
    gaps_path=PREFIX+'replicated_gap_ledger.json';gaps=read(gaps_path)
    for i,spec in SPECS.items():
        xid=f'X-T76-{i:03d}';brief=f'curriculum/v5/exercises/{xid}.md';meta=metadata(brief)
        contract=f'labs/replicated/{spec["module"]}/{spec["contract"]}.md'
        analysis=f'assessor/replicated/{spec["module"]}/'+('DURABLE_ANALYSIS.md' if i==1 else 'ANALYSIS.md')
        teaching=[brief,meta['assessor_analysis'],contract,analysis,'labs/replicated/CONTRACTS.md','labs/replicated/ordinary_depth/ORDINARY.md','labs/replicated/COST.md',COST]
        paths=['assessor/replicated/replicated.c','labs/replicated/tests/tests.c','labs/replicated/ordinary_depth/tests.c','tools/verify_replicated_ordinary_depth.py']
        for module in [spec['module']]+spec['extras']:
            paths += [p.relative_to(ROOT).as_posix() for area in ['labs','assessor'] for p in sorted((ROOT/area/'replicated'/module).rglob('*')) if p.is_file() and p.suffix in ['.c','.h','.md']]
        specific=[PREFIX+'replicated_ordinary_depth_validation.json',PREFIX+'replicated_'+spec['report']+'_validation.json',PREFIX+'replicated_build_cost_validation.json']
        for m in spec['extras']:specific.append(PREFIX+'replicated_'+('durable_current' if m=='followthrough' else m)+'_validation.json')
        if i==9:specific.append(PREFIX+'replicated_replay_target_validation.json')
        paths=list(dict.fromkeys(paths));pins={p:sha(p) for p in teaching+paths+specific};inputs.update(pins)
        limits=spec['limits']+['Author implementation and finite observations only; independent delayed learner performance and non-author review remain unobserved.','Published ISO C17 and C23 texts remain inaccessible; inspected drafts are not relabeled.']
        for r,demonstration in zip(meta['rubric'],spec['demonstrations']):
            criteria.append({**r,'author_readiness':'ready-bounded','demonstration':demonstration,'teaching':teaching,'implementation_and_observation':paths+specific,'anchors':meta['lab']['tests'],'pins':pins,'limits':limits,'independent_learner':'not-assessed','external_review':'not-run','physical_evidence':'not-claimed'})
        variants.append(dict(id=xid+'-DELAYED',author_support='implemented-and-author-validated',demonstration=spec['demonstrations'][3],evidence=teaching+paths+specific,pins=pins,limits=limits,independent_learner='not-assessed'))
        ids=[xid,xid+'-DELAYED',f'G-T76-{i:02d}-DELAYED',f'G-T76-{i:02d}-INVESTIGATION']+[r['id'] for r in meta['rubric']]
        for identity in ids:
            rationale=next((r['demonstration'] for r in criteria if r['id']==identity),spec['demonstrations'][3] if identity.endswith('-DELAYED') else ' '.join(spec['demonstrations']))
            updates.append(dict(id=identity,dimensions={'teaching':dict(state='complete',evidence=teaching),'implementation':dict(state='complete',evidence=paths),'author_validation':dict(state='passed',evidence=specific+[READINESS]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=[contract])},closure_rationale=rationale+' Exact model, physical, review and edition limits remain in the readiness map.'))
        for g in gaps['gaps']:
            if g['id'] in ids:g.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=specific+[READINESS],remaining_external=['Unaided delayed learner artifacts and per-objective judgments','Non-author review; physical claims require separate actual observations'])
    assert len(criteria)==36 and len(variants)==9 and len(updates)==72
    cost_gap=next(g for g in gaps['gaps'] if g['id']=='G-T76-COST-MEASUREMENTS')
    cost_gap.update(status='author-cost-analysis-and-measurements-complete-for-declared-contracts',author_evidence=[COST,PREFIX+'replicated_build_cost_validation.json'],remaining_external=['Learner-selected workload reproduction','Physical timing and external-review evidence are not supplied'])
    updates.append(dict(id=cost_gap['id'],dimensions={'teaching':dict(state='complete',evidence=['labs/replicated/COST.md',COST]),'implementation':dict(state='complete',evidence=['tools/author_replicated_cost.py','tools/run_replicated_build_cost.py']),'author_validation':dict(state='passed',evidence=reports),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=['labs/replicated/COST.md'])},closure_rationale='Nine specific thirteen-dimension analyses bind current runtime/traffic/hash/cut observations and22 actual isolated build-cost measurements. Equal-contract alternatives and single-sample timing uncertainty are explicit; physical timing and arbitrary workload performance are not inferred.'))
    gaps.update(author_implementation_complete=True,comprehensive_complete=False,readiness=READINESS)
    dump(gaps_path,gaps)
    remaining_path=PREFIX+'replicated_remaining_work.json';remaining=read(remaining_path)
    remaining.update(status='named-author-investigations-implemented-and-source-validated',author_implementation_complete=True,comprehensive_complete=False,readiness=READINESS,required_gap_ids=[g['id'] for g in gaps['gaps']],remaining_scope='The original IDs remain registered. Actual author support is supplied for nine investigations/delayed variants and cost analysis; independent learner assessment, non-author review, physical claims and exact published-edition access remain separately open.')
    dump(remaining_path,remaining)
    audit_path=PREFIX+'replicated_depth_audit.json';audit=read(audit_path)
    for row in audit['objective_map']:
        match=next(c for c in criteria if c['objective']==row['objective'])
        row.update(implementation_status='Required author support implemented and source-bound; learner not assessed',executed_scope=dict(readiness=READINESS,criterion=match['id'],reports=match['implementation_and_observation']),independent_assessment='not-attempted; actual learner judgment remains required')
    audit.update(producer_integration=dict(executed=['Actual persist.hamt, storage.pages, T20 record/reducer APIs, struct rf_parser and local POSIX process/file/socket adapters; exact source closures in current reports'],required_unexecuted=[]),validation=dict(current_reports=reports,readiness=READINESS),author_implementation_complete=True,comprehensive_complete=False)
    dump(audit_path,audit)
    for p in [gaps_path,remaining_path,audit_path,COST,'tools/author_replicated_cost.py','tools/build_replicated_readiness.py','tools/check_replicated_completion.py',PREFIX+'audit_replicated.py']+['tools/check_replicated_'+m+'.py' for m in MODULES]:inputs[p]=sha(p)
    value=dict(schema_version=1,domain='T76',criteria=criteria,variants=variants,current_reports=current,input_sha256=dict(sorted(inputs.items())),guard_observations=guards,limits=['Author-support readiness is not learner acceptance, a finite proof of absolute mastery, physical validation or independent human assessment.','All original prerequisite artifact identities and historical receipts remain preserved.'])
    dump(READINESS,value);inputs[READINESS]=sha(READINESS)
    dump(PREFIX+'replicated_completion_dispositions.json',dict(schema_version=1,domain='T76',updates=updates,current_reports=reports,input_sha256=dict(sorted(inputs.items())),scope='73 original IDs:9 exercises,36 rubrics,9 exercise delayed IDs,18 original investigation/delayed gaps and1 cost gap. Exact published C17/C23 reference gaps remain open.'))
    print(json.dumps(dict(passed=True,criteria=len(criteria),variants=len(variants),updates=len(updates))))

if __name__=='__main__':main()
