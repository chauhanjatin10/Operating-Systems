#include<iostream>
#include<fstream>
#include<pthread.h>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>
#include<semaphore.h>

using namespace std;

// Opening the file to write to
FILE* log_file = fopen("ME_log.txt", "w");

// defining the condition variables 
pthread_mutex_t mutex;

// defining variables
int num_threads, iterations, num_classes,
	average_cs_section, average_rem_section;

// random number generators
default_random_engine cs_engine, rem_engine;
exponential_distribution<double> cs_section;
exponential_distribution<double> rem_section;

//get formatted time
string ConvertTime(time_t input)
{
  struct tm * timeinfo;
  timeinfo = localtime (&input);
  static char output[10];
  sprintf(output,"%.2d:%.2d:%.2d",timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);
  string tim(output);
  return tim;
}

double av_waiting_time = 0.0;

void* func(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iterations; j++){

		time_t request_time = time(NULL);
		clock_t t;
		t = clock();
		struct tm * current_time_info;
		fprintf(log_file, "%dth CS Request at %s by thread %d for session \n", 
					j, ConvertTime(request_time).c_str(), current_thread_id);

		// using condition variables here
		pthread_mutex_lock(&mutex);

		time_t start_time = time(NULL);
		t = clock() - t;
		//average waiting time in microseconds
		av_waiting_time += (((float)t) * 1000.0)/CLOCKS_PER_SEC;

		fprintf(log_file, "%dth CS Entry at %s by thread %d for session\n", 
					j, ConvertTime(start_time).c_str(), current_thread_id);
		// simuation of critical section
		usleep(cs_section(cs_engine) * 1000); // sleep in milliseconds

		pthread_mutex_unlock(&mutex);

		time_t end_time = time(NULL);
		fprintf(log_file, "%dth CS Exit at %s by thread %d for session\n", 
					j, ConvertTime(start_time).c_str(), current_thread_id);
		// simulation of remainder section
		usleep(rem_section(rem_engine) * 1000); // sleep in milliseconds
	}
}

int main(){
	cin>>num_threads>>iterations>>num_classes
				>>average_cs_section>>average_rem_section;
	
	cs_section = exponential_distribution<double>(average_cs_section);
	rem_section = exponential_distribution<double>(average_rem_section);

	// creating num_threads threads
	pthread_t thread_workers[num_threads];
	// creating attributes for threads
	pthread_attr_t thread_attr[num_threads];

	// initialising thread attributes
    for(int i=0; i<num_threads; i++){
    	pthread_attr_init(thread_attr + i);
    }

    // creating threads for producers
    for(int i=0; i<num_threads; i++){
    	pthread_create(thread_workers + i, thread_attr + i,
    				   func, (void*)i);
    }

    // joining the producer threads 
    for(int i=0; i<num_threads; i++){
    	pthread_join(thread_workers[i], NULL);
    }
    fclose(log_file);
    double average_waiting_time = av_waiting_time/double(num_threads * iterations);
    cout<<"average waiting time is - "<<average_waiting_time<<" microseconds\n";
}