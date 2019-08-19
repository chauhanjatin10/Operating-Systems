#include<iostream>
#include<fstream>
#include<pthread.h>
#include<random>
#include<stdio.h>
#include<unistd.h>
#include<climits>

using namespace std;

// Opening the file to write to
FILE* log_file = fopen("dphil-log.txt", "w");

// defining the condition variables 
pthread_mutex_t mutex;
pthread_cond_t *cond_var;

// defining variables
int num_philosophers, iter_philosophers, 
	average_think_section, average_eat_section;
bool *chopsticks;

// random number generators
default_random_engine eat_engine, think_engine;
exponential_distribution<double> eat_section;
exponential_distribution<double> think_section;


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

double av_time = 0.0;
double worst_time = 0.0;


void* func_philosopher(void* thread_ptr) {
	int current_thread_id = *((int*)(&thread_ptr));	// current thread id

	for (int j=0; j<iter_philosophers; j++){
		
		time_t request_time = time(NULL);
		clock_t t;
		t = clock();
		struct tm * current_time_info;
		fprintf(log_file, "%dth eat request by Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(request_time).c_str());

		// use condition variable here
		pthread_mutex_lock(&mutex);
		while(chopsticks[current_thread_id%num_philosophers]!=true and 
				chopsticks[(current_thread_id-1)%num_philosophers]!=true){
			pthread_cond_wait(cond_var+(current_thread_id%num_philosophers), &mutex);
			// acquiring the chopsticks
			chopsticks[current_thread_id % num_philosophers] = false;
			chopsticks[(current_thread_id-1) % num_philosophers] = false;
		}
		pthread_mutex_unlock(&mutex);

		time_t start_time = time(NULL);
		t = clock() - t;

		fprintf(log_file, "%dth CS Entry by Philosopher Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(start_time).c_str());

		usleep(eat_section(eat_engine) * 1000);
		
		pthread_mutex_lock(&mutex);
		// the average time and worst time variables might also undergo race conditions
		// To prevent this, they are also updated inside the lock
		av_time += (((float)t) * 1000.0)/CLOCKS_PER_SEC;
		if ((((float)t) * 1000.0)/CLOCKS_PER_SEC > worst_time){
			worst_time = (((float)t) * 1000.0)/CLOCKS_PER_SEC;
		}
		//releasing the chopsticks
		chopsticks[current_thread_id % num_philosophers] = true;
		chopsticks[(current_thread_id-1) % num_philosophers] = true;
		pthread_cond_signal(cond_var+((current_thread_id+1)%num_philosophers));
		pthread_cond_signal(cond_var+((current_thread_id-1)%num_philosophers));
		pthread_mutex_unlock(&mutex);

		time_t end_time = time(NULL);
		fprintf(log_file, "%dth CS Exit by Philosopher Thread %d at %s\n", 
					j+1, current_thread_id+1, ConvertTime(end_time).c_str());

		usleep(think_section(think_engine) * 1000);
	}
}

int main(){
	cin>>num_philosophers>>iter_philosophers>>average_eat_section>>average_think_section;
	
	eat_section = exponential_distribution<double>(average_eat_section);
	think_section = exponential_distribution<double>(average_think_section);

	cond_var = new pthread_cond_t[num_philosophers];

	chopsticks = new bool[num_philosophers];
	for(int i=0; i<num_philosophers; i++){
		chopsticks[i] = true;
	}

	pthread_mutex_init(&mutex, NULL);
	for(int i=0; i<num_philosophers; i++){
		pthread_cond_init(cond_var+i, NULL);
	}

	// creating num_philosophers threads
	pthread_t thread_workers[num_philosophers];
	// creating attributes for threads
	pthread_attr_t thread_attrs[num_philosophers];

	// initialising thread attributes for philosophers
    for(int i=0; i<num_philosophers; i++){
    	pthread_attr_init(thread_attrs + i);
    }

    // creating threads for philosophers
    for(int i=0; i<num_philosophers; i++){
    	pthread_create(thread_workers + i, thread_attrs + i,
    				   func_philosopher, (void*)i);
    }

    // joining the philosophers threads 
    for(int i=0; i<num_philosophers; i++){
    	pthread_join(thread_workers[i], NULL);
    }

    fclose(log_file);
    log_file = fopen("Times.txt","w");
    fprintf(log_file, "Average waiting time for Philosopher threads is : %lf\n", av_time/double(num_philosophers * iter_philosophers));
    fprintf(log_file, "Worst case waiting time for Philosopher threads is : %lf\n", worst_time);
    fclose(log_file);
    // printf("Worst writer waiting time is : %lf\n", worst_writer_time);
    // printf("Worst reader waiting time is : %lf\n", worst_reader_time);
}