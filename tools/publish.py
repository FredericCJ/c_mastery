"""Generate the standalone Markdown edition and local handbook from canonical sources."""
from __future__ import annotations
import json
import html
import re
import shutil
import subprocess
import sys
import zipfile
from common import ROOT, anchor, dump, load, parse, read_json, total_hours, write, require_research_gates, check_program_order
from validate import check_data

SOURCE = ROOT / 'build/source'
HTML = ROOT / 'build/html'

def link(doc, label=None):
    return f'[{label or doc}]({doc}.md)'

def label(key):
    return f'({key})=\n'

def table(headers, rows):
    return '\n'.join(['| ' + ' | '.join(headers) + ' |', '| ' + ' | '.join(['---']*len(headers)) + ' |'] + ['| ' + ' | '.join(str(x).replace('|', '\\|').replace('\n', ' ') for x in row) + ' |' for row in rows]) + '\n'

def reading_rows(readings, resource_map):
    return [[f'[{html.escape(resource_map[r["resource_id"]]["title"])}](../references/{r["resource_id"]}.md)', r['locator'], r['purpose'], str(r['hours'])] for r in readings]

def publication_links(body):
    # Canonical projects live two directories below the project root; generated
    # pages live one below the generated source root. Downloads are copied by Sphinx.
    body = re.sub(r'\{ref\}`(T\d{2}\.\d{2}\.[a-z][a-z0-9_-]*)`',
        lambda m:'{ref}`'+m[1]+' <'+m[1]+'>`',body)
    return re.sub(r'\[([^\[\]\r\n]+)\]\((?:\.\./){1,2}((?:fixtures/[^)\r\n]+|research/enhancement/networking_contracts\.json))\)',
        lambda m:'{download}`'+m[1].replace('`','')+' <../'+m[2]+'>`',body)

def objective_block(domain, topic, coverage):
    rows = [r for r in coverage if r['topic']==topic['id']]
    out = ['\n#### Evidence checklist\n', 'Retain a separate evidence item for each objective below. Meeting one objective does not establish the others.\n']
    for row in rows:
        targets = ', '.join(f'[{p}](../projects/{p}.md)' for p in row['projects'])
        out += [label(row['objective']), f'- **{row["objective"]}** {row["text"]} **Disposition:** {row["status"]}. **Activity:** {targets}.\n']
    return '\n'.join(out)

def add_objectives(body, domain, coverage):
    for topic in domain['subtopics']:
        pattern = r'(^### '+re.escape(topic['id'])+r'\s+[—–-][^\n]*\n)(.*?)(?=^### |^## |\Z)'
        def replace(match):
            return label(topic['id']) + match[1] + match[2] + objective_block(domain, topic, coverage) + '\n'
        body, count = re.subn(pattern, replace, body, flags=re.M|re.S)
        if count != 1:
            raise ValueError('Teaching section not unique: ' + topic['id'])
    return body

