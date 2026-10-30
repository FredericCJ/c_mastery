/*
 * Numerical laboratory interfaces (binary64 on a hosted C17 system).
 *
 * Shared rules; the per-task contracts are in CONTRACTS.md:
 *  - Every input value is finite with magnitude at most ND_VALUE_LIMIT.
 *  - Pointer arguments name live objects of the stated extent; a null data
 *    pointer is accepted only with a zero count.
 *  - Unless a function says otherwise, a rejected call leaves every output and
 *    control object unchanged. Every status other than ND_OK is a rejection.
 *  - Nothing here blocks, allocates or retains a pointer after it returns.
 */
#ifndef NUMERICAL_DEPTH_H
#define NUMERICAL_DEPTH_H
#include <stddef.h>
#include <stdint.h>

/* Largest magnitude of any admitted input value. */
#define ND_VALUE_LIMIT 1e100

/* Admission limits: element counts unless the name says otherwise. */
enum
{
    ND_SUM_MAX = 4096,
    ND_QMAC_MAX = 32,
    ND_VIEW_DIM_MAX = 8,
    ND_INTERP_MAX = 256,
    ND_STREAM_MAX = 256,
    ND_SOLVE_MAX_ORDER = 3,
    ND_JACOBI_LIMIT_MAX = 10000,
    ND_LABEL_BYTES = 24
};

/* Status codes returned by every function below. */
enum
{
    ND_OK,
    ND_BAD,
    ND_RANGE,
    ND_SINGULAR,
    ND_LIMIT,
    ND_STAGNANT
};

/* Summation methods for nd_sum. */
enum
{
    ND_NAIVE,
    ND_PAIRWISE,
    ND_KAHAN,
    ND_NEUMAIER
};

/*
 * Sum of count values by the given method. Stores the result in *out and
 * returns ND_OK; ND_BAD for a null out, a null data pointer with a nonzero
 * count, more than ND_SUM_MAX values, an unknown method or a value that is
 * not finite or exceeds ND_VALUE_LIMIT. The sum of no values is +0.
 */
int nd_sum(const double *values, size_t count, unsigned method, double *out);

/*
 * Fixed-point arithmetic on int16_t storage with q fraction bits (q is 7 or
 * 11); an admitted operand lies in [-2^q, 2^q). ND_BAD for an unsupported q, a
 * null out or an operand outside that range; nd_qmac also rejects a count
 * above ND_QMAC_MAX and null arrays with a nonzero count.
 *
 * nd_qmul: rounded product of two operands.
 * nd_qmac: multiply-accumulate of count (0..ND_QMAC_MAX) pairs, rounded once.
 * nd_quantize: nearest fixed-point value of a finite real number; ties go away
 *              from zero and values beyond the range saturate. A NaN or an
 *              infinity is ND_BAD.
 */
int nd_qmul(int16_t a, int16_t b, unsigned q, int16_t *out);
int nd_qmac(const int16_t *a, const int16_t *b, size_t count, unsigned q,
            int16_t *out);
int nd_quantize(double value, unsigned q, int16_t *out);

/*
 * A strided window onto caller-owned doubles. Element (i, j) lives at
 * data[i * rs + j * cs]; extent is the number of doubles data points at, and
 * rs and cs are strides in elements (not bytes). The view does not own data.
 */
struct nd_view
{
    double *data;
    size_t extent;
    size_t rows;
    size_t cols;
    size_t rs;
    size_t cs;
};

/*
 * Validates a view: ND_OK, or ND_BAD for a null pointer, a dimension outside
 * 1..ND_VIEW_DIM_MAX, a zero stride, an index computation that overflows, an
 * element beyond extent, or two logical elements that share storage.
 */
int nd_view(struct nd_view view);

/*
 * c = a * b with the selected traversal method (0, 1 or 2). ND_BAD for an
 * invalid view, an input element that is not finite or exceeds ND_VALUE_LIMIT,
 * incompatible shapes, an unknown method, or an output element that overlaps
 * an input element. On ND_BAD no output element is written.
 */
int nd_matrix(struct nd_view a, struct nd_view b, struct nd_view c,
              unsigned method);

/* Multiplies every element in place by factor. ND_BAD for a bad view/factor. */
int nd_scale(struct nd_view view, double factor);

