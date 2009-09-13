#include "patient.h"

Patient::Patient(){
    char debugname[];
    sprintf(debugname,"patient_%d",Random());
    this->Thread(debugname);
    this->TokenNumber = -1;
    this->Prescription = -1;
}

int Patient::chooseDoctor(int TotalDoctorCount){
    return Random() % TotalDoctorCount;
}

int Patient::chooseCashier(int TotalCashierCount){
    return Random()% TotalCashierCount;
}
int Patient::getFee(int chashId){
    //TODO: Get fee to be paid from cashier
}
int Patient::getMedicine(int pharmId){
    //TODO: Get Meds cost from pharacist
}

void Patient::Pay(personType type, int Id){
    //Based on the type pay the particular person
    if(personType == CASHIER){
        //Pay the cashier
    }else if(personType == PHARMACY){
        //Pay the Pharmacy clerk
    }

}