// C program for implementation of Simulation 
#include<stdio.h> 
#include<limits.h>
#include<stdlib.h>
#include<string.h>
#include "process.h"
#include "util.h"


// ---------- Round Robin (works with arrival==0 or real arrivals) ----------
void findWaitingTimeRR(ProcessType plist[], int n,int quantum) 
{ 
    int *rem_bt = (int*)malloc(sizeof(int)*n);
    if (!rem_bt) { perror("malloc"); exit(1); }

    // init remaining times and waiting times
    for (int i=0; i<n; ++i) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int t = 0;                 // current time
    int done;                  // number of finished processes

    for (done = 0; done < n; ) {
        int progressed = 0;    // did we run at least one process this pass?

        for (int i=0; i<n; ++i) {
            if (rem_bt[i] <= 0) continue;     // already finished
            if (plist[i].art > t) continue;   // not yet arrived

            // run this process for up to 'quantum'
            progressed = 1;
            if (rem_bt[i] > quantum) {
                t += quantum;
                rem_bt[i] -= quantum;
            } else {
                // last cycle for this process
                t += rem_bt[i];
                rem_bt[i] = 0;
                // waiting time = finish - arrival - burst
                plist[i].wt = t - plist[i].art - plist[i].bt;
                if (plist[i].wt < 0) plist[i].wt = 0;
                done++;
            }
        }

        // If nothing ran because all remaining procs haven’t arrived yet,
        // fast-forward time to the next arrival.
        if (!progressed) {
            int next_arr = INT_MAX;
            for (int i=0; i<n; ++i)
                if (rem_bt[i] > 0 && plist[i].art < next_arr)
                    next_arr = plist[i].art;
            if (next_arr == INT_MAX) break; // safety
            if (next_arr > t) t = next_arr;
        }
    }

    free(rem_bt);
} 


// ---------- SJF (SRTF preemptive). Also works with nonzero arrivals ----------
void findWaitingTimeSJF(ProcessType plist[], int n)
{ 
    int *rem_bt = (int*)malloc(sizeof(int)*n);
    if (!rem_bt) { perror("malloc"); exit(1); }

    for (int i=0; i<n; ++i) {
        rem_bt[i] = plist[i].bt;
        plist[i].wt = 0;
    }

    int complete = 0;          // completed processes
    int t = 0;                 // current time
    int shortest = -1;         // index of currently selected process
    int minm = INT_MAX;        // remaining time of 'shortest'
    int finish_time;

    while (complete != n) {
        // pick process with minimum remaining time among arrived ones
        shortest = -1;
        minm = INT_MAX;
        for (int j=0; j<n; ++j) {
            if (plist[j].art <= t && rem_bt[j] > 0 && rem_bt[j] < minm) {
                minm = rem_bt[j];
                shortest = j;
            }
        }

        if (shortest == -1) {
            // no process has arrived yet; jump to next arrival to avoid O(T) idle loops
            int next_arr = INT_MAX;
            for (int j=0; j<n; ++j)
                if (rem_bt[j] > 0 && plist[j].art < next_arr)
                    next_arr = plist[j].art;
            if (next_arr == INT_MAX) break;      // safety
            if (next_arr > t) t = next_arr;
            continue;
        }

        // run the shortest for 1 time unit (preemptive)
        rem_bt[shortest]--;
        t++;

        // if finished, compute waiting time
        if (rem_bt[shortest] == 0) {
            complete++;
            finish_time = t;
            plist[shortest].wt = finish_time - plist[shortest].bt - plist[shortest].art;
            if (plist[shortest].wt < 0) plist[shortest].wt = 0;
        }
    }

    free(rem_bt);
} 


// ---------- FCFS waiting time (provided baseline) ----------
void findWaitingTime(ProcessType plist[], int n)
{ 
    // waiting time for first process is 0, or its arrival time if not
    plist[0].wt = 0 +  plist[0].art; 
  
    // FCFS chain (assumes arrival=0 or already sorted suitably)
    for (int  i = 1; i < n ; i++ ) 
        plist[i].wt =  plist[i-1].bt + plist[i-1].wt; 
} 
  
// ---------- Turnaround time = waiting + burst ----------
void findTurnAroundTime(ProcessType plist[], int n)
{ 
    for (int  i = 0; i < n ; i++) 
        plist[i].tat = plist[i].bt + plist[i].wt; 
} 
  
// ---------- Priority comparator for qsort ----------
int my_comparer(const void *this, const void *that)
{ 
    const ProcessType *a = (const ProcessType*)this;
    const ProcessType *b = (const ProcessType*)that;

    // Higher priority runs first. If lower integer means "higher priority",
    // sort ascending by pri. Tie-break by arrival, then pid for stability.
    if (a->pri != b->pri) return (a->pri - b->pri);
    if (a->art != b->art) return (a->art - b->art);
    return (a->pid - b->pid);
} 

// ---------- FCFS ----------
void findavgTimeFCFS(ProcessType plist[], int n) 
{ 
    findWaitingTime(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nFCFS\n");
}

// ---------- SJF ----------
void findavgTimeSJF(ProcessType plist[], int n) 
{ 
    findWaitingTimeSJF(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nSJF\n");
}

// ---------- Round Robin ----------
void findavgTimeRR(ProcessType plist[], int n, int quantum) 
{ 
    findWaitingTimeRR(plist, n, quantum); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nRR Quantum = %d\n", quantum);
}

// ---------- Priority: sort by priority, then reuse FCFS math ----------
void findavgTimePriority(ProcessType plist[], int n) 
{ 
    qsort(plist, n, sizeof(ProcessType), my_comparer);
    // Now just apply FCFS math on this priority order:
    findWaitingTime(plist, n); 
    findTurnAroundTime(plist, n); 
    printf("\n*********\nPriority\n");
}
