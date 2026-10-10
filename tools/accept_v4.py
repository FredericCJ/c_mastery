"""Accept v4 delivery from actual current checks; never infer learner mastery."""
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha
from v2_common import source_identity


def pinned_receipt(name):
    path=ROOT/'reviews/v4'/name
    record=read_json(path)
    if not record.get('passed',record.get('accepted',False)):
        raise ValueError('Failed or absent execution/review receipt: '+name)
    pins=record.get('inspected_paths',{})
    if not pins:raise ValueError('Execution/review receipt has no source pins: '+name)
    for relative,digest in pins.items():
        file=(ROOT/relative).resolve()
        if not file.is_relative_to(ROOT.resolve()) or not file.is_file() or sha(file)!=digest:
            raise ValueError('Stale receipt '+name+': '+relative)
    for finding in record.get('findings',[]):
        if finding.get('severity') in {'medium','high','critical'} and finding.get('status') not in {'resolved','closed'}:
            raise ValueError('Unresolved finding in '+name)
    return sha(path)


def main():
    identity=source_identity();publication=read_json(ROOT/'reviews/v4/publication.json')
    if publication.get('status')!='passed' or publication.get('input_sha256')!=identity['sha256']:
        raise ValueError('Current source-bound publication validation is required')
    receipts={name:pinned_receipt(name) for name in ['legacy_matrix.json','machine_hosted_acceptance.json',
        'firmware_integration_acceptance.json','workflow.json','review_delivery.json','target_matrix.json','os_comparison.json','release_guards.json']}
    matrix=read_json(ROOT/'reviews/v4/legacy_matrix.json')
    if {x['family'] for x in matrix['families'] if x['passed']}!={'foundations','algorithms','communications','runtime','storage','kernel','os_observatory','execution','kernel_smp'}:
        raise ValueError('Retained-family execution coverage differs')
    counts=read_json(ROOT/'curriculum/v4/summary.json')
    if counts['new_executable_units']!=56 or counts['projects']!=429 or counts['objectives']!=1881:
        raise ValueError('Release coverage/count contract differs')
    report={'schema_version':4,'status':'accepted','accepted':True,'passed':True,
        'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':identity['sha256'],
        'receipts':receipts,'publication_sha256':sha(ROOT/'reviews/v4/publication.json'),'counts':counts,
        'scope':'Authored and executable v4 curriculum delivery, local learner workflow, preserved identities, validated host gates and declared machine/firmware compilation evidence.',
        'physical_observations':'not-run','learner_mastery':'not-assessed','learning_effectiveness':'not-measured',
        'open_evidence':['Required F439/H745 and ten-interface physical captures remain learner obligations.',
            'Unavailable privileged/cgroup experiments remain separate from ordinary host evidence.',
            'Independent learner implementations, explanations, formal arguments, delayed transfer and time calibration are not established by author tests.']}
    dump(ROOT/'reviews/v4/acceptance.json',report)
    print('V4 delivery accepted. Physical and learner evidence remain explicitly separate.')


if __name__=='__main__':main()
