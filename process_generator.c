#include "headers.h"
#include <string.h>
void clearResources(int);

struct msgbuff
{
    long m_type;
    int process_prop[20];
    // int priority;
    // int process_id;
    // int process_time;
    // int arrival_time;
    // enum STATUS stat;
};
int msgq_id, send_t_sch;

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    signal(SIGSTOP, clearResources);

    //-----------------------------------------
    // 1. Read the input files.
    FILE *fp = fopen("processes.txt", "r");
    int process_data[50][4];
    int lines = line_count(fp) - 1;
    int process_count = lines;
    rewind(fp);
    fscanf(fp, "%*[^\n]\n"); // to skip the commented line
    int j = 0;
    while (j < lines)
    {
        for (int i = 0; i < 4; i++)
            fscanf(fp, "%d", &process_data[j][i]);
        j++;
    }
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("please enter scheduleing algorism\n1 for HPF\n2 for SRTN\n3 for RR\n");
    int algorithim = -1;
    scanf("%d", &algorithim);

    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid == 0)
        if (execv("./clk.out", argv) == -1)
            perror("failed to execv");
    // 4. Use this function after creating the clock process to initialize clock

    // To get time use this
    // printf("current time is %d\n", x);
    pid = fork();
    if (pid == 0)
    {
        char buffer1[20];
        char buffer2[20];
        sprintf(buffer1, "%d", process_count);
        sprintf(buffer2, "%d", algorithim);
        argv[1] = buffer1;
        argv[2] = buffer2;
        // argv[1] = process_count ; needs to be fixeds
        if (execv("./scheduler.out", argv) == -1)
            perror("failed to execv");
    }
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    int count1 = 0;

    msgq_id = msgget(key_sch_pgen, 0666 | IPC_CREAT); // creating the message queue
    struct msgbuff message;                           // the message to be sent to scheduler, details of every process
    // TODO Generation Main Loop
    initClk();
    while (count1 < process_count)
    {
        int time = getClk();
        while (getClk() < process_data[count1][1])
        {
            sleep(process_data[count1][1] - getClk()); // newly added
        }
        int priority = -1;
        switch (algorithim)
        {
        case 1: // for HPF
            priority = process_data[count1][3];
            break;
        case 2: // for STRN
            priority = process_data[count1][2];
            break;
        case 3: // for RR
            priority = 20;
            break;
        }

        message.m_type = process_data[count1][0];
        message.process_prop[0] = process_data[count1][0];
        message.process_prop[1] = process_data[count1][1];
        message.process_prop[2] = process_data[count1][2];
        message.process_prop[3] = priority;
        message.process_prop[4] = WAITING;
        send_t_sch = msgsnd(msgq_id, &message, sizeof(message.process_prop), !IPC_NOWAIT);
         if(algorithim == 2 && count1!=0)
        {
            kill(pid,SIGUSR1);
        }

        if (send_t_sch == 0)
        {
            printf("message successful at time %d \n", getClk());
        }
        count1++;
        //}
        //}
    }

    // 7. Clear clock resources

    int status;
    pid = wait(&status);
    if (WIFEXITED(status))
    {
        int msgq_del;
        msgq_del = msgctl(msgq_id, IPC_RMID, 0);
        destroyClk(true);
        exit(0);
    }
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    printf("the process generator has stopped\n");
    int msgq_del;
    msgq_del = msgctl(msgq_id, IPC_RMID, 0);
    destroyClk(true);
    exit(0);
}
