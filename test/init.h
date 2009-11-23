#ifndef INIT_H_
#define INIT_H_

#include "syscall.h"
#include "queue.h"
#include "itoa.h"
#include "print.h"
#include "string.h"
#include "p2pnetwork.h"

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
/*
 * LINKED LIST OF KEY VALUE PAIRS
 */

struct linkedlist_element {
    /*Used for storing the <token,fees> pairs */
    int key, value;
};

struct list {
    struct linkedlist_element listArray[MAX_PATIENTS];
    int head;
};
typedef struct list List;

/***************************************************
 * shared data struct related to a Receptionist ****
 * *************************************************/
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

struct PharmacyClerks_ {
    int patientsInLine;
    /* int state; */
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

struct Doctor_ {
    /* line lock and CV and protected variables */
    LockId LineLock;
    CVId LineCV;
    CVId doorboyBreakCV;
    LockId transLock;
    CVId transCV;

    int peopleInLine;
    int prescription;
    int patientToken;
};
typedef struct Doctor_ Doctor;

struct Doorboy_ {
    int doorboyid;
};
typedef struct Doorboy_ DoorBoy;

/**** GLOBAL VARIABLES *****/
int numDoctors = 0;
int numCashiers = 0;
int numClerks = 0;
int numDoorboys = 0;
int numRecp = 0;
int numPatients = 0;
int feesPaid = 0;
int test_state = 0;
int patientCount = 0;
int recptionistCount = 0;
int doorboyCount = 0;
int doctorCount = 0;
int cashierCount = 0;
int pharmacyCount = 0;
int hospitalmanagerCount = 0;
int TokenCounter = 0;
int totalsales = 0;
int peopleInHospital = 1;
int doorboyLineLength = 0;
queue_element wakingdoctor_element[MAX_PATIENTS];
Queue wakingDoctorList;
List feeList;

Receptionists receptionists[RECP_MAX];
DoorBoy doorboys[MAX_DOCTORS];
Doctor doctors[MAX_DOCTORS];
Cashier cashiers[MAX_CASHIER];
PharmacyClerks clerks[MAX_CLERKS];

/**** GLOBAL LOCKS ****/
LockId creationLock;
LockId testlock;
LockId TokenCounterLock;
LockId recpLineLock;
LockId feeListLock;
LockId cashierLineLock;
LockId feesPaidLock;
LockId ClerkLinesLock;
LockId PaymentLock;
LockId hospitalLock;
LockId doorboyLineLock;
/**** GLOBAL CVs ****/
CVId doorboyLineCV;

/**** FUNCTIONS ****/

int List_Append(List* l, int key, int val) {
    if (l == 0) {
        return -1;

    }
    if (l->head == MAX_PATIENTS + 1) {
        print("List is full\n");
        return 0;
    } else {
        l->listArray[l->head].key = key;
        l->listArray[l->head].value = val;
        l->head++;
        return 1;
    }

}

int List_getValue(List *l, int key) {
    int temp = 0;
    if (l != 0 || l->head == 0) {
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

int List_IsEmpty(List *l) {
    if (l->head == 0) {
        return 1;
    } else {
        return 0;
    }

}

int getNextRecpCV(int recepID){
    static int CVID = 0;
    return CVID + (20 * recepID);
}

int getNextRecpLock(int recepID){
    static int LockID = 0;
    return LockID + (20 * recepID + 10);
}

int getNextCashCV(int cashID){
    static int CVID = 0;
    return RECP_MAX + CVID + (20 * cashID);
}

int getNextCashLock(int cashID){
    static int LockID = 0;
    return RECP_MAX + LockID + (20 * cashID + 10);
}

int getNextClerkCV(int clerkID){
    static int CVID = 0;
    return RECP_MAX + MAX_CASHIER + CVID + (20 * clerkID);
}

int getNextClerkLock(int clerkID){
    static int LockID = 0;
    return RECP_MAX + MAX_CASHIER + LockID + (20 * clerkID + 10);
}

int getNextDocCV(int docID){
    static int CVID = 0;
    return RECP_MAX + MAX_CASHIER + MAX_DOCTORS + CVID + (20 * docID);
}

int getNextDocLock(int docID){
    static int LockID = 0;
    return RECP_MAX + MAX_CASHIER + MAX_DOCTORS + LockID + (20 * docID + 10);
}

void __Receptionists(Receptionists *recep, int recepID) {
    recep->peopleInLine = 0;
    recep->receptionCV = getNextRecpCV(recepID);
    recep->receptionistWaitCV = getNextRecpCV(recepID);
    recep->ReceptionistBreakCV = getNextRecpCV(recepID);
    recep->transLock = getNextRecpLock(recepID);
    recep->currentToken = 0;
}
void __Cashier(Cashier *cash, int ID) {
    char name[20];
    cash->lineLength = 0;
    cash->patToken = 0;
    cash->fee = 0;
    cash->payment = 0;
    cash->lineCV = getNextCashCV(ID);
    cash->transLock = getNextCashLock(ID);
    cash->transCV = getNextCashCV(ID);
    cash->breakCV = getNextCashCV(ID);
}
void __PharmacyClerks(PharmacyClerks *pcl, int ID) {
    char name[20];
    pcl-> patientsInLine = 0;
    pcl->payment = 0;
    pcl->fee = (int) (1267) % 100;
    pcl->patPrescription = 0;
    pcl->ClerkCV = getNextClerkCV(ID);
    pcl->ClerkBreakCV = getNextClerkCV(ID);
    pcl->ClerkTransLock = getNextClerkLock(ID);
    pcl->ClerkTransCV = getNextClerkCV(ID);
}

void __Doctor(Doctor *doc, int ID) {
    char name[20];
    doc->prescription = -1;
    doc->patientToken = -1;
    doc->peopleInLine = 0;
    strcpy(name, "");
    name = itoa(ID, name);
    strcpy(doc->LineLock, strcat(name, "_LineLock"));
    strcpy(name, "");
    name = itoa(ID, name);
    strcpy(doc->LineCV, strcat(name, "_LineCV"));
    strcpy(name, "");
    name = itoa(ID, name);
    strcpy(doc->doorboyBreakCV, strcat(name, "_Doctor.doorboyBreakCV"));
    strcpy(name, "");
    name = itoa(ID, name);
    strcpy(doc->transLock, strcat(name, "_Doctor.transLock"));
    strcpy(name, "");
    name = itoa(ID, name);
    strcpy(doc->transCV, strcat(name, "_Doctor.transCV"));
}

void createPatient() {
    int temp;
    Acquire(creationLock);
    temp = patientCount;
    patientCount++;
    Release(creationLock);
    patients(temp);
    Exit(0);
}

void createReceptionist() {
    int temp;
    Acquire(creationLock);
    temp = recptionistCount;
    recptionistCount++;
    Release(creationLock);
    receptionist(temp);
    Exit(0);
}

void createDoorBoy() {
    int temp;
    Acquire(creationLock);
    temp = doorboyCount;
    doorboyCount++;
    Release(creationLock);
    doorboy(temp);
    Exit(0);
}

void createDoctor() {
    int temp;
    Acquire(creationLock);
    temp = doctorCount;
    doctorCount++;
    Release(creationLock);
    doctor(temp);
    Exit(0);

}

void createCashier() {
    int temp;
    Acquire(creationLock);
    temp = cashierCount;
    cashierCount++;
    Release(creationLock);
    cashier(temp);
    Exit(0);
}

void createPharmacyClerk() {
    int temp;
    Acquire(creationLock);
    temp = pharmacyCount;
    pharmacyCount++;
    Release(creationLock);
    clerk(temp);
    Exit(0);
}

void createHospitalManager() {
    int temp;
    Acquire(creationLock);
    temp = hospitalmanagerCount;
    hospitalmanagerCount++;
    Release(creationLock);
    hospitalManager(temp);
    Exit(0);
}

#endif
