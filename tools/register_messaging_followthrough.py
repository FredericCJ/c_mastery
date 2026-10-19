"""Canonical additive routing for required T73 successors; no acceptance writer."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ROUTES = {
    'X-T73-010': {
        'tests':['messaging.stream_executor','messaging.stream_overload'],
        'contract':'labs/messaging/followthrough/STREAM.md',
    },
    'X-T73-007': {
        'tests':['messaging.transactions_effects','messaging.transactions_privatization','messaging.transactions_phases'],
        'contract':'labs/messaging/followthrough/TRANSACTIONS.md',
        'contracts':['labs/messaging/followthrough/LIFETIME.md'],
    },
    'X-T73-008': {
        'tests':['messaging.versions_owned','messaging.versions_predicate'],
        'contract':'labs/messaging/followthrough/VERSIONS.md',
        'contracts':['labs/messaging/followthrough/LIFETIME.md'],
    },
    'X-T73-001': {
        'tests':['messaging.nursery_nested','messaging.nursery_aggregate'],
        'contract':'labs/messaging/followthrough/NURSERY.md',
        'fixture':'fixtures/messaging/nursery_follow.json',
    },
    'X-T73-003': {
        'tests':['messaging.operation_inline','messaging.completion_winner'],
        'contract':'labs/messaging/followthrough/OPERATION.md',
        'contracts':['labs/messaging/followthrough/COMPLETION.md'],
    },
    'X-T73-002': {
        'tests':['messaging.owned_channels','messaging.select_registration'],
        'contract':'labs/messaging/followthrough/OWNED.md',
        'contracts':['labs/messaging/followthrough/REGISTRATION.md'],
        'fixture':'fixtures/messaging/owned_follow.json',
    },
    'X-T73-004': {
        'tests':['messaging.owned_actors','messaging.actor_executor'],
        'contract':'labs/messaging/followthrough/OWNED.md',
        'contracts':['labs/messaging/followthrough/ACTOR_EXECUTOR.md'],
        'fixture':'fixtures/messaging/owned_follow.json',
    },
    'X-T73-005': {
        'tests':['messaging.supervisor_tree'],
        'contract':'labs/messaging/followthrough/SUPERVISOR.md',
    },
    'X-T73-006': {
        'tests':['messaging.weak_deque','messaging.lifetime_growth','messaging.lifetime_references'],
        'contract':'labs/messaging/followthrough/WEAK.md',
        'contracts':['labs/messaging/followthrough/LIFETIME.md'],
    },
    'X-T73-009': {
        'tests':['messaging.weak_sequence','messaging.lifetime_snapshot'],
        'contract':'labs/messaging/followthrough/WEAK.md',
        'contracts':['labs/messaging/followthrough/LIFETIME.md'],
    },
}


def append_unique(values, additions):
    for value in additions:
        if value not in values: values.append(value)


def main():
    path = ROOT/'labs/messaging/family.json'
    family = json.loads(path.read_text(encoding='utf8'))
    append_unique(family['dependencies'], ['hosted_systems'])
    family['campaign']['fixture_oracle'] = 'tools/verify_messaging_completion.py'
    for exercise, route in ROUTES.items():
        row = next(r for r in family['exercises'] if r['id'] == exercise)
        append_unique(row['tests'], route['tests'])
        append_unique(family['tests'], route['tests'])
        brief = ROOT/'curriculum/v5/exercises'/f'{exercise}.md'
        text = brief.read_text(encoding='utf8'); start = text.index('```json\n')+8
        end = text.index('\n```', start)
        meta = json.loads(text[start:end])
        append_unique(meta['lab']['tests'],route['tests'])
        append_unique(meta['shared_contracts'],[route['contract']]+route.get('contracts',[])+['labs/messaging/followthrough/MEASUREMENTS.md'])
        cost_reference={'id':'R-MOD-MSG-POSIX-WAKE','path':'research/v5/modern/messaging_cost_resources.json',
                        'passages':['pthread_cond_wait RATIONALE: Condition Wait Semantics and Scheduling Behavior of Mutexes and Condition Variables']}
        if exercise=='X-T73-002':
            cost_reference['passages']+=['pthread_cond_wait DESCRIPTION: atomic mutex release, predicate recheck, dynamic binding, cancellation and timeout clock',
                                        'pthread_cond_broadcast DESCRIPTION and RATIONALE: scheduling order and Multiple Awakenings by Condition Signal']
        if not any(r['id']==cost_reference['id'] for r in meta['source_records']):meta['source_records'].append(cost_reference)
        if exercise=='X-T73-001':
            stack_reference={'id':'R-MOD-MSG-STACK-NP','path':'research/v5/modern/messaging_cost_resources.json','passages':['SYNOPSIS, DESCRIPTION, RETURN VALUE, ERRORS, STANDARDS and EXAMPLES']}
            if not any(r['id']==stack_reference['id'] for r in meta['source_records']):meta['source_records'].append(stack_reference)
        if 'fixture' in route: append_unique(meta['fixtures'],[route['fixture']])
        if exercise == 'X-T73-001':
            reference = {'id':'R-SYS-POSIX-THREAD-LIFECYCLE-2024','path':'references/resources.json',
                         'passages':['pthread_create — SYNOPSIS, DESCRIPTION, RETURN VALUE and ERRORS',
                                     'pthread_join — SYNOPSIS, DESCRIPTION, RETURN VALUE and ERRORS']}
            if not any(r['id']==reference['id'] for r in meta['source_records']):
                meta['source_records'].append(reference)
        if exercise in ('X-T73-006','X-T73-009'):
            reference={'id':'R-MOD-GENMC-019','path':'research/v5/modern/messaging_model_resources.json',
                       'passages':['doc/manual/usage.md: finite data-deterministic clients, C verification, bounding and error reporting',
                                   'doc/manual/features.md: Available Memory Models and Race Detection and Memory Errors',
                                   'doc/manual/api.md: supported C pthread and atomic APIs']}
            if not any(r['id']==reference['id'] for r in meta['source_records']):
                meta['source_records'].append(reference)
        if exercise == 'X-T73-006':
            reference={'id':'R-MOD-CHASE-LEV-ERRATA','path':'research/v5/modern/messaging_model_resources.json','passages':['Item2 reload after resizing and proof read instance','Item3 C11 array publication dependency and counterexample']}
            if not any(r['id']==reference['id'] for r in meta['source_records']):meta['source_records'].append(reference)
        if exercise == 'X-T73-010':
            reference=next(r for r in meta['source_records'] if r['id']=='R-MOD-REACTIVE')
            append_unique(reference['passages'],['Subscription rule3.17 bounded representation of unbounded demand','Subscriber controlled queue bounds'])
        if exercise == 'X-T73-007':
            reference=next(r for r in meta['source_records'] if r['id']=='R-MOD-TL2')
            append_unique(reference['passages'],['2.3 Mixed Transactional and Non-Transactional Memory Management'])
        if exercise == 'X-T73-008':
            reference={'id':'R-MOD-PG18-ISOLATION','path':'research/v5/modern/messaging_versions_resources.json','passages':['13.2.2 Repeatable Read Isolation Level','13.2.3 Serializable Isolation Level']}
            if not any(r['id']==reference['id'] for r in meta['source_records']):meta['source_records'].append(reference)
        body = text[end+4:]
        marker = '\n## Executable successor support\n'
        if marker in body: body = body[:body.index(marker)]
        body += marker+'\nThe required changed-condition implementation is specified in ['+Path(route['contract']).stem+'](../../../'+route['contract']+'). '
        body += 'Implement the named learner source files, retain the imported component contracts, and submit the mapped tests plus reasoning. '
        body += 'Author executions establish bounded support only; learner performance, unseen transfer and broader concurrency properties require their own evidence.\n'
        for contract in route.get('contracts',[]):
            body+='\nThe required ownership/lifetime transfer is specified in ['+Path(contract).stem+'](../../../'+contract+').\n'
        body+='\nThe required cost and progress experiment uses [MEASUREMENTS](../../../labs/messaging/followthrough/MEASUREMENTS.md), with equivalent workload checks, raw observations and explicit limits.\n'
        brief.write_text('```json\n'+json.dumps(meta,indent=2,ensure_ascii=False)+'\n```'+body,encoding='utf8')
    path.write_text(json.dumps(family,indent=2,ensure_ascii=False)+'\n',encoding='utf8')


if __name__ == '__main__': main()
