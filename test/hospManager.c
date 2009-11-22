#include "init.h"

int testmode = 0;

void hospitalManager(int ID) {
	char str[50];
	int sleeptime = 0;
	int test5cycles = 1;
	int patientsWaiting = 0;
	int i, j, sum;

	print("H_");
	print(itoa(ID, str));
	print(": Alive\n");

	sleeptime = Random() % 30000;
	while (1) {
		if (test_state == 51 || test_state == 52 || test_state == 53) {
			/*The patients will always be there in the system. */
			/*For test purposes, lets assume the simulation to be */
			/*complete after 100 cycles. */
			if (test5cycles > 0) {
				test5cycles--;
			} else {
				return;
			}
		}
		HLock_Acquire(hospitalLock);
		if (peopleInHospital <= 0) {

			print("H_");
			print(itoa(ID, str));
			print(":  No one to service, Killing myself!!!\n");

			return;
		}
		HLock_Release(hospitalLock);

		sleeptime = Random() % 30000;
		/*Sleep for some random amount of time */

		print("H_");
		print(itoa(ID, str));
		print(":  Sleeping for");
		print(itoa(sleeptime, str));
		print(" cycles\n");

		do {
			Yield();
			sleeptime--;
		} while (sleeptime > 0);
		/*I am on rounds now, Time to kick some ass */

		print("H_");
		print(itoa(ID, str));
		print(": Going on rounds\n");

		/*1. Check on the Receptionists */
		print("H_");
		print(itoa(ID, str));
		print(": Checking receptionists\n");

		patientsWaiting = 0;
		for (j = 0; j < numRecp; j++) {
			patientsWaiting += receptionists[j].peopleInLine;
		}

		if (patientsWaiting > 1) {
			for (j = 0; j < numRecp; j++) {
				HLock_Acquire(recpLineLock);
				HLock_Signal(receptionists[j].ReceptionistBreakCV, recpLineLock);
				HLock_Release(recpLineLock);
			}
		}
		/*2. Query Cashiers */

		print("H_");
		print(itoa(ID, str));
		print(": Checking cashiers\n");

		for (i = 0; i < numCashiers; i++) {/*Check for waiting patients */
			if (cashiers[i].lineLength > 0) {

				print("H_");
				print(itoa(ID, str));
				print(": Found");
				print(itoa(cashiers[i].lineLength, str));
				print(" patients waiting for C_");
				print(itoa(i, str));
				print("  -> Signal Cashier\n");

				/*Wake up this receptionist up */
				HLock_Acquire(cashierLineLock);
				HLock_Broadcast(cashiers[i].breakCV, cashierLineLock);
				HLock_Release(cashierLineLock);

			}
		}

		/*Query cashiers for total sales */

		HLock_Acquire(feesPaidLock);
		print(" T10: Total fees collected by cashiers:");
		print(itoa(feesPaid, str));

		if (test_state == 10) {
			/* this is a test for race conditions, so we can't have any: */
			/* IntStatus oldLevel = interrupt->SetLevel(IntOff);*/
			sum = 0;
			for (i = 0; i < numCashiers; i++) {

				print(" T10: cashier");
				print(itoa(i, str));
				print(" :");
				print(itoa(cashiers[i].sales, str));
				print("\n");

				sum += cashiers[i].sales;
			}

			print("T10: TOTAL:");
			print(itoa(sum, str));

			/* sum just printed should match feesPaid, printed earlier */
			/*  (void) interrupt->SetLevel(oldLevel);*/
		}
		HLock_Release(feesPaidLock);

		/*3. Query pharmacy */

		print("H_");
		print(itoa(ID, str));
		print(":Checking clerks\n");

		for (i = 0; i < numClerks; i++) {/*Check for waiting patients */
			if (clerks[i].patientsInLine > 0) {

				print("H_");
				print(itoa(ID, str));
				print(": found CL_");
				print(itoa(i, str));
				print(": sleeping and ");
				print(itoa(clerks[i].patientsInLine, str));
				print("waiting -> Signaling Clerk\n");

				/*Wake up this clerk up */
				HLock_Acquire(ClerkLinesLock);
				HLock_Signal(clerks[i].ClerkBreakCV, ClerkLinesLock);
				HLock_Release(ClerkLinesLock);

			}
		}

		/*Query clerks for total sales */

		HLock_Acquire(PaymentLock);

		print("H_");
		print(itoa(ID, str));
		print("T10: Total amount collected by clerks: ");
		print(itoa(totalsales, str));
		print("\n");

		if (test_state == 10) {
			/* this is a test for race conditions, so we can't have any: */
			/*   IntStatus oldLevel = interrupt->SetLevel(IntOff);*/
			sum = 0;
			for (i = 0; i < numClerks; i++) {

				print("T10: clerk ");
				print(itoa(i, str));
				print(" : ");
				print(itoa(clerks[i].sales, str));
				print("\n");

				sum += clerks[i].sales;
			}

			print("T10: TOTAL: ");
			print(itoa(sum, str));

			/* sum just printed should match feesPaid, printed earlier */
			/* (void) interrupt->SetLevel(oldLevel);*/
		}
		HLock_Release(PaymentLock);

		Yield();

		/*Check on the doorboys */

		print("H_");
		print(itoa(ID, str));
		print(": Checking doorboys\n");

		for (i = 0; i < numDoctors; i++) {/*Check for waiting patients */
			if (doctors[i].peopleInLine > 0) {

				print("H_");
				print(itoa(ID, str));
				print(": found ");
				print(itoa(doctors[i].peopleInLine, str));
				print(": people in doctor ");
				print(itoa(i, str));
				print("'s line -> Signal Doorboy\n");

				Acquire(doctors[i].LineLock);
				Broadcast(doctors[i].doorboyBreakCV, doctors[i].LineLock);
				Release(doctors[i].LineLock);

			}
		}
	}
	Exit(0);
}

