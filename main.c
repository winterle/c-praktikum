#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

//#define DEBUG
//#define NOOUT
/**
 * @author Leander Winter
 * */

#define ERROR_CODE (-1)
#define SUCCESS_CODE 0
#define UNKNOWN_CHAR 2
#define ULONG_OVERFLOW 3
#define WRONG_NUMBER_COUNT 4
#define DOUBLE_VALUE 5
#define INIT_SIZE 512
#define BUF_LEN 1024

/*todo list
 * add buffer overflow handler --> done, but weird --> rewrite?
 * further improve neighbor finding
 * improve the queue used in findLongerAugmentations(); pathes of same length in one iteration
 * can reset() be done more efficiently? --> alternate flag values?
 * */

/* global variables and structs */
char buf[BUF_LEN];


typedef struct node_s{
    unsigned long x;
    unsigned long y;
    struct node_s *up,*down,*left,*right,*matchingPartner, *prev;
}node_t;

size_t graphSize;
node_t *graph;
unsigned long lineNumber;
/*workaround for qsort allocation, so we have to free and exit after qsort finished when detecting duplicate nodes*/
int dup;


/* function declarations */
void die(int error_num);
int getNextLine();
void checkLineSanity();
void parseLine(unsigned long lineNumber);
void parseInput();
int compareFunctionX(const void *a, const void *b);
int compareFunctionY(const void *a, const void *b);
void findNeighbors();
void findShortestAugmentations();
int findLongerAugmentations(node_t **);
int checkDone();
void invertPath(node_t *rootNodeReverseDFS);
static inline unsigned short rootSet(node_t *);
void reset();

void printMatched();


int main(){
    dup=0;

	parseInput();
    /*empty input*/
    if(lineNumber == 0){
        free(graph);
        exit(0);
    }
    qsort(graph,lineNumber, sizeof(node_t), &compareFunctionY);
    if(dup)die(DOUBLE_VALUE);

    findNeighbors();//implicitly calls compareFunctionX
    if(dup)die(DOUBLE_VALUE);
    #ifdef DEBUG
    printf("neighbors done\n");
    #endif

    findShortestAugmentations();
    #ifdef DEBUG
    printf("shortest done\n");
    #endif

    node_t **queue = malloc(sizeof(node_t *) * lineNumber);
    while(findLongerAugmentations(queue)>0)reset();
    free(queue);

    if(checkDone()){
        #ifndef NOOUT
        //printMatchedNodes();
        printMatched();
        #else
        printf("found solution\n");
        #endif
    }
    else printf("None\n");

	free(graph);
	return 0;
}


void printMatched(){
    /*trying to make the least syscalls (printf and further) as possible*/

    unsigned int line_size = 100;
    unsigned long max_lines = lineNumber/2;
    unsigned char *output = malloc(sizeof(unsigned char)*line_size*max_lines);
    unsigned char *position = output;
    unsigned int lineno = 0; //range 0-1023
    printf("linenumber = %ld\n",lineNumber);
    for(unsigned int i = 0; i < lineNumber; i++){
        if(rootSet(&graph[i])&&graph[i].matchingPartner!=NULL){
            lineno++;

            unsigned long x = graph[i].x;
            for(int j = 9; j > -1; j--){
                *(position+j) = (x%10)+48;
                x = x - (x%10);
                x=x/10;
            }
            *(position+10) = ' ';
            position+=11;
            unsigned long y = graph[i].y;
            for(int j = 9; j > -1; j--){
                *(position+j) = (y%10)+48;
                y = y - (y%10);
                y/=10;
            }
            *(position+10) = ';';
            position+=11;
            x = graph[i].matchingPartner->x;
            for(int j = 9; j > -1; j--){
                *(position+j) = (x%10)+48;
                x = x - (x%10);
                x/=10;
            }
            *(position+10) = ' ';
            position+=11;
            y = graph[i].matchingPartner->y;
            for(int j = 9; j > -1; j--){
                *(position+j) = (y%10)+48;
                y = y - (y%10);
                y/=10;
            }
            position+=10;
            *position = '\n';
            position++;

        }
    }
    *position = '\0';

    printf("%s",output);
    free(output);

     }


