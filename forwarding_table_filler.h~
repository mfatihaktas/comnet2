#ifndef __FORWARDINGTABLEFILLER_H_INCLUDED__
#define __FORWARDINGTABLEFILLER_H_INCLUDED__

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>                                // for std::for_each

using namespace boost;

template <typename Graph>
class ForwardingTableFiller{
	public:
		ForwardingTableFiller(Graph& g, const char *name);
		void print_graph();
		//void run_dijkstra(int source_index);
		typedef typename graph_traits < Graph >::vertex_descriptor vertex_descriptor;
		typedef typename graph_traits < Graph >::edge_descriptor edge_descriptor;
		typedef typename std::vector<vertex_descriptor>::iterator vertex_iterator;
		
		void run_dijkstra(int source_index){
			std::vector<vertex_descriptor> p(num_vertices(g));
			std::vector<int> d(num_vertices(g));

			vertex_descriptor s = vertex(source_index, g);
			dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));
			//Print the shortest-path distances
			std::cout << "shortest path distances from host:" << name[s] << std::endl;
			std::cout << "distances and parents:" << std::endl;
			typename graph_traits < Graph >::vertex_iterator vi, vend;
			for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
				std::cout << "distance(" << name[*vi] << ") = " << d[*vi] << ", ";
				std::cout << "parent(" << name[*vi] << ") = " << name[p[*vi]] << "\n";
			}
			std::cout << std::endl;
			//
		  //Shortest-Path formation between (start)-(end)
		  std::vector<vertex_descriptor> path;
		  
		  for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
		  	vertex_descriptor current=*vi;
		  	std::cout << "path in reverse from " << name[s] << " to " << name[*vi] << std::endl;
		  	path.clear();
				while(current!=s) {
	    		path.push_back(current);
  		 		current=p[current];
			  }
				path.push_back(s);
				//This prints the path reversed use reverse_iterator and rbegin/rend
				vertex_iterator it;
				for (it=path.begin(); it != path.end(); ++it) {
					std::cout << name[*it] << " ";
				}
				std::cout << std::endl;
			}
			
		}
	private:
		Graph g;
		const char *name;
};

#include "forwarding_table_filler.tpp"

#endif
