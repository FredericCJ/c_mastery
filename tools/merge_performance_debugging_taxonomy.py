"""Combine the three staged, separately reviewed taxonomy contributions."""
from common import ROOT, read_json, dump, require_scoped_research_gate

def main():
    domains=[]
    for group,key in [('performance','T43'),('debugging','T44'),('memory','T45')]:
        require_scoped_research_gate(group)
        value=read_json(ROOT/f'research/performance_debugging/{group}_taxonomy.json')['domains']
        if [d['id'] for d in value]!=[key]: raise ValueError('Unexpected contributed domains: '+group)
        domains+=value
    dump(ROOT/'curriculum/performance_debugging_taxonomy.json',{'edition':'Performance and debugging extension, October2026','domains':domains})
    print('Merged separately versioned performance/debugging taxonomy:',[d['id'] for d in domains])

if __name__=='__main__': main()
