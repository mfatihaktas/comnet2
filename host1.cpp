#include "common.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include "newport1.h"

using namespace std;

class Packet;
class Port;
class Address;
class SendingPort;

int main(int argc, const char * argv[])
  {
  ifstream FPtr;
  char id, Src, Dst, fn;
  int TxP, RxP, DstP;
  int Pksize=1400;
  char *memblock, *SFName;
  FPtr.open("sample_network.txt", ios::in);
  id = (char)(*argv[1]);
  cout<<endl<<"The id of this host is "<<id;
  cout<<endl<<"Config File Successfully opened!!";
  while(1)
    {
      FPtr>>Src;
      FPtr>>RxP;
      FPtr>>TxP;
      FPtr>>DstP;
      FPtr>>Dst;
      if(Src == id)
        {
          cout<<endl<<"Match found";
          break;
        }
      else if(FPtr.eof())
        break;
    }
  FPtr.close();
  cout<<endl<<"Link to neighbor found!!";
  cout<<endl<<"TxP = "<<TxP<<"      RxP = "<<RxP<<"     DstP = "<<DstP<<"     Dst = "<<Dst;
  // int m,k,Pksize=1492;
  // char * memblock, * SFName;
  try 
    {
      const char *id1 = "localhost";
      const char *Dst1 = "localhost";
      // *id1 = id;
      // *Dst1 = Dst; 
      Address *my_addr = new Address(id1, TxP);
      Address *dst_addr =  new Address(Dst1, DstP);
      mySendingPort *my_port = new mySendingPort();
      my_port->setAddress(my_addr);
      my_port->setRemoteAddress(dst_addr);
      my_port->init();

      //configure receiving port to listen to ACK frames
      Address *my_rx_addr = new Address(id1, RxP);
      LossyReceivingPort * my_rx_port = new LossyReceivingPort(0);
      my_rx_port->setAddress(my_rx_addr);
      my_rx_port->init();

      /* Transmitting the packet to the gateway router */
      Packet * Hello;
      Hello = new Packet();
      PacketHdr *hdr1 = Hello->accessHeader();
      // Hello->setPayloadSize(15);
 	hdr1->setOctet('H',0);      
	hdr1->setOctet(id,1);
     
     
      // Hello->fillPayload(15, (char *)"This is a HELLO");
      my_port->sendPacket(Hello);
      cout<<endl<<"HELLO Packet Sent to "<<Dst<<"!!!";
      my_port->lastPkt_ = Hello;
      my_port->setACKflag(false);
      //schedule retransmit
      my_port->timer_.startTimer(25);
      cout <<endl<<"begin waiting for ACK from gateway router........" <<endl;
      Packet *Pack;
      int x = 1, y = 1;
      while (x==1 || y==1)
        {
          Pack = my_rx_port->receivePacket();
          if (Pack!= NULL)
	    {
	      if(Pack->accessHeader()->getOctet(0) == 'A' && Pack->accessHeader()->getOctet(1) == Dst)
	        {
	          my_port->setACKflag(true);
	          my_port->timer_.stopTimer();
                  cout<<endl<<"ACK for HELLO received from "<<Dst<<"!!!"<<endl<<"........";
                  x = 0;
	        }
              if(Pack->accessHeader()->getOctet(0) == 'H' && Pack->accessHeader()->getOctet(1) == Dst)
	        {
                  cout<<"A HELLO packet has been received from "<<Dst<<"!!!"<<endl<<"........";
                  y = 0;
                  Packet *ACKPkt;
                  ACKPkt = new Packet();
                  PacketHdr *hdr1 = ACKPkt->accessHeader();
                  // ACKPkt->setPayloadSize(14);
               
                  hdr1->setOctet('A',0);
                  hdr1->setOctet(id,1);

	          // ACKPkt->fillPayload(14, (char *)"This is an ACK");
                  my_port->sendPacket(ACKPkt);
                  cout<<endl<<"ACK for HELLO has been sent to "<<Dst<<"!!!"<<endl<<"........";
                  delete ACKPkt;
	        }

	    }
        }
      cout<<endl<<endl<<"Do you want to send or receive [s/r]   :   ";
      cin>>fn;
      }

  catch(const char *reason)
    {
      cerr<<endl<<"Exception:  "<<reason<<endl;
      exit(-1);
    }
  return(0);
}
