"""Authoring migration from the archived first edition; not a normal build step."""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import re
import zipfile
from common import ROOT, dump, parse, write

BASELINE = "6750f52"
ARCHIVE = ROOT / "dist/c_mastery_v1_2026_10_10.zip"
EARLY = {
    "PR-T44-01": ["PR-T03-01", "PR-T07-01"],
    "PR-T44-02": ["PR-T44-01"],
    "PR-T44-03": ["PR-T44-02"],
    "PR-T45-01": ["PR-T03-01", "PR-T07-01"],
    "PR-T45-02": ["PR-T45-01", "PR-T03-02"],
    "PR-T45-03": ["PR-T45-02", "PR-T04-01"],
    "PR-T43-01": ["PR-T15-01", "PR-T07-01"],
    "PR-T43-02": ["PR-T14-02", "PR-T43-01"],
}

THREADS = {
 "T01": ("A bounded sum with a repair history", "Predict each intermediate sum, reject an overflowing prefix, compile a minimal program and explain one failure before changing it.", "An input whose final mathematical sum fits while an intermediate prefix does not."),
 "T02": ("A public header with an actual consumer", "Make declarations agree across two translation units; examine a conflicting type, a missing definition and a valid private definition separately.", "Build the consumer with a changed include order and an independently compiled implementation."),
 "T03": ("One owner, several borrowed views", "Draw each live object and admitted extent before taking an address. Track views through mutation, resize, return and release; distinguish address reuse from lifetime continuation.", "A successful resize returning the same numerical address must still invalidate old aliases under its contract."),
 "T04": ("Checked machine arithmetic", "Derive representable preconditions before addition, multiplication, conversion or shifting. Compare a mathematical reference with the actual C operation and its promoted operand types.", "A size product that wraps to a small apparently valid allocation, followed by a narrowing conversion."),
 "T05": ("A feature claim with a compiler witness", "Keep language, library, ABI and extension claims in separate columns. Implement a fallback and compile the same bounded behavior under the chosen standards modes.", "A compiler accepting syntax while the required header or runtime operation remains unavailable."),
 "T06": ("A single registry, several generated views", "Follow tokens through preprocessing; compare expansion with evaluation. Generate declarations and dispatch from one small registry and test bad configuration values.", "A side-effect argument used twice by a superficially type-correct macro."),
 "T07": ("A build graph with observable edges", "Touch a header, generated configuration and source separately; predict which artifacts must change. Reproduce one component with Make and CMake before trusting cached outputs.", "A clean build that passes while an incremental build reuses an incompatible object."),
 "T08": ("A hostile caller around an assembly kernel", "Name argument/result locations, preserved registers, stack alignment and permitted accesses. Test adversarial surrounding register values and inspect actual instruction bytes.", "A routine returning the right answer while corrupting a callee-saved register."),
 "T09": ("A symbol through compilation, linking and loading", "Trace a reference through relocation, archive selection and final placement. Separate source visibility, symbol binding, section reachability and runtime mapping.", "An archive order change or garbage-collected section that alters the executable despite unchanged C."),
 "T10": ("A Linux executable owning its entry", "Decode the real initial stack, establish the ABI before entering C and wrap only declared syscalls. Record partial progress and exit behavior without importing hosted startup assumptions.", "A short write followed by interruption, where resending the original span duplicates bytes."),
 "T11": ("The complete dependency closure", "Audit both object requirements and the linked image. Supply selected support routines and explain where structure copies, division or atomics can introduce helpers.", "An optimized build introducing a helper that was absent at debug optimization."),
 "T12": ("Library contracts as implementation experiments", "Separate the public contract, local implementation policy and your independent oracle. Exercise zero length, overlap, locale, failure and execution-environment assumptions only where admitted.", "Two implementations disagree because the supposed test input violates one interface's precondition."),
 "T13": ("An effect ledger that includes hidden work", "Measure first-use and steady-state paths separately; identify allocation, locks, stack, callbacks and global state. Explain the boundaries of interception and opaque dependencies.", "A no-malloc spelling audit missing allocation in a callback or lazy initialization."),
 "T14": ("A representation with executable invariants", "For each mutator, state the abstraction function, representation invariant and failure-preserving transition. Trace heap repair, hash tombstones and tree rotations before generalizing interfaces.", "Exhaust a storage pool exactly when a multi-step mutation has already changed one link."),
 "T15": ("Algorithms over bounded representations", "Derive the invariant and work count before timing. Compare stable permutations, graph frontier policies and incremental parser states against independent small-case models.", "A duplicate-heavy input exposing a partition or stability assumption absent from random distinct keys."),
 "T16": ("Allocation policy separated from storage legality", "Track live extents, metadata, alignment, fragmentation and ownership through every request. Explain whether storage is a typed pool, allocated region or explicitly implementation-bound mapping.", "A failed resize after metadata preparation must preserve the complete original object and owner."),
 "T17": ("A numerical result with an error model", "State units, ranges, conditioning and rounding before choosing a layout. Compare residuals and convergence rather than treating one floating result as exact.", "A small residual masking a large forward error in an ill-conditioned problem."),
 "T18": ("An API whose failures preserve a state", "Write success and failure postconditions, ownership transfer points and callback effects. Compile a second consumer that attempts every documented boundary.", "A capacity error leaving a caller unable to determine which object it owns."),
 "T19": ("Abstractions with visible machine costs", "Derive an operation-table interface from two concrete users. Compare typed and erased representations while tracking indirection, lifetime and invalid states.", "A retained callback context outliving its owner although the function pointer remains valid."),
 "T20": ("A portable mechanism behind semantic adapters", "Separate policy from host, MCU and model effects. Define completion, cancellation and buffer borrowing at each adapter boundary before composing them.", "A supposedly interchangeable adapter returning from submit before the borrowed buffer is reusable."),
 "T21": ("A failure-complete state machine", "Enumerate acquisition prefixes and commit points. Preserve the original cause while cleanup runs, and distinguish cancellation request from confirmed quiescence.", "Cleanup itself failing after a partially published operation."),
 "T22": ("A library consumed outside its own tree", "Install the interface and rebuild an independent client. Carry compiler options, dependency provenance, compatibility claims and reproducible release evidence with the package.", "A consumer that succeeds only because it accidentally includes a private build-tree header."),
 "T23": ("A process and descriptor ownership ledger", "Trace creation, inheritance, sharing, close, mapping and synchronization at actual system-call boundaries. Keep returned bytes, file offsets and persistence claims distinct.", "An inherited write descriptor keeping a pipe alive after the apparent producer exits."),
 "T24": ("An event loop with generation identities", "Define readiness, bounded work, deadlines and continuation state. Reconstruct partial I/O and stale notifications with a deterministic adapter before using live descriptors.", "An event for an old descriptor number arriving after that number has been reused."),
 "T25": ("A happens-before argument with a counterexample", "Write ownership and publication conditions before adding atomics. Draw conflicting accesses, predicate transitions and synchronization edges; test waiting and shutdown together.", "A notification occurring before a waiter commits to sleep, or a relaxed flag publishing unfinished data."),
 "T26": ("Progress and reclamation as separate claims", "Construct finite histories for linearization, identity reuse and reclamation. Compare thread completion with system-wide progress and state the scheduler assumptions.", "A correct compare-and-swap on a recycled address that no longer denotes the original object."),
 "T27": ("Reset-to-main as an inspected memory transform", "Name reset state, vector ownership, stack region and load/run addresses. Verify copy, zero and retained ranges before trusting global values.", "A valid link map paired with an incorrect startup zero range."),
 "T28": ("A bounded peripheral transaction", "Read the device's register semantics before designing the driver state machine. Separate controller protocol, electrical interface, clock source and completion condition.", "A read-modify-write accidentally clearing a write-one-to-clear status bit."),
 "T29": ("Interrupt and DMA ownership histories", "Follow CPU, ISR and device access rights through submit, completion, timeout and reset. State which memory masters can reach the region and which ordering operation is required.", "A late DMA completion touching storage already reused after an unconfirmed cancellation."),
 "T30": ("An execution budget with interference", "Separate algorithmic work, release jitter, blocking, interrupt interference and deadline. Compare a recurrence with measured traces under stated scheduling assumptions.", "A watchdog feed succeeding while a required task no longer makes useful progress."),
 "T31": ("An image-bound fault record", "Decode the captured frame against the exact map and build. Bound trace overhead and retained records; reproduce startup and deployment failures with a known recovery image.", "A debugger session changing timing or reset state enough to conceal the original defect."),
 "T32": ("Tests that reject plausible wrong mechanisms", "Start from a contract, then combine boundary examples, stateful generation, metamorphic relations, mutation and reduced reproducers. Keep oracle independence visible.", "A high-coverage suite accepting a deliberately corrupted invariant because it asserts only nominal outputs."),
 "T33": ("A bounded proof with visible assumptions", "Translate the C arithmetic and environment into the verifier's model. Preserve counterexamples, unwinding checks, unproved obligations and assumptions as separate outputs.", "An overstrong precondition or insufficient unwinding bound making a false claim appear verified."),
 "T34": ("Optimization after a fixed correctness gate", "Measure the workload before transforming layout, branches, alias information or vector code. Compare generated code and consumed results under matched builds.", "An apparently faster kernel whose observable work was optimized away or whose input contract changed."),
 "T35": ("A bounded authority and resource boundary", "Validate framing and sizes before granting effects. Model who may request operations, which resources they consume and how overload or corruption is contained.", "A syntactically valid request causing unbounded retention or accessing a descriptor beyond its authority."),
 "T36": ("One invariant through two ISAs and a C interface", "Implement lower-bound, stable insertion and heap repair kernels under both target ABIs. Use representation-preserving traces to connect assembly state to the abstract algorithm.", "An equal-key or extreme-index case that leaves nominal examples correct while violating the invariant."),
 "T37": ("A machine model with explicit observation limits", "Move from Boolean slices to committed instructions, hazards and cache traces. Compare model predictions with actual architectural evidence without equating simulated cycles to hardware time.", "A flushed instruction changing architectural state, or a cache miss being charged as an assumed fixed elapsed time."),
 "T38": ("The complete controller-to-wire path", "Derive frames and transaction states, then measure the relevant electrical behavior. Keep UART/USART, SPI/I2C controllers and RS/CAN transceivers as distinct mechanisms.", "A transmitter declaring completion while the final stop bit or bus-turnaround interval is still active."),
 "T39": ("Ethernet frames with owned descriptors", "Trace PHY readiness, MAC framing and DMA ownership independently. Validate every length and descriptor transition before exposing a frame to upper layers.", "A link reset followed by completion from an earlier descriptor incarnation."),
 "T40": ("Seven explicit service boundaries", "Give each layer an input contract, peer identity and failure responsibility. Test routing across two physical segments while preventing the relay from acquiring application responsibility.", "A valid lower-layer frame carrying an invalid session or application operation."),
 "T41": ("A minimal Internet endpoint with actual interoperability", "Separate sequence arithmetic, stream delivery, timers, flow pressure and HTTP framing. Verify each substate with virtual time and an independent peer before closing physical gates.", "A delayed duplicate, sequence wrap or half-close exposing a disagreement between transport and application completion."),
 "T42": ("One HTTP core across transport adapters", "Implement octet grammar, authority and framing before routing. Preserve parser/output cursors across fragmentation, short writes, persistence and bounded deadlines.", "Two interpretations of the same request framing or a partially emitted response followed by buffer reuse."),
 "T43": ("A measurement that answers a named question", "Specify workload, unit, boundary and uncertainty before choosing a counter. Distinguish asymptotic work, machine cycles, CPU time, waiting, completion and durable effects.", "A counter wrap, multiplexed event or asynchronous return producing a plausible but invalid measurement."),
 "T44": ("A diagnosis that discriminates competing causes", "Seal the prediction, establish a same-failure reproducer and choose an observation before editing. Preserve the earliest supported invalid transition and an adjacent case rejecting a symptom-only patch.", "A delayed variant with a different root cause but the same visible crash."),
 "T45": ("A memory defect with a paired repaired control", "Distinguish extent, initialization, lifetime, placement and capacity failures. Compare native, diagnostic and abstract histories without promising deterministic undefined-behavior symptoms.", "A global-object overwrite mislabeled as a dynamically growing .bss, or clearing one pointer presented as repair of every stale alias."),
}

