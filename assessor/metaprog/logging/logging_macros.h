#ifndef CMASTERY_MPL_MACROS_H
#define CMASTERY_MPL_MACROS_H
#include "logging.h"
#ifndef MPL_MUTATION
#define MPL_MUTATION 0
#endif
#define MPL_SITE_NONE ((MplSite){NULL,0})
#define MPL_SITE_HERE ((MplSite){__FILE__,__LINE__})
#define MPL_SELECT(_1,_2,_3,_4,_5,NAME,...) NAME
#if MPL_MUTATION==1
#define MPL_ONE(s,p,a) ((void)(a),mpl_log1((s),(p),(a)))
#else
#define MPL_ONE(s,p,a) mpl_log1((s),(p),(a))
#endif
#define MPL_DISPATCH(s,p,...) MPL_SELECT(__VA_ARGS__,mpl_log_arity_error,mpl_log4,mpl_log3,mpl_log2,MPL_ONE,unused)((s),(p),__VA_ARGS__)
#define MPL_LOG(s,...) MPL_DISPATCH((s),MPL_SITE_NONE,__VA_ARGS__)
#define MPL_LOG_AT(s,...) MPL_DISPATCH((s),MPL_SITE_HERE,__VA_ARGS__)
#if MPL_MUTATION==3
#define MPL_LOG0(s) mpl_log1((s),MPL_SITE_NONE,mpl_number(0))
#else
#define MPL_LOG0(s) mpl_log0((s),MPL_SITE_NONE)
#endif
#define MPL_LOG_AT0(s) mpl_log0((s),MPL_SITE_HERE)
#endif
