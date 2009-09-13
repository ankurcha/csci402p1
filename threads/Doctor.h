/* 
 * File:   Doctor.h
 * Author: ankurcha
 *
 * Created on September 13, 2009, 2:26 PM
 */

#ifndef _DOCTOR_H
#define	_DOCTOR_H

#include "synchlist.h"

class Doctor : public Thread{
public:
    Doctor();
    Doctor(const Doctor& orig);
    virtual ~Doctor();
    void consult(int PatientId);
private:
    SynchList *waitingPatients;
};

#endif	/* _DOCTOR_H */

