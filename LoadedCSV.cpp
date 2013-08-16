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
#include <list>
#include <cstdio>
#include <cstring>
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
#include <Owl429Utils/Xml429.hpp>

// All OWL objects are in the Owl429 namespace
using namespace Owl429;

/**
 * A class to read in data from comma seperated value files and populate Owl objects.
 */
class LoadedCSV
{
public:

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
		 * @brief The value at which to set the rate in FSIM. Derived from minTransitInterval
		 */
		double rate;
		/**
		 * @brief Defines the units of the rate. ms or Hz
		 */
		bool isPeriod;
		/**
		 * @brief The maximum Transit Interval of the data
		 */
		std::string maxTransitInterval;
		/**
		 * @brief The maximum Transport Delay of the data
		 */
		OwUInt16 maxTransportDelay;
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
		 * @brief The value at which to set the rate in FSIM. Derived from minTransitInterval
		 */
		double rate;
		/**
		 * @brief Defines the units of the rate. ms or Hz
		 */
		bool isPeriod;
		/**
		 * @brief The maximum Transit Interval of the data
		 */
		std::string maxTransitInterval;
		/**
		 * @brief The maximum Transport Delay of the data
		 */
		OwUInt16 maxTransportDelay;
	};

	/**
	 * A structure to store label data
	 */
	typedef struct Transmission
	{
		/**
		 * @brief The 9 bit octal identifier of the label
		 */
		OwInt16 codeNo;
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
		/*
		 * @brief A list of all the transmissions the Equipment can produce.
		 */
		std::list<Transmission*> transmissions;
	};

	/**
	 * Loads the equipment data from the EquipmentIDs.csv file
	 */
	void loadEquipmentList(const std::string& aFile )
	{
		if ( aFile.empty() )
		{
			std::invalid_argument("loadEquipmentList: argument aFile is empty");
			return;
		}

		//Clear the equipment list
		equipmentList.clear();

		//Open the equipment file
		FILE* pFile;
		fopen_s( &pFile, aFile.c_str(), "r");
		if( pFile == NULL )
		{
			std::invalid_argument("loadEquipmentList: The given file could not be opened");
			return;
		}

		char line[256];
		char field[256];
		int charPointer;

		//Read in the first line
		fgets( line, 256, pFile );

		//Compare the first line to what we expect
		if( strcmp( line, "\"Equip ID(Hex)\",\"Equipment Type\"\n" ) != 0)
		{
			std::invalid_argument("loadEquipmentList: The first line of the given file was not what was expected");
			return;
		}

		//Start reading in lines
		while( fgets( line, 256, pFile ) != NULL )
		{
			//Create a new equipment structure
			Equipment equipment;
			equipment.id = -1;
			charPointer = 0;

			readField( line, &charPointer, field );	//Read in the ID
			if( field[0] != '\0' )	//If we got something
			{
				equipment.id = (OwInt16)strtol( field, NULL, 16 );	//Parse and store
			}

			readField( line, &charPointer, field);	//read in the Type
			if( field[0] != '\0' )	//We read in something
			{
				equipment.type = std::string(field);	//store it to the equipment
				if( equipment.id != -1 )
				{
					equipmentList.push_back(equipment);		//save the equipment
				}
			}
		}

		fclose(pFile);
	}

	/**
	 * Loads the transmission data from the LabelIDs.csv file. To be run after loadEquipmentList
	 */
	void loadTransmissionList(const std::string& aFile )
	{
		if ( aFile.empty() )
		{
			std::invalid_argument("loadTransmissionList: argument aFile is empty");
			return;
		}

		//Clear the equipment list
		transmissionList.clear();

		//Open the label file
		FILE* pFile;
		fopen_s( &pFile, aFile.c_str(), "r");
		if( pFile == NULL )
		{
			std::invalid_argument("loadTransmissionList: The given file could not be opened");
			return;
		}

		char line[256];
		char field[256];
		int charPointer;

		//Read in the first line
		fgets( line, 256, pFile );

		//Compare the first line to what we expect
		if( strcmp( line, "\"Code No. (Octal)\",,,\"Eqpt. ID (Hex)\",,,\"Transmission Order Bit Position\",,,,,,,,\"Parameter\",\"Data\",,,,\"Notes & Cross Ref. to Tables in Att. 6\"\n" ) != 0)
		{
			std::invalid_argument("loadTransmissionList: The first line of the given file was not what was expected");
			return;
		}

		//Read in the second line
		fgets( line, 256, pFile );
		//Compare the second line to what we expect
		if( strcmp( line, ",,,,,,1,2,3,4,5,6,7,8,,\"BNR\",\"BCD\",\"DISC\",\"SAL\",\n" ) != 0)
		{
			std::invalid_argument("loadTransmissionList: The second line of the given file was not what was expected");
			return;
		}

		//Start reading in lines
		OwInt16 currentCodeNo = 0;
		while( fgets( line, 256, pFile ) != NULL )
		{
			//Create a new transmission structure
			Transmission transmission;
			charPointer = 0;

			readField( line, &charPointer, field );	//Read in the code No
			if( field[0] != '\0')	//If we read something in
			{
				//Remove all the whitespace
				char codeNoString[4] = "\0\0\0";
				for( int i = 0, j = 0; field[i] != '\0'; ++i)
				{
					if( field[i] != ' ' )
					{
						codeNoString[j] = field[i];
						++j;
					}
				}

				//Parse it for the number and update the current Code No
				currentCodeNo = (OwInt16)strtol( codeNoString, NULL, 8 );
			}
			transmission.codeNo = currentCodeNo;	//Save the code No

			readField( line, &charPointer, field );	//These two fields don't have anything
			readField( line, &charPointer, field );

			//Read in the equipment ID
			OwUInt16 equipmentID = 0;
			bool wildcard = false;	//If all three digits are X or Y, then it's a wildcard
			bool valid = true;		//If all three digits aren't numbers, unless it's a wildcard, then it's invalid
			readField( line, &charPointer, field );	//Read in the first digit of the hardware ID
			if( field[0] != '\0' )
			{
				if( field[0] != 'X' && field[0] != 'Y' && field[0] != ' ' )
				{
					equipmentID = equipmentID | (OwInt16)strtol( field, NULL, 16 ) << 8;
				}
				else if( field[0] == 'X' || field[0] == 'Y' )
				{
					wildcard = true;
				}
			}
			readField( line, &charPointer, field );	//Read in the second digit of the hardware ID
			if( field[0] != '\0' )
			{
				if( field[0] != 'X' && field[0] != 'Y' && field[0] != ' ' )
				{
					equipmentID = equipmentID | (OwInt16)strtol( field, NULL, 16 ) << 4;
					if( wildcard == true )
					{
						valid = false;
					}
				}
				else if( field[0] == 'X' || field[0] == 'Y' )
				{
					if( wildcard == false )
					{
						valid = false;
					}
				}
			}
			readField( line, &charPointer, field );	//Read in the third digit of the hardware ID
			if( field[0] != '\0' )
			{
				if( field[0] != 'X' && field[0] != 'Y' && field[0] != ' ' )
				{
					equipmentID = equipmentID | (OwInt16)strtol( field, NULL, 16 );
					if( wildcard == true )
					{
						valid = false;
					}
				}
				else if( field[0] == 'X' || field[0] == 'Y' )
				{
					if( wildcard == false )
					{
						valid = false;
					}
				}
			}

			if( !valid )	//If it isn't valid, move on to the next transmission
			{
				continue;
			}

			//Read in the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = 0;
			readField( line, &charPointer, field );	//Read in the first digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 7;
			readField( line, &charPointer, field );	//Read in the second digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 6;
			readField( line, &charPointer, field );	//Read in the third digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 5;
			readField( line, &charPointer, field );	//Read in the fourth digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 4;
			readField( line, &charPointer, field );	//Read in the fifth digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 3;
			readField( line, &charPointer, field );	//Read in the sixth digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 2;
			readField( line, &charPointer, field );	//Read in the seventh digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 ) << 1;
			readField( line, &charPointer, field );	//Read in the eighth digit of the Transmission Order Bit Position
			transmission.transmissionOrderBitPosition = transmission.transmissionOrderBitPosition | (OwInt8)strtol( field, NULL, 2 );

			//Read in the Parameter
			readField( line, &charPointer, field );
			transmission.parameter.assign(field);

			//Read in the data types
			readField( line, &charPointer, field );
			transmission.bnr = field[0] == 'X';
			readField( line, &charPointer, field );
			transmission.bcd = field[0] == 'X';
			readField( line, &charPointer, field );
			transmission.disc = field[0] == 'X';
			readField( line, &charPointer, field );
			transmission.sal = field[0] == 'X';

			transmission.bcdData = NULL;
			transmission.bnrData = NULL;

			//The next field is the notes and cross references, which aren't important

			//Add the transmission to the list
			this->transmissionList.push_back(transmission);

			//Search for the equipment.
			for( std::list<Equipment>::iterator it = this->equipmentList.begin(); it != this->equipmentList.end(); ++it )
			{
				if( wildcard || it->id == equipmentID )
				{
					it->transmissions.push_back( &(this->transmissionList.back()) );	//Add a pointer to this transmission to the equipment
					if( !wildcard )
					{
						break;
					}
				}
			}

		}

		fclose(pFile);
	}

	/**
	 * Loads the bnr data from the BnrData.csv file. To be run after loadTransmissionList
	 */
	void loadBnrData(const std::string& aFile )
	{
		if ( aFile.empty() )
		{
			std::invalid_argument("loadBnrData: argument aFile is empty");
			return;
		}

		//Open the bnr data file
		FILE* pFile;
		fopen_s( &pFile, aFile.c_str(), "r");
		if( pFile == NULL )
		{
			std::invalid_argument("loadBnrData: The given file could not be opened");
			return;
		}

		char line[256];
		char field[256];
		int charPointer;

		//Read in the first line
		fgets( line, 256, pFile );

		//Compare the first line to what we expect
		if( strcmp( line, "\"Label\",\"Eqpt ID(Hex)\",\"Parameter Name\",\"Units\",\"Range(Scale)\",\"Sig Bits\",\"Pos Sense\",\"Resolution\",\"Min Transit Interval(msec) 2\",\"Max Transit Interval(msec) 2\",\"Max Trans-port Delay(msec) 3\",\"Notes & Cross Ref. to Tables and Attachments\"\n" ) != 0)
		{
			std::invalid_argument("loadBnrData: The first line of the given file was not what was expected");
			return;
		}

		//Start reading in lines
		OwInt16 currentLabel = 0;
		while( fgets( line, 256, pFile ) != NULL )
		{
			if(strcmp(line, ",,,,,,,,,,,\n") == 0 || !(line[0] == '\"' || line[0] == ',') || strlen(line) < 12 )	//If it's an empty line, or if it's invalidly formatted
			{
				continue;
			}
			bool valid = true;

			//Create a new bnr structure
			BNR bnr;
			charPointer = 0;

			readField( line, &charPointer, field );	//Read in the label
			if( field[0] != '\0')	//If we read something in
			{
				//Remove all the whitespace
				char labelString[4] = "\0\0\0";
				for( int i = 0, j = 0; field[i] != '\0'; ++i)
				{
					if( field[i] != ' ' )
					{
						if( j > 3 )
						{
							valid = false;
							break;
						}
						labelString[j] = field[i];
						++j;
					}
				}
				if( !valid ) {continue;}	//Improperly formatted label. Skip this entry
				//Parse it for the number and update the current Label
				currentLabel = (OwInt16)strtol( labelString, NULL, 8 );
			}



			readField( line, &charPointer, field );	//Read in the equipment ID
			OwUInt16 equipmentID = 0;
			bool wildcard = false;
			if( field[0] != '\0')	//If we read something in
			{
				//Remove all the whitespace
				char idString[4] = "\0\0\0";
				for( int i = 0, j = 0; field[i] != '\0'; ++i)
				{
					if( field[i] != ' ' )
					{
						if( j > 3 )
						{
							valid = false;
							break;
						}
						idString[j] = field[i];
						++j;
					}
				}
				if( !valid ) {continue;}	//Improperly formatted label. Skip this entry

				//Parse it for the number and update the equipment Id
				if( strcmp(idString, "XXX") == 0 || strcmp(idString, "YYY") == 0)
				{
					wildcard = true;
				}
				else
				{
					equipmentID = (OwInt16)strtol( idString, NULL, 16 );
				}
			}

			readField( line, &charPointer, field );	//Read in the Parameter Name, but this is redundant info, so don't do anything with it.

			readField( line, &charPointer, field );	//Read in the units
			bnr.units.assign(field);

			readField( line, &charPointer, field );	//Read in the range
			bnr.range.assign(field);

			readField( line, &charPointer, field );	//Read in the sig bits
			bnr.sigBits = 0;
			bnr.sigBits = (OwInt8)strtol( field, NULL, 10 );

			readField( line, &charPointer, field );	//Read in the pos sense
			bnr.posSense.assign(field);

			readField( line, &charPointer, field );	//Read in the resolution
			bnr.resolution.assign(field);

			readField( line, &charPointer, field );	//Read in the min transit interval
			bnr.minTransitInterval.assign(field);

			bnr.rate = 0;
			bnr.isPeriod = true;

			bnr.rate = strtod( field, NULL );
			if( bnr.rate == 0 )
			{
				//The rate is unknown. Skip this bnr
				continue;
			}
			bnr.isPeriod = strstr(field, "Hz") == NULL;	//Check if it's in Hz
			if( strstr( field, "." ) != NULL && bnr.isPeriod == true )	//Check if it's a period with a decimal point
			{
				bnr.rate = 1000 / bnr.rate;	//Convert to Hz
				bnr.isPeriod = false;
			}

			readField( line, &charPointer, field );	//Read in the max transit interval
			bnr.maxTransitInterval.assign(field);

			readField( line, &charPointer, field );	//Read in the max transport delay
			bnr.maxTransportDelay = 0;
			if( field[0] != '\0' )
			{
				bnr.maxTransportDelay = (OwUInt16)strtol( field, NULL, 10);
			}

			//Add the bnr to the list
			this->bnrList.push_back(bnr);
			BNR* bnrReference = &(this->bnrList.back());

			//Search for the equipment.
			bool breakFlag = false;
			for( std::list<Equipment>::iterator it = this->equipmentList.begin(); it != this->equipmentList.end() && !breakFlag; ++it )
			{
				if( wildcard || it->id == equipmentID )
				{
					for( std::list<Transmission*>::iterator it2 = it->transmissions.begin(); it2 != it->transmissions.end(); ++it2 )	//Search for the label
					{
						if( (*it2)->codeNo == currentLabel )
						{
							(*it2)->bnrData = bnrReference;	//Add the bnr reference
							breakFlag = true;
							break;
						}
					}
				}
			}

		}

		fclose(pFile);
	}

	/**
	 * Loads the bcd data from the BcdData.csv file. To be run after loadTransmissionList
	 */
	void loadBcdData(const std::string& aFile )
	{
		if ( aFile.empty() )
		{
			std::invalid_argument("loadBcdData: argument aFile is empty");
			return;
		}

		//Open the bnr data file
		FILE* pFile;
		fopen_s( &pFile, aFile.c_str(), "r");
		if( pFile == NULL )
		{
			std::invalid_argument("loadBcdData: The given file could not be opened");
			return;
		}

		char line[256];
		char field[256];
		int charPointer;

		//Read in the first line
		fgets( line, 256, pFile );

		//Compare the first line to what we expect
		if( strcmp( line, "\"Label\",\"Eqpt ID(Hex)\",\"Parameter Name\",\"Units\",\"Range(Scale)\",\"Sig Bits\",\"Pos Sense\",\"Resolution\",\"Min Transit Interval(msec) 2\",\"Max Transit Interval(msec) 2\",\"Max Trans-port Delay(msec) 3\",\"Notes & Cross Ref. to Tables and Attachments\"\n" ) != 0)
		{
			std::invalid_argument("loadBcdData: The first line of the given file was not what was expected");
			return;
		}

		//Start reading in lines
		OwInt16 currentLabel = 0;
		while( fgets( line, 256, pFile ) != NULL )
		{
			if(strcmp(line, ",,,,,,,,,,,\n") == 0 || !(line[0] == '\"' || line[0] == ',') || strlen(line) < 12 )	//If it's an empty line, or if it's invalidly formatted
			{
				continue;
			}
			bool valid = true;

			//Create a new bnr structure
			BCD bcd;
			charPointer = 0;

			readField( line, &charPointer, field );	//Read in the label
			if( field[0] != '\0')	//If we read something in
			{
				//Remove all the whitespace
				char labelString[4] = "\0\0\0";
				for( int i = 0, j = 0; field[i] != '\0'; ++i)
				{
					if( field[i] != ' ' )
					{
						if( j > 3 )
						{
							valid = false;
							break;
						}
						labelString[j] = field[i];
						++j;
					}
				}
				if( !valid ) {continue;}	//Improperly formatted label. Skip this entry
				//Parse it for the number and update the current Label
				currentLabel = (OwInt16)strtol( labelString, NULL, 8 );
			}



			readField( line, &charPointer, field );	//Read in the equipment ID
			OwUInt16 equipmentID = 0;
			bool wildcard = false;
			if( field[0] != '\0')	//If we read something in
			{
				//Remove all the whitespace
				char idString[4] = "\0\0\0";
				for( int i = 0, j = 0; field[i] != '\0'; ++i)
				{
					if( field[i] != ' ' )
					{
						if( j > 3 )
						{
							valid = false;
							break;
						}
						idString[j] = field[i];
						++j;
					}
				}
				if( !valid ) {continue;}	//Improperly formatted label. Skip this entry

				//Parse it for the number and update the equipment Id
				if( strcmp(idString, "XXX") == 0 || strcmp(idString, "YYY") == 0)
				{
					wildcard = true;
				}
				else
				{
					equipmentID = (OwInt16)strtol( idString, NULL, 16 );
				}
			}

			readField( line, &charPointer, field );	//Read in the Parameter Name, but this is redundant info, so don't do anything with it.

			readField( line, &charPointer, field );	//Read in the units
			bcd.units.assign(field);

			readField( line, &charPointer, field );	//Read in the range
			bcd.range.assign(field);

			readField( line, &charPointer, field );	//Read in the sig bits
			bcd.sigBits = 0;
			bcd.sigBits = (OwInt8)strtol( field, NULL, 10 );

			readField( line, &charPointer, field );	//Read in the pos sense
			bcd.posSense.assign(field);

			readField( line, &charPointer, field );	//Read in the resolution
			bcd.resolution.assign(field);

			readField( line, &charPointer, field );	//Read in the min transit interval
			bcd.minTransitInterval.assign(field);

			bcd.rate = 0;
			bcd.isPeriod = true;

			bcd.rate = strtod( field, NULL );
			if( bcd.rate == 0 )
			{
				//The rate is unknown. Skip this bnr
				continue;
			}
			bcd.isPeriod = strstr( field, "Hz") == NULL;	//Check if it's in Hz
			if( strstr( field, "." ) != NULL && bcd.isPeriod == true )	//Check if it's a period with a decimal point
			{
				bcd.rate = 1000 / bcd.rate;	//Convert to Hz
				bcd.isPeriod = false;
			}

			readField( line, &charPointer, field );	//Read in the max transit interval
			bcd.maxTransitInterval.assign(field);

			readField( line, &charPointer, field );	//Read in the max transport delay
			bcd.maxTransportDelay = 0;
			if( field[0] != '\0' )
			{
				bcd.maxTransportDelay = (OwUInt16)strtol( field, NULL, 10);
			}

			//Add the bnr to the list
			this->bcdList.push_back(bcd);
			BCD* bcdReference = &(this->bcdList.back());

			//Search for the equipment.
			bool breakFlag = false;
			for( std::list<Equipment>::iterator it = this->equipmentList.begin(); it != this->equipmentList.end() && !breakFlag; ++it )
			{
				if( wildcard || it->id == equipmentID )
				{
					for( std::list<Transmission*>::iterator it2 = it->transmissions.begin(); it2 != it->transmissions.end(); ++it2 )	//Search for the label
					{
						if( (*it2)->codeNo == currentLabel )
						{
							(*it2)->bcdData = bcdReference;	//Add the bnr reference
							breakFlag = true;
							break;
						}
					}
				}
			}

		}

		fclose(pFile);
	}

	/**
	 * A function to convert a LoadedCSV to Owl429 objects and dump them to Xml
	 */
	void save()
	{
		for( std::list<Equipment>::iterator it = this->equipmentList.begin(); it != this->equipmentList.end(); ++it)
		{
			TxRateOrientedConfig txRateOrientedConfig = TxRateOrientedConfig();
			RxChronMonConfig rxChronMonConfig = RxChronMonConfig();
			LabelBufferConfig labelBufferConfig = LabelBufferConfig(1);
			//Set the Channel Name
			txRateOrientedConfig.setName(it->type);
			//Add the Transfers
			for( std::list<Transmission*>::iterator it2 = it->transmissions.begin(); it2 != it->transmissions.end(); ++it2)
			{
				TxScheduledLabelConfig txScheduledLabelConfig = Owl429::TxScheduledLabelConfig((OwUInt8)(*it2)->codeNo);
				//Set the transfer name
				txScheduledLabelConfig.setName((*it2)->parameter);
				//Set some of the other values
				if( (*it2)->bcd && (*it2)->bcdData != NULL )
				{
					//Set the Rate
					if( (*it2)->bcdData->isPeriod )
					{
						txScheduledLabelConfig.setTransferPeriod( (OwUInt32)(*it2)->bcdData->rate );
					}
					else
					{
						txScheduledLabelConfig.setTransferRate( (*it2)->bcdData->rate );
					}
				}
				else if( (*it2)->bnr && (*it2)->bnrData != NULL )
				{
					//Set the Rate
					if( (*it2)->bnrData->isPeriod )
					{
						txScheduledLabelConfig.setTransferPeriod( (OwUInt32)(*it2)->bnrData->rate );
					}
					else
					{
						txScheduledLabelConfig.setTransferRate( (*it2)->bnrData->rate );
					}
				}
				else
				{
					//Unknown data type
					continue;
				}
				//Add the Transfer
				txRateOrientedConfig.addTransfer(txScheduledLabelConfig);
				rxChronMonConfig.addLabelBufferConfig((OwUInt8)((*it2)->codeNo), labelBufferConfig, (*it2)->parameter);
			}
			txRateOrientedConfig.setMonitorConfig(rxChronMonConfig);
			//save to xml
			std::string xmlFileName;

			//Owl429Utils::Xml429::save( txRateOriented, 1, xmlFileName);
		}
		return;
	}

