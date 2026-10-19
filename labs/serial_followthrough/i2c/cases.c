int ia_electrical(void);int ia_arbitration(void);int ia_transaction(void);
int ia_failures(void);int ia_recovery(void);int ia_late(void);
int ia_cases(unsigned n){switch(n){case 0:return ia_electrical();case 1:return ia_arbitration();case 2:return ia_transaction();case 3:return ia_failures();case 4:return ia_recovery();case 5:return ia_late();default:return 42;}}