def decode(text):
    match = re.match(r'\A```json\s*\n(.*?)\n```\s*\n', text, re.S)
    return json.loads(match[1]), text[match.end():]

def encode(meta, body):
    return "```json\n" + json.dumps(meta, indent=2, ensure_ascii=False) + "\n```\n\n" + body

def main():
    if (ROOT/'curriculum/edition.json').exists():
        raise SystemExit('Edition already migrated. Refusing to overwrite active teaching from the v1 archive.')
    with zipfile.ZipFile(ARCHIVE) as archive:
        raw = {name: archive.read(name) for name in archive.namelist() if name.startswith(("curriculum/", "references/"))}
    old_domains = []
    for name in ["taxonomy.json", "enhancement_taxonomy.json", "performance_debugging_taxonomy.json"]:
        old_domains += json.loads(raw["curriculum/" + name])["domains"]
    objectives = {o["id"]: o for d in old_domains for t in d["subtopics"] for o in t["objectives"]}
    old_projects = {Path(name).stem: decode(data.decode("utf-8-sig"))[0]
                    for name, data in raw.items() if name.startswith("curriculum/projects/") and name.endswith(".md")}
    dump(ROOT / "curriculum/edition.json", {
        "schema_version": 2, "edition": "C Mastery 2", "baseline_commit": BASELINE,
        "baseline_tag": "curriculum-v1-2026-10-10", "completion_policy": "comprehensive",
        "language": "C17 with explicitly bounded C23 comparisons",
        "targets": ["Linux x86-64", "Cortex-M4 Thumb", "QEMU x86-64 educational kernel"],
        "hardware_required": ["UART", "USART", "I2C", "SPI", "RS-232", "RS-422", "RS-485", "CAN", "CAN FD", "Ethernet"],
        "new_required_families": ["T46", "T47", "T48"],
        "baseline_counts": {"domains": 45, "projects": len(old_projects), "objectives": len(objectives)},
        "learner_time_calibration": "not-run", "physical_validation": "not-run"})
    dump(ROOT / "curriculum/migration.json", {
        "schema_version": 2, "baseline_commit": BASELINE,
        "objectives": [{"old_id": key, "old_text": value["text"], "new_ids": [key],
                         "disposition": "retained", "rationale": "Preserve the assessable claim while rewriting its teaching and practice."}
                        for key, value in sorted(objectives.items())],
        "projects": [{"old_id": key, "new_ids": [key], "disposition": "revised",
                      "baseline_sha256": hashlib.sha256(raw["curriculum/projects/"+key+".md"]).hexdigest()}
                     for key in sorted(old_projects)]})
    for key, (title, mechanism, counterexample) in THREADS.items():
        name = "curriculum/modules/" + key + ".md"
        meta, body = decode(raw[name].decode("utf-8-sig"))
        meta["edition"] = 2
        marker = "## Study sequence and teaching"
        introduction = ("## Component progression\n\n**" + title + ".** " + mechanism +
            "\n\nBefore implementation, predict this discriminating case: **" + counterexample +
            "** Carry its evidence into the projects below. Reuse one accepted component revision; "
            "new projects charge their new mechanism or experiment, not a second implementation of the old one.\n\n"
            "Follow the [project route](../guides/project-route.md) for ordering. Domain numbers are reference categories. "
            "The [laboratory](../guides/laboratory.md) and [shared contract cards](../guides/component-contracts.md) "
            "separate automatic execution from reasoning, transfer and physical observations.\n\n")
        body = body.replace(marker, introduction + marker, 1)
        write(ROOT / name, encode(meta, body))
    redundant = "Attach a trace, artifact, counterexample or explicit bounded analysis; optional implementation conditions in this project remain in force."
    for key in sorted(old_projects):
        name = "curriculum/projects/" + key + ".md"
        meta, body = decode(raw[name].decode("utf-8-sig"))
        meta["edition"] = 2
        meta["evidence_template"] = "compact" if meta["module"] in {"T01", "T02", "T03", "T04", "T05", "T06"} else "component"
        if key in EARLY:
            meta["requires_projects"] = EARLY[key]
            meta["prelude_policy"] = "Selected objective cards may be studied and evidenced without completing their whole domain."
        title, mechanism, counterexample = THREADS.get(meta["module"], (
            "Integration of accepted components", "Declare the newly exercised boundary and reference existing component evidence.",
            "A changed adapter completion or ownership assumption invalidating an otherwise correct component."))
        front = ("## Working card\n\n" +
            "**Build:** " + meta["title"] + ". **Evidence:** " + meta["evidence_template"] + " dossier. " +
            "**Start:** predict the supplied case before reading its expected result.\n\n" +
            "**Engineering question:** " + mechanism + "\n\n" +
            "**Changed-condition challenge:** " + counterexample + "\n\n" +
            "Use `python -m cmastery init " + key + " <empty-workspace>` to obtain the brief, fixtures and any executable family scaffold. "
            "A `not-run` or `unavailable` result leaves the corresponding claim open. "
            "Apply the [shared contract cards](../guides/component-contracts.md), then the project-specific obligations below.\n\n")
        body = body.replace(redundant, "Link distinct evidence in the dossier; the project's execution conditions still apply.")
        write(ROOT / name, encode(meta, front + body))
    dump(ROOT / "reviews/v2/migration_origin.json", {
        "baseline_commit": BASELINE, "archive_sha256": hashlib.sha256(ARCHIVE.read_bytes()).hexdigest(),
        "original_objectives": len(objectives), "original_projects": len(old_projects),
        "scope": "Authoring migration identity; active acceptance is recorded by separate current-source checks."})
    print("Prepared second-edition teaching entry cards and explicit coverage migration.")

if __name__ == "__main__":
    main()
