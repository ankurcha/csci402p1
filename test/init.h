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

int getNextRecpCV(int recepID) {
    static int CVID = 0;
    return CVID + (ENTITY_OFFSET * recepID);
}

int getNextRecpLock(int recepID) {
    static int LockID = 0;
    return LockID + (ENTITY_OFFSET * recepID + LOCK_OFFSET);
}

int getNextCashCV(int cashID) {
    static int CVID = 0;
    return RECP_MAX * ENTITY_OFFSET + CVID + (ENTITY_OFFSET * cashID);
}

int getNextCashLock(int cashID) {
    static int LockID = 0;
    return RECP_MAX * ENTITY_OFFSET + LockID + (ENTITY_OFFSET * cashID
            + LOCK_OFFSET);
}

int getNextClerkCV(int clerkID) {
    static int CVID = 0;
    return (RECP_MAX + MAX_CASHIER) * ENTITY_OFFSET + CVID + (ENTITY_OFFSET
            * clerkID);
}

int getNextClerkLock(int clerkID) {
    static int LockID = 0;
    return (RECP_MAX + MAX_CASHIER) * ENTITY_OFFSET + LockID + (ENTITY_OFFSET
            * clerkID + LOCK_OFFSET);
}

int getNextDocCV(int docID) {
    static int CVID = 0;
    return (RECP_MAX + MAX_CASHIER + MAX_DOCTORS) * ENTITY_OFFSET + CVID
            + (ENTITY_OFFSET * docID);
}

int getNextDocLock(int docID) {
    static int LockID = 0;
    return (RECP_MAX + MAX_CASHIER + MAX_DOCTORS) * ENTITY_OFFSET + LockID
            + (ENTITY_OFFSET * docID + LOCK_OFFSET);
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
    doc->prescription = -1;
    doc->patientToken = -1;
    doc->peopleInLine = 0;
    doc->LineLock = getNextDocLock(ID);
    doc->transLock = getNextDocLock(ID);
    doc->LineCV = getNextDocCV(ID);
    doc->doorboyBreakCV = getNextDocCV(ID);
    doc->transCV = getNextDocCV(ID);
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

/* Data Update Handling Functions */
int UpdateData_Patient(Packet p) {
    /* !!! */
    return 0;
}

int UpdateData_Receptionist(Packet p) {
    int id, peopleInLine, currentToken;
    id = copyOutInt(p.data, 0);
    peopleInLine = copyOutInt(p.data, 4);
    currentToken = copyOutInt(p.data, 8);
    /* Apply the update */
    receptionists[id].currentToken = currentToken;
    receptionists[id].peopleInLine = peopleInLine;
    return id;
}
int UpdateData_Doorboy(Packet p) {
    /* !!! */
    return 0;
}
int UpdateData_Doctor(Packet p) {
    int id, peopleInLine, prescription, patientToken;
    id = copyOutInt(p.data, 0);
    prescription = copyOutInt(p.data, 4);
    patientToken = copyOutInt(p.data, 8);
    /* Apply update */
    doctors[id].patientToken = patientToken;
    doctors[id].peopleInLine = peopleInLine;
    doctors[id].prescription = prescription;
    return id;
}

int UpdateData_Cashier(Packet p) {
    int id, lineLength, patToken, fee, payment, sales;
    id = copyOutInt(p.data, 0);
    lineLength = copyOutInt(p.data, 4);
    patToken = copyOutInt(p.data, 8);
    fee = copyOutInt(p.data, 12);
    payment = copyOutInt(p.data, 16);
    sales = copyOutInt(p.data, 20);
    /* Apply Update */
    cashiers[id].lineLength = lineLength;
    cashiers[id].patToken = patToken;
    cashiers[id].fee = fee;
    cashiers[id].payment = payment;
    cashiers[id].sales = sales;
    return id;
}

int UpdateData_Clerk(Packet p) {
    int id, patientsInLine, payment, fee, patPrescription, sales;
    id = copyOutInt(p.data, 0);
    patientsInLine = copyOutInt(p.data, 4);
    payment = copyOutInt(p.data, 8);
    fee = copyOutInt(p.data, 12);
    patPrescription = copyOutInt(p.data, 16);
    sales = copyOutInt(p.data, 20);
    /* Apply Update */
    clerks[id].patientsInLine = patientsInLine;
    clerks[id].payment = payment;
    clerks[id].fee = fee;
    clerks[id].patPrescription = patPrescription;
    clerks[id].sales = sales;
    return id;
}

int UpdateData_HospitalManager(Packet p) {
    /* !!! */
    return 0;
}

int UpdateData_Global(Packet p) {
    int status = -1;
    short variableToUpdate = 0x00;
    int value = -1;
    int key = -1;
    variableToUpdate = copyOutShort(p.data, 0);
    switch (variableToUpdate) {
        case NUMDOCTORS:
            value = copyOutInt(p.data, 2);
            numDoctors = value;
            break;
        case NUMCASHIERS:
            value = copyOutInt(p.data, 2);
            numCashiers = value;
            break;
        case NUMCLERKS:
            value = copyOutInt(p.data, 2);
            numClerks = value;
            break;
        case NUMDOORBOYS:
            value = copyOutInt(p.data, 2);
            numDoorboys = value;
            break;
        case NUMRECP:
            value = copyOutInt(p.data, 2);
            numRecp = value;
            break;
        case NUMPATIENTS:
            value = copyOutInt(p.data, 2);
            numPatients = value;
            break;
        case FEESPAID:
            value = copyOutInt(p.data, 2);
            feesPaid = value;
            break;
        case TEST_STATE:
            value = copyOutInt(p.data, 2);
            test_state = value;
            break;
        case TOKENCOUNTER:
            value = copyOutInt(p.data, 2);
            TokenCounter = value;
            break;
        case TOTALSALES:
            value = copyOutInt(p.data, 2);
            totalsales = value;
            break;
        case PEOPLEINHOSPITAL:
            value = copyOutInt(p.data, 2);
            peopleInHospital = value;
            break;
        case DOORBOYLINELENGTH:
            value = copyOutInt(p.data, 2);
            doorboyLineLength = value;
            break;
        case PATCOUNT:
            value = copyOutInt(p.data, 2);
            patientCount = value;
            break;
        case RECPCOUNT:
            value = copyOutInt(p.data, 2);
            recptionistCount = value;
            break;
        case DOORBCOUNT:
            value = copyOutInt(p.data, 2);
            doorboyCount = value;
            break;
        case DOCCOUNT:
            value = copyOutInt(p.data, 2);
            doctorCount = value;
            break;
        case CASHCOUNT:
            value = copyOutInt(p.data, 2);
            cashierCount = value;
            break;
        case CLERKCOUNT:
            value = copyOutInt(p.data, 2);
            pharmacyCount = value;
            break;
        case HOSPMANCOUNT:
            value = copyOutInt(p.data, 2);
            hospitalmanagerCount = value;
            break;
        case FEELIST_APPEND:
            key = copyOutInt(p.data, 2);
            value = copyOutInt(p.data, 6);
            List_Append(&feeList, key, value);
            break;
        case QUEUE_PUSH:
            value = copyOutInt(p.data, 2);
            Queue_Push(&wakingDoctorList, value);
            break;
        case QUEUE_POP:
            /* NO DATA */
            Queue_Pop(&wakingDoctorList);
            break;
        default:
            break;
    }
    return variableToUpdate;
}
#endif
