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
    std::cout << "sample_LoadedCSV:        " << sample_LoadedCSV()                         << std::endl;
    return 0;
}
