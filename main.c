#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>

#define ERROR_CODE (-1)
#define SUCCESS_CODE 0
#define UNKNOWN_CHAR 2
#define ULONG_OVERFLOW 3
#define WRONG_NUMBER_COUNT 4
#define INIT_SIZE 512
#define BUF_LEN 1024

/*todo list
 * add buffer overflow handler
 * fix reallocing too much
 * space partitioning (needed?)
 * */

/* global variables and structs */
char buf[BUF_LEN]; //assuming one line never exceeds 1024 characters


typedef struct node_s{
    unsigned long x;
    unsigned long y;
    struct node_s *up,*down,*left,*right,*matchingPartner, *prev;
    short mark;
}node_t;

size_t graphSize;
node_t *graph;
unsigned long reallocCounter;
unsigned long lineNumber;
int debug;
int debug2;


/* function declarations */
void die(int error_num);
int getNextLine();
void checkLineSanity();
void parseLine(unsigned long lineNumber);
void parseInput();
int compareFunction(const void *a, const void *b);
void findNeighbors();
void findShortestAugmentationsOld();
void findShortestAugmentations();
void findLongerAugmentations();
int checkDone();
void invertAugmentingPath(node_t *rootNodeReverseDFS);
void printMatchedNodes();
static inline unsigned short rootSet(node_t *);

int main(){
    debug = 0;
    debug2 = 1;


	parseInput();
    qsort(graph,lineNumber, sizeof(node_t),&compareFunction);
    findNeighbors();
    findShortestAugmentationsOld();
    /*
    for(int i = 0; i < lineNumber; i++){
        printf("%ld    %ld",graph[i].x,graph[i].y);
        //if(graph[i].match)printf("  match =  %ld  %ld",((node_t *)(graph+i+2*sizeof(unsigned long)+(graph[i].match -1)))->x,((node_t *)(graph+i+2*sizeof(unsigned long)+(graph[i].match -1)))->y);
        printf("\n");
    }
     */
    if(debug) {
        if (checkDone())printf("found perfect matchings only using depth 1\n");
        else printf("not done, need to check for deeper augmentations\n");
        printf("Size in element count of provDataStruct = %ld\n", graphSize / sizeof(node_t));
        printf("total parsed lines = %ld\n", lineNumber);
    }
    findLongerAugmentations();

    if(debug2)printMatchedNodes();

	free(graph);
	return 0;
}

void printMatchedNodes(){
    for(int i = 0; i < lineNumber; i++){
        if(rootSet(&graph[i])&&graph[i].matchingPartner!=NULL){
            printf("%ld %ld;%ld %ld\n",graph[i].x,graph[i].y,graph[i].matchingPartner->x,graph[i].matchingPartner->y);
        }
    }
}

void invertAugmentingPath(node_t *rootNodeReverseDFS){
    node_t *curr = rootNodeReverseDFS;
    //node_t *tmp;
    /*currently only printing augmenting path*/
    do{
        /**/
        printf("%ld %ld\n",curr->x,curr->y);
        if(curr->prev == (node_t *)0x01)curr = curr->matchingPartner;
        else curr = curr->prev;

    }while(curr->prev!=(node_t *)0x01 || curr->matchingPartner!=NULL);
}

int checkDone(){
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner==NULL) {
            if(debug)printf("%ld   %ld is not matched!\n",graph[i].x,graph[i].y);
            else return 0;
            continue;
        }
    }
    return 1;
}

static inline unsigned short rootSet(node_t *node){
    return ((node->x%2)+(node->y%2))%2;
}

