/*
 *  init.h
 *  
 *
 *  Created by Ankur Chauhan on 10/11/09.
 *  Copyright 2009 University of Southern California. All rights reserved.
 *
 */

#include "syscall.h"
#include "queue.h"
#include "queue.c"
#include "itoa.c"
#include "print.h"

#define BUSY 0
#define FREE 1
#define SLEEPING 2

#define MAX_DOCTORS 10
#define MIN_DOCTORS 4

#define MAX_DOORB 10
#define MIN_DOORB 4

#define MAX_PATIENTS 100
#define MIN_PATIENTS 20

#define RECP_MAX 5
#define RECP_MIN 3

#define MAX_CLERKS 5
#define MIN_CLERKS 3

#define MAX_CASHIER 5
#define MIN_CASHIER 3

#define totalHospMan 1

int numDoctors = 0;
int numCashiers = 0;
int numClerks = 0;
int numDoorboys = 0;
int numRecp = 0;
int numPatients = 0;
char test1active = 0;
char test2active = 0;
char test4active = 0;
char test7active = 0;
char test5active = 0;
int feesPaid = 0;

struct linkedlist_element { 
    /*Used for storing the <token,fees> pairs */
    int key, value;
};


struct list {
    struct linkedlist_element listArray[MAX_PATIENTS];
    int head;
};
typedef struct list List;

int List_Append(List* l, int key, int val){
    if (l == 0) {
        return -1;
        
    }
    if(l->head == MAX_PATIENTS + 1){
        print("List is full\n");
        return 0;
    }else {
        l->listArray[l->head].key = key;
        l->listArray[l->head].value = val;
        l->head++;
        return 1;
    }
    
}


int List_getValue(List *l,int key){
    int temp = 0;
    if(l != 0 || l->head == 0){
        print("Empty or invalid list\n");
        return 0;
    } 
    while (temp <= l->head) {
        if (l->listArray[temp].key == key) {
            return l->listArray[temp].value;
        }
        temp++;
    }
    print("Key not found in list");
    return -1;
}

int List_IsEmpty(List *l){
    if (l->head == 0) {
        return 1;
    }else {
        return 0;
    }
    
}
/*queue elements for the waking doctors list*/
queue_element wakingdoctor_element[MAX_PATIENTS];

LockId testlock;
/* tokenCounter for assigning tokens to patients */
LockId TokenCounterLock;
int TokenCounter = 0;
/* global for all receptionists */
LockId recpLineLock;
/*shared data struct related to a Receptionist */
struct Receptionists_ {
    /* receptionist line CV */
    CVId receptionCV;
    int peopleInLine;
    
    /* receptionist transactional lock and CV and protected variables */
    LockId transLock;
    CVId receptionistWaitCV;
    int currentToken;
    
    /* receptionist break CV */
    CVId ReceptionistBreakCV;
    
};
typedef struct Receptionists_ Receptionists;
void __Receptionists(Receptionists *recep ){
    recep->peopleInLine = 0;
    recep->receptionCV = CreateCondition("receptionCV");
    recep->transLock = CreateLock("Receptionists.transLock");
    recep->receptionistWaitCV = CreateCondition("receptionistWaitCV");
    recep->ReceptionistBreakCV = CreateCondition("ReceptionistBreakCV");
    recep->currentToken = 0;
}
/* list mapping patient tokens to consultFees */
LockId feeListLock;
List feeList;
/* global for all cashiers */
LockId cashierLineLock ;
LockId feesPaidLock;
/* shared data struct related to a Cashier */
struct Cashier_ {
    /* line CV and length */
    int lineLength;
    CVId lineCV;
    
    /* transaction lock, CV, and variables protected by the former */
    LockId transLock;
    CVId transCV;
    int patToken;
    int fee;
    int payment;
    
    /*protected by feesPaidLock, but only modified by one thread */
    int sales;
    
