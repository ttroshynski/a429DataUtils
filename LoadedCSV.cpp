/**
 * @file Continuous.cpp
 * @brief OWL 429 sample code for loading a comma seperated values file into Owl Objects.
 *
 *    - $HeadURL: https://birch/svn/Owl429/429Owl/trunk/samples/LoadedCSV.cpp $
 *    - $Date: 2012-05-29 14:28:20 -0500 (Tue, 29 May 2012) $
 *    - $Revision: 3424 $
 *    - $Author: nveys $
 *
 * Copyright (c) Avionics Interface Technologies
 *  - Omaha, NE USA
 *  - Beavercreek, OH USA
 *  .
 * All rights reserved.  No part of this software may
 * be published, distributed, translated or otherwise
 * reproduced by any means or for any purpose without
 * the prior written consent of AIT. 
 */

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <cstdio>
#include <cstdlib>

#include <Owl429/ArincUtil>
#include <Owl429/BoardConfig>
#include <Owl429/IBoard>
#include <Owl429/IBoardFactory>
#include <Owl429/IBoardInfo>
#include <Owl429/IBufferEntry>
#include <Owl429/IChannelVariant>
#include <Owl429/IRxChronMonChannel>
#include <Owl429/ITxRateOrientedChannel>
#include <Owl429/ITxRateOrientedTransfer>
#include <Owl429/ITxScheduledLabel>
#include <Owl429/RxChronMonConfig>
#include <Owl429/RxChronMonEventHandler>
#include <Owl429/SmartPtr>
#include <Owl429/TxRateOrientedConfig>
#include <Owl429/TxScheduledLabelConfig>

// All OWL objects are in the Owl429 namespace
using namespace Owl429;

/**
 * A class to read in data from comma seperated value files and populate Owl objects.
 */
class LoadedCSV
{
public:

	/**
	 * A structure to store equipment data
	 */
	typedef struct Equipment
	{
		/**
		 * @brief The 12 bit hexadecimal identifier for the equipment
		 */
		OwInt16 id;
		/**
		 * @brief The type of the equipment
		 */
		std::string type;
	};

	/**
	 * A structure to store bnr data
	 */
	typedef struct BNR
	{
		/**
		 * @brief The units of the data
		 */
		std::string units;
		/**
		 * @brief The range of the data
		 */
		std::string range;
		/**
		 * @brief The number of significant bits of the data
		 */
		OwUInt8 sigBits;
		/**
		 * @brief The Pos Sense of the data
		 */
		std::string posSense;
		/**
		 * @brief The resolution of the data
		 */
		std::string resolution;
		/**
		 * @brief The minimum Transit Interval of the data
		 */
		std::string minTransitInterval;
		/**
		 * @brief The maximum Transit Interval of the data
		 */
		std::string maxTransitInterval;
	};

	/**
	 * A structure to store bcd data
	 */
	typedef struct BCD
	{
		/**
		 * @brief The units of the data
		 */
		std::string units;
		/**
		 * @brief The range of the data
		 */
		std::string range;
		/**
		 * @brief The number of significant bits of the data
		 */
		OwUInt8 sigBits;
		/**
		 * @brief The Pos Sense of the data
		 */
		std::string posSense;
		/**
		 * @brief The resolution of the data
		 */
		std::string resolution;
		/**
		 * @brief The minimum Transit Interval of the data
		 */
		std::string minTransitInterval;
		/**
		 * @brief The maximum Transit Interval of the data
		 */
		std::string maxTransitInterval;
	};

	/**
	 * A structure to store label data and join the other structures
	 */
	typedef struct Transmission
	{
		/**
		 * @brief The 9 bit octal identifier of the label
		 */
		OwInt16 codeNo;
		/**
		 * @brief The equipment
		 */
		Equipment* equipment;
		/**
		 * @brief The Transmission Order Bit Position
		 */
		OwUInt8 transmissionOrderBitPosition;
		/**
		 * @brief The Parameter
		 */
		std::string parameter;
		/**
		 * @brief Denotes whether or not the data is Binary
		 */
		bool bnr;
		/**
		 * @brief Denotes whether or not the data is Binary Coded Decimal
		 */
		bool bcd;
		/**
		 * @brief Denotes whether or not the data is Discrete
		 */
		bool disc;
		/**
		 * @brief Denotes whether or not the data is System Address Label
		 */
		bool sal;
		/**
		 * @brief a pointer to the bnr data if any
		 */
		BNR* bnrData;
		/**
		 * @brief a pointer to the bcd data if any
		 */
		BCD* bcdData;
	};

	/**
	 * Loads the equipment data from the EquipmentIDs.csv file
	 */
	void loadEquipmentList(const std::string& aFile ){
		//Clear the equipment list
		equipmentList.clear();

		//Open the equipment file
		FILE* pFile;
		fopen_s( &pFile, aFile.c_str(), "r");
		if( pFile == NULL )
		{
			std::invalid_argument("loadEquipmentList: The given file could not be opened");
		}

		char line[256];
		char field[256];
		int charPointer;

		//Read in the first line
		fgets( line, 256, pFile );

		//Compare the first line to what we expect
		if( strcmp( line, "\"Equip ID(Hex)\",\"Equipment Type\"" ) != 0)
		{
			std::invalid_argument("loadEquipmentList: The first line of the given file was not what was expected");
		}

		//Start reading in lines
		while( fgets( line, 256, pFile ) != NULL )
		{
			//Create a new equipment structure
			Equipment equipment;
			equipment.id = -1;

			charPointer = readField( line, field );	//Read in the ID

			if( field[0] != '\0' )	//If we got something
			{
				equipment.id = (OwInt16)strtol( field, NULL, 16 );
			}

			if( line[charPointer] == ',' )	//We expect another field
			{
				charPointer = readField( &line[charPointer + 1], field);	//read in the Type
				if( field[0] != '\0' )	//We read in something
				{
					equipment.type = std::string(field);	//store it to the equipment
					equipmentList.push_back(equipment);		//save the equipment
				}
			}
		}

		fclose(pFile);
	}

private:

	/**
	 * A helper function to parse out fields from csv files
	 * @param inputString the string to parse
	 * @param outputString the string to store the parsed field to
	 * @returns the index of the char after the last quotation mark of the parsed field
	 */
	int readField( const char* inputString, char* outputString)
	{
		int i = 0; //inputString index
		int o = 0; //outputString index
		if( inputString[i] == '\"' )	//If the input String has a quotation mark surrounded field
		{
			i++;	//go to the first char of the field
			while( inputString[i] != '\"' )	//while we aren't at the end of the field
			{
				outputString[o] = inputString[i];	//copy the char
				i++;	//increment
				o++;	//increment
			}
		}
		outputString[o] = '\0';	//append a null Terminating character
		return i + 1;	//return the index of the char after the last quotation mark of the parsed field
	}

	std::vector<Equipment> equipmentList;
	std::vector<Transmission> transmissionList;
};


/**
 * A sample program for loading from csv.
 *
 * @return 0 for success or 1 on error.
 */
int sample_LoadedCSV()
{
	LoadedCSV loadedCsv;
	loadedCsv.loadEquipmentList("");
	return 0;
}