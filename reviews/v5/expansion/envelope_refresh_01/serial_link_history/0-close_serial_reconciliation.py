"""Exact original Ethernet/EDU7 X obligations, with scoped executable evidence."""
import argparse,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_reconciliation'
sys.path.insert(0,str(ROOT))
BASE={
 'b45':('research/v5/expansion/legacy_serial_reconciliation/b045_current.json','labs/b045_systems/CONTRACTS.md'),
 'sd':('reviews/v5/expansion/integrated/serial_depth_campaign.json','labs/serial_depth/CONTRACTS.md')}
for name,lab in [('uart','serial_followthrough/uart'),('sync','serial_followthrough/sync'),('transceivers','serial_followthrough/transceivers'),('i2c','serial_followthrough/i2c'),('spi','serial_followthrough/spi'),('rs485','serial_followthrough/rs485'),('can','serial_followthrough/can'),('fd','serial_followthrough/fd'),('mcp','serial_followthrough/mcp'),('variants','serial_variants'),('retry','serial_retry')]:BASE[name]=('research/v5/expansion/legacy_serial_'+name+'/validation.json','labs/'+lab+'/CONTRACTS.md')
BASE['pipeline']=('research/v5/expansion/legacy_edu7_pipeline/validation.json','labs/edu7_followthrough/pipeline/CONTRACTS.md')
MAP={
 '38-001':{
 'R1':('b45:i2c i2c:arbitration,electrical','Actual bit drive/sample with identical-prefix wired-AND and low-SCL nonprogress, distinct from sampled electrical bounds.'),
 'R2':('b45:i2c i2c:arbitration,transaction,failure','Arbitration loss releases without STOP; ACK/NACK and incomplete stretch remain separate results.'),
 'V1':('retry:stretch','Actual earlier T30 elapsed primitive, overall/continuous-low budgets, exact expiry, wrap and distinct lost-arbitration outcome.'),
 'V2':('retry:transaction','Actual SI2c bytes feed SDI2c address/register/repeated START and final NACK/read publication; no receiver waveform claim.')},
 '38-002':{
 'R1':('sd:uart uart:frames','LSB-first width/parity/stop derivation, asymmetric payload and malformed-frame output canaries.'),
 'R2':('sd:uart uart:baud','Integer divisor/OVER8 packing and rational error; physical period remains separately unobserved.'),
 'R3':('sd:uart uart:rx,tx','Conservative error publication, ORE survival and explicit TXE versus final TC.'),
 'V1':('uart:frames,rx,deadline,mapped','Actual F439 register adapter, admitted M/parity, SR/DR sequence and one bounded budget; no silicon measurement.'),
 'V2':('variants:baud','Different actual RP2040 integer/fractional format and exact rational admission, executed as a model only.')},
 '38-003':{
 'R1':('sd:sync sync:edges','All CPOL/CPHA traces and LBCL-dependent final data-clock pair.'),
 'R2':('sd:sync sync:transfer,config','No generated receive clocks without transmission; explicit clock exclusion for start/stop.'),
 'R3':('sd:sync sync:config,failures,mapped','Live configuration refusal and source/MMIO model distinct from electrical capture.'),
 'V1':('variants:word','Nine-bit hardware width, parity occupying the high word bit, payload extent and final parity/data clock under LBCL.'),
 'V2':('variants:capture','Unseen synthetic edge capture diagnosed for phase, polarity and selected setup/hold bounds; physical capture remains required.')},
 '38-004':{
 'R1':('sd:i2c i2c:transaction','Actual seven-bit combined read, per-phase ACK and terminal NACK before one-byte output.'),
 'R2':('sd:i2c i2c:failure,late_publication','Address/data NACK, arbitration, timeout and stale-token/late-publication ownership boundaries.'),
 'R3':('sd:i2c i2c:recovery','Conditional exclusive nine-clock clear versus stuck-clock reset and bounded target recovery.'),
 'V1':('retry:retry,transaction','Actual byte/transaction loss feeds application policy and bounded idle/backoff admission; uncertain increment is not replayed.'),
 'V2':('i2c:transaction,mapped','Actual one/two-byte F439 ACK/POS/BTF closing with inspected selected register passages; no invented generic last-byte sequence.')},
 '38-005':{
 'R1':('sd:spi spi:edges','All four MSB-first full-duplex modes with separate analog timing premises.'),
 'R2':('sd:spi spi:quiescence,queue','Receive drain and BSY completion plus confirmed cancel, with copied queue/borrow invariants.'),
 'R3':('sd:spi spi:irq,mapped','Stale observations and bounded model completion remain separate from electrical capture.'),
 'V1':('spi:quiescence,irq,mapped','Actual target adapter timeout/overrun/clear and conservative TX/RX DMA lease transitions.'),
 'V2':('spi:device','Changed partner dummy bytes, chip-select setup/hold and failure-contained device scheduling.')},
 '38-006':{
 'R1':('sd:electrical transceivers:thresholds','Synthetic inverted RS232 thresholds and unknown central interval versus selected receiver guarantees.'),
 'R2':('sd:electrical transceivers:differential','One-driver load assumptions and stated A-minus-B convention without universal ratings.'),
 'R3':('transceivers:connections,differential','One-driver/two-receiver topology distinguished from multipoint and addressing.'),
 'R4':('sd:electrical transceivers:capture232,captures422,exclusion','Pair swap only complements known decisions; measured termination/cable claims require evidence.'),
 'V1':('transceivers:connections','Selected documented connector map and output-to-output/missing-reference refusal over wiring permutations.'),
 'V2':('variants:receiver','Actual newly inspected THVD1450 common-mode/threshold envelope and history-sensitive hysteresis; typical values do not become guarantees.')},
 '38-007':{
 'R1':('sd:rs485 rs485:timing,cancellation,port','Enable, accepted bytes, final TC, disable and guard; unconfirmed stop retains ownership.'),
 'R2':('sd:rs485 rs485:events,config','Bounded once-only byte/error events and backpressure; nonoverlap model does not replace measurement.'),
 'V1':('rs485:link pipeline:exchange,faults','Actual copied channel into EDU7, lost byte/framing/cancellation and retained lease outcomes.'),
 'V2':('rs485:timing,cancellation','Modular half-range premise, ambiguous-delay rejection and finite token retirement.')},
 '38-008':{
 'R1':('sd:can can:arbitration','Actual standard/extended/data/remote arbitration prefix ordering.'),
 'R2':('sd:can can:codec','Strict retained CRC15/stuffing/trailer contract and malformed output preservation.'),
 'V1':('can:stream variants:stream','Actual incremental stuffing state plus generation-fenced reset/feed across every split; terminal failures are distinct from partial input.'),
 'V2':('can:fault variants:policy','Inspected2024 reserved-field/EOF/protocol-exception policy and actual error-state update; selected bounded policy not a complete controller.')},
 '38-009':{
 'R1':('sd:can can:timing','Exact timing fields, rate/sample point and SJW/range admission.'),
 'R2':('sd:can can:fifo,mailbox,port','Predicate, retained FIFO semantics and once-only mailbox result publication.'),
 'R3':('sd:can can:fifo,mailbox,recovery','Locked/unlocked loss, arbitration ownership and explicitly bounded fault recovery.'),
 'V1':('can:port variants:bank','Actual filter-register adapter plus IDE/RTR-safe list/mask packing and capacity comparison.'),
 'V2':('can:mailbox variants:bank','Actual no-auto-retry terminal loss plus oldest-drop retained-client information-loss counterexample; no impossible postloss conversion claimed.')},
 '38-010':{
 'R1':('sd:fd fd:admission','Raw metadata validated before Boolean coercion, exact DLC and capability boundaries.'),
 'R2':('sd:fd fd:wire,tdc','Selected CRC width and actual protected-stuff-count/fixed-stuffing/TDC model successor; physical timing gap retained.'),
 'R3':('sd:fd fd:controller mcp:layout,transfer','Original four-byte padded transport is distinct from actual MCP2518FD two-byte framing.'),
 'V1':('mcp:init,failure,transfer,spi,atomic','Actual selected controller SPI faults, configuration timeout, staged publication and capability/length conversion.'),
 'V2':('fd:controller mcp:bench,spi','Different TCAN/MCP power/word-framing contract plus dedicated dual-rate board protocol; no physical execution asserted.')}
}

