"""Register bounded passages actually read for the additive T07 implementation."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import dump,sha
folder=Path(__file__).parent
entries=[
 ('MAKE','GNU Make4.4.1 manual source','https://raw.githubusercontent.com/mirror/make/4.4.1/doc/make.texi','GNU Make4.4.1',
  ['Multiple Targets in a Rule: Rules with Grouped Targets, lines2997–3040'],
  'One invocation owns all grouped outputs; graph scheduling is not arbitrary-reader crash atomicity.'),
 ('PACKAGE','CMake packages manual','https://cmake.org/cmake/help/v3.31/manual/cmake-packages.7.html','CMake3.31.12 documentation',
  ['Creating a Package Configuration File, lines296–318','Creating Relocatable Packages, lines339–370'],
  'Installed target usage requirements and dependency rediscovery across relocated prefixes.'),
 ('COMPDB','Clang JSON Compilation Database Format Specification','https://clang.llvm.org/docs/JSONCompilationDatabase.html','Moving primary specification inspected2026-10-17',
  ['Format: command object and field contracts, lines169–198'],
  'Multiple entries per source, cwd, argv and output identity; generation and linking remain separate.'),
 ('NINJA','Ninja build system manual','https://ninja-build.org/manual.html','Moving primary manual inspected2026-10-17',
  ['Comparison to Make: command invalidation, lines74–84','The Ninja log, lines304–308','Build dependencies and path identity, lines503–517'],
  'Recorded command, discovered dependencies and exact path identity explain observed rebuilds.')]
resources=[]
for suffix,title,url,edition,locators,reason in entries:
    resources.append({'id':'R-LTOOL-'+suffix,'title':title,'url':url,'authors':['Primary project maintainers'],
      'year':2026,'edition':edition,'kind':'manual','access':'Named primary passages directly opened and read',
      'domains':['T07'],'role':'oracle','assignable':True,'reason':reason,
      'prerequisites':['C17 translation phases; original T07 prerequisite graph'], 'locators':locators,
      'inspection':{'date':'2026-10-17','method':'Read actual named primary passages before final implementation validation.',
        'scope':reason,'limits':'Bounded tool contract only; no whole manual, ISO-edition, hardware or independent-review claim.'}})
dump(folder/'resources.json',{'schema_version':1,'resources':resources})
dump(folder/'research_gate.json',{'status':'passage-inspected','domains':['T07'],
 'inspection_kind':'Author inspection, not external review',
 'research_input_sha256':{(folder/'resources.json').relative_to(ROOT).as_posix():sha(folder/'resources.json')}})
print({'inspected_resources':len(resources)})
