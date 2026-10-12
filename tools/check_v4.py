"""Independent identity, progression, accounting and learning-record checks."""
from __future__ import annotations
from collections import Counter, defaultdict
from copy import deepcopy
import hashlib
from common import ROOT, dump, load, read_json
from v4_common import V4, identity

EXPECTED_V4={'new_executable_units':{'MI':12,'HS':12,'FW':12,'IN':8,'EX':12},'implementation_verification_hours':3599,
    'catalog_hours':4879,'core_hours':4799,'recall_sets':627}


def inspect(data, artifacts, recall, budget, projects, taxonomy, resources, expected=None):
    expected=expected or EXPECTED_V4
    errors=[]
    def require(condition,message):
        if not condition:errors.append(message)
    units=data['units']; by_id={u['id']:u for u in units}; order=data['order']
    objectives={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    topics={t['id'] for d in taxonomy for t in d['subtopics']}
    refs={r['id'] for r in resources}; arts={a['id']:a for a in artifacts['artifacts']}
    require(len(units)==len(by_id),'Duplicate unit IDs')
    require(len(order)==len(set(order)) and set(order)==set(by_id),'Order must contain every unit once')
    covered=set();allocated=Counter();new=Counter();done=set();strand_hours=Counter()
    for key in order:
        if key not in by_id:continue
        u=by_id[key]; pid=u['project']
        require(pid in projects,key+': unknown owning project')
        require(u['budget_owner']==pid,key+': ambiguous budget owner')
        require(60<=u['minutes']<=240,key+': working-unit minutes outside 1–4 hours')
        require(abs(u['minutes']/60-u['hours'])<1e-8,key+': hours/minutes disagree')
        require(set(u['requires'])<=done,key+': forward dependency or cycle')
        require(set(u['requires'])==set(u['requires_reasons']),key+': missing dependency rationale')
        require(bool(u['objectives']) and set(u['objectives'])<=objectives,key+': invalid objective reference')
        require(set(u['recall_sets'])<=topics,key+': invalid recall set')
        require(set(u['critical_objectives'])<=objectives,key+': invalid critical objective')
        require(set(u['imports'])<=set(arts) and set(u['exports'])<=set(arts),key+': dangling artifact')
        require(u['target_scope'] in {'host','model','emulator','physical'},key+': ambiguous evidence scope')
        require(bool(u['steps']) and all(u['steps']) and bool(u['checks']) and all(u['checks']),key+': empty working card')
        for r in u['readings']:
            require(r.get('resource_id') in refs,key+': unknown reading resource')
            require(bool(r.get('locator')),key+': reading lacks bounded locator')
        if u['new_executable']:
            new[key.split('-')[1]]+=1
            require(bool(u['family']) and bool(u['tests']),key+': executable unit has no rejection gate')
            require(bool(u['remaining_evidence']),key+': executable scope lacks remaining obligations')
        if u['decision_required']:
            p=u.get('decision_prompt',{})
            require(len(p.get('alternatives',[]))>=2 and p.get('discriminator') and p.get('reversal_trigger'),key+': incomplete architecture gate')
            require(u.get('decision_before') in by_id,key+': missing prior decision boundary')
        if pid.startswith('PR-P') or pid.startswith('PR-CAP'):
            require(len(u['imports'])>=2,key+': integration does not identify two existing components')
        covered.update(u['objectives']);allocated[pid]+=u['minutes'];done.add(key)
        strand_hours[u['strand']]+=u['hours']
        require(strand_hours[u['strand']]<=24+1e-8,key+': more than 24 strand hours without adaptation')
        if u['reuse_checkpoint']:strand_hours[u['strand']]=0
    require(covered==objectives,'Missing or spurious objective activity mappings')
    require(set(allocated)==set(projects),'Missing project activity envelopes')
    for key,p in projects.items():
        require(allocated[key]==p['meta']['hours']*60,key+': unit work changes original project budget')
        local=[u for u in units if u['project']==key]
        finals=[u for u in local if u['milestone']]
        require(len(finals)==1 and finals[0].get('project_completion'),key+': missing project completion gate')
        concrete={u['family'] for u in local if u['family']}
        if len(concrete)==1:
            require(all(u['family'] for u in local),key+': family continuity cannot advance through a manual-only increment')
        for previous,current in zip(local,local[1:]):
            if previous['family']!=current['family']:
                require(bool(current.get('workspace_transition')),current['id']+': unexplained workspace transition')
        if key.startswith(('PR-P','PR-CAP')) and key.endswith('-02'):
            require(finals[0]['decision_required'],key+': integration lacks a sealed architectural decision')
        if p['meta']['module']=='INTEGRATION':
            stem,number=key.rsplit('-',1);previous=stem+'-'+f'{int(number)-1:02}'
            if previous in projects:
                prior=next(u for u in units if u['project']==previous and u['milestone'])
                require(prior['id'] in local[0]['requires'],key+': integration stage does not consume its preceding result')
        if key in {'PR-CAP-MCU-02','PR-CAP-MCU-03','PR-CAP-MCU-04'}:
            require(any(c['scope']=='physical' and c.get('target')=='stm32f439' and not c.get('source_units') for c in finals[0]['claims']),key+': integrated application requires its own current board observation')
            require(finals[0].get('capture_targets',{}).get('stm32f439',{}).get('manual_only'),key+': integrated capture registration is missing')
    cap=next(u for u in units if u['project']=='PR-CAP-MCU-04' and u['milestone'])
    physical=[c for c in cap['claims'] if c['scope']=='physical']
    interfaces={i for c in physical for i in c.get('interfaces',[])}
    require(interfaces=={'UART','USART','I2C','SPI','RS-232','RS-422','RS-485','CAN','CAN FD','Ethernet'},'Final physical interface obligations differ')
    require({t for c in physical for t in c.get('targets',[])}=={'stm32f439','stm32h745'},'Final board obligations differ')
    for u in units:
        for claim in u['claims']:
            require(claim['scope'] in {'host','model','emulator','physical'},u['id']+': unknown claim scope')
            sources=claim.get('source_units',[])
            require(set(sources)<=set(u['requires']),u['id']+': portfolio evidence is not an actual prerequisite')
            for source in sources:
                require(any(c['scope']==claim['scope'] for c in by_id[source]['claims']),u['id']+': portfolio source has no matching scoped evidence')
            if claim.get('target')=='interface-bench' and not sources:
                declared=u.get('capture_targets',{}).get('interface-bench',{})
                require(declared.get('physical') and declared.get('manual_only'),u['id']+': physical capture target is undeclared')
                require(set(claim['interfaces'])<=set(declared.get('interfaces',[])),u['id']+': claimed interface is not declared')
        if u['family']=='kernel_smp' and u['milestone']:
            require(any(c['scope']=='emulator' and c.get('target')=='x86-smp' and c.get('cpus')==[1,2,4] for c in u['claims']),u['id']+': SMP target matrix is not required')
    require(new==expected['new_executable_units'],'Expected 56 newly executable units by declared investment: '+str(dict(new)))
    require(sum(allocated.values())==expected['implementation_verification_hours']*60,'Implementation/verification total changed')
    require(budget['catalog_hours']==expected['catalog_hours'] and budget['core_hours']==expected['core_hours'],'Planning ceilings changed')
    require(sum(budget['categories'].values())==expected['catalog_hours'],'Category conservation does not reconcile')
    for art in arts.values():
        producer=by_id.get(art['producer_unit'])
        require(producer is not None,art['id']+': missing producer')
        if producer:require(art['id'] in producer['exports'],art['id']+': producer does not export contract')
        require(bool(art['public_contract']) and bool(art['contract_version']),art['id']+': unnamed contract')
        require(set(art['critical_objectives'])<=objectives,art['id']+': invalid critical contract facet')
        if producer:require(set(art['critical_objectives'])<=set(producer['objectives']),art['id']+': critical facet not taught by producer')
        require(set(art['consumers'])=={u['id'] for u in units if art['id'] in u['imports']},art['id']+': lineage differs from real imports')
    sets=recall['sets']
    vocabulary_ids={s['id'] for s in sets if s.get('kind')=='vocabulary'}
    require({s['id'] for s in sets}-vocabulary_ids==topics and len(sets)==expected['recall_sets'],'Recall set coverage must be 627 unique topics')
    prompts=set()
    for s in sets:
        if s.get('kind')=='vocabulary':
            require(s['id'].startswith('V-') and bool(s['objectives']) and all(o.startswith(s['id']+'-') for o in s['objectives']),s['id']+': vocabulary facets must be term identities')
        else:
            require(set(s['objectives'])=={x for x in objectives if x.rsplit('.',1)[0]==s['id']},s['id']+': recall facets differ')
        require(len(s['variants'])==3,s['id']+': missing rotated variants')
        require(len({v['id'] for v in s['variants']})==3,s['id']+': duplicate variant identity')
        for v in s['variants']:
            require(v['prompt'] not in prompts,s['id']+': duplicate uncontextualized prompt')
            prompts.add(v['prompt'])
            require(set(v['objectives'])<=set(s['objectives']) and bool(v['checklist']),s['id']+': invalid scoring coverage')
    elapsed=0;tool=None;boundary=None;diagnosis=0
    for key in order:
        u=by_id[key];elapsed+=u['hours']
        if key==order[0] and u['project']=='PR-T01-01':diagnosis=u['hours']
        if 'records.tool' in u['exports']:tool=elapsed-diagnosis
        if key=='LAB-HS-02':boundary=elapsed-diagnosis
        if tool is not None and boundary is not None:break
    require(tool is not None and tool<=12,'First bounded tool is not reachable within 12 assigned hours')
    require(boundary is not None and boundary<=48,'Real file boundary is not reachable within 48 assigned hours')
    return errors,{'units':len(units),'new_executable_units':sum(new.values()),'topics':len(sets),
        'objectives':len(covered),'artifacts':len(arts),'first_tool_hours_after_diagnostic':tool,
        'first_real_file_boundary_hours_after_diagnostic':boundary,'project_hours':sum(allocated.values())/60}


def main():
    taxonomy,modules,projects,resources=load()
    data=read_json(V4/'units.json');arts=read_json(V4/'artifacts.json');recall=read_json(V4/'recall.json');budget=read_json(V4/'budget.json')
    errors,counts=inspect(data,arts,recall,budget,projects,taxonomy,resources)
    baseline=read_json(V4/'baseline.json')
    for name,expected in baseline['files'].items():
        path=ROOT/name
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest()!=expected:
            errors.append('Historical v3 teaching/identity changed: '+name)
    # Counterexamples exercise each structural guard against a real coherent catalog.
    guards=[]
    for label,change in [
        ('budget',lambda d:d['units'][0].update(minutes=500)),
        ('dependency-cycle',lambda d:d['units'][0]['requires'].append(d['units'][0]['id'])),
        ('unknown-objective',lambda d:d['units'][0]['objectives'].append('T99.99.a')),
        ('missing-unit',lambda d:d['order'].pop()),
        ('missing-executable-test',lambda d:next(u for u in d['units'] if u['new_executable']).update(tests=[])),
        ('missing-physical-gate',lambda d:next(u for u in d['units'] if u['project']=='PR-CAP-MCU-04' and u['milestone']).update(claims=[])),
        ('missing-smp-gate',lambda d:next(u for u in d['units'] if u['family']=='kernel_smp' and u['milestone']).update(claims=[])),
        ('missing-integrated-deployment',lambda d:next(u for u in d['units'] if u['project']=='PR-CAP-MCU-02' and u['milestone']).update(claims=[])),
        ('missing-integrated-recovery',lambda d:next(u for u in d['units'] if u['project']=='PR-CAP-MCU-03' and u['milestone']).update(claims=[])),
        ('portfolio-substitutes-local-capture',lambda d:next(c for u in d['units'] if u['project']=='PR-CAP-MCU-04' and u['milestone'] for c in u['claims'] if c['id'].endswith('.integrated-physical')).update(source_units=['U-T50-12-04'])),
    ]:
        sample=deepcopy(data);change(sample)
        rejected=bool(inspect(sample,arts,recall,budget,projects,taxonomy,resources)[0]);guards.append({'fault':label,'rejected':rejected})
        if not rejected:errors.append('Guard accepted '+label)
    report={'schema_version':4,'status':'passed' if not errors else 'failed','passed':not errors,
        'input_sha256':identity()['sha256'],'counts':counts,'historical_files':len(baseline['files']),
        'negative_controls':guards,'errors':errors,'scope':'Authored progression, exact accounting and preserved coverage; actual execution and learner evidence are separate.'}
    dump(ROOT/'reviews/v4/structure.json',report)
    print({**report,'errors':errors[:25],'error_count':len(errors)})
    if errors:raise SystemExit(1)


if __name__=='__main__':main()
