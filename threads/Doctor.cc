/* 
 * File:   Doctor.cc
 * Author: ankurcha
 * 
 * Created on September 13, 2009, 2:26 PM
 */

#include "Doctor.h"

Doctor::Doctor() {
    this->waitingPatients = new SynchList();
}

Doctor::Doctor(const Doctor& orig) {
}

Doctor::~Doctor() {
}

Doctor::consult(int PatientId){
    //This process will cause this doctor to consult this patient by causing to
    //Yielding 10 - 20 times
    
}
