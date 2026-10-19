"""Independent interval/dictionary oracle, never invokes the C algorithm for expectations."""
import argparse,itertools,random,subprocess

def checksum(data):
    words=[int.from_bytes(data[i:i+2].ljust(2,b'\0'),'big') for i in range(0,len(data),2)]
    total=sum(words)
    # Modular arithmetic distinguishes the all-zero representation.
    folded=0 if total==0 else 1+(total-1)%65535
    return 65535-folded

def header(payload,ident,protocol):
    raw=bytearray.fromhex('450000000000000040010000c0000201c0000202')
    raw[2:4]=(20+len(payload)).to_bytes(2,'big');raw[4:6]=ident.to_bytes(2,'big');raw[9]=protocol
    raw[10:12]=checksum(raw).to_bytes(2,'big');return bytes(raw)+payload

class Model:
    def step(self,line):
        words=line.split();act=words[0];v=list(map(int,words[1:]));index=99;out=None;rc=0
        if act=='I':
            self.count,self.policy,self.last,self.timeout=v;self.slots=[None]*self.count
        elif act=='T':
            now=v[0]
            if now<self.last:rc=2
            else:self.tick(now)
        elif act=='D':
            index=v[0]
            if index>=self.count:rc=2
            elif self.slots[index] is None or self.slots[index]['state']!=2:rc=1
            else:
                s=self.slots[index];out=header(bytes(s['bytes'][j] for j in range(s['total'])),*s['key']);s['state']=3
        else:
            ident,proto,offset,length,more,now,pattern=v;end=offset+length
            invalid=now<self.last or now>(1<<64)-1-self.timeout or end>1480 or ((offset or more) and (not length or (more and length%8)))
            if invalid:rc=2
            else:
                self.tick(now);key=(ident,proto);chosen=next((i for i,s in enumerate(self.slots) if s is not None and s['key']==key),None)
                if chosen is None:
                    chosen=next((i for i,s in enumerate(self.slots) if s is None),None)
                    if chosen is not None:self.slots[chosen]={'key':key,'state':1,'expiry':now+self.timeout,'bytes':{},'ranges':[],'final':None,'total':0,'first':False}
                if chosen is None:rc=3
                else:
                    s=self.slots[chosen]
                    if s['state']==3:rc=4
                    elif s['state']==2:rc=3
                    else:
                        data={offset+j:(offset+j+pattern)%256 for j in range(length)}
                        conflict=(s['final'] is not None and (end>s['final'] or (not more and end!=s['final']) or (more and end>=s['final'])))
                        conflict|=not more and any((b>end or (m and b>=end)) for a,b,m in s['ranges'])
                        if not more and end==0:conflict=True
                        common=s['bytes'].keys()&data.keys()
                        conflict|=bool(common) and (self.policy==0 or any(s['bytes'][k]!=data[k] for k in common))
                        if conflict:s['state']=3;rc=4
                        else:
                            s['bytes'].update(data);s['ranges'].append((offset,end,more));s['first']|=offset==0
                            if not more:s['final']=end;s['total']=end
                            # Set equality uses byte positions, not bitmap counters.
                            complete=s['first'] and s['final'] is not None and set(s['bytes'])==set(range(s['final']))
                            if complete:s['state']=2
                            index=chosen;rc=0 if complete else 1
        text=f'{act} {rc} {index}'
        for s in self.slots:
            text+=' 0' if s is None else f" {s['state']}:{len(s['bytes'])}:{s['total']}:{s['expiry']}"
        if out is not None:text+=' bytes='+out.hex()
        return text
    def tick(self,now):
        self.slots=[None if s and s['state'] in (1,3) and now>=s['expiry'] else s for s in self.slots];self.last=now

def scenarios():
    for count in range(2,6):
        for order in itertools.permutations(range(count)):
            for policy in (0,1):
                yield 'I 2 %d 0 60000'%policy
                for j in order:yield f'F 7 1 {j*8} {3 if j==count-1 else 8} {int(j<count-1)} 0 0'
                yield 'D 0';yield 'T 60000'
    rng=random.Random(0x14105)
    for case in range(600):
        now=[0,(1<<32)-1,(1<<64)-130000][case%3];timeout=[1,60000,120000][case%3]
        yield f'I {1+case%2} {case%2} {now} {timeout}'
        for j in range(32):
            choice=rng.randrange(12)
            if choice==0:yield f'D {rng.randrange(3)}'
            elif choice==1:now=min((1<<64)-1,now+timeout);yield f'T {now}'
            elif choice==2:yield f'T {max(0,now-1)}'
            else:
                offset=rng.choice([0,8,16,24,1472,1480]);length=rng.choice([0,1,3,8,16]);more=rng.randrange(2)
                yield f'F {rng.randrange(3)} {rng.choice([1,6])} {offset} {length} {more} {now} {rng.choice([0,0,1])}'
    for length in (1,7,8,9,555,556,557,1479,1480):
        yield 'I 2 1 0 60000'
        chunks=[(i,min(8,length-i)) for i in range(0,length,8)]
        for i,n in reversed(chunks):yield f'F 21 6 {i} {n} {int(i+n<length)} 1 0'
        yield 'D 0'

def main():
    p=argparse.ArgumentParser();p.add_argument('probe');p.add_argument('--trace',required=True);a=p.parse_args()
    lines=list(scenarios());m=Model();expected=[m.step(s) for s in lines]
    result=subprocess.run([a.probe],input='\n'.join(lines)+'\n',text=True,capture_output=True,check=True)
    from pathlib import Path
    Path(a.trace).write_text(result.stdout)
    observed=result.stdout.splitlines();assert len(observed)==len(expected)
    for i,(x,y) in enumerate(zip(expected,observed)):
        assert x==y,(i,lines[max(0,i-4):i+1],x,y)
    print(f'NI_ORACLE_PASS events={len(lines)} seed=0x14105 permutations=304 random_histories=600')
if __name__=='__main__':main()
