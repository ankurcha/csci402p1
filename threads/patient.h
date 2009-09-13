#ifndef _PATIENT_H
#define _PATIENT_H
#endif

#ifndef _HOSPITAL_H
#include "hospital.h"
#endif

#include "thread.h"


class Patient : public Thread{
private:
    int currentState;
    int TokenNumber;
    int Prescription;
public:

    Patient();
    ~Patient();

    int GetPrescription() const {
        return Prescription;
    }

    void SetPrescription(int Prescription) {
        this->Prescription = Prescription;
    }

    int GetTokenNumber() const {
        return TokenNumber;
    }

    void SetTokenNumber(int TokenNumber) {
        this->TokenNumber = TokenNumber;
    }
    int chooseDoctor(int);
    int chooseCashier();
    int getFee(int);
    int getMedicine(int);
    void Pay(personType,int);
};