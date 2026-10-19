"""Additive canonical project/source attachment; preserves original project body and IDs."""
import hashlib
import importlib.util
import json
import pathlib
import re
import sys

ROOT=pathlib.Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/cacheable_boot_depth'
LAB=ROOT/'labs/cacheable_boot_depth'
PROVIDER=ROOT/'labs/firmware_lifecycle/targets'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def main():
    imported=['h745_cache.h','selected_startup.h','h745_flash.h','boot_storage.h','boot_pair.h','ecc_guard.h','flash_maintenance.h','selected_startup.c','h745_cache.c']
    (LAB/'imports').mkdir(exist_ok=True)
    for n in imported:(LAB/'imports'/n).write_bytes((PROVIDER/n).read_bytes())
    write(HERE/'import_identity.json',dict(scope='Unsolved public prerequisite sources and exact headers only; never assessor implementations.',files={n:sha(PROVIDER/n) for n in imported}))
    attachment=dict(project='PR-T59-07',shared_contracts=['labs/cacheable_boot_depth/CONTRACTS.md','labs/cacheable_boot_depth/BOARD_CAPTURE.md'],supplied_artifacts=[p.relative_to(ROOT).as_posix() for p in sorted(LAB.rglob('*')) if p.is_file() and '__pycache__' not in p.parts],support='research/v5/expansion/cacheable_boot_depth/completion_dispositions.json',body='\n\n## Required cacheable cold-start successor\n\nThe original transfer now has an actual cold-start/linker/MPU implementation task in [the cacheable boot contract](../../labs/cacheable_boot_depth/CONTRACTS.md), with an explicit [dedicated-board capture protocol](../../labs/cacheable_boot_depth/BOARD_CAPTURE.md). Implement the supplied successor from its unsolved source and the actual completed prerequisite providers. Retain the original physical transfer criterion: host/model execution and a linked target do not supply physical cache/DMA observations.\n')
    write(HERE/'attachment.json',attachment)
    sys.path.insert(0,str(ROOT/'research/v5/expansion/b06'))
    from author_domains import attach_project_support
    project=ROOT/'curriculum/projects/PR-T59-07.md';text=project.read_text(encoding='utf8');m=re.match(r'```json\n(.*?)\n```\n\n(.*)',text,re.S);assert m
    old=HERE/'PR-T59-07.before-cacheable.md'
    if not old.exists():old.write_bytes(project.read_bytes())
    meta,body=attach_project_support('PR-T59-07',json.loads(m[1]),m[2]);project.write_text('```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body,encoding='utf8')
    base=json.loads((ROOT/'research/v5/expansion/b06/followup_resources.json').read_text(encoding='utf8'))
    source=dict(id='R-CACHEBOOT-PM0253-MPU-COLD',title='PM0253 selected MPU mapping and cold data-cache initialization',authors=['STMicroelectronics'],edition='PM0253 revision5, March2020',year=2020,kind='manual',url='https://www.st.com/resource/en/programming_manual/DM00237416.pdf',access='Actual private primary PDF inspected at exact bounds',domains=['T59'],role='primary',reason='Derive selected32-byte MPU map and complete invalidate-before-enable sequence',prerequisites=['Cache-line ownership','Privileged cold reset'],locators=['pp.219-220 CCSIDR field encodings and five four-way data cache geometries','pp.224-230 MPU control, region size/base, memory attributes and update barriers','pp.240-244 address/set-way maintenance and invalidate-before-enable'],assignable=True,inspection=dict(date='2026-10-18',method='Actual primary PDF text read; page228 table visually inspected',scope='pp.219-220 CCSIDR field encodings and five four-way data cache geometries; pp.224-230 MPU control, region size/base, memory attributes and update barriers; pp.240-244 address/set-way maintenance and invalidate-before-enable',limits='Selected revision only; not whole manual, latest-silicon applicability or observed physical behavior',private_payload='build/execution-sources/PM0253-r5.pdf',local_sha256=sha(ROOT/'build/execution-sources/PM0253-r5.pdf')))
    shared=[dict(id=i,path='research/v5/expansion/b06/followup_resources.json',passages=next(r for r in base['resources'] if r['id']==i)['locators']) for i in ['R-B06-FOLLOWUP-RM0399-STARTUP','R-B06-FOLLOWUP-RM0399-HSEM']]
    write(HERE/'resources.json',dict(resources=[source],canonical_external_sources=shared))
    gatepaths=['research/v5/expansion/cacheable_boot_depth/resources.json','research/v5/expansion/b06/followup_resources.json']
    write(HERE/'research_gate.json',dict(status='passage-inspected',research_input_sha256={p:sha(ROOT/p) for p in gatepaths},limits='Author bounded primary inspection only; no physical or independent acceptance.'))
    print('Registered cacheable source and exact additive PR-T59-07 attachment')
if __name__=='__main__':main()
