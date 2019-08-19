#include<iostream>
#include<fstream>
#include<pthread.h>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>
#include<semaphore.h>

using namespace std;

pthread_mutex_t mutex;
pthread_mutex_t prod_time_mutex;
pthread_mutex_t cons_time_mutex;

// Opening the file to write to
FILE* log_file = fopen("mutex_lock.txt", "w");

// defining variables
int CAPACITY, num_producers, num_consumers, iter_producers, iter_consumers, 
	average_prod_section, average_consm_section;
int* BUFFER;	// number of times each thread has to run

// random number generators
default_random_engine producer_engine, consumer_engine;
exponential_distribution<double> producers_section;
exponential_distribution<double> consumers_section;


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

double producer_time = 0.0;
double consumer_time = 0.0;

int count = 0;

int in = 0;
int out = 0;

void* func_producer(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_producers; j++){
		
		time_t start_time = time(NULL);
		struct tm * current_time_info;
		
		while(count == CAPACITY){
			continue;
		}

		// used mutex lock here
		pthread_mutex_lock(&mutex);
		int produced_item = 1000;

		time_t production_time = time(NULL);
		// writing the Critical Section request time
		fprintf(log_file, "%dth item (a number): %d produced by thread %d at %s into buffer location %d\n", 
					j+1, produced_item, current_thread_id+1,
					ConvertTime(production_time).c_str(), in+1);


		BUFFER[in] = produced_item;
		in = (in + 1)%CAPACITY;
		count++;
		pthread_mutex_unlock(&mutex);

		usleep(producers_section(producer_engine) * 1000);
		time_t end_time = time(NULL);
		pthread_mutex_lock(&prod_time_mutex);
		producer_time += end_time - start_time;
		pthread_mutex_unlock(&prod_time_mutex);

	}
}

void* func_consumer(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_consumers; j++){
		
		time_t start_time = time(NULL);
		struct tm * current_time_info;

		while(count == 0){
			continue;
		}

		// used mutex lock here
		pthread_mutex_lock(&mutex);
		int consumed_item = BUFFER[out];

		time_t production_time = time(NULL);
		// writing the Critical Section request time
		fprintf(log_file, "%dth item (a number): %d read from buffer by thread %d at %s from location %d\n", 
					j+1, consumed_item, num_producers + current_thread_id+1,
					ConvertTime(production_time).c_str(), out+1);

		out = (out + 1)%CAPACITY;
		count--;
		pthread_mutex_unlock(&mutex);

		usleep(consumers_section(consumer_engine) * 1000);
		time_t end_time = time(NULL);
		pthread_mutex_lock(&cons_time_mutex);
		consumer_time += end_time - start_time;
		pthread_mutex_unlock(&cons_time_mutex);
	}
}

int main(){
	cin>>CAPACITY>>num_producers>>num_consumers>>iter_producers>>iter_consumers
				 >>average_prod_section>>average_consm_section;
	BUFFER = new int[CAPACITY];
	
	producers_section = exponential_distribution<double>(average_prod_section);
	consumers_section = exponential_distribution<double>(average_consm_section);

	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&prod_time_mutex, NULL);
	pthread_mutex_init(&cons_time_mutex, NULL);

	// creating num_producers threads
	pthread_t thread_workers_producers[num_producers];
	// creating attributes for threads
	pthread_attr_t thread_attr_producers[num_producers];

	// creating num_consumers threads
	pthread_t thread_workers_consumers[num_consumers];
	// creating attributes for threads
	pthread_attr_t thread_attr_consumers[num_consumers];

	// initialising thread attributes for producers
    for(int i=0; i<num_producers; i++){
    	pthread_attr_init(thread_attr_producers + i);
    }
    // initialising thread attributes for consumers
    for(int i=0; i<num_consumers; i++){
    	pthread_attr_init(thread_attr_consumers + i);
    }

    // creating threads for producers
    for(int i=0; i<num_producers; i++){
    	pthread_create(thread_workers_producers + i, thread_attr_producers + i,
    				   func_producer, (void*)i);
    }

    // creating threads for consumers
    for(int i=0; i<num_consumers; i++){
    	pthread_create(thread_workers_consumers + i, thread_attr_consumers + i,
    				   func_consumer, (void*)i);
    }

    // joining the producer threads 
    for(int i=0; i<num_producers; i++){
    	pthread_join(thread_workers_producers[i], NULL);
    }
    // joining the consumer threads
    for(int i=0; i<num_consumers; i++){
    	pthread_join(thread_workers_consumers[i], NULL);
    }

    fclose(log_file);
    // cout<<"producer average time is : "<<producer_time/double(num_producers * iter_producers)<<"\n";
    // cout<<"consumer average time is : "<<consumer_time/double(num_consumers * iter_consumers);
}