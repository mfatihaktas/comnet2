#include "forwarding_table_filler.h"

template <typename Graph> 
struct exercise_vertex {
	exercise_vertex(Graph& g_, const char name_[]) : g(g_),name(name_) { }
	typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
	void operator()(const Vertex& v) const
	{
	  using namespace boost;
	  typename property_map<Graph, vertex_index_t>::type vertex_id = get(vertex_index, g);
	  std::cout << "vertex: " << name[get(vertex_id, v)] << std::endl;

	  // Write out the outgoing edges
	  std::cout << "\tout-edges: ";
	  typename graph_traits<Graph>::out_edge_iterator out_i, out_end;
	  typename graph_traits<Graph>::edge_descriptor e;
	  for (boost::tie(out_i, out_end) = out_edges(v, g); out_i != out_end; ++out_i){
	    e = *out_i;
	    Vertex src = source(e, g), targ = target(e, g);
	    std::cout << "(" << name[get(vertex_id, src)]
	              << "," << name[get(vertex_id, targ)] << ") ";
	  }
	  std::cout << std::endl;

	  // Write out the incoming edges
	  std::cout << "\tin-edges: ";
	  typename graph_traits<Graph>::in_edge_iterator in_i, in_end;
	  for (boost::tie(in_i, in_end) = in_edges(v, g); in_i != in_end; ++in_i){
	    e = *in_i;
	    Vertex src = source(e, g), targ = target(e, g);
	    std::cout << "(" << name[get(vertex_id, src)]
	              << "," << name[get(vertex_id, targ)] << ") ";
	  }
	  std::cout << std::endl;

	  // Write out all adjacent vertices
	  std::cout << "\tadjacent vertices: ";
	  typename graph_traits<Graph>::adjacency_iterator ai, ai_end;
	  for (boost::tie(ai,ai_end) = adjacent_vertices(v, g);  ai != ai_end; ++ai)
	    std::cout << name[get(vertex_id, *ai)] <<  " ";
	  std::cout << std::endl;
	}
	Graph& g;
	const char *name;
};

template <typename Graph>
ForwardingTableFiller<Graph>::ForwardingTableFiller(Graph& g, const char *name){
	this->g=g;
	this->name=name;
}

template <typename Graph>
void ForwardingTableFiller<Graph>::print_graph(){
	/*
	//!= needs to be overwritten
	if(g != NULL){
		std::cout<<"graph is null\n";
		exit(1);
	}
	*/
	std::for_each(vertices(g).first, vertices(g).second, exercise_vertex<Graph>(g, name));
}

/*
template <typename Graph>
void ForwardingTableFiller<Graph>::run_dijkstra(int source_index){}
*/

