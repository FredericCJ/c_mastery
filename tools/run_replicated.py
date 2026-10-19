"""Execute the exact current replicated family without changing its C contracts."""
from run_replicated_current import run
if __name__=='__main__':
    import argparse
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--report',default='reviews/v5/expansion/b07/replicated_campaign.json')
    raise SystemExit(run(p.parse_args().report))
