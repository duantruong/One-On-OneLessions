/*
 ============================================================================
 Name        : hw4.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
/* the maximum time (in seconds) to rest */
#define MAX_REST_TIME 3
/* the maximum time (in seconds) of a session */
#define MAX_SESSION_TIME 4
/* number of students */
#define NUM_OF_STUDENTS 4
/* number of instructors */
#define NUM_OF_INSTRUCTORS 2
/* # of sessions each student must take before exit */
#define NUM_OF_SESSIONS 2
/* binary semaphores */
sem_t mutex;
	/* # of waiting instructors, students in the queue */
int waiting_instructors, waiting_students;
	/* guarantee execution sequence - queue for instructors, students */
sem_t instructors_q, students_q;
	/* guarantee execution sequence - session over? */
sem_t session_over;
typedef struct{
	int *seed;
	int index;
	int session;
}studentStruct;
typedef struct{
	int *seed;
	int index;
	int session;
}instructorStruct;

void* student(void *a){

	//printf("student");
	studentStruct *para = (studentStruct*) a;
	int time = (rand_r(para->seed) % MAX_REST_TIME) +1;
	while (para->session !=2){
		sem_wait(&mutex); // lock mutex
		++waiting_students; //add waiting student
		sem_post(&mutex); // unlock mutex
		sleep(time); // enter lounge
		printf("student[%d, %d]: rest for %d seconds\n",para->index, para->session, time);
		fflush(NULL);
		while (waiting_instructors == 0){
			printf("student[%d,%d]: waiting_instructor=%d, wait_students (excluding me) = %d\n",para->index,para->session,0,waiting_students-1);
			fflush(NULL);
			sem_wait(&students_q);
		}

		if (waiting_instructors != 0){
			sem_post(&instructors_q); // signal instructors_q to say hey i'm ready
			sem_wait(&session_over); // wait for the signal from session_over to say hey the session is over
			++para->session; // increase session variable
			printf("student[%d, %d]: learn to skydive\n", para->index, para->session);
			fflush(NULL);
		}

	}

	sem_wait(&mutex); // unlock mutex
	--waiting_students;
	sem_post(&mutex); // lock mutex
	printf("student[%d, %d]: done\n",para->index,para->session);
	fflush(NULL);
}

void* instructor(void *b){
	//printf("instructor");
	instructorStruct *para = (instructorStruct*) b;
	int time = (rand_r(para->seed) % MAX_REST_TIME) +1;
	while (1){
		sem_wait(&mutex); //lock mutex
		++waiting_instructors; //increase waiting_instructor
		sem_post(&mutex); //unlock mutex
		sleep(time); //enter lounge
		printf("instructor[%d, %d]: rest for %d seconds\n",para->index, para->session, time);
		fflush(NULL);
		while (waiting_students == 0){
			sem_wait(&instructors_q);// wait if any student call sem_post(&instructors_q
		}
		if (waiting_students != 0){
			sem_wait(&mutex); //lock mutex
			--waiting_instructors;
			sem_post(&mutex); //unlock mutex
			printf("instructor[%d, %d]: waiting_instructors (excluding me) = %d, wait_students=%d\n",para->index, para->session, 0, waiting_students);
			fflush(NULL);
			sem_post(&students_q); //signal to (&students_q) in student method
			sleep(time); //teaching session
			sem_post(&session_over); //signal to (&session_over) in student method
			++para->session;
			printf("instructor[%d, %d]: teach a session for %d seconds\n",para->index, para->session, time);
			fflush(NULL);

		}

	}

}


int main(void) {
	//printf("a");
	printf("CS149 Fall 2021 Skydiving School from Duan Truong\n");

	pthread_t stu[NUM_OF_STUDENTS];//array pthread students

	pthread_t instruct[NUM_OF_INSTRUCTORS];//array pthread instructor

	studentStruct sStruct[NUM_OF_STUDENTS];//struct studentParameter

	instructorStruct iStruct[NUM_OF_INSTRUCTORS];//struct instructorParameter
	sem_init(&mutex,0,1);
	//printf("1");
	unsigned int s = time(NULL);
	for (int i=0; i < NUM_OF_STUDENTS; i++){
		sStruct[i].index=i;
		sStruct[i].seed = &s;
		sStruct[i].session = 0;
		pthread_create(&stu[i],NULL,student,(void*)&sStruct[i]);

		//printf("2");

	}
	//array instructor parameeter
	for (int i=0; i < NUM_OF_INSTRUCTORS; i++){
		iStruct[i].index=i;
		iStruct[i].seed= &s;
		iStruct[i].session=0;
		pthread_create(&instruct[i], NULL, instructor, (void*)&iStruct[i]);

		//printf("3");

	}
	//printf("4");
	for (int j=0; j< NUM_OF_STUDENTS; j++){
		printf("9");
		int e =pthread_join(stu[j],NULL);
		if (e!=0){
			printf("Join error");
		}
	}
	for (int i=0; i<NUM_OF_INSTRUCTORS; i++){
		pthread_cancel(instruct[i]);
	}


	return EXIT_SUCCESS;
}
