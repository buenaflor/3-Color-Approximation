/**
 * @file sharedmem.h
 * @author Giancarlo Buenaflor <e51837398@tuwien.ac.at>
 * @date 18.11.2020
 *  
 * @brief Provides shared memory, semaphore functions and the circular buffer structure including the algorithm for solving the 3-colorable problem.
 *
 * The sharedmem module. It contains functions for deallocating ressources associated with semaphores and shared memory. The circular buffer is also
 * structured inside this module. The names for the shared memory object and the sizes for the circular buffer are also defined here.
 */

#include <semaphore.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SHM_NAME "/51837398_myshm_gb"
#define USED_SEM "/51837398_used_sem"
#define FREE_SEM "/51837398_free_sem"
#define MUTEX_SEM "/51837398_mutex_sem"
#define MAX_DATA (128)
#define MAX_SOLUTION_EDGES (12)

/** Represents the edge structure
 * @brief The source and destination represent the nodes
 */
typedef struct edge {
  int source;
  int destination;
} edge;

/** Represents the removed edge structure
 * @brief The removedEdge struct includes the number of edges and an array of edges that have been removed to make the graph 3-colorable
 */
typedef struct removedEdge {
    int numOfEdges;
    edge edges[MAX_SOLUTION_EDGES];
} removedEdge;

/** Represents the mapping for the shared memory object
 * @brief The state will indicate if the program will terminate or not. (state == 1 means termination)
 * The generator count keeps track of the number of generator processes that are running
 * removedEdge represents the circular buffer with an array of edges in each cell
 */
typedef struct myshm {
    int state;
	int generator_count;
	removedEdge removed_edges[MAX_DATA];
} myshm;

/** Stores the program name
 * @brief The program name is specified by argv[0] at the start of main
 */
extern char *pgm_name;

/**
 * Closes semaphores
 * @brief This function attempts to close any ressources of semaphore
 * @details If any attempt of closing fails, the function prints an error and exits
 * @param myshm The type of signal received.
*/
void closeSemaphores(sem_t *used_sem, sem_t *free_sem, sem_t *mutex_sem);

/**
 * Unmaps the mapped shared memory object
 * @brief This function attempts to unmap the shared memory object
 * @details If any attempt of unmapping fails, the function prints an error and exits
 * @param myshm The mapped shared memory object
*/
void unmapSHM(myshm *myshm);

/**
 * Unlinks any ressource 
 * @brief This function attempts to unlink any ressources of shared memory and semaphore
 * @details If any attempt of unlinking fails, the function prints an error and exits
*/
void unlinkRessources();

/**
 * Create the shared memory mapped object
 * @brief This function attempts to create the shared memory mapped object
 * @details If the attempt fails, or the file descriptor cannot be closed, the program prints an error and exits immediately
 * @return Returns a mapped object of type myshm*.
*/
myshm* createMappedSHMObject(int shmfd);

/**
 * Create shared memory file descriptor
 * @brief This function attempts to create a file descriptor
 * @details If creation fails the function will print an error and exit immediately
 * @return Returns a file descriptor (nonnegative integer).
*/
int createSHMFileDescriptor();

/**
 * Open a file descriptor of the shared memory object
 * @brief This function attempts to open a file descriptor of the shared memory object
 * @details If the attempt fails, the program prints an error and exits
 * @return Returns a file descriptor (nonnegative integer).
*/
int openSHMFileDescriptor();

/**
 * Randomizes the colors for the 3-colorable algorithm
 * @brief Each color_indices cell will be assigned a random color
 * @details Each cell in color_indices array represents a color (integer from 1 inclusive to 3 inclusive) which will be randomly created
 * @param numOfVertices The number of vertices
 * @param color_indices The color_indices int array
*/
void randomizeColors(int numOfVertices, int *color_indices);

/**
 * Algorithm for the 3-color problem 
 * @brief This function solves the 3-color problem
 * @details The solutions will be available in removed_edges and removed_edges_count
 * If the vertices of an edge have the same color, this edge will be removed
 * The removal doesn't actually happen, this function only marks the edges that would be removed and stores them in removed_edges
 * @param color_indices The color_indices array 
 * @param removed_edges The removed_edges array that will be filled
 * @param removed_edges_count The count for removed_edges (pointer, will be set inside the function)
 * @param edges The edges array that is going to be checked
 * @param numOfEdges The number of edges
*/
void solveColorProblem(int* color_indices, edge removed_edges[], int *removed_edges_count, edge edges[], int numOfEdges);

/**
 * Print edges
 * @brief This function prints the edges onto the console
 * @details Prints nothing if there are no edges
 * @param edges The edges
 * @param edges The number of edges
*/
void printGraph(edge edges[], int numOfEdges);

/**
 * Print error function
 * @brief This function writes error information about the program to stderr and exits.
 * @details global variables: pgm_name
 * @param strerr This string is printed to stdout.
*/
void printErrAndExit(char* strerr);
