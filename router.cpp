#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>     /* atoi */
#include <pthread.h>
#include "router.h"

//using namespace std;

//PACKET HEADER FIELD INDEXES
#define PACKET_TYPE_INDEX 0
#define TTL_INDEX 1
#define SRC_ADDR_INDEX 2
#define NUM_ADDR_INDEX 3
#define PAYLOAD_SIZE_INDEX(num_addr) (NUM_ADDR_INDEX + num_addr+1)
//PACKET HEADER FIELD SPECS
#define DATA_PACKET_TYPE 'd'

void Router::print_packet(Packet* p){
	//First print the header
	PacketHdr *hdr = p->accessHeader();
	
	std::cout << "PRINT PACKET with Header size: " <<hdr->getSize() << "\n";
	std::cout << "packet_type: " <<hdr->getOctet(PACKET_TYPE_INDEX) << "\n";
	std::cout << "ttl: " << hdr->getOctet(TTL_INDEX) << "\n";
	std::cout << "src_addr: " << hdr->getOctet(SRC_ADDR_INDEX) << "\n";
	char num_addr = hdr->getOctet(NUM_ADDR_INDEX);
	std::cout << "num_addr: " << num_addr << "\n";
	
	char dest_addrs_buffer[num_addr+1];
	int i;
	for(i=0; i < num_addr; i++){
		dest_addrs_buffer[i]=hdr->getOctet(NUM_ADDR_INDEX+i+1);
	}
	dest_addrs_buffer[num_addr]='\0';
	std::cout << "packet_identifier: " << dest_addrs_buffer << "\n";
	//Second print the payload
	std::cout << "packet_payload: " << p->getPayload() << "\n\n";
}

Packet* Router::create_data_packet(char p_type, char ttl, char src_addr, char num_addr, 
														 			 char* dest_addrs, int payload_size, char* payload){
	Packet * my_dpacket;
	my_dpacket = new Packet();//create the data packet object
	
	//Settint the specific header fields
	PacketHdr *hdr = my_dpacket->accessHeader();
	hdr->setOctet(p_type,PACKET_TYPE_INDEX);
	hdr->setOctet(ttl,TTL_INDEX);
	hdr->setOctet(src_addr,SRC_ADDR_INDEX);
	hdr->setOctet(num_addr,NUM_ADDR_INDEX);
	for(int i=0; i<num_addr; i++){
		hdr->setOctet(*(dest_addrs+i), NUM_ADDR_INDEX+i+1);
	}
	hdr->setOctet(payload_size,PAYLOAD_SIZE_INDEX(num_addr));
	//Fill the packet with the data payload
	my_dpacket->fillPayload(payload_size, payload);
	
	return my_dpacket;
}
void Router::forward_incoming_data_packet(Packet* p){
	if( p == NULL){ //check for null packet
		std::cout << "Rxed packet is NULL !\n";
		exit(1);
	}
	
	int* f_ports = handle_rxed_data_packet(p);
}
int* Router::handle_rxed_data_packet(Packet* p){
	PacketHdr *hdr = p->accessHeader();
	char p_type = hdr->getOctet(PACKET_TYPE_INDEX);
	if(p_type != 'd'){ //check for packet type
		std::cout << "This function is supposed to be given DATA packet. This has another type !\n";
		exit(1);
	}
	//Extract necessary info for giving the forwarding decision
	char num_addr = hdr->getOctet(NUM_ADDR_INDEX);
	int* d_p = new int[int(num_addr-'0')];
	char dest_addrs_buffer[num_addr+1];
	for(int i=0; i < num_addr; i++){
		dest_addrs_buffer[i]=hdr->getOctet(NUM_ADDR_INDEX+i+1);
	}
	//dest_addrs_buffer[num_addr] = '/0';
	ftf.forward_to_port(int(num_addr - '0'), (const char*)dest_addrs_buffer, &d_p);
	
	return d_p;
}

