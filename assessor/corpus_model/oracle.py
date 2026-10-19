"""Independent recursive graph/value model and actual persistent process traces."""
import argparse,json,pathlib,random,subprocess
LOW,HIGH=-(2**31),2**31-1
def checked(x):
    if not LOW<=x<=HIGH:raise OverflowError
    return x
def apply(model,revision,edits):
    if len({e[0] for e in edits})!=len(edits):return 6,model,revision
    stage=dict(model)
    for identity,kind,a,b,w0,w1,bias in edits:stage[identity]=(kind,a,b,w0,w1,bias)
    values={};visiting=set()
    def visit(i):
        if i in values:return values[i]
        if i in visiting:raise RuntimeError
        if i not in stage:raise KeyError
        kind,a,b,w0,w1,bias=stage[i];visiting.add(i)
        # C excludes missing references before topological arithmetic. All
        # generated error probes isolate one cause, avoiding invented precedence.
        value=bias if kind==1 else checked(checked(checked(visit(a)*w0)+checked(visit(b)*w1))+bias)
        visiting.remove(i);values[i]=value;return value
    try:
        for i in sorted(stage):visit(i)
    except KeyError:return 4,model,revision
    except RuntimeError:return 5,model,revision
    except OverflowError:return 3,model,revision
    return 0,stage,revision+bool(edits)
def values(model):
    def visit(i):
        kind,a,b,w0,w1,bias=model[i]
        return bias if kind==1 else visit(a)*w0+visit(b)*w1+bias
    return [(model[i][0],visit(i)) if i in model else (0,0) for i in range(16)]
def model_line(rc,model,revision):
    return 'M '+str(rc)+' '+str(revision)+''.join(f' {kind}:{value}' for kind,value in values(model))
def text(edits,csv):
    lines=[]
    for i,kind,a,b,w0,w1,bias in edits:
        fields=[('I' if csv else 'input'),i,bias] if kind==1 else [('L' if csv else 'linear'),i,a,w0,b,w1,bias]
        lines.append((',' if csv else ' ').join(map(str,fields)))
    return '\n'.join(lines)