int main(int argc, char** argv) {
	int i;
	char inp[20];
	strcpy(testlock, "TestLock");
	strcpy(TokenCounterLock, "TokenCounterLock");
	strcpy(recpLineLock, "recpLineLock");
	strcpy(feeListLock, "feeListLock");
	strcpy(cashierLineLock, "cashierLineLock");
	strcpy(feesPaidLock, "feesPaidLock");
	strcpy(ClerkLinesLock, "ClerkLineLock");
	strcpy(PaymentLock, "PaymentLock");
	strcpy(hospitalLock, "HospitalLock");
	strcpy(doorboyLineLock, "doorboyLineLock");
	strcpy(doorboyLineCV, "doorboyLineCV");
	strcpy(creationLock, "creationLock");
	wakingDoctorList.queue = wakingdoctor_element;
	wakingDoctorList.length = MAX_PATIENTS;
	wakingDoctorList.head = -1;
	wakingDoctorList.tail = -1;
	Init_Queue(&wakingDoctorList);
	feeList.head = 0;
	/*Initialize datastructures for all the threads
	 //1. Patients don't need initialization
	 //2. Receptionists
	 */
	Write("Initializing Recptionists DS\n", 25, 1);
	for (i = 0; i < RECP_MAX; i++) {
		__Receptionists(&receptionists[i], i);
	}
	/*3. DoorBoy doesn't need anything
	 4. Doctors*/
	print("Initializing Doctors DS\n");
	for (i = 0; i < MAX_DOCTORS; i++) {
		__Doctor(&doctors[i], i);
	}
	print("Initializing Cashiers DS\n");
	/*5. Cashiers*/
	for (i = 0; i < MAX_CASHIER; i++) {
		__Cashier(&cashiers[i], i);
	}
	print("Initializing Clerks DS\n");
	/*6. Clerks */
	for (i = 0; i < MAX_CLERKS; i++) {
		__PharmacyClerks(&clerks[i], i);
	}
	/* 7. Hospital Manager */
	print("Initializing Hospital Manager DS\n");
	for (i = 0; i < totalHospMan; i++) {

	}

	print("Creating 1 Hospital Manager \n");
	Fork(createHospitalManager);

	for (i = 0; i < 100; i++)
		Yield();
}
