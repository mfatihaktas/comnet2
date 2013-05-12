#include "common.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include "newport.h"
#include <string>
#include <vector>
#include <algorithm>
#include <stdint.h>

using namespace std;

class Packet;
class Port;
class Address;
class SendingPort;

struct nhopinfo
  {
    char id,Type;
    int TxP, RxP, DstP;
  };

struct route
  {
    char Dst,NxtHop;
    int Dist;
  };

char id;
int NNeighbors = 0;
int NNeighbors_replied_hello = 0;
bool discovery_done = false;

vector<route> RtngTbl;
vector<nhopinfo> NbrTbl;
vector<char> Destinations;
Address **my_addr, **dst_addr, **my_rx_addr, **my_addr1;
mySendingPort **my_port, **my_port1;
LossyReceivingPort **my_rx_port;

void sendUpdate(int tid, char Dest)
  {
    
    
     int temp = 0;
     vector<char> Dsts;
     vector<int> HpCnts;
     for(int i=0; i<Destinations.size(); i++)
       {
         int min = 100,exists = 0, DontInclude = 0;
         for(int j=0; j<RtngTbl.size(); j++)
           {
             if(RtngTbl[j].Dst == Destinations[i] && RtngTbl[j].Dist < min && RtngTbl[j].NxtHop != Dest)
	       {
	         min = RtngTbl[j].Dist;
		 exists = 1;
	       }
	     //if(RtngTbl[j].Dst == Destinations[i]  && RtngTbl[j].NxtHop == Dest)
	     //  DontInclude = 1;
           }
	 // if(exists == 1 && DontInclude == 0)
	 if(exists == 1)
	   {
	     Dsts.push_back(Destinations[i]);
	     HpCnts.push_back(min);
	     temp++;
	   }
       }
 }


void *ListenOnRxP(void * threadid)
  {
    int tid = (uintptr_t)(threadid);
    cout<<endl<<"Starting to listen on port "<<NbrTbl[tid].RxP<<"......";
    Packet *Pack;

    while(1)
      {
        Packet *Pack = NULL;
	Pack = my_rx_port[tid]->receivePacket();
     
//cout<<"hello";
        /* if(my_port[tid]->UpdateNow == true)
          {
            cout<<endl<<"Transmitting UPDATE on timeout to "<<NbrTbl[tid].id;
            sendUpdate(tid, NbrTbl[tid].id);
          }*/
	if(Pack != NULL)
	  {
	    char PkType = Pack->accessHeader()->getOctet(0);

/*
check if the rxed packet type is ls
if it is create_pthread(..., handle_rx_ls(port #,.))
.
.
void handle_rx_ls(port #, ..){
	extract direct connectivity info from the rxed_ls_pkt
	put this extracted info to net_map
}

*/
if(PkType == 'H')
	      {
	        
	        Packet *ACKPkt = new Packet;
	        // ACKPkt->setPayloadSize(14);
                PacketHdr *hdr1 = ACKPkt->accessHeader();
		hdr1->setOctet('A',0);
		hdr1->setOctet(id,1);
		
		// cout<<endl<<"ACK packet number = "<<ACKPkt->accessHeader()->getIntegerInfo(3);
		// ACKPkt->fillPayload(14,(char *)"This is an ACK");
		my_port[tid]->sendPacket(ACKPkt);
		//cout<<endl<<"Sent ACK for HELLO from "<<Pack->accessHeader()->getOctet(0);
		++NNeighbors_replied_hello;
		if(NNeighbors_replied_hello == NNeighbors)
			discovery_done = true;
		delete ACKPkt;
		}
if(PkType == 'A')
	      {

cout<<endl<<"Received ACK from "<<Pack->accessHeader()->getOctet(1);
		    my_port[tid]->setACKflag(true);

	      
		char UpdateFrm = Pack->accessHeader()->getOctet(1);
                route NewRoute;
			NewRoute.NxtHop = UpdateFrm;
			/*if(UpdateFrm=='a'&& a[0]==0){a[0]++;}
if(UpdateFrm=='b' && a[1]==0){a[1]++;}
if(UpdateFrm=='c' && a[2]==0){a[2]++;}
if(UpdateFrm=='d'&& a[3]==0){a[3]++;}
if(UpdateFrm=='x'&& a[4]==0){a[4]++;}
if(UpdateFrm=='y'&& a[5]==0){a[5]++;}
if(UpdateFrm=='z'&& a[6]==0){a[6]++;}
if(UpdateFrm=='w'&& a[7]==0){a[7]++;}*/
			
			RtngTbl.push_back(NewRoute);
			for(int k=0; k<NNeighbors; k++)
			  {
			   
			        //cout<<endl<<"Calling send Update on routing table change, sending to "<<NbrTbl[k].id;
			        sendUpdate(k, NbrTbl[k].id);
			       
			  }
			cout<<endl<<endl<<"*************"<<"Routing Table"<<"*************"<<endl;	  
			for(int k=0; k<RtngTbl.size(); k++)
		          {
		            cout<<"    "<<RtngTbl[k].NxtHop<<"    "<<endl;		     
		          }

/*for(i=0;i<8;i++)
{
if(a[i]==1)
j++;
}
if(j==NNeighbors)
break;*/

		}
}

}
 pthread_exit(NULL);
}


