#!/bin/bash
echo $1
if [ $1 = 'eg' ]
then
	make eg
	./boost_deneme
	
elif [ $1 = 'f' ]
then
	make clean
	make common
	make router
	make final
	./final a
elif [ $1 = 'g' ]
then
	make gateway
	./gateway
else
	echo "Argument did not match !"
fi
