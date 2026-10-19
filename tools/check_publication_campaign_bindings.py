"""Check the closed publication exception at the campaign source-pin boundary.

The publication producer independently validates all preserved bytes, the full
unchanged mechanism closure, and actual portable exports. These controls verify
that consumers retain the execution identity and refuse unrelated differences.
"""
import argparse
import copy
import json
from unittest.mock import patch
from common import ROOT, dump, sha
from check_expansion_campaigns import source_binding
import rebind_modern_publication as publication


def check():
    assert not publication.validate_publication_proof()
    old=dict(publication.ORIGINALS)
    report={'inspected_paths':old,'scope_input_sha256':'retained-execution-identity'}
    before=copy.deepcopy(report)
    errors,qualified=source_binding(report)
    assert not errors and len(qualified)==19 and report==before
    for row in qualified:
        assert row['execution_document_sha256']==old[row['path']]
        assert row['current_publication_sha256']==sha(ROOT/row['path'])
        assert row['new_c_execution'] is False
        assert row['state']=='original-execution-identity-retained;current-publication-validated-separately'
        assert set(row['proof_sha256'])=={publication.MANIFEST,publication.PROOF,publication.TOOL}
    first=next(iter(old))
    cases={
        'wrong-historical-hash':{'inspected_paths':{first:'0'*64}},
        'unlisted-source-difference':{'inspected_paths':{'tools/check_expansion_campaigns.py':'0'*64}},
        'missing-source':{'inspected_paths':{'labs/publication-no-such-required-file.md':'0'*64}},
        'missing-source-map':{},
        'non-string-hash':{'inspected_paths':{first:None}},
    }
    controls=[]
    for name,value in cases.items():
        errors,bindings=source_binding(value)
        assert errors and not bindings,name
        controls.append(name)
    with patch.object(publication,'validate_publication_proof',return_value=['stale proof']):
        errors,bindings=source_binding({'inspected_paths':{first:old[first]}})
        assert errors and not bindings
        controls.append('failed-current-publication-proof')
    original=publication.read
    def missing(path):
        if path==publication.PROOF:raise FileNotFoundError(path)
        return original(path)
    with patch.object(publication,'read',side_effect=missing):
        errors,bindings=source_binding({'inspected_paths':{first:old[first]}})
        assert errors and not bindings
        controls.append('missing-current-publication-proof')
    assert not publication.approved_pin(first,old[first],'0'*64)
    controls.append('wrong-current-publication-hash')
    return dict(passed=True,accepted=False,negative_controls=controls,
        qualified_documents=len(qualified),historical_publication_differences=qualified,
        execution_receipt_rewritten=False,whole_source_execution_current=False,
        new_c_execution=False,
        source_sha256={p:sha(ROOT/p) for p in ['tools/check_publication_campaign_bindings.py',
            'tools/check_expansion_campaigns.py',publication.TOOL,publication.MANIFEST,publication.PROOF]})


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    result=check();dump(ROOT/a.report,result)
    print(json.dumps({'passed':True,'negative_controls':len(result['negative_controls']),'qualified_documents':19}))
