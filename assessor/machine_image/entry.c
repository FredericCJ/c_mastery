long mi_entry(const unsigned long *sp) {
  if (sp[0] != 3)
    return 91;
  const char *a = (const char *)sp[2], *b = (const char *)sp[3];
  return a[0] == 'o' && a[1] == 'k' && a[2] == 0 && b[0] == '4' &&
                 b[1] == '2' && b[2] == 0
             ? 42
             : 92;
}
