//
//  main.cpp
//  OS HW2
//
//  Created by James Garcia on 11/14/13.
//  Copyright (c) 2013 James Garcia. All rights reserved.
//
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <string>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace std;

#define FIFO 0
#define LIFO 1
#define LRU 2
#define MFU 3
#define RANDOM 4
#define WS 5


/* Gets the max number that can be made from bit size ps */
int getMaxBitSize(int ps)
{
    int result = 0;
    for(int i = 0; i<ps; i++)
    {
        result += pow(2, i);// 2 to the i
    }
    return result;
}

int getAddressIndex(vector<pair<int, int>>::iterator iter, int addressSpace[], int k, int r)
{
    for(int i = 0; i<k; i++)
    {
        if(iter->first == addressSpace[i])
        {
            return i;
        }
    }
}

/* Get offset value */
int getOffset(int address, int maxNumberOfOffset)
{
    return address & maxNumberOfOffset;
}

/* Get Page value */
int getPage(int address,int ps , int maxNumberOfPages)
{
    return (address >> ps) & maxNumberOfPages;
}

int getSegment(int address, int ps, int sl)
{
    return address >> (ps+sl);
}

/* Replacement Algorithms */
int fifoFrame(int r, int index, int mainMemory[])
{
    return ((index * r)+r)-1;
}
int lifoFrame(int r, int index, int mainMemory[])
{
    return ((index * r)+r)-1;
}
int lruFrame(int r, int index, int mainMemory[])
{
    return ((index * r)+r)-1;
}

int mfuFrame(int r, int index, int mainMemory[], int mfuCounter[])
{
    int mostFrequent = 0;

    for(int i = 0; i<r; i++)
    {
        int curFrame = r*index + i;
        if(mfuCounter[curFrame] > mfuCounter[mostFrequent])
        {
            mostFrequent = curFrame;
        }
        
    }
    return mostFrequent;
}
int randomFrame(int r, int index)
{
    int num =(rand() % r) + (index*r);
    return num;
}
int wsFrame(int r, int index, int mainMemory[])
{
    return ((index * r)+r)-1;;
}

/* fault handler */
int faultHandler(vector<pair<int, int>>::iterator iter,int k, int r, int ps, int tp, int index, int vmm[], int mainMemory[], int replacementAlgorithm, int mfuCounter[])
{
    for(int i = 0; i<r; i++)
    {
        int curFrame = r*index + i;
        
        if(mainMemory[curFrame]==-1)
        {
            return curFrame;
        }
    }
  
    switch (replacementAlgorithm)
    {
        case FIFO:
            return fifoFrame(r,index,mainMemory);
            break;
            
        case LIFO:
            return lifoFrame(r,index,mainMemory);
            break;
            
        case LRU:
            return lruFrame(r,index,mainMemory);
            break;
            
        case MFU:
            return mfuFrame(r,index,mainMemory, mfuCounter);
            break;
            
        case RANDOM:
            return randomFrame(r, index);
            break;
            
        case WS:
            return wsFrame(r,index,mainMemory);
            break;
        
        default:
            break;
    }
}

void diskDriver(vector<pair<int, int>> processAddr, int addressSpace[],int k, int r, int ps, int tp, int max, int vmm[], int mainMemory[], int numberOfPageFaults[], int replacementAlgorithm)
{
    //process request
    for (vector<pair<int, int>>::iterator iter = processAddr.begin(); iter!=processAddr.end(); iter++)
    {
        //create counter for MFU
        int mfuCounter[r*k];
        for(int i=0; i<r*k; i ++)
        {
            mfuCounter[i]=0;
        }
        
        //create Working Set
        int workingSet[max];
        for(int i=0; i<max; i ++)
        {
            workingSet[i]=0;
        }
        
        bool addressInMainMemory = false;
        //check for end of process.
        if(iter->second==-1)
        {
            //cout<<"End of Process "<<iter->first<<endl;
            continue;
        }
        else
        {
            int index = getAddressIndex(iter, addressSpace, k, r);
            
            if(vmm[index*tp+(iter->second >>ps)] == -1)
            {
                addressInMainMemory = false;
            }
            else
            {
                addressInMainMemory = true;
            }
            
            if(!addressInMainMemory)
            {
                numberOfPageFaults[index]+=1;
                
                int newFrame = faultHandler(iter, k, r, ps, tp, index, vmm, mainMemory, replacementAlgorithm, mfuCounter);
                
                if(mainMemory[newFrame]!=-1)
                {
                    vmm[mainMemory[newFrame]] = -1;
                }
                if(replacementAlgorithm == FIFO)
                {
                    for(int i = index*r; i<index*r+r-2; i++)
                    {
                        vmm[mainMemory[i]]= vmm[mainMemory[i+1]];
                        mainMemory[i]=mainMemory[i+1];
                    }
                    
                    vmm[index*tp+ (iter->second >> ps)] = newFrame;
                    mainMemory[newFrame] = index*tp+ (iter->second >> ps);
                }
                else if (replacementAlgorithm == LRU)
                {
            
                    for(int i = index*r; i<index*r+r-2; i++)
                    {
                        vmm[mainMemory[i]]= vmm[mainMemory[i+1]];
                        mainMemory[i]=mainMemory[i+1];
                    }
                    
                    vmm[index*tp+ (iter->second >> ps)] = newFrame;
                    mainMemory[newFrame] = index*tp+ (iter->second >> ps);
                }
                else if (replacementAlgorithm == MFU)
                {
                   
                    vmm[index*tp+ (iter->second >> ps)] = newFrame;
                    mainMemory[newFrame] = index*tp+ (iter->second >> ps);
                }
                else
                {
                    vmm[index*tp+ (iter->second >> ps)] = newFrame;
                    mainMemory[newFrame] = index*tp+ (iter->second >> ps);
                }
            }
            else
            {
                if(replacementAlgorithm == LRU)
                {
                    int foundInMainMemory = vmm[index*tp+(iter->second >>ps)];
                    int newFrame = ((index * r)+r)-1;
                    for(int i = vmm[index*tp+(iter->second >>ps)]; i<index*r+r-2; i++)
                    {
                        vmm[mainMemory[i]]= vmm[mainMemory[i+1]];
                        mainMemory[i]=mainMemory[i+1];
                    }
                
                    vmm[foundInMainMemory] = newFrame;
                    mainMemory[newFrame] = index*tp+ (iter->second >> ps);
                }
                if(replacementAlgorithm == MFU)
                {
                    mfuCounter[vmm[index*tp+(iter->second >>ps)]]++;
                }
            }
        }
    }
}


