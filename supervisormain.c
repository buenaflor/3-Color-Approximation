/**
 * @file supervisor.c
 * @author Giancarlo Buenaflor <e51837398@tuwien.ac.at>
 * @date 18.11.2020
 *
 * @brief Main program module.
 * 
 * This program "supervisor" acts as the server. It will create and remove shared memories and semaphores
 * The supervisor reads from the circular buffer and print out the best solution for the 3-color problem so far
 * 
**/
 
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include "sharedmem.h"

/** Stores an atomic variable quit
 * @brief If quit is set to 1, it signals to terminate all associated processes
 */
 static volatile sig_atomic_t quit = 0;

/**
 * Handle signal
 * @brief This function handles the functionality when receiving a signal
 * @details Once a signal is received, quit is set to 1 and to signal termination
 * @param signal The type of signal received.
*/
static void handle_signal(int signal) { 
	quit = 1; 
}

/**
 * Initialize signal handling
 * @brief This function initializes the signal handling
 * @details Signal handling is delegated to the "handle_signal function"
*/
static void initializeSignalHandling() {
	struct sigaction sa = {
		.sa_handler = handle_signal
	};
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

/** Stores the semaphore indicating used spots in our circular buffer. 
 * @brief If used_sem is 0 then the generator won't be able to write onto the buffer
 */
static sem_t *used_sem;

/** Stores the semaphore indicating free spots in our circular buffer. 
 * @brief If free_sem is 0 then the generator won't be able to read from the buffer
 */
static sem_t *free_sem;

/** Stores the semaphore used for synchronizing mutual access to a space
 * @brief If two processes try to manipulate a mutual variable, this semaphore will synchronize a correct behaviour
 */
static sem_t *mutex_sem;

/**
 * Initialize semaphores function
 * @brief This function attempts to initialize semaphores used to synchronize between the generator and this supervisor
 * @details Three essential semaphores are initialized that are necessary for synchronizing the circular buffer read and write
*/
static void initializeSemaphores() {
	used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, 0600, 0);
    if (used_sem == SEM_FAILED) {
        printErrAndExit("USED_SEM failed creation");
    }

    free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, 0600, MAX_DATA);
    if (free_sem == SEM_FAILED) {
        printErrAndExit("FREE_SEM failed creation");
    }

	mutex_sem = sem_open(MUTEX_SEM, O_CREAT | O_EXCL, 0600, 1);
	if(mutex_sem == SEM_FAILED) {
        printErrAndExit("MUTEX_SEM failed creation");
	}
}

/** Stores the current read position in our buffer. 
 * @brief This variable is set inside the "readBuff(..)" function and represents the current read position
 */
static int rd_pos = 0;

/**
 * Read buffer function
 * @brief This function reads the circular buffer in our shared memory object.
 * @details Only reads if free_sem is > 0
 * @param edges_to_read A removedEdge array that represents the circular buffer.
 * @return Returns an integer referring to the amount of edges that have been removed.
*/
static int readBuff(removedEdge edges_to_read[]) {
	sem_wait(used_sem);
	int removed_edges_count = edges_to_read[rd_pos].numOfEdges;
	sem_post(free_sem);
	rd_pos += 1;
	rd_pos %= sizeof(edges_to_read);
	return removed_edges_count;
}

/**
 * Program entry point.
 * @brief The program starts here. The supervisor creates and manages the semaphores and shared memory object. 
 * @details If any creation, opening or closing fails, the program will immediately exit. 
 * The supervisor reads from the buffer the best solution so far and prints it out as long as a SIGNAL has come.
 * If a SIGINT or SIGTERM signal has come, the supervisor tells the generators to terminate.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
	pgm_name = argv[0];
 
	if(argc != 1) {
		printErrAndExit("Invalid arguments");
    }

	initializeSignalHandling();
    initializeSemaphores();

	int shmfd = createSHMFileDescriptor();
	myshm *myshm = createMappedSHMObject(shmfd);

	/* DONE SETTING UP SHARED MEMORY OBJECT */

	myshm->generator_count = 0;	
	int curr_best_solution = INT_MAX;

	while (!quit) {
		int temp = readBuff(myshm->removed_edges);
		if (temp == 0) {
			curr_best_solution = 0;
			break;
		}
		if (temp < curr_best_solution) {
			printf("[%s] Solution with %d edges:", pgm_name, temp);
			for (int j = 0; j < temp; j++) {
				int source = myshm->removed_edges[rd_pos].edges[j].source;
				int destination = myshm->removed_edges[rd_pos].edges[j].destination;
				printf(" %d - %d ", source, destination);
			}
			printf("\n");
			curr_best_solution = temp;
		}
	}

	sem_wait(mutex_sem);
	myshm->state = 1;
	sem_post(mutex_sem);
	
	for (int i = 0; i < myshm->generator_count; i++) {
		sem_post(free_sem);
	}
	
	printf("[%s] Best found solution: %d edges\n", pgm_name, curr_best_solution);

	if (curr_best_solution == 0) {
		printf("[%s] The graph is 3-colorable!\n", pgm_name);
	}


    /* CLOSE, UNLINK AND DEALLOCATE  */
	closeSemaphores(used_sem, free_sem, mutex_sem);
	unmapSHM(myshm);
	unlinkRessources();
	
    printf("[%s] Terminating...\n", pgm_name);

	return EXIT_SUCCESS;
}