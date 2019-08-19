#include<iostream>
#include<algorithm>
#include<map>
#include<vector>
#include<climits>
#include<climits>
#include<fstream>

using namespace std;

// Output file streams
ofstream log_stream;
ofstream stats_stream;

void initiate_streams(){
	log_stream.open("RM-Log.txt");	
	stats_stream.open("RM-Stats.txt");	
}

//Struct for storing process information
struct ProcessInfo{	
	int process_id;		// process ID
	int processing_time;// times for each CPU burst
	int deadline;		// deadline for current cycle
	int period;			// period of current process
	int time_left;		// time for which it has to run in current cycle
	int expected_next_incoming_time;	// time at which it will come in ready queue next
	int num_times;		// number of times it has to be executed
	int ready_queue_incoming;	// time at which it arrives in ready queue
	int dispatch_time;	// time at which it is dispatched out of ready queue
	bool running;		// if its running on CPU
	bool in_ready_queue;	// if it is in ready queue
};

struct comparison_key //Sorting function comparator
{
    inline bool operator() (ProcessInfo* p1, ProcessInfo* p2)
    {
        return (p1->period < p2->period);
    }
};

//Varibles made global, for they are to be used in most of the funcitons
vector<ProcessInfo*> ready_queue;	// ready queue to store processes to run on cpu
int global_timer = 0;	// timer to get current clock values (in milliseconds)
int total_runs = 0;		// total number of processes to run
int number_of_failures = 0;		// number of processes missing their deadlines
double average_waiting_time = 0.0;	// average waiting time


// to print the initial information of the processes
void print_process_info(ProcessInfo processes[], int num_processes){
	for(int i=0; i<num_processes; i++){
		log_stream<<"Process P"<<processes[i].process_id<<": processing time = "
		<<processes[i].processing_time<<"; deadline: "
		<<processes[i].deadline<<"; period: "<<processes[i].period
		<<" joined the system at time 0\n";
	}
}

// to fetch a process form ready queue (ready queue is already sorted)
ProcessInfo* get_process_from_ready_queue(){
	ProcessInfo* process = ready_queue[0];
	process->in_ready_queue = false;
	ready_queue.erase(ready_queue.begin());
	return process;
}

// to check if current running process had to be premepted
bool need_to_be_preempted(ProcessInfo* current_process, ProcessInfo processes[], int num_processes){
	for(int i=0; i<num_processes; i++){
		if(processes[i].num_times > 0 && processes[i].in_ready_queue==false && processes[i].expected_next_incoming_time < global_timer){
			if(current_process->process_id != processes[i].process_id && current_process->period > processes[i].period){
				return true;
			}
		}
	}
	return false;
}

// recreating ready queue. It includes ->
//	1) Reassinging the attributes of processes which have missed their current deadlines
//	2) Processes outside ready queue which have to be put back
void recreate_ready_queue(ProcessInfo* current_process, ProcessInfo processes[], int num_processes){
	for(int i=0; i<ready_queue.size(); i++){
		if(ready_queue[i]->deadline <= global_timer){
			// increasing the value of average waiting time variable
			average_waiting_time += ready_queue[i]->deadline - ready_queue[i]->ready_queue_incoming;
			
			ready_queue[i]->expected_next_incoming_time = ready_queue[i]->deadline;
			log_stream<<"Deadline miss by Process P"<<ready_queue[i]->process_id<<" at time "<<ready_queue[i]->deadline<<"\n";
			number_of_failures++;
			total_runs--;	// decreasing total processes that have to be run
			ready_queue[i]->deadline += ready_queue[i]->period;
			ready_queue[i]->num_times--;
			ready_queue[i]->ready_queue_incoming = ready_queue[i]->expected_next_incoming_time;
			ready_queue[i]->time_left = ready_queue[i]->processing_time;
			ready_queue[i]->in_ready_queue = true;
			if(ready_queue[i]->num_times <= 0){
				ready_queue.erase(ready_queue.begin() + i);
				i = i-1;
			}
		}
	}

	for(int i=0; i<num_processes; i++){
		if(processes[i].num_times > 0 && processes[i].expected_next_incoming_time <= global_timer && 
								processes[i].in_ready_queue == false){
			if(current_process->process_id != processes[i].process_id){
				processes[i].ready_queue_incoming = processes[i].expected_next_incoming_time;
				ready_queue.push_back(&processes[i]);
				processes[i].in_ready_queue = true;
			}
		}
	}

	// sorting the ready queue on basis of sorting comparator
	sort(ready_queue.begin(),ready_queue.end(), comparison_key());
}

// get earliest process, when CPU is idle
ProcessInfo* get_earliest_possible_process(ProcessInfo processes[], int num_processes){
	int min_time = INT_MAX;
	int index = 0;		// index of process with highest priority
	ProcessInfo* process;
	for(int i=0; i<num_processes; i++){
		if(processes[i].num_times > 0 && processes[i].expected_next_incoming_time < min_time && 
								processes[i].in_ready_queue == false){
			min_time = processes[i].expected_next_incoming_time;
			index = i;
		}
	}
	
	process = &processes[index];
	return process;
}

