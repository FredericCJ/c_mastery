"""Declared fields only; this function never infers execution or safety."""
def classify(v):
 h=v['headers'];s=set(v['symbols'])
 return dict(pie_marked=v['elf_type']=='DYN' and v['pie_flag'],relro_segment=any(x['type']=='GNU_RELRO' for x in h),eager_binding=v['bind_now'],nx_stack_marked=any(x['type']=='GNU_STACK' and 'E' not in x['flags'] for x in h),wx_load_segment=any(x['type']=='LOAD' and 'W' in x['flags'] and 'E' in x['flags'] for x in h),guard_site_dependency='__stack_chk_fail' in s,checked_call_present='__memcpy_chk' in s,aslr_policy=v['aslr_policy'],base_changed=len(set(v['bases']))>1)
