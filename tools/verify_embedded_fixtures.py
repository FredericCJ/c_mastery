"""Recompute original embedded fixtures, without implementing the learner's C programs.

The models cover declared arithmetic and synthetic protocol inputs only. Actual
compiler, verifier, emulator and physical-board evidence is gathered separately.
"""
from pathlib import Path
from fractions import Fraction as F
from itertools import combinations
import hashlib
import json
import re
import statistics

ROOT = Path(__file__).resolve().parents[1]


def number(value):
    return int(value, 0) if isinstance(value, str) else value


def fraction(value):
    return str(F(value))


def fir(values, coefficients):
    return [sum(F(c) * F(values[n-j]) for j, c in enumerate(coefficients) if n >= j)
            for n in range(len(values))]


def queue(data):
    assert data["slots"] >= 2
    pending, popped, dropped = [], [], 0
    for event in data["operations"]:
        if event[0] == "push":
            if len(pending) == data["slots"] - 1:
                dropped += 1
            else:
                pending.append(event[1])
        elif event[0] == "pop":
            if pending:
                popped.append(pending.pop(0))
        else:
            raise ValueError("Unknown queue event")
    return dict(popped=popped, remaining=pending, dropped=dropped)


def publication_bad_reads(producer, consumer):
    total = bad = 0
    length = len(producer) + len(consumer)
    for positions in combinations(range(length), len(producer)):
        p, c, events = iter(producer), iter(consumer), []
        for index in range(length):
            events.append(next(p) if index in positions else next(c))
        total += 1
        written = published = observed = False
        for event in events:
            if event == "write":
                written = True
            elif event == "publish":
                published = True
            elif event == "observe":
                observed = published
            elif event == "read" and observed and not written:
                bad += 1
    return total, bad


