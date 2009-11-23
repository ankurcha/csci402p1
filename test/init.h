#include "syscall.h"
#include "queue.h"
#include "itoa.h"
#include "print.h"
#include "string.h"

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
int feesPaid = 0;
int test_state = 0;

struct linkedlist_element {
	/*Used for storing the <token,fees> pairs */
	int key, value;
};

struct list {
	struct linkedlist_element listArray[MAX_PATIENTS];
	int head;
};
typedef struct list List;

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
/*queue elements for the waking doctors list*/
queue_element wakingdoctor_element[MAX_PATIENTS];

char testlock[20];
/* tokenCounter for assigning tokens to patients */
char TokenCounterLock[20];
int TokenCounter = 0;
/* global for all receptionists */
char recpLineLock[20];

/*shared data struct related to a Receptionist */
struct Receptionists_ {
	/* receptionist line CV */
	char receptionCV[20];
	int peopleInLine;

	/* receptionist transactional lock and CV and protected variables */
	char transLock[20];
	char receptionistWaitCV[20];
	int currentToken;

	/* receptionist break CV */
	char ReceptionistBreakCV[20];

};
typedef struct Receptionists_ Receptionists;

void __Receptionists(Receptionists *recep, int recepID) {
	char name[20];
	name = "";
	name = itoa(recepID, name);
	recep->peopleInLine = 0;
	name = "";
	name = itoa(recepID, name);
	strcpy(recep->receptionCV, strcat(name, "_receptionCV"));
	name = "";
	name = itoa(recepID, name);
	strcpy(recep->transLock, strcat(name, "_Receptionists.transLock"));
	name = "";
	name = itoa(recepID, name);
	strcpy(recep->receptionistWaitCV, strcat(name, "_receptionistWaitCV"));
	name = "";
	name = itoa(recepID, name);
	strcpy(recep->ReceptionistBreakCV, strcat(name, "_ReceptionistBreakCV"));
	recep->currentToken = 0;
}

/* list mapping patient tokens to consultFees */
LockId feeListLock;
List feeList;
/* global for all cashiers */
LockId cashierLineLock;
LockId feesPaidLock;

/* shared data struct related to a Cashier */
struct Cashier_ {
	/* line CV and length */
	int lineLength;
	char lineCV[20];

	/* transaction lock, CV, and variables protected by the former */
	char transLock[20];
	char transCV[20];
	int patToken;
	int fee;
	int payment;

	/*protected by feesPaidLock, but only modified by one thread */
	int sales;

	/* cashier's CV for going on break */
	char breakCV[20];
};
typedef struct Cashier_ Cashier;
void __Cashier(Cashier *cash, int ID) {
	char name[20];
	cash->lineLength = 0;
	cash->patToken = 0;
	cash->fee = 0;
	cash->payment = 0;
	name = "";
	name = itoa(ID, name);
	strcpy(cash->lineCV, strcat(name, "_Cashier.lineCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(cash->transLock, strcat(name, "_Cashier.transLock"));
	name = "";
	name = itoa(ID, name);
	strcpy(cash->transCV, strcat(name, "_Cashier.transCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(cash->breakCV, strcat(name, "_Cashier.breakCV"));
}
void _Cashier(Cashier *cash) {

}
char ClerkLinesLock[20];
char PaymentLock[20];
int totalsales = 0;
/* hospitalLock protects the count of patients remaining in the hospital */
char hospitalLock[20];
int peopleInHospital = 1;

struct PharmacyClerks_ {
	int patientsInLine;
	/* int state; */
	int payment;
	int fee;
	int patPrescription;
	char ClerkCV[20];

	char ClerkBreakCV[20];
	char ClerkTransLock[20];
	char ClerkTransCV[20];

	/*protected by PaymentLock */
	int sales;
};
typedef struct PharmacyClerks_ PharmacyClerks;
void __PharmacyClerks(PharmacyClerks *pcl, int ID) {
	char name[20];
	pcl-> patientsInLine = 0;
	/* pcl->state = FREE; */
	pcl->payment = 0;
	pcl->fee = (int) (1267) % 100;
	pcl->patPrescription = 0;
	name = "";
	name = itoa(ID, name);
	strcpy(pcl->ClerkCV, strcat(name, "_ClerkCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(pcl->ClerkBreakCV, strcat(name, "_ClerkBreakCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(pcl->ClerkTransLock, strcat(name, "_ClerkTransLock"));
	name = "";
	name = itoa(ID, name);
	strcpy(pcl->ClerkTransCV, strcat(name, "_ClerkTransCV"));
}

struct Doctor_ {
	/* line lock and CV and protected variables */
	char LineLock[20];
	char LineCV[20];
	char doorboyBreakCV[20];
	char transLock[20];
	char transCV[20];

	int peopleInLine;
	int prescription;
	int patientToken;
};
typedef struct Doctor_ Doctor;
void __Doctor(Doctor *doc, int ID) {
	char name[20];
	doc->prescription = -1;
	doc->patientToken = -1;

	doc->peopleInLine = 0;
	name = "";
	name = itoa(ID, name);
	strcpy(doc->LineLock, strcat(name, "_LineLock"));
	name = "";
	name = itoa(ID, name);
	strcpy(doc->LineCV, strcat(name, "_LineCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(doc->doorboyBreakCV, strcat(name, "_Doctor.doorboyBreakCV"));
	name = "";
	name = itoa(ID, name);
	strcpy(doc->transLock, strcat(name, "_Doctor.transLock"));
	name = "";
	name = itoa(ID, name);
	strcpy(doc->transCV, strcat(name, "_Doctor.transCV"));
}
void _Doctor(Doctor *doc) {
}
/* globals to track the queue of doorboys waiting to service doctors */
char doorboyLineLock[20];
char doorboyLineCV[20];
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

char creationLock[20];
int patientCount = 0;
int recptionistCount = 0;
int doorboyCount = 0;
int doctorCount = 0;
int cashierCount = 0;
int pharmacyCount = 0;
int hospitalmanagerCount = 0;

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