// calculating the statistics and saving to file
void calculate_stats(int total_processes){
	average_waiting_time = double(average_waiting_time)/double(total_processes);
	// stats_stream<<"Total processes ran - "<<total_processes<<"\n";
	stats_stream<<"Number of Processes entering the system : "<<total_processes<<"\n";
    stats_stream<<"Number of Processes successfully completed : "<<total_processes-number_of_failures<<"\n";
	stats_stream<<"Numer of Processes which missed the deadline : "<<number_of_failures<<"\n";
	stats_stream<<"RMS : Average waiting time : "<<average_waiting_time<<"\n";
}

int main() {
	initiate_streams();

	int num_processes;	// number of processes
	cin>>num_processes;

	ProcessInfo processes[num_processes];

	for(int i=0; i<num_processes; i++){
		int p_id, t, period, k;
		cin>>p_id>>t>>period>>k;
		total_runs += k;

		processes[i].process_id = p_id;
		processes[i].processing_time = t;
		processes[i].deadline = period;
		processes[i].period = period;
		processes[i].time_left = t;
		processes[i].num_times = k;
		processes[i].expected_next_incoming_time = 0;
		processes[i].in_ready_queue = true;
		processes[i].running = false;
		processes[i].ready_queue_incoming = 0;

		// pushing in ready queue
		ready_queue.push_back(&processes[i]);
	}

	// printing the process on screen 
	print_process_info(processes, num_processes);

	int total_processes = total_runs;

	sort(ready_queue.begin(),ready_queue.end(), comparison_key()); //sorts the queue according to period
	
	ProcessInfo* current_process;
	current_process = get_process_from_ready_queue();	// getting first process to run
	current_process->running = true;
	current_process->dispatch_time = global_timer;

	int time_before_execution = 0;

	int count = 0;

	while(total_runs > 0){
		if(global_timer != 0){
			// create ready queue after each iteration
			recreate_ready_queue(current_process, processes, num_processes);

			// if size of ready queue is 0, there is no process to run currently
			if(ready_queue.size() == 0){
				// fetching process to run on CPU
				current_process = get_earliest_possible_process(processes, num_processes);
				current_process->in_ready_queue = false;
				current_process->running = true;
				current_process->ready_queue_incoming = current_process->expected_next_incoming_time;
				
				time_before_execution = global_timer;
				// incrementing timer
				global_timer = current_process->expected_next_incoming_time;
				log_stream<<"CPU is idle till time "<<global_timer<<"\n";
			}
		}

		// if no process is running on CPU
		if(current_process->running == false){
			current_process = get_process_from_ready_queue();
		}
		
		// check if a process to start it s current cycle or resumes after it was preempted
		if(current_process->time_left == current_process->processing_time){
			log_stream<<"Process P"<<current_process->process_id<<" starts execution at time "<<global_timer<<".\n";
			current_process->dispatch_time = global_timer;
		}
		else{
			log_stream<<"Process P"<<current_process->process_id<<" resumes execution at time "<<global_timer<<".\n";	
			current_process->dispatch_time = global_timer;
		}

		time_before_execution = global_timer;
		global_timer += current_process->time_left; //setting the timer value

		//preemption condition
		if(need_to_be_preempted(current_process, processes, num_processes)){
			recreate_ready_queue(current_process, processes, num_processes);
			ProcessInfo* new_process = get_process_from_ready_queue();
			current_process->time_left = global_timer - new_process->expected_next_incoming_time;
			
			average_waiting_time += (current_process->dispatch_time - current_process->ready_queue_incoming);
			
			global_timer = new_process->expected_next_incoming_time;
			current_process->ready_queue_incoming = global_timer;
			ready_queue.push_back(current_process);
			current_process->in_ready_queue = true;
			log_stream<<"Process P"<<current_process->process_id<<
			" is preempted by Process P"<<new_process->process_id<<" at time "<<global_timer<<
			". Remaining processing time:"<<current_process->time_left<<"\n";
			current_process = new_process;
			current_process->running = true;
		}

		// deadline missed
		else if(global_timer > current_process->deadline){
			total_runs--;
			number_of_failures++;
			current_process->num_times--;

			average_waiting_time += (current_process->deadline - current_process->ready_queue_incoming);
			current_process->ready_queue_incoming = current_process->expected_next_incoming_time;

			// setting the current process attributes
			global_timer = current_process->deadline;
			current_process->expected_next_incoming_time = global_timer;
			current_process->deadline += current_process->period;
			current_process->running = false;
			current_process->in_ready_queue = false;
			log_stream<<"Deadline miss by Process P"<<current_process->process_id<<" at time "<<global_timer<<"\n";
		}

		else if(global_timer <= current_process->deadline){
			total_runs--;
			log_stream<<"Process P"<<current_process->process_id<<" finishes execution at time "<<global_timer<<".\n";
			
			average_waiting_time += (current_process->dispatch_time - current_process->ready_queue_incoming);
			
			// setting attributes of current process
			current_process->expected_next_incoming_time = current_process->deadline;
			current_process->deadline += current_process->period;
			current_process->time_left = current_process->processing_time;
			current_process->in_ready_queue = false;
			current_process->num_times--;
			current_process->running = false;

		}
	}

	// calculating the stats
	calculate_stats(total_processes);

	// closing file streams
	log_stream.close();
	stats_stream.close();

}