#include "variants.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SV_CHECK:%d:%s\n", __LINE__, #x);                       \
      return 1;                                                                \
    }                                                                          \
  } while (0)

struct ac_sink {
  unsigned char bytes[512];
  size_t used, quota;
  unsigned calls, disposition;
  size_t spans;
  int bad_count, bad_span;
};

static struct sv_write_result sink(void *context, const unsigned char *bytes,
                          size_t length) {
  struct ac_sink *state = context;
  ++state->calls;
  if (state->bad_count)
    return (struct sv_write_result){length + 1, SV_PROGRESS};
  size_t take = length < state->quota ? length : state->quota;
  for (size_t i = 0; i < take; ++i)
    state->bytes[state->used++] = bytes[i];
  return (struct sv_write_result){take, state->disposition};
}

static struct sv_write_result vector_sink(void *context, const struct sv_span *spans,
                                 size_t count) {
  struct ac_sink *state = context;
  ++state->calls;
  if (!count || count > 3)
    state->bad_span = 1;
  size_t left = state->quota, total = 0;
  state->spans += count;
  for (size_t i = 0; i < count; ++i) {
    if (!spans[i].length)
      state->bad_span = 1;
    size_t take = spans[i].length < left ? spans[i].length : left;
    for (size_t j = 0; j < take; ++j)
      state->bytes[state->used++] = spans[i].data[j];
    total += take;
    left -= take;
  }
  return (struct sv_write_result){total, state->disposition};
}

/* Independent wire oracle: no SF/SV helpers or internal-state copying. */
static int decoded(const unsigned char *wire, size_t length,
                   const unsigned char *payload, size_t n) {
  if (length != n + 2 || wire[0] != n)
    return 0;
  unsigned checksum = 0;
  for (size_t i = 0; i < n; ++i) {
    if (wire[i + 1] != payload[i])
      return 0;
    checksum = (checksum + payload[i]) % 256;
  }
  return wire[n + 1] == checksum;
}

static int result_case(void) {
  const unsigned char payload[] = {1, 2, 3};
  for (size_t cut = 0; cut <= 5; ++cut) {
    struct sv_stream stream = {0};
    struct sv_handle handle = {0};
    sv_init(&stream);
    CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
    struct ac_sink state = {.quota = cut, .disposition = SV_ERROR};
    size_t accepted = 99;
    CHECK(sv_pump(handle, sink, &state, 8, &accepted) == SF_IO);
    CHECK(accepted == cut && state.used == cut && state.calls == 1);
    CHECK(sv_pump(handle, sink, &state, 8, &accepted) == SF_IO &&
          state.calls == 1);
    struct sv_handle old = handle;
    CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
    accepted = 99;
    CHECK(sv_pump(old, sink, &state, 1, &accepted) == SF_INVALID &&
          accepted == 99);
  }
  struct sv_stream stream = {0};
  struct sv_handle handle = {0};
  sv_init(&stream);
  CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
  struct ac_sink state = {.quota = 2, .disposition = SV_PROGRESS};
  size_t accepted = 99;
  CHECK(sv_pump(handle, sink, &state, 0, &accepted) == SF_PENDING &&
        accepted == 0 && !state.calls);
  CHECK(sv_pump(handle, sink, &state, 1, &accepted) == SF_PENDING &&
        accepted == 2);
  state.quota = 0;
  state.disposition = SV_WAIT;
  CHECK(sv_pump(handle, sink, &state, 100, &accepted) == SF_PENDING &&
        accepted == 2 && state.calls == 2);
  state.quota = 1;
  state.disposition = SV_PROGRESS;
  CHECK(sv_pump(handle, sink, &state, 1, &accepted) == SF_PENDING &&
        accepted == 3);
  state.quota = 0;
  state.disposition = SV_ERROR;
  CHECK(sv_pump(handle, sink, &state, 1, &accepted) == SF_IO && accepted == 3);
  CHECK(state.used == 3 && state.bytes[0] == 3 && state.bytes[1] == 1 &&
        state.bytes[2] == 2);
  CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
  state = (struct ac_sink){.disposition = SV_RETRY};
  CHECK(sv_pump(handle, sink, &state, 7, &accepted) == SF_PENDING &&
        accepted == 0 && state.calls == 7);
  state = (struct ac_sink){.quota = 2, .disposition = SV_WAIT};
  CHECK(sv_pump(handle, sink, &state, 7, &accepted) == SF_PENDING &&
        accepted == 2 && state.calls == 1);
  CHECK(sv_cancel(handle) == SF_OK);
  CHECK(sv_pump(handle, sink, &state, 7, &accepted) == SF_CANCELLED &&
        accepted == 2 && state.calls == 1);
  for (unsigned bad = 0; bad < 4; ++bad) {
    CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
    state = (struct ac_sink){.disposition =
                       bad == 1 ? 91 : (bad == 2 ? SV_RETRY : SV_PROGRESS),
                   .quota = bad == 2 ? 1 : 0,
                   .bad_count = bad == 0};
    CHECK(sv_pump(handle, sink, &state, 1, &accepted) == SF_INVALID &&
          accepted == 0);
    CHECK(sv_pump(handle, sink, &state, 9, &accepted) == SF_INVALID &&
          state.calls == 1);
  }
  puts("SV_RESULT:6 partial-error boundaries;2/0/1/error;retry/wait "
       "budgets;invalid results;terminal/stale handles");
  return 0;
}

