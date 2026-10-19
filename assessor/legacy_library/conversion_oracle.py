"""Independent regex grammar and Python unbounded-integer expected values."""
import ctypes as C,itertools,json,re,sys
lib=C.CDLL(sys.argv[1]);assert lib.lc_uintmax_bits()==64
lib.lc_i8.argtypes=[C.c_char_p,C.c_size_t,C.c_int,C.POINTER(C.c_int32)];lib.lc_unsigned.argtypes=[C.c_char_p,C.c_size_t,C.c_int,C.c_uint64,C.POINTER(C.c_uint64)]
tokens=[bytes(t) for n in range(6) for t in itertools.product(b'01+- x',repeat=n)]+[str(n).encode() for n in (-129,-128,127,128,255,256,2**32-1,2**32,2**64-1,2**64,2**128)]+[b'010',b'0x10',b'0Xf',b'0b10',b'08',b'0'*128,b'0'*129,b'1\x002']
counts={'signed':0,'unsigned':0,'syntax':0,'range':0,'too_long':0}
for token in tokens:
 for plus in (0,1):
  out=C.c_int32(77);pattern=rb'[+-]?[0-9]+' if plus else rb'-?[0-9]+'
  status=4 if len(token)>128 else 1 if not re.fullmatch(pattern,token) else 2 if not -128<=int(token)<=127 else 0
  actual=lib.lc_i8(token,len(token),plus,C.byref(out));assert actual==status and out.value==(int(token) if status==0 else 77),(token,plus,actual,status,out.value);counts['signed']+=1
 for prefixes in (0,1):
  for limit in (255,2**64-1):
   out=C.c_uint64(77);pattern=rb'\+?(?:0[xX][0-9a-fA-F]+|0[0-7]*|[1-9][0-9]*)' if prefixes else rb'\+?[0-9]+'
   if len(token)>128:status=4
   elif not re.fullmatch(pattern,token):status=1
   else:
    digits=token.lstrip(b'+');base=16 if prefixes and digits[:2].lower()==b'0x' else 8 if prefixes and digits.startswith(b'0') else 10;value=int(digits,base);status=2 if value>limit else 0
   actual=lib.lc_unsigned(token,len(token),prefixes,limit,C.byref(out));assert actual==status and out.value==(value if status==0 else 77),(token,prefixes,limit,actual,status,out.value);counts['unsigned']+=1
   if status in (1,2,4):counts[{1:'syntax',2:'range',4:'too_long'}[status]]+=1
print(json.dumps({'passed':True,'tokens':len(tokens),'counts':counts,'model':'Linux uintmax_t64 confirmed by compiled query; independent Python regex/int; no undefined atoi overflow execution'},sort_keys=True))
