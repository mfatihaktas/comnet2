boost_deneme_sf=boost_deneme.cpp
boost_deneme_tf=boost_deneme

bd: $(boost_deneme_sf)
		gcc $< -O3 -o $(boost_deneme_tf)
eg: $(boost_deneme_sf)
		c++ -I /home/mehmet/Desktop/comnet2/ComNet_project/boost_1_53_0 $< -o $(boost_deneme_tf)
clean:
		rm -rf $(boost_deneme_tf) 
