#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#define ERROR_CODE (-1)
#define UNKNOWN_CHAR 2
#define UINT_OVERFLOW 3
#define TOO_MANY_NUMBERS 4
/* global variables and structs */

/* function declarations */
void die(unsigned short error_num);
unsigned short getNextLine();
void parseInput();
int main() {
    parseInput();
    return 0;
}

void parseInput(){
    unsigned short rc;
    while((rc = getNextLine())!=1) {
        if (rc != 0) {
            die(rc);
        };
    }
}

/**
 * reads the next line (until '\ n') as ASCII-encoded byte-stream and handles errors
 * @returns 0 on success, 1 when reaching EOF, defined error codes when encountering errors
 * */
unsigned short getNextLine(){
    char c;
    char num0[11];
    char num1[11];
    num0[10] = num1[10] = '\0';
    short numbersRead = 0;
    short lastRead = 0; //boolean: lastRead char was /n or blank: 0, else 1
    char *pos = num0;
    while((c=fgetc(stdin))!='\n' && c != EOF){
        if(c == ' ' && lastRead==1){
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
    printf("%s\n",num0);
    printf("%s\n",num1);
    if(c == EOF){
        return 1;
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
    else{
        fprintf(stderr, "An unknown error occured, exiting...\n");
    }
    exit(ERROR_CODE);
}