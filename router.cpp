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

void Router::print_data_packet(Packet* p){
	//First print the header
	PacketHdr *hdr = p->accessHeader();
	
	std::cout << "PRINT PACKET with Header size: " <<hdr->getSize() << "\n";
	std::cout << "packet_type: " <<hdr->getOctet(PACKET_TYPE_INDEX) << "\n";
	std::cout << "ttl: " << hdr->getOctet(TTL_INDEX) << "\n";
	std::cout << "src_addr: " << hdr->getOctet(SRC_ADDR_INDEX) << "\n";
	char num_addr = hdr->getOctet(NUM_ADDR_INDEX);
	std::cout << "num_addr: " << num_addr << "\n";
	
	int int_num_addr = (int)(num_addr-'0');
	char dest_addrs_buffer[int_num_addr+1];
	for(int i=0; i < int_num_addr; i++){
		dest_addrs_buffer[i]=hdr->getOctet(NUM_ADDR_INDEX+i+1);
	}
	dest_addrs_buffer[int_num_addr]='\0';
	std::cout << "dst_addrs: " << dest_addrs_buffer << "\n";
	//Second print the payload
	//std::cout << "payload_size: " << hdr->getOctet(PAYLOAD_SIZE_INDEX(int_num_addr)) << "\n";
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
	int int_num_addr = (int)(num_addr-'0');
	for(int i=0; i<int_num_addr; i++){
		hdr->setOctet(*(dest_addrs+i), NUM_ADDR_INDEX+i+1);
	}
	hdr->setOctet(payload_size,PAYLOAD_SIZE_INDEX(int_num_addr));
	//Fill the packet with the data payload
	my_dpacket->fillPayload(payload_size, payload);
	
	return my_dpacket;
}

void Router::send_data_packet(Packet* p){
	//std::cout << "The following packet ll be sent:" << std::endl;
	//print_data_packet(p);
			
	PacketHdr *hdr = p->accessHeader();
	char num_addr = hdr->getOctet(NUM_ADDR_INDEX);
	int int_num_addr = (int)(num_addr-'0');
	
	
	if(int_num_addr == 1 && hdr->getOctet(NUM_ADDR_INDEX+1) == id){ //if the packet is uni and targeted to me
		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		std::cout << "DATA PACKET TARGETED TO ME IS RXED: \n";
		print_data_packet(p);
		std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
		return;
	}
	int* f_ports = forward_rxed_data_packet_to_port(p);
	//Use the "goodness" of at most 3 MC receivers
	std::vector <int> ports_done;
	std::vector <char> temp_dests;
	
	for(int i=0; i<int_num_addr; i++){
		int curr_port = f_ports[i];
		temp_dests.clear();
		if(std::find(ports_done.begin(), ports_done.end(), curr_port) == ports_done.end()){ //not contained
			temp_dests.push_back(hdr->getOctet(NUM_ADDR_INDEX+i+1));
			for(int j=i+1; j<int_num_addr; j++){
				if(curr_port == f_ports[j])
					temp_dests.push_back(hdr->getOctet(NUM_ADDR_INDEX+j+1));
			}
			char* dest_addrs = new char[temp_dests.size()+1];
			for(int k=0; k<temp_dests.size(); k++){
				dest_addrs[k] = temp_dests.at(k);
			}
			dest_addrs[temp_dests.size()] = (char)'\0';
			//hdr->getOctet(PAYLOAD_SIZE_INDEX(temp_dests.size()))
			int payload_size = strlen(p->getPayload());
			char* payload = new char[payload_size];
			strcpy(payload, p->getPayload());
			Packet* new_p = create_data_packet('m', (char)((int)(hdr->getOctet(TTL_INDEX)-'0')-1), 
																			hdr->getOctet(SRC_ADDR_INDEX),
																			'0'+temp_dests.size(), dest_addrs,
																			payload_size,	payload);
			send_packet_over_port(new_p, curr_port);
			//std::cout << "The following packet ll be sent over port:" << curr_port << std::endl;
			//print_data_packet(p);
			//
			ports_done.push_back(curr_port);
		}
	}
	
}

void Router::forward_incoming_data_packet(Packet* p){
	if( p == NULL){ //check for null packet
		std::cout << "Rxed packet is NULL !\n";
		exit(1);
	}
	std::cout << "Rxed data packet which ll be forwarded is:\n";
	print_data_packet(p);
	//Correct packets will be sent to correct ports
	send_data_packet(p);
}