/**inverts the augmenting path starting at @param rootNodeReverseDFS, marks used nodes*/
void invertPath(node_t *rootNodeReverseDFS){
    node_t *curr = rootNodeReverseDFS;
    node_t *before,*tmp;

    do{
        curr->matchingPartner = curr->prev;
        before = curr;
        curr = curr->prev;

        before->prev = (node_t*)0x01;
        curr->prev = (node_t*)0x01;

        tmp = curr->matchingPartner;
        curr->matchingPartner = before;
        curr = tmp;


    }while(curr != NULL);
}

/**checks if any nodes are already used in other paths, returns 1 if not*/
int pathValid(node_t *rootNodeReverseDFS){
    node_t *curr = rootNodeReverseDFS;
    do{
        if(curr->prev == (node_t*)0x01){return 0;}
        curr = curr->prev;
        if(curr->prev == (node_t*)0x01){return 0;}
        curr = curr->matchingPartner;

    }while(curr != NULL);
    return 1;
}

int checkDone(){
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner==NULL) {
            return 0;
        }
    }
    return 1;
}


void reset(){
    for(int i = 0; i < lineNumber; i++){
        graph[i].prev = NULL;
    }
}


static inline unsigned short rootSet(node_t *node){
    return ((node->x%2)+(node->y%2))%2;
}


/**
 * finds and inverts augmenting paths,
 * @return 0 when no path can be found since there are no free nodes where the path could end
 * @return 1 when at least one path was found and inverted
 * @return -1 when no path can be found but there are free nodes, meaning the problem is not solvable
 * */
int findLongerAugmentations(node_t **queue){
#ifdef DEBUG
    printf("enter\n");
#endif
    /*BFS starting w/ free nodes that are also part of the rootSet*/
    size_t queuesize = 0;
    short done = 1; //boolean, if we added any new nodes (if not, no more augmenting paths can be found)
    /*initializing the queue w/free nodes */
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL && !rootSet(graph+i)){
            done = 0;
            if(graph[i].left != NULL) {
                queue[queuesize++] = graph[i].left;
                graph[i].left->prev = &graph[i];
            }
            if(graph[i].right != NULL) {
                queue[queuesize++] = graph[i].right;
                graph[i].right->prev = &graph[i];
            }
            if(graph[i].up != NULL) {
                queue[queuesize++] = graph[i].up;
                graph[i].up->prev = &graph[i];
            }
            if(graph[i].down != NULL) {
                queue[queuesize++] = graph[i].down;
                graph[i].down->prev = &graph[i];
            }
        }
    }
    if(done){
        //free(queue);
        return 0;
    }
    /*now alternately replacing matching partners to queue resp. adding the neighbours to the queue whilst removing the resp. node*/
    node_t *curr;
    size_t queuestart = 0;
    size_t oldsize = 0;
    size_t oldstart = 0;
    short added,path_done;
    /*debug variable*/
    unsigned long path_len = 3;
    do {
            path_done = 0;
            for(size_t i = queuestart; i < queuestart+queuesize;i++) { //replacing matching partners
                curr = queue[i];
                if (curr->matchingPartner == NULL){
                    /*
                    #ifdef DEBUG
                    printf("we did it: found free node %ld  %ld, ending BFS\n",curr->x,curr->y);
                    printf("length = %ld ",path_len);
                    #endif
                     */
                    if(pathValid(curr)) {
                        invertPath(curr);
                    }
                    else {
                        #ifdef DEBUG
                        printf("not valid\n");
                        #endif
                        continue;
                    }
                    path_done = 1;
                }//we can end after this iteration since we found a free node
                else {
                    queue[i] = curr->matchingPartner;
                }
            }
        if(path_done)return 1;


        oldsize = queuesize;
        oldstart = queuestart;
        added = 0;
            for(size_t i = queuestart; i < oldstart+oldsize; i++) {//adding neighbours
                curr = queue[i];
                queuesize--;
                queuestart++;
                if (curr->left != NULL) {
                    if(rootSet(curr->left) && curr->left->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->left;
                        curr->left->prev = curr;
                        added = 1;
                    }
                }
                if (curr->right != NULL) {
                    if(rootSet(curr->right) && curr->right->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->right;
                        curr->right->prev = curr;
                        added = 1;
                    }
                }
                if (curr->up != NULL) {
                    if(rootSet(curr->up) && curr->up->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->up;
                        curr->up->prev = curr;
                        added = 1;
                    }
                }
                if (curr->down != NULL) {
                    if(rootSet(curr->down) && curr->down->prev == NULL) {
                        queue[queuestart+queuesize++] = curr->down;
                        curr->down->prev = curr;
                        added = 1;
                    }
                }
            }
        if(!added){
            //free(queue);
            return -1;
        }
        path_len+=2;
    }
    while(1);

}


