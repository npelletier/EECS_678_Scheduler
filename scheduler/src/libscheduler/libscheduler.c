/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _job_t
{
  int job_id, response_time, arrival_time, original_run_time, run_time, priority, start_time, core_id;
} job_t;

int last_time_checked_PSJF;
priqueue_t queue;

/*                                          **
**------------COMPARISON FUNCTIONS----------**
**                                          */
int FCFS_comp(const void* left,const void* right)
{
  return -1;
}
int SJF_comp(const void* left, const void* right)
{
  job_t* left_job = (job_t*)left;
  job_t* right_job = (job_t*)right;
  int l = left_job->run_time;
  int r = right_job->run_time;
  if(l == r) //same run time
  {
    //use FCFS
    return -1;
  }
  return l-r;//not equal
}
int PRI_comp(const void* left, const void* right)
{
  job_t* left_job = (job_t*)left;
  job_t* right_job = (job_t*)right;
  int l = left_job->priority;
  int r = right_job->priority;
  if(l == r)
  {
    //use FCFS
    return -1;
  }else
  {
    return l - r;
  }
}
//end comparers
/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  /*                               **
  *---INITIALIZE GLOBAL VARIABLES---*
  **                               */
  m_waiting_time = 0.0;
  m_turnaround_time = 0.0;
  m_response_time = 0.0;
  num_jobs = 0;

  num_cores = cores;
  //this array will be filled with 0 for a free core, 1 for a busy core
  avail_cores = malloc((sizeof(int)) * cores);
  for(int i = 0; i < num_cores; i++)
  {
    avail_cores[i] = 0;
  }
  //set comparison scheme
  scheduling_scheme = scheme;

  switch(scheduling_scheme)
  {
    case FCFS:
    {
      priqueue_init(&queue,&FCFS_comp);

      break;
    }
    case SJF:
    {
      priqueue_init(&queue,SJF_comp);
      break;
    }
    case PSJF:
    {
      priqueue_init(&queue,SJF_comp);
      break;
    }
    case PRI:
    {
      priqueue_init(&queue,PRI_comp);
      break;
    }
    case PPRI:
    {
      priqueue_init(&queue,PRI_comp);
      break;
    }
    case RR:
    {
      priqueue_init(&queue,FCFS_comp);
      break;
    }
    default:
    {
      printf("something happened\n");
      break;
    }
  }
}


