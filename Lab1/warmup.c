#include <stdio.h>
#include <stdlib.h>

/* Maxwell Daum Assignment 0 Comp530 F16
Thanks to James for his helpful test case addressing null characters on Piazza.
This assignment was submitted late due to extension granted by Professor Porter.
Extension was to Tuesday 9/6
I have complied with all the rules of the honor code in completing this assignment.
*/
int main(){
    int counter=0;
    char curr; //current character I am processing
    char line[81];//contains current line
    int  lastWasPercent=0;//indicator if the last viewed char was percent
    curr='t';//temp
    while(curr!=EOF){
        if(counter==80){ //here will need to peek if recently processed was %
            line[80]='\n';
            if(lastWasPercent){
                curr=getchar();//peeking for %
                if(curr=='%'){  //replace, print the line, reset vars
                    line[79]='*';
                    printLine(line);
                    lastWasPercent=0;
                    counter=0;
                }
                else{ //then I have processed first char of next line,print before overwriting.
                    printLine(line);
                    line[0]=curr;
                    counter=1;
                    lastWasPercent=0;
                }
            }
            else{   //simply print, set counter back to 0
                printLine(line);
                counter=0;
            }
        }
        curr=getchar(); //read from stdin
        if(curr=='\n')curr=' ';
        if(curr=='%'){
            if(lastWasPercent){ //we replace previous % with *, do not increment counter, essentially skipping the second % we just saw
                *(line+counter-1)='*';
                lastWasPercent=0;
                continue;
            }
            else lastWasPercent=1; //we just saw one...setting flag
        }
        else lastWasPercent=0; //last viewed no longer %
        line[counter]=curr;
        counter++;
    }
    exit(0);

}

void printLine(char *c){ //pass in pointer to start of line
    int i;
    for(i=0;i<81;i++){  //print each char
        putchar(*(c+i));
    }
}
