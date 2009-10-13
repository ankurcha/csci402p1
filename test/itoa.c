/*
 *  itoa.cc
 *  
 *
 *  Created by Ankur Chauhan on 10/10/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

char* itoa(int a){
    static char str[50];
    int i = 49;
    do{
        str[i] = '0'+ a%10;
    }while((a=a/10) && i>=0);
    return str;
}
