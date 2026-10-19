"""Verify rendered standalone anchors, objective text and local destinations."""
from collections import Counter
from urllib.parse import unquote,urlsplit
from bs4 import BeautifulSoup
from markdown_it import MarkdownIt
import re
from common import ROOT,read_json,dump,anchor,normalize_prose,find_glued
from check_html import norm, compact

def main():
    source=ROOT/'c_programming_mastery_curriculum.md'; text=source.read_text(encoding='utf-8')
    renderer=MarkdownIt('commonmark').enable('table')
    soup=BeautifulSoup(renderer.render(text),'html.parser')
    counts=Counter(t['id'] for t in soup.find_all(id=True)); errors=[]; checked=0
    for key,count in counts.items():
        if count!=1:errors.append('Duplicate anchor '+key)
    for tag in soup.find_all('a',href=True):
        parts=urlsplit(tag['href'])
        if parts.scheme in ['http','https','mailto']:continue
        if parts.fragment and not parts.path:
            if unquote(parts.fragment) not in counts:errors.append('Missing anchor '+tag['href'])
        elif parts.path:
            target=(ROOT/unquote(parts.path)).resolve()
            if not target.is_relative_to(ROOT.resolve()) or not target.is_file():errors.append('Missing/escaping file '+tag['href'])
        checked+=1
    actual=compact(soup.get_text(' ')); objective_checks=0
    for row in read_json(ROOT/'curriculum/coverage.json')['records']:
        if anchor(row['objective']) not in counts:errors.append('Missing objective anchor '+row['objective'])
        expected=compact(BeautifulSoup(renderer.render(normalize_prose(row['text'])),'html.parser').get_text(' '))
        if expected not in actual:errors.append('Missing objective text '+row['objective'])
        objective_checks+=1
    if '{ref}`' in text or '{download}`' in text or '{term}`' in text:errors.append('Unconverted publication role')
    doubled=len(re.findall(r'^\s*- - ',text,flags=re.M))+len(re.findall(r'^\s*\d+\. - ',text,flags=re.M))
    if doubled:errors.append('Doubled list markers rendered: '+str(doubled))
    glued=sorted(set(find_glued(text)))
    if glued:errors.append('Glued word/number forms rendered: '+str(glued[:40]))
    report={'passed':not errors,'local_links':checked,'unique_anchors':len(counts),'objective_checks':objective_checks,'bytes':source.stat().st_size,'doubled_bullets':doubled,'glued_forms':len(glued),'errors':errors}
    dump(ROOT/'reviews/markdown_checks.json',report); print(report)
    if errors:raise SystemExit(1)

if __name__=='__main__':main()
