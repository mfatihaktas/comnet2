CC = g++
BGLFLAGS = -I /home/mehmet/Desktop/comnet2/ComNet_project/boost_1_53_0 -O3
CCOPTS = 
LIB = -lpthread
#-g -Wall

bd_sf=boost_deneme.cpp
bd_tf=boost_deneme
#ftf_tf=forwarding_table_filler.o
d_tf=deneme
f_tf=final


eg: $(bd_sf)
		$(CC) $(CCOPTS) $< -o $(bd_tf)

common : common.h common.cpp
		$(CC) $(CCOPTS) -c common.cpp

router : router.h router.cpp common.o
		$(CC) $(CCOPTS) -c router.cpp $(LIB)
		
deneme : deneme.cpp router.o
		$(CC) $(CCOPTS) -c deneme.cpp
		
#final: deneme.o router.o common.o
#		$(CC) $(CCOPTS) -o $(f_tf) $(BGLFLAGS) $< router.o common.o

final: router.o common.o
		$(CC) $(CCOPTS) -o $(f_tf) $(BGLFLAGS) router.o common.o $(LIB)
		
clean:
		rm -rf $(bd_tf) $(d_tf) $(f_tf) router.o common.o deneme.o
		