void Router::send_packet_over_port(Packet* p, int port){
	
	char correspoding_nhop = port_nhop.find(port)->second;
	for(int i=0; i<num_neighbors; i++){
		if(nbr_tbl.at(i).id[0] == correspoding_nhop){ //corresponding port index -> i
			//send disc message
			try{
				std::cout << "==================================\n";
				std::cout << "The following packet is being sent over TxP:" << nbr_tbl.at(i).TxP
								  << "_DstP:" << nbr_tbl.at(i).DstP << std::endl;
				print_data_packet(p);
				std::cout << "==================================\n";
				txing_ports[i]->sendPacket(p);
			}catch(const char *reason){
				cerr << "Exception:" << reason << endl;
			}
		}
	}
}


int* Router::forward_rxed_data_packet_to_port(Packet* p){
	PacketHdr *hdr = p->accessHeader();
	char p_type = hdr->getOctet(PACKET_TYPE_INDEX);
	if(p_type != 'm'){ //check for packet type
		std::cout << "This function is supposed to be given DATA packet. This has another type !\n";
		exit(1);
	}
	//Extract necessary info for giving the forwarding decision
	char num_addr = hdr->getOctet(NUM_ADDR_INDEX);
	int int_num_addr = (int)(num_addr-'0');
	int* d_p = new int[int_num_addr];
	char dest_addrs_buffer[int_num_addr+1];
	for(int i=0; i < num_addr; i++){
		dest_addrs_buffer[i]=hdr->getOctet(NUM_ADDR_INDEX+i+1);
	}
	dest_addrs_buffer[num_addr] = (char)'/0';
	/*
	std::cout << "=============================================\n";
	std::cout << "int_num_addr: " << int_num_addr << " dest_addrs_buffer: " 
					  << dest_addrs_buffer << std::endl;
	std::cout << "=============================================\n";
	*/
	if(int_num_addr == 1){//unicast packet
		*d_p = ftf_p->forward_to_port(int_num_addr, (char*)dest_addrs_buffer, &d_p);
	}
	else{ //MC packet
		ftf_p->forward_to_port(int_num_addr, (char*)dest_addrs_buffer, &d_p);
	}	
	return d_p;
}

void Router::build_map(){
	//Add the this router itself to graph_name and netnode_nhops and nhop_port
	graph_name.push_back(id);
	
	nhops = new char[nbr_tbl.size()];
	for(int i=0; i<nbr_tbl.size(); i++){
		nhops[i] = nbr_tbl.at(i).id[0];
		nhop_port.insert (std::pair<char,int>(nbr_tbl.at(i).id[0], i+1));
		port_nhop.insert (std::pair<int,char>(i+1, nbr_tbl.at(i).id[0] ));
	}
	netnode_nhops.insert (std::pair<char,char*>(id, nhops));
	
	std::map<char,int>::iterator it1;
	std::cout << "nhop - port\n";
	for (it1=nhop_port.begin(); it1!=nhop_port.end(); ++it1){
		std::cout << it1->first << "->" << it1->second << std::endl;
	}
	//put graph_name into char[]
	final_graph_name = new char[++num_of_total_nodes];
	for(int i=0; i<num_of_total_nodes; i++){
		final_graph_name[i] = graph_name.at(i);
	}

	std::map<char,int> vname_vindex;
	for(int i=0; i<strlen(final_graph_name); i++){
		vname_vindex.insert (std::pair<char,int>(final_graph_name[i], i));
	}
	//calculate the # of edges in the network
	num_edges = 0;
	std::map<char,char*>::iterator it;
	std::cout << "overall network topo:\n";
	for (it=netnode_nhops.begin(); it!=netnode_nhops.end(); ++it){
		std::cout << it->first << " -> " << it->second << std::endl;
		num_edges += strlen(it->second);
	}
	std::cout << "network num_edges: " << num_edges << std::endl;

	edge_array = new Edge[num_edges];
	int counter = 0;
	for (it=netnode_nhops.begin(); it!=netnode_nhops.end(); ++it){
		char* temp_array = it->second;
		for(int i=0; i<strlen(temp_array); i++){
			edge_array[counter] = Edge(vname_vindex.find(it->first)->second, 
																 vname_vindex.find(temp_array[i])->second);
			++counter;
		}
	}
	int weights[num_edges];
	for(int i=0; i<num_edges; i++){
		weights[i]=1;
	}
	/*
	std::map <char, int> nhop_port;
  nhop_port.insert (std::pair<char,int>('B', 1));
  nhop_port.insert (std::pair<char,int>('D', 2));
  */
	// declare the graph object
	//graph_t g(edge_array, edge_array+num_edges, weights, strlen(final_graph_name));
	g_p = new graph_t(edge_array, edge_array+num_edges, weights, strlen(final_graph_name));
	
	ftf_p = new ForwardingTableFiller <graph_t> ();
	ftf_p->set_all(vname_vindex.find(id)->second,*g_p, final_graph_name, vname_vindex, nhop_port);
  ftf_p->do_initial_job();
  //Everything is ready to rock
  
  //Some experiments
  /*
  if(id == 'W'){
  	int f_port = ftf_p->forward_to_port(1, "B", NULL);
		std::cout << "unicast to \"B\" forwarding_port: " << f_port << std::endl;
		int* f_ports = new int[3];
		f_port = ftf_p->forward_to_port(2, "BD", &f_ports);
		std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
		delete f_ports;
  }
  */
  Packet* trial_mc_pkt = create_data_packet('m', '9', id, '2',
														 			 					(char*)"XY", 6, (char*)"deneme");
	Packet* trial_u_pkt = create_data_packet('m', '9', id, '1', 
														 			 					(char*)"X", 6, (char*)"deneme");
	//std::cout << "///////////////////////////////////\n";
	//print_data_packet(trial_u_pkt);
	if(id == 'W')
  	send_data_packet(trial_mc_pkt);
}

