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
/* function declarations */
void die(int error_num);
int getNextLine();
void checkLineSanity();
void parseLine(unsigned long lineNumber);
void parseInput();
int compareFunction(const void *a, const void *b);


int main(){

	parseInput();
    qsort(provisionalDataStruct,lineNumber, sizeof(unsigned long)*2,&compareFunction);
    for(unsigned long i = 0; i < 2*lineNumber; i+=2){
        printf("%ld    %ld\n",provisionalDataStruct[i],provisionalDataStruct[i+1]);
    }
	free(provisionalDataStruct);
	return 0;
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
 * @param 2 Arrays of length 11 to save the parsed numbers in. Any other length results in undefined behavior!
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
    printf("%s -->   ",buf);
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
