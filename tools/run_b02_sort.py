"""Run required stable-sort variants with the frozen profile producer."""
from run_b02_followthrough import run
if __name__=="__main__":raise SystemExit(0 if run("sort") else 1)
