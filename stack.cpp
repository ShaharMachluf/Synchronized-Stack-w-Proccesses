// #include "part5.cpp"
#include "stack.hpp"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

void push(char* d, char *head){
    int i=0;
    for(int i=0; i<strlen(d); i++){
        *head = d[i];
        head++;
    }
    *head = '\0';
}

char* pop(char *head, char *bottom){
    head-=2;
    while(head != bottom && *head != '\0'){
        head--;
    }
    if(*head == '\0'){
        head++;
    }
    return head;
}

char* top(char *head, char* bottom){
    char* curr = head;
    curr-=2;
    while(curr != bottom && *curr != '\0'){
        curr--;
    }
    if(*curr == '\0'){
        curr++;
    }
    return curr;
}