LIMITS=['Existing scoped author observations and three new timer/transaction/retry experiments are scoped evidence, not an independent learner attempt or external review.','No physical board, peer timing, serial controller or electrical line observations are supplied.','Model profiles and changed-format analysis do not establish unexecuted target hardware behavior.','Stored generic campaign global tree hashes are historical; only current inspected-path identities and exact retained successful commands are reused.']
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
   match=((name and any(('.'+case in name and not name.endswith('.compile')) for case in cases.split(','))) or (not name and 'ctest' in q.get('command',[]) and any((('b045_systems' if key=='b45' else 'serial_depth')+'.'+case) in text for case in cases.split(','))))
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
 print('PASS10 canonical X integrations')
def package():
 values=rows();files={pathlib.Path(__file__),ROOT/'tools/check_serial_reconciliation.py',ROOT/'tools/check_serial_reconciliation_exports.py',HERE/'exports.json'}
 for suffix in MAP:
  p,_,_=brief('X-T'+suffix);files.add(p);files.add(HERE/(p.stem+'.before-reconciliation.md'))
 for report,contract in BASE.values():
  files|={ROOT/report,ROOT/contract};d=read(ROOT/report)
  for name in d.get('source_sha256',d.get('inspected_paths',{})):files.add(ROOT/name)
 for name in ['audit.md']:files.add(HERE/name)
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='All original X-T38 problem/rubric/variant identities; author dimensions only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=sorted({q[0] for q in BASE.values()}|{'research/v5/expansion/legacy_serial_reconciliation/exports.json'}),updates=values,remaining_author_work=['Mixed LRUN23/LRUN24 retain their separate scope; independent delayed learner and physical obligations remain unperformed.'],evidence_limits=LIMITS))
 print('PASS'+str(len(values))+' exact original X dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);integrate() if a.integrate else package()
if __name__=='__main__':main()
