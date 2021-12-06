#ifndef VIRTUAL_PROCESS_H
#define VIRTUAL_PROCESS_H

typedef struct virtualprocess VP;
typedef struct virtualprocess *VPptr;

typedef struct virtualprocess
{
    int processId;
    int size;        //Size in KB
    int duration;    //Duration of the process in seconds
    int timeWaiting; //Seconds waiting before the process starts excecuting
    int finished;    //0=NOT FINISHED, 1=FINISHED
    int startSec;    //Second that process started
    int endSec;      //Seconfd that process ended
} VP;

void VPInit(VPptr VirtualP, int duration, int size, int procId, int timeWaited);
int getSize(VPptr VirtualP);
int getDuration(VPptr VirtualP);
void VPdestruct(VPptr VirtualP);

#endif