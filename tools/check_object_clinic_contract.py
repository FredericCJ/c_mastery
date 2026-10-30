"""Strict one-card historical projection plus current versioned-contract/export guard."""
import argparse,copy,hashlib,json,pathlib
from close_object_clinic_contract import ROOT,HERE,OLD,CARD,CONTRACT,history,rows,attach
import check_object_clinic as prior
import run_object_clinic as campaign
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def historical_errors():
    old=read(HERE/'historical_completion_dispositions.json');r=read(OLD/'validation.json');saved_read,saved_sha=prior.read,prior.sha
    original_card=HERE/'before_contract_v2.json'
    assert sha(original_card)==old['source_sha256'][CARD]
    original_cli=HERE/'historical_cli.py'
    assert sha(original_cli)==old['source_sha256']['cmastery/cli.py']==read(HERE/'historical_exports.json')['source_sha256']['cmastery/cli.py']
    assert 'cmastery/cli.py' not in campaign.pins()
    # Two explicit historical metadata inputs: the old card and executed export
    # CLI. Current export evidence separately binds and executes today's CLI.
    # Every C source, command, runtime, log and output remains the actual input.
    prior.read=lambda p:read(original_card) if p==ROOT/CARD else saved_read(p)
    prior.sha=lambda p:sha(original_card) if p==ROOT/CARD else sha(original_cli) if p==ROOT/'cmastery/cli.py' else saved_sha(p)
    try:return prior.validate(old,r,campaign.pins(),campaign.external())
    finally:prior.read,prior.sha=saved_read,saved_sha
def validate(d):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    current=read(ROOT/CARD);base=history();need(current['criteria']==base['criteria'] and current['baseline_binding']==base['baseline_binding'],'unchanged original task/baseline objects');need(attach(copy.deepcopy(current))==current,'idempotent current overlay');need(d.get('updates')==rows() and len(d.get('updates',[]))==6,'six unchanged task identities with precise clarification');need(d.get('historical_card_binding')==dict(path='research/v5/expansion/legacy_object_clinic_contract/before_contract_v2.json',sha256=sha(HERE/'before_contract_v2.json'),original_path=CARD),'exact one-card historical projection');need(d.get('reason_no_c_rerun')==read(HERE/'source_access.json')['reason_no_c_rerun'],'honest retained execution rationale')
    for n,h in d.get('source_sha256',{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current source '+n)
    need({'tools/check_object_clinic_contract.py','tools/close_object_clinic_contract.py','research/v5/expansion/legacy_object_clinic_contract/author_support.py',CONTRACT,CARD}<=set(d.get('source_sha256',{})),'current guard/producer/contract pins')
    old=read(HERE/'historical_completion_dispositions.json');need(sha(HERE/'before_contract_v2.json')==old['source_sha256'][CARD],'historical card exact receipt identity');need(sha(OLD/'completion_dispositions.json')==sha(HERE/'historical_completion_dispositions.json') and sha(OLD/'exports.json')==sha(HERE/'historical_exports.json'),'historical receipts preserved byte-for-byte')
    need(d.get('historical_cli_binding')==dict(path='research/v5/expansion/legacy_object_clinic_contract/historical_cli.py',sha256=sha(HERE/'historical_cli.py'),original_path='cmastery/cli.py'),'exact historical export CLI projection')
    need(sha(HERE/'historical_cli.py')==old['source_sha256']['cmastery/cli.py']==read(HERE/'historical_exports.json')['source_sha256']['cmastery/cli.py'],'historical CLI actual original bytes')
    r=read(OLD/'validation.json');need(r['source_sha256']==campaign.pins() and r['status']=='pass' and len(r['commands'])==192 and all(q['passed'] for q in r['commands']),'unchanged actually executed C campaign')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected premises '+n)
    text=' '.join((ROOT/CONTRACT).read_text(encoding='utf8').split());need(all(s in text for s in ['initialized to `NULL`','hold its own live counted','overwrites the only slot','externally established quiescence','retained without claiming a new run','ISO C does not']),'explicit actual caller premises')
    test=''.join((ROOT/'labs/object_followthrough/clinic/shared_tests.c').read_text().split());need(all(s in test for s in ['structmc_shared*extra=NULL;','structmc_shared*third=NULL;','structmc_shared*excess=NULL;','context.owner=extra;extra=NULL','pthread_join(thread,&result)','mc_shared_drop(&main_owner)']),'existing tested caller sequence complies')
    e=read(HERE/'exports.json');need(e['status']=='pass' and len(e['commands'])==8 and all(q['passed'] for q in e['commands']),'fresh actual export checks')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e[key].items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    need(CONTRACT in e['source_sha256'] and any(n.endswith('/'+CONTRACT) for n in e['outputs_sha256']),'clarification actually exported')
    gate=read(HERE/'research_gate.json');need(gate['status']=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');errors=historical_errors()+validate(d);controls=[]
    if a.self_test and not errors:
        for k in ['missing-row','changed-original','changed-delay','inflated-external','inflated-learner','inflated-physical','stale-source','unbound-contract','wrong-historical-card','broaden-projection','broaden-cli-projection','new-execution-claim']:
            x=copy.deepcopy(d)
            if k=='missing-row':x['updates'].pop()
            elif k=='changed-original':x['updates'][0]['original_task']='placeholder'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-external':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k=='stale-source':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
            elif k=='unbound-contract':del x['source_sha256'][CONTRACT]
            elif k=='wrong-historical-card':x['historical_card_binding']['sha256']='0'*64
            elif k=='broaden-projection':x['historical_card_binding']['original_path']='assessor/object_followthrough/clinic/shared.c'
            elif k=='broaden-cli-projection':x['historical_cli_binding']['original_path']='assessor/object_followthrough/clinic/shared.c'
            elif k=='new-execution-claim':x['reason_no_c_rerun']='New C tests executed'
            rejected=bool(validate(x));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    print(json.dumps(dict(passed=not errors,updates=6,preserved_campaign_commands=192,new_export_commands=8,negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
