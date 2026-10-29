#ifndef TEXT_COMPOSITION_H
#define TEXT_COMPOSITION_H
#include "parsing.h"
#include <stddef.h>
#include <stdint.h>
enum {
  TC_OK = 0,
  TC_RECOVERED = 1,
  TC_INVALID = 2,
  TC_RANGE = 3,
  TC_FUEL = 4,
  TC_SYNTAX = 5,
  TC_CYCLE = 6
};
struct tc_text {
  unsigned char bytes[4096];
  size_t length;
};
struct tc_error {
  size_t begin, end, error_offset;
  int status;
};
struct tc_batch {
  int32_t values[16];
  size_t begins[16];
  unsigned count, errors;
  struct tc_error diagnostics[16];
};
/* newline_sync=0: semicolons only; 1: semicolons or newline. */
int tc_recover(const unsigned char *input, size_t length, unsigned newline_sync,
               unsigned budget, struct tc_batch *out);
enum { TD_NIL, TD_TEXT, TD_LINE, TD_CAT, TD_NEST, TD_GROUP };
struct tc_node {
  unsigned kind, left, right, value;
  size_t offset, length;
};
struct tc_doc {
  struct tc_node nodes[128];
  unsigned count, root;
  unsigned char text[512];
  size_t length;
};
int tc_layout(const struct tc_doc *doc, unsigned width, unsigned budget, struct tc_text *out);
/* Actual successful PdTree, at most14 nodes; source offsets need not survive.
 */
int tc_expression_doc(const struct pd_tree *tree, struct tc_doc *out);
enum { TT_LITERAL, TT_VALUE, TT_IF, TT_LOOP, TT_END, TT_INCLUDE };
struct tc_op {
  unsigned kind, value, jump;
};
struct tc_program {
  struct tc_op ops[128];
  unsigned count;
};
struct tc_value {
  unsigned char bytes[256];
  size_t length;
};
struct tc_field {
  struct tc_value values[4];
  unsigned count;
};
struct tc_model {
  struct tc_field fields[4];
};
int tc_template_compile(const unsigned char *text, size_t length,
                        struct tc_program *out);
/* html_text=1 escapes model values for HTML text; 0 is explicit plain text. */
int tc_template_render(const struct tc_program *bank, unsigned count, unsigned entry,
                       const struct tc_model *model, unsigned html_text,
                       unsigned budget, struct tc_text *out);
#endif
