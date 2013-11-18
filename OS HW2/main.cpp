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
            //cout<<"Address space is "<<i<<endl;
            //cout<<"searching starts at frame "<<i*r<<endl;
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

/* fault handler */
void faultHandler()
{
    cout<<"I am the fault handler"<<endl;
    exit(0);
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

    
    vector<pair<int, int>> processID;
    vector<pair<int, int>> processAddr;
    
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
        
        cout<<tp<<endl;
        int logsl = ceil(log2(sl));
        cout<<sl<<endl;
        ps = ceil(log2(ps));
        cout<<ps<<endl;
        cout<<r<<endl;
        cout<<min<<endl;
        cout<<max<<endl;
        cout<<k<<endl;
        
        //create DTP a=ProcessID, p=Page number
        int DTP[k][sl];
        int counter = 0;
        for(int i = 0; i<k; i++)
        {
            for(int j = 0; j<sl; j++)
            {
                DTP[i][j]=counter;
                cout<<"DTP["<<i<<"]["<<j<<"]="<<counter<<endl;
                counter++;
            }
        }
        
        
        for (vector<pair<int, int>>::iterator iter = processID.begin(); iter!=processID.end(); iter++)
        {
            cout << iter->first <<" "<< iter->second << endl;
        }
        
        int maxNumberofOffset = getMaxBitSize(ps);
        int maxNumberOfPages = getMaxBitSize(logsl);
        for (vector<pair<int, int>>::iterator iter = processAddr.begin(); iter!=processAddr.end(); iter++)
        {
            cout<<"-----"<<iter->first<<"-----"<<endl;
            cout<<"Offset="<<getOffset(iter->second, maxNumberofOffset)<<endl;
            cout<<"Page="<<getPage(iter->second, ps, maxNumberOfPages)<<endl;
            cout<<"Segment="<<getSegment(iter->second, ps, logsl)<<endl;
            
        }
    }
    else cout << "Unable to open file\n";
    myfile.close();
    /* END READ FILE */
    
    //create main memeory size tp
    int mainMemory[tp];
    
    // create virtual memory which is size of main memory * k number of processes
    int vmm[k*tp];
    
    for(int i = 0; i<tp; i++)
    {
        mainMemory[i] = -1;
    }
    
    //create address space pointers for each process
    int addressSpace[k];
    int count = 0;
    
    for (vector<pair<int, int>>::iterator iter = processID.begin(); iter!=processID.end(); iter++)
    {
        addressSpace[count] = iter->first;
        cout<<addressSpace[count]<<endl;
        count++;
    }
    
    //process request
    for (vector<pair<int, int>>::iterator iter = processAddr.begin(); iter!=processAddr.end(); iter++)
    {
        if(iter->second==-1)
        {
            cout<<iter->first<<" "<<iter->second<<endl;
            continue;
        }
        
        int index = getAddressIndex(iter, addressSpace, k, r);
        
        vmm[index*tp+ (iter->second >> ps)] = index*tp+ (iter->second >> ps);
        cout<<"virtual main memory="<<vmm[(index* tp) + (iter->second >> ps)]<<endl;
        
        //sift through main memory
        for(int i = index*r; i<index*r+r; i++)
        {
            cout<<"mainMemory["<<i<<"]"<<"="<<vmm[index*tp+ (iter->second >> ps)]<<endl;
            if (mainMemory[i]==vmm[getAddressIndex(iter, addressSpace, k, r)])
            {
                cout<<mainMemory[i]<<"="<<vmm[getAddressIndex(iter, addressSpace, k, r)];
            }
            else
            {
                //faultHandler();
            }
        }
        
    }
    
    

    return 0;
}

