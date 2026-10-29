"""Run C17 struct ow_cell callback and pthread guard-lifetime checks."""
from run_ownership_module import run
if __name__=="__main__":raise SystemExit(0 if run("cell_guard") else 1)
