#include <iostream> 
#include <utility>                   // for std::pair
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include "forwarding_table_filler.h"

using namespace boost;

int main(){

	// create a typedef for the Graph type
	typedef adjacency_list<vecS, vecS, bidirectionalS, no_property,
	                       property<edge_weight_t, int> > graph_t;
	typedef graph_traits < graph_t >::vertex_descriptor vertex_descriptor;
  typedef graph_traits < graph_t >::edge_descriptor edge_descriptor;
	// Make convenient labels for the vertices
	enum { A, B, C, D, E, F, N };
	const int num_nodes = N;
	const char* name = "ABCDEF";
	// writing out the edges in the graph
	typedef std::pair<int, int> Edge;
	Edge edge_array[] = { Edge(A,B), Edge(B,A), Edge(C,A), Edge(A,C), Edge(E,F), Edge(F,E),
		                    Edge(D,C), Edge(C,D), Edge(C,E), Edge(E,C), Edge(B,D), Edge(D,B) };
	const int num_edges = sizeof(edge_array)/sizeof(edge_array[0]);
	int weights[num_edges];
	for(int i=0; i<num_edges; i++){
		weights[i]=1;
	}
	// declare a graph object
	graph_t g(edge_array, edge_array+num_edges, weights, num_nodes);
	//
  ForwardingTableFiller <graph_t> ftf (g, name);
	
	ftf.print_graph();
  std::cout<<"--------------------------------\n";
  ftf.run_dijkstra(0);
	return 0;
}

