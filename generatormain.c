/**
 * @file generatormain.c
 * @author Giancarlo Buenaflor <e51837398@tuwien.ac.at>
 * @date 18.11.2020
 *
 * @brief Main program module.
 * 
 * This program "generator" acts as the client. It will randomize color for a parsed graph through the arguments and create solution for the 3-color problem
 * It will write to the circular buffer and semaphores act as a synchronizer so multiple generator don't write to the same space.
 * 
**/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include "sharedmem.h"

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

/** Stores the current write position in our buffer. 
 * @brief This variable is set inside the "writeBuff(..)" function and represents the current write position
 */
static int wr_pos = 0;

/**
 * Read buffer function
 * @brief This function reads the circular buffer in our shared memory object.
 * @details Only reads if free_sem is > 0
 * @param val A removedEdge array that represents the circular buffer.
 * @param removed_edges A edge array that represents the arrays that should be removed to make the graph 3-colorable.
 * @param edges_to_write A removedEdge array that represents the circular buffer.
*/
static void writeBuff(int val, edge removed_edges[], removedEdge edges_to_write[]) {
  sem_wait(free_sem);
  edges_to_write[wr_pos].numOfEdges = val;
  for (int j = 0; j < val; j++) {
    edges_to_write[wr_pos].edges[j].source = removed_edges[j].source;
    edges_to_write[wr_pos].edges[j].destination = removed_edges[j].destination;
  }
  sem_post(used_sem);
  wr_pos += 1;
  wr_pos %= sizeof(edges_to_write);
}

/**
 * Initialize semaphores function
 * @brief This function attempts to initialize semaphores used to synchronize between this generator and the supervisor
 * @details Three essential semaphores are initialized that are necessary for synchronizing the circular buffer read and write
*/
static void initializeSemaphores() {
  used_sem = sem_open(USED_SEM, 0);
  if(used_sem == SEM_FAILED) {
    printErrAndExit("USED_SEM failed creation");
	}

  free_sem = sem_open(FREE_SEM, 0);
  if(free_sem == SEM_FAILED) {
    printErrAndExit("FREE_SEM failed creation");
	}

  mutex_sem = sem_open(MUTEX_SEM, 0);
  if(mutex_sem == SEM_FAILED) {
    printErrAndExit("MUTEX_SEM failed creation");
	}
}

/**
 * Program entry point.
 * @brief The program edge1s here. This function takes care about parameters, and if the
 * overall program is reasonable trivial it could implement the whole program functionality -
 * like here. On topic (you should not state such general (coding style) information in your 
 * documentation): This function writes a few lines of text including "Hello World" to stdout. 
 * @details Try to keep the main function small and move any functionality that is used more 
 * than  a single time to extra function(s). Note that you should restrict visibility of those
 * extra functions to the smallest required scope (edge1 with static).
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
	pgm_name = argv[0];

  printf("[%s] Starting generator...\n", pgm_name);

  int i, max = -1;

  // Seeding the random variable
  srand (time(NULL));
  
  int edge1 = 0;  
  int edge2 = 0;
  for (i = 1; i < argc; i++) {
    if (sscanf(argv[i], "%d-%d", &edge1, &edge2) != 2) {
      printErrAndExit("Couldn't parse all edges");
    }
    if (edge1 > edge2) {
      max = edge1;
    } else {
      max = edge2;
    }
  }

  int numOfVertices = max + 1, numOfEdges = argc - 1;
  int color_indices[numOfVertices];
  edge edges[numOfEdges];

  for (i = 1; i < argc; i++) {
    if (sscanf(argv[i], "%d-%d", &edge1, &edge2) != 2) {
      printErrAndExit("Couldn't parse all edges");
    }
    edges[i - 1].source = edge1;
    edges[i - 1].destination = edge2;
  }

  randomizeColors(numOfVertices, color_indices);

  // Worst case, all edges are removed so we allocate numOfEdges
  edge removed_edges[numOfEdges];
  int removed_edges_count = 0;
  solveColorProblem(color_indices, removed_edges, &removed_edges_count, edges, numOfEdges);

  int shmfd = openSHMFileDescriptor();
	myshm *myshm = createMappedSHMObject(shmfd);
  initializeSemaphores();

	/* DONE SETTING UP SHARED MEMORY OBJECT AND SEMAPHORES */

  sem_post(mutex_sem);
  myshm->generator_count += 1;
  sem_wait(mutex_sem);
  
  // Only write to the buffer, if the removed edges are less than MAX_SOLUTION_EDGES
  // Big solutions are not wanted
  while(myshm->state != 1) {
    if (removed_edges_count <= MAX_SOLUTION_EDGES) {
      sem_post(mutex_sem);
      writeBuff(removed_edges_count, removed_edges, myshm->removed_edges);
      sem_wait(mutex_sem);
    }
    randomizeColors(numOfVertices, color_indices);
    removed_edges_count = 0;
    solveColorProblem(color_indices, removed_edges, &removed_edges_count, edges, numOfEdges);
  }

  /* CLOSE SEMAPHORES AND UNMAP */
  printf("[%s] Terminating...\n", pgm_name);

  unmapSHM(myshm);
  closeSemaphores(used_sem, free_sem, mutex_sem);

	return EXIT_SUCCESS;
} 