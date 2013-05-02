#!/bin/bash
echo $1
if [ $1 = 'eg' ]
then
	make eg
	./boost_deneme
	
elif [ $1 = 'd' ]
then
	make deneme
	./deneme
	
elif [ $1 = 'g' ]
then
	make gateway
	./gateway
else
	echo "Argument did not match !"
fi
