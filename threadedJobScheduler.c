/* Program:     Job Scheduler - Multi-Threaded
   Author:      Mike Delevan
   Date:        October 24, 2018
   File name:   asgn5-delevanm2.c
   Compile:     cc -lpthread -o asgn5-delevanm2 asgn5-delevanm2.c
   Run:         ./asgn5-delevanm2

   This program runs on the freeBSD platform to accept jobs and execute them at a specified time. A user
   can specify if they want to add a job (add), remove a job (rm), or list all of the jobs in the queue (list).
   
   add:   adds a given job to a list, sorted by start time
   list:  lists all the jobs in the queue
   rm:    removes a job at a specified index
   
   Uses POSIX threads to create a scheduler, dispatcher, and executer threads so that jobs do not have to
   wait for another job to be completed and to allow more jobs to be added while another may be executing.
   
   scheduler:   accepts user input for new jobs
   dispatcher:  dispatches jobs to an executer thread when the job is ready to be executed
   executer:    starts the job as a new process and waits until it finishes
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct JOB {
   int startTime;       
   int numOfParams;      
   char job[5][25];     
   time_t submissionTime;    
   struct JOB *next;
} Node;

typedef struct HEADER
{  
   Node *headPtr;
   int count;
} Header;

void initializeHeader(Header *headerPtr);
void insert(Header *headerPtr, Node *newNodePtr);
void printNode(Node *node);
void list(Header *headerPtr);
Node* removeNode(Header *headerPtr, int index);
void getNodeInfo(Node *nodePtr);
void *dispatcher(void *ptr);
void *scheduler(void *ptr);
void *executer(void *ptr);

int main(int argc, char *argv[])
{ 
   pthread_t schedulerThread, dispatcherThread;
   
   Header *headerPtr = (Header*) malloc(sizeof(Header));
   initializeHeader(headerPtr);

   pthread_create(&schedulerThread, NULL, scheduler, (void*) headerPtr);
   pthread_create(&dispatcherThread, NULL, dispatcher, (void*) headerPtr);   
   
   pthread_join(schedulerThread, NULL);
   pthread_join(dispatcherThread, NULL);
   
   return 0;
}
   
void initializeHeader(Header *headerPtr) 
{
   headerPtr->headPtr = NULL;
   headerPtr->count = 0;
   return;
}
 
void getNodeInfo(Node *nodePtr)
{
   int i;
   nodePtr->next = NULL;
   nodePtr->submissionTime = time(NULL);
   scanf("%d%d", &(nodePtr->startTime), &(nodePtr->numOfParams)); 
   for(i=0; i < nodePtr->numOfParams; i++)
   {
      scanf("%s", nodePtr->job[i]);
   }
   return;
}

      
void insert(Header *headerPtr, Node *nodePtr)
{
   Node *tempPtr = headerPtr->headPtr;
   Node *prevPtr = NULL;
   if (tempPtr == NULL) 
   {
      headerPtr->headPtr = nodePtr;
      nodePtr->next = NULL;
      headerPtr->count = headerPtr->count + 1;
      return;
   }
      
   if(nodePtr->startTime < tempPtr->startTime)
   {
      nodePtr->next = tempPtr;
      headerPtr->headPtr = nodePtr;
   }
      
   else
   {
      while(tempPtr != NULL)
      {
         if(nodePtr->startTime >= tempPtr->startTime)
         {
            prevPtr = tempPtr;
            tempPtr = tempPtr->next;
         }
            
         else
         {
            prevPtr->next = nodePtr;
            nodePtr->next = tempPtr;
            headerPtr->count = headerPtr->count + 1;
            return;
         }
       }          
       prevPtr->next = nodePtr;
    }
    headerPtr->count = headerPtr->count + 1;
    return;
}

void printNode(Node *node)
{
   int i;
   printf("%d, %d, ", node->startTime, node->numOfParams);
   for(i=0;i<(node->numOfParams);i++)
   {
      printf("%s, ", node->job[i]);
   }         
   printf("%ld\n", (long) node->submissionTime);
}

void list(Header *headerPtr) 
{
   int i;
   printf("\nLIST:\n");
   if (headerPtr->count != 0) 
   {
      Node *tempPtr = headerPtr->headPtr;
      for (i = 0; i < headerPtr->count; i++) 
      {
         printNode(tempPtr);
         tempPtr = tempPtr->next;         
      }
      printf("\n");
   }
   else 
   {
      printf("EMPTY\n");
   }
   return;
} 

Node* removeNode(Header *headerPtr, int index)
{
   int i;
   Node *tempPtr = headerPtr->headPtr;
   Node *next;
   Node *deleted;
   
   if(headerPtr->headPtr == NULL)
   {
      printf("The list is empty.\n");    
      return NULL;
   }
   
   else if(index < 0 || index >= headerPtr->count)
   {
      printf("Invalid Index.");    
      return NULL;
   }
   
   else if(index == 0)
   {
      headerPtr->headPtr = tempPtr->next;
      printf("DELETE(%d): ", index);
      printNode(tempPtr);
      headerPtr->count = headerPtr->count - 1;
      return tempPtr;
   }
   
   else
   { 
      for(i = 0; tempPtr != NULL && i<(index-1); i++)
      {
         tempPtr = tempPtr->next; /*This is the previous node to the one to be deleted*/
      }
      
      next = tempPtr->next->next; /*Node after one to be deleted*/
      deleted = tempPtr->next; /*Node to be deleted*/
      
      tempPtr->next = next;
      
      printf("DELETE(%d): ", index);
      printNode(deleted); 
      headerPtr->count = headerPtr->count - 1;
      return deleted;
    }   
    return NULL;  
}  

