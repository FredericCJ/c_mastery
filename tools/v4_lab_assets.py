"""Regenerate v4 lab manifests, unit cards and additive CMake wiring.

Does not modify the historical project catalog or publication inputs.
"""
from pathlib import Path
import json,re
ROOT=Path(__file__).resolve().parents[1]
def write(path,text):
 p=ROOT/path;p.parent.mkdir(parents=True,exist_ok=True);p.write_text(text.rstrip()+'\n',encoding='utf-8')
def project(pid):
 return json.loads((ROOT/'curriculum/projects'/f'{pid}.md').read_text(encoding='utf-8').split('```json')[1].split('```')[0])
specs={
 'firmware_io':[
 ('LAB-FW-01','PR-T27-03','startup','Reset data and BSS boundaries','Copy only the linker-declared data span and zero only BSS; preserve surrounding canaries.'),
 ('LAB-FW-02','PR-T29-01','vectors','Stack and reset-vector validation','Reject unaligned stacks, out-of-RAM stack tops, non-Thumb reset addresses and out-of-flash entry points.'),
 ('LAB-FW-03','PR-T28-01','gpio','Atomic GPIO adapter','Write set/reset registers without read-modify-write side effects; reject invalid pins.'),
 ('LAB-FW-04','PR-T28-02','uart','Bounded UART publication','Wait for TXE with a finite poll budget; never publish data on timeout.'),
 ('LAB-FW-05','PR-T38-05','spi','SPI selection and completion','Assert chip select, exchange a byte, wait for not-busy and release selection on every failure.'),
 ('LAB-FW-06','PR-T38-04','i2c','Conditional I2C bus recovery','With exclusive bus authority, clock at most nine pulses, observe release and produce STOP; fail on stuck lines.'),
 ('LAB-FW-07','PR-T28-03','timer','Timer/PWM arithmetic and update order','Reject inexact or out-of-range divisors; configure disabled then publish period/duty atomically at the adapter boundary.'),
 ('LAB-FW-08','PR-T28-05','adc','ADC deadline and unchanged output','Start conversion, poll bounded completion, mask sample width and preserve output on timeout.'),
 ('LAB-FW-09','PR-T29-04','dma','DMA generation and cancellation','Model CPU, device and cancelling ownership; require matching generation and observed quiescence before reuse.'),
 ('LAB-FW-10','PR-T39-07','ethernet','MDIO readiness and TX descriptors','Wait bounded MDIO readiness and populate descriptor length/generation before publishing ownership; reject device-owned reuse.'),
 ('LAB-FW-11','PR-T38-08','rs485','RS-485 true-completion turnaround','Keep driver enable asserted until TC, not merely TXE. A timeout after publication remains uncertain and requires explicit recovery.'),
 ('LAB-FW-12','PR-T30-05','watchdog','Progress-qualified watchdog and fault trace','Feed only after all required progress bits; retain the first supplied fault observation without feeding.')],
 'algorithms':[
 ('LAB-EX-01','PR-T14-02','array','Failure-preserving dynamic-array reserve','Use explicit typed allocation callbacks, check byte-size overflow, preserve storage on failure and invalidate old borrows only after success.'),
 ('LAB-EX-02','PR-T14-03','list','Typed linked queue with reusable slots','Maintain head/tail and a typed eight-slot pool across full, empty, wrap and reuse transitions.'),
 ('LAB-EX-03','PR-T16-04','freelist','Byte-offset free-list coalescing','Split first-fit spans, reject overlapping returns, coalesce both neighbors and preserve state when capacity is unavailable.'),
 ('LAB-EX-04','PR-T15-03','radix','Unsigned stable radix passes','Implement four byte passes with caller-owned scratch, explicit capacity, and unsigned ordering over the complete uint32 domain.'),
 ('LAB-EX-05','PR-T15-06','dp','Bounded zero-one dynamic programming','Derive descending capacity updates, reject zero weights and arithmetic overflow; demonstrate why ascending updates change the problem.'),
 ('LAB-EX-06','PR-T17-05','filter','Streaming FIR and first-order IIR','Preserve sample history across calls; use wide accumulators and explicit signed rounding. Compare impulse and extreme-value traces.')],
 'communications':[
 ('LAB-EX-07','PR-T40-05','reassembly','Granted reassembly with expiry','Reuse EDU7 fragment acceptance; gate publication by link authority, expire incomplete state, preserve duplicate suppression and delivered data.'),
 ('LAB-EX-08','PR-T41-04','arp','Active ARP queue and bounded retries','Retain one immutable payload, match the resolved peer, bound retry count, and separate address resolution from payload transfer.'),
 ('LAB-EX-09','PR-T41-06','tcp_fault','TCP retry through a dropping adapter','Reuse net_tick and retained packets; inject sink losses, count them independently, and verify bounded retry exhaustion without duplicating TCP.'),
 ('LAB-EX-10','PR-T38-11','canfd','CAN FD configuration and loopback','Respect INIT/CCE configuration order, bounded acknowledgement and supported FD lengths; keep H745 clock/cache/message-RAM/cable obligations open.')],
 'foundations':[
 ('LAB-EX-11','PR-T44-10','diagnosis','Sealed causal diagnosis and admission repair','Before inspecting the test source, seal predictions and the first invalid state. Repair failure-atomic admission and retain a delayed changed-capacity variant.'),
 ('LAB-EX-12','PR-T43-08','measurement','Reusable component measurement differences','Capture wall/CPU time, one-wrap cycle difference, read bytes, accepted/completed writes and I/O counts separately. Reject clock or counter reset without changing output.')],
 'integration':[
 ('LAB-IN-01','PR-P1-02','p1','Byte cursor and queue boundary','Import the earlier cursor, decimal parser and ring. Publish complete records only; preserve prior commits and report the exact accepted prefix on exhaustion.'),
 ('LAB-IN-02','PR-P2-02','p2','Queue to short-transfer adapter','Import the ring and write-all component. Retain the active byte across retryable failure and resume without replaying an accepted prefix.'),
 ('LAB-IN-03','PR-P3-02','p3','Incremental codec and malformed-input campaign','Import the cursor/parser/ring rather than duplicating them. Admit fragmented length-delimited records and enter an explicit failed state on malformed input.'),
 ('LAB-IN-04','PR-P4-02','p4','DMA completion to foreground queue','Import the DMA generation protocol and ring. Do not enqueue completion before observed quiescence or consume ownership when the queue is full.'),
 ('LAB-IN-05','PR-P5-02','p5','Queue shutdown after asynchronous quiescence','Import the asynchronous lifecycle and ring; cancellation is a request. Drain only after completion, and preserve exactly-once release on repeated shutdown.'),
 ('LAB-IN-06','PR-P6-02','p6','Chunked filter with distinct measurement','Reuse the filter, checked extent and measurement capture. Show chunk invariance and refuse insufficient output capacity before modifying state.'),
 ('LAB-IN-07','PR-CAP-LINUX-02','linux','VM result to checked store page','Import the VM compiler/runtime and storage page codec. Execute a bounded expression, serialize its result, reject bad source and detect later corruption.'),
 ('LAB-IN-08','PR-CAP-MCU-02','mcu','ADC to filter to progress watchdog','Import ADC, ring, numerical filter and watchdog. A conversion timeout must publish nothing and must never masquerade as healthy progress.')]
}
refs={
 'firmware_io':[('R-STM32F439-RM','GPIO, USART, SPI, I2C, DMA, timers and Ethernet chapters'),('R-ARMV7M-ARM','Exception entry and memory ordering')],
 'algorithms':[('R-CLRS4','Dynamic programming, amortized analysis and integer sorting'),('R-KNUTH1','Linked allocation and list processing')],
 'communications':[('R-RFC9293','Retransmission and TCP state processing'),('R-RFC826','Address resolution')],
 'foundations':[('R-C17','Integer arithmetic, objects and expressions')],
 'integration':[('R-C17','Objects and storage duration')]
}
# Resolve resource IDs from the actual registry, never manufacture citations.
resources=json.loads((ROOT/'references/resources.json').read_text(encoding='utf-8'))['resources']
def resource(term):
 hits=[r for r in resources if term.lower() in (r['id']+' '+r['title']).lower()]
 return hits[0]['id'] if hits else None
