import copy,importlib.util,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('evidence',ROOT/'labs/capstone_followthrough/firmware/evidence.py');e=importlib.util.module_from_spec(spec);spec.loader.exec_module(e)
def main():
    assert e.timing(65530,6,1,16000000,12,20)
    assert e.timing(65530,6,1,8000000,12,20)
    assert not e.timing(65530,6,2,16000000,12,20)
    assert e.timing(65530,6,2,16000000,65548,70000)
    assert not e.timing(65530,6,2,16000000,65548,20)
    assert (12*1000000000)//16000000==750 and (12*1000000000)//8000000==1500
    rows=[dict(claim=k,evidence=list(v),physical=False,supported_by_emulator=k not in ['dma-lifetime']) for k,v in e.REQUIRED.items()];assert e.matrix(rows,'cpu-only')
    for claim,fake in [('parser','elf-checksum'),('helpers','host-pass'),('dma-lifetime','pointer-range'),('startup','cross-build'),('reset-recovery','no-crash'),('safe-output','CRC')]:
        bad=copy.deepcopy(rows);r=next(r for r in bad if r['claim']==claim);r['evidence']=[fake];assert not e.matrix(bad,'cpu-only')
    bad=copy.deepcopy(rows);bad[0]['physical']=True;bad[0]['evidence']+=['host-timestamp','elf-checksum'];assert not e.matrix(bad,'cpu-only')
    bad=copy.deepcopy(rows);next(r for r in bad if r['claim']=='dma-lifetime')['supported_by_emulator']=True;assert not e.matrix(bad,'cpu-only');assert e.matrix(bad,'selected-device-model')
    if len(sys.argv)==3:
        folder=pathlib.Path(sys.argv[1]);manifest=json.loads((folder/'release.json').read_text());assert e.release(folder/'image.elf',folder/'image.bin',manifest)
        stale=pathlib.Path(sys.argv[2]);bad=copy.deepcopy(manifest);bad['elf']=dict(path=str(stale),sha256=e.sha(stale));assert not e.release(stale,folder/'image.bin',bad)
        debug=json.loads((folder/'debug.json').read_text());assert e.debug_pair(folder/'stripped.elf',folder/'symbols.elf',debug);bad=copy.deepcopy(debug);bad['symbols_sha256']='0'*64;assert not e.debug_pair(folder/'stripped.elf',folder/'symbols.elf',bad)
    print('CAP_MCU_EVIDENCE_PASS categories8 timing6 stale-bundle debug-identity physical-pending')
if __name__=='__main__':main()