void findShortestAugmentations(){
    /*
    int changed = 1;
    int neighbor_count = 0;
    enum direction  {left,right,up,down,none};
    int dir = none;
    while(changed){
        changed = 0;
        for(int i = 0; i < lineNumber; i++){
            if(graph[i].matchingPartner == NULL){
                if(graph[i].down != NULL){
                    if(graph[i].down->matchingPartner == NULL) {
                        neighbor_count++;
                        dir = down;
                    }
                }
                if(graph[i].right!=NULL){
                    if(graph[i].right->matchingPartner == NULL) {
                        neighbor_count++;
                        dir = right;
                    }
                }

                if(graph[i].up!=NULL){
                    if(graph[i].up->matchingPartner == NULL) {
                        neighbor_count++;
                        dir = up;
                    }
                }
                if(graph[i].left!=NULL){
                    if(graph[i].left->matchingPartner == NULL) {
                        neighbor_count++;
                        dir = left;
                    }
                }

                if(neighbor_count<1){
                    printf("None\n");
                    free(graph);
                    exit(SUCCESS_CODE);
                }
                if(neighbor_count<2){
                    changed = 1;
                    if(dir ==left){
                        graph[i].matchingPartner = graph[i].left;
                        graph[i].left->matchingPartner=&graph[i];
                    }
                    if(dir ==right){
                        graph[i].matchingPartner = graph[i].right;
                        graph[i].right->matchingPartner=&graph[i];
                    }
                    if(dir ==up){
                        graph[i].matchingPartner = graph[i].up;
                        graph[i].up->matchingPartner=&graph[i];
                    }
                    if(dir ==down){
                        graph[i].matchingPartner = graph[i].down;
                        graph[i].down->matchingPartner=&graph[i];
                    }
                neighbor_count = 0;
                dir = none;
                }
            }
        }
    }
     */

    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL){
            if(graph[i].left != NULL){
                if(graph[i].left->matchingPartner == NULL){
                    graph[i].left->matchingPartner = graph[i].left->right;
                    graph[i].matchingPartner = graph[i].left;
                    continue;
                }
            }
            if(graph[i].down != NULL){
                if(graph[i].down->matchingPartner == NULL){
                    graph[i].down->matchingPartner = graph[i].down->up;
                    graph[i].matchingPartner = graph[i].down;
                    continue;
                }
            }
        }
    }
}


void findNeighbors(){

    for(int i = 0; i < lineNumber; i++){
        if(graph[i].right == NULL && i != lineNumber-1){
            if(graph[i+1].y == graph[i].y && graph[i+1].x == graph[i].x+1){
                graph[i].right = (node_t *)0x01; //marker, that there is a neighbor (cannot take absolute pointer value, since we will resort)
            }
        }
    }
    qsort(graph,lineNumber, sizeof(node_t), &compareFunctionX);
    for(int i = 0; i < lineNumber; i++){
        unsigned long x = graph[i].x;

        if(graph[i].up == NULL && i != lineNumber-1){
            if(graph[i+1].x == graph[i].x && graph[i+1].y == graph[i].y+1){
                graph[i+1].down = &graph[i];
                graph[i].up = &graph[i+1];
            }
        }

        //Fixme: this is inefficient AND out of bound checking is not(?) correct

        if(graph[i].right == (node_t*)0x01){
            node_t *xmore = &graph[i];
            while(xmore->x == x && (xmore-graph)<=lineNumber)xmore++;
            while(xmore->x == x+1 && (xmore-graph)<=lineNumber){
                if(xmore->y == graph[i].y){
                    xmore->left = &graph[i];
                    graph[i].right = xmore;
                    break;
                }
                xmore++;
            }
        }
    }
}


