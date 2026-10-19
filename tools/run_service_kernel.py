"""Author-only wrapper: explicitly selects reference implementations."""
import importlib.util
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('sd_guest',ROOT/'labs/service_depth/adapters/guest.py')
guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
if __name__=='__main__':
    selected=['--kernel-source',str(ROOT/'assessor/kernel_smp'),
      '--kernel-lab',str(ROOT/'labs/kernel_smp'),
      '--adapter-source',str(ROOT/'assessor/service_depth/kernel'),
      '--store-source',str(ROOT/'assessor/storage/store.c'),
      '--storage-lab',str(ROOT/'labs/storage'),
      '--build-root',str(ROOT/'build/service_depth/kernel')]
    raise SystemExit(guest.main(selected+sys.argv[1:],provenance='author-reference-components; never learner work',extra_inputs=[Path(__file__)]))