void findLongerAugmentations(){
    /*BFS starting w/ free nodes that are also part of the rootSet*/
    //FIXME queue can be allocated once, keep counter for actually used indices (whats the upper bound for the size of this queue?) -> can be done much more space efficient
    node_t **queue = malloc(sizeof(node_t *) * lineNumber);
    size_t queuesize = 0;
    if(debug)printf("starting BFS...\n");
    /*initializing the queue w/free nodes */
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL && !rootSet(graph+i)){
            if(graph[i].left != NULL) {
                queue[queuesize++] = graph[i].left;
                graph[i].left->prev = &graph[i];
                if(debug)printf("added %ld %ld . left to queue\n",graph[i].x,graph[i].y);
            }
            if(graph[i].right != NULL) {
                queue[queuesize++] = graph[i].right;
                graph[i].right->prev = &graph[i];
                if(debug)printf("added %ld %ld . right to queue\n",graph[i].x,graph[i].y);
            }
            if(graph[i].up != NULL) {
                queue[queuesize++] = graph[i].up;
                graph[i].up->prev = &graph[i];
                if(debug)printf("added %ld %ld . up to queue\n",graph[i].x,graph[i].y);
            }
            if(graph[i].down != NULL) {
                queue[queuesize++] = graph[i].down;
                graph[i].down->prev = &graph[i];
                if(debug)printf("added %ld %ld . down to queue\n",graph[i].x,graph[i].y);
            }
            graph[i].prev = (node_t *)0x01; //Flag, that this node cannot be part of any other augmenting paths
        }
    }
    if(debug){
        printf("BFS is starting with the following free nodes: \n");
        for(int i = 0; i < queuesize; i++){
            printf(" %ld   %ld\n",queue[i]->prev->x,queue[i]->prev->y);
        }
    }
    /*now alternately replacing matching partners to queue resp. adding the neighbours to the queue whilst removing the resp. node*/

    node_t *curr;
    size_t queuestart = 0;
    size_t oldsize = 0;
    size_t oldstart = 0;

    do {

            for(size_t i = queuestart; i < queuestart+queuesize;i++) { //replacing matching partners
                curr = queue[i];
                if (curr->matchingPartner == NULL){
                    if(debug)printf("we did it: found free node %ld  %ld, ending BFS\n",curr->x,curr->y);
                    invertAugmentingPath(curr);
                    return;
                }//we can end after this iteration since we found a free node
                else {
                    if(debug)printf("repacing: now %ld   %ld\n",curr->matchingPartner->x,curr->matchingPartner->y);
                    queue[i] = curr->matchingPartner;
                    curr->matchingPartner->prev = (node_t *) 0x01;
                }
            }


        oldsize = queuesize;
        oldstart = queuestart;
            for(size_t i = queuestart; i < oldstart+oldsize; i++) {//adding neighbours
                if(debug)printf("oldsize = %ld, index = %ld\n",oldsize,i);
                curr = queue[i];
                queuesize--;
                queuestart++;
                if (curr->left != NULL) {
                    if(rootSet(curr->left) && curr->left->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->left;
                        curr->left->prev = curr;
                        if (debug)printf("added %ld %ld . left to queue\n", curr->x, curr->y);
                    }
                }
                if (curr->right != NULL) {
                    if(rootSet(curr->right) && curr->right->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->right;
                        curr->right->prev = curr;
                        if (debug)printf("added %ld %ld . right to queue\n", curr->x, curr->y);
                    }
                }
                if (curr->up != NULL) {
                    if(rootSet(curr->up) && curr->up->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->up;
                        curr->up->prev = curr;
                        if (debug)printf("added %ld %ld . up to queue\n", curr->x, curr->y);
                    }
                }
                if (curr->down != NULL) {
                    if(rootSet(curr->down) && curr->down->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->down;
                        curr->down->prev = curr;
                        if (debug)printf("added %ld %ld . down to queue\n", curr->x, curr->y);
                    }
                }
            }
    }
    while(1);

}

