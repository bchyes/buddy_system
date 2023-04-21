#include "buddy.h"
#define NULL ((void *)0)
int is_power_of_2(int x) {
    return !(x & (x - 1));
}
int left_child(int index){
    return ((index << 1) | 1); 
}
int right_child(int index){
    return ((index << 1) + 2);
}
int parent(int index){
    return (((index + 1) >> 1) - 1);
}
int pow(int x,int k){
    if (k==0) return 1;
    int p = pow(x,k>>1);
    if (k % 2){
        return p*p*x;
    } else {
        return p*p;
    }
}
int log(int x){
    if (x==1) return 0;
    return log(x>>1)+1;
}
int init_page(void *p, int pgcount){
    self = (struct buddy_system*)malloc(2 * pgcount * sizeof(int) + sizeof(struct buddy_system));
    self -> rank = log(pgcount) + 1;
    int node_rank = log(pgcount) + 2;
    for (int i = 0;i < 2 * pgcount - 1;i++){
        if (is_power_of_2(i + 1)) {
            node_rank--;
        }
        self -> longest[i] = node_rank;
    }
    //printf("%d\n",(int) p);
    self -> address = p;
    return OK;
}
void *alloc_pages(int rank){
    if (self -> longest[0] < rank) return (void *) -ENOSPC;
    //printf("%d\n",1);
    int index = 0;
    //printf("%d\n",self -> rank);
    //printf("%d\n",self -> longest[0]);
    for (int rank_ = self -> rank;rank_ != rank;rank_--){
        if (self -> longest[left_child(index)] >= rank){
            index = left_child(index);
        } else {
            index = right_child(index);
        }
        //printf("%d\n",index);
    }
    //printf("%d\n",1);
    self -> longest[index] = 0;
    //printf("%d\n",index);
    int address = (index + 1) * pow(2,rank - 1) - pow(2,self -> rank - 1);
    //printf("%d\n",address);
    while (index){
        index = parent(index);
        if (self -> longest[left_child(index)] > self -> longest[right_child(index)])
            self -> longest[index] = self -> longest[left_child(index)];
        else 
            self -> longest[index] = self -> longest[right_child(index)];
        //printf("%d\n",index);
    }
    //printf("%d\n",self -> longest[16383]);
    //printf("%d\n",self -> longest[32767]);
    //printf("%d\n",self -> longest[32768]);
    //printf("%d\n",self -> address);
    return (void *) (self -> address + address * 1024 * 4);
}
int return_pages(void *p){
    if (p == NULL) return -EINVAL;
    //if ((int)p - (int)(self -> address) % (1024 * 4)) return -EINVAL;
    if ((int)p - (int)(self -> address) < 0 || (int)p - (int)(self -> address) > pow(2,self -> rank - 1) * 1024 * 4) return -EINVAL;
    int offset = ((int)p - (int)(self -> address)) / (1024 * 4);
    int index = offset + pow(2,self -> rank - 1) - 1;
    int node_rank = 1;
    for (; self -> longest[index] != 0;index = parent(index)){
        node_rank++;
        if (index == 0){
            break;
        }
    }
    if (self -> longest[index] == 0)
        self -> longest[index] = node_rank;
    while (index){
        index = parent(index);
        node_rank++;
        int left_longest = self -> longest[left_child(index)];
        int right_longest = self -> longest[right_child(index)];
        if (left_longest == right_longest && left_longest + 1 == node_rank){
            self -> longest[index] = node_rank;
        } else {
            if (left_longest > right_longest){
                self -> longest[index] = left_longest;
            } else {
                self -> longest[index] = right_longest;
            }
        }
    }
    return OK;
}
int query_ranks(void *p){
    int offset = ((int)p - (int)(self -> address)) / (1024 * 4);
    int index = offset + pow(2,self -> rank - 1) - 1;
    int rank = 1;
    while (self -> longest[index] && index){
        index = parent(index);
        rank++;
    }
    return rank;
}
int query_page(int index,int node_rank,int rank){
    int ans = 0;
    int left_rank = self -> longest[left_child(index)];
    int right_rank = self -> longest[right_child(index)];
    if (rank == left_rank && left_rank == node_rank) ans++;
    //if (index == 8191 && left_rank == 2) printf("%d\n",node_rank);
    if (left_rank >= rank && left_rank < node_rank) ans += query_page(left_child(index),node_rank - 1,rank);
    if (rank == right_rank && right_rank == node_rank) ans++;
    if (right_rank >= rank && right_rank < node_rank) ans += query_page(right_child(index),node_rank - 1,rank);
    return ans;
}
int query_page_counts(int rank){
    if (rank == self -> longest[0] && self -> longest[0] == self -> rank) return 1;
    if (rank < self -> rank && self -> longest[0] == self -> rank) return 0;
    if (self -> longest[0] < rank) return 0;
    return query_page(0,self -> rank - 1,rank);
}
/*int query_page_counts(int rank){
    //if (rank == self -> rank && self -> longest[0] == self -> rank) return 1;
    //printf("%d\n",self -> longest[0]);
    //printf("%d\n",self -> rank);
    if (rank == self -> longest[0]) return 1;
    if (rank < self -> rank && self -> longest[0] == self -> rank) return 0;
    if (self -> longest[0] < rank) return 0;
    int index = 0;
    int node_rank = self -> rank - 1;
    while (node_rank >= rank){
        int left_rank = self -> longest[left_child(index)];
        int right_rank = self -> longest[right_child(index)];
        if (left_rank == rank || right_rank == rank) return 1;
        if (left_rank < rank || left_rank == node_rank){
            if (right_rank < rank || right_rank == node_rank) return 0;
            index = right_child(index);
        } else {
            index = left_child(index);
        }
        node_rank--;
    }
    return OK;
}*/