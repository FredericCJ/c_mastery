"""Independent sequence, arithmetic, document and immutable-template models."""
import argparse,itertools,json,subprocess

def hx(s):return s.encode().hex() or '-'
def recovered(source,newline,budget):
    if len(source)>512:return '3'
    if budget<len(source)+1:return '4'
    known={'1':(0,1),'22':(0,22),'3*4':(0,12),'-2^2':(0,-4),'2^3^2':(0,512),'':(1,None),'1+':(1,None),'(':(1,None),'?':(1,None),'2147483647+1':(3,None),'2^-1':(8,None)}
    valid=[];bad=[];begin=0
    for i in range(len(source)+1):
        if i<len(source) and source[i]!=';' and not(newline and source[i]=='\n'):continue
        part=source[begin:i];trim=part.strip(' \t\n');start=begin+len(part)-len(part.lstrip(' \t\n'))
        if not trim and i==len(source):break
        status,value=known.get(trim,(1,None)) if i<len(source) else (105,None)
        if len(trim)>256:status,value=3,None
        if status:bad.append(f'{start}:{i}:{status}')
        else:valid.append(f'{value}:{start}')
        begin=i+1
    if len(valid)>16 or len(bad)>16:return '3'
    return ' '.join(map(str,[int(bool(bad)),len(valid),len(bad),*valid,*bad]))

class TemplateError(Exception):
    def __init__(self,status):self.status=status

def parse_template(source):
    def block(pos,nested=False):
        nodes=[]
        while pos<len(source):
            if source.startswith('{{',pos) or source.startswith('}}',pos):nodes.append(('text',source[pos]));pos+=2
            elif source.startswith('{/}',pos):
                if not nested:raise TemplateError(5)
                return nodes,pos+3
            elif source[pos] not in '{}':nodes.append(('text',source[pos]));pos+=1
            elif source[pos]=='{' and '}' in source[pos:]:
                end=source.index('}',pos);word=source[pos+1:end];pos=end+1
                if word in ['a','b','c','d','.']:nodes.append(('value',word))
                elif len(word)==2 and word[0] in '?*' and word[1] in 'abcd':
                    children,pos=block(pos,True);nodes.append((word[0],word[1],children))
                elif len(word)==2 and word[0]=='>' and word[1] in '0123':nodes.append(('include',int(word[1])))
                else:raise TemplateError(5)
            else:raise TemplateError(5)
        if nested:raise TemplateError(5)
        return nodes,pos
    return block(0)[0]

def template(source,include,values,presence,html):
    try:
        bank=[parse_template(source),parse_template(include)];model={'a':values,'b':['B'] if presence else [],'c':[],'d':[]}
        def escape(s):
            if not html:return s
            # One replacement per original code point, not repeated entity rewriting.
            table={'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}
            return ''.join(table.get(c,c) for c in s)
        def walk(nodes,item,active):
            parts=[]
            for node in nodes:
                if node[0]=='text':parts.append(node[1])
                elif node[0]=='value':parts.append(escape(item if node[1]=='.' else next(iter(model[node[1]]),'')))
                elif node[0]=='?':
                    if model[node[1]]:parts.append(walk(node[2],item,active))
                elif node[0]=='*':parts.extend(walk(node[2],v,active) for v in model[node[1]])
                else:
                    if node[1]>=len(bank):raise TemplateError(2)
                    if node[1] in active:raise TemplateError(6)
                    parts.append(walk(bank[node[1]],item,active|{node[1]}))
            return ''.join(parts)
        text=walk(bank[0],'',{0})
        if len(text)>4096:return '3 0 -'
        return f'0 {len(text)} {hx(text)}'
    except TemplateError as e:return f'{e.status} 0 -'

def cases():
    result=[]
    fragments=['1','22','3*4','-2^2','2^3^2','','1+','(','?','2147483647+1','2^-1']
    for parts in itertools.product(fragments,repeat=3):
        for newline in [0,1]:
            separator='\n' if newline else ';';source=separator.join(parts)+separator
            result.append((f'R {newline} 10000 {hx(source)}',recovered(source,newline,10000),'recovery'))
    for source in ['1\n22;','1;22','1;'*17,';'*17,'9'*257+';1;',' \t']:
        for mode in [0,1]:
            for fuel in [0,len(source),len(source)+1]:result.append((f'R {mode} {fuel} {hx(source)}',recovered(source,mode,fuel),'recovery'))
    for a,b,tail,indent,width in itertools.product([1,3,7],[1,4,9],[0,2,5],[0,2,5],range(25)):
        flat=a+1+b+tail<=width
        text='a'*a+(' ' if flat else '\n'+' '*indent)+'b'*b+'z'*tail
        result.append((f'L {width} 10000 {a} {b} {tail} {indent}',f'0 {len(text)} {hx(text)}','layout'))
    sources=['{a}','{.}','[{*a}{.},{/}]','{?a}A{a}{/}{?b}B{/}','{*a}{>1}{/}','{*a}{.}{*b}{.}{/}{.}{/}','{{a}}','{?c}{>0}{/}','{>0}','{>3}','{?a}','{/}','{a*2}','{*a}{*a}{.}{/}{/}']
    valuesets=[[],[''],['A'],['<tag>&','B'],['\"','\'','{>0}','&amp;']]
    for source,include,values,presence,html in itertools.product(sources,['{.}','{?b}{b}{/}','{>0}'],valuesets,[0,1],[0,1]):
        command=f'T {html} 100000 {hx(source)} {hx(include)} {len(values)} {presence} '+' '.join(map(hx,values))
        result.append((command.strip(),template(source,include,values,presence,html),'template'))
    return result

def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args();rows=cases()
    if a.driver:
        r=subprocess.run([a.driver],input='\n'.join(x[0] for x in rows)+'\n',text=True,capture_output=True,timeout=120)
        assert r.returncode==0,(r.returncode,r.stderr)
        actual=r.stdout.splitlines();assert len(actual)==len(rows),(len(actual),len(rows))
        for got,(command,want,kind) in zip(actual,rows):assert got==want,(kind,command,want,got)
    counts={kind:sum(x[2]==kind for x in rows) for kind in ['recovery','layout','template']}
    print(json.dumps(dict(passed=True,cases=len(rows),counts=counts,limits='Finite independent models; no learner mastery or universal layout/escaping safety.')))
if __name__=='__main__':main()
