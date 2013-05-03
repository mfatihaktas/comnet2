CC = g++
BGLFLAGS = -I /home/mehmet/Desktop/comnet2/ComNet_project/boost_1_53_0 -O3
CCOPTS = 
#-g -Wall

bd_sf=boost_deneme.cpp
bd_tf=boost_deneme
ftf_tf=forwarding_table_filler.o
d_tf=deneme


eg: $(bd_sf)
		$(CC) $(CCOPTS) $< -o $(bd_tf)
		
ftf: forwarding_table_filler.cpp
		$(CC) $(CCOPTS) $(BGLFLAGS) $< -o $(ftf_tf)
		
deneme: deneme.cpp
		$(CC) $(CCOPTS) $(BGLFLAGS) $< -o $(d_tf)
		
clean:
		rm -rf $(bd_tf) $(ftf_tf) $(d_tf) 