void Router::simple_exp(int source_index, graph_t& g, const char *name, 
												std::map<char,int>& vname_vindex, std::map<char,int>& nhop_port){
	ftf.set_all(0,g, name, vname_vindex, nhop_port);
  ftf.do_initial_job();
  
  //Some experiments
  int f_port = ftf.forward_to_port(1, "D", NULL);
	std::cout << "unicast to \"D\" forwarding_port: " << f_port << std::endl;
	int* f_ports = new int[3];
	f_port = ftf.forward_to_port(3, "GEF", &f_ports);
	std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
	delete f_ports;
}

#define MAX_LINE_SIZE 50
void Router::find_numneighbors_init_nbrtbl(char* file_name){
	char line [MAX_LINE_SIZE];
	if(file_ptr->is_open()){
		while(file_ptr->good())
		{
			file_ptr->getline(line,MAX_LINE_SIZE);
		  if(line[0] == id)
		  {
				++num_neighbors;
		  }
		}
		file_ptr->clear(); //clear the failure flag
		file_ptr->seekg(0, file_ptr->beg);
		file_ptr->close();
	}
}

void Router::port_info_read_fromtxt(char* file_name){
  try{
  	file2_ptr = new ifstream( file_name , ifstream::in);
  	//file2_ptr->open(file_name, ios::in);
  }catch(const char *reason){
  	std::cout << reason << "\n";
  }
	if(!file2_ptr->is_open()){
		std::cout << "File could not be opened !\n";
		exit(1);
	}
	char line [MAX_LINE_SIZE];
	int which_neighbor = -1;
	
	while(file2_ptr->good())
  {
	  file2_ptr->getline(line,MAX_LINE_SIZE);
    if(line[0] == id)
    {
	    ++which_neighbor;
  	  std::cout << "line:" << line << std::endl;
    	char* token = strtok (line," ");
    	int counter = -1;
    	
    	while (token != NULL)
			{
				//std::cout << "token:" << token << std::endl;
				++counter;
				switch(counter){
				case 1: strcpy(nbr_tbl[which_neighbor].RxP, token);
				case 2: strcpy(nbr_tbl[which_neighbor].TxP, token);
				case 3: strcpy(nbr_tbl[which_neighbor].DstP, token);
				case 4: strcpy(nbr_tbl[which_neighbor].id, token);
				}
				token = strtok (NULL, " ");
			}
			
    }
  }
  file2_ptr->close();
  
	cout<<endl<<"The links discovered are...."<<endl;
	for(int i=0; i<num_neighbors; i++)
  {
    cout << id << " TxP:" << nbr_tbl[i].TxP << " RxP:" << nbr_tbl[i].RxP
    		 << " DstP:" << nbr_tbl[i].DstP << " Id:" << nbr_tbl[i].id << std::endl;
  }
}

pthread_mutex_t mutex_bst_lop = PTHREAD_MUTEX_INITIALIZER;
int lop_thread_index = -1;
void* Router::bst_listen_on_port(void* context){ //void* thread_id){
	pthread_mutex_lock( &mutex_bst_lop );
	++lop_thread_index;
	pthread_mutex_unlock( &mutex_bst_lop );
	return ((Router*)context)->listen_on_port((void *)&lop_thread_index);
}

