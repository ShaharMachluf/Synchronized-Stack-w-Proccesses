#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

void* malloc(size_t size) {
    void* head = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
    if(head == MAP_FAILED){
        printf("Mapping Failed\n");
        printf("%d", errno);
        return NULL;
    }
    return head;
}

void free(void* ptr, size_t size) {
    int err = munmap(ptr, size);
    if(err != 0){
        printf("UnMapping Failed\n");
    }
}
