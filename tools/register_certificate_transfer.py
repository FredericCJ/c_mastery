"""Attach the three original T36 exercise successors without changing their rubrics."""
import json,pathlib
from common import ROOT,dump,sha
BASE='research/v5/expansion/certificate_transfer';IDS=['X-T36-001','X-T36-002','X-T36-003']
HEADERS=['labs/algorithm_frontier/frontier.h','labs/algorithm_tree/tree.h','labs/algorithm_lookup/lookup.h','labs/algorithm_depth/algorithms.h','labs/containers_depth/containers.h','labs/algorithms/include/algo.h']
def main():
 record=dict(id='R-X36-TRANSFER',title='Original certificate and callback-stream transfer contracts',url='labs/certificate_transfer/CONTRACTS.md',authors=['Curriculum authors'],edition='Original C17 companion2026-10-18',year=2026,kind='manual',access='Actual specified source passages inspected',domains=['T36'],role='primary',reason='Complete original streamed-edge, explicit-stack and callback-stream variants with exact frozen-provider composition.',prerequisites=['Typed storage and byte decoding','Shortest-path and tree certificate proofs','Completed prefix matcher and callback ownership'],assignable=True,locators=['labs/certificate_transfer/CONTRACTS.md: all sections','labs/match_transfer/CONTRACTS.md: all sections'],inspection=dict(date='2026-10-18',method='Actual complete named contracts and implementation definitions read; no bibliography-derived source credit',scope='New mechanisms and exact retained provider contracts',limits='Author inspection only; published C17/C23, independent performance and physical evidence remain separate.'))
 clrs=json.loads((ROOT/'research/v5/expansion/legacy_algorithm_tree/resources.json').read_text())['resources'][0]
 clrs['id']='R-X36-CLRS13';clrs['locators']=['CLRS fourth edition13.1: color/NIL properties, black-height convention and inductive subtree-count argument; build/containers-depth/clrs13.txt lines1–90'];clrs['inspection']['scope']=clrs['locators'][0];clrs['inspection']['method']='Actually read the existing extracted passage; preserve original edition and verify captured identities.'
 dump(ROOT/BASE/'resources.json',dict(resources=[record,clrs]))
 passages={
 'assessor/algorithm_frontier/frontier.c':['graph and complete ag_certificate; finite signed labels, rooted witnesses and reachable edge inequalities'],
 'assessor/algorithm_tree/certificate.c':['Complete at_check/visit; tree adapters and duplicate-policy wrapper'],
 'assessor/algorithm_lookup/lookup.c':['enumerate,al_match,al_publish,al_stream_init/feed'],
 'assessor/algorithm_depth/matching.c':['ad_match_init/feed and independent naive-window provider'],
 'assessor/containers_depth/indexed.c':['cd_csr_build'],
 'labs/algorithm_tree/CONTRACTS.md':['Certificate policies and actual retained provider differences'],
 'labs/algorithm_lookup/CONTRACTS.md':['Callback effect boundary, stream consumption and changed count-on-failure'],
 'labs/certificate_transfer/CONTRACTS.md':['Complete contract'],
 'labs/match_transfer/CONTRACTS.md':['Complete contract']}
 for id in IDS:
  file=ROOT/'curriculum/v5/exercises'/(id+'.md');s=file.read_text(encoding='utf8');prefix,rest=s.split('```json\n',1);encoded,tail=rest.split('\n```',1);d=json.loads(encoded)
  history=ROOT/BASE/'history'/(id+'.before.md');history.parent.mkdir(parents=True,exist_ok=True)
  if not history.exists():history.write_bytes(file.read_bytes())
  folder='labs/match_transfer' if id.endswith('003') else 'labs/certificate_transfer';contract=folder+'/CONTRACTS.md';assets=[p.relative_to(ROOT).as_posix() for p in (ROOT/folder).iterdir() if p.is_file() and p.suffix in ['.c','.h','.py','.md']]+HEADERS
  d['fixtures']=list(dict.fromkeys(d['fixtures']+assets));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[contract]))
  d['source_records']=[r for r in d['source_records'] if r['id']!=record['id']]+[dict(id=record['id'],path=BASE+'/resources.json',passages=record['locators'])]
  d['status']['required_certificate_followthrough']=dict(contract=contract,dispositions=BASE+'/completion_dispositions.json',report=('research/v5/expansion/match_transfer/validation.json' if id.endswith('003') else BASE+'/validation.json'),scope='Exact original root/rubric/variant bindings; author execution separate from learner, external review and physical evidence.')
  marker='\n## Required original T36 transfer companion\n'
  if marker in tail:tail=tail.split(marker,1)[0]
  tail+=marker+'\nComplete the applicable [required transfer companion](../../../'+contract+'), retaining the original exercise gate and rubric. Import your actual prerequisite components explicitly. The companion supplies concrete missing changed-condition implementations and verification, not learner credit or a replacement for the original contracts.\n'
  file.write_text(prefix+'```json\n'+json.dumps(d,ensure_ascii=False,indent=2)+'\n```'+tail,encoding='utf8',newline='\n')
  old=json.loads(history.read_text(encoding='utf8').split('```json\n',1)[1].split('\n```',1)[0]);assert all(d[k]==old[k] for k in ['id','project','objectives','rubric','delayed_transfer','objective_coverage'])
  passages[file.relative_to(ROOT).as_posix()]=['Complete original problem, objectives, rubrics and both V1/V2 clauses; additive companion only']
 dump(ROOT/BASE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={BASE+'/resources.json':sha(ROOT/BASE/'resources.json')},source_sha256={p:sha(ROOT/p) for p in passages},passages=passages,external_source_sha256=clrs['inspection']['external_source_sha256'],limits=['Bounded listed passages only; no whole-book or final ISO text inspection.','Original mathematical derivations and source behavior are separate from externally enforced guarantees.']))
 print(dict(attached=IDS,original_criteria_preserved=True))
if __name__=='__main__':main()
