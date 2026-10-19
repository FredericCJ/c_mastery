"""Exact-range document successors; never rewrite original execution inputs."""
import argparse
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from research.v5.expansion.b06_document_utf8.author_support import attach, BODY, GUIDE, MANIFEST

BASE = 'research/v5/expansion/b06_document_utf8'
REPORT = BASE + '/validation.json'
PACKET = BASE + '/completion_dispositions.json'
MAP = BASE + '/encoding_map.json'
GAPS = BASE + '/gap_ledger.json'
SNAPSHOT = BASE + '/predecessor_rows.json'
PROJECTS = BASE + '/predecessor_projects.json'
GENERATOR = 'research/v5/expansion/b06/author_domains.py'
ORIGINAL = 'research/v5/expansion/b06/completion_dispositions.json'
ENVELOPE = 'research/v5/expansion/b06/completion_envelope.json'
OLD_F439 = 'research/v5/expansion/b06_document_encoding/completion_dispositions.json'
F439 = 'labs/firmware_lifecycle/targets/F439_UPDATE_UTF8.md'
F439_HASH = '41de9125bd5b2641d14ddf62d1ee9029eb2ce0a7c500da48e960448be89ecb1c'
# Offsets below address original bytes, not decoded character positions.
SPECS = [
 ('F439-ANALYSIS','assessor/firmware_lifecycle/targets/F439_UPDATE_ANALYSIS.md','b4c08570305a66af3be30999135b0b8c05ef453ede79228bcd91f7229ec842b0',[1142],'96','windows-1252-dash'),
 ('INSTALL-ANALYSIS','assessor/firmware_lifecycle/targets/INACTIVE_INSTALL_ANALYSIS.md','2f735e32189a75ee70ffa3473e4f6e68f05352a79dc7a0eadf6981a61ea065df',[1708,1722],'97','windows-1252-dash'),
 ('H745-ERRATA','labs/firmware_lifecycle/targets/H745_ERRATA.md','c2ea879b858ce7d2316f408ae109f85ff4e064978bd0b8aba010a97fa9f6aa88',[223,242],'96','windows-1252-dash'),
 ('INSTALL','labs/firmware_lifecycle/targets/INACTIVE_INSTALL.md','02b7a7ee1dff00a3ea4b78b197e17a2ccc86632aa4de9e3c8e9ec723f4d1e24e',[4584],'96','windows-1252-dash'),
 ('STARTUP','labs/firmware_lifecycle/targets/SELECTED_STARTUP.md','6362ef82db59ce2f0c6472bd9ca45fc2119526cd95613e888d852306206f95bb',[4434],'96','windows-1252-dash'),
 ('HEALTH','labs/firmware_lifecycle/targets/TRIAL_HEALTH.md','0186bd09e5df91de1315409259d827a0e21d743590859f8d3afbc08a0eacbfa5',[7257],'96','windows-1252-dash'),
 ('SOURCE-NOTE','research/v5/expansion/b06/followup_sources.md','0da90e7cd8e4d08667f4a230ced0357e69e8499ed4f780d4082aca28e5e217f8',[14581,14590],'96','mixed-encoding-exact-spans'),
 ('RECOVERY','labs/firmware_lifecycle/targets/RECOVERY_TRANSPORT.md','f960cfe6dbb1a9021710c10c5989a77913e812b13a80832873f45926a79d87ca',[4215],'efbfbd','contextual-deliverable-range-repair'),
 ('COMPILER','labs/subset_compiler/extension/README.md','dba0696d952f34fd3cefe019818c9f4ed4d5b573587c74bafbe8ea5a1e71bd9a',[42,438,2155,5599,5618,5637,5891,5910,6203,6222,6540,6559,6578,6892,7766],'c3a2e282ace2809c','exact-mojibake-en-dash-repair'),
 ('COMPILER-ANALYSIS','assessor/subset_compiler/extension/ANALYSIS.md','360c8926256bb37dbc3c900a7e09113e98bf229561eff35412264bee2971b029',[7],'c3a2e282ace2809c','exact-mojibake-en-dash-repair'),
]
LIMITS = [
 'Only recorded prose bytes change. No C, target, waveform, source inspection, external review or learner execution is added.',
 'Original files and original execution/source-gate digests remain authoritative for the historical runs.',
 'The recovery range repair is justified by the enumerated D1 through D5 obligations, not recovered unknown bytes.',
 'Current project assets include public renditions only; private assessor renditions remain outside learner export.',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read(name):
    return json.loads((ROOT / name).read_text(encoding='utf8'))


def put(name, value):
    p = ROOT / name
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n', encoding='utf8', newline='\n')


def current_path(old):
    if old.startswith('research/'):
        return BASE + '/followup_sources_utf8.md'
    p = Path(old)
    return (p.parent / (p.stem + '_UTF8.md')).as_posix()


def transform(raw, row):
    if hashlib.sha256(raw).hexdigest() != row['original_sha256']:
        raise ValueError('Original digest differs: ' + row['original'])
    result = bytearray()
    cursor = 0
    for patch in row['patches']:
        start, end = patch['byte_range']
        expected = bytes.fromhex(patch['original_hex'])
        replacement = bytes.fromhex(patch['replacement_utf8_hex'])
        if start < cursor or end != start + len(expected) or raw[start:end] != expected:
            raise ValueError('Exact original span differs')
        # Strictly decode unchanged chunks: this both preserves mixed UTF-8 and
        # rejects an omitted invalid original byte outside the declared spans.
        raw[cursor:start].decode('utf8')
        if replacement.decode('utf8') != chr(int(patch['codepoint'][2:], 16)):
            raise ValueError('Replacement/codepoint differs')
        result.extend(raw[cursor:start]); result.extend(replacement); cursor = end
    raw[cursor:].decode('utf8')
    result.extend(raw[cursor:])
    bytes(result).decode('utf8')
    return bytes(result)


def mapping():
    rows = []
    for key, old, digest, offsets, oldhex, operation in SPECS:
        cp = 0x2014 if oldhex == '97' else 0x2013
        raw = (ROOT / old).read_bytes()
        row = dict(id='G-B06-DOC-UTF8-' + key, original=old, current=current_path(old),
                   original_sha256=digest, operation=operation,
                   patches=[dict(byte_range=[p, p + len(bytes.fromhex(oldhex))], original_hex=oldhex,
                                 codepoint=f'U+{cp:04X}', replacement_utf8_hex=chr(cp).encode('utf8').hex(),
                                 original_line=raw[:p].count(b'\n')+1) for p in offsets])
        if key == 'RECOVERY':
            row['interpretation_basis'] = 'The preceding paragraphs explicitly enumerate D1, D2, D3, D4 and D5; this range does not include separately deferred D6.'
        if key == 'SOURCE-NOTE':
            row['already_valid_utf8_byte_ranges'] = [[6357,6360],[6444,6447]]
        rendered = transform(raw, row)
        row['current_sha256'] = hashlib.sha256(rendered).hexdigest()
        row['original_bytes'] = len(raw); row['current_bytes'] = len(rendered)
        rows.append(row)
    return dict(schema_version=1, scope='Exact original half-open byte ranges; no blanket transcoding', documents=rows)


def public_bindings(m):
    public = [r['current'] for r in m['documents'] if r['original'].startswith('labs/firmware_lifecycle/')]
    public.append(F439)
    compiler = [r['current'] for r in m['documents'] if r['original']=='labs/subset_compiler/extension/README.md']
    projects = {f'PR-T59-{n:02}': public for n in range(1,9)}
    projects.update({f'PR-T60-{n:02}': compiler for n in range(1,9)})
    # Both legacy projects share the guide, so all of its local public links must
    # resolve in their exports as well. No assessor/source-note file is supplied.
    projects.update({p: public + compiler for p in ['PR-T28-06','PR-T29-04']})
    # The common guide links both sections; supply those links for all projects.
    projects = {p:list(dict.fromkeys(v + public + compiler)) for p,v in projects.items()}
    return dict(schema_version=1, guide=GUIDE, projects=projects,
                assessor_documents=[r['current'] for r in m['documents'] if r['original'].startswith('assessor/')],
                author_source_note=BASE+'/followup_sources_utf8.md')


def parse(text):
    match = re.match(r'\A```json\s*\n(.*?)\n```\n?', text, re.S)
    if not match: raise ValueError('Missing project metadata')
    return json.loads(match[1]), text[match.end():]


def project_text(project, text):
    meta, body = parse(text)
    meta, body = attach(project, meta, body)
    return '```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```\n'+body


def replace_values(value, names):
    if isinstance(value,str): return names.get(value,value)
    if isinstance(value,list): return [replace_values(v,names) for v in value]
    if isinstance(value,dict): return {k:replace_values(v,names) for k,v in value.items()}
    return value


def gap_rows(m):
    project_map = {
        'F439-ANALYSIS':['PR-T59-02','PR-T59-03','PR-T59-05','PR-T59-07'],
        'INSTALL-ANALYSIS':['PR-T59-01','PR-T59-03','PR-T59-04','PR-T59-05'],
        'H745-ERRATA':['PR-T59-04','PR-T59-05','PR-T59-07','PR-T28-06','PR-T29-04'],
        'INSTALL':['PR-T59-01','PR-T59-03','PR-T59-04','PR-T59-05'],
        'STARTUP':['PR-T59-04'], 'HEALTH':['PR-T59-04','PR-T59-05','PR-T59-07'],
        'SOURCE-NOTE':[], 'RECOVERY':['PR-T59-01','PR-T59-03','PR-T59-04','PR-T59-05'],
        'COMPILER':[f'PR-T60-{n:02}' for n in range(1,9)],
        'COMPILER-ANALYSIS':[f'PR-T60-{n:02}' for n in range(1,9)],
    }
    children=[]
    for r in m['documents']:
        projects=project_map[r['id'].removeprefix('G-B06-DOC-UTF8-')]
        topics=[]
        for project in projects:
            meta,_=parse((ROOT/'curriculum/projects'/f'{project}.md').read_text(encoding='utf8'))
            topics+=meta['topics']
        children.append(dict(id=r['id'],parent_ids=['G-B06-DOCUMENT-UTF8'],
            task='Provide a strict UTF-8 current rendition of '+r['original']+' with exact byte-range mapping, original digest preservation and correct publication separation.',
            original_document=r['original'],original_sha256=r['original_sha256'],operation=r['operation'],
            projects=projects,domains=sorted({p[3:6] for p in projects}) if projects else [f'T{n}' for n in range(57,65)],
            existing_objectives=[t+'.'+leaf for t in sorted(set(topics)) for leaf in 'abc'],
            context_obligations=['G-B06-REFERENCES','G-B06-PHYSICAL'] if not projects else ['G-B06-COMPILER'] if all('T60' in p for p in projects) else ['G-B06-T59-TARGET'],
            origin=dict(path=r['original'],sha256=r['original_sha256'],byte_ranges=[p['byte_range'] for p in r['patches']],diagnosis='Original authored prose encoding/corruption; not an intentional malformed-input fixture'),
            prerequisite='Author must compare the original contract and exact affected range before repairing prose; no learner prerequisite or accepted assessment changes.',
            validation_method='tools/rebind_b06_documents.py --self-test; exact original digest, strict UTF-8 unchanged chunks and output, active binding/export separation and negative controls',
            completion_criteria=['Only registered byte ranges differ; valid UTF-8 and line endings remain byte-identical.',
                                 'Required public consumers export the current rendition; private analysis/source notes are excluded.',
                                 'Original source/experiment receipts and current dimensional limitations remain unchanged.']))
    parent = dict(id='G-B06-DOCUMENT-UTF8', task='Repair all ten diagnosed B06 prose encodings/corruptions without changing historical source identities or scientific claims; bind public and assessor renditions to current consumers.',
                  children=[r['id'] for r in children],projects=list(public_bindings(m)['projects']),domains=['T28','T29',*[f'T{n}' for n in range(57,65)]],
                  existing_obligations=[r['id'] for r in read(SNAPSHOT)['updates']],origin=dict(path=MAP,scope='Ten exact original document identities and ranges'),
                  validation_method='tools/rebind_b06_documents.py --self-test',completion_criteria=['Every listed child passes the exact-range guard and current bindings; no missing child is inferred from the parent heading.'],
                  prerequisite='Retain the current nineteen B06 evidence rows and original source/receipt digests before publication repair.')
    return dict(schema_version=1, scope='Concrete documentation defects diagnosed during final publication preflight; original bytes retained', gaps=[parent]+children)


def expected_rows(m):
    names = {r['original']:r['current'] for r in m['documents']}
    rows = replace_values(read(SNAPSHOT)['updates'], names)
    for row in rows:
        row['document_rendition_successor'] = dict(mapping=MAP,report=REPORT,scope='Document paths only; all predecessor dimension states, current source/target limits and execution records preserved.')
    for gap in gap_rows(m)['gaps']:
        doc = next((r for r in m['documents'] if r['id']==gap['id']),None)
        evidence = [doc['current']] if doc else [r['current'] for r in m['documents']]
        dims = {k:dict(state=s,evidence=e) for k,s,e in [
            ('teaching','complete',[GUIDE,MAP]),('implementation','complete',evidence+[MAP]),
            ('author_validation','passed',[REPORT]),('external_review','not-run',[]),
            ('independent_learner','not-assessed',[]),('physical_observation','not-applicable-document-repair',[]) ]}
        rows.append(dict(id=gap['id'],parent_ids=gap.get('parent_ids',[]),projects=gap.get('projects',[]),domains=gap['domains'],existing_objectives=gap.get('existing_objectives',[]),dimensions=dims,
                         artifacts=evidence+[MAP,GUIDE],closure_rationale='Exact original ranges corrected, strict output decoding and active public/private binding checked; original records unchanged.',
                         remaining_author_work=[],evidence_limits=LIMITS))
    return rows


def inputs(m):
    files = [BASE+'/README.md',BASE+'/author_support.py',MAP,MANIFEST,GAPS,SNAPSHOT,PROJECTS,
             'tools/rebind_b06_documents.py',GENERATOR,GUIDE,'cmastery/project_assets.py',ORIGINAL,ENVELOPE,OLD_F439,F439]
    files += [p for r in m['documents'] for p in [r['original'],r['current']]]
    files += ['curriculum/projects/'+p+'.md' for p in public_bindings(m)['projects']]
    files += list(read(SNAPSHOT)['preserved_source_sha256'])
    return {n:sha(ROOT/n) for n in sorted(set(files))}


def current_binding_errors(m):
    errors=[]
    errors+=binding_errors(read(MANIFEST),m)
    originals=read(PROJECTS)['projects']
    for p,old in originals.items():
        current=(ROOT/'curriculum/projects'/f'{p}.md').read_text(encoding='utf8')
        if current!=project_text(p,old):errors.append('current project attachment differs: '+p)
        if project_text(p,current)!=current:errors.append('non-idempotent attachment: '+p)
    for n,h in read(SNAPSHOT)['preserved_source_sha256'].items():
        if sha(ROOT/n)!=h:errors.append('historical source/receipt changed: '+n)
    if sha(ROOT/F439)!=F439_HASH:errors.append('existing public F439 rendition changed')
    return errors


def binding_errors(binding,m):
    errors=[]
    if binding!=public_bindings(m):errors.append('public/private binding manifest differs')
    for assets in binding.get('projects',{}).values():
        if any(not p.startswith(('curriculum/','labs/','fixtures/','tools/')) or 'assessor' in Path(p).parts for p in assets):
            errors.append('private/non-public project input')
    return errors


def expected_exports(m):
    return [dict(project=p,passed=True,operation='actual project_assets.prepare/export',
                 renditions=len(set(assets+[GUIDE])),assessor_files_exported=0)
            for p,assets in public_bindings(m)['projects'].items()]


def validate(packet,report,m):
    errors=[]
    try:
        if m!=mapping():errors.append('exact document/range manifest differs')
        for row in m['documents']:
            if (ROOT/row['current']).read_bytes()!=transform((ROOT/row['original']).read_bytes(),row):errors.append('changed rendition: '+row['current'])
        errors+=current_binding_errors(m)
        if packet.get('updates')!=expected_rows(m):errors.append('exact predecessor states and document defects differ')
        if packet.get('source_sha256')!=inputs(m)|{REPORT:sha(ROOT/REPORT)}:errors.append('current complete packet pins differ')
        if report.get('input_sha256')!=inputs(m):errors.append('current validation pins differ')
        if report.get('passed') is not True or report.get('new_C_execution') is not False:errors.append('invented execution/failed validation')
        if report.get('operation')!='exact-recorded-byte-ranges-only':errors.append('wrong rendition operation')
        if packet.get('current_reports')!=[REPORT]:errors.append('metadata/current report identity')
        if read(GAPS)!=gap_rows(m):errors.append('missing/or altered original document obligations')
        if report.get('exports')!=expected_exports(m):errors.append('exact eighteen actual public export records differ')
        if not report.get('generator_hook_idempotent'):errors.append('generator hook not checked')
    except (OSError,KeyError,ValueError,UnicodeError) as ex:
        errors.append(str(ex))
    return errors


def export_checks(m):
    from cmastery.project_assets import prepare,export
    rows=[]
    with tempfile.TemporaryDirectory(prefix='cmastery utf8 public inputs ') as tmp:
        for p in public_bindings(m)['projects']:
            dest=Path(tmp)/p;dest.mkdir()
            records=prepare(ROOT,p);export(ROOT,dest,records)
            expected=set(public_bindings(m)['projects'][p]+[GUIDE])
            for n in expected:
                assert (dest/n).read_bytes()==(ROOT/n).read_bytes()
                (dest/n).read_bytes().decode('utf8')
            assert not (dest/'assessor').exists()
            rows.append(dict(project=p,passed=True,operation='actual project_assets.prepare/export',renditions=len(expected),assessor_files_exported=0))
    return rows


def snapshot(m):
    if not (ROOT/SNAPSHOT).exists():
        originals=read(ORIGINAL)['updates'];oldpaths={r['original'] for r in m['documents']}
        ids={r['id'] for r in originals if any(p in json.dumps(r) for p in oldpaths)}
        active=read('curriculum/v5/expansion/implementation_updates.json')['updates']
        rows=[r for r in active if r['id'] in ids]
        assert len(rows)==len(ids)==19
        # These digests cover the entire original B06 execution envelope's
        # declared sources as well as its public F439 rendition predecessor.
        keep=read(ENVELOPE)['source_sha256'].copy()
        keep.update({ENVELOPE:sha(ROOT/ENVELOPE),OLD_F439:sha(ROOT/OLD_F439),F439:F439_HASH})
        for n,h in keep.items():assert sha(ROOT/n)==h,('historical pin changed',n)
        put(SNAPSHOT,dict(schema_version=1,source='curriculum/v5/expansion/implementation_updates.json',
                          original_whole_file_sha256=sha(ROOT/'curriculum/v5/expansion/implementation_updates.json'),
                          scope='Exact selected current rows captured before document-only successor; whole file is provenance, not a current dependency.',
                          updates=rows,preserved_source_sha256=keep))
    if not (ROOT/PROJECTS).exists():
        put(PROJECTS,dict(projects={p:(ROOT/'curriculum/projects'/f'{p}.md').read_text(encoding='utf8') for p in public_bindings(m)['projects']}))


def produce():
    m=mapping();snapshot(m);put(MAP,m);put(MANIFEST,public_bindings(m));put(GAPS,gap_rows(m))
    for row in m['documents']:
        (ROOT/row['current']).write_bytes(transform((ROOT/row['original']).read_bytes(),row))
    for p,text in read(PROJECTS)['projects'].items():
        path=ROOT/'curriculum/projects'/f'{p}.md';current=path.read_text(encoding='utf8');new=project_text(p,text)
        assert current in (text,new),('uncoordinated project change',p)
        path.write_text(new,encoding='utf8',newline='\n')
    spec=importlib.util.spec_from_file_location('b06_current_generator',ROOT/GENERATOR)
    generator=importlib.util.module_from_spec(spec);spec.loader.exec_module(generator)
    for p in public_bindings(m)['projects']:
        meta,body=parse((ROOT/'curriculum/projects'/f'{p}.md').read_text(encoding='utf8'))
        if p.startswith(('PR-T59-','PR-T60-')):
            assert generator.attach_project_support(p,copy.deepcopy(meta),body)==(meta,body)
    exports=export_checks(m)
    report=dict(schema_version=1,passed=True,operation='exact-recorded-byte-ranges-only',new_C_execution=False,
                generator_hook_idempotent=True,exports=exports,input_sha256=inputs(m),
                checks=[dict(name=n,passed=True) for n in ['all ten exact original hashes and byte ranges','unchanged mixed-UTF8 spans and line endings','all ten strict UTF8 outputs','current eighteen public project bindings','private assessor/source-note exclusion','maintained B06 hook idempotence','original370-input B06 envelope preserved','existing F439 rendition unchanged','nineteen current dimension states preserved']],limits=LIMITS)
    put(REPORT,report)
    packet=dict(schema_version=1,scope='Ten additive prose renditions, eighteen public bindings and nineteen unchanged current B06 evidence states.',
                source_sha256=inputs(m)|{REPORT:sha(ROOT/REPORT)},current_reports=[REPORT],updates=expected_rows(m),
                original_gap_register=GAPS,remaining_author_work=[],evidence_limits=LIMITS)
    put(PACKET,packet);assert not validate(packet,report,m)


def self_test(packet,report,m):
    controls=[]
    for name,edit in [
        ('missing-defect',lambda p,r:p['updates'].pop()),
        ('changed-predecessor-state',lambda p,r:p['updates'][0]['dimensions']['external_review'].update(state='passed')),
        ('stale-source',lambda p,r:p['source_sha256'].update({MAP:'0'*64})),
        ('stale-rendition',lambda p,r:p['source_sha256'].update({m['documents'][0]['current']:'0'*64})),
        ('invented-C-run',lambda p,r:r.update(new_C_execution=True)),
        ('missing-public-export',lambda p,r:r['exports'].pop()),
        ('duplicate-project-export',lambda p,r:r['exports'].__setitem__(0,copy.deepcopy(r['exports'][1]))),
        ('wrong-rendition-count',lambda p,r:r['exports'][0].update(renditions=0)),
        ('leaked-assessor-export',lambda p,r:r['exports'][0].update(assessor_files_exported=1)),
        ('unexecuted-export-operation',lambda p,r:r['exports'][0].update(operation='proposed copy')),
        ('missing-generator-check',lambda p,r:r.update(generator_hook_idempotent=False)),
        ('wrong-operation',lambda p,r:r.update(operation='decode-everything-as-cp1252')),
    ]:
        p=copy.deepcopy(packet);r=copy.deepcopy(report);edit(p,r)
        controls.append(dict(control=name,rejected=bool(validate(p,r,m))))
    for name,edit in [
        ('missing-active-project',lambda b:b['projects'].pop('PR-T59-01')),
        ('historical-only-active-binding',lambda b:b['projects']['PR-T28-06'].__setitem__(0,'labs/firmware_lifecycle/targets/H745_ERRATA.md')),
        ('private-assessor-leak',lambda b:b['projects']['PR-T59-01'].append(m['documents'][0]['current'])),
        ('author-source-note-leak',lambda b:b['projects']['PR-T59-01'].append(BASE+'/followup_sources_utf8.md')),
    ]:
        b=copy.deepcopy(read(MANIFEST));edit(b)
        controls.append(dict(control=name,rejected=bool(binding_errors(b,m))))
    # Pure transformer controls run in memory against synthetic and exact bytes;
    # never modify a pinned original to simulate corruption.
    for name,kind in [('wrong-original-hash','hash'),('wrong-byte-offset','offset'),('wrong-byte-value','value'),('wrong-codepoint','codepoint'),('omitted-invalid-byte','omit')]:
        row=copy.deepcopy(m['documents'][1]);raw=(ROOT/row['original']).read_bytes()
        if kind=='hash':row['original_sha256']='0'*64
        elif kind=='offset':row['patches'][0]['byte_range'][0]+=1
        elif kind=='value':row['patches'][0]['original_hex']='96'
        elif kind=='codepoint':row['patches'][0]['codepoint']='U+2013'
        else:row['patches'].pop()
        try:transform(raw,row);rejected=False
        except (ValueError,UnicodeError):rejected=True
        controls.append(dict(control=name,rejected=rejected))
    mixed=next(r for r in m['documents'] if r['operation']=='mixed-encoding-exact-spans')
    raw=(ROOT/mixed['original']).read_bytes();good=transform(raw,mixed)
    assert good!=raw.decode('cp1252').encode('utf8')
    for start,end in mixed['already_valid_utf8_byte_ranges']:
        assert good[start:end]==raw[start:end]==bytes.fromhex('e28093')
    controls.append(dict(control='whole-file-cp1252-misdecode',rejected=good!=raw.decode('cp1252').encode('utf8')))
    raw=b'ASCII\r\n'+bytes.fromhex('e28093')+b' utf8 '+bytes.fromhex('96')+b'\r\n'
    pos=raw.index(bytes.fromhex('96'))
    row=dict(original='synthetic',original_sha256=hashlib.sha256(raw).hexdigest(),patches=[dict(byte_range=[pos,pos+1],original_hex='96',replacement_utf8_hex='e28093',codepoint='U+2013')])
    assert transform(raw,row)==raw[:pos]+bytes.fromhex('e28093')+raw[pos+1:]
    return controls,[dict(control='mixed-valid-UTF8-and-CRLF-preserved',passed=True),dict(control='real-eighteen-public-export-boundaries',passed=len(report['exports'])==18)]


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--write',action='store_true');ap.add_argument('--self-test',action='store_true');ap.add_argument('--report');args=ap.parse_args()
    if args.write:produce()
    m=read(MAP);packet=read(PACKET);report=read(REPORT);errors=validate(packet,report,m);controls=[];positive=[]
    if args.self_test and not errors:
        controls,positive=self_test(packet,report,m)
        errors += ['vacuous '+r['control'] for r in controls if not r['rejected']]
        errors += ['failed '+r['control'] for r in positive if not r['passed']]
    result=dict(passed=not errors,documents=10,predecessor_rows=19,document_obligations=11,updates=len(packet['updates']),negative_controls=controls,positive_controls=positive,errors=errors,source_sha256={PACKET:sha(ROOT/PACKET),'tools/rebind_b06_documents.py':sha(Path(__file__))},limits=LIMITS)
    if args.report:put(args.report,result)
    print(json.dumps(result,indent=2));return int(bool(errors))


if __name__=='__main__':raise SystemExit(main())
