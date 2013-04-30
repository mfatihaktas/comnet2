#include <iostream>                  // for std::cout
#include <utility>                   // for std::pair
#include <algorithm>                 // for std::for_each
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace boost;

template <class Graph> 
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

int main(int,char*[])
{
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
  /* add the edges to the graph object
	for (int i = 0; i < num_edges; ++i)
		add_edge(edge_array[i].first, edge_array[i].second, g);
	*/
	// declare a graph object
	graph_t g(edge_array, edge_array+num_edges, weights, num_nodes);
  property_map<graph_t, edge_weight_t>::type weightmap = get(edge_weight, g);
  
  std::vector<vertex_descriptor> p(num_vertices(g));
  std::vector<int> d(num_vertices(g));
  vertex_descriptor s = vertex(A, g);
		
  // invoke variant 2 of Dijkstra's algorithm
  //dijkstra_shortest_paths(g, s, distance_map(&d[0]));
	dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));
	
	std::cout << "distances and parents:" << std::endl;
  graph_traits < graph_t >::vertex_iterator vi, vend;
  for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
    std::cout << "distance(" << name[*vi] << ") = " << d[*vi] << ", ";
    std::cout << "parent(" << name[*vi] << ") = " << name[p[*vi]] << "\n";
  }
  std::cout << std::endl;
  //Shortest-Path formation between (start)-(end)
  std::vector<vertex_descriptor> path;
	vertex_descriptor dest = vertex(F, g);
	vertex_descriptor start = vertex(A, g);
	vertex_descriptor current=dest;
	
	while(current!=start) {
    path.push_back(current);
    current=p[current];
	}
	path.push_back(start);
	//This prints the path reversed use reverse_iterator and rbegin/rend
	std::vector< vertex_descriptor >::iterator it;
	for (it=path.begin(); it != path.end(); ++it) {
		std::cout << name[*it] << " ";
	}
	std::cout << std::endl;
	
	//Trying to use exercise_vertex
	std::cout<<"--------------------------------------------\n";
	std::for_each(vertices(g).first, vertices(g).second, exercise_vertex<graph_t>(g, name));
	
	return 0;
}
