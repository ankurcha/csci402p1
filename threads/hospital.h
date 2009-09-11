#ifdef CHANGED
#include "SynchList.cc"
class Hospital{
 private:
  int HID;
  SynchList **cashQueues;
  SynchList **pharmQueue;
  SynchList *receptionQueue;
  SynchList *doctorQueue;

 public:
  Hospital();
  Hospital(int hospitalID);
}
#endif

