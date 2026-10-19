"""Add request successor navigation/source records without changing old receipts."""
from pathlib import Path
import json,sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import parse,write,dump,sha,read_json
folder=Path(__file__).parent;contract='labs/b05_depth/request_adapters/CONTRACTS.md'
path=ROOT/'curriculum/v5/exercises/X-T56-001.md';meta,body=parse(path)
if contract not in meta['shared_contracts']:meta['shared_contracts'].append(contract)
record={'id':'R-REQUEST-RESET-CONTRACT','path':'research/v5/expansion/followthrough/request_adapter_resources.json','passages':['V1: derive the transition and ownership contract','V2: actual two-request virtio adapter']}
if not any(r['id']==record['id'] for r in meta['source_records']):meta['source_records'].append(record)
heading='\n## Required reset and actual-device successor\n'
if heading in body:body=body.split(heading)[0]
body+=heading+'\nBoth original V1/V2 labels are implemented by the explicit '+contract+' package: a reset/unknown-outcome model and an actual two-request virtio adapter, with compiling unsolved learner files and explicit prerequisite imports. The original core remains a bounded host model; the successor records actual QEMU observations separately. Use tools/run_request_adapters.py and request_adapter_check.json for author-reference readiness, never as an independent learner attempt. The prescribed seven-day delay still applies to learner assessment.\n'
write(path,'```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body)
analysis=ROOT/meta['assessor_analysis'];text=analysis.read_text(encoding='utf-8')
if heading in text:text=text.split(heading)[0]
write(analysis,text+heading+'\nSee assessor/b05_depth/request_adapters/ANALYSIS.md for the actual reset/queue solution, counterexamples and delayed-release package. Preserve the distinction between a device acknowledgement, a known I/O outcome and an independent learner assessment.\n')
readme=ROOT/'labs/b05_depth/README.md';text=readme.read_text(encoding='utf-8')
if heading in text:text=text.split(heading)[0]
write(readme,text+heading+'\nX-T56-001 continues through [request reset and actual virtio integration](request_adapters/CONTRACTS.md). Supply explicit prerequisite sources; the public guest builder has no assessor fallback.\n')
base={'authors':['c_mastery repository authors'],'year':2026,'kind':'manual','domains':['T56'],'role':'oracle','assignable':True}
resources=[dict(base,id='R-REQUEST-RESET-CONTRACT',title='Original reset and two-request virtio adapter contract',url=contract,
 edition='Additive successor revision1',access='Original complete contract and corresponding code inspected',
 reason='Specify request outcome/ownership separation, actual queue reuse and independent assessment.',prerequisites=['C17 lifetimes; retained kernel/virtio ABI'],
 locators=['V1: derive the transition and ownership contract','V2: actual two-request virtio adapter','Concrete verification and diagnosis'],
 inspection={'date':'2026-10-17','method':'Complete original contract and corresponding implementation/probe inspection','scope':'Bounded author implementation','limits':'No independent review/learner/physical claim','local_sha256':sha(ROOT/contract)}),
 dict(base,id='R-REQUEST-RESET-VIRTIO-12',title='Virtual I/O Device (VIRTIO) Version1.2',url='https://docs.oasis-open.org/virtio/virtio/v1.2/virtio-v1.2.html',
 authors=['Michael S. Tsirkin','Cornelia Huck'],year=2022,edition='Committee Specification01,1 July2022',role='primary',
 access='Official HTML directly accessed; exact header and bounded reset/common-configuration passages inspected',
 reason='Establish the device reset acknowledgement and queue ownership premise without attributing it to ISO C.',
 prerequisites=['PCI/MMIO and split virtqueue concepts'],locators=['2.4 Device Reset','4.1.4.3 Common configuration structure layout'],
 inspection={'date':'2026-10-17','method':'Official HTML passage read, exact edition header checked; downloaded bytes hashed',
 'scope':'2.4.1/2.4.2 reset device/driver requirements;4.1.4.3 common configuration layout',
 'limits':'Not whole-document inspection or physical-device validation','document_sha256':'2f366c5ffaf4fd52282930a2f9fe8bdc555fe76de4b32245a46bc3b5f3467649'})]
resources.append(dict(base,id='R-REQUEST-BLKDEBUG-10-2-1',title='Block I/O error injection using blkdebug',
 url='https://raw.githubusercontent.com/qemu/qemu/v10.2.1/docs/devel/testing/blkdebug.rst',
 authors=['QEMU project'],year=2026,edition='QEMU v10.2.1 source tag',role='primary',
 access='Official source documentation Rules and Events read; matching installed QEMU10.2.1 and Linux EIO header inspected',
 reason='Specify actual emulated write-error injection and its observer limits.',prerequisites=['Virtio request/status and Linux host error numbers'],
 locators=['Rules: event,errno,sector,once','Events: write_aio'],
 inspection={'date':'2026-10-17','method':'Tagged primary source passages and installed Linux errno-base.h EIO definition read',
 'scope':'Rules/Events only, not all QEMU block implementation internals','limits':'Emulated fault injection, not a physical device fault',
 'document_sha256':'8180f1cd52841162ffc6eb8909d18a90613ee7571a263946a3db3d73d08037a8'}))
dump(folder/'request_adapter_resources.json',{'schema_version':1,'resources':resources})
family=ROOT/'labs/b05_depth/family.json';data=read_json(family)
data['supplemental_campaigns']=[{'id':'request-reset-virtio','producer':'tools/run_request_adapters.py','checker':'tools/check_request_adapters.py',
 'receipt':'reviews/v5/expansion/followthrough/request_adapter_campaign.json','contract':contract,'scope':'X-T56-001 V1/V2 author-reference execution; independent learners and physical devices separate.'}]
dump(family,data)