def recompute(d):
    k = d["kind"]
    if k == "sum":
        return dict(exact_sum=sum(d["values"]), absolute_sum=sum(abs(x) for x in d["values"]))
    if k == "qmul":
        scale = 1 << d["fraction_bits"]
        result = []
        for a, b in d["pairs"]:
            product = a*b
            rounded = (abs(product) + scale//2)//scale
            signed = -rounded if product < 0 else rounded
            result.append(max(d["min"], min(d["max"], signed)))
        return result
    if k == "matrix":
        a, b = d["a"], d["b"]
        assert all(len(row) == len(b) for row in a)
        assert len({len(row) for row in b}) == 1
        return [[sum(x*y for x, y in zip(row, col)) for col in zip(*b)] for row in a]
    if k == "interpolate":
        knots = d["knots"]
        assert all(a[0] < b[0] for a, b in zip(knots, knots[1:]))
        result = []
        for x in d["samples"]:
            if x <= knots[0][0]:
                y = F(knots[0][1])
            elif x >= knots[-1][0]:
                y = F(knots[-1][1])
            else:
                lo, hi = next((a, b) for a, b in zip(knots, knots[1:]) if a[0] <= x <= b[0])
                y = F(lo[1]) + F(x-lo[0], hi[0]-lo[0])*(hi[1]-lo[1])
            assert y.denominator == 1
            result.append(y.numerator)
        return result
    if k == "filters":
        a, prev, response = F(d["iir_a"]), F(0), []
        for x in d["step"]:
            prev = a*prev + (1-a)*x
            response.append(str(prev))
        return dict(fir=list(map(str, fir(d["impulse"], d["fir_coefficients"]))), iir=response)
    if k == "linear2":
        result = []
        for case in d["cases"]:
            a, b = case["a"], case["b"]
            det = a[0][0]*a[1][1] - a[0][1]*a[1][0]
            if det == 0:
                result.append(dict(status="singular"))
            else:
                x = F(b[0]*a[1][1]-a[0][1]*b[1], det)
                y = F(a[0][0]*b[1]-b[0]*a[1][0], det)
                residual = [str(a[i][0]*x+a[i][1]*y-b[i]) for i in range(2)]
                result.append(dict(status="ok", solution=[str(x), str(y)], residual=residual))
        return result
    if k == "euler":
        return [dict(value=str((1-F(c["h"]))**c["steps"]), time=str(F(c["h"])*c["steps"]),
                     stable=abs(1-F(c["h"])) < 1) for c in d["cases"]]
    if k == "target":
        assert d["boot0"] == 0, "BOOT1 selection is outside this fixture's declared scope"
        return dict(boot="FLASH", dma_banks=[b["name"] for b in d["banks"] if b["dma"]])
    if k == "vectors":
        sp, reset = number(d["sp"]), number(d["reset"])
        return dict(stack_aligned=sp%8 == 0,
                    stack_in_bounds=number(d["stack_low"]) <= sp <= number(d["stack_high"]),
                    thumb=bool(reset & 1),
                    reset_in_flash=number(d["flash_low"]) <= (reset & ~1) < number(d["flash_high"]))
    if k == "memory_init":
        ram, load = list(d["ram"]), d["load"]
        for c in d["copies"]:
            assert c["src"] >= 0 and c["dst"] >= 0 and c["count"] >= 0
            assert c["src"]+c["count"] <= len(load) and c["dst"]+c["count"] <= len(ram)
            ram[c["dst"]:c["dst"]+c["count"]] = load[c["src"]:c["src"]+c["count"]]
        for z in d["zeros"]:
            assert 0 <= z["dst"] <= z["dst"]+z["count"] <= len(ram)
            ram[z["dst"]:z["dst"]+z["count"]] = [0]*z["count"]
        return ram
    if k == "startup":
        ready = d["ready_at"] is not None and 0 <= d["ready_at"] < d["poll_limit"]
        return dict(status="ready" if ready else "timeout",
                    polls=d["ready_at"]+1 if ready else d["poll_limit"],
                    hooks=list(d["hooks"]) if ready else [])
    if k == "retained":
        return [r["magic"] == d["required_magic"] and r["version"] == d["required_version"]
                and r["length"] == len(r["payload"]) and r["checksum"] == sum(r["payload"])%65536
                for r in d["records"]]
    if k == "mmio":
        return dict(bsrr=(d["reset"] << 16) | d["set"],
                    odr=((d["odr"] & ~d["reset"]) | d["set"]) & 65535,
                    status=d["status"] & ~d["ack"])
    if k == "uart":
        assert d["clock"] > 0 and d["baud"] > 0
        return dict(brr=(2*d["clock"]+d["baud"])//(2*d["baud"]), frame_bits=1+8+1,
                    received=d["received"][:d["capacity"]],
                    dropped=max(0, len(d["received"])-d["capacity"]))
    if k == "timer":
        return dict(frequency=str(F(d["clock"], (d["psc"]+1)*(d["arr"]+1))),
                    duty=str(F(d["ccr"], d["arr"]+1)),
                    tick_seconds=str(F(d["psc"]+1, d["clock"])))
    if k == "serial_bus":
        assert len(d["spi_tx"]) == len(d["spi_rx"]) and 0 <= d["address"] < 128
        # Derive a one-byte register-read transaction from its request operations.
        phases = ["START", "ADDR_W", "REGISTER", "RESTART", "ADDR_R"]
        phases += ["BYTE", "NACK", "STOP"]  # sole final byte, hence NACK
        return dict(spi_received=list(d["spi_rx"]), i2c_address_write=d["address"]*2,
                    i2c_address_read=d["address"]*2+1, bus_phases=phases)
    if k == "adc":
        assert 0 <= d["code"] < 1 << d["bits"]
        return dict(mv_numerator=d["code"]*d["vref_mv"], mv_denominator=(1 << d["bits"])-1,
                    total_cycles=d["sample_cycles"]+d["conversion_cycles"],
                    trigger_code={"disabled":0, "rising":1, "falling":2, "both":3}[d["trigger"]])
    if k == "dma_validate":
        return [c["bank"] == "SRAM" and number(c["address"])%d["width"] == 0
                and d["width"]*d["count"] <= d["capacity"] for c in d["candidates"]]
    if k == "datagram":
        body = bytes(d["payload"])
        encoded = d["sequence"].to_bytes(2, "big")+len(body).to_bytes(2, "big")+body
        return dict(hex=encoded.hex(), length=len(encoded))
    if k == "frame":
        token = number(d["exc_return"])
        assert token in {0xfffffff1,0xfffffff9,0xfffffffd,0xffffffe1,0xffffffe9,0xffffffed}
        return dict(pc=d["words"][6], lr=d["words"][5],
                    stack="PSP" if token & 4 else "MSP", mode="thread" if token & 8 else "handler",
                    basic_frame=bool(token & 16),
                    priority_order=[p["name"] for p in sorted(d["priorities"], key=lambda x:x["value"])])
    if k == "mask_restore":
        result = []
        for old in d["prior"]:
            assert old in (0,1)
            saved, current = old, 1
            assert current == 1
            current = saved
            result.append(current)
        return result
    if k == "queue":
        return queue(d)
    if k == "dma_history":
        owner, terminal, count, history = "cpu", None, 0, []
        for event in d["events"]:
            if event == "submit":
                assert owner == "cpu"
                owner = "device"
            elif event == "cancel_request":
                assert owner == "device"
            elif event in ("complete", "stop_confirm"):
                if terminal is None:
                    assert owner == "device"
                    terminal, owner, count = ("complete" if event == "complete" else "cancel"), "service", count+1
            elif event == "reclaim":
                assert owner == "service" and terminal is not None
                owner = "cpu"
            else:
                raise ValueError(event)
            history.append(owner)
        return dict(owners=history, terminal=terminal, terminal_count=count)
    if k == "snapshot":
        assert len(d["hi_reads"]) == 2*len(d["lo_reads"])
        for i, low in enumerate(d["lo_reads"]):
            before, after = d["hi_reads"][2*i:2*i+2]
            if before == after:
                return dict(attempts=i+1, value=(before << 32) | low)
        raise ValueError("No coherent snapshot within supplied attempts")
    if k == "superloop":
        return dict(original_loop=sum(d["original_costs"]), longest_activity=max(d["original_costs"]),
                    sliced_loop=sum(d["sliced_costs"]))
    if k == "resources":
        q = d["queue_slots"]*d["slot_bytes"]
        return dict(queue_bytes=q, total=sum(d["stacks"])+q+d["persistent"]+d["interrupt"])
    if k == "timing":
        modulus = 1 << d["bits"]
        return dict(elapsed=(d["now"]-d["then"])%modulus, latency=d["start"]-d["release"],
                    execution=d["finish"]-d["start"], response=d["finish"]-d["release"], half_range=modulus//2)
    if k == "response":
        answers = []
        for i, task in enumerate(d["tasks"]):
            r = task["cost"]+task["blocking"]
            for _ in range(100):
                nxt = task["cost"]+task["blocking"]+sum(
                    ((r+t["period"]-1)//t["period"])*t["cost"] for t in d["tasks"][:i])
                if nxt == r or nxt > task["deadline"]:
                    r = nxt
                    break
                r = nxt
            else:
                raise ValueError("Response recurrence did not terminate")
            answers.append(r)
        return dict(responses=answers,
                    utilization=str(sum(F(t["cost"],t["period"]) for t in d["tasks"])),
                    schedulable=all(r <= t["deadline"] for r,t in zip(answers,d["tasks"])))
    if k == "watchdog":
        last, result = list(d["initial"]), []
        for window in d["windows"]:
            progressed = all(new > old for new, old in zip(window, last))
            result.append(progressed)
            if progressed:
                last = list(window)
        return result
    if k == "debug_stage":
        stages = [("connected","connection"),("programmed","flash"),("reset_ok","reset/application")]
        return [next((name for flag,name in stages if not item[flag]), "ready") for item in d["observations"]]
    if k == "fault":
        free = d["stack_bytes"]-d["observed_used"]
        return dict(precise_bus=bool(d["cfsr"] & d["masks"]["precise_bus"]),
                    address_valid=bool(d["cfsr"] & d["masks"]["address_valid"]),
                    forced=bool(d["hfsr"] & d["masks"]["forced"]),
                    observed_free=free, allowance_fits=d["reasoned_allowance"] <= free)
    if k == "diagnosis":
        return ["clock-disabled" if not x["clock"] else
                "stale-generation" if x["generation"] != x["current"] else "valid" for x in d["events"]]
    if k == "trace_image":
        assert all(b > a for a,b in zip(d["sequences"],d["sequences"][1:]))
        return dict(missing=sum(b-a-1 for a,b in zip(d["sequences"],d["sequences"][1:])),
                    vector_address=number(d["base"])+d["vector_offset"],
                    image_end=number(d["base"])+d["image_bytes"])
    if k == "update":
        versions = [s["version"] for s in d["slots"] if s["valid"] and s["committed"]
                    and s["version"] <= d["max_version"]]
        return dict(selected_version=max(versions) if versions else None,
                    recoveries=min(d["failures"], d["recovery_limit"]),
                    safe_stop=d["failures"] > d["recovery_limit"])
    if k == "metamorphic":
        base = fir(d["values"], d["coefficients"])
        return dict(base=list(map(str,base)), scaled=list(map(str,fir(
            [x*d["scale"] for x in d["values"]],d["coefficients"]))))
    if k == "packets":
        result = []
        for p in d["packets"]:
            if len(p) < 2: verdict = "length"
            elif p[0] not in (1,2): verdict = "opcode"
            elif p[1] > d["max_count"]: verdict = "count"
            elif len(p) != 2+2*p[1]: verdict = "length"
            elif p[0] == 2 and not d["authorized"]: verdict = "denied"
            else: verdict = "ok"
            result.append(verdict)
        return result
    if k == "rollback":
        acquired, released, live = [], [], []
        for fail in d["fail_at"]:
            held = []
            for resource in range(d["resources"]):
                if resource == fail:
                    break
                held.append(resource)
            acquired.append(len(held))
            undo = list(reversed(held)) if fail is not None else []
            released.append(len(undo))
            live.append(len(held)-len(undo))
        return dict(acquired=acquired, released=released, live_on_return=live)
    if k == "memory_events":
        size, alive, defined, errors = 0, False, set(), []
        for e in d["events"]:
            if e[0] == "allocate":
                size, alive, defined = e[1], True, set()
            elif e[0] == "free":
                assert alive
                alive = False
            elif not alive:
                errors.append("use-after-free")
            elif not 0 <= e[1] < size:
                errors.append("out-of-bounds")
            elif e[0] == "write":
                defined.add(e[1])
            elif e[0] == "branch_read" and e[1] not in defined:
                errors.append("uninitialized-use")
        return errors
    if k == "mcdc":
        decisions = [all(t) for t in d["tests"]]
        pairs = {}
        for condition, name in enumerate(("A","B")):
            pair = next((list(ij) for ij in combinations(range(len(decisions)),2)
                         if decisions[ij[0]] != decisions[ij[1]]
                         and d["tests"][ij[0]][condition] != d["tests"][ij[1]][condition]
                         and d["tests"][ij[0]][1-condition] == d["tests"][ij[1]][1-condition]), None)
            pairs[name] = pair
        return dict(decisions=decisions, independent_pairs=pairs)
    if k == "replay":
        digest = hashlib.sha256(d["payload"].encode()).hexdigest()
        assert d["environment"] == "host"
        return dict(manifest_valid=[h == digest for h in d["record_hashes"]],
                    supported_claims=[claim == "logic" for claim in d["claims"]])
    if k == "prefix":
        assert len(d["values"]) <= d["capacity"]
        assert all(0 <= x <= d["max_element"] for x in d["values"])
        return dict(sum=sum(d["values"]), bound=d["capacity"]*d["max_element"],
                    variant=list(range(len(d["values"]),-1,-1)))
    if k == "machine_add":
        total, modulus = d["a"]+d["b"], 1 << d["bits"]
        return dict(mathematical=total, converted=total%modulus, no_narrowing_loss=0 <= total < modulus)
    if k == "bounded_env":
        return dict(guard_evaluations=d["iterations"]+1,
                    outcomes=["error" if x < 0 else "no-progress" if x == 0 else
                              "invalid" if x > d["capacity"] else
                              "full" if x == d["capacity"] else "partial" for x in d["returns"]])
    if k == "proof_ledger":
        return dict(proved=d["statuses"].count("proved"), unproved=d["statuses"].count("unproved"),
                    assumed=d["statuses"].count("assumed"), blanket_success=all(x == "proved" for x in d["statuses"]))
    if k == "interval":
        coarse = [x+d["offset"] for x in d["coarse"]]
        refined = [x+d["offset"] for x in d["refined"]]
        return dict(coarse=coarse, coarse_zero_possible=coarse[0] <= 0 <= coarse[1],
                    refined=refined, refined_zero_possible=refined[0] <= 0 <= refined[1])
    if k == "publication":
        count, safe_bad = publication_bad_reads(d["producer_safe"],d["consumer"])
        bad_count, unsafe_bad = publication_bad_reads(d["producer_bad"],d["consumer"])
        assert count == bad_count
        return dict(interleavings=count, safe_order_bad_reads=safe_bad, early_publish_bad_reads=unsafe_bad)
    if k == "cost":
        ops = d["elements"]*d["operations_per_element"]
        byte_count = d["elements"]*d["bytes_per_element"]
        intensity = F(ops,byte_count)
        return dict(operations=ops, bytes=byte_count, intensity=str(intensity),
                    ceiling=str(min(F(d["compute_peak"]),d["bandwidth"]*intensity)))
    if k == "layout":
        return dict(contiguous_lines=len({a//d["line_bytes"] for a in d["contiguous"]}),
                    strided_lines=len({a//d["line_bytes"] for a in d["strided"]}),
                    clamped=[max(d["low"],min(d["high"],v)) for v in d["values"]])
    if k == "vector":
        assert len(d["a"]) == len(d["b"])
        return [(a+b)%(1 << d["bits"]) for a,b in zip(d["a"],d["b"])]
    if k == "simd":
        assert len(d["a"]) == len(d["b"])
        return dict(values=[(a+b)%(1 << 32) for a,b in zip(d["a"],d["b"])],
                    groups=len(d["a"])//d["lanes"], tail=len(d["a"])%d["lanes"],
                    paths=["sse2" if support > 0 and enabled else "scalar" for support,enabled in d["dispatch_cases"]])
    if k == "benchmark":
        assert d["time_running"] > 0
        return dict(median=str(F(statistics.median(d["samples"]))),
                    spread=max(d["samples"])-min(d["samples"]),
                    scaled_count=str(F(d["raw_count"]*d["time_enabled"],d["time_running"])))
    if k == "tradeoff":
        return [next((name for name in ("flash","stack","cycles") if v[name] > d["limits"][name]),"pass")
                .replace("cycles","latency") for v in d["variants"]]
    if k == "process_policy":
        return dict(inherited=[x["fd"] for x in d["descriptors"] if not x["cloexec"]],
                    allowed_results=[x in d["allowed"] for x in d["requests"]])
    if k == "quota":
        used, admitted = 0, []
        for cost in d["costs"]:
            accept = 0 <= cost <= d["max_request"] and used+cost <= d["total_limit"]
            admitted.append(accept)
            if accept:
                used += cost
        return dict(admitted=admitted, used=used, recoveries=min(d["failures"],d["recovery_limit"]),
                    safe_stop=d["failures"] > d["recovery_limit"])
    if k == "hardening":
        headers = d["headers"]
        stacks = [h for h in headers if h["type"] == "GNU_STACK"]
        return dict(pie_marked=d["elf_type"] == "DYN" and d["pie_flag"],
                    relro_segment=any(h["type"] == "GNU_RELRO" for h in headers),
                    eager_binding=d["bind_now"],
                    nx_stack_marked=bool(stacks) and all("E" not in h["flags"] for h in stacks),
                    wx_load_segment=any(h["type"] == "LOAD" and "W" in h["flags"] and "E" in h["flags"] for h in headers),
                    guard_site_dependency="__stack_chk_fail" in d["symbols"],
                    checked_call_present=any(s.startswith("__") and s.endswith("_chk") for s in d["symbols"]),
                    aslr_policy=d["aslr_policy"], base_changed=len(set(d["bases"])) > 1)
    if k == "secret_compare":
        result, early, fixed = [], [], []
        for candidate in d["candidates"]:
            assert len(candidate) == len(d["token"])
            result.append(candidate == d["token"])
            work = next((i+1 for i,(a,b) in enumerate(zip(candidate,d["token"])) if a != b),len(candidate))
            early.append(work); fixed.append(len(candidate))
        return dict(equal=result, early_work=early, fixed_work=fixed)
    if k == "integrity":
        return dict(sha256=hashlib.sha256(d["payload"].encode()).hexdigest(),
                    compatible=[v >= d["minimum_version"] for v in d["versions"]],
                    authenticity_supported=d["trusted_signer"])
    raise ValueError("Unsupported fixture kind: "+k)


def main():
    paths = sorted((ROOT/"fixtures/embedded").glob("*.json"))
    failures, cases = [], 0
    owned = {"T17", *(f"T{i}" for i in range(27, 36))}
    expected_paths = set()
    for project in sorted((ROOT/"curriculum/projects").glob("PR-T*.md")):
        match = re.match(r"\A```json\s*\n(.*?)\n```", project.read_text(encoding="utf-8"), re.S)
        if not match:
            failures.append(dict(file=project.name, error="Missing project contract"))
            continue
        meta = json.loads(match[1])
        if meta["module"] in owned:
            for relative in meta["fixtures"]:
                candidate = (ROOT/relative).resolve()
                if not candidate.is_relative_to((ROOT/"fixtures/embedded").resolve()):
                    failures.append(dict(file=project.name, error="Fixture escapes owned directory"))
                else:
                    expected_paths.add(candidate)
    actual_paths = {p.resolve() for p in paths}
    for path in sorted(expected_paths-actual_paths):
        failures.append(dict(file=path.name, error="Missing referenced fixture"))
    for path in sorted(actual_paths-expected_paths):
        failures.append(dict(file=path.name, error="Unreferenced fixture"))
    if not paths:
        failures.append(dict(file="fixtures/embedded", error="No fixtures found"))
    for path in paths:
        try:
            fixture = json.loads(path.read_text(encoding="utf-8"))
            assert {"id","description","input","expected","verification_method"} <= fixture.keys()
            assert fixture["id"] == path.stem+"-fixture", "Fixture/project identity mismatch"
            actual = recompute(fixture["input"])
            cases += 1
            if json.dumps(actual, sort_keys=True) != json.dumps(fixture["expected"], sort_keys=True):
                failures.append(dict(file=path.name, expected=fixture["expected"], actual=actual))
        except Exception as error:
            failures.append(dict(file=path.name, error=f"{type(error).__name__}: {error}"))
    print(json.dumps(dict(fixtures=len(paths), checks=cases, failures=failures),indent=2))
    raise SystemExit(bool(failures))


if __name__ == "__main__":
    main()
