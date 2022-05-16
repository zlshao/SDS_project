/**
MIT License

Copyright (c) 2021 hwu(hwu@seu.edu.cn), Szl(zlshao@seu.edu.cn)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <cstring>
#include <time.h>
#include <vector>
#include "_lib.h/libconfig.h++"
#include "_lib.h/libSketchPoolSE.h"
#include "_lib.h/libPcapSE.h"
#include "_lib.h/libCsvStorage.h"
#include "other/bit_conversion.h"

using namespace std;
using namespace libconfig;

bool iterPcapPacket(string name, ISketchPool* lpSKP, int ratio)
{
    bool bout = false;
    CPacket *lppck;
    uint64_t cntPck = 0, cntPckSample = 0;
    uint64_t lenPck = 0, lenPckSample = 0;
    clock_t beginC, endC;

    int iCheckNum = 0;
    //random
    if(ratio>1)
        iCheckNum = rand() % ratio;
    cout << "begin number:" << iCheckNum << " ratio:" << ratio << endl;

    CReader* pr = create_pcap_reader(name.c_str());
    bool bret = pr->openPcap();
    if(bret){
        beginC = clock();

        int iret = 0;
        while(iret>=0){
            if(ratio == 1 || cntPck % ratio == iCheckNum)
            {
                iret = pr->readPacket();
                if(iret>=0){
                    lppck = pr->getPacket();
                    if(lppck && (lppck->getProtocol()==6 || lppck->getProtocol()==17) )     //tcp || udp
                    {         
                        lpSKP->procPacket(lppck);
                    }
                    cntPckSample ++;
                    lenPckSample += lppck->getLenPck();
                }
            }
            else
            {
                iret = pr->nextPacket();
                //iret = pr->readPacket();
            }
            cntPck ++;
            if(!(cntPck & 0xfffff))
                 cout << "count:" << cntPck << endl;
        }
        cout << "================================" << endl;
        cout << "Pck. count:" << cntPck << ", sample Pck. count:" << cntPckSample << endl;
        cout << "sample Pck. length:" << lenPckSample << endl;
        endC = clock();
        double tmLen = (endC - beginC);
        double tmSk = tmLen - pr->getReadTime();
        cout << "--------Time to read  + sketch(ms):" << tmLen*1000.0/(double)CLOCKS_PER_SEC << endl;
        cout << "--------Time to read file(ms):" << pr->getReadTime()*1000.0/(double)CLOCKS_PER_SEC <<endl;
        cout << "--------Time to sketch(ms):" << tmSk*1000.0/(double)CLOCKS_PER_SEC <<endl;
        //saveStatSumSpeed();
    }
    else
        cout << "open pcap file " << name << " error." << endl;

    delete pr;    
    return bout;
}

bool add_SDS_SK(Config* lpCfg, ISketchPool* lpSKP)
{
    bool bout = false;
    if(lpCfg && lpSKP)
    {
        int type, bit, thre;
        //type
        lpCfg->lookupValue("SDS_SK_type", type);
        cout << "SDS sketch type:" << type << endl;
        //bit
        lpCfg->lookupValue("SDS_SK_hash_bit", bit);
        cout << "SDS sketch  length:2^" << bit << endl;
        //threshold
        lpCfg->lookupValue("SDS_SK_threshold", thre);
        cout << "sketch statistics threshold:" << thre << endl;
        //features
        string strFea = lpCfg->lookup("SDS_SK_feature");
        cout << "sketch features:" << strFea << endl;
        uint64_t ufea = convStringValue(strFea);
        //range
        uint32_t uflag = 12;
        vector<int> vctTCP, vctUDP;
        if(ufea & uflag) 
        {
            //have range flag
            vctTCP.clear();
            vctUDP.clear();

            int cntRange;
            lpCfg->lookupValue("SDS_range_count", cntRange);
            cout << "SDS sketch range count:" << cntRange+1 << endl;
            for(int j=1; j<=cntRange; j++)
            {
                int value;
                if(lpCfg->lookupValue("SDS_SK_range_TCP_" + to_string(j), value))
                {
                    vctTCP.push_back(value);
                    cout << "SDS sketch range TCP threshold " + to_string(j) << ":" << value << endl;
                }
                if(lpCfg->lookupValue("SDS_SK_range_UDP_" + to_string(j), value))
                {
                    vctUDP.push_back(value);
                    cout << "SDS sketch range UDP threshold " + to_string(j) << ":" << value << endl;
                }
            }
        }
        if(lpSKP->addSketch((packet_statistics_object_type)type, bit, thre, ufea, &vctTCP, &vctUDP))
            bout = true;
        else
            cout << "SDS sketch setting error!" << endl;
    }
    return true;
}

int main(int argc, char *argv[])
{
    char buf[UINT8_MAX] = "data.cfg";

    if(argc==2)
        strcpy(buf, argv[1]);

    std::cerr << "SDS from pcap file" << std::endl;        

    Config cfg;
    try
    {
        cfg.readFile(buf);
    }
    catch(...)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return(EXIT_FAILURE); 
    }    

    try
    {
        //name
        string name = cfg.lookup("SDS_in_pcap_file");    
        cout << "SDS incoming pcap file name:" << name << endl;

        int seed, rationo, ratio;
        //random seed
        cfg.lookupValue("SDS_random_seed", seed);
        cout << "Random seed:" << seed << endl;
        srand(seed);
        //ratio
        cfg.lookupValue("SDS_ratio", rationo);
        cout << "Sample ratio No.:" << rationo << endl;
        ratio = calSampleRate(rationo);
        cout << "Sample ratio:1/" << ratio << endl;

        if(name.length()>0)
        {
            IEigenvectorStorage* lpStorage = CCsvStorageCreator::createCsvStorage();
            lpStorage->initialPara(name, 0, "");
            ISketchPool* lpSKPool = CSketchPoolCreator::create_sketch_pool(ratio);
            lpSKPool->setStorage(lpStorage);

            if(add_SDS_SK(&cfg, lpSKPool))
                iterPcapPacket(name, lpSKPool, ratio);

            delete lpSKPool;
        }
        else
            cout << "parameter error." << endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return(EXIT_FAILURE);
    }
    std::cerr << "sketch pool end" << std::endl;        

    return 0;
}