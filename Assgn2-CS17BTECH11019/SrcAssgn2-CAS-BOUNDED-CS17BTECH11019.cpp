#include<iostream>
#include<fstream>
#include<pthread.h>
#include<atomic>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>

using namespace std;

// Opening the file to write to
FILE* log_file = fopen("CAS_bounded-Log.txt", "w");

// defining variables
int n, times, average_cs_section, average_rm_section;
double worst_wait_time = 0.0;
double wait_time;	// average waiting time
int* thread_information;	// number of times each thread has to run
bool* waiting_info;	// boolean waiting array

atomic<int> lock(0);	// atomic lock variable

// random number generators
default_random_engine engine_1, engine_2;
poisson_distribution<int> cs_section(100);
poisson_distribution<int> rem_section(100);

// function to be passed to threads
void* testCS(void* thread_ptr){
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id
	int j;
	bool thread_key;
	do {
		if(thread_information[current_thread_id] == 0){
			pthread_exit(0);
		}
		// setting the values for current thread
		waiting_info[current_thread_id] = true;
		thread_key = false;

		time_t request_time = time(NULL);
		struct tm * current_time_info;
		current_time_info = localtime (&request_time);
		char output_request_time[10];
		sprintf(output_request_time, "%.2d:%.2d:%.2d", current_time_info->tm_hour, current_time_info->tm_min, current_time_info->tm_sec);
		string string_request_time(output_request_time);
		
		// writing the Critical Section request time
		fprintf(log_file, "%dst CS Requested at %s by thread %d\n", 
					times - thread_information[current_thread_id]+1,
					string_request_time.c_str(), current_thread_id);

		// compare and swap intruction to set the value of key variable
		while(waiting_info[current_thread_id] && !thread_key){
			int x = 0, y = 1;
			thread_key = lock.compare_exchange_strong(x, y);
		}

		waiting_info[current_thread_id] = false;

		time_t start_time = time(NULL);
		current_time_info = localtime (&start_time);
		char output_start_time[10];
		sprintf(output_start_time, "%.2d:%.2d:%.2d", current_time_info->tm_hour, current_time_info->tm_min, current_time_info->tm_sec);
		string string_start_time(output_start_time);

		// writing the Critical Section start time
		fprintf(log_file, "%dst CS Entered at %s by thread %d\n", 
					times - thread_information[current_thread_id]+1,
					string_start_time.c_str(), current_thread_id);
		
		if (difftime(start_time, request_time) > worst_wait_time){
			worst_wait_time = difftime(start_time, request_time);
		}

		wait_time += difftime(start_time, request_time);
		
		// crtical section simulation
		usleep(cs_section(engine_1) * 100);

		thread_information[current_thread_id]--;
		
		time_t finish_time = time(NULL);
		current_time_info = localtime (&finish_time);
		char output_finish_time[10];
		sprintf(output_finish_time, "%.2d:%.2d:%.2d", current_time_info->tm_hour, current_time_info->tm_min, current_time_info->tm_sec);
		string string_finish_time(output_finish_time);
		
		fprintf(log_file, "%dst CS Exited at %s by thread %d\n", 
					times - thread_information[current_thread_id],
					string_finish_time.c_str(), current_thread_id);

		j = (current_thread_id + 1)%n;
		while((j != current_thread_id) && !waiting_info[j]){
			j = (j+1)%n;
		}

		if(j == current_thread_id){
			lock = 0;
		}
		else{
			waiting_info[j] = false;
		}
		
		//remainder section simulation
		usleep(rem_section(engine_2) * 100);

	}while(true);
	pthread_exit(0);
}

int main(){
	cin>>n>>times>>average_cs_section>>average_rm_section;
	thread_information = new int[n];
	waiting_info = new bool[n];
	
	for(int i=0; i<n; i++){
		thread_information[i] = times;
		waiting_info[i] = false;
	}

	// creating n threads
	pthread_t thread_workers[n];
	// creating attributes for threads
	pthread_attr_t thread_attr[n];
    
	fprintf(log_file, "Bounded CAS ME Output:\n");

	// initialising thread attributes
    for(int i=0; i<n; i++){
    	pthread_attr_init(thread_attr+i);
    }

    // creating threads
    for(int i=0; i<n; i++){
    	pthread_create(thread_workers+i, thread_attr+i, testCS, (void*)i);
    }

    // joining the threads
    for(int i=0; i<n; i++){
    	pthread_join(thread_workers[i], NULL);
    }

    fclose(log_file);
    log_file = fopen("Average_times_CAS_bounded.txt","w");
    fprintf(log_file, "Average wait time (milliseconds) of CAS_bounded :%lf\n", wait_time * 1000/(n * times));
    fprintf(log_file, "Worst wait time (seconds) in CAS_bounded :%lf\n", worst_wait_time);
    fclose(log_file);
}