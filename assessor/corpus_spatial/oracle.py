"""Independent linear geometry oracle: no tree, hash or target implementation import."""
import json
import subprocess
import sys

def overlap(a,b,d):
    return all(a[k]<=b[k+3] and b[k]<=a[k+3] for k in range(d))

def main():
    rows=0
    for kind in range(1,6):
        data=subprocess.run([sys.argv[1],str(kind),'dump'],capture_output=True,text=True,check=True).stdout
        items={};dim=0
        for line in data.splitlines():
            p=line.split()
            if not p:continue
            if p[0]=='S':items={};dim=int(p[2])
            elif p[0]=='I':items[int(p[1])]=list(map(int,p[2:]))
            elif p[0]=='Q':
                q=list(map(int,p[1:7]));expected=sum(1<<i for i,b in items.items() if overlap(b,q,dim))
                assert expected==int(p[7]),line;rows+=1
            elif p[0]=='N':
                point=list(map(int,p[1:4]));weights=list(map(int,p[4:7]))
                distance=lambda b:sum(weights[k]*max(b[k]-point[k],0,point[k]-b[k+3])**2 for k in range(dim))
                best=min((distance(b),i) for i,b in items.items());assert best==(int(p[8]),int(p[7])),line;rows+=1
            elif p[0]=='P':
                i=int(p[1]);expected=sum(1<<j for j,b in items.items() if i in items and j>i and overlap(items[i],b,dim))
                assert expected==int(p[2]),line;rows+=1
    print(json.dumps(dict(passed=True,independent_queries_and_pairs=rows)))
if __name__=='__main__':main()
