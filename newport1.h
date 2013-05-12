#include "common.h"
#include <iostream>
using namespace std;

class mySendingPort :public SendingPort
{

public:

  inline void setACKflag(bool flag){ackflag_ =flag;}
  inline bool isACKed(){return ackflag_;}
  void timerHandler(int TimerType)
  {
    cout<<endl<<"Retransmitting on timer expiry...."<<endl;
   // if (TimerType == 1)
    //  {
        if (!ackflag_)
          {       
            // cout << "Re-send..."; 
            sendPacket(lastPkt_);       
            //schedule a timer again
            timer_.startTimer(5);
          }              
     // }	  
   /* if(TimerType == 2)
      {
        UpdateNow = true;
	char x = 'x';
        sendUpdate(100,x);
	timer_u.startTimer(5);
      }*/
  }  
private:
  bool ackflag_;
public:
  Packet * lastPkt_;
 // bool UpdateNow;
};