static int vector_case(void) {
  unsigned char payload[64];
  {
    const unsigned char original[] = {1, 2, 3};
    const size_t quotas[] = {2, 1, 2};
    struct sv_stream stream = {0};
    struct sv_handle handle = {0};
    sv_init(&stream);
    CHECK(sv_start(&stream, original, 3, &handle) == SF_OK);
    struct ac_sink state = {.disposition = SV_PROGRESS};
    size_t accepted = 99;
    for (unsigned step = 0; step < 3; ++step) {
      state.quota = quotas[step];
      CHECK(sv_pumpv(handle, vector_sink, &state, 1, &accepted) ==
            (step == 2 ? SF_DONE : SF_PENDING));
    }
    CHECK(state.calls == 3 && accepted == 5 &&
          decoded(state.bytes, state.used, original, 3));
  }
  for (size_t i = 0; i < 64; ++i)
    payload[i] = (unsigned char)(i * 17 + 3);
  unsigned cases = 0;
  for (size_t n = 0; n <= 64; ++n)
    for (size_t quota = 1; quota <= 7; ++quota) {
      struct sv_stream stream = {0};
      struct sv_handle handle = {0};
      sv_init(&stream);
      CHECK(sv_start(&stream, payload, n, &handle) == SF_OK);
      struct ac_sink state = {.quota = quota, .disposition = SV_PROGRESS};
      size_t accepted = 99;
      int result = SF_PENDING;
      for (unsigned step = 0; step < 67 && result == SF_PENDING; ++step)
        result = sv_pumpv(handle, vector_sink, &state, 1, &accepted);
      CHECK(result == SF_DONE && accepted == n + 2 && !state.bad_span);
      CHECK(decoded(state.bytes, state.used, payload, n));
      unsigned calls = state.calls;
      CHECK(sv_pumpv(handle, vector_sink, &state, 1, &accepted) == SF_DONE &&
            state.calls == calls);
      ++cases;
    }
  for (size_t cut = 0; cut <= 5; ++cut) {
    struct sv_stream stream = {0};
    struct sv_handle handle = {0};
    sv_init(&stream);
    CHECK(sv_start(&stream, payload, 3, &handle) == SF_OK);
    struct ac_sink state = {.quota = cut, .disposition = SV_ERROR};
    size_t accepted = 99;
    CHECK(sv_pumpv(handle, vector_sink, &state, 1, &accepted) == SF_IO &&
          accepted == cut);
  }
  printf("SV_VECTOR:%u payload/quota pairs;6 partial-error boundaries;empty "
         "spans omitted\n",
         cases);
  return 0;
}