/* Dot product of two one-row views of equal width, stored in *out. */
int nd_dot(struct nd_view a, struct nd_view b, double *out);

/*
 * Piecewise-linear interpolation through count (1..ND_INTERP_MAX) strictly
 * increasing knots. Outside the knot interval, clamp selects the nearest
 * endpoint ordinate (1) or rejects with ND_RANGE (0). ND_BAD for invalid
 * knots, ordinates or query, or a clamp flag other than 0 or 1.
 */
int nd_interp(const double *knots, const double *ordinates, size_t count,
              double query, int clamp, double *out);

/* Operation counts of one nd_solve call, by kind. */
struct nd_cost
{
    size_t adds;
    size_t multiplies;
    size_t divisions;
    size_t row_moves;
};

/*
 * Direct solve of an n-by-n system (n is 2 or 3) with row-scaled partial
 * pivoting. A pivot at or below tolerance times its row scale gives
 * ND_SINGULAR; a nonfinite intermediate gives ND_RANGE. ND_BAD (nothing
 * written) for a tolerance outside the finite range [0, 1), n outside 2..3,
 * a view that is not valid, not square (rows and cols both n) or not
 * admitted (a nonfinite entry or one beyond ND_VALUE_LIMIT), or a NULL rhs,
 * out or cost. The solution and the operation counts are published together,
 * only on ND_OK.
 */
int nd_solve(struct nd_view a, const double *rhs, size_t n, double tolerance,
             double *out, struct nd_cost *cost);

/*
 * Jacobi iteration from zero, at most limit steps (up to ND_JACOBI_LIMIT_MAX)
 * until the infinity-norm residual is at most tolerance. *steps is written
 * only on ND_OK. Failures: ND_LIMIT, ND_STAGNANT, ND_RANGE, ND_SINGULAR for a
 * zero diagonal, ND_BAD otherwise.
 */
int nd_jacobi(struct nd_view a, const double *rhs, size_t n, double tolerance,
              unsigned limit, double *out, unsigned *steps);

/*
 * count Euler steps of y' = -rate * y with step h, writing the initial value
 * and every step (count + 1 doubles) to out, which holds capacity doubles.
 * ND_BAD for a count above ND_STREAM_MAX, capacity below count + 1, or a
 * nonpositive step or rate; ND_RANGE for a nonfinite intermediate value.
 */
int nd_euler(double y0, double h, double rate, size_t count, double *out,
             size_t capacity);

/*
 * First-order recursive filter: s = feed * x + feedback * s for each input.
 * *state carries s between calls. in and out may overlap completely or
 * partly; state is distinct from both. ND_BAD for a coefficient outside its
 * stable range, count above ND_STREAM_MAX or capacity below count.
 */
int nd_iir(double *state, const double *in, size_t count, double feed,
           double feedback, double *out, size_t capacity);

#define ND_ITEMS 16

/* One reconstructed record; labels are ND_LABEL_BYTES bytes, unterminated. */
struct nd_record
{
    double x;
    double y;
    unsigned id;
    char label[ND_LABEL_BYTES];
};

/*
 * Structure-of-arrays store of up to ND_ITEMS records. The caller owns it and
 * zero-initializes it. serial is the last identifier handed out; count is the
 * number of live records. Array index i of every member describes record i;
 * indices change when records are removed or reordered, identifiers do not.
 */
struct nd_store
{
    double x[ND_ITEMS];
    double y[ND_ITEMS];
    unsigned id[ND_ITEMS];
    char label[ND_ITEMS][ND_LABEL_BYTES];
    size_t count;
    unsigned serial;
};

/*
 * Store operations; each returns ND_OK or ND_BAD and leaves the store
 * unchanged on ND_BAD.
 *
 * nd_insert: appends a record and returns its new identifier in *id.
 * nd_remove: deletes the record with that identifier, compacting the arrays.
 * nd_reorder: applies a permutation of 0..count-1 (order[i] names the old
 *             index that moves to i); anything else is rejected.
 * nd_record: copies the record with that identifier into *out.
 */
int nd_insert(struct nd_store *store, double x, double y,
              const char label[ND_LABEL_BYTES], unsigned *id);
int nd_remove(struct nd_store *store, unsigned id);
int nd_reorder(struct nd_store *store, const unsigned *order, size_t count);
int nd_record(const struct nd_store *store, unsigned id, struct nd_record *out);
#endif
