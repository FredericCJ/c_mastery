"""Publish the v4 overlay without changing retained teaching or old receipts."""
from __future__ import annotations
import re
import shutil
import subprocess
import sys
import zipfile
from common import ROOT, anchor, dump, read_json, strip_bullet, write as write_raw
from v4_common import V4
import publish as legacy

write = legacy.write


def table(headers, rows):return legacy.table(headers, rows)


def card(u):
    aid=anchor(u['id'])
    out=[f'({aid})=\n### {u["id"]} — {u["title"]}',
        f'**Budget:** {u["minutes"]} minutes inside {u["project"]}. **Evidence scope:** {u["target_scope"]}. **Support:** {u["scaffolding"]}.',
        '**Requires:** '+(', '.join('{ref}`'+anchor(d)+'`' for d in u['requires']) or 'Entry knowledge only')+'.',
        '**Imports:** '+(', '.join('`'+a+'`' for a in u['imports']) or 'No implicit code dependency')+'.',
        '**New work:**\n\n'+'\n'.join(f'{i+1}. {s}' for i,s in enumerate(u['steps'])),
        '**Check and stop:**\n\n'+'\n'.join('- '+strip_bullet(s) for s in u['checks']),
        '**Recall before reopening source:** '+', '.join('{ref}`recall-'+anchor(t)+'`' for t in u['recall_sets'])+'.']
    if u['new_executable']:
        out+=['```sh\npython3 -m cmastery init '+u['project']+' ../'+u['id'].lower()+' --unit '+u['id']+'\npython3 -m cmastery check ../'+u['id'].lower()+' --unit '+u['id']+'\n```',
            '**Executable gates:** '+', '.join('`'+x+'`' for x in u['tests'])+'. {download}`Unsolved '+u['family']+' scaffold <../labdownloads/'+u['family']+'.zip>`.']
    elif not u['tests']:
        out+=['This increment uses the observed trace or artifact checks above. Later family gates stay deferred; an empty test selection does not constitute a pass.']
    if u['decision_required']:
        p=u['decision_prompt'];out+=['**Design gate:** '+p['question']+' Alternatives: '+'; '.join(p['alternatives'])+'. '+p['discriminator']+' **Reversal trigger:** '+p['reversal_trigger']]
    scoped=[c for c in u['claims'] if c['scope'] in {'physical','emulator'}]
    if scoped:
        out+=['**Required target evidence:**\n\n'+'\n'.join('- '+c['scope']+': '+', '.join(c.get('targets',[c.get('target','declared target')]))+(' / interfaces '+', '.join(c['interfaces']) if c.get('interfaces') else '')+(' / CPU counts '+str(c['cpus']) if c.get('cpus') else '')+(' / accepted source units '+', '.join(c['source_units']) if c.get('source_units') else '')+'.' for c in scoped)]
    if u['exports']:out+=['**Reusable exports:** '+', '.join('`'+a+'`' for a in u['exports'])+'. Export requires scoped component acceptance.']
    if u['readings']:
        out+=['**Bounded reading:**\n\n'+'\n'.join('- {ref}`'+r['resource_id']+'`: '+r['locator']+'. '+r.get('purpose','') for r in u['readings'])]
    remaining=u['remaining_evidence']
    out+=['**Remaining obligations:** '+('; '.join(remaining) if isinstance(remaining,list) else remaining)+'.',
        '**Objective facets:** '+', '.join('{ref}`'+o+' <'+o+'>`' for o in u['objectives'])+'.']
    return '\n\n'.join(out)+'\n'


def plain_roles(text):
    text=re.sub(r'^\(([^\n]+)\)=[ \t]*$',lambda m:'<a id="'+anchor(m[1])+'"></a>',text,flags=re.M)
    text=re.sub(r'\{ref\}`([^`<>]+) <([^`<>]+)>`',lambda m:'['+m[1]+'](#'+anchor(m[2])+')',text)
    text=re.sub(r'\{ref\}`([^`]+)`',lambda m:'['+m[1]+'](#'+anchor(m[1])+')',text)
    text=re.sub(r'\{download\}`([^`]+) <\.\./labdownloads/([^>]+)\.zip>`',lambda m:'['+m[1]+'](labs/'+m[2]+'/README.md)',text)
    text=re.sub(r'\[([^\]]+)\]\((?:\.\./)?(?:modules|projects)/([^/)]+)\.md\)',lambda m:'['+m[1]+'](#'+anchor(m[2])+')',text)
    return text