reference_terms={'firmware_io':[('RM0090','Reset/clock control, GPIO, UART, SPI, I2C, DMA and Ethernet'),('ARMv7','Exception model and memory model')],'algorithms':[('Introduction to Algorithms','Data structures, dynamic programming, amortized analysis and sorting')],'communications':[('9293','TCP retransmission and state processing'),('826','ARP packet reception')],'foundations':[('N1570','6.2.4, 6.5 and 7.20')],'integration':[('N1570','6.2.4 and 6.5')]}
for family,rows in specs.items():
 path=ROOT/'labs'/family/'family.json'
 data=json.loads(path.read_text(encoding='utf-8')) if path.exists() else {'schema_version':1,'id':family,'modules':[],'language':'C17','build_system':'cmake','default':'Compiling learner scaffolds; semantic gates reject','assessor_option':'CMASTERY_ASSESSOR_ROOT','assessor_directory':'assessor/'+family,'acceptance_scope':'Finite mechanism evidence only; no automatic project, objective or physical acceptance.','projects':[],'tests':[],'negative_controls':[]}
 for key in ('tests','negative_controls','modules'):data.setdefault(key,[])
 units=[]
 for uid,pid,case,title,contract in rows:
  meta=project(pid);test=f'{family}.v4_{case}' if family in ('algorithms','communications','foundations') else f'{family}.{case}'
  inputs=[]
  if case=='p1':inputs=['bytes.cursor','parser.integer','containers.ring']
  if case=='p2':inputs=['containers.ring','io.stream']
  if case=='p3':inputs=['bytes.cursor','parser.integer','containers.ring','integration.records']
  if case=='p4':inputs=['firmware_io.dma','containers.ring']
  if case=='p5':inputs=['containers.ring','io.async']
  if case=='p6':inputs=['numeric.filter','measurement.capture','bytes.checked']
  if case=='linux':inputs=['runtime.vm','storage.pages']
  if case=='mcu':inputs=['numeric.filter','containers.ring','firmware_io.adc','firmware_io.watchdog']
  selected_refs=reference_terms[family]
  if family=='firmware_io':
   selected_refs={
    'startup':[('ARMv7','System-level architecture: reset and exception entry'),('RM0090','Memory and bus architecture; embedded SRAM and Flash memory map')],
    'vectors':[('ARMv7','Exception handling: vector table, initial stack pointer and Thumb execution state')],
    'gpio':[('RM0090','General-purpose I/Os: GPIOx_BSRR bit set/reset register')],
    'uart':[('RM0090','Universal synchronous asynchronous receiver transmitter: USART_SR TXE and USART_DR')],
    'spi':[('RM0090','Serial peripheral interface: SPI_SR TXE, RXNE and BSY; software slave management')],
    'i2c':[('RM0090','Inter-integrated circuit interface: START/STOP conditions and error conditions'),('UM10204','Bus clear; nine clock pulses and STOP condition')],
    'timer':[('RM0090','General-purpose timers: counter auto-reload, capture/compare and PWM mode')],
    'adc':[('RM0090','Analog-to-digital converter: regular conversions, ADC_SR EOC and ADC_DR')],
    'dma':[('RM0090','DMA controller: stream enable/disable and DMA_SxCR EN'),('RM0399','DMA and memory architecture: domain reachability and cache maintenance obligations')],
    'ethernet':[('RM0090','Ethernet MAC: MII management interface and normal DMA transmit descriptors')],
    'rs485':[('RM0090','USART: transmission complete TC versus transmit-data-register empty TXE')],
    'watchdog':[('RM0090','Independent watchdog: IWDG_KR reload key and watchdog reset')]
   }[case]
  refs_used=[{'resource':resource(term),'locator':loc} for term,loc in selected_refs if resource(term)]
  specific={'filter':[('Scientist and Engineer','Chapters 6, 14 and 19: convolution and recursive filters')],'p6':[('Scientist and Engineer','Chapters 6, 14 and 19')],'canfd':[('RM0399','FDCAN controller: configuration, message RAM and internal loopback')],'diagnosis':[('Debugging with GDB','Stopping, examining state and optimized code')],'measurement':[('Roofline','Operational intensity and explicit measurement assumptions')]}.get(case,[])
  refs_used += [{'resource':resource(term),'locator':loc} for term,loc in specific if resource(term)]
  remaining='Independent changed-condition implementation, design defense and complete parent-project obligations.'
  if family=='firmware_io' or case in ('canfd','p4','mcu'):remaining+=' Hosted register models and cross-compilation do not establish actual MMIO addresses, cache/DMA ordering, electrical timing, target reset behavior or physical F439/H745 operation. Supply the parent project\'s required board captures separately.'
  unit={'id':uid,'project':pid,'title':title,'tests':[test],'hours':2,'objectives':[t+'.a' for t in meta['topics']],'new_executable':True,'scope':contract,'remaining_evidence':remaining,'steps':['Write the state, ownership and failure contract; predict normal, boundary and fault cases.','Implement the selected mechanism against the imported interfaces without reading assessor code.','Run the semantic gate and a deliberate negative control; minimize a disagreement.','Change one capacity, timing, ownership or failure assumption and record the design consequence.'],'checks':[test,'The supplied learner scaffold builds and does not pass this semantic gate.','The explicit assessor passes and the corresponding mutation is rejected.'],'artifact_inputs':inputs,'artifact_outputs':[{'id':{'array':'containers.array','filter':'numeric.filter','measurement':'measurement.capture','p1':'integration.records','p2':'io.stream','linux':'system.vm_store','mcu':'system.gateway'}.get(case,f'{family}.{case}'),'contract_version':1}],'references':refs_used}
  unit['reused_functions']={'reassembly':['edu_rx_accept'],'tcp_fault':['net_tick'],'p1':['f_take','f_parse_i32','algo_ring_put'],'p2':['algo_ring_take','f_write_all'],'p3':['in_records'],'p4':['fw_dma','algo_ring_put'],'p5':['f_cancel','f_complete','algo_ring_take'],'p6':['f_checked_size','v4_filter','v4_measure'],'linux':['cvm_compile','cvm_init','cvm_run','store_page_encode'],'mcu':['fw_adc','algo_ring_put','algo_ring_take','v4_filter','fw_watchdog']}.get(case,[])
  units.append(unit)
  if test not in data['tests']:data['tests'].append(test)
  neg=f'{family}.negative.{case}'
  if neg not in data['negative_controls']:data['negative_controls'].append(neg)
  entry=next((p for p in data['projects'] if p['id']==pid),None)
  if not entry:data['projects'].append({'id':pid,'tests':[test],'capabilities':['hosted-c17'],'scope':contract})
  elif test not in entry['tests']:entry['tests'].append(test)
  module=pid.split('-')[1]
  if module.startswith('T') and module not in data['modules']:data['modules'].append(module)
  card=f'# {uid}: {title}\n\nParent: `{pid}`. Budget: 2 hours, charged inside the parent project.\n\n## Contract\n\n{contract}\n\n## Steps\n\n'+ '\n'.join(f'{i}. {s}' for i,s in enumerate(unit['steps'],1))+f'\n\n## Acceptance boundary\n\nRun `{test}` with the selected learner implementation. {remaining}\n\nA passing visible check is practice evidence. Record assistance; preserve a sealed independent diagnosis or reconstruction before inspecting assessor material.\n\n## References\n\n'+'\n'.join(f"- `{r['resource']}`: {r['locator']}." for r in refs_used)
  write(f'labs/{family}/units/{uid}.md',card)
 data['units']=units
 if family=='integration':
  for n in range(1,7):
   target=next(p for p in data['projects'] if p['id']==f'PR-P{n}-02')
   pid=f'PR-P{n}-01'
   if not any(p['id']==pid for p in data['projects']):data['projects'].append(dict(target,id=pid))
  for cap in ('LINUX','MCU'):
   target=next(p for p in data['projects'] if p['id']==f'PR-CAP-{cap}-02')
   if not any(p['id']==f'PR-CAP-{cap}-03' for p in data['projects']):data['projects'].append(dict(target,id=f'PR-CAP-{cap}-03'))
  data['dependencies']=[{'family':f,'required_for':([f'integration.{c}' for c in ['p1','p2','p3','p4','p5','p6','linux','mcu']] if f in ('foundations','algorithms') else ['integration.p4','integration.mcu'] if f=='firmware_io' else ['integration.linux']),'implementation_variable':f'CMASTERY_{f.upper()}_ROOT','include_variable':f'CMASTERY_{f.upper()}_INCLUDE','fallback':'none'} for f in ['foundations','algorithms','firmware_io','runtime','storage']]
 if family=='firmware_io':data['tests'].append('firmware_io.cortex_compile') if 'firmware_io.cortex_compile' not in data['tests'] else None
 write(f'labs/{family}/family.json',json.dumps(data,indent=2))

