"""Independent finite visibility counterexamples; no ISO execution of a C race."""
from itertools import permutations
import json

# CPU STORE_DESCRIPTOR, STORE_START; device fetch occurs after START visibility.
# A compiler barrier constrains instruction generation, not the bus schedule.
instructions=list(permutations(('descriptor','start')))
assert ('start','descriptor') in instructions
compiler_order=[x for x in instructions if x.index('descriptor')<x.index('start')]
assert compiler_order==[('descriptor','start')]
bus=list(permutations(('descriptor_visible','start_visible','device_fetch')))
bus=[x for x in bus if x.index('start_visible')<x.index('device_fetch')]
bad=[x for x in bus if x.index('device_fetch')<x.index('descriptor_visible')]
assert bad
ordered=[x for x in bus if x.index('descriptor_visible')<x.index('start_visible')]
assert ordered and not any(x in bad for x in ordered)
# Separate cache view: memory ordering cannot publish dirty data still in cache.
dirty_cache,ram=42,7
assert ram!=dirty_cache
ram=dirty_cache # whole-line clean + completion
assert ram==42
cached_destination,device_ram=9,42
assert cached_destination!=device_ram
cached_destination=device_ram # post-quiescence invalidate/refill
assert cached_destination==42
# IRQ mask: absent CPU handler does not remove the independent device action.
events=list(permutations(('cpu_reuse','device_delayed_write')))
assert ('cpu_reuse','device_delayed_write') in events
# DMB orders accesses, DSB completes prior CPU accesses. Neither is a device
# shutdown command. A queued device write remains possible after either.
pending_write=1
assert pending_write
rows=[dict(id='compiler-only',witness=bad[0],violated='descriptor visible before device fetch'),
dict(id='DMB-with-dirty-line',observed=7,expected=42,violated='payload visibility'),
dict(id='DSB-with-live-device',pending_write=pending_write,violated='exclusive ownership'),
dict(id='IRQ-mask-with-DMA',witness=events[0],violated='exclusive ownership'),
dict(id='post-publication-no-invalidate',observed=9,expected=42,violated='CPU result visibility')]
print(json.dumps(dict(passed=True,descriptor_schedules=len(bus),ordered_schedules=len(ordered),counterexamples=rows,
 limits=['Abstract visibility schedules under explicit independent device/cache actors, not a simulator of all Arm memory-system rules.','Generated instructions and actual selected board port are checked separately.','No undefined C output or physical timing is a golden result.']),indent=2))