int compareFunctionX(const void *a, const void *b){
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
            printf("ax = %ld, bx = %ld, ay = %ld, by = %ld\n",*(unsigned long *)a, *(unsigned long *)b,*ay,*by);
            dup++;
            return 0;
        }
    }
}


int compareFunctionY(const void *a, const void *b){
    const unsigned long *ay = (const unsigned long *)a + 1;
    const unsigned long *by = (const unsigned long *)b + 1;
    if(*ay>*by)return 1;
    if(*ay<*by)return -1;
    else{
        if(*(const unsigned long *)a > *(const unsigned long *)b){
            return 1;
        }
        if(*(const unsigned long *)a < *(const unsigned long *)b)return -1;
        else {
            printf("ax = %ld, bx = %ld, ay = %ld, by = %ld\n",*(unsigned long *)a, *(unsigned long *)b,*ay,*by);
            dup++;
            return 0;
        }
    }
}


void parseInput(){
    lineNumber = 0;
    while(getNextLine()!=ERROR_CODE){
        checkLineSanity();
        parseLine(lineNumber++);
    }
}

/**
 * reads the next line (until '\ n') as ASCII-encoded byte-stream and handles errors
 * @returns 0 on success, 1 when reaching EOF, defined error codes when encountering errors
 * */
int getNextLine(){
    buf[BUF_LEN-1] = 'x';
    if(fgets(buf,sizeof(buf),stdin) == NULL)return ERROR_CODE;
    if(buf[BUF_LEN-1] != 'x'){
        //our buffer doesn't contain the entire line
        char *tmp = malloc(BUF_LEN*sizeof(char));
        memset(tmp,0,BUF_LEN*sizeof(char));
        int state = 0,i = 1,index_tmp = 0;
        char c = buf[0];
        while(c != '\n'){
            if(state == 0){
                if(c != '0'){
                    tmp[0] = '0';
                    tmp[1] = c;
                    index_tmp = 2;
                    state = 1;
                }
            }
            else if(state == 1){
                if(c != ' '){
                    tmp[index_tmp++] = c;
                }
                else state = 2;
            }
            else if(state == 2){
                if(c != ' '){
                    tmp[index_tmp++] = ' ';
                    tmp[index_tmp++] = c;
                    state = 3;
                }
            }
            else if(state == 3){
                if(c != '0' && c!='\0'){
                    tmp[index_tmp++] = c;
                    state = 4;
                }
            }
            else if(state > 3 && state < 20){
                tmp[index_tmp++] = c;
            }
            else break;

            if(i >= BUF_LEN)c = fgetc(stdin);
            else{
                c = buf[i++];
            }
        }
        tmp[index_tmp] = '\n';
        memcpy(buf,tmp,BUF_LEN*sizeof(char));
        free(tmp);
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
        if(graph == NULL){
            fprintf(stderr,"malloc() failed\n");
            die(ERROR_CODE);
        }
        graphSize = INIT_SIZE*sizeof(node_t);
    }
    if(graphSize/sizeof(node_t)<=lineNumber){
        graphSize*=2;
        graph = realloc(graph,graphSize);
        if(graph == NULL){
            fprintf(stderr,"realloc() failed\n");
            die(ERROR_CODE);
        }
    }
    char *endptr = NULL;
    errno = 0;
    graph[lineNumber].x = strtoul(buf, &endptr, 10);

    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);

    graph[lineNumber].y = strtoul(endptr,&endptr,10);

    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);
    /*set neighbor pointers to NULL*/
    memset((unsigned long*)&graph[lineNumber]+2,0,sizeof(node_t *)*6);
}


void die(int error_num){
    free(graph);
    switch(error_num){
        case UNKNOWN_CHAR:fprintf(stderr, "Unknown character, exiting...\n");break;
        case ULONG_OVERFLOW:fprintf(stderr,"Number too big!\n");break;
        case WRONG_NUMBER_COUNT:fprintf(stderr,"Too many or not enough Numbers in one Line, exiting...\n");break;
        case DOUBLE_VALUE:fprintf(stderr,"Double value, exiting\n");break;
        case ERROR_CODE:exit(ERROR_CODE);
        default:fprintf(stderr, "An unknown error occured, exiting...\n");break;
    }
    exit(ERROR_CODE);
}