private:

	/**
	 * A helper function to parse out fields from csv files
	 * @param inputString the string to parse
	 * @param startIndex the index to begin parsing from.
	 * This should point to the beginning quotation mark,
	 * and returns pointing to the beginning of the next field.
	 * @param outputString the string to store the parsed field to
	 * @returns the index of the char after the last quotation mark of the parsed field
	 */
	void readField( const char* inputString, int* startIndex, char* outputString)
	{
		int i = *startIndex; //inputString index
		int o = 0; //outputString index
		if( inputString[*startIndex] != ',' && inputString[*startIndex] != '\n' && inputString[*startIndex] != '\0') //If there is something to read
		{
			if( inputString[*startIndex] == '\"')	//If the field is surrounded by quotation marks
			{
				i++;
				while( inputString[i] != '\"' )	//while we aren't at the end of the field
				{
					outputString[o] = inputString[i];	//copy the char
					i++;	//increment
					o++;	//increment
				}
				i++;
			}
			else	//If the field isn't surrounded by quotation marks
			{
				while( inputString[i] != ',' )	//while we aren't at the end of the field
				{
					outputString[o] = inputString[i];	//copy the char
					i++;	//increment
					o++;	//increment
				}
			}
		}
		outputString[o] = '\0';	//append a null Terminating character
		if( inputString[*startIndex] != '\0' )
		{
			*startIndex = i + 1;	//return the index of the next field
		}
		return;
	}

	std::list<Equipment> equipmentList;
	std::list<Transmission> transmissionList;
	std::list<BNR> bnrList;
	std::list<BCD> bcdList;
};


/**
 * A sample program for loading from csv.
 *
 * @return 0 for success or 1 on error.
 */
int sample_LoadedCSV()
{
	LoadedCSV loadedCsv;
	loadedCsv.loadEquipmentList("C:\\Users\\Evan\\Downloads\\ARINC429P1-18-EquipmentIDs.csv");
	loadedCsv.loadTransmissionList("C:\\Users\\Evan\\Downloads\\ARINC429P1-18-LabelIDs.csv");
	loadedCsv.loadBnrData("C:\\Users\\Evan\\Downloads\\ARINC429P1-18-BnrData.csv");
	loadedCsv.loadBcdData("C:\\Users\\Evan\\Downloads\\ARINC429P1-18-BcdData.csv");
	loadedCsv.save();
	return 0;
}