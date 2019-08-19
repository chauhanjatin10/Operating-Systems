#include<iostream>
#include<fstream>
#include<pthread.h>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>
#include<semaphore.h>

using namespace std;

sem_t rw_mutex;
sem_t mutex;
int READ_COUNT = 0;

// Opening the file to write to
FILE* log_file = fopen("RW-log.txt", "w");

// defining variables
int num_writers, num_readers, iter_writers, iter_readers, 
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

double av_writer_time = 0.0;
double av_reader_time = 0.0;

double worst_writer_time = 0.0;
double worst_reader_time = 0.0;


void* func_writer(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_writers; j++){
		
		time_t request_time = time(NULL);
		clock_t t;
		t = clock();
		struct tm * current_time_info;
		fprintf(log_file, "%dth CS request by Writer Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(request_time).c_str());

		// use semaphore here
		sem_wait(&rw_mutex);

		time_t start_time = time(NULL);
		t = clock() - t;
		av_writer_time += (((float)t) * 1000.0)/CLOCKS_PER_SEC;

		if ((((float)t) * 1000.0)/CLOCKS_PER_SEC > worst_writer_time){
			worst_writer_time = (((float)t) * 1000.0)/CLOCKS_PER_SEC;
		}

		fprintf(log_file, "%dth CS Entry by Writer Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(start_time).c_str());

		usleep(cs_section(cs_engine) * 1000);

		sem_post(&rw_mutex);

		time_t end_time = time(NULL);
		fprintf(log_file, "%dth CS Exit by Writer Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(end_time).c_str());

		usleep(rem_section(rem_engine) * 1000);
	}
}

void* func_reader(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_readers; j++){
		
		time_t request_time = time(NULL);
		clock_t t;
		t = clock();
		struct tm * current_time_info;
		fprintf(log_file, "%dth CS request by Reader Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(request_time).c_str());

		// use semaphore here
		sem_wait(&mutex);
		READ_COUNT++;
		if (READ_COUNT == 1) {
			sem_wait(&rw_mutex);
		}
		sem_post(&mutex);

		time_t start_time = time(NULL);
		t = clock() - t;
		av_reader_time += (((float)t) * 1000.0)/CLOCKS_PER_SEC;

		if ((((float)t) * 1000.0)/CLOCKS_PER_SEC > worst_reader_time){
			worst_reader_time = (((float)t) * 1000.0)/CLOCKS_PER_SEC;
		}

		fprintf(log_file, "%dth CS Entry by Reader Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(start_time).c_str());

		usleep(cs_section(cs_engine) * 1000);

		sem_wait(&mutex);
		READ_COUNT--;
		if(READ_COUNT == 0){
			sem_post(&rw_mutex);
		}
		sem_post(&mutex);

		time_t end_time = time(NULL);
		fprintf(log_file, "%dth CS Exit by Reader Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(end_time).c_str());

		usleep(rem_section(rem_engine) * 1000);
	}
}

int main(){
	cin>>num_writers>>num_readers>>iter_writers>>iter_readers
				 >>average_cs_section>>average_rem_section;
	
	// cout<<average_cs_section<<" "<<average_rem_section<<"\n";
	cs_section = exponential_distribution<double>(average_cs_section);
	rem_section = exponential_distribution<double>(average_rem_section);

	sem_init(&mutex, 0, 1);
	sem_init(&rw_mutex, 0, 1);

	// creating num_writers threads
	pthread_t thread_workers_writers[num_writers];
	// creating attributes for threads
	pthread_attr_t thread_attr_writers[num_writers];

	// creating num_readers threads
	pthread_t thread_workers_readers[num_readers];
	// creating attributes for threads
	pthread_attr_t thread_attr_readers[num_readers];

	// initialising thread attributes for producers
    for(int i=0; i<num_writers; i++){
    	pthread_attr_init(thread_attr_writers + i);
    }
    // initialising thread attributes for consumers
    for(int i=0; i<num_readers; i++){
    	pthread_attr_init(thread_attr_readers + i);
    }

    // creating threads for producers
    for(int i=0; i<num_writers; i++){
    	pthread_create(thread_workers_writers + i, thread_attr_writers + i,
    				   func_writer, (void*)i);
    }

    // creating threads for consumers
    for(int i=0; i<num_readers; i++){
    	pthread_create(thread_workers_readers + i, thread_attr_readers + i,
    				   func_reader, (void*)i);
    }

    // joining the producer threads 
    for(int i=0; i<num_writers; i++){
    	pthread_join(thread_workers_writers[i], NULL);
    }
    // joining the consumer threads
    for(int i=0; i<num_readers; i++){
    	pthread_join(thread_workers_readers[i], NULL);
    }

    double average_waiting_time = ( av_writer_time/double(num_writers * iter_writers) 
      							  + av_reader_time/double(num_readers * iter_readers) )/2.0;

    fclose(log_file);
    log_file = fopen("Average_time_RW.txt","w");
    fprintf(log_file, "Average waiting time for writer threads is : %lf\n", av_writer_time/double(num_writers * iter_writers));
    fprintf(log_file, "Average waiting time for reader threads is : %lf\n", av_reader_time/double(num_readers * iter_readers));
    fprintf(log_file, "Average average time is : %lf\n", average_waiting_time);
    fclose(log_file);
    // printf("Worst writer waiting time is : %lf\n", worst_writer_time);
    // printf("Worst reader waiting time is : %lf\n", worst_reader_time);
}