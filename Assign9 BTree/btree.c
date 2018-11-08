#include <stdio.h>
#include <stdlib.h>

#define MINDIG 2
#define MAXKEYS 3
//MINDIG*2-1
#define MAXPTRS 4
//MINDIG*2 

struct node {
    int count;
    int key[MAXKEYS+1];
    struct node * (ptr[MAXPTRS+1]);
    struct node *par;
};

struct node * createNode(int data)
{
    struct node *curr = (struct node *) malloc(sizeof(struct node));
    curr->count = 1;
    curr->key[0] = data;
    curr->par = NULL;
    int i;
    for (i = 0; i < (MINDIG*2); i++) {
        curr->ptr[i] = NULL;
    }
    
    return curr;
}
void insert(struct node **root, int data);

void printinorder(struct node *root, int *c) 
{
    if (root != NULL) {
        (*c) += 1;
        int i;
        for (i = 0; i < root->count; i++) {
            printinorder(root->ptr[i], c);
            printf("%d ", root->key[i]);
        }

        printinorder(root->ptr[root->count], c);
    }
}

void insertInNode(struct node **root, int data) 
{
    int i = 0, j, n = (*root)->count;
    // printf("n : %d\n", n);

    for (i = n-1; i >= 0; i--) {
        if ((*root)->key[i] > data) {
            (*root)->key[i+1] = (*root)->key[i];
            (*root)->ptr[i+2] = (*root)->ptr[i+1];
        } else {
            break;
        }
    }
    
    (*root)->key[i+1] = data;
    (*root)->ptr[i+2] = NULL;
    (*root)->count = (*root)->count + 1;
}

int newSearchIndex(struct node *root, int data)
{
    int i = 0, n = root->count;
    for (i = 0; i < n; i++) {
        if (root->key[i] > data) {
            return i;
        }
    } 

    return n;
}


void printnode(struct node *root) 
{
    int i;
    for (i = 0; i < root->count; i++) {
        if (root->ptr[i] == NULL) {
            printf("0 ");
        } else {
            printf("1(%d)(%d) ", root->ptr[i]->count, root->ptr[i]->key[0]);
        }

        printf(" %d\n", root->key[i]);
    }

    if (root->ptr[root->count] == NULL) {
        printf("0 \n\n");
    } else {
        printf("1(%d)(%d) \n\n", root->ptr[root->count]->count, root->ptr[root->count]->key[0]);
    }
}

void findParent(struct node *root, struct node **par, int key) 
{
    if (root != NULL) {
        int i;
        int flag = -1;

        for (i = 0; i < root->count; i++) {
            if (root->key[i] == key) {
                flag = i;
                break;
            }
        }

        if (flag == -1) {
            *par = root;
            findParent(root->ptr[newSearchIndex(root, key)], par, key);
        }
    }
}



int main(int argc, char const *argv[])
{
    struct node *root = NULL;

    // int in[] = {10, 15, 20, 12, 26, 18, 29};
    int in[] = {5, 3, 21, 9, 1, 13, 2, 7, 10, 12, 4, 8};
    int nodeCount = 0;
    int i;
    int n = 12;
    // int n = sizeof(in) / sizeof(in[0]);
    for (i = 0; i < n; i++) {
        printf("key:%d\n", in[i]);
        insert(&root, in[i]);
        printf("key:%d  r.count: %d\n", in[i], root->count);
        printnode(root);
        printf("Inorder : ");
        printinorder(root, &nodeCount);
        printf("\n\n");
    }

    printf("RootNode : \n");
    printf("RootNode.count : %d\n", root->count);
    printnode(root);

    nodeCount = 0;
    printf("Inorder :");
    printinorder(root, &nodeCount);
    printf("\nNodes: %d\n", nodeCount);

    return 0;
}

struct node * split(struct node **root, struct node **curr, struct node **par)
{
    struct node *newNode = (struct node *) malloc(sizeof(struct node));
    int i;
    int tmp = -1;
    for (i = MINDIG; i < MAXPTRS; i++) {
        ++tmp;
        newNode->ptr[tmp] = (*curr)->ptr[i];
        (*curr)->ptr[i] = NULL;
    
        if (i < MAXKEYS) {
            newNode->key[tmp] = (*curr)->key[i];
            (*curr)->key[i] = -1;
        }
    }

    newNode->count = tmp;
    (*curr)->count = MINDIG-1;

    
    if ((*par) ==  NULL) {
        struct node *newRoot = (struct node *) malloc(sizeof(struct node));
        
        printf("1\n");
        newRoot->ptr[1] = newNode;
        newRoot->ptr[0] = (*curr);
        newRoot->key[0] = (*curr)->key[MINDIG-1];
        newRoot->count = 1;

        return newRoot;
    } else {
        insertInNode(par, (*curr)->key[MINDIG-1]);
        int newPositionOfKey = newSearchIndex(*par, (*curr)->key[MINDIG-1]);
        (*par)->ptr[newPositionOfKey + 1] = newNode;

        return (*par);
    }
    
}

void insert(struct node **root, int data) 
{
    if ((*root) == NULL) {
        struct node *curr = createNode(data);
        *root = curr;
    } else  {
        struct node *par = NULL;
        struct node *newNode = NULL;
        struct node *nextNode = NULL;

        if ((*root)->count >= MAXKEYS) {
            *root = split(root, root, &par);
        }

        struct node *curr = *root;

        while (curr->ptr[0] != NULL) {

            nextNode = curr->ptr[newSearchIndex(curr, data)];

            if (nextNode->count >= MAXKEYS) {
                curr->ptr[newSearchIndex(curr, data)] = split(root, &nextNode, &par);
                
                // printf("par : \n");
                // printf("par.count : %d\n", par->count);
                // printnode(par);
            }

            curr = curr->ptr[newSearchIndex(curr, data)];

        }

        insertInNode(&curr, data);
        printf("Inserted node for key : %d \n", data);
        printf("Inserted.count : %d\n", curr->count);
        printnode(curr);
    }
}