    /* cashier's CV for going on break */
    CVId breakCV;
};
typedef struct Cashier_ Cashier;
void __Cashier(Cashier *cash) {
    cash->lineLength = 0;
    cash->patToken = 0;
    cash->fee = 0;
    cash->payment = 0;
    
    cash->lineCV = CreateCondition("Cashier.lineCV");
    cash->transLock = CreateLock("Cashier.transLock");
    cash->transCV = CreateCondition("Cashier.transCV");
    cash->breakCV = CreateCondition("Cashier.breakCV");
}
void _Cashier(Cashier *cash) {
    DestroyCondition( cash->lineCV );
    DestroyLock( cash->transLock );
    DestroyCondition( cash->transCV );
    DestroyCondition( cash->breakCV );
}
LockId ClerkLinesLock;
LockId PaymentLock;
int totalsales=0;
/* hospitalLock protects the count of patients remaining in the hospital */
LockId hospitalLock;
int peopleInHospital = 1;
struct PharmacyClerks_ {
    int patientsInLine;
    int state;
    int payment;
    int fee;
    int patPrescription;
    CVId ClerkCV;
    
    CVId ClerkBreakCV;
    LockId ClerkTransLock;
    CVId ClerkTransCV;
    
    /*protected by PaymentLock */
    int sales;
};
typedef struct PharmacyClerks_ PharmacyClerks;
void __PharmacyClerks(PharmacyClerks *pcl){
    pcl-> patientsInLine= 0;
    pcl->state=FREE;
    pcl->payment=0;
    pcl->fee=(int)(1267)%100;
    pcl->patPrescription=0;
    
    pcl->ClerkCV = CreateCondition("ClerkCV");
    pcl->ClerkBreakCV = CreateCondition("ClerkBreakCV");
    pcl->ClerkTransLock = CreateLock("ClerkTransLock");
    pcl->ClerkTransCV = CreateCondition("ClerkTransCV");
}  
struct Doctor_ {
    /* line lock and CV and protected variables */
    LockId LineLock;
    CVId LineCV;
    int peopleInLine;
    /*CV for doorboys to sleep on */
    CVId doorboyBreakCV;
    
    
    /*transaction lock and CV and variables protected */
    LockId transLock;
    CVId transCV;
    int prescription;
    int patientToken;
} ;
typedef struct Doctor_ Doctor;
void __Doctor(Doctor *doc) {
    doc->prescription = -1;
    doc->patientToken = -1;
    
    doc->peopleInLine = 0;
    doc->LineLock = CreateLock("LineLock");
    doc->LineCV = CreateCondition("LineCV");
    doc->doorboyBreakCV = CreateCondition("Doctor.doorboyBreakCV");
    
    doc->transLock = CreateLock("Doctor.transLock");
    doc->transCV = CreateCondition("Doctor.transCV");
}
void _Doctor(Doctor *doc) {
    DestroyLock(doc->LineLock);
    DestroyCondition(doc->LineCV);
    DestroyCondition(doc->doorboyBreakCV);
    DestroyLock(doc->transLock);
    DestroyLock(doc->transCV);
}
/* globals to track the queue of doorboys waiting to service doctors */
LockId doorboyLineLock;
CVId doorboyLineCV;
int doorboyLineLength = 0;
/*int wakingDoctorID = 0; */
Queue wakingDoctorList;
struct Doorboy_ { 
    int doorboyid; 
};
typedef struct Doorboy_ DoorBoy;

Receptionists receptionists[RECP_MAX];
DoorBoy doorboys[MAX_DOCTORS];
Doctor doctors[MAX_DOCTORS];
Cashier cashiers[MAX_CASHIER];
PharmacyClerks clerks[MAX_CLERKS];

int test_state = 0;

LockId creationLock;
int patientCount = 0;
int recptionistCount = 0;
int doorboyCount = 0;
int doctorCount = 0;
int cashierCount = 0;
int pharmacyCount = 0;
int hospitalmanagerCount = 0;


