"""Construct an honest local debugger manifest; no device defaults are inferred."""
import hashlib,json

def digest(value):return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':')).encode()).hexdigest()

def make(probe,image_sha256,versions):
 return {'schema_version':1,'environment':'original owned QEMU mps2-an386 teaching target',
  'probe_firmware':{'state':'not-applicable','reason':'QEMU GDB server is a software endpoint, not a physical probe'},
  'board_revision':{'state':'unknown','reason':'No physical board identified'},
  'voltage_source':{'state':'unknown','reason':'No physical power measurement'},
  'reset_wiring':{'state':'unknown','reason':'No physical reset wire inspected'},
  'reset_strategy':{'state':'observed','value':'New QEMU process starts halted at reset; no physical SRST assertion'},
  'target_id':{'state':'observed','value':[r['output'] for r in probe['rows'] if r.get('command')=='x/wx 0xe000ed00']},
  'server_command':probe['qemu_command'],'server_version':versions['qemu-system-arm'],
  'debugger_version':versions['gdb'],'script_sha256':hashlib.sha256(probe['script'].encode()).hexdigest(),
  'configuration_sha256':digest(probe['qemu_command']),'image_sha256':image_sha256,
  'transport':probe['transport'],'command_elapsed_ns':[r['elapsed_ns'] for r in probe['rows'] if 'command' in r]}

def validate(record,probe,image_sha256,versions):
 if record!=make(probe,image_sha256,versions):raise ValueError('Notebook does not bind actual image/configuration/observations or invents physical evidence')
 if not all(record[k]['state']=='unknown' for k in ('board_revision','voltage_source','reset_wiring')):raise ValueError('Physical state was inferred')
 return {'physical_fields':'unknown','transport':record['transport']['kind'],'observed_command_timings':len(record['command_elapsed_ns'])}
