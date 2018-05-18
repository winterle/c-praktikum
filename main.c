#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>

#define ERROR_CODE (-1)
#define UNKNOWN_CHAR 2
#define UINT_OVERFLOW 3
#define TOO_MANY_NUMBERS 4
#define NOT_ENOUGH_NUMBERS 5
/* global variables and structs */

/* function declarations */
void die(unsigned short error_num);
unsigned short getNextLine(char *num0,char *num1);
void parseInput();


int main() {
    parseInput();
    return 0;
}

void parseInput(){
    unsigned short rc;
    char num0[11];
    char num1[11];
    while((rc = getNextLine(num0,num1))!=1) {
        if (rc != 0) {
            die(rc);
        };
        printf("num0 = %s , num1 = %s\n",num0,num1);
    }
}

/**
 * reads the next line (until '\ n') as ASCII-encoded byte-stream and handles errors
 * @param 2 Arrays of length 11 to save the parsed numbers in. Any other length results in undefined behavior!
 * @returns 0 on success, 1 when reaching EOF, defined error codes when encountering errors
 * */
unsigned short getNextLine(char *num0,char *num1){
    char c;
    num0[10] = num1[10] = '\0';
    num0[0] = num1[0] = '\0'; //to check if the second number was actually read
    short numbersRead = 0;
    short lastRead = 0; //boolean: lastRead char was /n or blank: 0, else 1
    char *pos = num0;
    while((c=fgetc(stdin))!='\n' && c != EOF){
        if(c == ' ' && lastRead==1){
            lastRead=0;
            pos = num1;
            numbersRead++;
        }
        if(c == ' '){
            lastRead=0;
            continue;
        }
        if(c < 48 || c > 57){
            return UNKNOWN_CHAR;
        }
        if(!numbersRead){
            lastRead = 1;
            if(pos == &num0[10])//read number is too long
                return UINT_OVERFLOW;
            *pos = c;
            pos++;
            *pos = '\0';
        }
        else if(numbersRead == 1){
            lastRead = 1;
            if(pos == &num0[10])//read number is too long
                return UINT_OVERFLOW;
            *pos = c;
            pos++;
            *pos = '\0';
        }
        else if(numbersRead > 1)
            return TOO_MANY_NUMBERS;
    }
    if(c == EOF){
        return 1;
    }
    if(num1[0] == '\0' || num0[0] == '\0'){
        return NOT_ENOUGH_NUMBERS;
    }
    return 0;
}

void die(unsigned short error_num){
    if(error_num==UNKNOWN_CHAR) {
        fprintf(stderr, "Unknown character, exiting...\n");
    }
    else if(error_num==UINT_OVERFLOW){
        fprintf(stderr,"Number too big!\n");
    }
    else if(error_num==TOO_MANY_NUMBERS){
        fprintf(stderr,"Too many Numbers in one Line, exiting...\n");
    }
    else if(error_num==NOT_ENOUGH_NUMBERS){
        fprintf(stderr,"Not enough Numbers in one Line, exiting...\n");
    }
    else{
        fprintf(stderr, "An unknown error occured, exiting...\n");
    }
    exit(ERROR_CODE);
}