#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

char lineBuffer[200];
char *lineBufferPosition1 = NULL;
char *lineBufferPosition2 = NULL;
char c;
int main() {
	/*
	 * sollte Zeichenweise eingelesen werden, da man dann sofort auf syntax (leerzeichen etc. checken kann!)
	 * */
    int pos = 0;
	while((c = fgetc(stdin))!=EOF && pos < sizeof(lineBuffer)-1){ //null terminator has to fit
		if(lineBufferPosition1 == NULL && c != ' '){
            lineBufferPosition1 = &lineBuffer[pos];
        }
        else if(lineBufferPosition2 == NULL && c != ' '){
            lineBufferPosition2 = &lineBuffer[pos];
        }
        lineBuffer[pos++] = c;
	}
    lineBuffer[pos] = '\0';
    printf("%s",lineBuffer);
    return 0;
}
