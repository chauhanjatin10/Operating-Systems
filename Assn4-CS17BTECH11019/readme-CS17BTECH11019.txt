Readme ->

Instructions - 

1) Compile using -> g++ rw-CS17BTECH11019.cpp -o rw -lpthread -w 
		    for unfair case, and 
		    g++ frw-CS17BTECH11019.cpp -o frw -lpthread -w
		    for fair case

2) Run the correspoding input file using ->

		./rw < input.txt , for unfair case
		./frw < input.txt , for fair case

A log file will be created, containing the details of all accesses of writer
and reader threads.
Another log file will be created containing the details of average waiting times
for reader and writer threads.
