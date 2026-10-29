"""Publish the same effective cards used by CLI initialization."""
from pathlib import Path
from cmastery import route


def prepare(source, supplements):
    active=route.load()
    if active is None:return [],supplements
    source=Path(source)
    old=source/'learning/route.md'
    if old.exists():
        previous=old.read_text(encoding='utf-8')
        historical=previous.replace('(v5-route)=','(v5-historical-route)=',1)
        (source/'learning/historical-route.md').write_text(historical,encoding='utf-8')
        supplements=[historical if s==previous else s for s in supplements]
    guide='(v5-route)=\n'+route.render_guide()
    guide+='\n```{toctree}\n:hidden:\n\n'+ '\n'.join('effective/'+key for key in active['order'])+'\n```\n'
    old.parent.mkdir(parents=True,exist_ok=True);old.write_text(guide,encoding='utf-8')
    folder=source/'learning/effective';folder.mkdir(exist_ok=True)
    for identifier in active['order']:
        text=route.render_node(identifier)
        (folder/(identifier+'.md')).write_text(text,encoding='utf-8')
    return ['learning/historical-route'],supplements+[guide]