pthread_mutex_t mutex_lop = PTHREAD_MUTEX_INITIALIZER;
void* Router::listen_on_port(void* thread_id){
	int* tid = (int *)(thread_id);
	pthread_mutex_lock( &mutex_lop );
	int tlocal_tid = *tid;
	pthread_mutex_unlock( &mutex_lop );
  cout<<endl<<"Starting to listen on port " << nbr_tbl.at(tlocal_tid).RxP<<"......\n";
  PacketHdr *hdr;

  while(1){
  	Packet* rxed_p = rxing_ports[tlocal_tid]->receivePacket();
  	hdr = rxed_p->accessHeader();
  	if(hdr->getOctet(0) == 'd'){ //Hello packet is rxed
  		cout << "Disc packet is rxed from node_id: " << hdr->getOctet(1)
  				 << " over rx_port:" << nbr_tbl.at(tlocal_tid).RxP << endl;
  		//Send Hello Ack
    	try{
	  		disc_txing_ports[tlocal_tid]->sendPacket(create_control_packet('1'));
		  }catch(const char *reason){
  			cerr << "Exception:" << reason << endl;
  		}
  		//disc_txing_ports[tlocal_tid]->sendPacket(create_control_packet('1'));
  	}
  	else if(hdr->getOctet(0) == '1'){ //Hello Ack is rxed
  		cout << "Hello Ack is rxed from node_id: " << hdr->getOctet(1)
  				 << " over rx_port:" << nbr_tbl.at(tlocal_tid).RxP << endl;
  	}
  	else if(hdr->getOctet(0) == 'l'){ //LSP packet is rxed
  		cout << "LSP packet is rxed ..." << endl;
  	}
  	else{
  		cout << "Some packet with unrecognized type is rxed\n";
  	}
  }
}

