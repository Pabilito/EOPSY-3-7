#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //usleep

//Predefining a few things
#define N	5
#define LEFT	( i + N - 1 ) % N
#define RIGHT	( i + 1 ) % N

#define THINKING 2
#define HUNGRY 1
#define EATING 0

#define MEALS 10

pthread_mutex_t		m; 					//initialized later to 1
int			state[N];				//initiated later to THINKING's
pthread_cond_t		s[N];					//initialized later to 0's
int 			meals_eaten[N];				//used to count meals
pthread_t 		thread_id[N];				//stores thread ids


void test( int i )
{
	if( state[i] == HUNGRY								//check if we can eat (our neighbors are not eating)
		&& state[LEFT] != EATING
		&& state[RIGHT] != EATING )
	{
		state[i] = EATING;
		printf("[%d]: Philosopher %d is eating\n", thread_id[i], i);
		usleep(1000);								//eating time
		pthread_cond_signal( &s[i] );
	}
}

void grab_forks( int i )
{
	pthread_mutex_lock( &m );				//lock critical section
	
	state[i] = HUNGRY;
	test( i );

	while(state[i] != EATING){				//wait for mutex if hungry
		pthread_cond_wait( &s[i], &m);
	}
			
	pthread_mutex_unlock( &m );				//unlock mutex
}	

void put_away_forks( int i )
{
	pthread_mutex_lock( &m );				//lock mutex to signalize puting forks away

	state[i] = THINKING;					//change state to not eating one
	printf("[%d]: Philosopher %d stopped eating\n", thread_id[i], i);
	test( LEFT );						//test neighbours
	test( RIGHT );

	pthread_mutex_unlock( &m );				//end of puting forks away, so unlock mutex
}

void* new_thread(void* arg){
	int phil = *(int*) arg;
	printf("[%d]: Philosopher %d sits at the table\n", thread_id[phil], phil);
	usleep(10000); 								//delay designed to make everyone be at the table by the time first eating starts

	int a;
	for(a=0; a<MEALS; ++a){							//loop that many times as many meals we need to eat
		usleep(100); 							
		grab_forks(phil);
		usleep(100);
		put_away_forks(phil);
		usleep(1000);								//delay to give every philosopher equal chances
	}
	printf("[%d]: Philosopher %d LEAVES THE TABLE <------------------------\n", pthread_self(), phil);	//pthread_self() is other way of obtaining id of thread
	//leave the table here
}

int main(){
	pthread_mutex_init(&m, NULL);							//initialize mutex, default attributes
	pthread_mutex_unlock(&m);							//set to 1
	void* ret;
	int i;
	for(i=0; i<N; i++){
		state[i] = THINKING;							//for now thinking
		meals_eaten[i] = 0;
		pthread_cond_init(&s[i], NULL);						//initialize condition variable, set to 0 by default
	}

	for(i=0; i<N; i++){
		if (pthread_create(&thread_id[i], NULL, &new_thread, (void*)&i) != 0) {	//create new threads, I will pass philosopher number, I need to convert it to voidptr
		    	perror("pthread_create() error");
		    	exit(-1);
		}
		usleep(100); //if no sleep here, all philosophers take number 0 for some reason
	}

	for(i=0; i<N; i++){
		if (pthread_join(thread_id[i], NULL) != 0) {				//main thread waits for subthreads
		    perror("pthread_join() error");
		    exit(-2);
		}
	}

	return 0; 						
}

/*
Q1 - I think it wouldn't be enough to add mutex in task 5 as it operates on threads, 
while in that task we had to organize critical section for processes.

Q2 - We start with mutex set to 1 as we always at first lock it to perform some operation and deny access for other processes and then we unlock it to indicate that we performed our action and we allow other threads to do their task. When it comes to conditional variable we need to have it set to 0 as then send signals from test() to indicate eating state.
*/


