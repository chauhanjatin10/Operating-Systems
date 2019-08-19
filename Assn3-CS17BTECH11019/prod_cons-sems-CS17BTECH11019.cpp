#include<iostream>
#include<fstream>
#include<pthread.h>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>
#include<semaphore.h>

using namespace std;

sem_t mutex;
sem_t empty;
sem_t full;
sem_t prod_time_sem;
sem_t cons_time_sem;

// Opening the file to write to
FILE* log_file = fopen("semaphore.txt", "w");

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
		
		// use semaphore here
		sem_wait(&empty);
		sem_wait(&mutex);

		int produced_item = 1000;

		time_t production_time = time(NULL);
		// writing the Critical Section request time
		fprintf(log_file, "%dth item (a number): %d , produced by thread %d at %s into buffer location %d\n", 
					j+1, produced_item, current_thread_id+1,
					ConvertTime(production_time).c_str(), in+1);


		BUFFER[in] = produced_item;
		in = (in + 1)%CAPACITY;
		count++;
		sem_post(&mutex);
		sem_post(&full);

		usleep(producers_section(producer_engine) * 1000);
		time_t end_time = time(NULL);
		sem_wait(&prod_time_sem);
		producer_time += end_time - start_time;
		sem_post(&prod_time_sem);
	}
}

void* func_consumer(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_consumers; j++){
		
		time_t start_time = time(NULL);
		struct tm * current_time_info;

		// use semaphore here
		sem_wait(&full);
		sem_wait(&mutex);

		int consumed_item = BUFFER[out];

		time_t production_time = time(NULL);
		// writing the Critical Section request time
		fprintf(log_file, "%dth item (a number): %d read from buffer by thread %d at %s from location %d\n", 
					j+1, consumed_item, num_producers + current_thread_id+1,
					ConvertTime(production_time).c_str(), out+1);

		out = (out + 1)%CAPACITY;
		count--;
		sem_post(&mutex);
		sem_post(&empty);

		usleep(consumers_section(consumer_engine) * 1000);
		time_t end_time = time(NULL);
		sem_wait(&cons_time_sem);
		consumer_time += end_time - start_time;
		sem_post(&cons_time_sem);
	}
}

int main(){
	cin>>CAPACITY>>num_producers>>num_consumers>>iter_producers>>iter_consumers
				 >>average_prod_section>>average_consm_section;
	BUFFER = new int[CAPACITY];
	
	// cout<<average_prod_section<<" "<<average_consm_section<<"\n";
	producers_section = exponential_distribution<double>(average_prod_section);
	consumers_section = exponential_distribution<double>(average_consm_section);

	sem_init(&mutex, 0, 1);
	sem_init(&empty, 0, CAPACITY);
	sem_init(&full, 0, 0);
	sem_init(&prod_time_sem, 0, 1);
	sem_init(&cons_time_sem, 0, 1);

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
    cout<<"producer average time is : "<<producer_time/double(num_producers * iter_producers)<<"\n";
    cout<<"consumer average time is : "<<consumer_time/double(num_consumers * iter_consumers);
}