int main(int argc, const char * argv[])
{
    int tp;  // total number of page frames in main memory
    int sl;  // maximum segment length in number of pages
    int ps;  // page size in number of bytes
    int r;   // number of page frames per process 
    int min; //
    int max; //
    int k;   // total number of processes
    vector<pair<int, int>> processID; //ProcessID and Space in disk
    vector<pair<int, int>> processAddr; //ProcessID and Address
    
    srand (time(NULL));
    
    /* READ FILE */
    ifstream myfile ("input.txt");
    if (myfile.is_open())
    {
        string line;
        //get tp
        getline(myfile,line);
        stringstream(line) >> tp;
        
        //get sl
        getline(myfile,line);
        stringstream(line) >> sl;
        
        //get ps
        getline(myfile,line);
        stringstream(line) >> ps;
        
        //get r
        getline(myfile,line);
        stringstream(line) >> r;
        
        //get min
        getline(myfile,line);
        stringstream(line) >> min;
        
        //get max
        getline(myfile,line);
        stringstream(line) >> max;
        
        //get k
        getline(myfile,line);
        stringstream(line) >> k;
        
        //get diskPagesPerProcess k
        istringstream iss;
        stringstream ss;
        
        //Store process and Max number of disk pages
        for(int i = 0; i<k; i++)
        {
            int process;
            int diskPages;
            getline(myfile,line);
            iss.str(line);
            iss >> process;
            iss >> diskPages;
            processID.push_back(std::make_pair(process, diskPages));
        }
        
        
        //Store process and address
        while(!myfile.eof())
        {
            int process;
            string convert;
            int address;
            getline(myfile,line);
            iss.str(line);
            iss >> process;
            iss >> convert;
            ss << hex << convert;
            ss >> address;
            ss.clear();
            processAddr.push_back(std::make_pair(process, address));
        }
        
        int logsl = ceil(log2(sl));
        ps = ceil(log2(ps));

        
        //create DTP a=ProcessID, p=Page number
        int DTP[k][sl];
        int counter = 0;
        for(int i = 0; i<k; i++)
        {
            for(int j = 0; j<sl; j++)
            {
                DTP[i][j]=counter;
                counter++;
            }
        }
        
        int maxNumberofOffset = getMaxBitSize(ps);
        int maxNumberOfPages = getMaxBitSize(logsl);

    }
    else cout << "Unable to open file\n";
    myfile.close();
    /* END READ FILE */
    
    //Semaphores
    int processRun;
    long key;
    int nbytes;
    processRun = semget(key, nbytes, 0666 | IPC_CREAT);
    

    //create main memeory size tp
    int mainMemory[tp];
    
    // create virtual memory which is size of main memory * k number of processes | Fill it with -1 to start
    int vmm[k*tp];

    
    //create address space pointers for each process
    int addressSpace[k];
    int count = 0;
    
    for (vector<pair<int, int>>::iterator iter = processID.begin(); iter!=processID.end(); iter++)
    {
        addressSpace[count] = iter->first;
        count++;
    }
    
    //create page fault counter
    int numberOfPageFaults[k];
    
    for(int i = 0; i <6; i++)
    {
        //clear mainMemory and vmm
        for(int j = 0; j<k*tp; j++)
        {
            vmm[j] = -1;
        }
        for(int j = 0; j<tp; j++)
        {
            mainMemory[j] = -1;
        }
        
        //reinitialize fault counter
        for(int j = 0; j<k; j++)
        {
            numberOfPageFaults[j]=0;
        }
        //find algorithm
        switch (i) {
            case FIFO:
                cout<<"------ "<<"FIFO"<<" page faults ------"<<endl;
                break;
                
            case LIFO:
                cout<<"------ "<<"LIFO"<<" page faults ------"<<endl;
                break;
                
            case LRU:
                cout<<"------ "<<"LRU"<<" page faults ------"<<endl;
                break;
                
            case MFU:
                cout<<"------ "<<"MFU"<<" page faults ------"<<endl;
                break;
                
            case RANDOM:
                cout<<"------ "<<"RANDOM"<<" page faults ------"<<endl;
                break;
                
            case WS:
                cout<<"------ "<<"WORKING SET"<<" page faults ------"<<endl;
                break;
                
            default:
                break;
        }
        diskDriver(processAddr, addressSpace, k, r, ps, tp, max, vmm, mainMemory, numberOfPageFaults, i);
        cout<<endl;
        
        //get total number of page faults
        int totalNumberOfPageFaults = 0;
        for(int j = 0; j<k; j++)
        {
            cout<<"Process "<<addressSpace[j]<<": "<<numberOfPageFaults[j]<<endl;
            
            totalNumberOfPageFaults+= numberOfPageFaults[j];
        }
        cout<<totalNumberOfPageFaults<<endl;
        
        if(i == WS)
        {
            cout<<"Min = "<<min<<endl;
            cout<<"Max = "<<max<<endl;
        }
    }
    return 0;
}

