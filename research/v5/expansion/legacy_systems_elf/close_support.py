"""Close only exact T09 author dimensions; preserve historical and learner evidence."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha
from check_systems_elf import validate
from expansion_evidence import require_batch_research
folder=Path(__file__).parent;require_batch_research(folder)
receipt='reviews/v5/expansion/legacy_systems_elf/full_validation.json';check='reviews/v5/expansion/legacy_systems_elf/validation_check.json'
report=read_json(ROOT/receipt);errors=validate(report)
if errors:raise SystemExit(str(errors))
guard=read_json(ROOT/check)
if not guard.get('passed') or guard.get('input_sha256',{}).get(receipt)!=sha(ROOT/receipt):raise SystemExit('Current rejection-guard receipt required')
public='labs/machine_image/elf_depth/CONTRACTS.md';analysis='assessor/legacy_systems/elf_depth/ANALYSIS.md'
support=folder/'support.json';relative=support.relative_to(ROOT).as_posix()
observations={(r['objective'],r['variant']):r for r in report['observations']};updates=[];criteria=[];canonical=[]
resources=read_json(folder/'resources.json')['resources']
source_records=[{'id':r['id'],'path':'research/v5/expansion/legacy_systems_elf/resources.json','passages':r['locators']} for r in resources]
public_inputs=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/machine_image/elf_depth').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T09-*.json')):
    record=read_json(path);origin=path.relative_to(ROOT).as_posix()
    record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public_inputs))
    existing={r['id'] for r in record['source_records']};record['source_records'] += [r for r in source_records if r['id'] not in existing]
    for criterion in record['criteria']:
        oid=criterion['objective'];criteria.append({'id':criterion['id'],'objective':oid,'project':record['project'],'original_source':origin,'task':criterion['task'],'teaching':[public,origin],'assessment_analysis':[analysis,record['assessor_analysis']],'ordinary_observation':observations[oid,False],'transfer_observation':observations[oid,True],'independent_assessment':'not-assessed; original minimum7days unchanged','receipt':receipt})
        for ident,variant in [(criterion['id'],False),*[(v['id'],True) for v in criterion['variants']]]:
            obs=observations[oid,variant];module='decoder' if oid.startswith('T09.01') else 'linker' if int(oid.split('.')[1])<=4 else 'dynamic' if int(oid.split('.')[1])<=7 else 'interfaces' if oid.startswith('T09.11') else 'layout'
            updates.append({'id':ident,'original_source':origin,'original_objective':oid,'project':record['project'],'dimensions':{
              'teaching':{'state':'complete','evidence':[public,origin,analysis]},
              'implementation':{'state':'complete-for-declared-reference-contract','evidence':['assessor/legacy_systems/elf_depth/'+module+'.py']},
              'author_validation':{'state':'passed-with-stated-scope','evidence':[receipt,check]},
              'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},
              'physical_observation':{'state':'not-applicable-to-declared-host-or-nonexecuted-layout-contract','evidence':[]}},
              'closure_rationale':obs['claim'],'required_environment':'Linux x86-64 GCC/Clang/binutils/GDB; Arm GCC for decoder object closure; original child-only loader/guard-page specimens.',
              'validation_method':'Exact source/argv/input identities, independent decoded records and arithmetic, actual linker/loader/compiler failures, retained-stage matrix, late-thread TLS, old/new ABI matrix and raw paired cost samples.',
              'evidence_limits':obs['limitations']+report['limits']})
        criterion['successor_support']={'path':relative,'criterion_id':criterion['id']}
    record['successor_support']={'path':relative,'criterion_ids':[r['id'] for r in record['criteria']],'scope':'Source-bound author experiments and assessor transfer packages; independent learner evidence remains separate.'}
    dump(path,record);canonical.append(origin)
parent={'id':'LSYS-EXEC-T09','dimensions':{k:dict(v) for k,v in updates[0]['dimensions'].items()},'closure_rationale':'All36 original criteria and36 changed conditions have distinct implemented/analysis packages and observed evidence. No one decoder test substitutes for loader/TLS/retention/ABI work.','evidence_limits':report['limits'],'criteria':[r['id'] for r in criteria]}
parent['dimensions']['implementation']['evidence']=['assessor/legacy_systems/elf_depth/'+m+'.py' for m in ('decoder','linker','dynamic','layout','interfaces')]
updates.append(parent)
pins=[public,analysis,receipt,check,*canonical,'research/v5/expansion/legacy_systems_elf/resources.json']
dump(support,{'schema_version':1,'required':True,'scope':'All36 original T09 supplemental criteria and36 transfers plus LSYS-EXEC-T09; original snapshots retained.','source_sha256':{p:sha(ROOT/p) for p in pins},'criteria':criteria,'updates':updates,'source_records':source_records})
dump(folder/'gap_ledger.json',{'schema_version':1,'required':True,'gaps':[
 {'id':'LSYS-EXEC-T09','originating_record':'research/v5/expansion/legacy_systems/gap_ledger.json','status':'author-implemented-validated; external-dimensions-separate','requirement':'All36 original T09 criteria and their36 changed conditions','support':relative,'remaining':['Non-author technical review','Independent ordinary and delayed learner attempts']},
 {'id':'LSYS-ABI-GABI-ELF-CORE','parent_id':'LSYS-ABI-GABI','status':'resolved-for-declared-inspector-and-relocation-scope','requirement':'Primary ELF header/section/string/symbol/relocation/program-header passages and AMD64 relocation rules','evidence':'research/v5/expansion/legacy_systems_elf/resources.json','limits':'Bounded chapter passages; not complete processor extensions or current-vs-historical ABI equivalence.'},
 {'id':'LSYS-RUNTIME-TLS-INIT','parent_id':'LSYS-RUNTIME','status':'resolved-for-tagged-2.42-body-analysis-and-observed-host-interface','requirement':'Template allocation/init/deallocation and init/fini consumer source bodies','evidence':'research/v5/expansion/legacy_systems_elf/resources.json','limits':'Observed Ubuntu glibc2.43 is not the inspected2.42 source build; exact-host body attribution remains unverified.'},
 {'id':'LSYS-RUNTIME-HOST-243','parent_id':'LSYS-RUNTIME','status':'required-source-version-correlation','requirement':'Obtain authorized exact Ubuntu glibc2.43-2ubuntu2.4 source before claiming internal source-to-binary equivalence','attempts':['Observed ldd version in full receipt','GitHub glibc-2.43/elf/dl-reloc-static-pie.c returned404'],'completed':['Declared-interface experiments','Tagged2.42 startup/TLS body analysis'],'affected_claim':'Only exact observed-host internal attribution; unrelated T09 implementation is complete.'},
 {'id':'LSYS-FORMATS-DEBUGLINK-HEX','parent_id':'LSYS-FORMATS','status':'resolved-for-declared-companion-and-sparse-format-scope','requirement':'GDB debuglink/build-ID/CRC and Keil HEX record/checksum/address descriptions','evidence':'research/v5/expansion/legacy_systems_elf/resources.json','limits':'Keil vendor description is not the original Intel specification; supported decoder subset is explicit. Microchip maintenance page was not inspected content.'}],
 'historical_note':'Original ledgers retain their earlier snapshot. This successor closes only demonstrated dimensions and preserves source-access/version limits.'})
statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
for row in updates:index[row['id']]=row
state['updates']=list(index.values());dump(statepath,state)
print({'criteria':len(criteria),'author_dispositions':len(updates),'independent_learner_claims':False})