# Derive buildable learner stubs from public signatures, preserving guards and
# preserving any implementation once generated (idempotent publication helper).
stub=ROOT/'labs/firmware_io/src/firmware_io.c'
if not stub.exists():
 header=(ROOT/'labs/firmware_io/include/firmware_io.h').read_text()
 # Parameter names are intentionally provided separately to keep stubs readable.
 body='''#include "firmware_io.h"
int fw_startup(uint8_t*a,const uint8_t*b,size_t n,uint8_t*c,size_t m){(void)a;(void)b;(void)n;(void)c;(void)m;return -1;}
int fw_vector(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return -1;}
int fw_gpio(struct fw_io a,unsigned b,int c){(void)a;(void)b;(void)c;return -1;}
int fw_uart(struct fw_io a,uint8_t b,unsigned c){(void)a;(void)b;(void)c;return -1;}
int fw_spi(struct fw_io a,uint8_t b,uint8_t*c,unsigned d){(void)a;(void)b;(void)c;(void)d;return -1;}
int fw_i2c_recover(struct fw_io a,unsigned b){(void)a;(void)b;return -1;}
int fw_timer(struct fw_io a,uint32_t b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return -1;}
int fw_adc(struct fw_io a,uint16_t*b,unsigned c){(void)a;(void)b;(void)c;return -1;}
int fw_dma(struct fw_dma*a,struct fw_io b,unsigned c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return -1;}
int fw_ethernet(struct fw_descriptor*a,struct fw_io b,uint32_t c,uint32_t d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return -1;}
int fw_rs485(struct fw_io a,uint8_t b,unsigned c){(void)a;(void)b;(void)c;return -1;}
int fw_watchdog(struct fw_watchdog*a,struct fw_io b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return -1;}
'''
 write('labs/firmware_io/src/firmware_io.c',body)
