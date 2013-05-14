#!/bin/bash
echo $1
if [ $1 = 'eg' ]
then
	make eg
	./boost_deneme
	
elif [ $1 = '1' ]
then
	make clean
	make common
	make router
	make final
	./final A
elif [ $1 = '2' ]
then
	./final B
elif [ $1 = '3' ]
then
	./final C
elif [ $1 = '4' ]
then
	./final D
elif [ $1 = '5' ]
then
	./final W
elif [ $1 = '6' ]
then
	./final X
elif [ $1 = '7' ]
then
	./final Y
elif [ $1 = '8' ]
then
	./final Z

elif [ $1 = 'g' ]
then
	make gateway
	./gateway
else
	echo "Argument did not match !"
fi
