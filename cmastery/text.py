"""Render-time prose normalization shared by the publisher, the lints and the learner CLI.

Canonical teaching text and inspected locators are identity-pinned, so a
generation defect that glued a word to a number ("printed10", "Problem2",
"at3") is repaired when text is rendered, never by rewriting the pinned bytes.
The lints apply the same rules, so a published page cannot keep a form that
the normalizer would have repaired.
"""
from __future__ import annotations
import re

MONTHS = ('January|February|March|April|May|June|July|August|September|October|November|December')
WORDS = ('at|to|the|is|are|and|of|in|on|or|than|least|most|exactly|within|after|before|printed|page|pages|'
         'section|sections|table|tables|chapter|chapters|figure|figures|theorem|lemma|remarks|remark|problem|'
         'modulo|maximum|minimum|step|steps|line|lines|from|with|version|returns|counts|count|by|as|for|per|'
         'about|around|under|into|onto|between|every|each|item|items|entry|entries|offset|offsets|byte|bytes|'
         'bit|bits|row|rows|column|columns|part|parts|clause|clauses|paragraph|paragraphs|note|notes|annex|'
         'lecture|lectures|unit|units|module|modules|week|weeks|day|days|hour|hours|then|hence|penalty|'
         'polynomial|match|second|capture|rev|revision|'
         'Chapter|Section|Sections|Problem|Theorem|Remarks|Remark|Tables|Table|Figure|Figures|Printed|Page|Pages|'
         'Volume|Register|Rev|Revision|Order|PDF|RFC|ISO|IEC|IEEE|' + MONTHS)
AFTER_DIGIT = ('payload|then|full|frames|bytes|samples|items|entries|packets|records|tasks|threads|processes|'
               'pages|lines|words|cycles|' + MONTHS)
_GLUED = re.compile(r'(?<![\w/#.&\-])(' + WORDS + r')(\d+)')
_AFTER = re.compile(r'(?<=\d)(' + AFTER_DIGIT + r')(?![\w-])')
_MANPAGES = re.compile(r'(man-pages)(\d)')
# Prose segments exclude fenced code, inline code, link targets and HTML anchors.
_SKIP = re.compile(r'```.*?```|~~~.*?~~~|`[^`\n]*`|\]\([^)\n]*\)|<a id="[^"]*"></a>|https?://\S+', re.S)


def _normalize_segment(segment):
    segment = _GLUED.sub(r'\1 \2', segment)
    segment = _AFTER.sub(r' \1', segment)
    return _MANPAGES.sub(r'\1 \2', segment)


def normalize_prose(text):
    """Insert the missing space in glued word/number forms outside code and links."""
    if not text or not isinstance(text, str):
        return text
    out = []
    position = 0
    for match in _SKIP.finditer(text):
        out.append(_normalize_segment(text[position:match.start()]))
        out.append(match.group(0))
        position = match.end()
    out.append(_normalize_segment(text[position:]))
    return ''.join(out)


def find_glued(text):
    """Return the glued forms a normalizer pass would still repair (lint helper)."""
    found = []
    position = 0
    for match in _SKIP.finditer(text):
        found += _findall(text[position:match.start()])
        position = match.end()
    return found + _findall(text[position:])


def _findall(segment):
    items = [m.group(0) for m in _GLUED.finditer(segment)]
    items += [m.group(0) for m in _AFTER.finditer(segment)]
    return items + [m.group(0) for m in _MANPAGES.finditer(segment)]


def strip_bullet(value):
    """Check strings extracted from '- Challenge…' lines must not render as '- - …'."""
    value = str(value)
    return value[2:].lstrip() if value.startswith('- ') else value
