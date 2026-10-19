#include "logging.h"
#ifndef MPL_MUTATION
#define MPL_MUTATION 0
#endif
MplField mpl_number(uint32_t value) { MplField f={MPL_NUMBER,value,NULL,0};return f; }
MplField mpl_bytes(const unsigned char *bytes,size_t length) { MplField f={MPL_BYTES,0,bytes,length};return f; }
int mpl_write(MplSink *sink,const MplSite *site,const MplField *fields,size_t count) {
    MplRecord staged={0};size_t i,j;
    if(!sink || count>4 || (count && !fields) || sink->count>4) return MPL_INVALID;
    if(sink->count==4) return MPL_MUTATION==6 ? MPL_OK : MPL_FULL;
    if(site) {
        if(!site->file || !site->line) return MPL_INVALID;
        for(i=0;i<sizeof staged.file && site->file[i];++i) staged.file[i]=site->file[i];
        if(i==sizeof staged.file) return MPL_RANGE;
        staged.has_site=1;staged.line=MPL_MUTATION==5 ? 0 : site->line;
    }
    if(MPL_MUTATION==2) ++sink->count;
    staged.count=count;
    for(i=0;i<count;++i) {
        staged.fields[i].kind=fields[i].kind;
        if(fields[i].kind==MPL_NUMBER) staged.fields[i].number=fields[i].number;
        else if(fields[i].kind==MPL_BYTES) {
            if(fields[i].length>8) return MPL_RANGE;
            if(fields[i].length && !fields[i].bytes) return MPL_INVALID;
            staged.fields[i].length=fields[i].length;
            for(j=0;j<fields[i].length;++j) if(MPL_MUTATION!=4 || j==0) staged.fields[i].bytes[j]=fields[i].bytes[j];
        } else return MPL_INVALID;
    }
    if(MPL_MUTATION==2) --sink->count;
    sink->records[sink->count]=staged;++sink->count;return MPL_OK;
}
static const MplSite *site_arg(MplSite *site) { return site->file || site->line ? site : NULL; }
int mpl_log0(MplSink *s,MplSite p) { return mpl_write(s,site_arg(&p),NULL,0); }
int mpl_log1(MplSink *s,MplSite p,MplField a) { return mpl_write(s,site_arg(&p),&a,1); }
int mpl_log2(MplSink *s,MplSite p,MplField a,MplField b) { MplField f[]={a,b};return mpl_write(s,site_arg(&p),f,2); }
int mpl_log3(MplSink *s,MplSite p,MplField a,MplField b,MplField c) { MplField f[]={a,b,c};return mpl_write(s,site_arg(&p),f,3); }
int mpl_log4(MplSink *s,MplSite p,MplField a,MplField b,MplField c,MplField d) { MplField f[]={a,b,c,d};return mpl_write(s,site_arg(&p),f,4); }
