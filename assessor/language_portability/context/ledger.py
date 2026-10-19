"""Original bounded history operation, separate from source-status interpretation."""
from copy import deepcopy
FIELDS={'revision','status','applies_to','source'}
def append_revision(history,incoming):
    if not isinstance(history,list) or not history or not isinstance(incoming,dict):raise ValueError('shape')
    for i,row in enumerate(history):
        if not isinstance(row,dict) or set(row)!=FIELDS or type(row['revision']) is not int or row['revision']!=i+1:raise ValueError('history revision')
        if row['status'] not in {'open','resolved'} or not all(isinstance(row[k],str) and row[k] for k in ['applies_to','source']):raise ValueError('history authority')
    if set(incoming)!=FIELDS or type(incoming['revision']) is not int or incoming['revision']!=len(history)+1:raise ValueError('next revision')
    if incoming['status'] not in {'open','resolved'} or not all(isinstance(incoming[k],str) and incoming[k] for k in ['applies_to','source']):raise ValueError('authority')
    return deepcopy(history)+[deepcopy(incoming)]
