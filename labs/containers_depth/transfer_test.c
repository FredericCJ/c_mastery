#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTAINERS_TRANSFER_ASSERT:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
typedef struct{int value;unsigned released;} Task;
typedef struct{CDLink link;Task*payload;} Wrapper;
int main(void){
 int a[3]={0};CDRing ring;CDQueue queue;CHECK(cd_ring_init(&ring,a,3)==CD_OK);cd_queue_init(&queue);size_t written;
 for(int i=1;i<=3;++i){CHECK(cd_ring_push(&ring,&i,1,0,&written)==CD_OK&&cd_queue_push(&queue,i,0,0)==CD_OK);}
 int value=4;CHECK(cd_ring_push(&ring,&value,1,0,&written)==CD_FULL&&cd_queue_push(&queue,4,0,0)==CD_FULL);CDQueue before=queue;CHECK(cd_queue_push(&queue,4,1,1)==CD_NOMEM&&!memcmp(&before,&queue,sizeof queue));
 int left,right;CHECK(cd_ring_pop(&ring,&left,1)==CD_OK&&cd_queue_pop(&queue,&right)==CD_OK&&left==1&&right==1);CHECK(cd_ring_push(&ring,&value,1,0,&written)==CD_OK&&cd_queue_push(&queue,4,0,0)==CD_OK);
 for(int i=2;i<=4;++i)CHECK(cd_ring_pop(&ring,&left,1)==CD_OK&&cd_queue_pop(&queue,&right)==CD_OK&&left==i&&right==i);
 for(int i=1;i<=6;++i){CHECK(cd_queue_push(&queue,i,1,0)==CD_OK);}CHECK(queue.available==6&&cd_queue_push(&queue,7,1,0)==CD_FULL);
 int values[2]={1,2};CHECK(cd_ring_push(&ring,values,2,0,&written)==CD_OK);int batch[2]={8,9};CHECK(cd_ring_push(&ring,batch,2,0,&written)==CD_FULL&&ring.count==2);CHECK(cd_ring_push(&ring,batch,2,1,&written)==CD_OK&&written==1&&ring.count==3);
 CDSegmented segmented={0};int*first=NULL,*last=NULL;CHECK(cd_segmented_append(&segmented,10,&first)==CD_OK);for(int i=1;i<16;++i)CHECK(cd_segmented_append(&segmented,i,&last)==CD_OK);CHECK(first==&segmented.blocks[0][0]&&*first==10&&last==&segmented.blocks[3][3]&&cd_segmented_append(&segmented,99,&last)==CD_FULL);
 CDOwned owned={0};CDMoveOnly input={7,1};for(unsigned cancel=1;cancel<=2;++cancel){CHECK(cd_owned_append(&owned,&input,cancel)==CD_FAILED&&input.resource==1&&!owned.count);}CHECK(cd_owned_append(&owned,&input,0)==CD_OK&&!input.resource&&owned.values[0].resource==1);
 CDMoveOnly same_value={7,2};CHECK(cd_owned_append(&owned,&same_value,0)==CD_OK&&owned.values[0].value==owned.values[1].value&&owned.values[0].resource!=owned.values[1].resource);CDMoveOnly duplicated={9,1};CHECK(cd_owned_append(&owned,&duplicated,0)==CD_DUPLICATE&&duplicated.resource==1&&owned.count==2);
 /* Wrapper deallocation and payload destruction have distinct ownership. */
 CDList list;cd_list_init(&list);Task task={9,0};Wrapper*wrapper=calloc(1,sizeof*wrapper);CHECK(wrapper);wrapper->payload=&task;CHECK(cd_list_push(&list,&wrapper->link,0)==CD_OK&&cd_list_remove(&list,&wrapper->link)==CD_OK);CHECK(!task.released&&wrapper->payload==&task);free(wrapper);task.released=1;
 /* Save next before freeing the containing wrapper, never after. */
 for(unsigned i=0;i<3;++i){wrapper=calloc(1,sizeof*wrapper);CHECK(wrapper);wrapper->payload=&task;CHECK(cd_list_push(&list,&wrapper->link,0)==CD_OK);}unsigned releases=0;for(CDLink*node=list.head;node;){CDLink*next=node->next;CHECK(cd_list_remove(&list,node)==CD_OK);free((Wrapper*)node);++releases;node=next;}CHECK(releases==3&&!list.count);
 CDDeque deque={0};CHECK(cd_deque_push(&deque,2,0)==CD_OK&&cd_deque_push(&deque,1,1)==CD_OK&&cd_deque_push(&deque,3,0)==CD_OK&&cd_deque_push(&deque,9,1)==CD_FULL);CHECK(cd_deque_pop(&deque,&value,0)==CD_OK&&value==3&&cd_deque_pop(&deque,&value,1)==CD_OK&&value==1&&cd_deque_pop(&deque,&value,0)==CD_OK&&value==2&&cd_deque_pop(&deque,&value,0)==CD_EMPTY);
 /* Stack is the explicitly selected back/back projection; queue is back/front. */
 for(int i=1;i<=3;++i){CHECK(cd_deque_push(&deque,i,0)==CD_OK);}for(int i=3;i>=1;--i){CHECK(cd_deque_pop(&deque,&value,0)==CD_OK&&value==i);}
 puts("CONTAINERS_TRANSFER_PASS typed_queue_refill segmented_stable move_ownership cancellation wrapper_lifetime stack_deque");return 0;
}
