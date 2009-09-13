#ifndef _HOSPITAL_H
#define _HOSPITAL_H



#include "synchlist.h"

#include "patient.h"
#include "Doctor.h"
#include "Receptionist.h"
#include "Cashier.h"
#include "DoorBoy.h"
#include "PharmClerk.h"
#include "HospManager.h"


#define MINPATIENTS 20
#define MAXPATIENTS 100
#define MINDOCTORS 4
#define MAXDOCTORS 10
#define MINRCP 3
#define MAXRCP 5


typedef enum PersonType{
    PATIENT,
    DOCTOR,
    RECEPTIONIST,
    CASHIER,
    DOORBOY,
    PHARMACY,
    HOSPITALMANAGER
}personType;

class Hospital{
private:
    int totalDocs;
    Doctor *doctors;
    DoorBoy *doorboys;
    Patient *patients;
    Receptionist *receptionists;
    Cashier *cashiers;
    PharmClerk *pharmclerks;
    HospManager *hospmanagers;
    
    int totalRecp;
    int totalCash;
    int totalDoor;
    int totalPharm;
    int totalHospMan;
    int totalPatients;
public:
    void init();
    void run();
    void finish();
    Hospital();
    ~Hospital();
};
#endif