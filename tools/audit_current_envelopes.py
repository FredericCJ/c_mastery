"""Read-only byte-pin audit of current support/readiness envelopes, with exact defects."""
import argparse
import hashlib
import json
import os
import re
from collections import Counter
from pathlib import Path
from common import ROOT, dump

PIN_KEYS = {'source_sha256','input_sha256','inspected_paths','scope_input_paths',
            'dependency_sha256','research_input_sha256','provider_sha256',
            'current_reports','pins','artifact_sha256','report_sha256','closure_source_sha256'}
EXCLUDED = {'foundation_followthrough','b01_followthrough','network_http_variants','alternate_close_depth',
            'certificate_followthrough','certificate_transfer','corpus_index','corpus_interval','corpus_parser',
            'corpus_spatial','corpus_ecc','cacheable_boot_depth'}


def historical(name):
    value = name.lower()
    return (value in {'history','runs','__pycache__','archive','archives'} or value.startswith(
        ('history_','history-','historical_','attempt_','attempt-','snapshot_','before_','metadata_before_'))
        or '_before_' in value or '.before-' in value)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def pins(value, prefix=''):
    if isinstance(value,dict):
        for key,item in value.items():
            here = prefix+'/'+key
            if key in PIN_KEYS and isinstance(item,dict):
                for path,expected in item.items():
                    if isinstance(expected,str) and re.fullmatch('[0-9a-f]{64}',expected):
                        yield here,path,expected
            elif isinstance(item,(dict,list)):
                yield from pins(item,here)
    elif isinstance(value,list):
        for index,item in enumerate(value):
            yield from pins(item,prefix+'/'+str(index))


def audit():
    rows=[];counts=Counter();cache={}
    for base in ['research/v5/modern','research/v5/expansion']:
        for parent,dirs,names in os.walk(ROOT/base):
            dirs[:]=[d for d in dirs if not historical(d)]
            directory=Path(parent)
            if any(part in EXCLUDED for part in directory.relative_to(ROOT).parts):
                dirs[:]=[];continue
            for name in sorted(names):
                if not name.endswith('.json') or historical(name):continue
                if not any(token in name for token in ('support','readiness','completion_dispositions')):continue
                file=directory/name
                try:data=json.loads(file.read_text(encoding='utf-8'))
                except (ValueError,UnicodeError) as exc:
                    rows.append(dict(envelope=file.relative_to(ROOT).as_posix(),error=str(exc)));continue
                source_pins=list(pins(data))
                if not source_pins:continue
                stale=[];producers=set();reports=[]
                for location,path,expected in source_pins:
                    # A Windows absolute path denotes the same host only; WSL tool hashes
                    # are deliberately excluded from this local repository audit.
                    resolved=Path(path) if Path(path).is_absolute() else ROOT/path
                    if path.startswith('/home/') or path.startswith('/usr/'):
                        continue
                    if path.startswith('tools/') and path.endswith('.py') and any(
                        token in path for token in ('close_','build_','export','check_')):producers.add(path)
                    if path not in cache:cache[path]=digest(resolved) if resolved.is_file() else None
                    actual=cache[path]
                    if actual!=expected:
                        category=('cli/export' if path.startswith('cmastery/') else
                                  'catalog' if 'exercise_catalog' in path or path.endswith('/exercises.json') else
                                  'implementation' if Path(path).suffix in {'.c','.h','.S','.ld'} else
                                  'report' if path.startswith('reviews/') else 'metadata/tool')
                        stale.append(dict(field=location,path=path,expected=expected,actual=actual,category=category))
                        counts[path]+=1
                row=dict(envelope=file.relative_to(ROOT).as_posix(),sha256=digest(file),
                    pins=len(source_pins),stale=stale,producer_candidates=sorted(producers),
                    current_reports=data.get('current_reports',[]))
                rows.append(row)
    return dict(schema_version=1,scope='Read-only direct recursive repository pin audit of current support/readiness envelopes; does not replace mechanism-specific guards or validate external tool paths.',
        exclusions=sorted(EXCLUDED),envelopes=rows,
        summary=dict(envelopes=len(rows),stale=sum(bool(r.get('stale')) for r in rows),
                     stale_pin_occurrences=sum(counts.values())),
        stale_paths=[dict(path=p,envelopes=n) for p,n in counts.most_common()])


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--output',required=True);args=parser.parse_args()
    result=audit();dump(ROOT/args.output,result);print(json.dumps(result['summary']))
