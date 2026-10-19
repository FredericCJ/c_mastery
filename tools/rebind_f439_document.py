"""Encoding-only publication successor; original firmware inputs and receipts stay byte-identical."""
import argparse,copy,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/b06_document_encoding'
OLD='labs/firmware_lifecycle/targets/F439_UPDATE.md';NEW='labs/firmware_lifecycle/targets/F439_UPDATE_UTF8.md';BASE='research/v5/expansion/b06/completion_dispositions.json';ENVELOPE='research/v5/expansion/b06/completion_envelope.json';EXECUTION='research/v5/expansion/b06/f439_update_validation.json';REPORT='research/v5/expansion/b06_document_encoding/validation.json'
EXPECTED={'G-B06-T59-02-TRANSFER','G-B06-T59-03-TRANSFER','G-B06-T59-05-TRANSFER','G-B06-T59-07-TRANSFER','G-B06-T59-TARGET','G-B06-REFERENCES'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def replace(x):
 if isinstance(x,str):return NEW if x==OLD else x
 if isinstance(x,list):return [replace(v) for v in x]
 if isinstance(x,dict):return {k:replace(v) for k,v in x.items()}
 return x
def expected_rows():
 rows=[replace(r) for r in read(ROOT/BASE)['updates'] if OLD in json.dumps(r)]
 assert {r['id'] for r in rows}==EXPECTED
 for r in rows:r['publication_encoding_successor']=dict(original=OLD,current=NEW,report=REPORT,scope='Same decoded content; original firmware execution and all six evidence dimensions unchanged')
 return rows
def inputs():
 files=[OLD,NEW,BASE,ENVELOPE,EXECUTION,'tools/rebind_f439_document.py','research/v5/expansion/b06_document_encoding/README.md']
 return {n:sha(ROOT/n) for n in files}
def check_history():
 r=read(ROOT/EXECUTION);assert r['passed'] and all(q['passed'] for q in r['checks'])
 assert r['input_sha256'][OLD]==sha(ROOT/OLD)
 for n,h in r['input_sha256'].items():assert sha(ROOT/n)==h,('stale firmware input',n)
 e=read(ROOT/ENVELOPE)
 for n,h in e['source_sha256'].items():assert sha(ROOT/n)==h,('stale predecessor envelope',n)
 return len(r['checks'])
def validate(d,report):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('updates')==expected_rows(),'exact six predecessor rows and unchanged dimensional status')
 need(d.get('source_sha256')==inputs()|{REPORT:sha(ROOT/REPORT)},'complete current metadata pins')
 need(report.get('input_sha256')==inputs(),'current encoding/source input identity')
 need(report.get('passed') is True and report.get('operation')=='cp1252-decode/utf8-encode-only','exact encoding operation')
 need((ROOT/NEW).read_bytes().decode('utf8')==(ROOT/OLD).read_bytes().decode('cp1252'),'identical code points and line endings')
 try:(ROOT/OLD).read_bytes().decode('utf8');old_invalid=False
 except UnicodeDecodeError:old_invalid=True
 need(old_invalid,'original encoding diagnosis')
 need(report.get('checks')==[
  dict(name='original-is-not-UTF8',passed=True),dict(name='strict-cp1252-to-UTF8-codepoint-equality',passed=True),dict(name='original-firmware-inputs-current',passed=True),dict(name='predecessor-envelope-current',passed=True),dict(name='exact-six-original-identities',passed=True),dict(name='dimensions-and-runtime-claims-unchanged',passed=True)],'exact observed metadata checks')
 need(d.get('current_reports')==[REPORT,EXECUTION],'metadata versus original execution reports')
 try:check_history()
 except (AssertionError,KeyError,OSError) as ex:errors.append(str(ex))
 return errors
def produce():
 HERE.mkdir(parents=True,exist_ok=True);old=(ROOT/OLD).read_bytes();text=old.decode('cp1252');(ROOT/NEW).write_bytes(text.encode('utf8'));count=check_history()
 try:old.decode('utf8');raise AssertionError('Original unexpectedly UTF8')
 except UnicodeDecodeError:pass
 checks=[dict(name=n,passed=True) for n in ['original-is-not-UTF8','strict-cp1252-to-UTF8-codepoint-equality','original-firmware-inputs-current','predecessor-envelope-current','exact-six-original-identities','dimensions-and-runtime-claims-unchanged']]
 report=dict(schema_version=1,passed=True,operation='cp1252-decode/utf8-encode-only',input_sha256=inputs(),original_firmware_checks_retained=count,checks=checks,limits=['Original bytes, line endings, accepted identities and prior receipts preserved.','No new firmware execution, physical result, independent review or learner result.'])
 write(ROOT/REPORT,report);d=dict(schema_version=1,scope='Versioned current-publication encoding bindings for six existing B06 identities; no content or execution changes',source_sha256=inputs()|{REPORT:sha(ROOT/REPORT)},current_reports=[REPORT,EXECUTION],updates=expected_rows(),predecessor=dict(path=BASE,envelope=ENVELOPE,original_document=OLD,encoding='cp1252'),current_document=dict(path=NEW,encoding='utf8'),remaining_author_work=[]);write(HERE/'completion_dispositions.json',d);assert not validate(d,report);print('PASS encoding-only six-identity successor; original212 firmware checks retained')
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args()
 if not a.self_test:produce();return 0
 d=read(HERE/'completion_dispositions.json');r=read(ROOT/REPORT);errors=validate(d,r);controls=[]
 if not errors:
  for kind in ['lost-identity','stale-old','stale-new','stale-report','changed-content-operation','inflated-independent','lost-original-execution','wrong-publication-path']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind=='lost-identity':x['updates'].pop()
   elif kind in ['stale-old','stale-new']:y['input_sha256'][OLD if kind=='stale-old' else NEW]='0'*64
   elif kind=='stale-report':x['source_sha256'][REPORT]='0'*64
   elif kind=='changed-content-operation':y['operation']='rewrite technical requirements'
   elif kind=='inflated-independent':x['updates'][0]['dimensions']['external_review']['state']='passed'
   elif kind=='lost-original-execution':x['current_reports']=[REPORT]
   else:x['updates'][0]['publication_encoding_successor']['current']=OLD
   found=validate(x,y);controls.append(dict(control=kind,rejected=bool(found)))
   if not found:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,original_ids=len(d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
