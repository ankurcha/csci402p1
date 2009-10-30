/*
 *  messageConstructor
 *  
 *
 *  Created by Ankur Chauhan on 10/26/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

#include "messageConstructor.h"
#include "post.h"

#include MaxDataSize (MaxMailSize - (2*sizeof(unsigned)))

MessageConstructor::MessageConstructor(){
    sequenceNumber = 0;
    flags = 0x0;
}

MessageConstructor::MessageConstructor(char *d, int len){
    if (len < MaxDataSize) {
        // Data size is possible
        data = d;
    }else {
        data = null;
    }
}

void MessageConstructor::resetFlags(){
    this->flags = 0x00;
}

void MessageConstructor::resetSequenceCount(int val){
    sequenceNumber = val;
}

char* MessageConstructor::generateMessage(){
    
}