def prepare():
    taxonomy, modules, projects, resources = load()
    errors, coverage = check_data(taxonomy, modules, projects, resources)
    if errors:
        raise ValueError('\n'.join(errors))
    require_research_gates()
    program = read_json(ROOT / 'curriculum/program.json')
    route_errors=check_program_order(program['order'],modules,projects)
    if route_errors: raise ValueError('\n'.join(route_errors))
    resource_map = {r['id']:r for r in resources}
    graph_path=ROOT/'curriculum/project_graph.json'
    project_labs={p['id']:p.get('lab_family') for p in read_json(graph_path)['projects']} if graph_path.exists() else {}
    resolved = SOURCE.resolve()
    expected = ROOT.resolve() / 'build' / 'source'
    if resolved != expected:
        raise ValueError('Unsafe generated-source target')
    if SOURCE.exists():
        shutil.rmtree(resolved)
    SOURCE.mkdir(parents=True)
    for family in sorted(set(project_labs.values())-{None}):
        destination=SOURCE/'labdownloads'/(family+'.zip')
        destination.parent.mkdir(parents=True,exist_ok=True)
        lab_root=ROOT/'labs'/family
        with zipfile.ZipFile(destination,'w',compression=zipfile.ZIP_DEFLATED) as archive:
            for path in sorted(lab_root.rglob('*')):
                relative=path.relative_to(lab_root)
                if path.is_file() and not {'build','__pycache__','assessor'} & set(relative.parts):
                    # Fixed archive metadata makes publication reproducible.
                    entry=zipfile.ZipInfo(relative.as_posix(),date_time=(2026,1,1,0,0,0))
                    entry.create_system=3
                    entry.external_attr=0o100644 << 16
                    entry.compress_type=zipfile.ZIP_DEFLATED
                    archive.writestr(entry,path.read_bytes())
    shutil.copy2(ROOT / 'site/conf.py', SOURCE / 'conf.py')
    shutil.copytree(ROOT / 'site/_static', SOURCE / '_static', dirs_exist_ok=True)
    contract=ROOT/'research/enhancement/networking_contracts.json'
    if contract.exists():
        destination=SOURCE/'research/enhancement/networking_contracts.json'
        destination.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(contract,destination)
    counts=f"{len(taxonomy)} domains · {sum(len(d['subtopics']) for d in taxonomy)} subtopics · {len(coverage):,} objectives"
    standalone = ['# C Programming Mastery\n', 'Language, algorithms, architecture, systems, firmware, communication, measurement and debugging.\n\n'+counts+'. The original 35 domains and 1,185 objectives are preserved.\n\nStart at [Entry](#guide-entry), follow the [Roadmap](#guide-roadmap), and use [Performance and debugging](#guide-performance-debugging), [Communication stack families](#guide-communication-stacks), [Assessment](#guide-assessment) and [Workload](#guide-workload). The complete source bibliography follows the project catalog.\n']
    guides=[]
    guide_order=['second-edition','entry','project-route','roadmap','component-contracts','enhancement','communication-stacks','performance-debugging','standards','profiles','inventory','laboratory','assessment','integrations','reading','workload','audit']
    guide_paths=sorted((ROOT / 'curriculum/guides').glob('*.md'),key=lambda p:(guide_order.index(p.stem) if p.stem in guide_order else 99,p.stem))
    for path in guide_paths:
        content = publication_links(path.read_text(encoding='utf-8'))
        write(SOURCE / 'guides' / path.name, label('guide-'+path.stem) + content)
        guides.append('guides/'+path.stem)
        standalone.append('<a id="guide-'+path.stem+'"></a>\n\n'+content)
    domain_map={d['id']:d for d in taxonomy}
    for key in program['order']:
        domain=domain_map[key]
        key=domain['id']; m=modules[key]; meta=m['meta']
        front = label(key) + '# ' + key + ' — ' + meta['title'] + '\n\n'
        front += table(['Track','Status','Hours','Dependency profiles'], [[meta['track'],meta['status'],total_hours(meta),', '.join(meta['profile']) if isinstance(meta['profile'],list) else meta['profile']]]) + '\n'
        if meta['prerequisites']:
            front += '**Prerequisites:** ' + ', '.join(f'[{k}]({k}.md)' for k in meta['prerequisites']) + '.\n\n'
        if meta['prelude_objectives']:
            front += '**Required selected foundations:** ' + ', '.join(f'{{ref}}`{k} <{k}>`' for k in meta['prelude_objectives']) + '. Complete those leaves before the dependent project; the rest of the referenced domain need not be finished.\n\n'
        front += table(['Reading','Implementation','Verification','Assessment','Consolidation'], [[meta['workload'][k] for k in ['reading','implementation','verification','assessment','consolidation']]]) + '\n'
        canonical_body=publication_links(re.sub(r'\A# [^\n]*\n+','',m['body']))
        body = add_objectives(canonical_body, domain, coverage)
        front += body + '\n## Assigned source passages\n\n' + table(['Resource','Inspected locator','Purpose','Hours'],reading_rows(meta['readings'],resource_map))
        front += '\n## Project directory\n\n' + '\n'.join(f'- [{p}: {projects[p]["meta"]["title"]}](../projects/{p}.md)' for p in meta['projects']) + '\n'
        write(SOURCE / 'modules' / (key+'.md'), front)
        standalone.append('<a id="'+anchor(key)+'"></a>\n\n'+front.replace(label(key),''))
        cov=[r for r in coverage if r['domain']==key]
        provenance='current taxonomy, with historical claims traced through the edition migration'
        page=label('coverage-'+key) + '# ' + key + ' objective map\n\nObjectives come from the '+provenance+'. The linked project provides its activity; the module gate and checklist identify the evidence to retain.\n\n'
        page+=table(['Objective','Disposition','Teaching','Project/activity'], [[r['objective'],r['status'],f'{{ref}}`{r["objective"]} <{r["objective"]}>`',', '.join(f'[{p}](../projects/{p}.md)' for p in r['projects'])] for r in cov])
        write(SOURCE / 'coverage' / (key+'.md'), page)
    for key, project in projects.items():
        meta=project['meta']
        content=label(key)+'# '+key+' — '+meta['title']+'\n\n'
        content+=table(['Budget','Disposition','Profile'],[[str(meta['hours'])+' h',meta['status'],meta['profile']]])+'\n'
        family=project_labs.get(key)
        if family:
            content+='**Executable anchors:** '+family+'. {download}`Download unsolved family scaffold <../labdownloads/'+family+'.zip>`. Read its family manifest for tested scope and remaining project obligations.\n\n'
        if meta['module']!='INTEGRATION':
            content+='**Teaching module:** '+f'[{meta["module"]}](../modules/{meta["module"]}.md).\n\n'
        if meta.get('prelude_objectives'):
            content+='**Required selected project foundations:** '+', '.join(f'{{ref}}`{p} <{p}>`' for p in meta['prelude_objectives'])+'. Complete these leaves before this milestone; their work remains in its original module.\n\n'
        if 'prelude_override' in meta:
            content+='**Project-specific foundation policy:** '+meta.get('prelude_policy','Use the project route for the selected foundations at this milestone.')+'\n\n'
        if meta.get('optional_runtime_prelude_objectives'):
            content+='**Foundations for the optional runtime route:** '+', '.join(f'{{ref}}`{p} <{p}>`' for p in meta['optional_runtime_prelude_objectives'])+'. Complete these leaves before the optional target experiment. They do not gate the core captured/static certificate; their work remains in its original module.\n\n'
        canonical_body=publication_links(re.sub(r'\A# [^\n]*\n+','',project['body']))
        content+='**Covered topics:** '+', '.join(f'{{ref}}`{t}`' for t in meta['topics'])+'.\n\n'+canonical_body
        content+='\n## Supplied fixtures\n\n'
        for fixture in meta['fixtures']:
            source=ROOT/fixture
            destination=SOURCE/fixture
            destination.parent.mkdir(parents=True,exist_ok=True)
            shutil.copy2(source,destination)
            content+=f'- {{download}}`{source.name} <../{fixture}>`\n'
            if source.suffix=='.json':
                content+='\n```json\n'+source.read_text(encoding='utf-8').strip()+'\n```\n'
        write(SOURCE / 'projects' / (key+'.md'),content)
        standalone.append('<a id="'+anchor(key)+'"></a>\n\n'+content.replace(label(key),''))
    for r in resources:
        content=label(r['id'])+'# '+html.escape(r['title'])+'\n\n'
        content+='**Authors/maintainers:** '+', '.join(r['authors'])+'.\n\n**Edition/version:** '+str(r['edition'])+'. **Year:** '+str(r.get('year') or 'not established')+'.\n\n'
        content+='**Role:** '+r['role']+'. **Access:** '+r['access']+'.\n\n'+f'[Authoritative resource page]({r["url"]}).\n\n'+r['reason']+'\n\n'
        prereq=r.get('prerequisites')
        prereq='; '.join(prereq) if isinstance(prereq,list) else prereq
        content+='**Reading foundations:** '+str(prereq or 'Complete the entry foundations of the assigned module; consult its reading guidance for targeted remediation.')+'\n\n'
        content+='**Domain mapping:** '+', '.join(f'[{k}](../modules/{k}.md)' for k in r['domains'])+'.\n\n'
        if r.get('assignable') is False:
            content+='**Disposition:** Bibliographic lead only; no teaching assignment is permitted.\n\n'
        content+='## Inspected reading locators\n\n'+'\n'.join('- '+v for v in r['locators'])+'\n\n'
        ins=r['inspection']
        content+='## Evidence and limits\n\n'+'**Check date:** '+ins['date']+'. **Method:** '+ins['method']+'.\n\n'+ins['scope']+'\n\n**Limits:** '+str(ins.get('limits') or 'Bounded inspection; no whole-work reading claim.')+'\n\n'
        content+='## Teaching backlinks\n\n'+'\n'.join('- ['+k+'](../modules/'+k+'.md)' for k,m in modules.items() if any(a['resource_id']==r['id'] for a in m['meta']['readings']))+'\n'
        write(SOURCE/'references'/ (r['id']+'.md'),content)
        standalone.append(content.replace(label(r['id']),'<a id="'+anchor(r['id'])+'"></a>\n\n'))
    for path in sorted((ROOT/'research').glob('*.md')):
        write(SOURCE/'research'/path.name,path.read_text(encoding='utf-8'))
    for path in sorted((ROOT/'research/contributions').glob('*synthesis.md')):
        write(SOURCE/'research'/path.name,path.read_text(encoding='utf-8'))
    for path in sorted((ROOT/'research/enhancement').glob('*.md')):
        write(SOURCE/'research'/('enhancement-'+path.name),path.read_text(encoding='utf-8'))
    for path in sorted((ROOT/'research/performance_debugging').glob('*.md')):
        write(SOURCE/'research'/('performance-debugging-'+path.name),path.read_text(encoding='utf-8'))
    for path in sorted((ROOT/'research/v2').glob('*.md')):
        write(SOURCE/'research'/('v2-'+path.name),path.read_text(encoding='utf-8'))
    resource_directory='# Reference library\n\nRead the assigned passages, not every book cover to cover. Historical works explain origins; current standards and implementation documents establish the applicable contracts.\n\n'
    resource_directory+=table(['Resource','Role','Scope'], [[f'[{html.escape(r["title"])}]({r["id"]}.md)',r['role'],', '.join(r['domains'])] for r in resources])
    resource_directory+='\n```{toctree}\n:hidden:\n\n'+'\n'.join(r['id'] for r in resources)+'\n```\n'
    write(SOURCE/'references/index.md',resource_directory)
    write(SOURCE/'coverage/index.md',f'# Objective coverage\n\nThe complete {len(coverage):,}-objective ledger is organized by domain. All original 1,185 objectives are preserved; enhancement objectives have their own taxonomy.\n\n```{{toctree}}\n:maxdepth: 1\n\n'+'\n'.join(d['id'] for d in taxonomy)+'\n```\n')
    write(SOURCE/'modules/index.md','# Module catalog\n\nThe roadmap supplies a dependency-based order; numbering preserves the original coverage domains.\n\n```{toctree}\n:maxdepth: 1\n\n'+'\n'.join(program['order'])+'\n```\n')
    write(SOURCE/'projects/index.md','# Implementation projects\n\nEach milestone has a 4–12-hour budget, an explicit contract and supplied data. Reading and delayed reassessment have separate budgets.\n\n```{toctree}\n:maxdepth: 1\n\n'+'\n'.join(projects)+'\n```\n')
    research_docs=[p.stem for p in sorted((SOURCE/'research').glob('*.md'))]
    write(SOURCE/'research/index.md','# Research and source selection\n\nSearch, reflection and focused follow-up preceded teaching authorship. The reports disclose inspection boundaries and rejected alternatives.\n\n```{toctree}\n:maxdepth: 1\n\n'+'\n'.join(research_docs)+'\n```\n')
    home=(ROOT/'site/index.md').read_text(encoding='utf-8')
    home=home.replace('{{COUNTS}}',counts).replace('{{PROJECTS}}',str(len(projects)))
    home+='\n```{toctree}\n:hidden:\n:maxdepth: 2\n:caption: Study handbook\n\n'+'\n'.join(guides)+'\nmodules/index\nprojects/index\nreferences/index\ncoverage/index\nresearch/index\n```\n'
    write(SOURCE/'index.md',home)
    merged='\n\n---\n\n'.join(standalone)
    merged=re.sub(r'\{ref\}`([^`<>]+) <([^`<>]+)>`',lambda m:f'[{m[1]}](#{anchor(m[2])})',merged)
    merged=re.sub(r'\{ref\}`([^`]+)`',lambda m:f'[{m[1]}](#{anchor(m[1])})',merged)
    merged=re.sub(r'\((?:\.\./)?(?:modules|projects|references)/([^/()]+)\.md\)',lambda m:'(#'+anchor(m[1])+')',merged)
    merged=re.sub(r'\((T\d{2})\.md\)',lambda m:'(#'+anchor(m[1])+')',merged)
    merged=re.sub(r'\((PR-[A-Za-z0-9-]+)\.md\)',lambda m:'(#'+anchor(m[1])+')',merged)
    merged=re.sub(r'\((?:\.\./)?guides/([^/()]+)\.md\)',lambda m:'(#guide-'+anchor(m[1])+')',merged)
    merged=re.sub(r'\(('+'|'.join(re.escape(p.stem) for p in guide_paths)+r')\.md\)',lambda m:'(#guide-'+anchor(m[1])+')',merged)
    merged=re.sub(r'^\((T\d{2}(?:\.\d{2}(?:\.[a-z][a-z0-9_-]*)?)?)\)=[ \t]*$',lambda m:'\n<a id="'+anchor(m[1])+'"></a>\n',merged,flags=re.M)
    merged=re.sub(r'\{download\}`([^`]+) <\.\./([^>]+)>`',r'[\1](\2)',merged)
    merged=re.sub(r'\[Download unsolved family scaffold\]\(labdownloads/([a-z0-9_-]+)\.zip\)',r'[Unsolved family scaffold](labs/\1/README.md)',merged)
    write(ROOT/'c_programming_mastery_curriculum.md',merged)
    dump(ROOT/'curriculum/coverage.json',{'records':coverage})
    dump(ROOT/'reviews/publication_inventory.json',{'modules':len(modules),'projects':len(projects),'resources':len(resources),'objectives':len(coverage),'canonical_pages':{k:str(v['path'].relative_to(ROOT)) for k,v in {**modules,**projects}.items()}})
    return SOURCE, HTML

def main():
    source,html=prepare()
    # A clean generated output also removes stale empty incremental-search
    # postings. Guard cleanup exactly as for generated source, never teaching.
    resolved=HTML.resolve()
    expected=ROOT.resolve()/'build'/'html'
    if resolved!=expected:
        raise ValueError('Unsafe generated-HTML target')
    if HTML.exists():
        shutil.rmtree(resolved)
    result=subprocess.run([sys.executable,'-m','sphinx','-b','html','-W','--keep-going',str(source),str(html)],capture_output=True,text=True,encoding='utf-8')
    write(ROOT/'reviews/sphinx_build.log',result.stdout+'\n'+result.stderr)
    dump(ROOT/'reviews/sphinx_build.json',{'passed':result.returncode==0,'exit_code':result.returncode,'warnings_as_errors':True,'stderr':result.stderr.strip()})
    if result.returncode:
        print(result.stderr[-12000:])
        raise SystemExit(result.returncode)
    print('Handbook built at',html)

if __name__=='__main__': main()