void Router::simple_exp(int source_index, graph_t& g, const char *name, 
												std::map<char,int>& vname_vindex, std::map<char,int>& nhop_port){
	ftf_p->set_all(0,g, name, vname_vindex, nhop_port);
  ftf_p->do_initial_job();
  
  //Some experiments
  int f_port = ftf_p->forward_to_port(1, "D", NULL);
	std::cout << "unicast to \"D\" forwarding_port: " << f_port << std::endl;
	int* f_ports = new int[3];
	f_port = ftf_p->forward_to_port(3, "GEF", &f_ports);
	std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
	delete f_ports;
}

#define MAX_LINE_SIZE 50
void Router::find_numneighbors_init_nbrtbl(char* file_name){
	std::vector<char> net_nodes;
	
	char line [MAX_LINE_SIZE];
	if(file_ptr->is_open()){
		while(file_ptr->good())
		{
			file_ptr->getline(line,MAX_LINE_SIZE);
		  if(line[0] == id)
		  {
				++num_neighbors;
		  }
		  if(std::find(net_nodes.begin(), net_nodes.end(), line[0]) == net_nodes.end()
		  	 && line[0] != (char)NULL && line[0] != id){ //not contained
				net_nodes.push_back(line[0]);
				//std::cout << line[0] << " is added to net_nodes\n";
			}
		}
		file_ptr->clear(); //clear the failure flag
		file_ptr->seekg(0, file_ptr->beg);
		file_ptr->close();
	}
	num_of_total_nodes = net_nodes.size();
	std::cout << "num_of_total_nodes: " << num_of_total_nodes << std::endl;
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
    nhop_hello_ack_rxed.insert (std::pair<char,int>(nbr_tbl[i].id[0], 0));		 
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
	  		txing_ports[tlocal_tid]->sendPacket(create_control_packet('1'));
		  }catch(const char *reason){
  			cerr << "Exception:" << reason << endl;
  		}
  	}
  	else if(hdr->getOctet(0) == '1'){ //Hello Ack is rxed
  		cout << "Hello Ack is rxed from node_id: " << hdr->getOctet(1)
  				 << " over rx_port:" << nbr_tbl.at(tlocal_tid).RxP << endl;
  		//Set the nhop_hello_ack_rxed map correctly to indicate the ACK is rxed from the nhop
  		nhop_hello_ack_rxed.find(hdr->getOctet(1))->second = 1;
  		/*
  		//print nhop_hello_ack_rxed
  		std::map<char,int>::iterator it;
  		std::cout << "nhop_hello_ack_rxed:\n";
  		for (it=nhop_hello_ack_rxed.begin(); it!=nhop_hello_ack_rxed.end(); ++it){
			if(it->second == 0){
					std::cout << it->first << " -> " << it->second << std::endl;
				}
			}
			*/
  	}
  	else if(hdr->getOctet(0) == 'l'){ //LSP packet is rxed
  		cout << "LSP packet is rxed from node_id: " << hdr->getOctet(2)
  				 << " over rx_port:" << nbr_tbl.at(tlocal_tid).RxP << endl;
  		//Extract the connectivity info
  		extract_ls_info(rxed_p);
  		//Send it to outgoing ports
  		int ttl = (int)(hdr->getOctet(1)-'0');
  		
  		//std::cout << "----------------------------------------\n";
  		//std::cout << "LS TTL: " << hdr->getOctet(1) << std::endl;
  		if( ttl != 0){ //check ttl if it is 0 then discard:dont forward further
  			--ttl;
  			hdr->setOctet( (char)('0'+ttl),1);
  			//std::cout << "UPDATED LS TTL: " << hdr->getOctet(1) << std::endl;	
	  		send_ls_to_outgoing_ports(nbr_tbl.at(tlocal_tid).TxP, rxed_p);
	  	}
	  	//std::cout << "----------------------------------------\n";
	  	
		}
		else if(hdr->getOctet(0) == 'm'){ //Data packet is rxed
			cout << "\nDATA packet is rxed from node_id: " << hdr->getOctet(2)
  				 << " over rx_port:" << nbr_tbl.at(tlocal_tid).RxP << endl;
			forward_incoming_data_packet(rxed_p);
		}
  	else{
  		cout << "Some packet with unrecognized type is rxed\n";
  	}
  }
}

