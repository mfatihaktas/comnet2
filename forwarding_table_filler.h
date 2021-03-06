#ifndef __FORWARDINGTABLEFILLER_H_INCLUDED__
#define __FORWARDINGTABLEFILLER_H_INCLUDED__

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <algorithm>                                // for std::for_each
#include <limits> 																	//for inf

using namespace boost;

template <typename Graph>
class ForwardingTableFiller{
	private:
		int source_index;
		Graph g;
		const char *name;
		std::map <char, int> vname_vindex;
		//To keep forwarding table
		std::map <char, int> nhop_port;
		std::map <char, int> dest_dist;
		std::map <char, int> dest_port;
		int num_neighbors;
		std::map < char, std::map <char, int> > nhop_dest_dist;
		std::map < char, std::map <int, char> > dest_poss_nhop;

		typedef typename graph_traits < Graph >::vertex_descriptor vertex_descriptor;
		typedef typename graph_traits < Graph >::edge_descriptor edge_descriptor;
		typedef typename std::vector<vertex_descriptor>::iterator vertex_iterator;
	public:
		ForwardingTableFiller() {};
		ForwardingTableFiller(int source_index, Graph& g, const char *name,
													std::map<char,int>& vname_vindex,	std::map<char,int>& nhop_port);
		void print_graph();
		void set_all(int source_index, Graph& g, const char *name, 
								 std::map<char,int>& vname_vindex, std::map<char,int>& nhop_port);
		void do_initial_job();
		
		
		int forward_to_port(int num_dests, const char* dests, int** d_p){
			if(num_dests == 1){ //Unicast packet
				return dest_port.find(dests[0])->second;
			}
			else{ //Multicast packet
				//Find MC combinations and eliminate the nonsense ones
				std::map<char,int>::iterator it;
				char current_dest;
				int counter;
				for(int i=0; i<num_dests; i++){
					counter = 0;
					current_dest = dests[i];
					
					for (it=nhop_port.begin(); it!=nhop_port.end(); ++it){
						if(dest_dist.find(current_dest)->second >= 
							(nhop_dest_dist.find(it->first)->second).find(current_dest)->second){
							dest_poss_nhop[current_dest][counter] = it->first;
							++counter;
						}
					}
				}
			  
				//Print possible forwarding ports for every destination
				std::cout << "Possible next-hops for given dests: " << dests << std::endl;
				std::map<int,char>::iterator it2;
				std::map<char, std::map <int, char> >::iterator it_dp;
				for (it_dp=dest_poss_nhop.begin(); it_dp!=dest_poss_nhop.end(); ++it_dp){
					std::cout << "for Dest: " << it_dp->first << std::endl;
					for (it2 = (it_dp->second).begin(); it2 != (it_dp->second).end(); ++it2){
						std::cout << it2->first << " - next-hop:" << it2->second << std::endl;
					}
					
				}
				// Compare the combinations and decide which one gives the least cost
					//Create all possible combinations by using the <index, ... >
				int dest_comb [num_dests];
				double min_cost = std::numeric_limits<double>::infinity();
				int min_indexes[num_dests];
				double cur_cost = 0;
				if(num_dests == 2){
					for(int i=0; i<(dest_poss_nhop.find(dests[0])->second).size(); i++){
						for(int j=0; j<(dest_poss_nhop.find(dests[1])->second).size(); j++){
							int index [] = {i, j};
							cur_cost = cost(num_dests, dests, index);
							if(min_cost > cur_cost){
								min_cost = cur_cost;
								min_indexes[0] = index[0];
								min_indexes[1] = index[1];
							}
							std::cout << "Combination dests[0]: " << dests[0] << " to nhop: " << 
												(dest_poss_nhop.find(dests[0])->second).find(i)->second << std::endl;
							std::cout << "            dests[1]: " << dests[1] << " to nhop: " << 
												(dest_poss_nhop.find(dests[1])->second).find(j)->second << std::endl;
							std::cout << "cost: "	<< cur_cost << std::endl;
						}
					}
					//Setting returns
					(*d_p)[0] = nhop_port.find((dest_poss_nhop.find(dests[0])->second).find(min_indexes[0])->second)->second;
					(*d_p)[1] = nhop_port.find((dest_poss_nhop.find(dests[1])->second).find(min_indexes[1])->second)->second;

					return -1;
				}
				else if(num_dests == 3){
					for(int i=0; i<(dest_poss_nhop.find(dests[0])->second).size(); i++){
						for(int j=0; j<(dest_poss_nhop.find(dests[1])->second).size(); j++){
							for(int k=0; k<(dest_poss_nhop.find(dests[2])->second).size(); k++){
								int index [] = {i, j, k};
								cur_cost = cost(num_dests, dests, index);
								if(min_cost > cur_cost){
									min_cost = cur_cost;
									min_indexes[0] = index[0];
									min_indexes[1] = index[1];
									min_indexes[2] = index[2];
								}
								std::cout << "Combination dests[0]: " << dests[0] << " to nhop: " << 
												(dest_poss_nhop.find(dests[0])->second).find(i)->second << std::endl;
								std::cout << "Combination dests[1]: " << dests[1] << " to nhop: " << 
												(dest_poss_nhop.find(dests[1])->second).find(j)->second << std::endl;
								std::cout << "Combination dests[2]: " << dests[2] << " to nhop: " << 
												(dest_poss_nhop.find(dests[2])->second).find(k)->second << std::endl;
								std::cout << "cost: "	<< cur_cost << std::endl;
								//Setting returns
							}
						}
					}
					(*d_p)[0] = nhop_port.find((dest_poss_nhop.find(dests[0])->second).find(min_indexes[0])->second)->second;
					(*d_p)[1] = nhop_port.find((dest_poss_nhop.find(dests[1])->second).find(min_indexes[1])->second)->second;
					(*d_p)[2] = nhop_port.find((dest_poss_nhop.find(dests[2])->second).find(min_indexes[2])->second)->second;
					
					return -1;
				}
			}
		}
		double cost(int num_dests, const char* dests, int index []){
			if(num_dests == 1){
				std::cout << "num_dests=1; no need for cost calculation !\n";
				exit(1);
			}
			std::vector <char> nhops_used;
			double cost = 0;
			char cur_nhop;
			// From nhop to dest total UNICAST path length
			for(int i=0; i<num_dests; i++){
				cur_nhop = (dest_poss_nhop.find(dests[i])->second).find(index[i])->second;
				cost += (nhop_dest_dist.find(cur_nhop)->second).find(dests[i])->second;
				// Add # of one-hop MC link used				
				if(std::find(nhops_used.begin(), nhops_used.end(), cur_nhop) == nhops_used.end()){
					nhops_used.push_back(cur_nhop);
					++cost;
				}
			}
			return cost;
		}
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
				dest_dist.insert (std::pair<char,int>(name[*vi], d[*vi]));
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
			  dest_port.insert (std::pair<char,int>(name[*vi], nhop_port.find(name[temp])->second)); 
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
