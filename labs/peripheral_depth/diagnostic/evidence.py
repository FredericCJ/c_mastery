"""Validate the scope of evidence and invalidate claims under changed assumptions.

This checks supplied records; it cannot authenticate a claimed physical experiment.
"""
from pathlib import Path
import argparse,json
CLAIMS={
 'application_wire':('application',{'model','host','guest','physical'},{'application','compiler'}),
 'udp_loopback':('stack',{'host'},{'application','os','stack_configuration'}),
 'frame_parser':('stack',{'model','host','guest'},{'frame_source','compiler'}),
 'descriptor_model':('controller',{'model','host','guest'},{'descriptor_source','compiler'}),
 'dma_board':('controller',{'physical'},{'mcu','board','routing','clock','firmware','dma_layout','compiler'}),
 'mdio_board':('controller',{'physical'},{'mcu','phy','board','routing','clock','firmware'}),
 'phy_link':('phy',{'physical'},{'phy','board','routing','clock','instrument','peer'}),
}
def inspect(record,current):
 errors=[];valid=[];invalidated=[];physical=[];seen=set();columns=set()
 if record.get('record_kind') not in {'synthetic-fixture','author-capture'}:errors.append('record kind missing')
 for r in record.get('evidence',[]):
  key=r.get('id');claim=r.get('claim');env=r.get('environment');dependencies=r.get('dependencies',{})
  if not isinstance(key,str) or key in seen:errors.append('missing/duplicate evidence identity')
  seen.add(key)
  if claim not in CLAIMS:errors.append(str(key)+': unknown claim');continue
  column,allowed,required=CLAIMS[claim]
  columns.add(column)
  if r.get('column')!=column:errors.append(key+': wrong evidence column')
  if env not in allowed:errors.append(key+': environment cannot support claim')
  if not required.issubset(dependencies):errors.append(key+': missing dependency')
  if any(not isinstance(v,str) or not v for v in dependencies.values()):errors.append(key+': unidentified dependency')
  if not isinstance(r.get('observation'),str) or not r['observation']:errors.append(key+': missing concrete observation')
  if env=='physical':
   needed={'source_passages','raw_capture_sha256','instrument_setup','uncertainty','reset_state','isolation','failure_control'}
   capture=r.get('capture',{})
   if not needed.issubset(capture) or any(not capture[k] for k in needed if k in capture):errors.append(key+': incomplete capture protocol')
   digest=capture.get('raw_capture_sha256','')
   if len(digest)!=64 or any(c not in '0123456789abcdef' for c in digest):errors.append(key+': invalid capture identity')
   if record.get('record_kind')=='author-capture':physical.append(key)
  changed=sorted(k for k,v in dependencies.items() if current.get(k)!=v)
  if changed:invalidated.append(dict(id=key,changed=changed))
  else:valid.append(key)
 if columns!={'application','stack','controller','phy'}:errors.append('four evidence columns required')
 return dict(passed=not errors,errors=errors,still_applicable=valid,invalidated=invalidated,
             physical_claims_requiring_authenticity_review=physical,
             physical_validation=False,
             limit='Record consistency and dependency transfer only; neither authenticity nor conformance is established.')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('record',type=Path);p.add_argument('current',type=Path);a=p.parse_args()
 r=inspect(json.loads(a.record.read_text()),json.loads(a.current.read_text()));print(json.dumps(r,indent=2));raise SystemExit(not r['passed'])