void findShortestAugmentations(){
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL && !rootSet(&graph[i])){
            if(graph[i].left != NULL){
                if(graph[i].left->matchingPartner == NULL && rootSet(graph[i].left)){
                    printf("%ld %ld;%ld %ld\n",graph[i].left->x, graph[i].left->y,graph[i].x, graph[i].y);
                    graph[i].left->matchingPartner = graph[i].left->right;
                    graph[i].matchingPartner = graph[i].left;
                    continue;
                }
            }
            if(graph[i].down != NULL){
                if(graph[i].down->matchingPartner == NULL && rootSet(graph[i].down)){
                    printf("%ld %ld;%ld %ld\n",graph[i].down->x, graph[i].down->y,graph[i].x, graph[i].y);
                    graph[i].down->matchingPartner = graph[i].down->up;
                    graph[i].matchingPartner = graph[i].down;
                    continue;
                }
            }
            if(graph[i].right != NULL){
                if(graph[i].right->matchingPartner == NULL&& rootSet(graph[i].right)){
                    printf("%ld %ld;%ld %ld\n",graph[i].right->x, graph[i].right->y,graph[i].x, graph[i].y);
                    graph[i].right->matchingPartner = graph[i].right->left;
                    graph[i].matchingPartner = graph[i].right;
                    continue;
                }
            }
            if(graph[i].up != NULL){
                if(graph[i].up->matchingPartner == NULL&& rootSet(graph[i].up)){
                    printf("%ld %ld;%ld %ld\n",graph[i].up->x, graph[i].up->y,graph[i].x, graph[i].y);
                    graph[i].up->matchingPartner = graph[i].up->down;
                    graph[i].matchingPartner = graph[i].up;
                    continue;
                }
            }
        }
    }
}

void findShortestAugmentationsOld(){
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL){
            if(graph[i].left != NULL){
                if(graph[i].left->matchingPartner == NULL){
                    if(debug)printf("%ld %ld;%ld %ld\n",graph[i].left->x, graph[i].left->y,graph[i].x, graph[i].y);
                    graph[i].left->matchingPartner = graph[i].left->right;
                    graph[i].matchingPartner = graph[i].left;
                    continue;
                }
            }
            if(graph[i].down != NULL){
                if(graph[i].down->matchingPartner == NULL){
                    if(debug)printf("%ld %ld;%ld %ld\n",graph[i].down->x, graph[i].down->y,graph[i].x, graph[i].y);
                    graph[i].down->matchingPartner = graph[i].down->up;
                    graph[i].matchingPartner = graph[i].down;
                    continue;
                }
            }
        }
    }
}



void findNeighbors(){
    /*find y-neighbours*/
    for(int i = 0; i < lineNumber; i++){
        unsigned long x = graph[i].x;
        if(graph[i].down == NULL){
            /*the x-value has to remain the same*/
            for(int j = i+1; graph[j].x == x; j++){
                if(graph[j].y == graph[i].y-1){
                    //they are neighbors
                    graph[j].up = &graph[i];
                    graph[i].down = &graph[j];
                }
            }
        }
        if(graph[i].up == NULL){
            for(int j = i+1; graph[j].x == x; j++){
                if(graph[j].y == graph[i].y+1){
                    //neighbors
                    graph[j].down = &graph[i];
                    graph[i].up = &graph[j];
                }
            }
        }
        //fixme: are those checkings for out of bounds correct? pointer arithmetic...
        if(graph[i].left == NULL){
            /*now it's getting abit complicated, since we have to check the intervals of x-1 and x+1 for same y-values, so we first try to find
             * the starting index of these intervals and then check, this is also a point where some optimisation (preprocessing?) could come in handy*/
            node_t *xless = &graph[i];
            while(xless->x == x && (xless - graph))xless-=1;
            if(xless->x == x);//do nothing, since there is no interval w/ x-1 values
            else{
                while(xless->x == x-1 && (xless- graph)){
                    if(xless->y == graph[i].y){//found neighbours
                        xless->right = &graph[i];
                        graph[i].left = xless;
                    }
                    xless-=1;
                }
            }
            /*now reusing xless value, therefore naming inconsistent*/
            xless = &graph[i];
            while(xless->x == x && (xless-graph)<= lineNumber)xless+=1;
            if(xless->x == x);//do nothing, since there is no interval w/ x+1 values
            else{
                while(xless->x == x+1 && (xless-graph)<= lineNumber){
                    if(xless->y == graph[i].y){//found neighbours
                        xless->left = &graph[i];
                        graph[i].right = xless;
                    }
                    xless+=1;
                }
            }

        }

    }

}


