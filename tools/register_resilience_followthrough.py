"""Canonical additive T75 successor routing; no historical acceptance writer."""
import json
import hashlib
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
ROUTES={
    'X-T75-001':{'tests':['resilience.scope_follow'],'contract':'labs/resilience/followthrough/SCOPE.md'},
    'X-T75-004':{'tests':['resilience.fractional_follow'],'contract':'labs/resilience/followthrough/FRACTIONAL.md'},
    'X-T75-003':{'tests':['resilience.timer_structures'],'contract':'labs/resilience/followthrough/TIMERS.md','analysis':'TIMERS_ANALYSIS.md'},
    'X-T75-005':{'tests':['resilience.retry_transport'],'contract':'labs/resilience/followthrough/RETRY.md','analysis':'RETRY_ANALYSIS.md'},
    'X-T75-002':{'tests':['resilience.pressure_owned'],'contract':'labs/resilience/followthrough/PRESSURE.md','analysis':'PRESSURE_ANALYSIS.md'},
    'X-T75-006':{'tests':['resilience.breaker_window'],'contract':'labs/resilience/followthrough/BREAKER.md','analysis':'BREAKER_ANALYSIS.md'},
    'X-T75-007':{'tests':['resilience.hedge_owned'],'contract':'labs/resilience/followthrough/HEDGE.md','analysis':'HEDGE_ANALYSIS.md'},
    'X-T75-008':{'tests':['resilience.receiver_journal'],'contract':'labs/resilience/followthrough/JOURNAL.md','analysis':'JOURNAL_ANALYSIS.md'},
    'X-T75-009':{'tests':['resilience.service_outbox','resilience.service_io','resilience.service_transport'],'contract':'labs/resilience/followthrough/OUTBOX.md','additional_contracts':['labs/resilience/followthrough/SERVICE_IO.md','labs/resilience/followthrough/COMPOSITION.md','labs/resilience/followthrough/COST.md'],'analysis':'OUTBOX_ANALYSIS.md'},
}
BREAKER_PASSAGES=['Count-based sliding window: circular measurements and subtract-on-eviction',
                  'Failure rate and slow call rate thresholds: minimum observations, ignored outcomes and half-open admission']
JOURNAL_SOURCES=[
    ('R-MOD-RZ-FSYNC','fsync(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/fsync.2.html',
     ['DESCRIPTION: successful file synchronization, fdatasync metadata and separate directory synchronization','RETURN VALUE and ERRORS: delayed writeback and synchronization failure']),
    ('R-MOD-RZ-WRITE','write(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/write.2.html',
     ['DESCRIPTION and RETURN VALUE: partial progress and retry suffix','NOTES: successful write does not establish durable storage']),
    ('R-MOD-RZ-PREAD','pread(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/pread.2.html',
     ['DESCRIPTION and RETURN VALUE: explicit offsets and partial transfers','BUGS: Linux O_APPEND overrides pwrite offset']),
]
SERVICE_SOURCES=[
    ('R-MOD-RZ-POLL','poll(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/poll.2.html',
     ['DESCRIPTION: readiness, POLLOUT versus nonblocking progress, POLLHUP with buffered input and timeout rounding','RETURN VALUE and ERRORS: timeout, error and EINTR']),
    ('R-MOD-RZ-SEND','send(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/send.2.html',
     ['DESCRIPTION: connected socket sends, nonblocking partial progress and no implicit delivery confirmation','FLAGS and ERRORS: per-call MSG_NOSIGNAL, EAGAIN or EWOULDBLOCK, EINTR and EPIPE']),
    ('R-MOD-RZ-SOCKETPAIR','socketpair(2), Linux man-pages 6.19','https://man7.org/linux/man-pages/man2/socketpair.2.html',
     ['DESCRIPTION and RETURN VALUE: unnamed connected endpoints and output ownership','HISTORY: SOCK_CLOEXEC and SOCK_NONBLOCK availability']),
]

def append(values, additions):
    for item in additions:
        if item not in values:values.append(item)

