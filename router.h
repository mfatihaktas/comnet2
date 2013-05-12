#ifndef __ROUTER_H_INCLUDED__
#define __ROUTER_H_INCLUDED__

#include "common.h"
#include "forwarding_table_filler.h"

using namespace std;

// create a typedef for the Graph type
typedef adjacency_list<vecS, vecS, bidirectionalS, no_property,
                     property<edge_weight_t, int> > graph_t;
typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
//
#define MAX_PORTFIELD_SIZE 10
struct nhopinfo
{
	char id [MAX_PORTFIELD_SIZE];
	char Type [MAX_PORTFIELD_SIZE];
	char TxP [MAX_PORTFIELD_SIZE];
	char RxP [MAX_PORTFIELD_SIZE];
	char DstP [MAX_PORTFIELD_SIZE];
};

struct route
{
	string Dst,NxtHop;
	string Dist;
};

class Router{
	private:
		char id;
		int num_neighbors;
		int num_neighbors_replied_hello;
		bool discovery_done;
		
		vector<nhopinfo> nbr_tbl;
		vector<char> Destinations;
		ifstream* file_ptr;
		ifstream* file2_ptr;
		
		SendingPort** sending_ports;
		ReceivingPort** rxing_ports;
		pthread_t* rxing_port_threads;
		SendingPort** disc_txing_ports;
		pthread_t* disc_txing_port_threads;
		//
		ForwardingTableFiller <graph_t> ftf;
	public:
		Router(){};
		Router(char id, char* nettopo_file_name){
			file_ptr = new ifstream( nettopo_file_name , ifstream::in);
			
			this->id = id;
			std::cout << "Router ID: " << id << std::endl;
			num_neighbors = 0;
			num_neighbors_replied_hello = 0;
			discovery_done = false;
			
			find_numneighbors_init_nbrtbl(nettopo_file_name);
			
			for(int i=0; i<num_neighbors; i++){
				nbr_tbl.push_back(nhopinfo());
			}
			port_info_read_fromtxt("deneme.txt");//nettopo_file_name);
			//std::cout << "nbr_tbl size: " << nbr_tbl.size() << std::endl;
			//Create port arrays for one-hop TX and RX
			open_rx_channels();
			open_disc_tx_channels();
		};
		~Router(){
		/*
			file_ptr->close();
			for(int i=0; i<num_neighbors; i++){
				delete [] sending_ports[i];
			}
			delete [] sending_ports;
			delete [] rxing_port_threads;
		*/
		};
		int* handle_rxed_data_packet(Packet* p);
		void forward_incoming_data_packet(Packet* p);
		
		void simple_exp(int source_index, graph_t& g, const char *name, 
										std::map<char,int>& vname_vindex, std::map<char,int>& nhop_port);
										
		void print_packet(Packet* p);
		Packet*	create_data_packet(char p_type, char ttl, char src_addr, char n_addr, 
															 char* dest_addrs, int payload_size, char* payload);
															 
		//For Discovery and LS purposes
		void port_info_read_fromtxt(char* file_name);
		void find_numneighbors_init_nbrtbl(char* file_name);
		void open_rx_channels();
		void* listen_on_port(void* thread_id);
		static void *bst_listen_on_port(void *context);
    void wait_for_all_threads();
    Packet* create_control_packet(char p_type);
    void* tx_disc_message_on_port(void* thread_id);
    static void* bst_tx_disc_message_on_port(void *context);
    void open_disc_tx_channels();
};




#endif
