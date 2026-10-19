#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTAINERS_GRAPH_IMPORT_ASSERT:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
int main(void){CDEdge edges[3]={{0,1,7},{0,2,5},{2,1,9}};CDCSR owner;CHECK(cd_csr_build(&owner,4,edges,3,0)==CD_OK);const CDCSR*import=&owner;CDCSR before=owner;uint32_t distances[8];CHECK(cd_csr_bfs(import,0,distances)==CD_OK&&import==&owner&&!memcmp(&before,&owner,sizeof owner));CHECK(distances[0]==0&&distances[1]==1&&distances[2]==1&&distances[3]==UINT32_MAX);CHECK(cd_csr_bfs(import,2,distances)==CD_OK&&distances[1]==1&&distances[2]==0&&distances[0]==UINT32_MAX);
 uint32_t saved[8];memcpy(saved,distances,sizeof saved);CHECK(cd_csr_bfs(import,4,distances)==CD_INVALID&&!memcmp(saved,distances,sizeof saved));owner.offset[1]=4;CHECK(cd_csr_bfs(import,0,distances)==CD_INVALID&&!memcmp(saved,distances,sizeof saved));owner.offset[1]=UINT32_MAX;CHECK(cd_csr_bfs(import,0,distances)==CD_INVALID&&!memcmp(saved,distances,sizeof saved));puts("CONTAINERS_GRAPH_IMPORT_PASS const_live_CSR_original_BFS_owner_unchanged");return 0;}