def main():
    source_path=ROOT/'research/v5/modern/resilience_resources.json'
    sources=json.loads(source_path.read_text(encoding='utf8'))
    for identity,title,url,locators in JOURNAL_SOURCES+SERVICE_SOURCES:
        record={'id':identity,'title':title,'authors':['Linux man-pages project'],'edition':'Linux man-pages 6.19','year':2026,
                'kind':'manual','url':url,'access':'Public primary passages opened and read','domains':['T75'],'role':'primary',
                'reason':('Bound actual local journal I/O, persistence and failure assumptions' if identity in {row[0] for row in JOURNAL_SOURCES} else 'Bound actual local stream readiness, partial sends and per-call failure containment'),
                'prerequisites':['Partial I/O','Explicit device ownership'],
                'locators':locators,'assignable':True,
                'inspection':{'date':'2026-10-17','method':'Actual primary page opened and read with web tool',
                              'scope':'; '.join(locators),'limits':'Named passages only; no physical power-loss test or hardware flush guarantee inferred.',
                              'evidence':'research/v5/modern/resilience_followthrough_sources.md','evidence_urls':[url]}}
        prior=next((row for row in sources['resources'] if row['id']==identity),None)
        if prior is None:sources['resources'].append(record)
        elif prior!=record:raise ValueError('Conflicting journal source identity '+identity)
    breaker=next(row for row in sources['resources'] if row['id']=='R-MOD-RZ-BREAKER')
    append(breaker['locators'],BREAKER_PASSAGES)
    addition={'date':'2026-10-17','method':'Opened and read actual primary documentation with web tool',
              'scope':'; '.join(BREAKER_PASSAGES),'limits':'Bounded page passages only; no external implementation or timing equivalence claimed.',
              'evidence':'research/v5/modern/resilience_followthrough_sources.md'}
    append(breaker['inspection'].setdefault('additional_inspections',[]),[addition])
    source_path.write_text(json.dumps(sources,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
    gate_path=ROOT/'research/v5/modern/resilience_research_gate.json';gate=json.loads(gate_path.read_text(encoding='utf8'))
    for source in ['research/v5/modern/resilience_resources.json','research/v5/modern/resilience_followthrough_sources.md']:
        gate['research_input_sha256'][source]=hashlib.sha256((ROOT/source).read_bytes()).hexdigest()
    gate_path.write_text(json.dumps(gate,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
    path=ROOT/'labs/resilience/family.json';family=json.loads(path.read_text(encoding='utf8'))
    append(family['dependencies'],['execution','communications','hosted_systems'])
    imported={'family':'execution','artifact':'execution.tasks','producer':'PR-T50-04',
              'learner_source':'labs/execution/src/cmx.c','include':'labs/execution/include',
              'assessor_source':'assessor/execution/cmx.c','contract':'curriculum/projects/PR-T50-05.md',
              'scope':'Actual fixed-array CMX due/cancel outcome intersection; new ordered alternatives have a stronger explicit event contract.'}
    if not any(row['family']=='execution' for row in family['source_imports']):family['source_imports'].append(imported)
    imported={'family':'communications','artifact':'communications.tcp_fault','producer':'PR-T41-06',
              'learner_source':'labs/communications/learner/v4_com.c','include':'labs/communications/include',
              'assessor_source':'assessor/communications/v4_com.c','contract':'labs/communications/units/LAB-EX-09.md',
              'scope':'Actual retained NetStack retransmission/net_receive completion via v4_tcp_delivery; explicit net/http/edu7 source dependencies, no production TCP or physical network claim.'}
    if not any(row['family']=='communications' for row in family['source_imports']):family['source_imports'].append(imported)
    for artifact,producer,part,contract in [
            ('io.deadline','PR-T24-05','deadline','labs/hosted_systems/CONTRACTS.md'),
            ('io.reactor','PR-T24-04','reactor','labs/hosted_systems/CONTRACTS.md')]:
        imported={'family':'hosted_systems','artifact':artifact,'producer':producer,
                  'learner_source':f'labs/hosted_systems/src/{part}.c','include':'labs/hosted_systems/include',
                  'assessor_source':f'assessor/hosted_systems/{part}.c','contract':contract,
                  'scope':'Actual read-until cancellation/deadline and completion-byte aggregation only; new framed partial-write adapter is a separate implementation.'}
        if not any(row['artifact']==artifact for row in family['source_imports']):family['source_imports'].append(imported)
    family['campaign']['fixture_oracle']='tools/verify_resilience_followthrough.py'
    family['campaign']['oracle_driver']='resilience-foundation'
    for exercise,route in ROUTES.items():
        row=next(row for row in family['exercises'] if row['id']==exercise)
        append(row['tests'],route['tests']);append(family['tests'],route['tests'])
        project=row['project']
        append(next(row for row in family['projects'] if row['id']==project)['tests'],route['tests'])
        for unit in family['units']:
            if unit['project']==project:append(unit['tests'],route['tests'])
        brief=ROOT/'curriculum/v5/exercises'/f'{exercise}.md';text=brief.read_text(encoding='utf8')
        start=text.index('```json\n')+8;end=text.index('\n```',start)
        meta=json.loads(text[start:end]);append(meta['lab']['tests'],route['tests'])
        if exercise=='X-T75-006':
            append(next(row for row in meta['source_records'] if row['id']=='R-MOD-RZ-BREAKER')['passages'],BREAKER_PASSAGES)
        if exercise=='X-T75-008':
            append(meta['source_records'],[{'id':identity,'path':'research/v5/modern/resilience_resources.json','passages':locators}
                                            for identity,_,_,locators in JOURNAL_SOURCES])
        if exercise=='X-T75-009':
            append(meta['source_records'],[{'id':identity,'path':'research/v5/modern/resilience_resources.json','passages':locators}
                                            for identity,_,_,locators in JOURNAL_SOURCES+SERVICE_SOURCES])
            append(meta['prerequisite_artifacts'],['io.deadline','io.reactor'])
            prior='preserve exactly-once notification for a new retained request and explicit replay/expiry semantics.'
            corrected='preserve one durable event for each new retained request, explicit replay/expiry semantics and at-least-once delivery across a pre-ACK restart.'
            for criterion in meta['rubric']:criterion['criterion']=criterion['criterion'].replace(prior,corrected)
        append(meta['shared_contracts'],[route['contract']])
        append(meta['shared_contracts'],route.get('additional_contracts',[]))
        if exercise=='X-T75-003':append(meta['prerequisite_artifacts'],['execution.tasks'])
        body=text[end+4:];marker='\n## Executable successor support\n'
        if exercise=='X-T75-009':body=body.replace(prior,corrected)
        if marker in body:body=body[:body.index(marker)]
        body+=marker+'\nThe required mechanism extension is specified in ['+Path(route['contract']).stem+'](../../../'+route['contract']+'). '
        body+='Implement the named learner sources and preserve imported contracts. Run the exact additional anchors and independent arithmetic/ownership oracle. Author executions support these bounded mechanisms; independent learner transfer, physical clock assumptions and broader integration have separate evidence requirements.\n'
        brief.write_text('```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+body,encoding='utf8')
        analysis=ROOT/meta['assessor_analysis'];text=analysis.read_text(encoding='utf8')
        marker='\n## Required successor analysis\n'
        if marker in text:text=text[:text.index(marker)]
        analysis_name=route.get('analysis','FOUNDATION_ANALYSIS.md')
        text+=marker+'\nSee ['+Path(analysis_name).stem+'](../followthrough/'+analysis_name+') for the actual successor representation, controls, changed-clock proof and remaining enforcement limits.\n'
        analysis.write_text(text,encoding='utf8')
    family['scope']='Retained virtual models plus owned native concurrency, local framed I/O and bounded durable journal/outbox successors. Exact executed scopes, external assessment and physical evidence remain separate.'
    path.write_text(json.dumps(family,indent=2,ensure_ascii=False)+'\n',encoding='utf8')

if __name__=='__main__':main()
