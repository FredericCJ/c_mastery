"""Additive service integration; original historical remaining/gate stay intact."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import parse,write,dump
contract='labs/service_depth/adapters/CONTRACTS.md'
for number in range(2,6):
    path=ROOT/f'curriculum/v5/exercises/X-T56-{number:03}.md';meta,body=parse(path)
    if contract not in meta['shared_contracts']:meta['shared_contracts'].append(contract)
    heading='\n## Executable adapter successor\n'
    if heading in body:body=body.split(heading)[0]
    body+=heading+'\nThe required D2 adapter and both original V1/V2 variants now have the explicit '+contract+' contract, compiling unsolved load.inc/services.inc/receipt.c scaffolds and the public guest.py workflow. Supply actual prerequisite imports; there is no assessor fallback. Use the source-bound service_adapter_campaign.json and service_adapter_check.json for author-reference readiness, not learner acceptance. The original seven-day independent assessment remains required. The sector-model bound and guest-versus-physical distinction are part of D3 reasoning.\n'
    write(path,'```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body)
    analysis=ROOT/meta['assessor_analysis'];text=analysis.read_text(encoding='utf-8')
    if heading in text:text=text.split(heading)[0]
    write(analysis,text+heading+'\nSee assessor/service_depth/kernel/ANALYSIS.md for actual imported-component solutions, variant invariants, counterexamples, intended mutant failures and the separate delayed-release protocol. Original portable-core criteria remain in force.\n')
readme=ROOT/'labs/service_depth/README.md';text=readme.read_text(encoding='utf-8')
heading='\n## Required kernel and durable-store adapters\n'
if heading in text:text=text.split(heading)[0]
write(readme,text+heading+'\nAfter the portable core, follow [the adapter contract](adapters/CONTRACTS.md). The public builder requires explicit kernel, adapter and store source paths and audits full images; it never locates assessor solutions. The author-only wrapper and full service campaign record reference provenance separately from subsequent learner integration.\n')
resource={'id':'R-SERVICE-ADAPTER-CONTRACT','title':'Original actual-kernel and durable-receipt adapter contract','url':contract,
 'authors':['c_mastery repository authors'],'year':2026,'edition':'Additive successor revision1','kind':'manual',
 'access':'Complete original contract and corresponding implementation/probes authored and inspected','domains':['T56'],'role':'oracle',
 'reason':'Define real imported-component ownership, guest experiments, changed conditions and exact evidence limits.',
 'prerequisites':['Retained checked ELF/frame/COW, archive and store contracts'],'assignable':True,
 'locators':['Transactional loader and restricted COW','Protected directional byte channels','Archive-backed file descriptions','Durable effect and receipt transaction','Experiments, evidence and limits'],
 'inspection':{'date':'2026-10-17','method':'Complete original contract and corresponding actual source/probe inspection.',
   'scope':'Original bounded adapter semantics; underlying ISA/device authority remains its explicitly retained source contracts.',
   'limits':'Same-author source/implementation; no independent review or physical observation.',
   'local_sha256':hashlib.sha256((ROOT/contract).read_bytes()).hexdigest()}}
dump(Path(__file__).parent/'service_adapter_resources.json',{'schema_version':1,'resources':[resource]})
