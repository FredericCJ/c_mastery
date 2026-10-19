int dispatch_index_authorized(unsigned request,unsigned selected,unsigned grant){return request<3&&selected==request&&(request==0||grant);}
