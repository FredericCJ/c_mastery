"""Run allocated reference graphs and real last-owner races under C17."""
from run_ownership_module import run
if __name__ == '__main__':raise SystemExit(0 if run('references') else 1)
