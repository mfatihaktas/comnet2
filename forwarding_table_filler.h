#ifndef __FORWARDINGTABLEFILLER_H_INCLUDED__
#define __FORWARDINGTABLEFILLER_H_INCLUDED__

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>                                // for std::for_each


using namespace boost;

template <typename Graph>
class ForwardingTableFiller{
	private:
		Graph g;
		const char *name;
		std::map <char, int> vname_vindex;
		//To keep forwarding table
		std::map <char, int> nhop_port;
		std::map <char, int> dest_port;
		int num_neighbors;
		std::map < char, std::map<char, int> > nhop_dest_dist;
	public:
		ForwardingTableFiller(int source_index, Graph& g, const char *name,
													std::map<char,int>& vname_vindex,	std::map<char,int>& nhop_port);
		void print_graph();
		//void run_dijkstra(int source_index);
		typedef typename graph_traits < Graph >::vertex_descriptor vertex_descriptor;
		typedef typename graph_traits < Graph >::edge_descriptor edge_descriptor;
		typedef typename std::vector<vertex_descriptor>::iterator vertex_iterator;
		
		void run_dijkstra(int source_index){
		  std::cout << "Dikstra is running\n";
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
		  //Printing the Shortest-Path formation between (start)-(end)
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
			//Filling up the tables
			//HashMap hash_map();
			
			//std::cout << "Next-Hop info\n";
			//std::cout << "from source: " << name[s] << std::endl;
			for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
		  	vertex_descriptor current=*vi;
		  	//std::cout << "next-hop to " << name[*vi];
		  	
				vertex_descriptor temp = current;
				while(current!=s) {
	    		temp = current;
  		 		current=p[current];
			  }
			  //Add the next hop to forwarding table
			  dest_port.insert (std::pair<char,int>(name[*vi], nhop_port[name[temp]]));
			  //std::cout << " is " << name[temp] << std::endl;
			}
			//Print dest_nhop to see if it is correctly filled up
			std::cout << "dest_port includes:\n";
			std::map<char,int>::iterator it;
			for (it=dest_port.begin(); it!=dest_port.end(); ++it)
  		  std::cout << it->first << " => " << it->second << "\n";
  		  
  		//Creating nhop_dest_dist table (this will be used to implement MC logic)
  		p.clear();
  		d.clear();
  		
			std::cout << "-----nhop_port------\n";
  		for (it=nhop_port.begin(); it!=nhop_port.end(); ++it)
  			std::cout << it->first << " => " << it->second << "\n";
  			
  		for (it=nhop_port.begin(); it!=nhop_port.end(); ++it){
  			s = vertex(vname_vindex.find(it->first)->second, g);
  			dijkstra_shortest_paths(g, s, predecessor_map(&p[0]).distance_map(&d[0]));
  			//nhop_dest_dist.insert (std::pair<char, std::map<char, int> >(it->first, std::map<char, int>));  			
  			//std::map <char, int> ha;
  			//nhop_dest_dist.insert (std::pair<char, std::map<char, int> >(it->first, new std::map <char, int>));
  			for (boost::tie(vi, vend) = vertices(g); vi != vend; ++vi) {
					nhop_dest_dist[it->first][name[*vi]] = d[*vi];
				}
  		}
  		//Check if the tables for nhops are created correctly
  		std::map< char, std::map<char, int> >::iterator it_ndd;
  		for (it_ndd=nhop_dest_dist.begin(); it_ndd!=nhop_dest_dist.end(); ++it_ndd){
  			std::cout << "Forwarding Table for nhop: " << it_ndd->first << std::endl;
  			for( it=(it_ndd->second).begin(); it != (it_ndd->second).end(); it++)
	      	std::cout << it->first << " => " << it->second << std::endl;
  		}
  		
		}
};

#include "forwarding_table_filler.tpp"

#endif
