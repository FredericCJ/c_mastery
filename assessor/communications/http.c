#include "com_http.h"
#include <string.h>
static int token(unsigned char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9') || (c && strchr("!#$%&'*+-.^_`|~", c));
}
static unsigned lower(unsigned c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }
static int equal(const char *a, size_t n, const char *b) {
  if (strlen(b) != n)
    return 0;
  for (size_t i = 0; i < n; i++)
    if (lower((unsigned char)a[i]) != lower((unsigned char)b[i]))
      return 0;
  return 1;
}
static int fail(struct http_parser *p, int status) {
  p->state = HTTP_FAILED;
  p->request.status = status;
  p->request.close = 1;
  return HTTP_ERROR;
}
void http_init(struct http_parser *p, const char *authority) {
  memset(p, 0, sizeof *p);
  if (!authority || strlen(authority) >= sizeof p->configured_authority) {
    fail(p, 400);
    return;
  }
  strcpy(p->configured_authority, authority);
  p->state = HTTP_LINE;
}
static int authority_valid(const char *s, size_t n) {
  if (!n || n >= 128)
    return 0;
  /* The lab has a configured DNS/IPv4 authority, no IPv6 literal or userinfo.
   */
  size_t colon = n;
  for (size_t i = 0; i < n; i++) {
    unsigned char c = (unsigned char)s[i];
    if (c == ':') {
      if (colon != n)
        return 0;
      colon = i;
    } else if (!((c >= '0' && c <= '9') ||
                 (lower(c) >= 'a' && lower(c) <= 'z') || c == '.' || c == '-'))
      return 0;
  }
  if (colon == 0 || (colon < n && colon + 1 == n))
    return 0;
  for (size_t i = colon + 1; i < n; i++)
    if (s[i] < '0' || s[i] > '9')
      return 0;
  return 1;
}
static int request_line(struct http_parser *p, char *s, size_t n) {
  char *a = memchr(s, ' ', n);
  if (!a)
    return fail(p, 400);
  size_t m = (size_t)(a - s);
  if (!m || m > 16)
    return fail(p, 501);
  for (size_t i = 0; i < m; i++)
    if (!token((unsigned char)s[i]))
      return fail(p, 400);
  char *b = memchr(a + 1, ' ', n - m - 1);
  if (!b || strcmp(b + 1, "HTTP/1.1"))
    return fail(p, 400);
  size_t t = (size_t)(b - a - 1);
  if (!t || t > 1024)
    return fail(p, t > 1024 ? 414 : 400);
  for (size_t i = 0; i < t; i++)
    if ((unsigned char)a[1 + i] <= 32 || (unsigned char)a[1 + i] >= 127 ||
        a[1 + i] == '#')
      return fail(p, 400);
  memcpy(p->request.method, s, m);
  p->request.method[m] = 0;
  p->request.head = !strcmp(p->request.method, "HEAD");
  memcpy(p->request.target, a + 1, t);
  p->request.target[t] = 0;
  if (!strncmp(p->request.target, "http://", 7)) {
    char *path = strchr(p->request.target + 7, '/');
    if (!path)
      return fail(p, 400);
    size_t h = (size_t)(path - p->request.target - 7);
    if (!authority_valid(p->request.target + 7, h) ||
        !equal(p->request.target + 7, h, p->configured_authority))
      return fail(p, 400);
    memcpy(p->request.authority, p->request.target + 7, h);
    p->request.authority[h] = 0;
    memmove(p->request.target, path, strlen(path) + 1);
    p->absolute = 1;
  } else if (p->request.target[0] != '/')
    return fail(p, !strcmp(p->request.method, "CONNECT") ? 501 : 400);
  p->state = HTTP_HEADERS;
  return HTTP_NEED;
}
static int coding(struct http_parser *p, const char *v, size_t n) {
  if (p->te++)
    return fail(p, 400);
  size_t i = 0;
  unsigned count = 0, chunk = 0;
  while (i < n) {
    while (i < n && (v[i] == ' ' || v[i] == '\t'))
      i++;
    size_t start = i;
    while (i < n && token((unsigned char)v[i]))
      i++;
    if (i == start || chunk)
      return fail(p, 400);
    /* A repeated unsupported coding is malformed, not merely unsupported. */
    for (size_t prior = 0; prior < start;) {
      while (prior < start &&
             (v[prior] == ' ' || v[prior] == '\t' || v[prior] == ','))
        prior++;
      size_t begin = prior;
      while (prior < start && token((unsigned char)v[prior]))
        prior++;
      size_t length = prior - begin;
      if (length == i - start) {
        size_t k = 0;
        while (k < length && lower((unsigned char)v[begin + k]) ==
                                 lower((unsigned char)v[start + k]))
          k++;
        if (k == length)
          return fail(p, 400);
      }
      if (prior == begin && prior < start)
        prior++;
    }
    count++;
    chunk = equal(v + start, i - start, "chunked");
    if (!chunk)
      p->te_unsupported = 1;
    while (i < n && (v[i] == ' ' || v[i] == '\t'))
      i++;
    if (i == n)
      break;
    if (v[i++] != ',' || i == n)
      return fail(p, 400);
  }
  if (!count || !chunk)
    return fail(p, 400);
  p->te_chunked = 1;
  p->te_count = count;
  return HTTP_NEED;
}
static int field(struct http_parser *p, char *s, size_t n, int trailer) {
  char *c = memchr(s, ':', n);
  if (!c || c == s)
    return fail(p, 400);
  size_t k = (size_t)(c - s);
  for (size_t i = 0; i < k; i++)
    if (!token((unsigned char)s[i]))
      return fail(p, 400);
  char *v = c + 1;
  size_t z = n - k - 1;
  for (size_t i = 0; i < z; i++)
    if (((unsigned char)v[i] < 32 && v[i] != '\t') ||
        (unsigned char)v[i] == 127)
      return fail(p, 400);
  while (z && (v[0] == ' ' || v[0] == '\t')) {
    v++;
    z--;
  }
  while (z && (v[z - 1] == ' ' || v[z - 1] == '\t'))
    z--;
  if (trailer)
    return HTTP_NEED; /* Validated but never merged into headers. */
  if (equal(s, k, "host")) {
    if (p->host++ || !authority_valid(v, z))
      return fail(p, 400);
    if (!p->absolute) {
      memcpy(p->request.authority, v, z);
      p->request.authority[z] = 0;
    }
  } else if (equal(s, k, "content-length")) {
    if (p->cl++ || !z)
      return fail(p, 400);
    size_t len = 0;
    for (size_t i = 0; i < z; i++) {
      if (v[i] < '0' || v[i] > '9' ||
          len > (SIZE_MAX - (size_t)(v[i] - '0')) / 10)
        return fail(p, 400);
      len = len * 10 + (size_t)(v[i] - '0');
    }
    p->remaining = len;
  } else if (equal(s, k, "transfer-encoding"))
    return coding(p, v, z);
  else if (equal(s, k, "expect")) {
    if (p->expect++ || !equal(v, z, "100-continue"))
      return fail(p, 417);
  } else if (equal(s, k, "upgrade"))
    p->upgrade = 1;
  else if (equal(s, k, "connection")) {
    size_t i = 0;
    while (i < z) {
      while (i < z && (v[i] == ' ' || v[i] == '\t'))
        i++;
      size_t a = i;
      while (i < z && token((unsigned char)v[i]))
        i++;
      if (a == i)
        return fail(p, 400);
      if (equal(v + a, i - a, "close"))
        p->request.close = 1;
      if (equal(v + a, i - a, "upgrade"))
        p->upgrade = 1;
      while (i < z && (v[i] == ' ' || v[i] == '\t'))
        i++;
      if (i < z && (v[i++] != ',' || i == z))
        return fail(p, 400);
    }
  }
  return HTTP_NEED;
}
static int route(struct http_request *r) {
  if (strcmp(r->method, "GET") && strcmp(r->method, "HEAD") &&
      strcmp(r->method, "POST"))
    return 501;
  if (!strcmp(r->target, "/health"))
    return !strcmp(r->method, "POST") ? 405 : 200;
  if (!strcmp(r->target, "/echo"))
    return strcmp(r->method, "POST") ? 405 : 200;
  return 404;
}
static int headers_done(struct http_parser *p) {
  if (p->host != 1 || !equal(p->request.authority, strlen(p->request.authority),
                             p->configured_authority))
    return fail(p, 400);
  if (p->te && p->cl)
    return fail(p, 400);
  if (p->remaining > 1024)
    return fail(p, 413);
  if (p->te_unsupported || p->upgrade)
    return fail(p, 501);
  p->request.status = route(&p->request);
  if (p->expect && p->request.status != 200)
    return fail(p, p->request.status);
  p->state = p->te ? HTTP_CHUNK : (p->remaining ? HTTP_BODY : HTTP_FINISHED);
  if (p->state == HTTP_FINISHED)
    return HTTP_DONE;
  return p->expect ? HTTP_CONTINUE : HTTP_NEED;
}
static int hex(unsigned char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  c = (unsigned char)lower(c);
  return c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
}
static int extensions(const char *s, size_t n) {
  size_t i = 0;
  while (i < n) {
    while (i < n && (s[i] == ' ' || s[i] == '\t'))
      i++;
    if (i == n || s[i++] != ';')
      return 0;
    while (i < n && (s[i] == ' ' || s[i] == '\t'))
      i++;
    size_t b = i;
    while (i < n && token((unsigned char)s[i]))
      i++;
    if (i == b)
      return 0;
    while (i < n && (s[i] == ' ' || s[i] == '\t'))
      i++;
    if (i < n && s[i] == '=') {
      i++;
      while (i < n && (s[i] == ' ' || s[i] == '\t'))
        i++;
      if (i < n && s[i] == '"') {
        i++;
        int closed = 0;
        while (i < n) {
          unsigned char c = (unsigned char)s[i++];
          if (c == '"') {
            closed = 1;
            break;
          }
          if (c == '\\') {
            if (i == n)
              return 0;
            c = (unsigned char)s[i++];
          }
          if ((c < 32 && c != '\t') || c == 127)
            return 0;
        }
        if (!closed)
          return 0;
      } else {
        b = i;
        while (i < n && token((unsigned char)s[i]))
          i++;
        if (i == b)
          return 0;
      }
    }
  }
  return 1;
}
static int line_done(struct http_parser *p) {
  char *s = p->line;
  size_t n = p->line_length;
  s[n] = 0;
  p->line_length = 0;
  if (p->state == HTTP_LINE)
    return request_line(p, s, n);
  if (p->state == HTTP_HEADERS) {
    if (!n)
      return headers_done(p);
    if (++p->fields > 32)
      return fail(p, 431);
    return field(p, s, n, 0);
  }
  if (p->state == HTTP_TRAILERS) {
    if (!n) {
      p->state = HTTP_FINISHED;
      return HTTP_DONE;
    }
    if (++p->trailers > 8)
      return fail(p, 400);
    return field(p, s, n, 1);
  }
  size_t i = 0, len = 0;
  int h;
  while (i < n && (h = hex((unsigned char)s[i])) >= 0) {
    if (len > (SIZE_MAX - (unsigned)h) / 16)
      return fail(p, 400);
    len = len * 16 + (unsigned)h;
    i++;
  }
  if (!i)
    return fail(p, 400);
  if (n - i > 128 || !extensions(s + i, n - i))
    return fail(p, 400);
  if (len > 1024 - p->request.body_length)
    return fail(p, 413);
  p->remaining = len;
  p->state = len ? HTTP_CHUNK_DATA : HTTP_TRAILERS;
  return HTTP_NEED;
}
int http_feed(struct http_parser *p, const void *v, size_t n, size_t *used) {
  const unsigned char *s = v;
  *used = 0;
  if (p->state == HTTP_FAILED)
    return HTTP_ERROR;
  if (p->state == HTTP_FINISHED)
    return HTTP_DONE;
  for (size_t i = 0; i < n; i++) {
    unsigned char c = s[i];
    *used = i + 1;
    if (p->state == HTTP_BODY || p->state == HTTP_CHUNK_DATA) {
      p->request.body[p->request.body_length++] = c;
      if (--p->remaining == 0) {
        if (p->state == HTTP_BODY) {
          p->state = HTTP_FINISHED;
          return HTTP_DONE;
        }
        p->state = HTTP_CHUNK_CR;
      }
      continue;
    }
    if (p->state == HTTP_CHUNK_CR) {
      if (c != '\r')
        return fail(p, 400);
      p->state = HTTP_CHUNK_LF;
      continue;
    }
    if (p->state == HTTP_CHUNK_LF) {
      if (c != '\n')
        return fail(p, 400);
      p->state = HTTP_CHUNK;
      continue;
    }
    if (p->state == HTTP_LINE || p->state == HTTP_HEADERS) {
      if (++p->header_bytes > 8192)
        return fail(p, 431);
    }
    if (p->state == HTTP_TRAILERS) {
      if (++p->trailer_bytes > 1024)
        return fail(p, 400);
    }
    if (p->saw_cr) {
      p->saw_cr = 0;
      if (c != '\n')
        return fail(p, 400);
      int r = line_done(p);
      if (r != HTTP_NEED)
        return r;
      continue;
    }
    if (c == '\r') {
      p->saw_cr = 1;
      continue;
    }
    if (c == '\n' || !c || (c < 32 && c != '\t') || c == 127)
      return fail(p, 400);
    if (p->line_length >= 8192)
      return fail(p, 431);
    p->line[p->line_length++] = (char)c;
  }
  return HTTP_NEED;
}
int http_eof(struct http_parser *p) {
  return p->state == HTTP_FINISHED ? HTTP_DONE : fail(p, 400);
}
static const char *reason(int n) {
  switch (n) {
  case 200:
    return "OK";
  case 400:
    return "Bad Request";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 408:
    return "Request Timeout";
  case 413:
    return "Content Too Large";
  case 414:
    return "URI Too Long";
  case 417:
    return "Expectation Failed";
  case 431:
    return "Request Header Fields Too Large";
  default:
    return "Not Implemented";
  }
}
static int append(unsigned char *o, size_t c, size_t *n, const void *v,
                  size_t z) {
  if (z > c - *n)
    return -1;
  memcpy(o + *n, v, z);
  *n += z;
  return 0;
}
static size_t decimal(size_t x, char *s) {
  char rev[32];
  size_t n = 0;
  do {
    rev[n++] = (char)('0' + x % 10);
    x /= 10;
  } while (x);
  for (size_t i = 0; i < n; i++)
    s[i] = rev[n - 1 - i];
  return n;
}
int http_response(const struct http_request *r, const char *date, int close, void *v,
                  size_t cap, size_t *length) {
  unsigned char *o = v;
  size_t n = 0;
  char num[32];
  const char *why = reason(r->status);
  const void *body;
  size_t z;
  if (strlen(date) != 29)
    return -1;
  for (size_t i = 0; i < 29; i++)
    if ((unsigned char)date[i] < 32 || (unsigned char)date[i] >= 127)
      return -1;
  if (r->status == 200) {
    if (!strcmp(r->target, "/health")) {
      body = "ok\n";
      z = 3;
    } else {
      body = r->body;
      z = r->body_length;
    }
  } else {
    body = why;
    z = strlen(why);
  }
#define ADD(s)                                                                 \
  do {                                                                         \
    if (append(o, cap, &n, (s), strlen(s)))                                    \
      return -1;                                                               \
  } while (0)
  ADD("HTTP/1.1 ");
  size_t k = decimal((size_t)r->status, num);
  if (append(o, cap, &n, num, k))
    return -1;
  ADD(" ");
  ADD(why);
  ADD("\r\nDate: ");
  ADD(date);
  ADD("\r\nContent-Length: ");
  k = decimal(z, num);
  if (append(o, cap, &n, num, k))
    return -1;
  ADD("\r\nContent-Type: application/octet-stream\r\n");
  if (r->status == 405) {
    ADD(!strcmp(r->target, "/health") ? "Allow: GET, HEAD\r\n"
                                      : "Allow: POST\r\n");
  }
  ADD(close || r->close ? "Connection: close\r\n\r\n"
                        : "Connection: keep-alive\r\n\r\n");
  if (!r->head && append(o, cap, &n, body, z))
    return -1;
  *length = n;
  return 0;
#undef ADD
}
int http_expired(uint64_t now, uint64_t start, uint64_t progress,
                 uint64_t total, uint64_t idle) {
  return now < start || now < progress || now - start >= total ||
         now - progress >= idle;
}
int http_drain(http_write_fn write, void *ctx, const void *v, size_t n,
               size_t *off) {
  if (*off > n)
    return -1;
  for (unsigned i = 0; i < 16 && *off < n; i++) {
    int r = write(ctx, (const unsigned char *)v + *off, n - *off);
    if (r == -1)
      continue;
    if (r == -2)
      return 0;
    if (r <= 0 || (size_t)r > n - *off)
      return -1;
    *off += (size_t)r;
  }
  return *off == n ? 1 : 0;
}
