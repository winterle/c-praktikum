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
/* global variables and structs */
char buf[BUF_LEN]; //assuming one line never exceeds 1024 characters

/*todo list
 * add buffer overflow handler
 * fix reallocing too much
 * space partitioning (needed?)
 * */


typedef struct node_s{
    unsigned long x;
    unsigned long y;
    struct node_s *up,*down,*left,*right,*matchingPartner;
}node_t;

size_t graphSize;
node_t *graph;
unsigned long reallocCounter;
unsigned long lineNumber;


/* function declarations */
void die(int error_num);
int getNextLine();
void checkLineSanity();
void parseLine(unsigned long lineNumber);
void parseInput();
int compareFunction(const void *a, const void *b);
void partitionNew();
void findShortestAugmentations();
int checkDone();

int main(){

	parseInput();

    qsort(graph,lineNumber, sizeof(node_t),&compareFunction);
    partitionNew();
    findShortestAugmentations();
    /*
    for(int i = 0; i < lineNumber; i++){
        printf("%ld    %ld",graph[i].x,graph[i].y);
        //if(graph[i].match)printf("  match =  %ld  %ld",((node_t *)(graph+i+2*sizeof(unsigned long)+(graph[i].match -1)))->x,((node_t *)(graph+i+2*sizeof(unsigned long)+(graph[i].match -1)))->y);
        printf("\n");
    }
     */
    if(checkDone())printf("found perfect matchings only using depth 1\n");
    else printf("not done, need to check for deeper augmentations\n");
    printf("Size in element count of provDataStruct = %ld\n", graphSize/sizeof(node_t));
    printf("total parsed lines = %ld\n",lineNumber);

	free(graph);
	return 0;
}

int checkDone(){
    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner==NULL) {
            printf("%ld   %ld is not matched!\n",graph[i].x,graph[i].y);
            continue;
        }
    }
    return 1;
}



void findShortestAugmentations(){

    for(int i = 0; i < lineNumber; i++){
        if(graph[i].matchingPartner == NULL){
            if(graph[i].left != NULL){
                if(graph[i].left->matchingPartner == NULL){
                    printf("%ld %ld;%ld %ld\n",graph[i].left->x, graph[i].left->y,graph[i].x, graph[i].y);
                    graph[i].left->matchingPartner = graph[i].left->right;
                    graph[i].matchingPartner = graph[i].left;
                    continue;
                }
            }
            if(graph[i].down != NULL){
                if(graph[i].down->matchingPartner == NULL){
                    printf("%ld %ld;%ld %ld\n",graph[i].down->x, graph[i].down->y,graph[i].x, graph[i].y);
                    graph[i].down->matchingPartner = graph[i].down->up;
                    graph[i].matchingPartner = graph[i].down;
                    continue;
                }
            }
        }
    }
}



void partitionNew(){
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
    //printf("%ld reallocs executed\n",reallocCounter);
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
    memset((unsigned long*)&graph[lineNumber]+2,0,sizeof(node_t *)*5);
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

