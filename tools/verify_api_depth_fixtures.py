"""Independent arithmetic/list oracle for the original API contracts."""
from common import ROOT, read_json


def verify():
    data=read_json(ROOT/'fixtures/api_depth/contracts.json')
    a=data['append']; maximum=a['maximum_capacity']
    pairs=[(length,capacity) for capacity in range(maximum+1) for length in range(capacity+1)]
    cases=[(length,capacity,n) for length,capacity in pairs for n in range(maximum+1)]
    accepted=sum(length+n<=capacity for length,capacity,n in cases)
    assert (len(pairs),len(cases),accepted,len(cases)-accepted)==(a['length_capacity_pairs'],a['cases'],a['accepted'],a['rejected'])
    for row in data['registry']:
        members=['A','B']; snapshot=tuple(members); calls=[]; retire=[]; closing=False
        for callback in snapshot:
            if closing:break
            calls.append(callback)
            if callback=='A':
                operation=row['operation']
                if operation=='remove-self-A':retire.append('A')
                elif operation=='remove-B-from-A':retire.append('B')
                elif operation=='destroy-from-A':closing=True
                elif operation=='add-C-from-A':members.append('C')
                else:raise AssertionError(operation)
        drops=members[:] if closing else [name for name in members if name in retire]
        remaining=[name for name in members if name not in drops]
        assert (calls,drops,remaining)==(row['calls'],row['drops_after_dispatch'],row['remaining'])
    return dict(arithmetic_cases=len(cases),callback_traces=len(data['registry']),passed=True,
                limits='Mathematical/list expectations only; C observations are the separate campaign.')


if __name__=='__main__':print(verify())
