#include "init.h"

void doorboy( ID) {
	char str[50];
	int myDoctor = 0;
	char doorboyBreak = 0;

	while (1) {

		print("DB_");
		print(itoa(ID, str));
		print(": Alive ");

		/*Get into the doorboyLine till some doctor asks for me */
		HLock_Acquire(doorboyLineLock);

		doorboyLineLength++;

		print("DB_");
		print(itoa(ID, str));
		print(": Waiting for some doctor to wake me up.");
		print("\n");

		HCV_Wait(doorboyLineCV, doorboyLineLock);

		doorboyLineLength--;

		/*Some doctor woke me up, lets check who */
		/*myDoctor =  wakingDoctorID; */
		if (Queue_IsEmpty(&wakingDoctorList) == 1) {

			print("DB_");
			print(itoa(ID, str));
			print(": ERROR: Waking doctor list is empty!\n");

			continue;
		}
		myDoctor = Queue_Pop(&wakingDoctorList);
		if (test_state == 2) {

			print("DB_");
			print(itoa(ID, str));
			print(":TEST2: Servicing D_");
			print(itoa(myDoctor, str));
			print("\n");

		} else {

			print("DB_");
			print(itoa(ID, str));
			print(":Servicing D_");
			print(itoa(myDoctor, str));
			print("\n");

		}

		HLock_Release(doorboyLineLock);

		/* Inform the doctor that I have arrived, and wait for him to take  */
		/*  a break, if he so chooses */
		HLock_Acquire(doctors[myDoctor].transLock);
		HCV_Signal(doctors[myDoctor].transCV, doctors[myDoctor].transLock);
		HCV_Wait(doctors[myDoctor].transCV, doctors[myDoctor].transLock);

		/*/// PATIENT LINE ///// */
		/*Acquire the lock to get the state of the line and take decision */

		HLock_Acquire(doctors[myDoctor].LineLock);

		print("DB_");
		print(itoa(ID, str));
		print(": Checking for Patients\n");

		/*while there is noone in line */
		doorboyBreak = 0;
		while (doctors[myDoctor].peopleInLine <= 0) {
			doorboyBreak = 1;
			/*I will be woken up by the manager only!! */

			/* prefix for test conditions */

			if (myDoctor == 0 && test_state == 8)
				print("T8: ");

			if (test_state == 11)
				print("T11: ");
			if (test_state == 2) {
				print("DB_");
				print(itoa(ID, str));
				print(":TEST2: Yawn!!...ZZZZzzzzz....\n");
			} else {
				print("DB_");
				print(itoa(ID, str));
				print(": Yawn!!...ZZZZzzzzz....\n");

			}

			HCV_Wait(doctors[myDoctor].doorboyBreakCV,
					doctors[myDoctor].LineLock);
			/* I got woken up, time to go back to work - by now there are  */
			/*  people dying on the floor! */
		}
		if (doorboyBreak) {
			/* prefix for test 8 condition */

			if (myDoctor == 0 && test_state == 8) {
				print("T8: \n");
			}
			print("DB_");
			print(itoa(ID, str));
			print(": Woken up!\n");
		}

		print("DB_");
		print(itoa(ID, str));
		print(": Found ");
		print(itoa(doctors[myDoctor].peopleInLine, str));
		print(" patients waiting in line for D_ ");
		print(itoa(myDoctor, str));
		print("\n");

		/*Now wake the patient up to go to the doctor */

		print("DB_");
		print(itoa(ID, str));
		print(":Tell patient to go to doctor D_");
		print(itoa(myDoctor, str));
		print("\n");
		HCV_Signal(doctors[myDoctor].LineCV, doctors[myDoctor].LineLock);
		/*My job with the patients and the doctor is done */
		/*I can go back on the doorboyLine */
		Release(doctors[myDoctor].transLock);
		Release(doctors[myDoctor].LineLock);

	}/*End of while */

	print("DB_");
	print(itoa(ID, str));
	print(":Dying...AAAaaaahhhhhhhhh!!\n");

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

	numDoctors = numberOfEntities[2];
	numDoorboys = numDoctors;
	print("Creating ");
	print(itoa(numDoorboys, str));
	print(" Doorboys\n");
	for (i = 0; i < numDoorboys; i++)
		Fork(createDoorBoy);

	for (i = 0; i < 100; i++)
		Yield();
}