def prepare(edition=4):
    tag=f'v{edition}';catalog=ROOT/f'curriculum/{tag}';receipts=ROOT/f'reviews/{tag}'
    guides=catalog if (catalog/'guide.md').is_file() else V4
    source,html=legacy.prepare()
    data=read_json(catalog/'units.json');units={u['id']:u for u in data['units']};overlays=read_json(catalog/'project_overlays.json')['projects']
    arts=read_json(catalog/'artifacts.json')['artifacts'];recall=read_json(catalog/'recall.json')['sets'];summary=read_json(catalog/'summary.json')
    budget=read_json(catalog/'budget.json')
    merged=(ROOT/'c_programming_mastery_curriculum.md').read_text(encoding='utf-8')
    merged=merged.replace('# C Programming Mastery\n',f'# C Programming Mastery {edition}\n',1)
    stale='Advanced kernel work, modern lock-free reclamation, production cryptography, every balanced-tree family and every allocator family are extensions.'
    corrected='Advanced kernel work and modern lock-free reclamation retain their required v3 dispositions. Other elective subjects retain their explicitly recorded dispositions.'
    merged=merged.replace(stale,corrected)
    guide=source/'guides/entry.md'
    write(guide,guide.read_text(encoding='utf-8').replace(stale,corrected))
    route=source/'guides/project-route.md'
    text=route.read_text(encoding='utf-8').replace('# Project route and reusable artifacts','# Historical project coverage route')
    text=text.replace('Follow this ordered project route after the entry diagnostic.','For current study use {ref}`'+tag+'-route`; this preserved v3 order is a coverage reference.')
    write(route,text)
    # All families are downloadable even when the retained legacy graph has no row.
    for folder in sorted((ROOT/'labs').iterdir()):
        if not (folder/'family.json').is_file():continue
        destination=source/'labdownloads'/(folder.name+'.zip')
        with zipfile.ZipFile(destination,'w',compression=zipfile.ZIP_DEFLATED) as z:
            for path in sorted(folder.rglob('*')):
                relative=path.relative_to(folder)
                if not path.is_file() or {'build','assessor','__pycache__'}&set(relative.parts):continue
                item=zipfile.ZipInfo(relative.as_posix(),date_time=(2026,1,1,0,0,0));item.create_system=3;item.external_attr=0o100644<<16;item.compress_type=zipfile.ZIP_DEFLATED
                z.writestr(item,path.read_bytes())
    supplements=[];docs=[]
    for name in ['guide','architecture','reuse','retrieval','budgets','workflow']:
        value=(guides/(name+'.md')).read_text(encoding='utf-8')
        if guides is V4 and edition!=4:value=value.replace('# C Mastery 4',f'# C Mastery {edition}',1)
        value='('+tag+'-'+name+')=\n'+value
        write(source/'learning'/(name+'.md'),value);docs.append('learning/'+name);supplements.append(value)
    for phase in data['phases']:
        selected=[units[k] for k in data['order'] if units[k]['phase']==phase['id']]
        body=f'({tag}-phase-{phase["id"]})=\n# Phase {phase["id"]}: {phase["title"]}\n\n'+phase['gate']+'\n\nThese are capability groupings, not serial blocks. Follow actual prerequisites and the global route.\n\n'
        body+=table(['Unit','Project / result','Minutes','Strand'],[['{ref}`'+anchor(u['id'])+'`',u['project']+' / '+u['title'],u['minutes'],u['strand']] for u in selected])
        write(source/'learning'/f'phase-{phase["id"]}.md',body);docs.append(f'learning/phase-{phase["id"]}');supplements.append(body)
    rows=[];elapsed=0
    for i,key in enumerate(data['order']):
        u=units[key];elapsed+=u['hours']
        rows.append([i+1,'{ref}`'+anchor(key)+'`',u['title'],u['phase'],u['minutes'],round(elapsed,2)])
    route='('+tag+'-route)=\n# The working-unit route\n\n'+str(len(units))+' working units preserve '+str(summary['projects'])+' project envelopes. Exactly '+str(summary['new_executable_units'])+' units have newly supplied executable gates. Recall variants are counted separately. The first three-hour diagnostic can identify conditional remediation; do not claim that diagnosis alone accepts every foundation.\n\n'
    route+=table(['Step','Unit','Observable increment','Phase','Minutes','Cumulative implementation / verification hours'],rows)
    write(source/'learning/route.md',route);docs.append('learning/route');supplements.append(route)
    registry='('+tag+'-artifacts)=\n# Component lineage registry\n\nExport only accepted claims and declared source/header interfaces. A named artifact is not automatically qualified for every target.\n\n'
    registry+=table(['Artifact','Producer','Public contract','Consumers'],[[a['id'],'{ref}`'+anchor(a['producer_unit'])+'`',a['public_contract'],len(a['consumers'])] for a in arts])
    write(source/'learning/artifacts.md',registry);docs.append('learning/artifacts');supplements.append(registry)
    for domain in sorted({s['id'].split('.')[0] for s in recall}):
        body='('+tag+'-recall-'+domain.lower()+')=\n# '+domain+' recall sets\n\nOne queue entry per set; rotate variants. Attempt before feedback and score only the objectives actually demonstrated.\n\n'
        for s in recall:
            if not s['id'].startswith(domain+'.'):continue
            body+='(recall-'+anchor(s['id'])+')=\n## '+s['id']+' — '+s['title']+'\n\n'
            for v in s['variants']:
                body+='### '+v['kind']+'\n\n'+v['prompt']+'\n\n'
                body+='<details><summary>After the attempt: scoring facets</summary>\n\n'+'\n'.join('- '+x for x in v['checklist'])+'\n\n</details>\n\n'
            body+='**Objectives:** '+', '.join('{ref}`'+o+' <'+o+'>`' for o in s['objectives'])+'.\n\n'
        write(source/'learning'/('recall-'+domain+'.md'),body);docs.append('learning/recall-'+domain);supplements.append(body)
    # The overlay is visible first. The legacy working card is retained as an
    # explicitly historical, collapsed record for unchanged prose verification.
    for key,overlay in overlays.items():
        path=source/'projects'/(key+'.md');original=path.read_text(encoding='utf-8')
        def historical(m):return '<details><summary>Historical v3 working card (use the '+tag+' units above)</summary>\n\n'+m[0]+'\n\n</details>\n\n'
        original=re.sub(r'^## Working card\n.*?(?=^## |\Z)',historical,original,flags=re.M|re.S)
        block=f'## V{edition} working units\n\n'+overlay['legacy_notice']+'\n\n**Engineering question:** '+overlay['engineering_question']+'\n\n'
        block+='\n'.join(card(units[uid]) for uid in overlay['units'])+'\n## Retained full project contract\n\n'
        # Keep the historical title and budget, then the new normative unit route.
        pos=original.find('\n\n',original.find('# '))+2
        write(path,original[:pos]+block+original[pos:])
        marker='<a id="'+anchor(key)+'"></a>'
        merged=merged.replace(marker,marker+'\n\n'+plain_roles(block),1)
    evidence='('+tag+'-executable)=\n# Newly executable units\n\nA lab is an implementation increment with its own rejecting gate. Shared mappings, fixture replays and recall questions are not additional executable units.\n\n'
    evidence+=table(['Unit','Family','Project budget owner','Declared scope'],[['{ref}`'+anchor(u['id'])+'`',u['family'],u['project'],u['stop_condition']] for u in data['units'] if u['new_executable']])
    write(source/'learning/executable.md',evidence);docs.append('learning/executable');supplements.append(evidence)
    research=ROOT/f'research/{tag}/search_reflection.md'
    if not research.is_file():research=ROOT/'research/v4/search_reflection.md'
    reflection='('+tag+'-research)=\n'+research.read_text(encoding='utf-8')
    write(source/'learning/research.md',reflection);docs.append('learning/research');supplements.append(reflection)
    intro=f'({tag}-index)=\n# C Mastery {edition}\n\n'+f'{summary["domains"]} domains · {summary["projects"]} projects · {summary["units"]} working units · {summary["new_executable_units"]} new executable units · {summary["recall_sets"]} recall sets.\n\n'
    intro+='Start with {ref}`'+tag+'-guide`, use {ref}`'+tag+'-workflow`, and follow {ref}`'+tag+'-route`. Grow a shared toolbox, Linux telemetry/service, firmware gateway and VM/store kernel appliance.\n\n'
    intro+='```{toctree}\n:maxdepth: 1\n\n'+'\n'.join(d.removeprefix('learning/') for d in docs)+'\n```\n'
    write(source/'learning/index.md',intro)
    coverage_text=f"{summary['domains']} domains / {summary['projects']} projects / {summary['objectives']:,} objectives"
    expansion_text=f"{summary['new_executable_units']} new units in {summary['families']} total families"
    recall_text=f"{summary['recall_sets']} sets / {summary['recall_variants']:,} variants"
    hours_text=f"{budget['catalog_hours']:,}"
    toctree='```{toctree}\n:hidden:\n:maxdepth: 2\n\nlearning/index\n'+ '\n'.join('guides/'+p.stem for p in sorted((ROOT/'curriculum/guides').glob('*.md')))+'\nmodules/index\nprojects/index\nreferences/index\ncoverage/index\nresearch/index\n```\n'
    if edition==4:
        home='# C Programming Mastery 4\n\nBuild small components, explain their mechanisms and reuse them across useful systems.\n\n'
        home+='Start with {ref}`v4-guide` and {ref}`v4-workflow`. Follow {ref}`v4-route`; domain numbers organize the reference catalog.\n\n'
        home+=table(['Coverage','Working units','Executable expansion','Recall'],[[coverage_text,summary['units'],expansion_text,recall_text]])
        home+='\nThe catalog stays within '+hours_text+' planned hours. Source, image, emulator, physical and learner-mastery claims remain distinct. Original freestanding and physical-interface requirements are retained.\n\n'
    else:
        # From the fifth edition the home page is the site template with manifest-driven placeholders.
        home=(ROOT/'site/index.md').read_text(encoding='utf-8')
        for key,value in {'{{EDITION}}':str(edition),'{{TAG}}':tag,'{{COVERAGE}}':coverage_text,'{{UNITS}}':str(summary['units']),
                          '{{EXECUTABLE}}':expansion_text,'{{RECALL}}':recall_text,'{{HOURS}}':hours_text}.items():
            home=home.replace(key,value)
        home=home.rstrip('\n')+'\n\n'
    # Keep legacy guide/catalog pages in the navigation for source and objective coverage.
    home+=toctree
    write(source/'index.md',home)
    supplement='\n\n---\n\n'.join(plain_roles(s) for s in supplements)
    # All v4 cross references point to explicit anchors in this same standalone artifact.
    title=f'# C Programming Mastery {edition}\n'
    merged=merged.replace(title,title+'\n'+plain_roles(intro.split('```{toctree}')[0])+ '\n\n'+supplement+'\n\n---\n',1)
    write(ROOT/'c_programming_mastery_curriculum.md',merged)
    dump(receipts/'publication_inventory.json',{**summary,'project_pages':len(overlays),'recall_pages':len({s['id'].split('.')[0] for s in recall}),'download_families':summary['families'],'historical_source':'retained; '+tag+' overlays displayed first'})
    return source,html


def main(edition=4):
    receipts=ROOT/f'reviews/v{edition}'
    source,html=prepare(edition)
    if html.resolve()!=ROOT.resolve()/'build'/'html':raise ValueError('Unsafe generated output')
    if html.exists():shutil.rmtree(html)
    result=subprocess.run([sys.executable,'-m','sphinx','-b','html','-W','--keep-going',str(source),str(html)],capture_output=True,text=True,encoding='utf-8')
    write_raw(receipts/'sphinx_build.log',result.stdout+'\n'+result.stderr)
    dump(receipts/'sphinx_build.json',{'status':'passed' if result.returncode==0 else 'failed','passed':result.returncode==0,'warnings_as_errors':True,'exit_code':result.returncode,'stderr':result.stderr.strip()})
    if result.returncode:print(result.stderr[-12000:]);raise SystemExit(result.returncode)
    print(f'V{edition} handbook built:',html)


if __name__=='__main__':main()
