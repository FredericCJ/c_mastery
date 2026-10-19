"""Bind exact current campaign bytes to the established integrated receipt path.

No command is reexecuted and no source pin is rewritten. Failed current inputs
remain failed, and any earlier destination is retained for audit.
"""
import argparse,json
from datetime import datetime,timezone
from common import ROOT,dump,sha
from check_expansion_campaigns import pin_errors,execution_errors,inspect_family
FOLDERS={name:name for name in ['corpus_codec','corpus_ecc','corpus_erase','corpus_escape','corpus_events','corpus_limbs','corpus_money','corpus_munch','corpus_query','corpus_ring','corpus_scene','corpus_shunting','corpus_sketch','corpus_spatial','corpus_storage','corpus_tdigest','corpus_model','corpus_treiber']}
FOLDERS.update(corpus_index='corpus_followthrough',numeric_interval='corpus_interval',parsing_depth='corpus_parsing',text_composition='corpus_text')

def main():
    p=argparse.ArgumentParser();p.add_argument('--family',action='append',choices=sorted(FOLDERS));p.add_argument('--report',required=True);a=p.parse_args()
    rows=[];errors=[];run=ROOT/'reviews/v5/expansion/campaign_routing'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');run.mkdir(parents=True)
    for family in a.family or sorted(FOLDERS):
        try:
            envelope_path=ROOT/'research/v5/expansion'/FOLDERS[family]/'completion_dispositions.json'
            envelope=json.loads(envelope_path.read_text())
            for n,h in envelope['source_sha256'].items():assert sha(ROOT/n)==h,('stale envelope',n)
            source=ROOT/envelope['current_reports'][0];data=json.loads(source.read_text())
            assert data.get('family')==family and data.get('passed') is True,'wrong or failed actual campaign'
            assert not pin_errors(data)+execution_errors(data),'campaign pins/observations differ'
            meta=json.loads((ROOT/'labs'/family/'family.json').read_text());name=meta['campaign'].get('receipt',family+'_campaign.json')
            assert '/' not in name and name.endswith('_campaign.json')
            destination=ROOT/'reviews/v5/expansion/integrated'/name
            old=None
            if destination.exists():
                old=run/name;old.write_bytes(destination.read_bytes())
            destination.write_bytes(source.read_bytes())
            assert sha(destination)==sha(source)
            result=inspect_family(family)
            rows.append(dict(family=family,source=source.relative_to(ROOT).as_posix(),source_sha256=sha(source),destination=destination.relative_to(ROOT).as_posix(),preserved_destination=old.relative_to(ROOT).as_posix() if old else None,origin_envelope=envelope_path.relative_to(ROOT).as_posix(),origin_envelope_sha256=sha(envelope_path),integrated_check=result))
            if not result['passed']:errors.append(dict(family=family,errors=result['errors']))
        except (AssertionError,OSError,KeyError,ValueError) as e:errors.append(dict(family=family,errors=[str(e)]))
    report=dict(passed=not errors,scope='Byte-identical routing of explicit current author campaigns, followed by existing integrated family checks; no new execution or acceptance inferred.',rows=rows,errors=errors,producer_sha256=sha(ROOT/'tools/route_current_corpus_campaigns.py'))
    dump(ROOT/a.report,report);print(json.dumps(dict(passed=not errors,routed=len(rows),errors=errors)));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
