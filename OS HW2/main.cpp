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

int randomNum(int r, int index)
{
    int num =(rand() % r) + (index*r);
    return num;
}

/* fault handler */
int faultHandler(vector<pair<int, int>>::iterator iter,int k, int r, int ps, int tp, int index, int vmm[], int mainMemory[], int replacementAlgorithm)
{
    for(int i = 0; i<index*r+r; i++)
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
            return -1;
            break;
            
        case LIFO:
            return -1;
            break;
            
        case LRU:
            return -1;
            break;
            
        case MFU:
            return -1;
            break;
            
        case RANDOM:
            return randomNum(r, index);
            break;
            
        case WS:
            return -1;
            // make a list of length (input dependent). Every access, if found, move to the back of list, remove the front. Else add new one to back, remove front.
            break;
    }
}

void processRequest(vector<pair<int, int>> processAddr, int addressSpace[],int k, int r, int, int ps, int tp, int vmm[], int mainMemory[], int numberOfPageFaults[], int replacementAlgorithm)
{
    //process request
    for (vector<pair<int, int>>::iterator iter = processAddr.begin(); iter!=processAddr.end(); iter++)
    {
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
                cout<<index<<endl;
                numberOfPageFaults[index]+=1;
                
                int newFrame = faultHandler(iter, k, r, ps, tp, index, vmm, mainMemory, replacementAlgorithm);
                if(mainMemory[newFrame]!=-1)
                {
                    vmm[mainMemory[newFrame]] = -1;
                }
                vmm[index*tp+ (iter->second >> ps)] = newFrame;
                mainMemory[newFrame] = index*tp+ (iter->second >> ps);
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
    
    
    cout<<"------------Begin Process Request------------"<<endl;
    for(int i = 0; i <6; i++)
    {
        //clear mainMemory and vmm
        for(int i = 0; i<k*tp; i++)
        {
            vmm[i] = -1;
        }
        for(int i = 0; i<tp; i++)
        {
            mainMemory[i] = -1;
        }
        
        //reinitialize fault counter
        for(int i = 0; i<k; i++)
        {
            numberOfPageFaults[i]=0;
        }
        
        cout<<"------algorithm "<<i<<" ------"<<endl;
        processRequest(processAddr, addressSpace, k, r, k, ps, tp, vmm, mainMemory, numberOfPageFaults, i);
        cout<<endl<<endl;
        
        //get total number of page faults
        int totalNumberOfPageFaults = 0;
        for(int i = 0; i<k; i++)
        {
            cout<<i<<" "<<numberOfPageFaults[i]<<endl;
            totalNumberOfPageFaults+= numberOfPageFaults[i];
        }
        cout<<totalNumberOfPageFaults<<endl;
    }
    return 0;
}