def run(driver):
    rng=random.Random(812021);commands=[];expected=[];model={};revision=0;graphs=0;transactions=0;expressions=0
    def reset():
        nonlocal model,revision
        model={};revision=0;commands.append('R\n');expected.append(model_line(0,model,revision))
    def batch(edits,mode='A'):
        nonlocal model,revision,transactions
        if mode=='A':commands.append('A '+str(len(edits))+'\n'+''.join(' '.join(map(str,e))+'\n' for e in edits))
        else:
            config=text(edits,mode=='C');commands.append(f'{mode} {len(config)}\n{config}\n')
        rc,model,revision=apply(model,revision,edits);expected.append(model_line(rc,model,revision));transactions+=1
    def expression(expr,result,rc=0):
        nonlocal expressions
        commands.append(f'E {len(expr)}\n{expr}\n');expected.append(f'E {rc} {result if rc==0 else 12345}');expressions+=1
    for case in range(240):
        count=rng.randrange(3,17)
        edits=[(0,1,0,0,0,0,rng.randrange(-9,10)),(1,1,0,0,0,0,rng.randrange(-9,10))]
        for i in range(2,count):edits.append((i,2,rng.randrange(i),rng.randrange(i),rng.randrange(-2,3),rng.randrange(-2,3),rng.randrange(-5,6)))
        rng.shuffle(edits)
        for mode in 'ALC':
            reset();batch(edits,mode);graphs+=1
            snapshot=dict(model)
            batch([(0,1,0,0,0,0,case%15-7)])
            batch([(0,2,0,0,1,0,0)]) # isolated self-cycle, irrespective of zero coefficient
            assert model!=snapshot or case%15-7==snapshot[0][-1]
            batch([(1,1,0,0,0,0,-5),(1,1,0,0,0,0,7)])
            val=values(model)[count-1][1]
            expression(f'(${count-1}+2)*3',(val+2)*3)
            expression('$0^2',model[0][-1]**2)
    # Independent checked arithmetic versus rejected intermediate cancellation.
    for value in [LOW,LOW+1,-3,-1,0,1,3,HIGH-1,HIGH]:
        reset();batch([(0,1,0,0,0,0,value),(1,1,0,0,0,0,1)])
        expression('$0',value)
        expression('-$0',-value,3 if value==LOW else 0)
        for weight in [-2,-1,0,1,2]:
            batch([(2,2,0,1,weight,0,0)])
    reset();batch([(0,1,0,0,0,0,HIGH),(1,1,0,0,0,0,-HIGH)])
    batch([(2,2,0,1,2,2,0)]) # products overflow even though exact final sum is zero
    batch([(2,2,0,15,0,0,0)]) # missing dependency despite coefficient0
    expression('2^3^2',512);expression('2^-1',0,2);expression('$15',0,4)
    for expr,status in [('$',2),('$16',3),('2147483648',3),('1+*2',2),('1\x002',2),('('*65+'1'+')'*65,3),('1+'*32+'1',3),('$0'*70,3)]:
        expression(expr,0,status)
    result=subprocess.run([driver],input=''.join(commands),text=True,capture_output=True,timeout=90)
    assert result.returncode==0,result.stderr
    actual=result.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected),result.stderr)
    for i,(a,e) in enumerate(zip(actual,expected)):
        assert a==e,(i,a,e)
    # Literal oracle transcript models command effects independently. It does
    # not call the C parser or derive its expected output from a reference run.
    script='input 0 -3\neval $0^2\ninput 1 4\nlinear 2 0 2 1 3 5\nget 2\nset 0 7\nget 2\nlinear 0 2 1 1 0 0\nget 0\neval -2147483648\nget 15\n'+'x'*257+'\nget 2'
    output='OK\nVALUE 9\nOK\nOK\nVALUE 11\nOK\nVALUE 31\nERR 5\nVALUE 7\nERR 3\nERR 4\nERR 3\nVALUE 31\n'
    directory=pathlib.Path(driver).parent;session=directory/'model_session_driver';repl=directory/'model_repl';streams=0
    configurations={(1,1,p) for p in range(len(output))}
    configurations|={(chunk,window,8192) for chunk in range(1,len(script)+1) for window in [7]}
    configurations|={(chunk,window,len(output)//2) for chunk in [1,2,7,31,256,1024] for window in [1,2,7,96]}
    for chunk,window,pause in sorted(configurations):
        result=subprocess.run([session,str(chunk),str(window),str(pause)],input=script,text=True,capture_output=True,timeout=10)
        assert result.returncode==0,(chunk,window,pause,result.returncode,result.stderr)
        assert result.stdout==output,(chunk,window,pause,result.stdout,output)
        parts=result.stderr.split();assert parts[:3]==['SESSION','4','1'] and int(parts[3])==len(script),result.stderr
        if pause<len(output):assert parts[4]=='1',result.stderr
        streams+=1
    for source,target,revision in [(script,output,4),('input 0 2\nexit\nset 0 9\n','OK\nBYE\n',1),('', '',0),('x'*257,'ERR 3\n',0),('input 0 5','OK\n',1)]:
        result=subprocess.run([repl],input=source,text=True,capture_output=True,timeout=10)
        assert result.returncode==0 and result.stdout==target,(result.returncode,result.stdout,target,result.stderr)
    # Real hosted fwrite/fflush failure, separate from resumable prefix callbacks.
    # /dev/full is an existing Linux error device; no host settings are changed.
    with open('/dev/full','wb') as failed_sink:
        failed=subprocess.run([repl],input='input 0 5\nget 0\n',text=True,stdout=failed_sink,stderr=subprocess.PIPE,timeout=10)
    assert failed.returncode==3,(failed.returncode,failed.stderr)
    return dict(passed=True,executed=True,graphs=graphs,transactions=transactions,expression_checks=expressions,model_rows=len(expected),stream_cases=streams,output_failure_prefixes=len(output),actual_stdio_traces=5,actual_stdio_print_failures=1,independent_strategy='Recursive dependency evaluation and explicit mathematical/transcript expectations; no C scan algorithm or parser output reused as oracle.')
def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
    fixture=pathlib.Path(__file__).resolve().parents[2]/'fixtures/corpus_model/cases.json'
    f=json.loads(fixture.read_text());assert f['expected_slot2']==-18 and f['expected_negative_reference_power']==9
    result=run(a.driver) if a.driver else dict(passed=True,executed=False,fixture='Independent hand-calculated graph and grouping checks')
    print(json.dumps(result));return 0
if __name__=='__main__':raise SystemExit(main())
