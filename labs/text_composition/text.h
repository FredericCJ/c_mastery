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
typedef struct {
  unsigned char bytes[4096];
  size_t length;
} TcText;
typedef struct {
  size_t begin, end, error_offset;
  int status;
} TcError;
typedef struct {
  int32_t values[16];
  size_t begins[16];
  unsigned count, errors;
  TcError diagnostics[16];
} TcBatch;
/* newline_sync=0: semicolons only; 1: semicolons or newline. */
int tc_recover(const unsigned char *input, size_t length, unsigned newline_sync,
               unsigned budget, TcBatch *out);
enum { TD_NIL, TD_TEXT, TD_LINE, TD_CAT, TD_NEST, TD_GROUP };
typedef struct {
  unsigned kind, left, right, value;
  size_t offset, length;
} TcNode;
typedef struct {
  TcNode nodes[128];
  unsigned count, root;
  unsigned char text[512];
  size_t length;
} TcDoc;
int tc_layout(const TcDoc *doc, unsigned width, unsigned budget, TcText *out);
/* Actual successful PdTree, at most14 nodes; source offsets need not survive.
 */
int tc_expression_doc(const PdTree *tree, TcDoc *out);
enum { TT_LITERAL, TT_VALUE, TT_IF, TT_LOOP, TT_END, TT_INCLUDE };
typedef struct {
  unsigned kind, value, jump;
} TcOp;
typedef struct {
  TcOp ops[128];
  unsigned count;
} TcProgram;
typedef struct {
  unsigned char bytes[256];
  size_t length;
} TcValue;
typedef struct {
  TcValue values[4];
  unsigned count;
} TcField;
typedef struct {
  TcField fields[4];
} TcModel;
int tc_template_compile(const unsigned char *text, size_t length,
                        TcProgram *out);
/* html_text=1 escapes model values for HTML text; 0 is explicit plain text. */
int tc_template_render(const TcProgram *bank, unsigned count, unsigned entry,
                       const TcModel *model, unsigned html_text,
                       unsigned budget, TcText *out);
#endif
