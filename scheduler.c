#include "headers.h"
struct msgbuff
{
    long m_type;
    int process_prop[20];
};
//-----------handlers----------//
void stop_scheduler(int signum);
void handler(int signum);
void handler2(int signum);
//-----------structs----------//
struct Queue *q;
struct Queue *q_done;
struct Node *Node_to_beinserted;
struct Node *process;
int c = 1;
int Remaining_time;

int running_time_all = 0;

int main(int argc, char *argv[])
{
    initClk();
    signal(SIGINT, stop_scheduler);
    // signal(SIGSTOP, stop_scheduler);
    signal(SIGUSR1, handler);
    printf("scheduler is running\n");
    // TODO implement the scheduler :(
    //  while p_id < process count
    int process_count = atoi(argv[1]); // change later to receive from system arguments
    int algorism = atoi(argv[2]);
    int quntum = 5;
    int process_id[process_count];
    int y = 0;
    int TA;
    float WTA;
    FILE *fptr;
    fptr = fopen("schedular.log", "w");
    fprintf(fptr, "#At time x process y state arr w total z remian y wait k \n");
    //------for statistics--------//
    int arr_waiting[process_count]; // waiting time

    int received_pross = 0;
    int time, nextime;
    time = getClk();
    int msgq_id1, rec_t_sch;
    msgq_id1 = msgget(key_sch_pgen, 0666 | IPC_CREAT);
    q = createQueue();
    q_done = createQueue();       // the queue after it is done
    int idprocess[process_count]; // array to save the process ids in
    int x = 0;
    int finished_process = 0;
    struct Node *p = NULL;
    struct msgbuff message;

    while (is_empty(q) == 0 || received_pross < process_count) // main loop for sceduler HINT: Never keep the queue empty while there are any processes left
    {
        // printqueue(q);
        //  printf(" running time process %d", p->running_time);
        do
        {
            rec_t_sch = msgrcv(msgq_id1, &message, sizeof(message.process_prop), 0, IPC_NOWAIT); // shouldn't wait for msg
            if (rec_t_sch == -1 && is_empty(q) == 1)
            {
                rec_t_sch = msgrcv(msgq_id1, &message, sizeof(message.process_prop), 0, !IPC_NOWAIT);
            }

            if (rec_t_sch != -1) // to check if there is received msg
            {
                printf("process ID= %d just arrived at schuedler\n", message.process_prop[0]);
                Node_to_beinserted = newNode(message.process_prop[0], message.process_prop[1], message.process_prop[2], message.process_prop[3], message.process_prop[4]);
                Node_to_beinserted->running_time = Node_to_beinserted->process_time;
                enQueue(q, Node_to_beinserted); // enqueue this process
                received_pross++;
            }

        } while (rec_t_sch != -1);

        nextime = getClk();
        if (nextime > time) // this block will be called every 1 sec
        {

            //     printf(" running time process %d", p->running_time);
            time = getClk();
            int pid, status;
            switch (algorism)
            {
            case 1: // HPF

                process = pop_queue(q);
                pid = fork();
                process->start_time = getClk();
                process->waiting_time = getClk() - process->arrival_time;
                int x = getClk();
                //     printf("The time is %d\n", x);
                if (pid == 0)
                {
                    fptr = fopen("scheduler.log", "w");
                    char buffer[20];
                    char *send_to_process[2];
                    sprintf(buffer, "%d", process->process_time);
                    char *argv[] = {"./process.out", buffer, NULL, 0};
                    execve(argv[0], &argv[0], NULL);
                }
                pid = wait(&status);
                if (WIFEXITED(status))
                {
                    finished_process++;
                    running_time_all += getClk() - process->start_time;
                    process->finish_time = getClk();
                    printf("waiting time is %d\n", process->waiting_time);
                    printf("finish time is %d\n", process->finish_time);
                    int TA = process->finish_time - process->arrival_time;
                    float WTA = (float)TA / (float)process->process_time;
                    fprintf(fptr, "At time %d process %d started arr %d total %d remain %d wait %d \n", process->start_time, process->process_id, process->arrival_time, process->process_time, process->process_time, process->waiting_time);
                    fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d  WTA %.2f\n", process->start_time, process->process_id, process->arrival_time, process->process_time, process->process_time, process->waiting_time, TA, WTA);
                    enQueue(q_done, process);
                }
                break;

            case 2: // STRN   HINT: just update both remaning time and set priority = remaining time each sec

                process = peek_queue(q);

                process->start_time = getClk();

                // Remaining_time = process->process_time - (getClk() - process->start_time);
                // process->Remaining_time = Remaining_time;

                if (process->stat == WAITING)
                {
                    pid = fork(); // call for process

                    process->Remaining_time = process->process_time;
                    process->waiting_time = getClk() - process->arrival_time;
                    process->stat = RUNNING;
                    process->waiting_time = getClk() - process->arrival_time;
                    fprintf(fptr, "At time %d process %d started arr %d total %d remain %d wait %d \n", process->start_time, process->process_id, process->arrival_time, process->process_time, process->Remaining_time, process->waiting_time);
                    if (pid == 0)
                    {

                        char buffer[20];
                        char *send_to_process[2];
                        sprintf(buffer, "%d", process->process_time);
                        char *argv[] = {"./process.out", buffer, NULL, 0};
                        execve(argv[0], &argv[0], NULL);
                    }
                    else if (pid != 0)
                    {
                        process_id[y] = pid;
                        y++;
                        c = 1;
                        while (c == 1)
                        {
                            pause();
                        }
                        if (c == 0)
                        {
                            kill(process_id[process->process_id - 1], SIGSTOP);
                            process->stat = STOPPED;
                            // Remaining_time=process->Remaining_time;
                            process->Remaining_time = process->process_time - (getClk() - process->start_time);
                            printf("The remaining time is %d\n", process->Remaining_time);
                            fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                        }
                        else if (c == 2)
                        {
                            running_time_all += getClk() - process->start_time;
                            process->finish_time = getClk();
                            printf("waiting time is %d\n", process->waiting_time);
                            printf("finish time is %d\n", process->finish_time);
                            int TA = process->finish_time - process->arrival_time;
                            float WTA = (float)TA / (float)process->process_time;
                            // printf(" the process time is %d",process->process_time);

                            fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d  WTA %.2f\n", process->start_time, process->process_id, process->arrival_time, process->process_time, process->Remaining_time, process->waiting_time, TA, WTA);
                            pop_queue(q);
                            enQueue(q_done, process);
                        }
                    }
                }
                else if (process->stat == RUNNING)
                {
                    c = 1;
                    while (c == 1)
                    {
                        pause();
                    }

                    process->Remaining_time -= (getClk() - process->start_time);

                    if (c == 0)
                    {
                        kill(process_id[process->process_id - 1], SIGSTOP);
                        process->stat = STOPPED;
                        // Remaining_time=process->Remaining_time;
                        process->Remaining_time = process->process_time - (getClk() - process->start_time);
                        printf("The remaining time is %d\n", process->Remaining_time);
                        fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                    }
                    else if (c == 2)
                    {
                        running_time_all += getClk() - process->start_time;
                        process->finish_time = getClk();
                        printf("waiting time is %d\n", process->waiting_time);
                        printf("finish time is %d\n", process->finish_time);
                        int TA = process->finish_time - process->arrival_time;
                        float WTA = (float)TA / (float)process->process_time;
                        // printf(" the process time is %d",process->process_time);

                        fprintf(fptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d  WTA %.2f\n", process->start_time, process->process_id, process->arrival_time, process->process_time, process->Remaining_time, process->waiting_time, TA, WTA);
                        pop_queue(q);
                        enQueue(q_done, process);
                    }
                }
                else if (process->stat == STOPPED)
                {
                    process->waiting_time = getClk() - process->arrival_time + process->waiting_time;
                    process->stat = COUNTIUNE;
                    fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                    kill(process_id[process->process_id - 1], SIGCONT);
                }
                break;

            case 3: // RR   HINT: Just Dequeue then Enqueue the process to go back to the end of the queue

                if (p != NULL && p->stat != FINSIHED)
                {
                    enQueue(q, p);
                }
                p = pop_queue(q); // get the process and dequeue it
                char buffer1[5];
                sprintf(buffer1, "%d", p->process_time); // pass the remianing time
                if (p->stat == WAITING)
                {
                    p->start_time = getClk(); // start time of the process added to the node
                    int pid = fork();         // fork the process
                    if (pid == 0)
                    {
                        char *argn[] = {"./process.out", buffer1, NULL, 0};
                        execve(argn[0], &argn[0], NULL);
                    }
                    else // it is the first time for the process to run
                    {
                        p->waiting_time = p->start_time - p->arrival_time; // get waiting time
                        // fptr = fopen("schedular.log", "a+");
                        fprintf(fptr, "At time  %d  process %d started arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                        // fclose(fptr);
                        idprocess[x] = pid;
                        x++;
                    }
                }
                else if (p->stat == STOPPED) // process is resumed
                {
                    p->waiting_time = getClk() - p->stopped_time + p->waiting_time;
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d resumed arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                    // fclose(fptr);
                    kill(idprocess[p->process_id - 1], SIGCONT); // continue the process
                }

                if (p->running_time > quntum)
                {
                    // printf("process %d bigger qunatum\n", p->process_id);
                    // printf(" running time process %d", p->running_time);
                    printf("process %d resumed\n", p->process_id);
                    sleep(quntum);
                    kill(idprocess[p->process_id - 1], SIGSTOP); // stop the process after the quntum
                    p->running_time = p->running_time - quntum;  // decrease the remaing time by quantum
                    p->stopped_time = getClk();
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d stopped arr %d total %d remian %d wait %d \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time);
                    // fclose(fptr);
                    p->stat = STOPPED;
                    if (is_empty(q) && finished_process == process_count - 1)
                    {
                        // printf("process id %d is done", p->process_id);
                        sleep(p->running_time);
                        p->running_time = 0; // decrease the remaing time by quantum
                        p->stat = FINSIHED;
                        TA = getClk() - p->arrival_time;
                        WTA = (float)TA / p->process_time;
                        p->finish_time = getClk();
                        // fptr = fopen("schedular.log", "a+");
                        fprintf(fptr, "At time  %d  process %d finished arr %d total %d remian %d wait %d TA %d WTA %.2f \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time, TA, WTA);
                        // fclose(fptr);
                        finished_process++;
                        running_time_all = p->process_time + running_time_all;
                        enQueue(q_done, p);
                    }
                }
                else if (p->running_time <= quntum && p->running_time != 0)
                {
                    // printf("process %d smaller qunatum\n", p->process_id);
                    // printf(" running time process %d", p->running_time);
                    // printf("process id %d is done", p->process_id);
                    sleep(p->running_time);
                    p->running_time = 0; // decrease the remaing time by quantum
                    p->stat = FINSIHED;
                    TA = getClk() - p->arrival_time;
                    WTA = (float)TA / p->process_time;
                    p->finish_time = getClk();
                    // fptr = fopen("schedular.log", "a+");
                    fprintf(fptr, "At time  %d  process %d finished arr %d total %d remian %d wait %d TA %d WTA %.2f \n", getClk(), p->process_id, p->arrival_time, p->process_time, p->running_time, p->waiting_time, TA, WTA);
                    // fclose(fptr);
                    finished_process++;
                    running_time_all = p->process_time + running_time_all;
                    enQueue(q_done, p);

                    // printf(" running time process %d", p->running_time);
                }
                break;

            default:
                break;
            }
        }
    }
    fclose(fptr);
    //  upon termination release the clock resources

    //------loop to get the weighted turnaround time and to get waiting time------//
    struct Node *print_process;
    float wta = 0.0;
    float wait = 0.0;
    float arr_std[process_count];
    float std_avg_wta = 0.0;

    float utilization = ((float)running_time_all / getClk()) * 100.0;
    for (int i = 0; i < process_count; i++)
    {
        print_process = pop_queue(q_done);
        wta += (float)(print_process->finish_time - print_process->arrival_time) / print_process->process_time;
        wait += print_process->waiting_time;
        arr_std[i] = (float)(print_process->finish_time - print_process->arrival_time) / print_process->process_time;
    }

    wta = wta / process_count;
    wait = (float)wait / process_count;
    for (int i = 0; i < process_count; i++)
    {
        std_avg_wta += (wta - arr_std[i]) * (wta - arr_std[i]);
    }

    std_avg_wta = sqrtf(std_avg_wta / process_count);

    FILE *perf_file;
    perf_file = fopen("scheduler.perf", "w");
    if (perf_file == NULL)
    {
        printf("error in opening file\n");
    }
    else
    {
        fprintf(perf_file, "Avg WTA = %.2f %% \n", utilization);
        fprintf(perf_file, "Avg WTA = %.2f\n", wta);
        fprintf(perf_file, "Avg Waiting = %.2f\n", wait);
        fprintf(perf_file, "Std WTA = %.2f\n", std_avg_wta);
    }
    fclose(perf_file);

    destroyClk(true);
    exit(0);
}

void stop_scheduler(int signum)
{
    printf("the scheduler has stopped\n");
    destroyClk(true);
    exit(0);
}
void handler(int signum)
{
    c = 0;
}
void handler2(int signum)
{
    int status, wait_pid;
    wait_pid = waitpid(-1, &status, WNOHANG);
    if (wait_pid != 0 && WIFEXITED(status))
    {

        c = 2;
    }
    else
        c = 0;
}
