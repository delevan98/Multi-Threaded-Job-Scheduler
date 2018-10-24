# Multi-Threaded-Job-Scheduler

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
