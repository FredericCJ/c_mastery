import hashlib,hmac,struct,sys
from pathlib import Path
rows=Path(sys.argv[1]).read_text().splitlines();message=b'SXREDO1\0'+struct.pack('<QIIII',7,1,10,1^10^0x93c72a5d,1)
key=bytes([1,2,3,4])+bytes(28)
assert rows[-3]==message.hex() and rows[-2]==hmac.new(key,message,hashlib.sha256).hexdigest()
print('AUTH_ORACLE_PASS canonical_message_and_actual_hmac=1')
