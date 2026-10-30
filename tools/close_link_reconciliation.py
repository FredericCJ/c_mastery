"""Exact original Ethernet/EDU7 X obligations, with scoped executable evidence."""
import argparse,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_link_reconciliation'
sys.path.insert(0,str(ROOT))
BASE={
 'b4':('research/v5/expansion/legacy_link_reconciliation/b04_current.json','labs/b04_depth/CONTRACTS.md'),
 'ed':('reviews/v5/expansion/integrated/ethernet_depth_campaign.json','labs/ethernet_depth/CONTRACTS.md'),
 'e7':('reviews/v5/expansion/integrated/edu7_depth_campaign.json','labs/edu7_depth/CONTRACTS.md'),
 'analysis':('research/v5/expansion/legacy_link_reconciliation/analysis_validation.json','curriculum/v5/expansion/link_transfer_analysis.md')}
for key,folder,lab in [('wire','ethernet_wire','ethernet_followthrough/wire'),('phy','ethernet_phy','ethernet_followthrough/phy'),('ring','ethernet_ring','ethernet_followthrough/ring'),('handoff','ethernet_handoff','ethernet_followthrough/handoff'),('physical','edu7_physical','edu7_followthrough/physical'),('transport','edu7_transport','edu7_followthrough/transport'),('session','edu7_session','edu7_followthrough/session'),('pipeline','edu7_pipeline','edu7_followthrough/pipeline'),('transfer','link_transfer','link_transfer')]:BASE[key]=('research/v5/expansion/legacy_'+folder+'/validation.json','labs/'+lab+'/CONTRACTS.md')
# Each entry: original X suffix -> actual producer case and concrete assessed mechanism.
MAP={
 '39-001':{
 'R1':('b4:dma ring:rx,recovery,visibility','Actual lease completion/invalidation, stale and duplicate release, exhaustion and late epoch refusal.'),
 'R2':('ring:init,visibility,port','Whole-interval placement and object/link audit remain separate from the two-line CPU/device visibility model.'),
 'V1':('ring:visibility analysis:witness','Two live payload extents sharing a32-byte line; separate rounded reservations or whole-line ownership prevents collateral maintenance.'),
 'V2':('analysis:witness','Explicit hypothetical coherent-domain visibility plus device release/acquire order; early-doorbell and early-reuse counterexamples. Target execution remains separately gated.')},
 '39-002':{
 'R1':('ed:layers wire:layers','Cumulative SDU/PDU extents, staged overflow and explicit distinction from real wire encoding.'),
 'R2':('ed:layers wire:layers','Version, namespace, service-shape and ownership mismatches rejected despite identical layer labels.'),
 'V1':('transfer:borrow','Immutable borrowed payload and copied length prefix, actual accepted suffix progress and retained reset obligation.'),
 'V2':('transfer:sizes','Dependent padded-length trailers with checked additions, capacity and atomic output publication.')},
 '39-003':{
 'R1':('ed:frame wire:frame,vlan','Admitted IEEE length/EtherType fields, excluded gap and original untagged scope; explicit additional VLAN format.'),
 'R2':('ed:frame wire:frame,vlan','Wire minimum padding versus meaningful length, directional stripped/retained FCS and buffer capacities.'),
 'R3':('ed:frame wire:frame','Whole-header/payload/padding FCS and validation before caller-owned parse-borrow publication.'),
 'V1':('wire:vlan','Inspected tag fields, shifted offsets, capacity and retained-FCS checksum/borrow boundaries.'),
 'V2':('wire:frame ring:rx,consumer','Hardware-generated TX FCS format and retained RX FCS parser borrow, with exact release on rejection.')},
 '39-004':{
 'R1':('ed:bridge wire:bridge','Group/individual and local-administration bits are forwarding properties, never authentication.'),
 'R2':('ed:bridge wire:bridge','Four-entry expiry/movement/capacity and ingress-excluding exact port masks.'),
 'V1':('wire:bridge','Explicit eviction versus unchanged atomic FULL, stale location and extra flooding under bounded state.'),
 'V2':('transfer:bridge wire:loop','Two VLAN-keyed learning tables plus independent repeated-frame loop counterexample; capacity cannot replace topology control.')},
 '39-005':{
 'R1':('ed:phy phy:access,mapped','MAC MII divider/address encodings versus external RMII clock; source and MMIO callback audits.'),
 'R2':('ed:phy phy:access,bounded,faults','Exact identity, latch-low double read, bounded BUSY polling and read-failure output canaries.'),
 'R3':('ed:phy phy:access,lifecycle','Actually negotiated speed/duplex admission and failure-preserved outputs.'),
 'V1':('phy:lifecycle,leases ring:recovery','Reset/reprobe with old hardware ownership retained until confirmed idle; timeout cannot recycle.'),
 'V2':('transfer:phy','Different exact DP83848-EP revision0 passages, full IDs, latch-low BMSR and PHYSTS mapping; no copied LAN8742 bit meanings.')},
 '39-006':{
 'R1':('ed:rx ring:chain,rx','Final FL authority, combined descriptor capacity and FCS-straddling borrowed span.'),
 'R2':('ed:rx ring:chain,drop','Missing FS/LS, malformed chain, late LS and drain state without publishing partial input.'),
 'R3':('ed:rx ring:rx,consumer','Held immutable actual ring backing, matching token once, stale/duplicate refusal and finite retirement.'),
 'V1':('ring:consumer','Actual parser rejection of valid DMA input invokes one release on every checked exit.'),
 'V2':('ring:rx,drop,chain','Four-descriptor retained ring with exhaustion/drain and separately declared FCS-stripping adapter; no invented changed-silicon FL mapping.')},
 '39-007':{
 'R1':('ed:tx ring:init,bounds','SRAM interval/alignment arithmetic and separate real linker placement plus CCM rejection.'),
 'R2':('ed:tx ring:order,port','Original dependency-order first violation and actual ordered MMIO initialization.'),
 'R3':('ed:tx ring:publication','Tail metadata/ownership before final head publication, retained barrier calls and premature-head refusal.'),
 'R4':('ed:tx ring:tx,publication','Copied admission, actual publication and returned ownership are separate from peer delivery.'),
 'R5':('ed:tx ring:recovery,bounds,publication','Two-slot backpressure, unconfirmed abort retains storage and late-incarnation rejection.'),
 'V1':('ring:publication','Actual struct ev_chain borrow until all returned or stop-confirmed; partial publication cancellation and caller-mutation prohibition.'),
 'V2':('ring:publication,recovery','Three-descriptor final-head model, separate reset/drain and newer generation; explicit changed device-format proof before target use.')},
 '39-008':{
 'R1':('ed:handoff handoff:clock','Origin-relative timestamp intervals, negative/overflow/nonmonotonic refusal.'),
 'R2':('ed:handoff handoff:divergence','First failed declared dependency and uncertainty-aware observation without unsupported root-cause certainty.'),
 'R3':('ed:handoff handoff:budget','Minimum padding and serialization budget distinguished from handshake, queue and application time.'),
 'V1':('handoff:clock','Bounded drift intervals with proved order only for disjoint bounds and explicit inconclusive touching/overlap.'),
 'V2':('handoff:stack,reassembly pipeline:exchange','Actual frozen ring/ARP/IP/TCP/HTTP frame-channel composition; EDU7 full routed exchange is separately scoped, not substituted for Ethernet.')},
 '40-001':{
 'R1':('b4:serial transfer:serial','Bounded outstanding span, old/future/equal and even-cycle antipode ambiguity.'),
 'V1':('transfer:serial','Wrapped transfer and delayed duplicate cumulative ACK with enforced outstanding-span assumption.'),
 'V2':('transfer:serial','Arbitrary non-power-of-two cycle, independent walking oracle and mask defect rejected.')},
 '40-002':{
 'R1':('e7:grant physical:grant','Guard, first byte and whole reply use distinct origins and exact inclusive boundaries.'),
 'R2':('e7:grant physical:physical,grant','Retained framing then local grant address/echo/fault gate; UART corruption never promotes valid input.'),
 'R3':('physical:physical,port','Local admission and actual target object audits do not supply physical timing or peer delivery.'),
 'V1':('physical:tx,grant,port pipeline:exchange','Actual struct pu_driver TXE/TC and DE, POLL final TC timestamp and receiver guard composition.'),
 'V2':('physical:physical analysis:witness','Exact42MHz/729 occupancy exceeds original10ms; separately negotiated20ms/50ms turn recomputation retains physical measurement gate.')},
 '40-003':{
 'R1':('e7:offer transport:retry','Copied immutable staged offer and index through admission refusal, acceptance and retry.'),
 'R2':('e7:offer transport:retry,expiry','Accepted lower transfer starts retry; blocked credit expiry is fixed and not renewed by polling.'),
 'R3':('e7:offer transport:retry,reassembly','Failed logical transfer releases only permitted state, partial upper publication forbidden and stale token completion refused.'),
 'V1':('transport:expiry pipeline:exchange,identities,faults','Actual grant/receiver/node identity composition and explicit transport epoch across drained restart.'),
 'V2':('transfer:partial','Prefix staging defines whole-fragment final-byte acceptance clock and independent admission deadline; timeout retains incomplete lower record until reset.')},
 '40-004':{
 'R1':('e7:session session:dialogue','Dialogue token authority, prepared operation and lost token completion remain distinct.'),
 'R2':('e7:session session:receipts','Separate directional receipts and exact cached response replay with one synthetic effect.'),
 'R3':('e7:session session:resume','Stale session/transport identity and reboot resume refusal, checkpoint/epoch headroom.'),
 'R4':('e7:session session:receipts pipeline:adversarial','Presentation/application staged before receipt; malformed/out-of-range SET preserves state.'),
 'V1':('session:dialogue,receipts pipeline:exchange,faults','Actual two endpoints plus physical grant/transport, lost token ACK and reply failure after commit.'),
 'V2':('session:resume','Original external retained pulse versus volatile receipt reset demonstrates duplicate effect; no durable exactly-once claim.')}
}
LIMITS=['Existing author observations and new finite experiments are scoped evidence, not an independent learner attempt or external review.','No physical board, peer timing, Ethernet silicon/PHY or RS485 line observations are supplied.','Model profiles and changed-format analysis do not establish unexecuted target hardware behavior.','Stored generic campaign global tree hashes are historical; only current inspected-path identities and exact retained successful commands are reused.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def brief(x):
 p=ROOT/('curriculum/v5/exercises/'+x+'.md');t=p.read_text(encoding='utf8');return p,json.loads(t.split('```json')[1].split('```')[0]),t
def tasks(x):
 _,d,t=brief(x);v=t.split('## Variants\n',1)[1].split('\n## ',1)[0]
 result={q['id']:q['criterion'] for q in d['rubric']}
 for n in ['V1','V2']:result[x+'-'+n]=re.search(r'\b'+n+r':\s*(.*?)(?=\s+V[12]:|\n\n|$)',v,re.S).group(1).strip()
 return d,t,result
def select(spec):
 out=[]
 for token in spec.split():
  key,cases=token.split(':');report,contract=BASE[key];d=read(ROOT/report);selected=[]
  for i,q in enumerate(d['commands']):
   name=q.get('id');text=q.get('stdout','')
   match=(key=='analysis' or (name and any(('.'+case in name and not name.endswith('.compile')) for case in cases.split(','))) or (not name and 'ctest' in q.get('command',[]) and any((('b04_depth' if key=='b4' else 'ethernet_depth' if key=='ed' else 'edu7_depth')+'.'+case) in text for case in cases.split(','))))
   if match:selected.append(name if name else 'command:'+str(i))
  assert selected,(spec,key,cases)
  out.append(dict(report=report,cases=cases.split(','),checks=selected,contract=contract))
 return out
def rows():
 out=[]
 for suffix,mapping in MAP.items():
  x='X-T'+suffix;d,t,original=tasks(x);assert set(original)=={x+'-'+key for key in mapping}
  child=[]
  for key,(spec,why) in mapping.items():
   evidence=select(spec);artifacts=list(dict.fromkeys([q['contract'] for q in evidence]+[d['assessor_analysis']]))
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',['curriculum/v5/exercises/'+x+'.md',*artifacts]),('implementation','complete',artifacts),('author_validation','passed',[q['report'] for q in evidence]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   row=dict(id=x+'-'+key,original_exercise=x,original_task=original[x+'-'+key],minimum_days=d['delayed_transfer']['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(scoped_evidence=evidence),closure_rationale=why,remaining_author_work=[],evidence_limits=LIMITS)
   if key.startswith('V'):row['original_variant']=key
   child.append(row)
  root=dict(id=x,original_exercise=x,original_task=t.split('## Problem\n',1)[1].split('\n## ',1)[0].strip(),minimum_days=d['delayed_transfer']['minimum_days'],dimensions={k:dict(state=v['state'],evidence=sorted(set(n for row in child for n in row['dimensions'][k]['evidence']))) for k,v in child[0]['dimensions'].items()},artifacts=sorted(set(n for row in child for n in row['artifacts'])),author_validation=dict(required_children=[q['id'] for q in child]),closure_rationale='Original problem, each objective-specific rubric and both exact changed-condition variants are bound separately; implementation closure is author work only.',remaining_author_work=[],evidence_limits=LIMITS)
  out.extend([root,*child])
 return out
