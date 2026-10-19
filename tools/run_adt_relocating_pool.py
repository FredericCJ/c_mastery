"""Eight real compiler profiles, including thread instrumentation."""
import argparse
import run_adt_module as campaign
campaign.PROFILES=[c+'-'+m for c in ['gcc','clang'] for m in ['debug','release','asan','tsan']]
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report');a=p.parse_args()
    raise SystemExit(0 if campaign.run('relocating_pool',a.report) else 1)
