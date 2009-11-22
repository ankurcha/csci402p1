/*
 *  string.c
 *  
 *
 *  Created by Ankur Chauhan on 11/21/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

#include "string.h"

int strlen(char *str){
    const char *s;
    for (s = str; *s; ++s);
    return(s - str);
}

char* strcat(char *dst, char *src){
    while(*dst++);
    while(*dst++ = *src++);
    return *dst;
}

int strcmp(const char *s1, const char *s2){
    while (*s1 == *s2++)
        if (*s1++ == 0)
            return 0;
    return (*(const char *)s1 - *(const char *)(s2 - 1));
}

char* strcpy(char *s1, const char *s2)
{
    char *dst = s1;
    const char *src = s2;
    /* Do the copying in a loop.  */
    while ((*dst++ = *src++) != '\0');
    
    /* Return the destination string.  */
    return s1;
}