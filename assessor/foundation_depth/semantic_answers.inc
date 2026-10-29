    const enum fd_class expected[]={FD_DEFINED,FD_DEFINED,FD_UNDEFINED,FD_UNSPECIFIED,
        FD_IMPLEMENTATION_DEFINED,FD_UNDEFINED,FD_UNDEFINED,FD_CONSTRAINT,
        FD_UNDEFINED,FD_DEFINED};
    for (unsigned i=0;i<FD_CASE_COUNT;i++) CHECK(fd_verdict((enum fd_case)i)==expected[i]);
