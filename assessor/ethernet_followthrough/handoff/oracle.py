"""Independent integer expectations and literal protocol observation packets."""
import json,pathlib,sys
t=pathlib.Path(sys.argv[1]).read_text();case=sys.argv[2]
dt=2000000;drift=(dt*100+999999)//1000000
packets={
 'route':'OBS handoff_route local=c0000209 gateway=c0000201 final=c6336409 frame=60 unchanged_on_refusal',
 'clock':f'OBS handoff_clock fixed=100,105,120 interval={98+dt-drift},{102+dt+drift} rounded=98,104 max={2**32-1-((2**32-1)*1000+999999)//1000000},{2**32-1+((2**32-1)*1000+999999)//1000000}',
 'divergence':'OBS handoff_divergence link=0 bad_fcs=2 repaired_frame_missing_route=3',
 'budget':f'OBS handoff_budget 72 90 110 {110*8*1000//100} 40 64 84 {84*8*1000//10}',
 'stack':'EH_STACK_PASS actual_raw_GET_frames_stream_time_events_DMA_ACK_TIME_WAIT',
 'reassembly':'EH_REASSEMBLY_PASS holes=8 output=44 tombstone=60000 immutable_deadline=120000 conflicting_overlap_poison'}
keys=list(packets) if case=='all' else [case]
for k in keys:assert t.count(packets[k])==1,(k,t)
if 'stack' in keys:
 for marker in ['NH_PASS actual_driver_parser_encoder_raw_peer','NH_LINK_PASS']:assert marker in t
print(json.dumps(dict(status='pass',keys=keys,packets=len(keys),limits='Independent arithmetic and literal finite trace expectations; no general interoperability or physical measurements.')))
