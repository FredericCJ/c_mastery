"""A declared lexical branch indicator; not a cyclomatic correctness oracle."""
import json,pathlib,re,sys
text=pathlib.Path(sys.argv[1]).read_text();results={}
for name in ['lc_construct','lc_construct_nested','lc_construct_helpers','acquire_step','lc_destroy']:
 match=re.search(r'\b'+name+r'\([^;]+?\)\s*\{',text);assert match,name
 depth=1;i=match.end();start=i
 while depth:
  depth+=(text[i]=='{')-(text[i]=='}');i+=1
 body=text[start:i-1];tokens=re.findall(r'\b(?:if|for|while|case)\b|&&|\|\||\?',body)
 results[name]={'lexical_decisions':len(tokens),'indicator_plus_one':1+len(tokens)}
assert results['lc_construct_helpers']['lexical_decisions']<results['lc_construct_nested']['lexical_decisions']
print(json.dumps(dict(definition='count if/for/while/case/&&/||/? in selected original function bodies plus1; not a full CFG cyclomatic implementation',functions=results,resource_states='prefix empty,1,1+2,1+2+3 or empty,1,1+3; failure prefixes retire in reverse',helper_obligation_retained=True)))
