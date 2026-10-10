"""Accept a current, explicitly scoped v2 deliverable from actual receipts."""
from datetime import datetime, timezone
from common import ROOT, dump, read_json, sha
from v2_common import source_identity, receipt_current

def main():
    identity=source_identity(); receipts={}
    for name in ['publication','lab_matrix','cli_checks']:
        path=ROOT/'reviews/v2'/f'{name}.json'; record=read_json(path)
        if not receipt_current(record,identity): raise ValueError('Missing, failed or stale current check: '+name)
        receipts[name]=sha(path)
    required={'runtime','storage','kernel','foundations','algorithms','communications'}
    matrix=read_json(ROOT/'reviews/v2/lab_matrix.json')
    if {x['family'] for x in matrix['families'] if x['passed']} != required:
        raise ValueError('Actual lab matrix must cover every delivered family')
    reviews={}
    for name in ['review_pedagogy','review_sources','review_labs','review_kernel_communications']:
        path=ROOT/'reviews/v2'/f'{name}.json'; record=read_json(path)
        if record.get('accepted') is not True: raise ValueError('Independent scope not accepted: '+name)
        pins=record.get('inspected_paths',{})
        if not pins: raise ValueError('Review has no inspected input hashes: '+name)
        for relative,digest in pins.items():
            file=(ROOT/relative).resolve()
            if not file.is_relative_to(ROOT) or not file.is_file() or sha(file)!=digest:
                raise ValueError('Review stale: '+name+' '+relative)
        for finding in record.get('findings',[]):
            if finding.get('severity') in {'high','critical','medium'} and finding.get('status') not in {'resolved','closed'}:
                raise ValueError('Open blocking finding: '+name)
        reviews[name]={'sha256':sha(path),'scope':record.get('scope'),'files':len(pins)}
    publication=read_json(ROOT/'reviews/publication_inventory.json')
    report={'schema_version':2,'accepted':True,'status':'accepted','recorded_at':datetime.now(timezone.utc).isoformat(),
        'input_sha256':identity['sha256'],'canonical_source_snapshot':identity['files'],
        'scope':'Published curriculum, migration, original laboratory scaffolds, scoped assessor implementations and actual checks; learner mastery remains to be earned.',
        'counts':publication,'receipts':receipts,'reviews':reviews,
        'open_learner_evidence':['All physical interface measurements','Target-specific profiling and timing claims beyond recorded emulator checks','Assigned formal proofs and independent delayed transfer','Learner-time calibration'],
        'limits':['Family manifests state executable anchors and unautomated project obligations. A family pass is not automatic acceptance of every linked objective.','Finite failure campaigns validate their declared models; they do not certify production systems or protocol/electrical conformance.','Browser rendering review was unavailable in this environment; publication is checked through generated HTML structure, destinations and canonical-content fidelity.']}
    dump(ROOT/'reviews/v2/acceptance.json',report)
    print('Current v2 curriculum and scoped laboratories accepted; physical and learner claims remain open.')

if __name__=='__main__': main()
