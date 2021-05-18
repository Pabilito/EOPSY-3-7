#include <stdio.h>
#include <unistd.h>     // fork()
#include <stdlib.h>	// exit()
#include <sys/types.h>	// for semaphores
#include <sys/ipc.h>	// for semaphores
#include <sys/sem.h>	// for semaphores
#include <sys/types.h>
#include <sys/wait.h> 	//for waitpid()

//semget() - connect existing semaphore
//semop()  - perform operation
//semctl() - control operations on message queue

//Predefining a few things
const int PHILOSOPHERS = 5;
const int ITERATIONS = 2;						// Easting iterations
int forks;								// ID for array of IPC semaphores
int not_eating;								// Used to prepare children for eating
struct sembuf op;             						// Used to perform semaphore operations

void grab_forks(int left_fork_id ){

	op.sem_op = -1;							//Decrease value by 1 when semop() called
	op.sem_num = left_fork_id;					//Fork to be taken
	semop(forks, &op, 1);						//Set it semval to 0, something like semwait
									//Nobody else can access it

	printf("Process [%d]: Fork #%d grabbed\n", getpid(), left_fork_id);
	return;
}

void put_away_forks(int left_fork_id ){

	op.sem_op = 1;							//Increase value by 1 when semop() called
	op.sem_num = left_fork_id;					//Fork to be released
	semop(forks, &op, 1);						//Something like semsignal, sets semval to 1
									//Philosophers can take it now

	printf("Process [%d]: Fork #%d released\n", getpid(), left_fork_id);
	return;
}

int do_philosopher_things(int n){			//Child processes are philosophers
	int i, j, first_fork, second_fork;	
	op.sem_flg = 0;					// Flag for operation (would change outcome of semop() if was set to 1 & IPC_NOWAIT was used)
	
	// Get ready to eat
	op.sem_op = -1;					//Decrease value by 1 when semop() called (sem_flg=0)
	op.sem_num = 0;					//Operate on semaphore 0
	semop(not_eating, &op, 1);			//Operation on 1 sembuf array (called op)
	printf("Process [%d]: Philosopher %d at the table\n", getpid(), n);
	//not_eating will reach 0 when we acknowledge all philosophers
	
	// Get all acknowledgements
  	op.sem_op = 0;					//On 0 semop() should just return (sem_flg=0)
  	op.sem_num = 0;					//Still semaphore 0
  	semop(not_eating, &op, 1);			//Operation on 1 sembuf array (called op)

	first_fork =  (n < PHILOSOPHERS)? n : 0;    			//fork id for grab/put_away functions          
	second_fork = (n < PHILOSOPHERS)? n + 1 : PHILOSOPHERS-1; 	//order to avoid deadlock
									//philosophers 1 and 5 fight for fork0
									//so if forks 1,2,3 are taken by p2,p3,p4 blocking each other
									//fork 0 will be at p1 or p5
									//if p1 has f0, p3 will take f4
									//if p5 has f0, he or p4 may take f4 as well 
	usleep(100);

	// Eating phase
	for(i = 0; i < ITERATIONS; i++) {
		usleep(10);         			//suspends execution of the calling thread for (at least) 10 microseconds
							//gives others chance to take forks, but still it is a kind of battle royale

		grab_forks(first_fork);			//grab first fork
		grab_forks(second_fork);		//grab second fork
		printf("Process [%d]: P%d eating\n", getpid(), n);
		
		printf("Process [%d]: P%d stopped eating\n", getpid(), n);   	  
		put_away_forks(first_fork);		//return first fork
		put_away_forks(second_fork);		//return second fork
	  }

  	printf("Process [%d]: Philosopher %d leaves the table\n", getpid(), n);

	return 0;
}



int main() {

	int i, pid, status;
	pid_t PH_ID[PHILOSOPHERS]; 					// we need to store 5 process IDs

  	forks = semget(IPC_PRIVATE, PHILOSOPHERS, IPC_CREAT | 0600);	// 0600 means that only the user that created it has access to it
	// key = IPC_PRIVATE =>new shared memory segment, with size equal to the value of size rounded up to a multiple of PAGE_SIZE
  	
	for(i=0; i<PHILOSOPHERS; i++){
    		semctl(forks, i, SETVAL, 1);				//sets semval to 1
  	}

  	not_eating = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);		//not eating process

	// Make children unable to eat
	semctl(not_eating, 0, SETVAL, 5);				//sets semval to 5

	// Generate child processes
	for(i=0; i < PHILOSOPHERS; i++){
		pid = fork();
		if(pid == 0){               				// child pid = 0	
			printf("Process [%d]: Philosopher %d created\n", getpid(), i);
	      		int exit_code = do_philosopher_things(i); 	// child acts as philosopher
	      		exit(exit_code);                			
		}
	    	else{                       				// parent pid > 0
	      		PH_ID[i] = pid;            			// parent tracks children (philosophers)
	    	}
	 }

	// Waits until all children exit
	for(i = 0; i < PHILOSOPHERS; i++) {
	 	waitpid(PH_ID[i], &status, 0);
	}

	// Eliminate semaphores at the end
	semctl(forks, 0, IPC_RMID, 0);					//IPC_RMID - Mark the segment to be destroyed
	semctl(not_eating, 0, IPC_RMID, 0);
	
	printf("Process [%d]: Parent terminated\n", getpid());
	return 0; 						
}