for family,stem,var,extra in [('algorithms','v4_algo','ENGINE_DIR',''),('communications','v4_com','COM_SOURCE','target_link_libraries(v4_communications_tests PRIVATE com_core)'),('foundations','v4_foundation','V4_FOUNDATION_SOURCE','')]:
 pre=''
 if family=='foundations':pre='set(V4_FOUNDATION_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src")\nif(CMASTERY_ASSESSOR_ROOT)\n set(V4_FOUNDATION_SOURCE "${CMASTERY_ASSESSOR_ROOT}")\nendif()\n'
 cases=' '.join(row[2] for row in specs[family])
 block=f'''\n# v4 additive implementation units
{pre}add_executable(v4_{family}_tests "${{{var}}}/{stem}.c" tests/{stem}.c)
target_include_directories(v4_{family}_tests PRIVATE include)
target_compile_options(v4_{family}_tests PRIVATE -Wall -Wextra -Wpedantic)
{extra}
foreach(case {cases})
 add_test(NAME {family}.v4_${{case}} COMMAND v4_{family}_tests ${{case}})
 set_tests_properties({family}.v4_${{case}} PROPERTIES TIMEOUT 10)
endforeach()
if(CMASTERY_ASSESSOR_ROOT)
 find_package(Python3 COMPONENTS Interpreter REQUIRED)
 include(tests/v4_mutations.cmake)
endif()
'''
 path=ROOT/f'labs/{family}/CMakeLists.txt';old=path.read_text(encoding='utf-8');old=old.split('# v4 additive implementation units')[0];path.write_text(old.rstrip()+'\n'+block,encoding='utf-8')
