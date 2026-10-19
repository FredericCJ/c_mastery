"""Download public primary-source captures privately; no external code execution."""
import hashlib,json,pathlib,sys,urllib.request
ROOT=pathlib.Path(__file__).resolve().parents[4]
HERE=pathlib.Path(__file__).resolve().parent
CACHE=ROOT/'build/corpus_index_sources'
SOURCES=[
('fenwick','https://static.aminer.org/pdf/PDF/001/073/976/a_new_data_structure_for_cumulative_frequency_tables.pdf',[1,3,4,5,6,7]),
('fenwick_correction','https://onlinelibrary.wiley.com/doi/epdf/10.1002/spe.4380240706',[0]),
('interval','https://www.cs.virginia.edu/~robins/cs4102/slides/Cormen%20et%20al%20Slides/PDF/11-Augmenting%20Data%20Structures.pdf',list(range(8,25))),
('suffix','https://niplav.site/doc/cs/suffarr/suffix_arrays_a_new_method_for_on_line_string_searches_manber_myers_1990.pdf',[1,2,3,4,5]),
('segment','https://atcoder.github.io/ac-library/production/document_en/segtree.html',[]),
('lazy','https://atcoder.github.io/ac-library/production/document_en/lazysegtree.html',[])]
def main():
 sys.stdout.reconfigure(encoding='utf8');CACHE.mkdir(parents=True,exist_ok=True);rows=[]
 for name,url,pages in SOURCES:
  p=CACHE/(name+('.pdf' if pages else '.html'));row=dict(id=name,url=url,pages_zero_based=pages)
  try:
   if not p.exists():p.write_bytes(urllib.request.urlopen(urllib.request.Request(url,headers={'User-Agent':'Mozilla/5.0'}),timeout=20).read())
   row.update(status='captured',path=p.relative_to(ROOT).as_posix(),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size)
  except Exception as exc:row.update(status='capture-failed',error=str(exc))
  rows.append(row)
 (HERE/'source_access.json').write_text(json.dumps(dict(sources=rows,limits='Capture alone is not inspection. sources.md identifies the passages actually read.'),indent=2)+'\n')
 print(json.dumps(rows,indent=2))
 if len(sys.argv)>1:
  sys.path.insert(0,str(ROOT/'build/pdf_reader_deps'));import fitz
  for row in rows:
   if row['id']!=sys.argv[1] or row['status']!='captured':continue
   doc=fitz.open(ROOT/row['path'])
   for page in row['pages_zero_based']:print('PAGE',page+1,doc[page].get_text())
if __name__=='__main__':main()
