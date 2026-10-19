"""Independent destination, asset, canonical-prose and contract checks."""
from __future__ import annotations
import html
from contextlib import contextmanager
from functools import lru_cache
from pathlib import Path
import re
from urllib.parse import unquote,urlsplit
from bs4 import BeautifulSoup
from markdown_it import MarkdownIt
from common import ROOT, dump, load, read_json, normalize_prose

def norm(value):
    return re.sub(r'\s+',' ',html.unescape(value)).strip()

def compact(value):
    """Whitespace-free comparison form: inline links add spaces at tag boundaries, never words."""
    return re.sub(r'\s+','',html.unescape(value))

def release_soup(soup):
    # The document root need not have next_element. Decomposing only the root
    # can leave child cycles awaiting cyclic GC; explicitly decompose children.
    soup.clear(decompose=True)
    soup.decompose()

@contextmanager
def page_soup(path):
    """Own one page tree; break BeautifulSoup's cycles before the next page."""
    soup=BeautifulSoup(path.read_text(encoding='utf-8'),'html.parser')
    try:
        yield soup
    finally:
        release_soup(soup)


def inspect_html(base, modules, projects, coverage, *, cache_limit=4096, diagnostics=None):
    """Check the same publication with ID sets, not an archive of page DOMs.

    The first pass retains only page IDs. The second checks links, canonical
    content and the objectives belonging to that page before releasing its DOM.
    Destination caches have explicit bounds independent of total link count.
    Required ID sets and reported errors still grow with publication size.
    """
    if type(cache_limit) is not int or cache_limit < 1:
        raise ValueError('Destination cache limit must be a positive integer')
    base=Path(base).resolve()
    # Theme Jinja macro files copied into _static are build assets, not rendered
    # pages. Their template expressions do not describe browser destinations.
    pages=sorted(p for p in base.rglob('*.html') if '_static' not in p.relative_to(base).parts)
    if not pages: raise ValueError('No handbook HTML')
    errors=[]; checked=0; prose_checks=0; literal_code_checks=0; remote_assets=set()
    # Unlinked static assets are still distributed. A prose/URL scan alone
    # missed private source captures copied into learner_sources; inspect the
    # complete output tree, including non-HTML payloads, independently of the
    # publisher's routing predicate.
    forbidden={'private','assessor','.venv','__pycache__'}
    for payload in sorted(base.rglob('*')):
        relative=payload.relative_to(base)
        if payload.is_file() and forbidden.intersection(relative.parts):
            errors.append(relative.as_posix()+': forbidden publication payload')
    ids={}
    for page in pages:
        with page_soup(page) as soup:
            ids[page]={t.get('id') for t in soup.find_all(id=True)}

    @lru_cache(maxsize=cache_limit)
    def target_for(origin, path):
        target=(origin/unquote(path)).resolve() if path else origin
        if target.is_dir(): target=target/'index.html'
        return target

    @lru_cache(maxsize=cache_limit)
    def exists(target):
        return target.exists()

    canonical={}
    canonical_order=[]
    canonical_errors={}
    for folder,records in [('modules',modules),('projects',projects)]:
        for key,record in records.items():
            path=base/folder/(key+'.html')
            canonical[path]=(folder,key,record)
            canonical_order.append(path)
    grouped_objectives={}
    objective_errors={}
    for index,row in enumerate(coverage):
        page=base/'modules'/(row['domain']+'.html')
        grouped_objectives.setdefault(page,[]).append((index,row))
    objective_text_checks=0
    renderer=MarkdownIt('commonmark').enable('table')
    for page in pages:
      with page_soup(page) as soup:
        article=soup.select_one('article.bd-article')
        if article and re.search(r'Licensed to|frederic\.ferragne|F:[/\\](?:Knowledge|Cours)[/\\]Library',article.get_text(' '),re.I):
            errors.append(str(page.relative_to(base))+': private source leakage')
        for tag in soup.find_all(['a','script','link','img']):
            value=tag.get('href') if tag.name in ['a','link'] else tag.get('src')
            if not value: continue
            parts=urlsplit(value)
            if parts.scheme in ['http','https','mailto','data','javascript']:
                if parts.scheme in ['http','https'] and (tag.name in ['script','img'] or tag.name=='link' and 'stylesheet' in tag.get('rel',[])):
                    remote_assets.add(value)
                continue
            if parts.scheme:
                errors.append(str(page.relative_to(base))+': unsupported URL '+value); continue
            # Fragment-only links belong to this page, not every page in its
            # directory; relative file destinations can share a directory cache.
            target=target_for(page.parent if parts.path else page,parts.path)
            if not target.is_relative_to(base):
                errors.append(str(page.relative_to(base))+': escaping local destination '+value); continue
            if not exists(target):
                errors.append(str(page.relative_to(base))+': missing local destination '+value); continue
            if parts.fragment and target.suffix=='.html' and unquote(parts.fragment) not in ids.get(target,set()):
                errors.append(str(page.relative_to(base))+': missing anchor '+value)
            checked+=1
        actual=compact(article.get_text(' ')) if article is not None else None
        if page in canonical:
            folder,key,record=canonical[page]
            local_errors=[]
            canonical_errors[page]=local_errors
            if article is None:
                local_errors.append(key+': missing rendered article')
            else:
              # Compare each complete canonical paragraph and literal code block.
              # Expected Markdown is bounded by this one canonical source page.
              expected=BeautifulSoup(renderer.render(normalize_prose(record['body'])),'html.parser')
              try:
                expected_tags=expected.find_all(['p','pre','h2','h3','li'])
                actual_code={tag.get_text().rstrip('\n') for tag in article.find_all('pre')}
                for tag in expected_tags:
                    if tag.name=='pre':
                        if tag.get_text().rstrip('\n') not in actual_code:
                            local_errors.append(key+': canonical literal code absent: '+tag.get_text()[:110])
                        prose_checks+=1; literal_code_checks+=1
                        continue
                    text=norm(tag.get_text(' '))
                    if not text or '$' in text or '{ref}' in text or '{download}' in text or '{term}' in text: continue
                    if compact(text) not in actual:
                        local_errors.append(key+': canonical prose/code absent: '+text[:110])
                    prose_checks+=1
              finally:
                release_soup(expected)
              meta=record['meta']
              if folder=='projects' and compact(str(meta['hours'])+' h') not in actual:
                  local_errors.append(key+': budget absent')
        for index,row in grouped_objectives.get(page,[]):
            local_errors=[]
            objective_errors[index]=local_errors
            if row['objective'].lower().replace('.','-') not in ids[page]:
                local_errors.append(row['objective']+': objective target absent')
            if article is None:
                local_errors.append(row['objective']+': rendered article absent')
                continue
            expected=BeautifulSoup(renderer.render(normalize_prose(row['text'])),'html.parser')
            try:
                expected_text=compact(expected.get_text(' '))
            finally:
                release_soup(expected)
            if expected_text not in actual:
                local_errors.append(row['objective']+': original objective text absent')
            objective_text_checks+=1

    # Keep the previous receipt's deterministic error order: destinations,
    # canonical module/project order, original coverage order, remote assets.
    for page in canonical_order:
        errors.extend(canonical_errors.get(page,[canonical[page][1]+': missing rendered page']))
    for index,row in enumerate(coverage):
        errors.extend(objective_errors.get(index,[row['objective']+': objective target absent']))
    if remote_assets: errors.append('Remote required assets: '+str(sorted(remote_assets)))
    if diagnostics is not None:
        diagnostics.update(page_count=len(pages),id_count=sum(len(values) for values in ids.values()),
                           page_passes=2,target_cache=target_for.cache_info()._asdict(),
                           existence_cache=exists.cache_info()._asdict())
    return {'passed':not errors,'pages':len(pages),'local_destinations_assets':checked,'canonical_text_checks':prose_checks,'literal_code_checks':literal_code_checks,'objective_text_checks':objective_text_checks,'remote_required_assets':sorted(remote_assets),'errors':errors}


def main():
    taxonomy,modules,projects,resources=load()
    report=inspect_html(ROOT/'build/html',modules,projects,read_json(ROOT/'curriculum/coverage.json')['records'])
    dump(ROOT/'reviews/html_checks.json',report)
    print({**report,'error_count':len(report['errors']),'errors':report['errors'][:12]})
    if report['errors']: raise SystemExit(1)

if __name__=='__main__': main()
