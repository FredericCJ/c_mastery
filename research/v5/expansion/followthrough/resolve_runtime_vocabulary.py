"""Bounded corrections from actually read FreeRTOS11.2.0 source bodies."""
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
FOLDER=Path(__file__).parent
RESOURCE='R-FOLLOWTHROUGH-FREERTOS-11-2-0'
DEFINITIONS={
 'V-T30-period':'The interval between successive nominal releases of a periodic task. It is distinct from execution time and deadline. Under a simple periodic model, utilization is execution demand divided by period; period alone cannot determine demand. Absolute-phase delay accounting avoids accumulating execution time into each nominal interval, but does not guarantee dispatch or completion at that instant.',
 'V-T30-cooperative-scheduler':'A scheduler in which ordinary task switching depends on yield or blocking points. A single-core task cannot assume this removes all concurrent interference: interrupts, callbacks, additional cores and the execution-environment memory model still matter. A task that does not yield or block can delay another ready task indefinitely.',
 'V-T30-preemptive-scheduler':'A scheduler that can replace a running task without that task voluntarily yielding, subject to interrupt masking, scheduler locking, priorities and port rules. Preemption creates possible interleavings; required synchronization depends on which state is actually shared and accessed, not merely on the existence of preemption.',
 'V-T30-stack-depth-analysis':'Deriving a stack-use bound from the reachable call graph, frame requirements, recursion limits, context saves and admissible interrupt nesting under a stated compiler/ABI model. A fill-pattern high-water observation measures bytes changed in the tested executions and cannot replace that argument. Dynamic or indirect calls and library/port frames must be accounted for explicitly.',
 'V-T45-stack-exhaustion':'Exceeding the stack storage provided for an execution context. The outcome depends on memory mapping, guard configuration, architecture and fault handling: it may fault or corrupt other storage. Fill-pattern and saved-pointer checks can detect some cases after use but do not guarantee prevention or detection. The FreeRTOS11.2.0 application callback is vApplicationStackOverflowHook; it is not a hook-registration setter.'}
LOCATORS={
 'V-T30-cooperative-scheduler':'tasks.c configUSE_PREEMPTION macros, lines67–109; task.h taskYIELD',
 'V-T30-preemptive-scheduler':'tasks.c configUSE_PREEMPTION macros, lines67–109; task.h taskYIELD',
 'V-T30-stack-depth-analysis':'tasks.c prvTaskCheckFreeStackSpace and uxTaskGetStackHighWaterMark, lines6364–6455; task.h high-water API contract',
 'V-T45-stack-exhaustion':'include/stack_macros.h overflow checks; task.h vApplicationStackOverflowHook declaration'}
changes=[]
for name in ('T30.json','T45.json'):
    path=ROOT/'curriculum/vocabulary'/name
    document=json.loads(path.read_text(encoding='utf-8'))
    for entry in document['entries']:
        ident=entry['id']
        if ident in DEFINITIONS: entry['definition']=DEFINITIONS[ident]
        if ident in LOCATORS:
            for authority in entry['authority']:
                if authority.get('inspection')=='pending' and 'FREERTOS' in authority['resource_id']:
                    changes.append({'entry':ident,'file':path.relative_to(ROOT).as_posix(),
                                    'prior_authority':dict(authority),'replacement_locator':LOCATORS[ident]})
                    authority.update(resource_id=RESOURCE,locator=LOCATORS[ident],inspection='registry')
    path.write_text(json.dumps(document,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
captures={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in (
    'research/private/memory_lab/freertos_stack.c','research/private/memory_lab/freertos_tasks.c',
    'research/private/memory_lab/freertos_task_header.c')}
urls=['https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/V11.2.0/'+p for p in ('tasks.c','include/task.h','include/stack_macros.h')]
record={'id':RESOURCE,'title':'FreeRTOS Kernel11.2.0 scheduler and stack observation source',
 'authors':['FreeRTOS contributors'],'edition':'V11.2.0','year':None,'kind':'source','url':urls[0],
 'access':'Version-tagged public primary source and retained private captures',
 'domains':['T30','T45'],'role':'comparison','reason':'Correct invented API names and separate cooperative execution, observed stack fill and proven bounds.',
 'prerequisites':['Task states','C pointers and object bounds','Interrupt/ABI stack model'],
 'locators':list(dict.fromkeys(LOCATORS.values()))+['task.h xTaskDelayUntil documentation and vTaskDelayUntil wrapper; tasks.c xTaskDelayUntil lines2366–2451'],
 'assignable':True,'inspection':{'date':'2026-10-17','method':'Read listed source bodies and header contracts; inspected tagged primary URLs',
 'scope':'Named scheduler macros, absolute-phase delay arithmetic, fill scan, stack-check macros and hook declaration only.',
 'limits':'No execution of FreeRTOS or board validation. Delay source does not establish general response-time/deadline theory. FreeRTOS is a comparison source, never a dependency of strict original exercises.',
 'evidence_urls':urls,'local_document_id':None,'local_sha256':captures['research/private/memory_lab/freertos_tasks.c'],
 'capture_sha256':captures}}
(FOLDER/'runtime_vocabulary_resources.json').write_text(json.dumps({'resources':[record]},indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
receipt=FOLDER/'runtime_vocabulary_corrections.json'
if changes:
    old=json.loads(receipt.read_text(encoding='utf-8')).get('changed_entries',[]) if receipt.exists() else []
    receipt.write_text(json.dumps({'schema_version':1,'changed_entries':old+changes,
        'limits':'The four periodic scheduling theoretical locators remain pending: delay implementation alone does not justify every definition.'},indent=2)+'\n',encoding='utf-8')
print({'updated_authorities':len(changes),'resource':RESOURCE})
