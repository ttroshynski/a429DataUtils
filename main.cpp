/**
 * @file main.cpp
 * @brief Driver for all samples...
 *
 *    - $HeadURL: https://birch/svn/Owl429/429Owl/trunk/samples/main.cpp $
 *    - $Date: 2012-06-15 13:35:53 -0500 (Fri, 15 Jun 2012) $
 *    - $Revision: 3476 $
 *    - $Author: jcole $
 *
 * Copyright (c) Precision Fibre Channel 2007
 *  - Elkhorn, NE USA
 *  - Beavercreek, OH USA
 *  .
 * All rights reserved.  No part of this software may
 * be published, distributed, translated or otherwise
 * reproduced by any means or for any purpose without
 * the prior written consent of Precision Fibre Channel. 
 */

#include <iostream>
#include <Owl429/definitions>

int sample_Acyclic(OwUInt64 aSerialNumber, OwUInt8 aTxChannelNum);
int sample_Asynchronous(OwUInt64 aSerialNumber, OwUInt8 aTxChannelNum);
int sample_BlockTransfer(OwUInt64 aSerialNumber, OwUInt8 aTxChannelNum);
int sample_BoardLogin(OwUInt64 aSerialNumber);
int sample_Continuous(OwUInt64 aSerialNumber, OwUInt8 aTxChanNum, OwUInt8 aRxChanNum);
int sample_DataCorrupter(OwUInt64 aSerialNumber, OwUInt8 aRxChanNum, OwUInt8 aTxChanNum);
int sample_Loopback(OwUInt64 aSerialNumber, OwUInt8 aTxChanNum, OwUInt8 aRxChanNum);
int sample_ScheduledLabel(OwUInt64 aSerialNumber, OwUInt8 aTxChannelNum);
int sample_DynamicScheduledLabel(OwUInt64 aSerialNumber, OwUInt8 aTxChannelNum);
int sample_Discretes(OwUInt64 aSerialNumber);
int sample_LoadedCSV();

/* Loopback samples need a loopback cable linking a Tx channel to an Rx channel */
#define TX_CHAN 1
#define RX_CHAN 5

/**
 * This is a simple runner to run the samples.
 * The samples output error information so all this needs
 * to do is exit.
 */
int main()
{
    //std::cout << "sample_Acyclic:        " << sample_Acyclic(0, TX_CHAN)               << std::endl;
    //std::cout << "sample_Asynchronous:   " << sample_Asynchronous(0, TX_CHAN)          << std::endl;
    //std::cout << "sample_BlockTransfer:  " << sample_BlockTransfer(0, TX_CHAN)         << std::endl;
    //std::cout << "sample_BoardLogin:     " << sample_BoardLogin(0)                     << std::endl;
    //std::cout << "sample_Continuous:     " << sample_Continuous(0, TX_CHAN, RX_CHAN)   << std::endl;
    //std::cout << "sample_DataCorrupter:  " << sample_DataCorrupter(0, RX_CHAN, TX_CHAN) << std::endl;
    //std::cout << "sample_Loopback:       " << sample_Loopback(0, TX_CHAN, RX_CHAN)     << std::endl;
    //std::cout << "sample_ScheduledLabel: " << sample_ScheduledLabel(0, TX_CHAN)        << std::endl;
    //std::cout << "sample_DynamicScheduledLabel: " << sample_DynamicScheduledLabel(0, TX_CHAN) << std::endl;
    //std::cout << "sample_Discretes:      " << sample_Discretes(0)                      << std::endl;
	std::cout << "sample_LoadedCSV:        " << sample_LoadedCSV()                         << std::endl;
    return 0;
}
