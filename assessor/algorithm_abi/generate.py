"""Exact controlled derivations of original teaching assembly, never frozen-provider edits."""
import pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def changed(text,old,new,count=1):
 assert text.count(old)==count,(old,text.count(old));return text.replace(old,new)
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
 original=(ROOT/'assessor/algorithms/x86_64.S').read_text(encoding='utf8')
 scale=changed(original,'cmpl %edx,(%rdi,%r8,4)','cmpl %edx,(%rdi,%r8,8)')
 overread=changed(original,'.Llower_done:\n','.Llower_done:\n    movl (%rdi,%rsi,4),%r10d\n')
 (out/'scale.S').write_text(scale,encoding='utf8');(out/'overread.S').write_text(overread,encoding='utf8')
 for arch in ['x86_64','thumb']:
  probe=(ROOT/('labs/algorithms/tests/abi_'+arch+'.S')).read_text(encoding='utf8')
  # Change every seed and matching comparison together; preserve all register checks.
  replacements={'0x1122':'0x7812','0x2233':'0x3923','0x3344':'0x4a34','0x4455':'0x5b45','0x5566':'0x6c56','0x6677':'0x7d67','0x7788':'0x1e78','0x8899':'0x2f89','0xdeadbeef':'0x7139acde','0xabcdef01':'0x9234abfe','0x12345678':'0x4589be01','0x23456789':'0x5712ace0','0xdead':'0x3bed'}
  for a,b in replacements.items():
   if a in probe:probe=probe.replace(a,b)
  (out/('probe_'+arch+'.S')).write_text(probe,encoding='utf8')
 print('OBS generated exact scale/overread mutations and changed probe poisons')
if __name__=='__main__':main()
