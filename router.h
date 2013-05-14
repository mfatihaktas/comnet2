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
#define MAX_NUM_NODES 10

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


class MySendingPort;

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
		
		MySendingPort** sending_ports;
		ReceivingPort** rxing_ports;
		pthread_t* rxing_port_threads;
		MySendingPort** txing_ports;
		pthread_t* txing_port_threads;
		
		MySendingPort* disc_tx_timer_port;
		MySendingPort* ls_tx_timer_port;
		
		std::map <char, int> nhop_hello_ack_rxed;
		int ls_seq_num;
		vector<char> graph_name;
		int num_of_total_nodes;
		std::map <char, char*> netnode_nhops;
		char* final_graph_name;
		std::map <char, int> nhop_port;
		std::map <int, char> port_nhop;
		//
		typedef std::pair<int, int> Edge;
		int num_edges;
		Edge* edge_array;
		//int weights[num_edges];
		char* nhops;
		graph_t* g_p;
		//
		ForwardingTableFiller <graph_t> *ftf_p;
	public:
		Router(){};
		Router(char id, char* nettopo_file_name){
			file_ptr = new ifstream( nettopo_file_name , ifstream::in);
			
			this->id = id;
			std::cout << "Router ID: " << id << std::endl;
			num_neighbors = 0;
			num_neighbors_replied_hello = 0;
			discovery_done = false;
			ls_seq_num = 0;
			
			find_numneighbors_init_nbrtbl(nettopo_file_name);
			
			for(int i=0; i<num_neighbors; i++){
				nbr_tbl.push_back(nhopinfo());
			}
			port_info_read_fromtxt(nettopo_file_name);//nettopo_file_name);
			//std::cout << "nbr_tbl size: " << nbr_tbl.size() << std::endl;
			//Create port arrays for one-hop TX and RX
			open_rx_channels();
			open_disc_tx_channels();
			wait_for_all_nhops_to_be_disced();
			broadcast_ls_packets();
			
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
		int* forward_rxed_data_packet_to_port(Packet* p);
		void forward_incoming_data_packet(Packet* p);
		
		void simple_exp(int source_index, graph_t& g, const char *name, 
										std::map<char,int>& vname_vindex, std::map<char,int>& nhop_port);
										
		void print_data_packet(Packet* p);
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
    void disc_tx_timer_action();
    void wait_for_all_nhops_to_be_disced();
    //Disc is done now time for LSP
    void* tx_ls_packet_on_port(void* thread_id);
    static void* bst_tx_ls_packet_on_port(void *context);
    void broadcast_ls_packets();
    void ls_tx_timer_action();
    void extract_ls_info(Packet* p);
    void send_ls_to_outgoing_ports(char* incoming_TxP, Packet* p);
    void build_map();
    void send_packet_over_port(Packet* p, int port);
    void send_data_packet(Packet* p);
};

class MySendingPort :public SendingPort
{
public:
	MySendingPort():SendingPort(){ }
	MySendingPort(Router* my_router, char timer_for_what):SendingPort(){
		this->my_router = my_router;
		this->timer_for_what = timer_for_what;
	}
	inline void timerHandler()
	{
	 	cout << "\nTimer is fired! ";
	 	if(timer_for_what == 'd'){
		 	cout<< "Resend the Hello Packets !\n"; 
		 	my_router->disc_tx_timer_action();
	 	}
		else if(timer_for_what =='l'){
			cout<< "Resend the LS Packets !\n"; 
		 	my_router->ls_tx_timer_action();
		}
	}
private:
	char timer_for_what;
	Router* my_router;
public:
};
#endif
