typedef struct stack{
    char data[1024];
    stack* next;
}node, *pnode;

void push(char* d, pnode *head);
void pop(pnode *head);
char* top(pnode *head);
