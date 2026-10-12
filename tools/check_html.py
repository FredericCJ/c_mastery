"""Independent destination, asset, canonical-prose and contract checks."""
from __future__ import annotations
import html
from pathlib import Path
import re
from urllib.parse import unquote,urlsplit
from bs4 import BeautifulSoup
from markdown_it import MarkdownIt
from common import ROOT, dump, load, read_json, normalize_prose

def norm(value):
    return re.sub(r'\s+',' ',html.unescape(value)).strip()

def main():
    base=ROOT/'build/html'
    # Theme Jinja macro files copied into _static are build assets, not rendered
    # pages. Their template expressions do not describe browser destinations.
    pages=sorted(p for p in base.rglob('*.html') if '_static' not in p.relative_to(base).parts)
    if not pages: raise ValueError('No handbook HTML')
    errors=[]; checked=0; prose_checks=0; literal_code_checks=0; remote_assets=[]
    target_cache={}; exists_cache={}; base_resolved=base.resolve()
    parsed={p:BeautifulSoup(p.read_text(encoding='utf-8'),'html.parser') for p in pages}
    ids={p:{t.get('id') for t in soup.find_all(id=True)} for p,soup in parsed.items()}
    for page,soup in parsed.items():
        article=soup.select_one('article.bd-article')
        if article and re.search(r'Licensed to|frederic\.ferragne|F:[/\\](?:Knowledge|Cours)[/\\]Library',article.get_text(' '),re.I):
            errors.append(str(page.relative_to(base))+': private source leakage')
        for tag in soup.find_all(['a','script','link','img']):
            value=tag.get('href') if tag.name in ['a','link'] else tag.get('src')
            if not value: continue
            parts=urlsplit(value)
            if parts.scheme in ['http','https','mailto','data','javascript']:
                if parts.scheme in ['http','https'] and (tag.name in ['script','img'] or tag.name=='link' and 'stylesheet' in tag.get('rel',[])):
                    remote_assets.append(value)
                continue
            if parts.scheme:
                errors.append(str(page.relative_to(base))+': unsupported URL '+value); continue
            # Fragment-only links belong to this page, not every page in its
            # directory; relative file destinations can share a directory cache.
            cache_key=(page if not parts.path else page.parent,parts.path)
            if cache_key not in target_cache:
                target=(page.parent/unquote(parts.path)).resolve() if parts.path else page
                if target.is_dir(): target=target/'index.html'
                target_cache[cache_key]=target
            target=target_cache[cache_key]
            if not target.is_relative_to(base_resolved):
                errors.append(str(page.relative_to(base))+': escaping local destination '+value); continue
            if target not in exists_cache: exists_cache[target]=target.exists()
            if not exists_cache[target]:
                errors.append(str(page.relative_to(base))+': missing local destination '+value); continue
            if parts.fragment and target.suffix=='.html' and unquote(parts.fragment) not in ids.get(target,set()):
                errors.append(str(page.relative_to(base))+': missing anchor '+value)
            checked+=1
    taxonomy,modules,projects,resources=load()
    renderer=MarkdownIt('commonmark').enable('table')
    for folder,records in [('modules',modules),('projects',projects)]:
        for key,record in records.items():
            path=base/folder/(key+'.html')
            if path not in parsed:
                errors.append(key+': missing rendered page');continue
            article=parsed[path].select_one('article.bd-article')
            actual=norm(article.get_text(' '))
            actual_code={tag.get_text().rstrip('\n') for tag in article.find_all('pre')}
            # Compare each complete canonical paragraph and literal code block.
            expected=BeautifulSoup(renderer.render(normalize_prose(record['body'])),'html.parser')
            for tag in expected.find_all(['p','pre','h2','h3','li']):
                if tag.name=='pre':
                    if tag.get_text().rstrip('\n') not in actual_code:
                        errors.append(key+': canonical literal code absent: '+tag.get_text()[:110])
                    prose_checks+=1; literal_code_checks+=1
                    continue
                text=norm(tag.get_text(' '))
                if not text or '$' in text or '{ref}' in text or '{download}' in text: continue
                if text not in actual:
                    errors.append(key+': canonical prose/code absent: '+text[:110])
                prose_checks+=1
            meta=record['meta']
            if folder=='projects' and str(meta['hours'])+' h' not in actual:
                errors.append(key+': budget absent')
    objective_text_checks=0
    for row in read_json(ROOT/'curriculum/coverage.json')['records']:
        page=base/'modules'/(row['domain']+'.html')
        soup=parsed.get(page)
        if soup is None or row['objective'].lower().replace('.','-') not in ids.get(page,set()):
            errors.append(row['objective']+': objective target absent')
        if soup is not None:
            expected_text=norm(BeautifulSoup(renderer.render(normalize_prose(row['text'])),'html.parser').get_text(' '))
            actual_text=norm(soup.select_one('article.bd-article').get_text(' '))
            if expected_text not in actual_text: errors.append(row['objective']+': original objective text absent')
            objective_text_checks+=1
    if remote_assets: errors.append('Remote required assets: '+str(sorted(set(remote_assets))))
    report={'passed':not errors,'pages':len(pages),'local_destinations_assets':checked,'canonical_text_checks':prose_checks,'literal_code_checks':literal_code_checks,'objective_text_checks':objective_text_checks,'remote_required_assets':sorted(set(remote_assets)),'errors':errors}
    dump(ROOT/'reviews/html_checks.json',report)
    print({**report,'error_count':len(errors),'errors':errors[:12]})
    if errors: raise SystemExit(1)

if __name__=='__main__': main()