for family,stem,var in [('algorithms','v4_algo','ENGINE_DIR'),('communications','v4_com','COM_SOURCE'),('foundations','v4_foundation','V4_FOUNDATION_SOURCE'),('firmware_io','firmware_io','FW_SOURCE'),('integration','integration','IN_SOURCE')]:
 runner=(ROOT/'labs/firmware_io/tests/mutation.py').read_text(encoding='utf-8')
 write(f'labs/{family}/tests/v4_mutation.py',runner)
 test='contracts.c' if family in ('firmware_io','integration') else stem+'.c'
 includes='--include "${CMAKE_CURRENT_SOURCE_DIR}/include"'
 extras=''
 if family=='communications':extras=' '.join('--extra "${COM_SOURCE}/'+f+'.c"' for f in ['http','edu7','net','mcu'])
 pre=''
 if family=='integration':
  pre='set(MUTATION_ARGS "")\nforeach(path ${IN_INCLUDES})\n list(APPEND MUTATION_ARGS --include "${path}")\nendforeach()\nforeach(path ${IN_DEPS})\n list(APPEND MUTATION_ARGS --extra "${path}")\nendforeach()\nforeach(def ${IN_DEFINES})\n list(APPEND MUTATION_ARGS --define "${def}")\nendforeach()\n'
  includes='${MUTATION_ARGS}'
 cases='${IN_CASES}' if family=='integration' else ' '.join(r[2] for r in specs[family])
 text=f'''{pre}foreach(case {cases})
 add_test(NAME {family}.negative.${{case}} COMMAND "${{Python3_EXECUTABLE}}" "${{CMAKE_CURRENT_SOURCE_DIR}}/tests/v4_mutation.py" --cc "${{CMAKE_C_COMPILER}}" --source "${{{var}}}/{stem}.c" --test "${{CMAKE_CURRENT_SOURCE_DIR}}/tests/{test}" --mutations "${{{var}}}/v4_mutations.json" --case ${{case}} {includes} {extras})
 set_tests_properties({family}.negative.${{case}} PROPERTIES TIMEOUT 30 LABELS "negative-control;v4")
endforeach()
'''
 write(f'labs/{family}/tests/'+('mutations.cmake' if family in ('firmware_io','integration') else 'v4_mutations.cmake'),text)
print('Wrote 32 executable-unit records, cards and compatible build wiring.')
