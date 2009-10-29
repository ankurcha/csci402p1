/*
 *  messageConstructor.h
 *  
 *
 *  Created by Ankur Chauhan on 10/26/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

class MessageConstructor{
private:
    char *message;
    unsigned int sequenceNumber;
    unsigned int flags;
    char *data;
public:
    MessageConstructor();
    MessageConstructor(char *d, int len);
    
    void resetFlags();  // Resets all the flags to 0
    void resetSequenceCount(int val); // Reset the seguence counter to val
    char* generateMessage(); // generated the final message with the header/data
};