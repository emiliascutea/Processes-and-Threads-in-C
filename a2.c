#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>           
#include <sys/stat.h> 
#include "a2_helper.h"

sem_t semaphore1;
sem_t semaphore2;
sem_t semaphore3;
sem_t mutex;
sem_t barrier;
sem_t* sem1 = NULL;
sem_t* sem2 = NULL;

int count = 0;

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

bool thread1Found = false;
bool thread4Found = false;

void P7(){
    info(BEGIN, 7, 0);

    info(END, 7, 0);
}

void P6(){
    
    info(BEGIN, 6, 0);

    info(END, 6, 0);
}

void P4(){
    info(BEGIN, 4, 0);

    info(END, 4, 0);
}

void * thread_function5(void * index){
    int thread_no = *(int*)index + 1;

    if(thread_no == 4){ // thread T5.4 waits for thread T3.2 to end and only after that it begins
        sem_wait(sem2);
    }
    info(BEGIN, 5, thread_no);

    info(END, 5, thread_no);
    if(thread_no == 2){ /// thread T5.2 must end before T3.2 begins
        sem_post(sem1); // signal the end of thread T5.2
    }

    return 0;
}

void P5(){
    pid_t p5;


    info(BEGIN, 5, 0);

    sem1 = sem_open("sem1_2.5",O_CREAT,0777,0);
    sem2 = sem_open("sem2_2.5", O_CREAT, 0777, 0);
  
    if(sem1 == SEM_FAILED ||  sem2 == SEM_FAILED){
        printf("sem_open failed\n");
        return;
    }


    pthread_t * threads = (pthread_t*)malloc(sizeof(pthread_t)*4);
    int * id = (int*)malloc(sizeof(int)*4);
    for(int index = 0; index < 4; index++){
        id[index] = index;
        pthread_create(&threads[index],NULL, thread_function5, &id[index]);
    }  

    for(int index = 0; index < 4; index++){
        pthread_join(threads[index], NULL);
    }

    p5 = fork();

    if(p5 == 0){
        P6();
        exit(0);
    }
    
    waitpid(p5, NULL, 0);

    info(END, 5, 0);
}

void * thread_function3(void * index){
    int thread_no = *(int*)index + 1;

    if(thread_no == 2){ //// 2.5 !!! thread T3.2 waits for thread T5.2 to end and only after that it starts
        sem_wait(sem1);
    }

    pthread_mutex_lock(&lock1);
    while(thread_no != 1 && thread1Found == false){   //// waits for thread 1       
        pthread_cond_wait(&cond1,&lock1);    /// the other threads go in the waiting queue
    }     
    if(thread_no == 1){ // if thread 1
        thread1Found = true;
        info(BEGIN, 3, thread_no); // print its begin info
        pthread_cond_broadcast(&cond1); // take out all threads from the waiting queue
    }
    pthread_mutex_unlock(&lock1);

    
    pthread_mutex_lock(&lock1);
    while(thread_no != 4  && thread4Found == false){ ///// waits for thread 4
        pthread_cond_wait(&cond2,&lock1); // the other threads go in the waiting queue
    }
    if(thread_no == 4){ // if thread 4
        thread4Found = true;
        info(BEGIN, 3, thread_no); // print its begin info
        info(END, 3, thread_no); // print its end info
        pthread_cond_broadcast(&cond2); // take out all threads from the waiting queue
    }
    pthread_mutex_unlock(&lock1);
   
    if(thread_no == 1){ // print end info of thread 1
        info(END, 3, thread_no);       
    }
    else if(thread_no != 4){ // print begin and end info of the other threads except from thread 4
        
        info(BEGIN, 3, thread_no);
        info(END, 3, thread_no);
        if(thread_no == 2){ // 2.5 !! signal the end of thread T3.2
            sem_post(sem2);
        }
    }

    return 0;
}

void P3(){
    pid_t p3;

    info(BEGIN, 3, 0);    

    sem1 = sem_open("sem1_2.5",O_CREAT,0777,0);
    sem2 = sem_open("sem2_2.5", O_CREAT, 0777, 0);
  
    if(sem1 == SEM_FAILED ||  sem2 == SEM_FAILED){
        printf("sem_open failed\n");
        return;
    }

    pthread_t * threads = (pthread_t*)malloc(sizeof(pthread_t)*4);
    int * id = (int*)malloc(sizeof(int)*4);
    for(int index = 0; index < 4; index++){
        id[index] = index;
        pthread_create(&threads[index],NULL, thread_function3, &id[index]);
    }  

    for(int index = 0; index < 4; index++){
        pthread_join(threads[index], NULL);
    }

    p3 = fork();

    if(p3 == 0){
        P7();
        exit(0);
    }
    
    waitpid(p3, NULL, 0);
    
    info(END, 3, 0);
}

void * thread_function2(void * index){
    int thread_no = *(int*)index + 1;
    

    if(thread_no != 13){   
        sem_wait(&semaphore2);         
    }
   
    sem_wait(&semaphore1);    

    info(BEGIN, 2, thread_no);

    for(int index = 1; index <= 4; index++){
        sem_post(&semaphore2);
    }    

    sem_wait(&mutex);
    count++;
    sem_post(&mutex);   
        
    if(count == 4){          
        sem_post(&barrier);           
    }       

    sem_wait(&barrier);
    sem_post(&barrier);
            
    
    if(thread_no != 13){
        sem_wait(&semaphore3);
        info(END, 2, thread_no);
    }
    else{
        info(END, 2, thread_no);
        sem_post(&semaphore3);
    }
    sem_post(&semaphore3);     
    
    
    sem_post(&semaphore1);    

    return 0;
}

void P2(){
    pid_t p2;

    info(BEGIN, 2, 0);

    sem_init(&semaphore1, 0, 4);
    sem_init(&mutex, 0, 1);
    sem_init(&semaphore2, 0, 0);
    sem_init(&semaphore3, 0, 0);
    sem_init(&barrier, 0, 0);

    pthread_t * threads = (pthread_t*)malloc(sizeof(pthread_t)*38);
    int * id = (int*)malloc(sizeof(int)*38);
    for(int index = 0; index < 38; index++){
        id[index] = index;
        pthread_create(&threads[index],NULL, thread_function2, &id[index]);
    }  

    for(int index = 0; index < 38; index++){
        pthread_join(threads[index], NULL);
    }

    sem_close(&semaphore1);
    sem_close(&semaphore2);
    sem_close(&semaphore3);
    sem_close(&mutex);
    sem_close(&barrier);



    p2 = fork();
    if(p2 == 0){
        P3();
        exit(0);
    }

    

    p2 = fork();

    if(p2 == 0){
        P4();
        exit(0);
    }


    p2 = fork();

    if(p2 == 0){
        P5();
        exit(0);
    }

    // waitpid(p2, NULL, 0);
    // waitpid(p2, NULL, 0);
    // waitpid(p2, NULL, 0);
    wait(NULL);
    wait(NULL);
    wait(NULL);


    info(END, 2, 0);

}

int main(){
    
    init();
    
    sem_unlink("sem1_2.5");
    sem_unlink("sem2_2.5");

    pid_t p1;
    info(BEGIN, 1, 0);
    
    p1 = fork();
    if(p1 == 0 ){
        P2();
        exit(0);        
    }

    waitpid(p1, NULL, 0);        
    
    info(END, 1, 0);
    return 0;
}
