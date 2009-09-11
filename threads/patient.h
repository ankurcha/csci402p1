#ifdef CHANGED

class Patient{
 private:
  int TokenNumber;
  int Prescription;
 public:
  void setTokenNumber(int);
  int getTokenNumber();
  int chooseDoctor();
  int chooseCashier();
  int getFee(int);
  int getMedicine(int);
  Patient();
  Patient(int);
}

#endif
