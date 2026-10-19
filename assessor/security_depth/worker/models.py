"""Independent descriptor schedule and selected MPU design assessment."""
import hashlib,itertools,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 fixture=json.loads((ROOT/'fixtures/embedded/PR-T35-02.json').read_text())
 inherited=[x['fd'] for x in fixture['input']['descriptors'] if not x['cloexec']]
 permissions=[x in fixture['input']['allowed'] for x in fixture['input']['requests']]
 assert inherited==fixture['expected']['inherited'] and permissions==fixture['expected']['allowed_results']
 schedules=[]
 for steps in itertools.permutations(['open','fcntl','fork_exec']):
  if steps.index('open')>steps.index('fcntl'):continue
  leaked=steps.index('open')<steps.index('fork_exec')<steps.index('fcntl')
  schedules.append(dict(steps=steps,split_creation_leaks=leaked,atomic_cloexec_leaks=False))
 assert len(schedules)==3 and sum(x['split_creation_leaks'] for x in schedules)==1
 regions=[dict(base=0,size=2**21,ap=6,xn=False),dict(base=0x20000000,size=2**17,ap=3,xn=True)]
 for r in regions:assert r['size']>=32 and not r['size']&(r['size']-1) and r['base']%r['size']==0 and r['base']+r['size']<=2**32
 assert regions[0]['base']+regions[0]['size']<=regions[1]['base']
 matrix=[]
 for privilege,kind,operation in itertools.product([False,True],['code','ram','unmapped'],['read','write','execute']):
  r=regions[0] if kind=='code' else regions[1] if kind=='ram' else None
  result='background-contract' if r is None and privilege else 'fault' if r is None else 'allow' if (operation!='write' or r['ap']==3) and (operation!='execute' or not r['xn']) else 'fault'
  matrix.append(dict(privileged=privilege,region=kind,operation=operation,result=result))
 report_path=ROOT/'research/v5/expansion/b06/boot_pair_validation.json';report=json.loads(report_path.read_text())
 assert report['passed'] and all(c['passed'] for c in report['checks'])
 for n,h in report['input_sha256'].items():assert sha(ROOT/n)==h,('stale actual MPU producer',n)
 observations=[c for c in report['checks'] if 'actual CPU execute' in c['label']]
 assert len(observations)==12 and sum('FB_GUEST_MPU_FAULT:' in c.get('stdout','') for c in observations)==8
 print(json.dumps(dict(status='pass',fixture=dict(inherited=inherited,allowed=permissions),cloexec_schedules=schedules,mpu=dict(regions=regions,matrix=matrix,prior_actual_cpu_report=report_path.relative_to(ROOT).as_posix(),prior_actual_cpu_report_sha256=sha(report_path),prior_cpu_cases=[c['label'] for c in observations],new_cpu_execution=False,privileged_code_can_reconfigure=True,dma_covered_by_cpu_mpu=False,physical=False),enforcement=dict(iso_c='type/lifetime semantics; no hostile-module memory isolation',diagnostics='selected constraint and warning evidence, not runtime authority',runtime='explicit application gate and scoped bounds',os='tested descriptor rights, exec inheritance and private mappings',discipline='live borrowed inputs and serialized ownership'),limits=['Finite logical fork/open schedule, not multithreaded fork stress.','Selected nonoverlapping MPU design and source-bound prior MPS2 execution reused; not new hardware observation or complete MPU emulator.','Private process mappings and read-only descriptor do not remove ambient reopen/network/same-user authority.']),indent=2))
if __name__=='__main__':main()