/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{

  job_t* to_add = malloc(sizeof(job_t));
  to_add->job_id = job_number;
  to_add->original_run_time = running_time;
  to_add->run_time = running_time;
  to_add->arrival_time = time;
  to_add->priority = priority;
  to_add->start_time = -1;
  to_add->core_id = -1;
  to_add->response_time = 0;
  //find the first available core
  int to_return = -1;
  int i = 0;
  for(;i < num_cores; i++)
  {
    if(avail_cores[i] == 0)
    {
      to_return = i;
      break;
    }
  }
  //mark the chosen core as in use
  if(i < num_cores)
  {
    avail_cores[i] = 1;
    if(scheduling_scheme == PSJF)
    {
      last_time_checked_PSJF = time;
    }
  }else
  //i == num_cores => there is no free core, we need to check for preemption
  {
    switch(scheduling_scheme)
    {
      case PSJF:
      {
        //we need to know how long since we added the last job
        int time_diff = time - last_time_checked_PSJF;
        //need to find the longest remaining time of jobs on cores
        int longest_run_time = -1;
        //this is the core to be run on
        //also the index of the job in the queue
        int core_of_longest_run_time = -1;
        job_t* curr_check;
        int index;
        //update run times
        for(int p = 0; p < priqueue_size(&queue);p++)
        {
          curr_check = (job_t*)priqueue_at(&queue,p);
          if(curr_check->core_id != -1)
          {
            if(!(curr_check->start_time == time))
            {
              curr_check->run_time -= time_diff;
            }
          }
        }
        int j;
        for(j = 0; j < num_cores; j++)
        {
          curr_check = (job_t*)priqueue_at(&queue,j);

          if(curr_check->run_time > longest_run_time)
          {
            longest_run_time = curr_check->run_time;
            core_of_longest_run_time = curr_check->core_id;
            index = j;
          }
        }
        if(running_time < longest_run_time)
        {
          to_return = core_of_longest_run_time;
          curr_check = (job_t*)priqueue_at(&queue,index);
          curr_check->core_id = -1;
          if(curr_check->start_time == time)
          {
            curr_check->start_time = -1;
            m_response_time -= (time - curr_check->arrival_time);
          }else
          {
            curr_check->start_time = time;
          }
        }
        last_time_checked_PSJF = time;
        break;
      }
      case PPRI:
      {
        int j;
        job_t* curr_check = (job_t*)priqueue_peek(&queue);
        int lowest_priority = curr_check->priority;
        int core_of_lowest_priority = curr_check->core_id;
        for(j = 0; j < num_cores; j++)
        {
          curr_check = (job_t*) priqueue_at(&queue,j);
          if (curr_check->priority > lowest_priority)
          {
            lowest_priority = curr_check->priority;
            core_of_lowest_priority = curr_check->core_id;
          }
        }
        if(priority < lowest_priority)
        {
          core_of_lowest_priority = curr_check->core_id;
          to_return = core_of_lowest_priority;
          curr_check->core_id = -1;
          curr_check->start_time = time;
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }
  //update the core id of the new job
  to_add->core_id = to_return;
  //if the job is assigned to a core, set its start time to be current time
  if(to_return != -1)
  {
    to_add->start_time = time;
  }
  //add to queue
  num_jobs++;
  priqueue_offer(&queue,to_add);
	return to_return;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
  int i = 0, j = 0;
  // int removal_index = -1;
  int return_job_id = -1;
  job_t* temp;

  avail_cores[core_id] = 0;
  for(;i < priqueue_size(&queue);i++)
  {
    temp = priqueue_at(&queue,i);
    if(temp->core_id == core_id)
    {
      m_turnaround_time = m_turnaround_time + time - temp->arrival_time;
      priqueue_remove_at(&queue,i);
      for(;j<priqueue_size(&queue);j++)
      {
        temp = priqueue_at(&queue,j);
        if(temp->core_id == -1)
        {
          avail_cores[core_id] = 1;
          temp->core_id = core_id;
          if(temp->start_time==-1)
          {
            m_waiting_time = m_waiting_time + time - temp->arrival_time;
            m_response_time = m_response_time + time - temp->arrival_time;
          }
          else
          {
            m_waiting_time = m_waiting_time + time - temp->start_time;
          }
          //here, update waiting time somehow temp->
          return_job_id = temp->job_id;
          temp->start_time=time;
          return return_job_id;
        }
      }
    }
  }
	return return_job_id;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  job_t* temp;
  for(int i = 0; i<priqueue_size(&queue); i++)
  {
    temp = priqueue_at(&queue, i);
    if(temp->core_id == core_id)
    {
      priqueue_remove_at(&queue, i);
      temp->core_id=-1;
      temp->start_time = time;
      priqueue_offer(&queue, temp);
      for(int j = 0; j<priqueue_size(&queue); j++)
      {
        temp = priqueue_at(&queue, j);
        if(temp->core_id==-1)
        {
          if(temp->start_time==-1)
          {
            m_waiting_time = m_waiting_time + time - temp->arrival_time;
            m_response_time = m_response_time + time - temp->arrival_time;
          }
          else
          {
            m_waiting_time = m_waiting_time + time - temp->start_time;
          }
          temp->start_time=time;
          temp->core_id=core_id;
          return temp->job_id;
        }
      }
    }
  }
  avail_cores[core_id]=0;
  return -1;

}

/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  //total waiting
  return m_waiting_time/num_jobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
  //arrived to finished
  return m_turnaround_time/num_jobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
  //arrived to started
  return m_response_time/num_jobs;
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  job_t* temp;
  for(int i = 0; i < priqueue_size(&queue);i++)
  {
    temp = (job_t*)priqueue_at(&queue,i);
    printf("%d(%d) ",temp->job_id,temp->priority);
  }
}
