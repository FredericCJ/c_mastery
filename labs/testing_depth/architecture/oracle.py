"""Independent Python list semantics; never reuses C ring indices."""
import io
def check(text):
 lines=iter(io.StringIO(text));rows=0
 for layout in range(4):
  for word in range(16384):
   values=[];code=word;accepted=removed=0
   for step in range(7):
    op=code%4;code//=4;out=99
    if op<2:
     status=1 if len(values)==2 else 0
     if status==0:values.append(7+op);accepted+=1
    else:
     status=0 if values else 2
     if values:
      out=values[0]
      if op==2:del values[0];removed+=1
    expected=[layout,word,step,op,status,out,len(values),accepted,removed,1]
    line=next(lines,None)
    if line is None or list(map(int,line.split()))!=expected:raise ValueError(f'Independent FIFO disagreement at {layout}/{word}/{step}')
    if accepted!=removed+len(values):raise ValueError('Oracle conservation error')
    rows+=1
 if next(lines,None) is not None:raise ValueError('Unexpected trailing observation')
 return {'histories':65536,'operations':rows,'layouts':4,'word_length':7,'oracle':'Python list append/front deletion; independent of component indices','proof_limit':'All selected finite words only; not all C executions'}
