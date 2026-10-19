"""Uncached homogeneous matrices and independent visibility graph shortest paths."""
import json
import math
import subprocess
import sys
from fractions import Fraction

def cross(a,b,c):return (b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0])
def interval(a,b,polygon):
    lo,hi=Fraction(0),Fraction(1)
    for p,q in zip(polygon,polygon[1:]+polygon[:1]):
        x=cross(p,q,a);d=cross(p,q,b)-x
        if not d:
            if x<0:return None
        elif d>0:lo=max(lo,Fraction(-x,d))
        else:hi=min(hi,Fraction(-x,d))
    return (lo,hi) if lo<=hi else None
def visible(a,b,polygons):
    ranges=sorted(t for p in polygons if (t:=interval(a,b,p)) is not None)
    end=Fraction(0)
    for lo,hi in ranges:
        if lo>end:return False
        end=max(end,hi)
    return end>=1
def shortest(start,end,polygons):
    points=list(dict.fromkeys([start,end]+[v for p in polygons for v in p]));n=len(points)
    dist=[math.inf]*n;dist[0]=0;done=set()
    for _ in points:
        i=min((i for i in range(n) if i not in done),key=lambda i:dist[i]);done.add(i)
        for j in range(n):
            if j not in done and visible(points[i],points[j],polygons):dist[j]=min(dist[j],dist[i]+math.dist(points[i],points[j]))
    return dist[points.index(end)]
def run(driver):
    scene_count=0;nav_count=0;locals={};parents={}
    text=subprocess.check_output([driver,'1','dump'],text=True)
    def world(i):
        local=locals[i]+[[0,0,0,1]]
        if parents[i]==16:return local
        parent=world(parents[i]);return [[sum(parent[r][k]*local[k][c] for k in range(4)) for c in range(4)] for r in range(4)]
    for line in text.splitlines():
        r=line.split()
        if r[0]=='L':
            nums=list(map(int,r[3:]));locals[int(r[1])]=[nums[i:i+4] for i in range(0,12,4)];parents[int(r[1])]=int(r[2])
        elif r[0]=='W':
            expected=[x for row in world(int(r[1]))[:3] for x in row];assert expected==list(map(int,r[2:14])),line;scene_count+=1
    text=subprocess.check_output([driver,'2','dump'],text=True);polygons=[]
    for line in text.splitlines():
        r=line.split()
        if r[0]=='N':start=tuple(map(int,r[4:6]));end=tuple(map(int,r[6:8]));polygons=[]
        elif r[0]=='P':nums=list(map(int,r[2:]));polygons.append([tuple(nums[i:i+2]) for i in range(0,len(nums),2)])
        elif r[0]=='R':
            nums=list(map(int,r[2:]));path=[tuple(nums[i:i+2]) for i in range(0,len(nums),2)]
            assert path[0]==start and path[-1]==end
            assert all(visible(a,b,polygons) for a,b in zip(path,path[1:])),(polygons,path)
            length=sum(math.dist(a,b) for a,b in zip(path,path[1:]));best=shortest(start,end,polygons)
            assert math.isclose(length,best,rel_tol=1e-12,abs_tol=1e-10),(polygons,path,length,best)
            nav_count+=1
    assert scene_count==4096 and nav_count==1280
    return dict(passed=True,uncached_matrices=scene_count,independent_visibility_paths=nav_count,limits='Exact rational containment; host floating distance comparison tolerance1e-10, not a formal real-number proof.')
if __name__=='__main__':print(json.dumps(run(sys.argv[1])))