int compareFunction(const void *a, const void *b){
    if(*(const unsigned long *)a > *(const unsigned long *)b){
        return 1;
    }
    if(*(const unsigned long *)a < *(const unsigned long *)b)return -1;
    else{
        const unsigned long *ay = (const unsigned long *)a + 1;
        const unsigned long *by = (const unsigned long *)b + 1;
        if(*ay>*by)return 1;
        if(*ay<*by)return -1;
        else {
            printf("double value detected!\n");
            printf("ax = %ld, bx = %ld, ay = %ld, by = %ld\n",*(unsigned long *)a, *(unsigned long *)b,*ay,*by);
            exit(-1);
        }
    }
}


void parseInput(){
    lineNumber = 0;
    while(getNextLine()!=ERROR_CODE){
        checkLineSanity();
        parseLine(lineNumber++);
    }
    if(debug)printf("%ld reallocs executed\n",reallocCounter);
}

/**
 * reads the next line (until '\ n') as ASCII-encoded byte-stream and handles errors
 * @returns 0 on success, 1 when reaching EOF, defined error codes when encountering errors
 * */
int getNextLine(){
    buf[BUF_LEN-1] = 'x';
    if(fgets(buf,sizeof(buf),stdin) == NULL)return ERROR_CODE;
    if(buf[BUF_LEN-1] != 'x'){
        //TODO:handle this!
        printf("fgets didnt get all chars in one line!\n");
        exit(-1);
    }
    return SUCCESS_CODE;
}

void checkLineSanity(){
    int pos = 0;
    char last = '\0';
    int numCount = 0;
    while(buf[pos]!='\0' && buf[pos]!='\n') {
        if (buf[pos] == ' ') {
            last = buf[pos++];
            continue;
        }
        if ((unsigned int) buf[pos] < 48 || (unsigned int) buf[pos] > 57) {
        printf("FOUND [%c]\n", buf[pos]);
        die(UNKNOWN_CHAR);
        }
        if(last == ' ' || last == '\0')numCount++;
        last = buf[pos++];
    }
    if(numCount!=2)die(WRONG_NUMBER_COUNT);
}

void parseLine(unsigned long lineNumber) {
    if (lineNumber == 0){
        graph = malloc(INIT_SIZE*sizeof(node_t));
        graphSize = INIT_SIZE*sizeof(node_t);
        reallocCounter = 0;
    }
    if(graphSize/sizeof(node_t)<=lineNumber*2){
        reallocCounter++;
        graphSize*=2;
        graph = realloc(graph,graphSize);
        if(graph == NULL){printf("realloc failed\n");die(-1);}
    }
    char *endptr = NULL;
    errno = 0;
    graph[lineNumber].x = strtoul(buf, &endptr, 10);

    //printf("%ld   ", graph[lineNumber].x);
    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);
    graph[lineNumber].y = strtoul(endptr,&endptr,10);

    //printf("%ld   \n", graph[lineNumber].y);

    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);
    /*set neighbor pointers to NULL*/
    memset((unsigned long*)&graph[lineNumber]+2,0,sizeof(node_t *)*6);
}

void die(int error_num){
    if(error_num==UNKNOWN_CHAR) {
        fprintf(stderr, "Unknown character, exiting...\n");
    }
    else if(error_num==ULONG_OVERFLOW){
        fprintf(stderr,"Number too big!\n");
    }
    else if(error_num==WRONG_NUMBER_COUNT){
        fprintf(stderr,"Too many or not enough Numbers in one Line, exiting...\n");
    }
    else{
        fprintf(stderr, "An unknown error occured, exiting...\n");
    }
    exit(ERROR_CODE);
}

