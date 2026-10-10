"""Accept delivered v3 software/publication scope from current, actual receipts."""
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha
from v2_common import source_identity, receipt_current

def main():
    identity=source_identity(); receipts={}
    for name in ['publication','lab_matrix','cli_checks','target_matrix','os_comparison']:
        path=ROOT/'reviews/v3'/f'{name}.json'; record=read_json(path)
        if not receipt_current(record,identity): raise ValueError('Missing, failed or stale check: '+name)
        receipts[name]=sha(path)
    required={p.parent.name for p in (ROOT/'labs').glob('*/family.json')}
    matrix=read_json(ROOT/'reviews/v3/lab_matrix.json')
    if {x['family'] for x in matrix['families'] if x['passed']}!=required:
        raise ValueError('Actual family matrix must cover all delivered laboratories')
    reviews={}
    for name in ['review_curriculum','review_execution','review_kernel','review_observatory']:
        path=ROOT/'reviews/v3'/f'{name}.json'; record=read_json(path)
        if record.get('accepted') is not True: raise ValueError('Review not accepted: '+name)
        pins=record.get('inspected_paths',{})
        if not pins: raise ValueError('Review has no inspected input hashes: '+name)
        for relative,digest in pins.items():
            file=(ROOT/relative).resolve()
            if not file.is_relative_to(ROOT) or not file.is_file() or sha(file)!=digest:
                raise ValueError('Review stale: '+name+' '+relative)
        for finding in record.get('findings',[]):
            if finding.get('severity') in {'medium','high','critical'} and finding.get('status') not in {'resolved','closed'}:
                raise ValueError('Open blocking finding: '+name)
        reviews[name]={'sha256':sha(path),'scope':record.get('scope'),'files':len(pins)}
    report={'schema_version':3,'accepted':True,'status':'accepted',
        'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':identity['sha256'],
        'canonical_source_snapshot':identity['files'],'counts':read_json(ROOT/'reviews/publication_inventory.json'),
        'scope':'Published project curriculum, preserved prior identities, unsolved scaffolds, scoped assessor implementations, host/emulator execution and strict image builds.',
        'receipts':receipts,'reviews':reviews,
        'physical_observations':'not-run','learner_mastery':'not-assessed',
        'open_evidence':['STM32F439 and NUCLEO-H745ZI-Q physical boot/timing/cache/DMA/fault captures',
            'Physical interface measurements inherited from earlier editions',
            'Delegated cgroup and privileged Linux scheduling experiments when unavailable in the test environment',
            'Independent learner implementation, explanation, transfer, formal claims and time calibration'],
        'limits':['Named stage tests validate their stated finite scope, not every linked project objective.',
            'Emulator overlap and image linkage audits establish neither physical performance nor hard real-time guarantees.',
            'Review scopes and source inspection limits are recorded explicitly; no production-kernel certification is claimed.']}
    dump(ROOT/'reviews/v3/acceptance.json',report)
    print('Current v3 software and publication scope accepted; physical and learner claims remain open.')

if __name__=='__main__': main()
