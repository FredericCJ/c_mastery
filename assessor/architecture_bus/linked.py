"""Derive the test's numeric placement tags from actual linked typed objects."""
import hashlib,json,pathlib,sys
def symbols(p):
 result={}
 for line in p.read_text().splitlines():
  fields=line.split()
  if len(fields)==4:result[fields[3]]=[int(fields[0],16),int(fields[1],16)]
 return result
def main():
 first,changed,out=map(pathlib.Path,sys.argv[1:]);a,b=symbols(first),symbols(changed);assert all(a[n][1]==b[n][1]==16 for n in ['ab_cpu_only','ab_dma_candidate','mb_dma_source'])
 if a['ab_dma_candidate']!=[0x20020000,16] or b['ab_dma_candidate']!=[0x20020000,16]:print('MB_LINK_REJECT: DMA destination');raise SystemExit(1)
 assert a['mb_dma_source']==b['mb_dma_source']==[0x20000010,16];assert a['ab_cpu_only']==[0x20000000,16] and b['ab_cpu_only']==[0x10000000,16]
 values=dict(MB_LINK_SOURCE=a['mb_dma_source'][0],MB_LINK_DESTINATION=a['ab_dma_candidate'][0],MB_CCM_LINK_SOURCE=b['mb_dma_source'][0],MB_CCM_LINK_DESTINATION=b['ab_dma_candidate'][0],MB_LINK_CPU_SRAM=a['ab_cpu_only'][0],MB_LINK_CPU_CCM=b['ab_cpu_only'][0]);out.mkdir(parents=True,exist_ok=True);(out/'linked_map.h').write_text('/* Generated from actual linked symbols, not substituted fixture constants. */\n'+''.join(f'#define {n} 0x{v:08x}u\n' for n,v in values.items()));(out/'linked.json').write_text(json.dumps(dict(values=values,original=a,changed=b,symbol_sources={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [first,changed]}),indent=2)+'\n');print('OBS actual typed linker ledger')
if __name__=='__main__':main()
