/*
 *  itoa.c
 *  
 *  Provide conversions between strings and ints
 *
 * USC CS 402 Fall 2009
 *  Created by Ankur Chauhan on 10/10/09.
 *  Extended by Max Pflueger
 *
 */

#include "itoa.h"

char* itoa(unsigned int a, char *str) {
    int i = 0;
    int j = 0;
    char rts[50];

    /* print the string in reverse */
    do {
        rts[i] = '0' + a % 10;
        i++;
    } while ((a = a / 10) && i < 49);
    i--;

    /* reverse it into the output, now it is forward */
    while (i >= 0 && j < 32) {
        str[j] = rts[i];
        i--;
        j++;
    }
    str[j] = 0;

    return str;
}

int atoi(char* str) {
    int sum = 0;
    char neg = 0;
    int i = 0;

    /* clear out the non-int stuff */
    while (str[i] != 0) {
        if (str[i] == '-' || (str[i] >= '0' && str[i] <= '9')) {
            break;
        }
        i++;
    }

    /* read the int */
    if (str[i] == '-') {
        neg = 1;
        i++;
    }
    while (str[i] >= '0' && str[i] <= '9') {
        sum *= 10;
        sum += str[i] - '0';
        i++;
    }

    if (neg) {
        sum *= -1;
    }

    return sum;
}

