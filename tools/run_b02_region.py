"""Run required region cohorts with actual frozen providers."""
from run_b02_followthrough import run
if __name__=="__main__":raise SystemExit(0 if run("region") else 1)
