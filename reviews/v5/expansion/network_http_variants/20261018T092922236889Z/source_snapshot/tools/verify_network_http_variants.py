"""Independent unbounded-integer and datetime expected values; fixed seed."""
import datetime,hashlib,json,random,subprocess,sys
def main():
    rng=random.Random(0x4142);rows=[];expected=[];counts={'length':0,'date':0,'checksum':0}
    def add(row,want,kind):rows.append(row);expected.append(want);counts[kind]+=1
    for maximum in list(range(20))+[255,256,1023,1024,65535,2**32-1]:
        for value in list(range(20))+[maximum,max(0,maximum-1),maximum+1,1024,1025,2**32-1,2**32]:
            for text in [str(value),'00'+str(value),str(value)+'x']:
                for cap in [0,7,1024]:
                    rc=400 if not text.isdecimal() or value>maximum else 413 if value>cap else 0
                    add(f'L {maximum} {cap} {text}',f'{rc} {77 if rc else value}','length')
    base=datetime.date(1970,1,1);week=['Mon','Tue','Wed','Thu','Fri','Sat','Sun'];months=['Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec']
    days=set([0,2932896,2932897,2**32-1]+[rng.randrange(2932897) for _ in range(2048)])
    for y in [1972,2000,2100,2400,9999]:
        for month in [1,2,3,12]:
            d=(datetime.date(y,month,1)-base).days;days.update([d-1,d,d+1])
    for d in sorted(days):
        for s in [0,86399,86400]:
            if d>2932896 or s>86399:want='1 77 !'
            else:
                date=base+datetime.timedelta(days=d);want=f'0 29 {week[date.weekday()]}, {date.day:02} {months[date.month-1]} {date.year:04} {s//3600:02}:{s//60%60:02}:{s%60:02} GMT'
            add(f'D {d} {s}',want,'date')
    for i in range(4096):
        data=bytes(rng.randrange(256) for _ in range(rng.randrange(129)));a=rng.randrange(len(data)+1);b=rng.randrange(a,len(data)+1)
        padded=data+b'\0'*(len(data)%2);total=sum(int.from_bytes(padded[j:j+2],'big') for j in range(0,len(padded),2))
        while total>65535:total=(total&65535)+(total>>16)
        add(f'S {len(data)} {a} {b} '+' '.join(map(str,data)),f'0 {total^65535} {len(data)%2}','checksum')
    p=subprocess.run([sys.argv[1]],input='\n'.join(rows)+'\n',capture_output=True,text=True,check=True)
    actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for row,want,got in zip(rows,expected,actual):assert want==got,(row,want,got)
    print(json.dumps({'passed':True,'seed':0x4142,'cases':counts,'trace_sha256':hashlib.sha256(('\n'.join(rows)+'\n').encode()).hexdigest()}))
if __name__=='__main__':main()
