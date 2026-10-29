"""Exact-source, explicit single-thread caller-workspace adaptation of frozen CVM.

This is a separate build profile. Its verification-dependent entry points must
be entered through kf_load_scratch/kf_run_scratch. No baseline provider is edited.
The borrowed workspace and machine are non-reentrant; a callback cannot reacquire
the port. VM recursion still uses the original explicit CvmFrame array.
"""
import hashlib
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[3] / 'tools'))
from c_source import replace_unique, significant, tokenize  # noqa: E402

# Digest of the comment-free C token sequence, so reformatting the frozen source
# does not invalidate the registration while any token change still does.
TOKEN_SHA256 = 'ace1b90129f6df6313402e45208136ab9eb501a9787e0c2d05bedb3248ba740a'
OLD = '''  uint8_t boundary[CVM_CODE];
  int16_t height[CVM_CODE];
  uint16_t queue[CVM_CODE];'''
NEW = '''  if (!kf_borrowed_workspace)
    return CVM_RANGE;
  uint8_t *boundary = kf_borrowed_workspace->boundary;
  int16_t *height = kf_borrowed_workspace->height;
  uint16_t *queue = kf_borrowed_workspace->queue;'''
PREFIX = '''/* Generated explicit workspace profile; see port_registration.py. */
#include "probe.h"
static KfVerifierWorkspace *kf_borrowed_workspace;
'''
SUFFIX = '''
CvmStatus kf_load_scratch(KfVerifierWorkspace *workspace, const uint8_t *bytes,
                          size_t count, CvmProgram *program) {
  if (!workspace || kf_borrowed_workspace) return CVM_RANGE;
  kf_borrowed_workspace = workspace;
  CvmStatus status = cvm_load(bytes, count, program);
  kf_borrowed_workspace = 0;
  return status;
}
CvmStatus kf_run_scratch(KfVerifierWorkspace *workspace, Cvm *machine,
                         const CvmProgram *program, CvmValue *result) {
  if (!workspace || kf_borrowed_workspace) return CVM_RANGE;
  kf_borrowed_workspace = workspace;
  CvmStatus status = cvm_run(machine, program, result);
  kf_borrowed_workspace = 0;
  return status;
}
'''

def token_sha256(text):
    stream = '\n'.join(t.kind + ' ' + t.text for t in significant(tokenize(text)))
    return hashlib.sha256(stream.encode()).hexdigest()

def render(source):
    text = source.decode().replace('\r\n', '\n')
    assert token_sha256(text) == TOKEN_SHA256
    return PREFIX + replace_unique(text, OLD.strip(), NEW.lstrip()) + SUFFIX

if __name__ == '__main__':
    root = Path(__file__).resolve().parents[3]
    output = Path(__file__).with_name('cvm_scratch.c')
    output.write_text(render((root/'assessor/runtime/cvm.c').read_bytes()), newline='\n')
