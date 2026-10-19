"""Host-only Expat event oracle; not linked into the studied parser."""
import random
import subprocess
import xml.parsers.expat

def canonical(events):
    out=[]
    for kind,depth,data in events:
        if kind==2 and out and out[-1][0]==2 and out[-1][1]==depth:
            out[-1]=(kind,depth,out[-1][2]+data)
        else:out.append((kind,depth,data))
    return out

def expected(document):
    p=xml.parsers.expat.ParserCreate();depth=0;out=[]
    def start(name,attrs):
        nonlocal depth
        assert not attrs;depth+=1;out.append((1,depth,name.encode()))
    def end(name):
        nonlocal depth
        out.append((3,depth,name.encode()));depth-=1
    def text(value):
        if depth:out.append((2,depth,value.encode()))
    p.StartElementHandler=start;p.EndElementHandler=end;p.CharacterDataHandler=text
    p.Parse(document,True);out.append((4,0,b''));return canonical(out)

def run(driver):
    rng=random.Random(15019);documents=[]
    def tree(depth):
        name=rng.choice(['a','b_1','z-2']);text=rng.choice(['','text','a&amp;b','&lt;&gt;&quot;&apos;',' \n\t','012345678901234567890'])
        if depth==0 or rng.randrange(4)==0:return '<'+name+'/>'
        children=''.join(tree(depth-1) for _ in range(rng.randrange(3)))
        return '<'+name+'>'+text+children+text+'</'+name+'>'
    for _ in range(1024):documents.append(tree(rng.randrange(1,5)))
    rows=[d.encode().hex()+' '+str(rng.randrange(1,29))+' '+str(i%2) for i,d in enumerate(documents)]
    p=subprocess.run([str(driver),'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True,check=True)
    lines=p.stdout.splitlines();assert len(lines)==len(documents)
    for document,line in zip(documents,lines):
        words=line.split();assert int(words[0])==2,(document,line)
        actual=[]
        for word in words[1:]:
            k,d,b=word.split(',');actual.append((int(k),int(d),bytes.fromhex(b)))
        assert canonical(actual)==expected(document),(document,actual,expected(document))
    return len(documents)
