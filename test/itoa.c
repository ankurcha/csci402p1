/*
 *  itoa.cc
 *  
 *
 *  Created by Ankur Chauhan on 10/10/09.
 *
 */

char* itoa(int a, char *str){
    int i = 0;
    int j = 0;
    char rts[50];

    // print the string in reverse
    do{
        rts[i] = '0'+ a%10;
        i++;
    }while((a=a/10) && i < 49);
    i--;

    // reverse it into the output, now it is forward
    while(i >= 0 && j < 32) {
        str[j] = rts[i];
        i--;
        j++;
    }
    str[j] = 0;

    return str;
}
