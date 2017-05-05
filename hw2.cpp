//
//  hw2.cpp
//  cs42
//
//  Created by Alan Paul Paragas on 4/19/17.
//  Copyright © 2017 Alan Paul Paragas. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#define Maxcustomer 25

using namespace std;

sem_t waitingRoom;
sem_t barberchair;
sem_t buffer;

pthread_mutex_t waiter;
int customer_left=0;
bool customerleft=true;
bool awake_barber=false;

void *customer(void *arg){
    pthread_mutex_lock(&waiter);
    if(awake_barber==false){
        cout<<"Barber is asleep"<<endl;
        awake_barber=true;
    }
    pthread_mutex_unlock(&waiter);
    
    int customernum=*(int*) arg;
    
    //sparce out text delivery of customers arriving and leaving
    sem_wait(&buffer);
    cout<<"Customer "<< customernum<< " is leaving for barbershop"<<endl;
    sem_post(&buffer);
    sem_wait(&buffer);
    cout<<"Customer "<< customernum<< " has arrived at barbershop"<<endl;
     sem_post(&buffer);
    
    //wait for chairs in waiting room to be free
    sem_wait(&waitingRoom);
    sem_wait(&buffer);
    cout<<"Customer "<< customernum<< " entering waiting room"<<endl;
    sem_post(&buffer);
      //CRITICAL SECTION
    sem_wait(&barberchair);
    sem_post(&waitingRoom); //give up waiting chair
    //barber will keep giving haircuts if customer is waiting
    sem_wait(&buffer);
    cout<<"Customer "<< customernum<< " wakes up barber"<<endl;
    sem_post(&buffer);
    //wait for sem_t haircut given by barber
    sem_wait(&buffer);
    cout<<"Barber is giving a haircut "<<endl;
    sem_post(&buffer);
    sem_wait(&buffer);
    cout<<"Barber is finished with haircut "<<endl;
    sem_post(&buffer);
    sem_wait(&buffer);
    cout<<"Barber is sleeping again"<<endl;
   
    cout<<"Customer "<< customernum<< " leaving the barbershop"<<endl;
    sem_post(&buffer);
    //give back chair to allow other haircuts
    sem_post(&barberchair);
    //END CRITICAL SECTION
    sem_post(&buffer);
    customernum=NULL;
    delete (int*)customernum;
    return NULL;
}

int main(int argc, char *argv[]){
    
    pthread_t customer_thread[Maxcustomer];
    int customercount;
    int chairs;
    int *customerEntry;
    
    cout<<"Enter Number of Customers and Chairs , Cannot exceed 25"<<endl;
    
    cin>>customercount;cin>>chairs;
    
    if(customercount>Maxcustomer){
        cout<<"Number exceeds 25"<<endl;
        return 0;
    }
    if(chairs>Maxcustomer){
        cout<<"Number exceeds 25"<<endl;
        return 0;
    }
    sem_init(&waitingRoom,0,chairs);
    sem_init(&barberchair,0,1);
    sem_init(&buffer,0,1);
    pthread_mutex_init(&waiter,0);
    
    for(int k=0;k<customercount;k++){
        customerEntry= new int(k);
        if(pthread_create(&customer_thread[k],NULL,&customer,(void*)customerEntry)){
            cout<<"Could not create thread \n";
            return -1;
        }
    }
    customerEntry=0;
    delete customerEntry;
    for(int j=0;j<customercount;j++){
        if(pthread_join(customer_thread[j],NULL)){
            cout<<"Could not join thread \n";
            return -1;
        }
    }
    cout<<"The barber is going home"<<endl;
    pthread_mutex_destroy(&waiter);
    sem_destroy(&waitingRoom);
    sem_destroy(&barberchair);
    sem_destroy(&buffer);
    return 0;
}