void Router::open_rx_channels(){
 	if(num_neighbors == 0){
 		std::cout << "Router::open_rx_channels(); num_neighbors is 0\n";
 		exit(1);
 	}
	rxing_ports = new ReceivingPort*[num_neighbors];
	rxing_port_threads = new pthread_t[num_neighbors];
	
	for(int i=0; i<num_neighbors; i++){
		Address* router_port_addr = new Address("localhost", (short)(atoi(nbr_tbl[i].RxP)));
		rxing_ports[i] = new ReceivingPort();
		rxing_ports[i]->setAddress(router_port_addr);
		rxing_ports[i]->init();
		
		int rc = pthread_create(&rxing_port_threads[i], NULL, &Router::bst_listen_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
}

void Router::open_disc_tx_channels(){
	if(num_neighbors == 0){
 		std::cout << "Router::open_disc_tx_channels(); num_neighbors is 0\n";
 		exit(1);
 	}
 	disc_txing_ports = new MySendingPort*[num_neighbors];
	disc_txing_port_threads = new pthread_t[num_neighbors];
	
	for(int i=0; i<num_neighbors; i++){
		Address* router_port_addr = new Address("localhost", (short)(atoi(nbr_tbl[i].TxP)));
		Address* nhop_dst_addr =  new Address("localhost", (short)(atoi(nbr_tbl[i].DstP)));
  	disc_txing_ports[i] = new MySendingPort();
  	disc_txing_ports[i]->setAddress(router_port_addr);
  	disc_txing_ports[i]->setRemoteAddress(nhop_dst_addr);
  	disc_txing_ports[i]->init();

  	int rc = pthread_create(&disc_txing_port_threads[i], NULL, &Router::bst_tx_disc_message_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
	
	//Disc Timer business
 	disc_tx_timer_port = new MySendingPort(this);
	//Initializing disc_timer_port and starting the timer
	//Careful !!! not to use the ports below
	Address * my_tx_addr = new Address((char*)"localhost", (short)(8001+(int)id));
	Address * dst_addr =  new Address((char*)"localhost", (short)(8000+(int)id));
  disc_tx_timer_port->setAddress(my_tx_addr);
  disc_tx_timer_port->setRemoteAddress(dst_addr);
	disc_tx_timer_port->init();

	disc_tx_timer_port->timer_.startTimer(10);
	cout << "Dimer started\n";
	
}

pthread_mutex_t mutex_tdmop = PTHREAD_MUTEX_INITIALIZER;
void* Router::tx_disc_message_on_port(void* thread_id){
	int* tid = (int *)(thread_id);
	pthread_mutex_lock( &mutex_tdmop );
	int tlocal_tid = *tid;
	pthread_mutex_unlock( &mutex_tdmop );
  cout<<endl<<"Sending disc message from tx_port " << nbr_tbl.at(tlocal_tid).TxP
  		<<" to dst_port " << nbr_tbl.at(*tid).DstP << endl;
  //send disc message
  try{
	  disc_txing_ports[tlocal_tid]->sendPacket(create_control_packet('d'));
  }catch(const char *reason){
  	cerr << "Exception:" << reason << endl;
  }
  
}

pthread_mutex_t mutex_bst_tdmop = PTHREAD_MUTEX_INITIALIZER;
int tdmop_thread_index = -1;
void* Router::bst_tx_disc_message_on_port(void *context){
	
	pthread_mutex_lock( &mutex_bst_tdmop );
	++tdmop_thread_index;
	pthread_mutex_unlock( &mutex_bst_tdmop );
	return ((Router*)context)->tx_disc_message_on_port((void *)&tdmop_thread_index);
}

void Router::disc_tx_timer_action(){
	tdmop_thread_index = -1;
	for(int i=0; i<num_neighbors; i++){
  	int rc = pthread_create(&disc_txing_port_threads[i], NULL, &Router::bst_tx_disc_message_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
	disc_tx_timer_port->timer_.startTimer(10);
}

Packet* Router::create_control_packet(char p_type){
	Packet * my_packet = new Packet();
	PacketHdr *hdr = my_packet->accessHeader();
	
	if(p_type == 'd'){ //Packet is discovery type
		//Setting the specific header fields
		hdr->setOctet('d',0);
		hdr->setOctet(id,1);

		return my_packet;
	}
	else if(p_type == '1'){
		//Disc Ack packet type is '1'
		hdr->setOctet('1',0);
		hdr->setOctet(id,1);
		
		return my_packet;
	}
	else if(p_type == 'l'){ //Packet is LSP type
		
		return my_packet;
	}
}

void Router::wait_for_all_threads(){
	for(int j=0; j < num_neighbors; j++)
	{
		pthread_join( rxing_port_threads[j], NULL);
		pthread_join( disc_txing_port_threads[j], NULL);
	}
	
}

int main(int argc, char* argv[]){
	if(argc != 2){
		std::cout << "Wrong argv; <Router ID>\n";
		exit(1);
	}
	
	Router router((char)(*argv[1]), (char*)"sample_network.txt");
	
	//router.wait_for_all_threads();
	
	string input = "";
	//cout << "Please enter a valid sentence (with spaces):\n>";
	getline(cin, input);
	
	return 0;
}

/*
	enum { A, B, C, D, E, F, G, N };
	const int num_nodes = N;
	const char* name = "ABCDEFG";
	std::map<char,int> vname_vindex;
	for(int i=0; i<N; i++){
		vname_vindex.insert (std::pair<char,int>(name[i], i));
	}
	typedef std::pair<int, int> Edge;
	Edge edge_array[] = { Edge(A,B), Edge(B,A), Edge(C,A), Edge(A,C), Edge(E,F), Edge(F,E),
		                    Edge(D,C), Edge(C,D), Edge(C,E), Edge(E,C), Edge(B,D), Edge(D,B),
		                    Edge(B,G), Edge(G,B)};
	const int num_edges = sizeof(edge_array)/sizeof(edge_array[0]);
	int weights[num_edges];
	for(int i=0; i<num_edges; i++){
		weights[i]=1;
	}
	// declare the graph object
	graph_t g(edge_array, edge_array+num_edges, weights, num_nodes);
	
	//Filling up map for <next_hop, port#> (will be done by Discovery protocol)
  std::map <char, int> nhop_port;
  nhop_port.insert (std::pair<char,int>('B', 1));
  nhop_port.insert (std::pair<char,int>('C', 2));
  
  Router router((char)(*argv[1])); //argv[1]: id of the router
  router.simple_exp(0,g, name, vname_vindex, nhop_port);
	
	//Router trials
	char payload [] = "ahmet\0";
	Packet* data_packet = router.create_data_packet('d', '12', 'A', '3', "BGD", strlen(payload), payload);
	router.print_packet(data_packet);
	
	int* f_ports = router.handle_rxed_data_packet(data_packet);
	std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
	delete [] f_ports;
	*/
