Readme ->

Instructions - 

1) Compile the code using -> g++ SrcAssgn2-TAS-CS17BTECH11019.cpp -o tas -lpthread
					      -> g++ SrcAssgn2-CAS-CS17BTECH11019.cpp -o cas -lpthread
					      -> g++ SrcAssgn2-CAS-BOUNDED-CS17BTECH11019.cpp -o cas_bounded -lpthread

2) Run the correspoding input file using -> ./tas < input.txt for TAS
											./cas < input.txt for CAS
											./cas_bounded < input.txt for CAS-BOUNDED


if number of threads is greater than 1, then two files will be created for each program ->

	1) TAS/CAS/CAS-Bounded-Log.txt -> this contains the details of all events. Events are 			considered->
		a) when a thread requests to enter critical section
		b) when it enters the critical section
		c) When it leaves the critical section

	2) Average_times_TAS/CAS/CAS-Bounded.txt -> this contains the following stats ->
		(a) average waiting time to enter critical section (in milliseconds)
		(b) worst waiting time to enter the critical section (in seconds)