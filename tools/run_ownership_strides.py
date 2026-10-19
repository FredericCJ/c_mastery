"""Run actual C17 exact-set and padded-tile borrow checks."""
from run_ownership_module import run
if __name__=="__main__":raise SystemExit(0 if run("strides") else 1)
