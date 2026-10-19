"""Original synthetic captures expose decoded-byte and changed-mode errors."""
import importlib.util,json,pathlib,sys
root=pathlib.Path(__file__).resolve().parents[3];s=importlib.util.spec_from_file_location('capture',root/'labs/serial_followthrough/i2c/capture.py');m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
def main():
 mutant=len(sys.argv)>1
 good=dict(mode='fast',rise_ns=250,uncertainty_ns=10,scl_low_ns=1400,scl_high_ns=700,decoded=[160,7,161,90])
 for change,expected in [({},True),({'rise_ns':310},False),({'rise_ns':295},False),({'scl_high_ns':605},False),({'mode':'standard','rise_ns':800,'scl_low_ns':5000,'scl_high_ns':4500},True)]:
  r=m.assess(good|change,mutant)
  if r['timing_window_pass']!=expected or r['physical_gate']!='requires raw capture and assessor verification':print('CAPTURE_ASSERT: decoded bytes cannot waive timing or source evidence');return 43
 a=m.nominal_rise(2200,100);b=m.nominal_rise(4700,100);c=m.nominal_rise(2200,400)
 assert a<=300<b and 300<c<=1000
 print(json.dumps(dict(passed=True,captures=5,rational_rise_ns=[str(x) for x in [a,b,c]],scope='Original synthetic observations only; no physical measurement or automatic evidence attestation')));print('CAPTURE_INVESTIGATION_PASS');return 0
if __name__=='__main__':raise SystemExit(main())
