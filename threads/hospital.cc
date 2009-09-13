


#include "hospital.h"
#include "Doctor.h"
void Hospital::init(){
    //TODO: Initialize different Classes of people
}
void Hospital::run(){
    //TODO: Prepare all threads and initiate them
    
}
void Hospital::finish();

Hospital::Hospital(){
    //TODO: Initialize the different members of the hospital

    //1. Doctors
    this->totalDocs = Random() % (MAXDOCTORS - MINDOCTORS + 1) + MINDOCTORS;
    this->doctors = new Doctor[this->totalDocs];
    //2. Receptionists
    this->totalRecp = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    this->receptionists = new Receptionist[this->totalRecp];
    //3. Cashiers
    this->totalCash = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    this->cashiers = new Cashier[this->totalCash];
    //4. DoorBoys
    this->totalDoor = this->totalDocs;
    this->doorboys = new DoorBoy[this->totalDoor];
    //5. Pharmacys
    this->totalPharm = Random() % (MAXRCP - MINRCP +1) + MINRCP ;
    this->pharmclerks = new PharmClerk[this->totalPharm];
    //6. HospitalManager
    this->totalHospMan = 1;
    this->hospmanagers = new HospManager[this->totalHospMan];
    //7. Patients
    this->totalPatients = Random() % (MAXPATIENTS - MINPATIENTS +1) + MINPATIENTS ;
    this->receptionists = new Receptionist[this->totalRecp];
}
Hospital::~Hospital(){
    //TODO: Delete all DMA objects before exit
    delete this->cashiers;
    delete this->doctors;
    delete this->doorboys;
    delete this->hospmanagers;
    delete this->patients;
    delete this->pharmclerks;
    delete this->receptionists;
}