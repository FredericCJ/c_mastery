"""Validate capture completeness only; a schema pass is not witness independence."""
import argparse,json,pathlib,re
def validate(d):
 errors=[]
 for field in ['board_marking','assembly_revision','device_marking','silicon_revision','power_configuration','clock_source','boot_readback','instrument_setup']:
  if not isinstance(d.get(field),str) or not d[field].strip():errors.append('missing '+field)
 for field in ['image_sha256','elf_sha256']:
  if not re.fullmatch('[0-9a-f]{64}',str(d.get(field,''))):errors.append('missing digest '+field)
 if not isinstance(d.get('voltage_V'),(float,int)) or not 2.7<=d['voltage_V']<=3.6:errors.append('selected voltage profile')
 if not d.get('tool_versions'):errors.append('missing tool versions')
 if d.get('status')!='observed':errors.append('not observed')
 modes={'debugger','software','watchdog','reset-pin','power-cycle','bootloader-offset'}
 runs=d.get('runs',[])
 if {r.get('mode') for r in runs}!=modes:errors.append('missing separate reset modes')
 for r in runs:
  if not isinstance(r.get('raw_flags'),int) or not 0<=r['raw_flags']<2**32:errors.append('missing raw flags')
  if r.get('stage')!=5:errors.append('startup stage incomplete')
  if not r.get('raw_capture') or not r.get('attach_reset_policy') or not r.get('utc'):errors.append('missing capture provenance')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('capture',type=pathlib.Path);a=p.parse_args();e=validate(json.loads(a.capture.read_text(encoding='utf8')));print(json.dumps(dict(passed=not e,errors=e,scope='Packet schema only, not authenticity or independent review')));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
