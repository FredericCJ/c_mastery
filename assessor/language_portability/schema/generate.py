"""Original bounded schema implementation; never imported by the learner scaffold."""
from pathlib import Path
import argparse,hashlib,json,re
FEATURES=['raw','framed','crc','datagram'];REQUIRES=[('crc','framed')];EXCLUSIVE=[('raw','framed','datagram')]
def encoded(v):return (json.dumps(v,sort_keys=True,separators=(',',':'),ensure_ascii=True)+'\n').encode('ascii')
def digest(v):return hashlib.sha256(encoded(v)).hexdigest()
def outputs(value):
 if type(value)!=dict or set(value)!={'version','features','internal_fast','fields','constraints'}:raise ValueError('schema keys')
 if type(value['version'])!=int or value['version'] not in [1,2]:raise ValueError('schema version')
 f=value['features']
 if type(f)!=dict or set(f)!=set(FEATURES) or any(type(x)!=bool for x in f.values()) or type(value['internal_fast'])!=bool:raise ValueError('Boolean feature policy')
 for a,b in REQUIRES:
  if f[a] and not f[b]:raise ValueError('requires:'+a+'->'+b)
 for group in EXCLUSIVE:
  if sum(f[n] for n in group)>1:raise ValueError('exclusive:'+','.join(group))
 rows=value['fields'];names=set();occupied=0;normalized=[]
 if type(rows)!=list or not 3<=len(rows)<=8:raise ValueError('field count3..8')
 for i,row in enumerate(rows):
  if type(row)!=dict or set(row) not in [{'name','offset','width'},{'name','offset','width','reserved'}]:raise ValueError('field keys:'+str(i))
  n=row['name'];off=row['offset'];width=row['width'];reserved=row.get('reserved',False)
  if type(n)!=str or not re.fullmatch('[a-z][a-z0-9_]{0,23}',n):raise ValueError('field name:'+str(i))
  if n in names:raise ValueError('duplicate field:'+n)
  if type(off)!=int or type(width)!=int or not 0<=off<16 or not 1<=width<=16-off:raise ValueError('field extent:'+n)
  if type(reserved)!=bool or reserved and value['version']!=2:raise ValueError('reserved version:'+n)
  if not reserved and n not in ['tag','length','flags']:raise ValueError('unknown active field:'+n)
  mask=(2**width-1)*2**off
  if occupied&mask:raise ValueError('overlap field:'+n)
  occupied|=mask;names.add(n);normalized.append(dict(name=n,offset=off,width=width,reserved=reserved))
 active=sorted([r for r in normalized if not r['reserved']],key=lambda r:r['name'])
 if {r['name'] for r in active}!={'tag','length','flags'}:raise ValueError('required active fields')
 constraints=value['constraints']
 if type(constraints)!=list or len(constraints)>8:raise ValueError('constraint count')
 widths={r['name']:r['width'] for r in normalized}
 for c in constraints:
  if type(c)!=dict or set(c)!={'kind','field','other'} or c['kind']!='width_at_least' or c['field'] not in widths or c['other'] not in widths:raise ValueError('constraint references')
  if widths[c['field']]<widths[c['other']]:raise ValueError('semantic width:'+c['field']+'>='+c['other'])
 canonical=dict(value,fields=sorted(normalized,key=lambda r:r['name']),constraints=sorted(constraints,key=lambda c:(c['kind'],c['field'],c['other'])))
 wire=digest(dict(features=f,active_fields=active));abi=digest(dict(record_revision=1,sequence=f['framed'],checksum=f['crc']));build=digest(canonical)
 config=['#ifndef LS_GENERATED_CONFIG_H','#define LS_GENERATED_CONFIG_H','#include <stdint.h>'];rules=['#ifndef LS_GENERATED_RULES_H','#define LS_GENERATED_RULES_H']
 for n in FEATURES:
  macro='LS_'+n.upper();config+=['#ifndef '+macro,'#define '+macro+' '+str(int(f[n])),'#elif '+macro+' != '+str(int(f[n])),'#error LS_CONFIGURATION_IDENTITY_'+n,'#endif'];rules+=['#ifndef '+macro,'#define '+macro+' 0','#endif','#if '+macro+' != 0 && '+macro+' != 1','#error LS_BOOLEAN_'+n,'#endif']
 for a,b in REQUIRES:rules+=['#if LS_'+a.upper()+' && !LS_'+b.upper(),'#error LS_REQUIRES_'+a+'_'+b,'#endif']
 for group in EXCLUSIVE:rules+=['#if '+' + '.join('LS_'+n.upper() for n in group)+' > 1','#error LS_EXCLUSIVE_TRANSPORT','#endif']
 rules+=['#endif'];config+=['#include "rules.h"']
 config+=['#define LS_WIRE_ID "'+wire+'"','#define LS_ABI_ID "'+abi+'"','#define LS_BUILD_ID "'+build+'"','typedef struct {uint32_t value;','#if LS_FRAMED','uint32_t sequence;','#endif','#if LS_CRC','uint32_t checksum;','#endif','} LsApiRecord;','#endif']
 constants=['#ifndef LS_GENERATED_FIELDS_H','#define LS_GENERATED_FIELDS_H','#include <stdint.h>']
 for row in sorted(normalized,key=lambda r:r['name']):
  n=row['name'].upper();off=row['offset'];width=row['width'];constants+=['#define LS_'+n+'_SHIFT '+str(off),'#define LS_'+n+'_MAX UINT32_C('+str(2**width-1)+')','#define LS_'+n+'_MASK UINT32_C('+str((2**width-1)*2**off)+')']
 allowed=sum((2**r['width']-1)*2**r['offset'] for r in active);constants+=['#define LS_ALLOWED UINT32_C('+str(allowed)+')','#endif']
 codec=['#ifndef LS_GENERATED_CODEC_H','#define LS_GENERATED_CODEC_H','#include "fields.h"','static int lsg_pack(const struct ls_fields *v,uint16_t *out){uint32_t w;','if(!v || !out)return -1;']
 codec+=['if(v->'+r['name']+'>LS_'+r['name'].upper()+'_MAX)return -1;' for r in active]
 terms=['(v->'+r['name']+'<<LS_'+r['name'].upper()+'_SHIFT)' if value['internal_fast'] else '(v->'+r['name']+'*UINT32_C('+str(2**r['offset'])+'))' for r in active]
 codec+=['w='+' | '.join(terms)+';','*out=(uint16_t)w;return 0;}','static int lsg_unpack(uint16_t w,struct ls_fields *out){struct ls_fields v;','if(!out || ((uint32_t)w & ~LS_ALLOWED))return -1;']
 codec+=['v.'+r['name']+'=((uint32_t)w & LS_'+r['name'].upper()+'_MASK)>>LS_'+r['name'].upper()+'_SHIFT;' for r in active]
 codec+=['*out=v;return 0;}','#endif']
 files={'config.h':('\n'.join(config)+'\n').encode('ascii'),'rules.h':('\n'.join(rules)+'\n').encode('ascii'),'fields.h':('\n'.join(constants)+'\n').encode('ascii'),'codec.h':('\n'.join(codec)+'\n').encode('ascii')}
 manifest=dict(schema_version=value['version'],enabled=sorted(n for n in f if f[n]),wire_identity=wire,abi_identity=abi,build_identity=build,canonical_input=canonical,constraint_graph=dict(requires=REQUIRES,exclusive=EXCLUSIVE),outputs={n:hashlib.sha256(b).hexdigest() for n,b in files.items()})
 files['manifest.json']=encoded(manifest);return files
def main():
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('output',type=Path);p.add_argument('--check',action='store_true');a=p.parse_args()
 try:
  result=outputs(json.loads(a.source.read_text(encoding='utf8')))
  if a.check:
   if any(not (a.output/n).is_file() or (a.output/n).read_bytes()!=b for n,b in result.items()):raise ValueError('stale generated bytes')
  else:
   a.output.mkdir(parents=True,exist_ok=True)
   for n,b in result.items():(a.output/n).write_bytes(b)
 except (ValueError,KeyError,TypeError) as e:print('LS_SCHEMA_REJECT '+str(e));return 1
 return 0
if __name__=='__main__':raise SystemExit(main())
