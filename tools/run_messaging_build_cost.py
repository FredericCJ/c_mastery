"""Observe clean, no-op and precise header-triggered builds in a private copy."""
import argparse
from datetime import datetime,timezone
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time
from run_messaging_cost import SOURCE,source_pins

ROOT=Path(__file__).resolve().parents[1]


def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--report',default='research/v5/modern/messaging_build_cost_validation.json');args=parser.parse_args()
    if os.name=='nt':raise SystemExit('Run in the declared WSL/Linux tool environment.')
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    area=ROOT/'build/modern_completion/messaging-build-cost'/stamp;area.mkdir(parents=True)
    logs=ROOT/'reviews/v5/expansion/modern_completion/runs'/(stamp+'-messaging-build-cost');logs.mkdir(parents=True)
    inputs=source_pins();inputs['tools/run_messaging_build_cost.py']=sha(Path(__file__))
    records=[];profiles=[];error=None;copied={}
    def execute(command,label):
        start=time.monotonic();r=subprocess.run([str(x) for x in command],cwd=ROOT,text=True,capture_output=True,timeout=180)
        log=logs/(str(len(records)).zfill(3)+'-'+label+'.log');log.write_text(r.stdout+'\n'+r.stderr,encoding='utf8')
        records.append(dict(command=[str(x) for x in command],exit_code=r.returncode,seconds=time.monotonic()-start,
                            log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        if r.returncode:raise AssertionError((label,r.returncode,r.stdout[-2000:],r.stderr[-2000:]))
        return records[-1]
    def objects(build):
        return {p.relative_to(build).as_posix():dict(sha256=sha(p),mtime_ns=p.stat().st_mtime_ns) for p in build.rglob('*.o')}
    try:
        original=[ROOT/p for p in SOURCE]
        original+=list((ROOT/'labs/messaging/followthrough').glob('*.h'))
        original+=[ROOT/'labs/messaging/include/messaging.h']
        assert len({p.name for p in original})==len(original),'flat copy collision'
        for compiler in ('gcc','clang'):
            for optimization in ('-O0','-O3'):
                label=compiler+optimization[1:];folder=area/label;source=folder/'source';source.mkdir(parents=True)
                for p in original:
                    dest=source/p.name;shutil.copyfile(p,dest);assert sha(p)==sha(dest)
                    copied[dest.relative_to(ROOT).as_posix()]={'source':p.relative_to(ROOT).as_posix(),'sha256':sha(dest)}
                recipe='cmake_minimum_required(VERSION 3.20)\nproject(messaging_build_cost C)\nset(CMAKE_C_STANDARD 17)\nset(CMAKE_C_EXTENSIONS OFF)\nfind_package(Threads REQUIRED)\n'
                recipe+='add_executable(measure '+' '.join(Path(p).name for p in SOURCE)+')\n'
                recipe+='target_include_directories(measure PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")\n'
                recipe+='target_compile_options(measure PRIVATE '+optimization+' -Wall -Wextra -Wpedantic -Werror)\n'
                recipe+='target_link_libraries(measure PRIVATE Threads::Threads)\n'
                (source/'CMakeLists.txt').write_text(recipe,encoding='utf8')
                recipe_path=logs/(label+'-CMakeLists.txt');recipe_path.write_text(recipe,encoding='utf8')
                build=folder/'build'
                execute(['cmake','-G','Ninja','-S',source,'-B',build,'-DCMAKE_C_COMPILER='+compiler],label+'-configure')
                clean=execute(['cmake','--build',build,'--verbose','-j','1'],label+'-clean');first=objects(build)
                assert len(first)==len(SOURCE)
                noop=execute(['cmake','--build',build,'--verbose','-j','1'],label+'-noop');second=objects(build)
                assert first==second,'no-op unexpectedly rebuilt objects'
                # Only private copied header mtime changes; canonical sources stay untouched.
                header=source/'cost_reference.h';latest=max(p['mtime_ns'] for p in second.values())
                # DrvFS can round utime to seconds although object writes carry
                # finer timestamps. Verify the trigger instead of assuming it.
                for _ in range(8):
                    os.utime(header,None)
                    if header.stat().st_mtime_ns>latest:break
                    time.sleep(0.3)
                assert header.stat().st_mtime_ns>latest,'header trigger did not advance beyond object times'
                incremental=execute(['cmake','--build',build,'--verbose','-j','1'],label+'-header-incremental');third=objects(build)
                changed=sorted(p for p in third if third[p]['mtime_ns']!=second[p]['mtime_ns'])
                assert {Path(p).name for p in changed}=={'cost_driver.c.o','cost_reference.c.o'},changed
                execute(['python3',ROOT/'labs/messaging/followthrough/cost_check.py',build/'measure','128'],label+'-semantic-check')
                profiles.append(dict(profile=label,clean_seconds=clean['seconds'],noop_seconds=noop['seconds'],
                                     header_incremental_seconds=incremental['seconds'],changed_objects=changed,
                                     object_count=len(first),recipe=recipe_path.relative_to(ROOT).as_posix(),recipe_sha256=sha(recipe_path)))
                print('messaging build cost '+label+': clean/no-op/two-object rebuild and26 equal-work checks passed',flush=True)
    except (AssertionError,OSError,subprocess.TimeoutExpired) as exc:error=repr(exc)
    after=source_pins();after['tools/run_messaging_build_cost.py']=sha(Path(__file__))
    stable=inputs==after;passed=error is None and stable
    data=dict(schema_version=1,passed=passed,status='passed' if passed else 'failed',producer='tools/run_messaging_build_cost.py',
              recorded_at=datetime.now(timezone.utc).isoformat(),inspected_paths=inputs,source_stable=stable,
              input_sha256=hashlib.sha256(json.dumps(inputs,sort_keys=True).encode()).hexdigest(),executions=records,
              profiles=profiles,private_source_copies=copied,error=error,
              limitations=['Single observed clean/no-op/incremental build per compiler/optimization; not statistical compiler benchmarks.',
                           'Private byte-identical flat copies and generated minimal CMake/Ninja build; not whole-curriculum rebuild costs.',
                           'Only copied cost_reference.h timestamp changed; declared two dependent translation units rebuilt and semantic checks reran.',
                           'Serial build timing includes process/filesystem overhead on the shared WSL host; no universal build-cost claim.'])
    (ROOT/args.report).write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    if error:print(error)
    return 0 if passed else 1


if __name__=='__main__':raise SystemExit(main())
