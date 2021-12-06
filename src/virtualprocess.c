#include <stdlib.h>
#include <string.h>
#include "virtualprocess.h"

void VPInit(VPptr VirtualP, int duration, int size, int procId, int timeWaited)
{
    VirtualP->processId = procId;
    VirtualP->duration = duration;
    VirtualP->size = size;
    VirtualP->timeWaiting = timeWaited;
    VirtualP->finished = 0;
    VirtualP->startSec = -1;
    VirtualP->endSec = -1;
}

int getSize(VPptr VirtualP)
{
    return VirtualP->size;
}

int getDuration(VPptr VirtualP)
{
    return VirtualP->duration;
}

void VPdestruct(VPptr VirtualP)
{
    free(VirtualP);
}