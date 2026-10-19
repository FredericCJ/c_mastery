"""Public C17 core build; assessor sources require explicit arguments."""
import argparse,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--cc',default='gcc');p.add_argument('--output',required=True);p.add_argument('--memory-source');p.add_argument('--foundation-source');a=p.parse_args();root=Path(__file__).resolve().parents[2]
cmd=[a.cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O2','-I'+str(root/'labs/memory_followthrough'),'-I'+str(root/'labs/foundations/include'),str(root/'labs/memory_followthrough/tests.c'),a.memory_source or str(root/'labs/memory_followthrough/memory.c'),a.foundation_source or str(root/'labs/foundations/src/foundation.c'),'-o',a.output]
raise SystemExit(subprocess.run(cmd).returncode)