static int queue_case(void) {
  struct sv_queue queue = {0};
  svq_init(&queue);
  unsigned char one[] = {1, 2, 3}, two[] = {8};
  struct sv_handle first = {0}, second = {0}, third = {0};
  struct sv_event event = {0};
  CHECK(svq_submit(&queue, one, 3, &first) == SF_OK);
  CHECK(svq_submit(&queue, two, 1, &second) == SF_OK);
  one[0] = 99;
  two[0] = 77;
  CHECK(svq_submit(&queue, one, 3, &third) == SF_STATE && !third.owner);
  struct ac_sink state = {.disposition = SV_WAIT};
  CHECK(svq_pump(&queue, sink, &state, 99, &event) == SF_PENDING &&
        event.accepted == 0 && state.calls == 1);
  CHECK(event.handle.owner == first.owner);
  state = (struct ac_sink){.quota = 1, .disposition = SV_PROGRESS};
  for (unsigned step = 0; step < 5; ++step) {
    CHECK(svq_pump(&queue, sink, &state, 1, &event) ==
          (step == 4 ? SF_DONE : SF_PENDING));
    CHECK(event.handle.owner == first.owner && event.accepted == step + 1);
  }
  for (unsigned step = 0; step < 3; ++step)
    CHECK(svq_pump(&queue, sink, &state, 1, &event) ==
          (step == 2 ? SF_DONE : SF_PENDING));
  const unsigned char original_one[] = {1, 2, 3}, original_two[] = {8};
  CHECK(decoded(state.bytes, 5, original_one, 3) &&
        decoded(state.bytes + 5, 3, original_two, 1));
  CHECK(svq_pump(&queue, sink, &state, 1, &event) == SF_STATE &&
        state.calls == 8);

  /* A previous request identity must not cancel the next occupant. */
  CHECK(svq_submit(&queue, one, 3, &third) == SF_OK &&
        third.generation != first.generation);
  event.accepted = 99;
  CHECK(svq_cancel(&queue, first, &event) == SF_INVALID &&
        event.accepted == 99);
  CHECK(svq_cancel(&queue, third, &event) == SF_OK && event.accepted == 0 &&
        !queue.tainted);
  CHECK(svq_cancel(&queue, third, &event) == SF_INVALID);

  CHECK(svq_submit(&queue, original_one, 3, &first) == SF_OK);
  CHECK(svq_submit(&queue, original_two, 1, &second) == SF_OK);
  state = (struct ac_sink){.quota = 2, .disposition = SV_PROGRESS};
  CHECK(svq_pump(&queue, sink, &state, 1, &event) == SF_PENDING &&
        event.accepted == 2);
  CHECK(svq_cancel(&queue, first, &event) == SF_OK && event.accepted == 2 &&
        queue.tainted);
  unsigned calls = state.calls;
  event.accepted = 99;
  CHECK(svq_pump(&queue, sink, &state, 100, &event) == SF_STATE &&
        state.calls == calls && event.accepted == 99);
  CHECK(svq_reset_transport(&queue, 0) == SF_STATE);
  /* New sink models a verified closed old channel and reset peer decoder. */
  state = (struct ac_sink){.quota = 1, .disposition = SV_PROGRESS};
  CHECK(svq_reset_transport(&queue, 1) == SF_OK);
  CHECK(svq_pump(&queue, sink, &state, 3, &event) == SF_DONE);
  CHECK(decoded(state.bytes, state.used, original_two, 1));

  CHECK(svq_submit(&queue, original_one, 3, &first) == SF_OK);
  CHECK(svq_submit(&queue, original_two, 1, &second) == SF_OK);
  CHECK(svq_cancel(&queue, second, &event) == SF_OK && !queue.tainted);
  state = (struct ac_sink){.quota = 5, .disposition = SV_ERROR};
  CHECK(svq_pump(&queue, sink, &state, 1, &event) == SF_IO &&
        event.accepted == 5 && queue.tainted);
  CHECK(svq_reset_transport(&queue, 1) == SF_OK);
  queue.next_ticket = UINT64_MAX;
  third = (struct sv_handle){0};
  CHECK(svq_submit(&queue, one, 3, &third) == SF_RANGE && !third.owner);
  svq_init(&queue);
  queue.slots[0].frame.generation = UINT32_MAX;
  CHECK(svq_submit(&queue, one, 3, &first) == SF_OK &&
        first.owner == &queue.slots[1]);
  CHECK(svq_cancel(&queue, first, &event) == SF_OK);
  queue.slots[1].frame.generation = UINT32_MAX;
  CHECK(svq_submit(&queue, one, 3, &third) == SF_STATE);
  puts("SV_QUEUE:two owned frames;FIFO;full preservation;stale cancel;partial "
       "cancellation reset;diagnostic after complete frame;retirement");
  return 0;
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "result"))
    return result_case();
  if (!strcmp(argv[1], "vector"))
    return vector_case();
  if (!strcmp(argv[1], "queue"))
    return queue_case();
  return 2;
}
