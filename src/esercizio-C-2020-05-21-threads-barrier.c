/*
 * esercizio-C-2020-05-21-threads-barrier.c
 *
 *  Created on: May 19, 2020
 *      Author: marco
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define LOWER 1
#define UPPER 3

#define N 10

pthread_barrier_t thread_barrier;

int number_of_threads = N;

void* first_phase(void *arg);
void second_phase(int *fd, long int id);

/*****************************************************************************************/

int main() {
	char *fileName = "prova.txt";
	printf("scrivo nel file %s\n", fileName);

	int fd = open(fileName,
	O_CREAT | O_TRUNC | O_WRONLY,
	S_IRUSR | S_IWUSR);

	int saved_stdout = dup(STDOUT_FILENO);

	if (fd == -1) {
		perror("errore di apertura del file!!");
		exit(EXIT_FAILURE);
	}


	if (dup2(fd, STDOUT_FILENO) == -1) {
		perror("problema con dup2");
		exit(EXIT_FAILURE);
	}

	close(fd);

	int s;
	pthread_t threads[number_of_threads];

	// https://linux.die.net/man/3/pthread_barrier_init
	s = pthread_barrier_init(&thread_barrier, NULL, N);
	if (s == -1) {
		perror("pthread_init");
		exit(EXIT_FAILURE);
	}

	//Lancio N thread che eseguono la fase 1 con la barriera
	for (int i = 0; i < number_of_threads; i++) {
		s = pthread_create(&threads[i], NULL, first_phase, &fd);

		if (s != 0) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}
	//Attendo che i vari thread terminino il loro compito
	for (int i = 0; i < number_of_threads; i++) {
		s = pthread_join(threads[i], NULL);

		if (s != 0) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}

	}
	//close(fd);
	// https://linux.die.net/man/3/pthread_barrier_init
	s = pthread_barrier_destroy(&thread_barrier);
	if (s == -1) {
		perror("pthread_destroy");
		exit(EXIT_FAILURE);
	}



	if (dup2(saved_stdout, STDOUT_FILENO) == -1) {
			perror("problema con dup2");
			exit(EXIT_FAILURE);
		}
	close(saved_stdout);

	printf("Scrittura completata sul file %s",fileName);

	return 0;
}

/*****************************************************************************/
void* first_phase(void *arg) {

	int s;
	int *fd = (int*) arg;
	int random_number;
	random_number = rand() % UPPER + LOWER;
	long int thread_id = pthread_self();

	//printf("Dormo per %d secondi", random_number);
	printf("FASE 1, thread id = %ld, sleep period = %d s \n", thread_id,
			random_number);

	s = pthread_barrier_wait(&thread_barrier);
	/*
	 The pthread_barrier_wait() function shall synchronize participating threads at
	 the barrier referenced by barrier. The calling thread shall block until
	 the required number of threads have called pthread_barrier_wait() specifying the barrier.
	 */

	//fase critica
	second_phase(fd, thread_id);

	return NULL;
}

void second_phase(int *fd, long int id) {

	struct timespec t;

	struct timespec remaining;

	t.tv_sec = 0;  // seconds
	t.tv_nsec = 100; // nanoseconds

	printf("FASE 2, thread id = %ld dopo la barriera\n", id);
	if (nanosleep(&t, &remaining) == -1) {
		perror("nanosleep");
	}
	printf("thread id = %ld BYE\n", id);

	return;
}

