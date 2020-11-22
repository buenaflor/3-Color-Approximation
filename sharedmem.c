/**
 * @file sharedmem.c
 * @author Giancarlo Buenaflor <e51837398@tuwien.ac.at>
 * @date 18.11.2020
 * 
 * @brief Implementation of the sharedmem module.
 *
 **/

#include "sharedmem.h"

char *pgm_name;

void closeSemaphores(sem_t *used_sem, sem_t *free_sem, sem_t *mutex_sem) {
  if (sem_close(used_sem) == -1) {
		printErrAndExit("Closing used_sum failed");
	}
  if (sem_close(free_sem) == -1) {
		printErrAndExit("Closing free_sem failed");
	}
  if (sem_close(mutex_sem) == -1) {
		printErrAndExit("Closing mutex_sem failed");
	}
}

void unmapSHM(myshm *myshm) {
  if (munmap(myshm, sizeof(myshm)) == -1) {
		printErrAndExit("Unmapping SHM failed");
	}
}

void unlinkRessources() {
  if (sem_unlink(USED_SEM) == -1) {
		printErrAndExit("Unlinking USED_SEM failed");
	}
  if (sem_unlink(FREE_SEM) == -1) {
		printErrAndExit("Unlinking FREE_SEM failed");
	}
	if (sem_unlink(MUTEX_SEM) == -1) {
		printErrAndExit("Unlinking FREE_SEM failed");
	}
  if (shm_unlink(SHM_NAME) == -1) {
		printErrAndExit("Unlinking SHM object failed");
  }
}

myshm* createMappedSHMObject(int shmfd) {
  myshm *myshm = mmap(NULL, sizeof(myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (myshm == MAP_FAILED) {
		printErrAndExit("Mapping SHM failed");
	}
  if (close(shmfd) == -1) {
		printErrAndExit("Closing file descriptor failed");
	}
  return myshm;
}

int createSHMFileDescriptor() {
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
	if (shmfd == -1) { 
		printErrAndExit("SHM_NAME failed creation");
	}

	if (ftruncate(shmfd, sizeof(struct myshm)) < 0) {
		printErrAndExit("Truncate SHM failed");
	}
	return shmfd;
}

int openSHMFileDescriptor() {
  int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
	if (shmfd == -1) { 
		printErrAndExit("Couldn't open shmf");
	}
  return shmfd;
}

void randomizeColors(int numOfVertices, int *color_indices) {
  for (int v = 0; v < numOfVertices; v++) {
    int random_num = 1 + (rand() % 3);
    color_indices[v] = random_num;
  }
}

void solveColorProblem(int* color_indices, edge removed_edges[], int *removed_edges_count, edge edges[], int numOfEdges) {
  int e, rem_count = 0;
  for (e = 0; e < numOfEdges; e++) {
    int source = edges[e].source;
    int destination = edges[e].destination;
    int color1 = color_indices[source];
    int color2 = color_indices[destination];
    if (color1 == color2) {
      removed_edges[rem_count].destination = source;
      removed_edges[rem_count].source = destination;
      rem_count += 1;
    }
  }
  *removed_edges_count = rem_count;
}
  
void printGraph(edge edges[], int numOfEdges) {
  for (int e = 0; e < numOfEdges; e++) {
    int source = edges[e].source;
    int destination = edges[e].destination;
    printf("%d - %d\n", source, destination);
  }
}

void printErrAndExit(char* strerr) {
	(void) fprintf(stderr, "[%s]: %s\n", pgm_name, strerr);
	exit(EXIT_FAILURE);
}