int main(int argc, const char *argv[])
  {
    ifstream FPtr;
    char Src, Dst;
    int TxP, RxP, DstP;
	int Pksize=1400;
  	char *memblock, *SFName;
    FPtr.open("sample_network.txt", ios::in);
    id = (char)(*argv[1]);
    cout<<endl<<"The id of this host is "<<id;
    cout<<endl<<"Config File Successfully opened!";
    int j = 1;
    while(!FPtr.eof())
      {
        FPtr>>Src;
        FPtr>>RxP;
	FPtr>>TxP;
	FPtr>>DstP;
	FPtr>>Dst;
	if(Src == id)
	  ++NNeighbors;
      }
cout<<endl<<"Number of Neighbors="<<NNeighbors<<endl;
static int i=0;
FPtr.close();
FPtr.open("sample_network.txt", ios::in);
 while(i<NNeighbors)
      {
        FPtr>>Src;
        FPtr>>RxP;
        FPtr>>TxP;
        FPtr>>DstP;
        FPtr>>Dst;
        if(Src == id)
          {
            nhopinfo NewNeighbor;
	    NewNeighbor.id = Dst;
	    NewNeighbor.RxP = RxP;
	    NewNeighbor.TxP = TxP;
            NewNeighbor.DstP = DstP;
            NbrTbl.push_back(NewNeighbor);
	    /* NbrTbl[i].id = Dst;
	    NbrTbl[i].TxP = TxP;
	    NbrTbl[i].RxP = RxP;
	    NbrTbl[i].DstP = DstP;*/
	    i++;
          }
        if(FPtr.eof())
          break;
      }
  FPtr.close();

    // NNeighbors = i;
    cout<<endl<<"The links discovered are...."<<endl;
    for(int i=0; i<NNeighbors; i++)
      {
        cout<<id<<"  "<<NbrTbl[i].TxP<<"  "<<NbrTbl[i].RxP<<"  "<<NbrTbl[i].DstP<<"  "<<NbrTbl[i].id<<endl;
      }

try
      {
        const char *id1 = "localhost";
	const char *Dst1 = "localhost";
        my_addr = new Address*[NNeighbors];
        my_addr1 = new Address*[NNeighbors];
	dst_addr = new Address*[NNeighbors];
	my_port = new mySendingPort*[NNeighbors];
	my_port1 = new mySendingPort*[NNeighbors];
	my_rx_addr = new Address*[NNeighbors];
	my_rx_port = new LossyReceivingPort*[NNeighbors];

	// The X Packet
	Packet *XPkt = new Packet();
	PacketHdr *hdr = XPkt->accessHeader();
	hdr->setOctet('X',0);
	hdr->setOctet(id,1);


        for(int i=0; i<NNeighbors; i++)
	  {
	    
	    // *Dst1 = NbrTbl[i].id;
	    cout<<endl<<"Going to initialize port set "<<i;
	    my_addr[i] = new Address(id1, NbrTbl[i].TxP);
	    my_addr1[i] = new Address(id1, NbrTbl[i].TxP+100);
	    dst_addr[i] = new Address(Dst1, NbrTbl[i].DstP);
	    my_port[i] = new mySendingPort();
	    my_port[i]->setAddress(my_addr[i]);
	    my_port[i]->setRemoteAddress(dst_addr[i]);
	    my_port[i]->init();
	    /*my_port1[i] = new mySendingPort();
	    my_port1[i]->setAddress(my_addr1[i]);
	    my_port1[i]->setRemoteAddress(dst_addr[i]);
	    my_port1[i]->init();
	    my_port1[i]->lastPkt_ = XPkt;
	    my_port1[i]->timer_u.startTimer(25);*/
	    cout<<endl<<"Tx Port "<<i<<" initialized.";
            my_rx_addr[i] = new Address(id1, NbrTbl[i].RxP);
	    my_rx_port[i] = new LossyReceivingPort(0);
	    my_rx_port[i]->setAddress(my_rx_addr[i]);
	    my_rx_port[i]->init();
	    cout<<endl<<"Rx Port "<<i<<" initialized.";
	  }

cout<<endl<<"All ports for all links initialized!!"<<endl;

 // Creating and transmitting the HELLO packets
	Packet **Hello = new Packet*[NNeighbors];
	for(int i=0; i<NNeighbors; i++)
	  {
	    Hello[i] = new Packet();
	    // Hello[i]->setPayloadSize(0);
	    PacketHdr *hdr1 = Hello[i]->accessHeader();
	    hdr1->setOctet('H',0);
	    hdr1->setOctet(Src,1);
	    //hdr1->setOctet(NbrTbl[i].id,2);
            //hdr1->setOctet('R',3);
//Hello[i]->fillPayload(15, (char *)"This is a HELLO");
            my_port[i]->sendPacket(Hello[i]);
	    my_port[i]->lastPkt_ = Hello[i];
	    my_port[i]->setACKflag(false);
	    my_port[i]->timer_.startTimer(25);
		    
	//delete Hello[i];
	  }

cout<<endl<<"HELLO Packet has been sent to all neighbors."<<endl<<"......"<<endl<<"Now starting to listen on all ports......"<<endl;   
cout<<endl<<"..........";

// Creating threads for each of the receiving ports
	int rc;
	pthread_t threads[NNeighbors];
	for(int i=0;i<NNeighbors;i++)
	  {
	
	    rc = pthread_create(&threads[i], NULL, ListenOnRxP, (void *)(uintptr_t)i);
	    if(rc)
	      {
	        cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
		exit(-1);
	      }
	  }
      }
    catch(const char *reason)
      {
        cerr<<"Exception  :  "<<reason<<endl;
	exit(-1);
      }
	/*Wait for the discovery
	  use bool variable; "discovery_done" for checking if the discovery is done
	*/
	//State A: Create the threads for ls_packet sending

	/*
	initiate timer
	create_pthread (handle_tx_ls());
	.
	.
	void handle_tx_ls(){
		while(1){
			If timer expires
				for every port create_pthread(send_ls(.))
		}
		


	}
	for every port create_thread(..., send_ls);
	.
	.
	void send_ls(){	...	}
	*/
    pthread_exit(NULL);
    return 0;
  }






