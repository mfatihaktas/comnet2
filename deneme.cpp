#include <iostream> 
#include <utility>                   // for std::pair
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "forwarding_table_filler.h"
#include "router.h"

using namespace boost;

int main(){

	// create a typedef for the Graph type
	typedef adjacency_list<vecS, vecS, bidirectionalS, no_property,
	                       property<edge_weight_t, int> > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
  typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
	// Make convenient labels for the vertices
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
	// declare a graph object
	graph_t g(edge_array, edge_array+num_edges, weights, num_nodes);
	//
	//Filling up map for <next_hop, port#> (will be done by Discovery protocol)
  std::map <char, int> nhop_port;
  nhop_port.insert (std::pair<char,int>('B', 1));
  nhop_port.insert (std::pair<char,int>('C', 2));
  
  //
  ForwardingTableFiller <graph_t> ftf (0,g, name, vname_vindex, nhop_port);
	int f_port = ftf.forward_to_port(1, "D", NULL);
	std::cout << "unicast to \"D\" forwarding_port: " << f_port << std::endl;
	int* f_ports = new int[2];
	f_port = ftf.forward_to_port(2, "GE", &f_ports);
	std::cout << "f_ports: " << f_ports[ 0] << " - "	<< f_ports[1] << " - " << f_ports[2] << std::endl;
	delete f_ports;
	
	//Router trials
	Router my_r;
	char payload [] = "ahmet\0";
	Packet* data_packet = my_r.create_data_packet('d', '12', 'A', '3', "GEF", strlen(payload), payload);
	my_r.print_packet(data_packet);
	
	return 0;
}

