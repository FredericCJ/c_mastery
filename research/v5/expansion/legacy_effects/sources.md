# T13 effects source inspection

The source register records exact passages actually read on 2026-10-17. The
private roots are `/home/frede/cmastery-effects-tools/glibc-2.41`,
`musl-1.2.5` and `picolibc-1.8.10`. `download_manifest.json` in that private
directory records observed URLs and archive hashes. No third-party source
extract is republished here.

The GNU libc and musl qsort implementations were read in full. Picolibc's
`doc/os.md` and `doc/build.md` were read in full. GCC's named stack/call-graph and
sanitizer option paragraphs and GNU ld's wrapping option were actually opened
and read. These inspections support bounded implementation and tooling claims,
not universal library behavior or unseen dependency internals.

The Open Group mutex pages returned inaccessible through the web tool. That
attempt is an access gap, not an inspected source. Existing repository source
records remain separate historical evidence. The exact published C17/C23 text
gap is not closed by this work.

Native builds are private and preserve their configure commands and logs.
Picolibc's native configuration relies on the host's operating-system interface.
The initial glibc build failed because a privately extracted Bison lacked m4;
the original log is retained and a separate resumed build records the repair.
Neither a source read nor a successful build is a runtime observation: those
are distinguished in `validation.json`.

The native custom-console build exposed a versioned documentation discrepancy:
`doc/os.md` suggests `-Dposix-io=false`, but the tagged Meson option definitions
do not define it and the actual configure command rejected it. Named option
lines were then read; the supported `posix-console=false` configuration was
built, retaining the earlier failure log. The complete `fseek.c` implementation
was read to bound the custom stream's lack of a seek callback. The original
custom-console probe checks that failure; a separate file-backed probe and
missing-`lseek` link control examine the other dependency boundary.

No source registry entry establishes independent review, learner completion,
physical interrupt timing, or comprehensive effect coverage.