def integrate():
 for suffix,mapping in MAP.items():
  x='X-T'+suffix;p,d,t=brief(x);old=HERE/(x+'.before-reconciliation.md')
  if not old.exists():old.write_bytes(p.read_bytes())
  contracts=sorted({BASE[token.split(':')[0]][1] for spec,_ in mapping.values() for token in spec.split()})
  d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+contracts));body=t.split('```',2)[2];heading='\n## Required source-bound depth integration\n';body=body.split(heading)[0]
  body+=heading+'\nThe original problem, rubric and both delayed variants remain required. Apply these concrete contracts to the original tasks:\n\n'+'\n'.join('- ['+n.rsplit('/',2)[-2]+' contract]('+n+')' for n in contracts)+'\n\nUse the exported unfinished producer artifacts from the parent project for executable work. A shared analysis contract is copied directly with this exercise. Existing author campaigns are bounded reference evidence; independent delayed assessment and physical observations remain separate requirements.\n'
  p.write_text('```json\n'+json.dumps(d,indent=2)+'\n```'+body,encoding='utf8')
 print('PASS12 canonical X integrations')
def package():
 values=rows();files={pathlib.Path(__file__),ROOT/'tools/check_link_reconciliation.py',ROOT/'tools/check_link_analysis.py',ROOT/'tools/check_link_reconciliation_exports.py',HERE/'exports.json'}
 for suffix in MAP:
  p,_,_=brief('X-T'+suffix);files.add(p);files.add(HERE/(p.stem+'.before-reconciliation.md'))
 for report,contract in BASE.values():
  files|={ROOT/report,ROOT/contract};d=read(ROOT/report)
  for name in d.get('source_sha256',d.get('inspected_paths',{})):files.add(ROOT/name)
 for name in ['analysis_validation.json','audit.md']:files.add(HERE/name)
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='All original X-T39 and X-T40 problem/rubric/variant identities; author dimensions only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=sorted({q[0] for q in BASE.values()}|{'research/v5/expansion/legacy_link_reconciliation/exports.json'}),updates=values,remaining_author_work=['T38 original X variants and mixed LRUN23/LRUN24 retain their separate scope.'],evidence_limits=LIMITS))
 print('PASS'+str(len(values))+' exact original X dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);integrate() if a.integrate else package()
if __name__=='__main__':main()