void *dispatcher(void *ptr)
{
   Header *headerPtr = (Header*)ptr;
   time_t app_start_time = time(NULL);
   time_t app_current_time = 0;
   Node *currentJob;
   pthread_t executerThread;
   bool run = true;
   
   while(run)
   {      
      if((headerPtr->count != 0) && (app_current_time >= headerPtr->headPtr->startTime))
      {
         printf("\nAPPLICATIONS CURRENT TIME: %ld\n", (long) app_current_time);
         currentJob = removeNode(headerPtr,0);     
         pthread_create(&executerThread, NULL, executer, (void*) currentJob);          
      }
      
      else
      {
         sleep(1);
      }     
      
      app_current_time = time(NULL) - app_start_time;
   }
   
   return NULL;
}  

void *scheduler(void *ptr)
{
   Header *headerPtr = (Header*)ptr;
   char answer[10];
   Node *nodePtr = NULL;
   int i;
   int index;
   bool run = true;

   while(run)
   {
      scanf("%s", answer);
      
      if(strcmp(answer, "add") == 0)
      {
         nodePtr = (Node *) malloc(sizeof(Node));
         getNodeInfo(nodePtr);
         printf("ADD: ");
         printNode(nodePtr);     
         insert(headerPtr, nodePtr);
      }
      
      else if(strcmp(answer, "list") == 0)
      {
         list(headerPtr);
      }
      
      else if(strcmp(answer, "rm") == 0)
      {
         scanf("%d", &index);
         free(removeNode(headerPtr, index));
      }
   }
   
   return NULL;
}

void *executer(void *ptr)
{   
   int child_pid;
   int status = 999;
   char *cmdString;
   int i,j;
   
   Node *currentJob = (Node*)ptr;
   
   if((child_pid=fork()) != 0)
   {
      waitpid(child_pid, &status, WEXITED);
   }
         
   else
   {
       strcpy(cmdString, currentJob->job[0]);
       char *argv[currentJob->numOfParams+1];
       for(i = 0; i < currentJob->numOfParams; i++)
       { 
           argv[i] = currentJob->job[i];
       }
       argv[currentJob->numOfParams] = NULL;
           
       execvp(cmdString, argv);
       printf("\nJOB FAILED TO EXECUTE, EXITING NOW\n");
       pthread_exit(NULL);
    }
    printf("\nJOB COMPLETED: ");
    printNode(currentJob);
    free(currentJob);    
    pthread_exit(NULL);
         
}