void Router::extract_ls_info(Packet* lsp){
	PacketHdr *hdr = lsp->accessHeader();
	char id = hdr->getOctet(2);
	if(std::find(graph_name.begin(), graph_name.end(), id) != graph_name.end()){
		std::cout << "Connectivity info of node_id: " << id << " is already in graph_name\n";
		std::cout << "here comes the graph_name: ";
		for( std::vector<char>::const_iterator i = graph_name.begin(); i != graph_name.end(); ++i)
  	  std::cout << *i << '-';
		std::cout << std::endl;
		return;
	}
	graph_name.push_back(id);
	
	std::cout << "here comes the graph_name: ";
	for( std::vector<char>::const_iterator i = graph_name.begin(); i != graph_name.end(); ++i)
    std::cout << *i << '-';
	std::cout << std::endl;
	
	//Extract the connectivity info
	/*
	//Check the seq_num
	hdr->getOctet(1);
	*/
	int num_nhops = hdr->getOctet(3);
	char* nhops = new char[num_nhops+1];
	for(int i=0; i<num_nhops; i++){
		nhops[i] = hdr->getOctet(3+i+1);
	}
	nhops[num_nhops] = (char)'\0';
	netnode_nhops.insert (std::pair<char,char*>(id, nhops));
	
	
	if(graph_name.size() == num_of_total_nodes){
		std::cout << "CONNECTIVITY INFO FROM EACH NODE IS EXTRACTED YAY !\n";
		build_map();
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
 	txing_ports = new MySendingPort*[num_neighbors];
	txing_port_threads = new pthread_t[num_neighbors];
	
	for(int i=0; i<num_neighbors; i++){
		Address* router_port_addr = new Address("localhost", (short)(atoi(nbr_tbl[i].TxP)));
		Address* nhop_dst_addr =  new Address("localhost", (short)(atoi(nbr_tbl[i].DstP)));
  	txing_ports[i] = new MySendingPort();
  	txing_ports[i]->setAddress(router_port_addr);
  	txing_ports[i]->setRemoteAddress(nhop_dst_addr);
  	txing_ports[i]->init();

  	int rc = pthread_create(&txing_port_threads[i], NULL, &Router::bst_tx_disc_message_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
	
	//Disc Timer business
 	disc_tx_timer_port = new MySendingPort(this, 'd');
	//Initializing disc_timer_port and starting the timer
	//Careful !!! not to use the ports below
	Address * my_tx_addr = new Address((char*)"localhost", (short)(8001+(int)id));
	Address * dst_addr =  new Address((char*)"localhost", (short)(8000+(int)id));
  disc_tx_timer_port->setAddress(my_tx_addr);
  disc_tx_timer_port->setRemoteAddress(dst_addr);
	disc_tx_timer_port->init();

	disc_tx_timer_port->timer_.startTimer(10);
	cout << "Timer for disc messages started\n";
	
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
	  txing_ports[tlocal_tid]->sendPacket(create_control_packet('d'));
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

pthread_mutex_t mutex_tlpop = PTHREAD_MUTEX_INITIALIZER;
void* Router::tx_ls_packet_on_port(void* thread_id){
	int* tid = (int *)(thread_id);
	pthread_mutex_lock( &mutex_tlpop );
	int tlocal_tid = *tid;
	pthread_mutex_unlock( &mutex_tlpop );
  cout<<endl<<"Sending ls packet from tx_port " << nbr_tbl.at(tlocal_tid).TxP
  		<<" to dst_port " << nbr_tbl.at(*tid).DstP << endl;
  //send ls packet
  try{
	  txing_ports[tlocal_tid]->sendPacket(create_control_packet('l'));
  }catch(const char *reason){
  	cerr << "Exception:" << reason << endl;
  }
}

pthread_mutex_t mutex_bst_tlpop = PTHREAD_MUTEX_INITIALIZER;
int tlpop_thread_index = -1;
void* Router::bst_tx_ls_packet_on_port(void *context){
	pthread_mutex_lock( &mutex_bst_tlpop );
	++tlpop_thread_index;
	pthread_mutex_unlock( &mutex_bst_tlpop );
	return ((Router*)context)->tx_ls_packet_on_port((void *)&tlpop_thread_index);
}

void Router::broadcast_ls_packets(){
	for(int i=0; i<num_neighbors; i++){
  	int rc = pthread_create(&txing_port_threads[i], NULL, &Router::bst_tx_ls_packet_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
	//LS Timer business
 	ls_tx_timer_port = new MySendingPort(this, 'l');
	//Initializing disc_timer_port and starting the timer
	//Careful !!! not to use the ports below
	Address * my_tx_addr = new Address((char*)"localhost", (short)(9001+(int)id));
	Address * dst_addr =  new Address((char*)"localhost", (short)(9000+(int)id));
  ls_tx_timer_port->setAddress(my_tx_addr);
  ls_tx_timer_port->setRemoteAddress(dst_addr);
	ls_tx_timer_port->init();

	ls_tx_timer_port->timer_.startTimer(13);
	cout << "Timer for LS messages started\n";
}

void Router::send_ls_to_outgoing_ports(char* incoming_TxP, Packet* p){
	PacketHdr *hdr = p->accessHeader();
	std::cout << "LS packed rxed from node_id" << hdr->getOctet(2) << " is sent to outgoing ports" 
						<< " with TTL:" << hdr->getOctet(1) << std::endl;
	int counter = 0;
	for(int i=0; i<num_neighbors; i++){
		if(strcmp(nbr_tbl.at(i).TxP, incoming_TxP) != 0){ //Do not send it to incoming port
			try{
				++counter;
				txing_ports[i]->sendPacket(p);
			}catch(const char *reason){
				cerr << "Exception:" << reason << endl;
			}
		}
		std::cout << "Incoming LS packet is forwarded to " << counter
							<< " outgoin ports\n";
	}
}

void Router::ls_tx_timer_action(){
	tlpop_thread_index = -1;
	for(int i=0; i<num_neighbors; i++){
  	int rc = pthread_create(&txing_port_threads[i], NULL, &Router::bst_tx_ls_packet_on_port, (void *)(this));
    if(rc)
    {
      cout<<endl<<"ERROR: return code from pthread_create() is "<<rc;
			exit(-1);
    }
	}
	ls_tx_timer_port->timer_.startTimer(13);
}

void Router::disc_tx_timer_action(){
	tdmop_thread_index = -1;
	for(int i=0; i<num_neighbors; i++){
  	int rc = pthread_create(&txing_port_threads[i], NULL, &Router::bst_tx_disc_message_on_port, (void *)(this));
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
		hdr->setOctet('l',0);
		hdr->setOctet('8',1);//TTL
		hdr->setOctet(id,2); //src_addr = id
		hdr->setOctet(num_neighbors,3);
		for(int i=0; i<num_neighbors; i++){
			hdr->setOctet(nbr_tbl.at(i).id[0], 3+i+1);
		}
		++ls_seq_num; //inc for the next ls packet formation
		
		return my_packet;
	}
}

void Router::wait_for_all_threads(){
	for(int j=0; j < num_neighbors; j++)
	{
		pthread_join( rxing_port_threads[j], NULL);
		pthread_join( txing_port_threads[j], NULL);
	}
	
}

void Router::wait_for_all_nhops_to_be_disced(){
	std::map<char,int>::iterator it;
	bool flag = true;
	while(flag){
		flag = false;
		for (it=nhop_hello_ack_rxed.begin(); it!=nhop_hello_ack_rxed.end(); ++it){
			if(it->second == 0){
				flag = true;
				break;
			}
		}
	}
	discovery_done = true;
	std::cout << "ALL NHOPES ARE DISCOVERED YAY !\n";
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
	router.print_data_packet(data_packet);
	
	int* f_ports = router.forward_rxed_data_packet_to_port(data_packet);
	std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
	delete [] f_ports;
	*/
