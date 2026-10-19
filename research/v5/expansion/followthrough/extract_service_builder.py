"""One-time author migration; the public builder accepts explicit learner imports."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
source=ROOT/'tools/run_service_kernel.py'
text=source.read_text()
start=text.index('ROOT=Path(__file__)')
end=text.index('\ndef sha(',start)
text=text[:start]+"PUBLIC=Path(__file__).resolve().parent\n"+text[end:]
text=text.replace('def main():','def main(argv=None, provenance="explicit-user-supplied-imports", extra_inputs=()):')
text=text.replace(';args=p.parse_args()', '''
    p.add_argument('--kernel-source',type=Path,required=True)
    p.add_argument('--kernel-lab',type=Path,required=True)
    p.add_argument('--adapter-source',type=Path,required=True)
    p.add_argument('--store-source',type=Path,required=True)
    p.add_argument('--storage-lab',type=Path,required=True)
    p.add_argument('--build-root',type=Path,required=True)
    args=p.parse_args(argv)
    ROOT=Path.cwd().resolve()
    SOURCE=args.kernel_source.resolve();LAB=args.kernel_lab.resolve();ADAPTER=args.adapter_source.resolve()
    STORAGE=args.storage_lab.resolve();STORE=args.store_source.resolve()
    spec=importlib.util.spec_from_file_location('cmk_guest',LAB/'tools/guest.py')
    guest=importlib.util.module_from_spec(spec);spec.loader.exec_module(guest)
    def label(path):
        path=path.resolve()
        return path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else str(path)
''')
text=text.replace("ROOT/'build/service_depth/kernel'",'args.build_root.resolve()')
text=text.replace("'Adapter imports assessor reference components, never learner artifacts.'", "'Imported artifact provenance: '+provenance")
text=text.replace("ROOT/'labs/service_depth/adapters/kernel_abi.h'", "PUBLIC/'kernel_abi.h'")
text=text.replace("ROOT/'labs/service_depth/adapters/receipt.h'", "PUBLIC/'receipt.h'")
text=text.replace("ROOT/'labs/service_depth/adapters'", 'PUBLIC')
text=text.replace("ROOT/'labs/storage/include/store.h'", "STORAGE/'include/store.h'")
text=text.replace("ROOT/'labs/storage/include'", "STORAGE/'include'")
text=text.replace("ROOT/'assessor/storage/store.c'", 'STORE')
text=text.replace("report['input_sha256']={x.relative_to(ROOT).as_posix():sha(x) for x in inputs if x.is_file()}", "inputs += list((PUBLIC/'tests').glob('*.c'))+list(extra_inputs)\n    report['input_sha256']={label(x):sha(x) for x in inputs if x.is_file()}")
text=text.replace("ADAPTER/('user.c' if args.suite=='services' else 'receipt_user.c')", "PUBLIC/'tests'/('user.c' if args.suite=='services' else 'receipt_user.c')")
text=text.replace('"""Build an explicit reference adapter, never modifying the retained kernel sources."""', '"""Build and exercise explicitly supplied service/kernel/store implementations.\n\nNo assessor discovery, download, or fallback; supplied source trees are read-only.\nThe staged hooks implement the published integration protocol, not its mechanisms.\n"""')
(ROOT/'labs/service_depth/adapters/guest.py').write_text(text)
tests=ROOT/'labs/service_depth/adapters/tests';tests.mkdir(exist_ok=True)
for name in ('user.c','receipt_user.c'):
    old=ROOT/'assessor/service_depth/kernel'/name
    (tests/name).write_bytes(old.read_bytes())
    old.unlink()
source.write_text('''"""Author-only wrapper: explicitly selects reference implementations."""
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
''')
