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
size_t provisionalDataStructSize;
unsigned long *provisionalDataStruct;
unsigned long reallocCounter;
unsigned long lineNumber;

unsigned long * space;
unsigned long * sameSpaces;
/* function declarations */
void die(int error_num);
int getNextLine();
void checkLineSanity();
void parseLine(unsigned long lineNumber);
void parseInput();
int compareFunction(const void *a, const void *b);
void partition();
void partitionNew();
void getSameSpace(unsigned long ax, unsigned long ay, unsigned long bx, unsigned long by);

int main(){

	parseInput();

    qsort(provisionalDataStruct,lineNumber, sizeof(unsigned long)*2,&compareFunction);

    partitionNew();
    for(unsigned long i = 0; i < 2*lineNumber; i+=2){
        printf("%ld    %ld , space = %ld \n",provisionalDataStruct[i],provisionalDataStruct[i+1],space[i/2]);
    }

    printf("%ld * 2  = %ld numbers parsed and sorted\n",lineNumber,lineNumber*2);


	free(provisionalDataStruct);
	return 0;
}

void partitionNew(){
    space = malloc(lineNumber*sizeof(unsigned long));
    sameSpaces = malloc(lineNumber*sizeof(unsigned long));
    unsigned long spaceCtr = 0;
    short firstXElement;
    unsigned long xCounter = 0;
    for(unsigned long i = 0; i < lineNumber*2;){
        xCounter++;
        unsigned long x = provisionalDataStruct[i];
        firstXElement = 1;
        while(provisionalDataStruct[i] == x){
            //printf("i = %ld,x = %ld\n",i,x);
            if(firstXElement){
                //printf("first element: new space = %ld\n",spaceCtr);
                space[i/2] = spaceCtr;
                spaceCtr++;
            }
            else{
                if(provisionalDataStruct[i+1] - provisionalDataStruct[i-1] == 1){
                    //printf("sameSpace found: %ld(%ld) - %ld = %ld \n",provisionalDataStruct[i+1],i+1,provisionalDataStruct[i-1],(provisionalDataStruct[i+1]-provisionalDataStruct[i-1]));
                    space[i/2] = space[(i-2)/2];
                }
                else{
                    //printf("not in the same space new space = %ld\n",spaceCtr);
                    space[i/2] = spaceCtr;
                    spaceCtr++;
                }
            }
            firstXElement = 0;
            i+=2;
        }
    }
    //TODO find overlapping spaces w/different x
    /*for(int i = 0; i < xCounter; i++){

    }
    */

}

void partition(){//TODO space array is space-inefficient
    space = malloc(lineNumber*sizeof(unsigned long));
    sameSpaces = malloc(lineNumber*sizeof(unsigned long));
    unsigned long sameSpacesCtr = 0;
    unsigned long *lastXStart = NULL;
    unsigned long x;
    unsigned long spaceCtr = 0;
    space[0] = spaceCtr++;
    short sameSpace = 0; //boolean, if kachel is in same space considering only y-values
    short sameSpaceBefore = 0; //boolean, if kachel is in same space considering also x-1 values
    for(unsigned long i = 0; i < lineNumber*2; i+=2){ //alle elemente einmal anschauen -> = O(n) bis jetzt
        x = provisionalDataStruct[i];
        printf("partition: for, x = %ld\n",x);
        unsigned long oldI = i;
        while(provisionalDataStruct[i] == x){
            //printf("partition: while with i = %ld\n",i);
            sameSpace = 0;
            sameSpaceBefore = 0;
            if(lastXStart != NULL){
                for(unsigned long j = 0; lastXStart[j+1]<= provisionalDataStruct[i+1];j+=2){
                    if(lastXStart[j+1] == provisionalDataStruct[i+1]) {
                        space[i/2] = space[(&provisionalDataStruct[i+1]-&lastXStart[j+1])/2];
                        sameSpaceBefore = 1;
                    }
                }
            }
            if(i!=oldI) {
                if (provisionalDataStruct[i + 1] - provisionalDataStruct[i - 1] == 1) {
                    sameSpace = 1;
                }
            }
            if(sameSpaceBefore && sameSpace){
                sameSpaces[sameSpacesCtr++] = space[i/2];
                sameSpaces[sameSpacesCtr++] = space[(i-2)/2];
            }
            else if(sameSpace){
                space[i/2] = space[(i-2)/2];
            }
            else if(sameSpaceBefore){
                i+=2;
                continue;
            }
            else{
                space[i/2] = spaceCtr++;
            }

            i+=2;
        }
        lastXStart = provisionalDataStruct+=oldI;
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
        else {printf("double value detected!\n");
        return 0;}
    }
}


void parseInput(){
    lineNumber = 0;
    while(getNextLine()!=ERROR_CODE){
        checkLineSanity();
        parseLine(lineNumber++);
    }
    printf("%ld reallocs executed\n",reallocCounter);
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
        provisionalDataStruct = malloc(INIT_SIZE*sizeof(unsigned long));
        provisionalDataStructSize = INIT_SIZE*sizeof(unsigned long);
        reallocCounter = 0;
    }
    if(provisionalDataStructSize/sizeof(unsigned long)<=lineNumber*2){
        reallocCounter++;
        provisionalDataStructSize*=2;
        provisionalDataStruct = realloc(provisionalDataStruct,provisionalDataStructSize);
        if(provisionalDataStruct == NULL){printf("realloc failed\n");die(-1);}
    }
    char *endptr = NULL;
    errno = 0;
    provisionalDataStruct[lineNumber * 2] = strtoul(buf, &endptr, 10);

    printf("%ld   ", provisionalDataStruct[lineNumber*2]);

    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);
    provisionalDataStruct[lineNumber * 2+1] = strtoul(endptr,&endptr,10);

    printf("%ld   \n", provisionalDataStruct[lineNumber*2+1]);

    if (errno == ERANGE) {
        die(ULONG_OVERFLOW);
    }
    if (errno != 0 || endptr==NULL)die(ERROR_CODE);
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
