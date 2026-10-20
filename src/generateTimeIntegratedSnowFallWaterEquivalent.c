/* generate Time Integrated Snow Fall Water Equivalent
**
**  Generates a valid tigge file for T.I. snow-fall water equivalent.
**
** Steven Anthony, 2007
** 	Modified by Dan Swank 2008
**
*/
#include "ncdcTigge.h"

void generateTimeIntegratedSnowFallWaterEquivalent(int yyyy, int mm, int dd, int hh, int fff, int em, void** resource)
	{
// printf("\nStart : generateTimeIntegratedSnowFallWaterEquivalent\nARGS [%04d %02d %02d %02d:00 fct:%03d mem:%02d]\n",yyyy,mm,dd,hh,fff,em);

	// ------  DECLARATIONS ------

	// Primitives
    long temp = 0;
    int i;
//    int errorCode;
    char fileName[256];

	// SIZE DEFS
	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,2);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,2);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,2);
	
	// GRIB RECORD HANDLES
	grib_handle* gh = NULL;
    grib_handle* ncepTotalRainfall = NULL;
	grib_handle* ncepCategoricalSnowfall = NULL;
    grib_handle* ncepGribRecord = NULL;

	// DATA ARRAYS     (double array pointers)
    double* runningSum = NULL;
    double* tiggeTimeIntegratedSnowFallWaterEquivalentValues = NULL;
    double* ncepValues = NULL;

    // START

    // Define array to hold time integration values
    runningSum = calloc(ncepValuesSize, sizeof(double));
    // Ensure allocation was successful
    if(runningSum == NULL)
		{
		  fprintf(stderr, "Failed to allocate memory for Time Integrated Surface Sensible Heat Flux.  Exiting...\n");
		  exit(1);
		}
	
    // create a date/time structure with the initial time
    struct dt tmInit,tmEnd;
    tmInit.yr = yyyy;
    tmInit.mo = mm; 
    tmInit.dy = dd;  
    tmInit.hr = hh;  
    tmInit.mn = 0;  
	
    
    /* for scaling info */
    //long ncepRainfallDecimalScaleFactor;
    //long ncepRainfallBinaryScaleFactor;
    //long ncepRainfallReferenceValue;
    
    
    
    for(fff = 0; fff <= FORECAST_HOURS; fff+=6)
      {
	
	int resourceIndex = (int) (fff/6);
	
	if( resource[resourceIndex] == NULL )
	  { 
	    printf("Resource Index [%d] not available\n",resourceIndex); 
	    continue;
	  }
	
	if( bytesPerRecord[TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT][resourceIndex] <= 0 ) 
	  { 
	    printf("Sizing Index [%02d][%02d] not available or not set",
		   resourceIndex,TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT);
	    continue;
	  }
	
	// populate tmEnd with forecast time offset, add to initial time 
	tmEnd.yr = 0;
	tmEnd.mo = 0; 
	tmEnd.dy = 0;  
	tmEnd.hr = fff;  
	tmEnd.mn = 0;  
	timeAdd (&tmInit, &tmEnd);
	
	ncepGribRecord = grib_handle_new_from_message(NULL, resource[resourceIndex],
						      bytesPerRecord[TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT][resourceIndex]);
	GRIB_CHECK(grib_get_long(ncepGribRecord, "parameterNumber", &temp), 0);
	
	// Read data section --  Setup data array
	ncepValues = calloc(ncepValuesSize, sizeof(double));
	GRIB_CHECK(grib_get_double_array(ncepGribRecord, "values", ncepValues,
					 &ncepValuesSize), 0);
	
	// conjour the output fileName
	memset(fileName, 0, 128); //reset the string.
	generateFileName(TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT,
			 yyyy, mm, dd, hh, fff, em, fileName);
	
	//printf("%s\n", fileName);
	// reset the current fff's values.
        tiggeTimeIntegratedSnowFallWaterEquivalentValues = 
	  calloc(ncepValuesSize, sizeof(double));
	
        if( fff == 0)
	  {
            //	Forecast hr 0's values are always zero
            for( i=0; i<ncepValuesSize; i++ )
	      {
                runningSum[i] = 0.0;
                tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] = 0.0;
	      }
	  }
	else{
	  for(i=0; i<ncepValuesSize; i++)
	    {
	      // Do the integration
	      runningSum[i] += ncepValues[i];
	      tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] =
		runningSum[i];
	      
	      //		not sure why this was being divided, so removing it.
	      //				tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] =
	      //						 (runningSum[i] / (fff*3600.0));
	      
	    }
	}
	
	// now create a new grib_handle to store the data.
	gh = newGribRecord( yyyy, mm, dd, hh, fff, em, n_lat, n_lon );
	// Set MetaData
	GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 11), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterCategory", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterNumber", 53), 0);
	GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 1), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfFirstFixedSurface"), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaledValueOfFirstFixedSurface"), 0);
	GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 255), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfSecondFixedSurface"), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaledValueOfSecondFixedSurface"), 0);
	GRIB_CHECK(grib_set_long(gh, "forecastTime", 0), 0);
	
	
	// Set the "forecast/valid" dates/times
	GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", (long)tmEnd.yr), 0);
	GRIB_CHECK(grib_set_long(gh, "monthOfEndOfOverallTimeInterval", (long)tmEnd.mo), 0);
	GRIB_CHECK(grib_set_long(gh, "dayOfEndOfOverallTimeInterval", (long)tmEnd.dy), 0);
	GRIB_CHECK(grib_set_long(gh, "hourOfEndOfOverallTimeInterval", (long)tmEnd.hr), 0);
	GRIB_CHECK(grib_set_long(gh, "minuteOfEndOfOverallTimeInterval", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "secondOfEndOfOverallTimeInterval", 0), 0);
	
	
	//GRIB_CHECK(grib_set_long(gh, "numberOfTimeRangeSpecificationsDescribingTheTimeIntervalsUsedToCalculateTheStatisticallyProcessedField", 1), 0);
	GRIB_CHECK(grib_set_long(gh, 
				 "numberOfMissingInStatisticalProcess", 0), 0);
	
	// set MORE MetaData slots
	GRIB_CHECK(grib_set_long(gh, "typeOfStatisticalProcessing", 1), 0);
	GRIB_CHECK(grib_set_long(gh, 
				 "typeOfTimeIncrementBetweenSuccessiveFieldsUsedInTheStatisticalProcessing",
				 2), 0);
	GRIB_CHECK(grib_set_long(gh, 
				 "indicatorOfUnitOfTimeRange",
				 1), 0);
	GRIB_CHECK(grib_set_long(gh, 
				 "lengthOfTimeRange", fff), 0);
	GRIB_CHECK(grib_set_long(gh, 
				 "indicatorOfUnitForTimeIncrement",
				 255), 0);
	GRIB_CHECK(grib_set_long(gh, "timeIncrement", 0), 0);
	
	GRIB_CHECK(grib_set_long(gh, "startStep", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "endStep", fff), 0);
	GRIB_CHECK(grib_set_long(gh, "lengthOfTimeRange", fff), 0);
	
	
        // store the data into the grib file
        GRIB_CHECK( grib_set_double_array(gh, "values", 
					  tiggeTimeIntegratedSnowFallWaterEquivalentValues, ncepValuesSize),  0);
	
	// Outputs the resulting file!
        writeGribToFile(gh, fileName);
	
	// clean up after ourselves.
	grib_handle_delete(gh);
	free(ncepValues);
	free(tiggeTimeIntegratedSnowFallWaterEquivalentValues);
	grib_handle_delete(ncepGribRecord);
      }
    
    // delete grib_handles
    grib_handle_delete(ncepCategoricalSnowfall);
    grib_handle_delete(ncepTotalRainfall);
    
    // rewind the file
    //    rewind(aFile);
    free( runningSum );
    
    // printf("\nDone : generateTimeIntegratedSnowFallWaterEquivalent\n\n");
    return;
	}




/*


	** Section 1.  read the A file (from NCEP)
	
	// first, is the file here?
	if(aFile == NULL)
		{
		fprintf(stderr, "Error, could not read the required file.  Exiting generateSnowFallWaterEquivalent()\n");
		printf("\nDone : generateTimeIntegratedSnowFallWaterEquivalent\n\n");
		return;
		}

	
	// first create the new skeleton file.
	gh = newGribRecord(yyyy, mm, dd, hh, fff, em, ncepValuesSize);

	// name format from:   http://tigge.ecmwf.int/ldm_protocol.html
	generateFileName(TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT,
		 yyyy, mm, dd, hh, fff, em, fileName);

// start plugging in the metadata.
// Set all the stuff up that defines the "snow fall water equivalent" variable
// product definition template number, this means that it conforms to template 4.XX, where XX is this number

	GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 11), 0);
	
	GRIB_CHECK(grib_set_long(gh, "parameterCategory", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterNumber", 53), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 1), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfFirstFixedSurface"), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaledValueOfFirstFixedSurface"), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 255), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfSecondFixedSurface"), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaledValueOfSecondFixedSurface"), 0);
	
	GRIB_CHECK(grib_set_long(gh, "forecastTime", (long)fff), 0);

	// required date transform
	// date + fff = newDate
	timeStructure->tm_hour += (fff + 6);
	mktime(timeStructure);	

	GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", (long)timeStructure->tm_year + 1900 ), 0);
	GRIB_CHECK(grib_set_long(gh, "monthOfEndOfOverallTimeInterval", (long)timeStructure->tm_mon + 1 ), 0);
	GRIB_CHECK(grib_set_long(gh, "dayOfEndOfOverallTimeInterval", (long)timeStructure->tm_mday ), 0);
	GRIB_CHECK(grib_set_long(gh, "hourOfEndOfOverallTimeInterval", (long)timeStructure->tm_hour ), 0);
	GRIB_CHECK(grib_set_long(gh, "minuteOfEndOfOverallTimeInterval", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "secondOfEndOfOverallTimeInterval", 0), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "numberOfTimeRangeSpecificationsDescribingTheTimeIntervalsUsedToCalculateTheStatisticallyProcessedField", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "numberOfMissingInStatisticalProcess", 0), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfStatisticalProcessing", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "typeOfTimeIncrementBetweenSuccessiveFieldsUsedInTheStatisticalProcessing", 2), 0);
	
	GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitOfTimeRange", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "lengthOfTimeRange", 6), 0);
	GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitForTimeIncrement", 255), 0);
	GRIB_CHECK(grib_set_long(gh, "timeIncrement", 0), 0);
	
	// do the data section.
	ncepDiscipline = ncepParameterCategory = ncepParameterNumber = -1;


	// 	OBTAIN THE PRECIP-TYPE RECORD / DATA SECTION

	// only do the search if it is not the 000 forecast hour.
	if(fff != 0)
		{
		// while there is no match on the ncep variable.
		while( (ncepDiscipline != 0) || (ncepParameterCategory != 1) || (ncepParameterNumber != 195) )
			{
			if(ncepCategoricalSnowfall != NULL)
				{
				grib_handle_delete(ncepCategoricalSnowfall);
				ncepCategoricalSnowfall = NULL;
				}
			ncepCategoricalSnowfall = grib_handle_new_from_file(0, aFile, &errorCode);
			if(ncepCategoricalSnowfall == NULL)
				{
				fprintf(stderr, "Error, There was a problem reading the grib_handle.  Exiting generateSnowFallWaterEquivalent()\n");
				return;	
				}
			// set the 3 numbers up for determining the product.
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "parameterNumber", &ncepParameterNumber), 0);
			}
		
		// test the data.
		GRIB_CHECK(grib_get_size(ncepCategoricalSnowfall, "values", &ncepCategoricalSnowfallSize), 0);
		//printf("ncepCategoricalSnowfallSize = %d\n", ncepCategoricalSnowfallSize);
		//printf("%ld - %ld - %ld\n", ncepDiscipline, ncepParameterCategory, ncepParameterNumber);

		ncepCategoricalSnowfallValues = calloc(ncepCategoricalSnowfallSize, 
				sizeof(double));
		if(ncepCategoricalSnowfallValues == NULL)
			{
			fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Categorical Rainfall.   Exiting...\n");
			exit(1);
			}

		int rtn = 0;
//		rtn = GRIB_CHECK(grib_get_double_array(ncepCategoricalSnowfall, "values", ncepCategoricalSnowfallValues, &ncepCategoricalSnowfallSize), 0);
        rtn = grib_get_double_array(ncepCategoricalSnowfall, "values", ncepCategoricalSnowfallValues, &ncepCategoricalSnowfallSize);

		if( rtn != 0 ) 
			{
			fprintf(stderr,"generateTimeIntegratedSnowFallWaterEquivalent : Encountered a problem loading %d bytes into buffer ncepCategoricalSnowfallValues* !  Received error-code %d from the GRIB API call grib_get_double_array()",
				ncepCategoricalSnowfallSize, rtn);
			free( ncepCategoricalSnowfallValues );
			exit(1);
			}


    //  OBTAIN THE TOTAL-RAIN RECORD / DATA SECTION

		//
		// now that the ncepCategoricalSnowfall is filled
		// 	 we must rewind and scan for the total precipitation record
		//   then fill the array
		//

		rewind(aFile); // reset it to the beginning of the file 
		ncepDiscipline = ncepParameterCategory = ncepParameterNumber = -1;
		while( (ncepDiscipline != 0) || (ncepParameterCategory != 1) || (ncepParameterNumber != 8) )
			{
			if(ncepTotalRainfall != NULL)
				{
				grib_handle_delete(ncepTotalRainfall);
				ncepTotalRainfall = NULL;
				}
			
			ncepTotalRainfall = grib_handle_new_from_file(0, aFile, &errorCode);
			if(ncepTotalRainfall == NULL)
				{
				fprintf(stderr, "Error, There was a problem reading the grib_handle.  Exiting generateSnowFallWaterEquivalent()\n");
				return;
				}

			// set the 3 numbers up for determining the product.
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "parameterNumber", &ncepParameterNumber), 0);
			//printf("Discipline = %ld   Category = %ld   Number = %ld\n", ncepDiscipline, ncepParameterCategory, ncepParameterNumber);  
			}

		// we have the grib_handle for APCP now.
		GRIB_CHECK(grib_get_size(ncepTotalRainfall, "values", &ncepTotalRainfallSize), 0);
		ncepTotalRainfallValues = calloc(ncepTotalRainfallSize, sizeof(double));
		if(ncepTotalRainfallValues == NULL)
			{
			fprintf(stderr, "Error, There was a problem Allocating the memory for storing the APCP data. Exiting generateSnowFallWaterEquivalent()\n");
			exit(1);
			}

		GRIB_CHECK(grib_get_double_array(ncepTotalRainfall, "values", ncepTotalRainfallValues, &ncepTotalRainfallSize), 0);
		

	    printf(" TEST POINT * * * * ? ? ?\n");


		// save off some scaling information.
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "decimalScaleFactor", &ncepRainfallDecimalScaleFactor), 0);
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "binaryScaleFactor", &ncepRainfallBinaryScaleFactor), 0);;
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "referenceValue", &ncepRainfallReferenceValue), 0);
		}
	
	// Allocate and initialize (nx*ny) doubles - to hold the data
	//		& Perform allocation check
	tiggeTimeIntegratedSnowFallWaterEquivalentValues =
		 calloc(ncepValuesSize, sizeof(double));
	if( tiggeTimeIntegratedSnowFallWaterEquivalentValues == NULL)
		{
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generateTimeIntegratedSnowFallWaterEquivalent()\n");
		exit(1);
		}	
	
	// for each point in the grid.
	for(i=0; i<ncepValuesSize; i++)
		{
			// Entire grid undefined @ fct=0
		if(fff == 0)
			{
			tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] = 0.0;
			}
		else
			{
			if(ncepCategoricalSnowfallValues[i] == 1)
				{
					// if it has snowed
				tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] = 
					ncepTotalRainfallValues[i];

		// These were to test if scaling was automatically done.
		//  	test results concluded that they WERE in fact scaled by the api
		//	int value = ( ncepRainfallReferenceValue + (0.0 + ncepTotalRainfallValues[i]) * pow(2.0, ncepRainfallBinaryScaleFactor) ) / pow(10.0, ncepRainfallDecimalScaleFactor);
		//printf("Unscaled Value: %f    Scaled Value: %f\n", ncepTotalRainfallValues[i], value);

				}
            // if precip type is not 100% snow
			else
				{
				tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] = 0.0;
				}
			}
		}
	
	// store the new data into the grib file
	GRIB_CHECK( grib_set_double_array(gh, "values", tiggeTimeIntegratedSnowFallWaterEquivalentValues, ncepValuesSize),  0);
	
	if( writeGribToFile(gh, fileName) != 0 )
		{
		printf("ERROR, Write failed for file : \"%s\"!\n", fileName);
		}

    printf(" TEST POINT !  \n");

	// free allocated memory
	free( ncepCategoricalSnowfallValues );
	free( ncepTotalRainfallValues );
	free( tiggeTimeIntegratedSnowFallWaterEquivalentValues );
	
    printf(" TEST POINT ! ! \n");
	
	// delete grib_handles
	grib_handle_delete(gh);
	grib_handle_delete(ncepCategoricalSnowfall);
	grib_handle_delete(ncepTotalRainfall);

    printf(" TEST POINT ! ! ! \n");

	// rewind the file
 	rewind(aFile);

    free( runningSum );

	printf("\nDone : generateTimeIntegratedSnowFallWaterEquivalent\n\n");
	}
*/
//  END primary sub





/*
** loadDataForTimeIntegratedSurfaceNetSolarRadiation()
**  Loads the required data needed to calculate the sshf variable.
**
** Parameters:
**  void** gribBuffers    The array of pointers where the data is to be stored
**
** Returns:
**  void**     The array of pointers where the data was stored.
**
*/
void** loadDataForTimeIntegratedSnowFallWaterEquivalent(void** gribBuffers, int n_lat, int n_lon)
	{

//	printf("\nEntered sub loadDataForTiSFWE\n");

	// Need to obtain 2 parameters from the A file --

	// generic grib Handle for looping through file
	grib_handle* h1 = NULL;
	grib_handle* h2 = NULL;

	int err;
	void** retBuffer = NULL;
	const void* buffer = NULL;
	long sizeLong = 0;
	long ncepDiscipline, ncepCategory, ncepVariable;
	int i;
	int found1, found2;
	
	double* ncepCategoricalSnowfallValues = NULL;
	double* ncepTotalRainfallValues = NULL;
    double* tiggeTimeIntegratedSnowFallWaterEquivalentValues = NULL;

    size_t size = 0;
    size_t ncepValuesSize = n_lat * n_lon;

		// Is the a file here?
    if(sFile == NULL)
        {
        fprintf(stderr, "Error, could not read the required file.  Exiting generateSnowFallWaterEquivalent()\n");
        printf("\nTerminated : loadDataForTimeIntegratedSnowFallWaterEquivalent\n\n");
		exit(1);
        }
	rewind(sFile);

	// test if this is the first one.
	if( !numberOfSavedRecords[TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT] )
		{
		numberOfSavedRecords[TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT] = 0;
		}

	// seek to the correct grib_handle we need (first one)

	//		reset seeker values
	found1 = found2 = 0;
	ncepDiscipline = ncepCategory = ncepVariable = -1;
	rewind(sFile);

	// optomized for 2 variables, 
	//    hopefully this will speed things up quite a bit.
	// -- Until both are found, continue looping

	int recordCounter = 1;
	int setDummy = 0;
    while( !found1 || !found2 )
		{

				// Clear the record scanning buffer
        if( (h1 != NULL) ) 
			{ grib_handle_delete(h1); }

				// Snag the next grib record from fileHandle (sFile) -> Check Handle
				// 		Bail if problem
		h1 = grib_handle_new_from_file(0, sFile, &err);
		if ( feof(sFile)  )
			{

	            //	sFile is at EOF --  We got a big problem...
				// Either sFile is corrupt, or expected records not found in file
				// or even possibly - but not likely - a GRIB API problem

				// This issue will always occur @ fct hr 000, so silencing the message...
//			printf("\n All Essential fields were not found in A file. (at EOF)\n\n");

				// send a zero'ed array as placeholder

			// Set these FAKE buffers to avoid seg-fault from free() later
			ncepTotalRainfallValues = calloc( 1, sizeof(int));
			ncepCategoricalSnowfallValues = calloc( 1, sizeof(int));

			// Rewind file and reset grib handle to first record 
			// 		(Actually gets a valid record to use as the dummy)
			rewind(sFile);
			grib_handle_delete(h1);
			h1 = grib_handle_new_from_file(0, sFile, &err);

            //  set the flag to return a zero-ed array later on
            setDummy++;

			// Bail from loop
			break;

			}
		if (h1 == NULL) 
			{
			// Grib handle creation failed -- Bail
			printf("Error (generateTimeIntegratedSnowFallWaterEquivalent) unable to create handle from file. error code: %d \n", err);
			grib_handle_delete(h1);
			return NULL;
            }

			// Obtain the parameter information for this record
		GRIB_CHECK(grib_get_long(h1, "discipline", &ncepDiscipline), 0);
		GRIB_CHECK(grib_get_long(h1, "parameterCategory", &ncepCategory), 0);
		GRIB_CHECK(grib_get_long(h1, "parameterNumber", &ncepVariable), 0);

//  Prints GRIB2 inventory
//		printf("Rec# %d : (%d,%d,%d)\n",(int) recordCounter,
//			(int) ncepDiscipline,(int) ncepCategory,(int) ncepVariable);
		
			// Locate APCP Record
		if( (ncepDiscipline == 0) && (ncepCategory == 1) && (ncepVariable == 8) )
			{
			found1++;
//            printf("Found APCP\n");
				// Get data section size, create & check buffer
			GRIB_CHECK(grib_get_size( h1, "values", &ncepValuesSize), 0);
			ncepTotalRainfallValues = calloc( ncepValuesSize, sizeof(double));
			if( ncepTotalRainfallValues == NULL)
				{
				fprintf(stderr, "Error, There was a problem Allocating the memory for storing the APCP data. Exiting generateSnowFallWaterEquivalent()\n");
				exit(1);
				}

			int rtn = grib_get_double_array(h1, "values", 
					ncepTotalRainfallValues, &ncepValuesSize);
            if( rtn != 0 )
                { printf("\tError Loading APCP values : rtn code %d\n",rtn); }
			}

			// Locate CAT.SNOW Record
		if( (ncepDiscipline == 0) && (ncepCategory == 1) &&
				 (ncepVariable == 195) )
			{
			found2++;
//			printf("Found C.SNOW\n");
				// Clear buffer, if its not already
			GRIB_CHECK(grib_get_size( h1, "values",
					&ncepValuesSize), 0);
			ncepCategoricalSnowfallValues = 
					calloc( ncepValuesSize, sizeof(double));
			if( ncepCategoricalSnowfallValues == NULL)
				{
				fprintf(stderr, "Error, There was a problem Allocating the memory for storing the CAT.SNOW data. Exiting generateSnowFallWaterEquivalent()\n");
				exit(1);
				}

			int rtn = grib_get_double_array(h1, "values",
					ncepCategoricalSnowfallValues, &ncepValuesSize );
			if( rtn != 0 ) 
				{ printf("\tError Loading C.SNOW values : rtn code %d\n",rtn); }
			}

        recordCounter++;
		}		// END Loop over records

	tiggeTimeIntegratedSnowFallWaterEquivalentValues =
			calloc(ncepValuesSize, sizeof(double));
	if( tiggeTimeIntegratedSnowFallWaterEquivalentValues == NULL)
		{
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generateTimeIntegratedSnowFallWaterEquivalent()\n");
		exit(1);
		}

		    // for each point in the grid.
	for( i=0; i<ncepValuesSize; i++ )
		{

		if( setDummy > 0 ) 
			{  }
		// Do nothing, its already zero due to calloc()
		//    should really be set to an grib UNDEF value instead

		else if(ncepCategoricalSnowfallValues[i] == 1)
			{
               // if it has snowed
			tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] =
   	          	    ncepTotalRainfallValues[i];
   	        }
           // if precip type is not snow
        else
   	        { tiggeTimeIntegratedSnowFallWaterEquivalentValues[i] = 0.0; }

       	}

	free( ncepCategoricalSnowfallValues );
	free( ncepTotalRainfallValues );

		// Close existing handle, and make a new one to export data section
		//	We don't care about the metadata in this handle
	grib_handle_delete(h1);
	h2 = newGribRecord( 1, 1, 1, 0, 0, 1, n_lat, n_lon );

	// Computations finished at this point
	//   now prep. to load the new data into appropriate buffers

    GRIB_CHECK( grib_set_double_array(h2, "values", 
			tiggeTimeIntegratedSnowFallWaterEquivalentValues, ncepValuesSize),  0);

    GRIB_CHECK(grib_get_long( h2, "totalLength", &sizeLong), 0);
    size = (size_t)sizeLong;

    // returns a pointer to the message (buffer) and its size.
    GRIB_CHECK(grib_get_message( h2, &buffer, &size), 0);

    // store the data into the buffer
    retBuffer = addGribRecordToArray(TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT, 
			gribBuffers, buffer, size);

	grib_handle_delete(h2);
	free( tiggeTimeIntegratedSnowFallWaterEquivalentValues );
	rewind(sFile);

	return retBuffer;
	}


/*
	while( !found1 || !found2 )
		{
		if( (h1 != NULL) && !found1 ) grib_handle_delete(h1);
		if( (h2 != NULL) && !found2 ) grib_handle_delete(h2);
		
		if( !found1 && !found2 )
			{
		// search for the first one.
			h1 = grib_handle_new_from_file(0, aFile, &err);
			if (h1 == NULL) 
				{
				printf("Error: unable to create handle from file. error code: %d \n", err);
				return NULL;
				}
			GRIB_CHECK(grib_get_long(h1, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(h1, "parameterCategory", &ncepCategory), 0);
			GRIB_CHECK(grib_get_long(h1, "parameterNumber", &ncepVariable), 0);
			GRIB_CHECK(grib_get_long(h1, "typeOfFirstFixedSurface", &ncepLevel), 0);
			
			// if it found total precip record (0,1,8)
			if(	( (ncepDiscipline == 0) && (ncepCategory == 1) && 
				  (ncepVariable == 8) && (ncepLevel == 1)) )
				{
				found1 = 1;
				ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
				}

            // if it found Cat.Snow record (0,1,195)
            if( ( (ncepDiscipline == 0) && (ncepCategory == 1) &&
                  (ncepVariable == 195) && (ncepLevel == 1)) )
                {
                found1 = 1;
                ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
                }

			}
		else if( found1 && !found2)
			{
		// search for the second one.
			h2 = grib_handle_new_from_file(0, aFile, &err);
			if (h2 == NULL) 
				{
				printf("Error: unable to create handle from file. error code: %d \n", err);
				return NULL;
				}
			GRIB_CHECK(grib_get_long(h2, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(h2, "parameterCategory", &ncepCategory), 0);
			GRIB_CHECK(grib_get_long(h2, "parameterNumber", &ncepVariable), 0);
			GRIB_CHECK(grib_get_long(h2, "typeOfFirstFixedSurface", &ncepLevel), 0);
			
			if(	( (ncepDiscipline == 0) && (ncepCategory == 1) && 
				  (ncepVariable == 8)   && (ncepLevel == 1)) 		)
				{
				found2 = 1;
				ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
				}

            if( ( (ncepDiscipline == 0) && (ncepCategory == 1) &&
                  (ncepVariable == 195) && (ncepLevel == 1)) )
                {
                found2 = 1;
                ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
                }



			}
		}

		// array for the new values to be stored.
	newValues = calloc(ncepValuesSize, sizeof(double));
	oldValues1 = calloc(ncepValuesSize, sizeof(double));
	oldValues2 = calloc(ncepValuesSize, sizeof(double));

		// Snag the data section from where the grib handle stopped seeking
	GRIB_CHECK(
		grib_get_double_array(h1, "values", oldValues1, &oldValues1Size), 0);
	GRIB_CHECK(
		grib_get_double_array(h2, "values", oldValues2, &oldValues2Size), 0);

	for(i = 0; i < ncepValuesSize; i++)
		{
		newValues[i] = oldValues1[i] + oldValues2[i];
		//printf("%f + %f = %f\n", oldValues1[i], oldValues2[i], newValues[i]);
		//printf(".");
		}

	// write the values back into the grib handle.
	GRIB_CHECK( grib_set_double_array(h1, "values", newValues, ncepValuesSize),  0);

	//printf("%ld, %ld, %ld, %ld\n", ncepDiscipline, ncepCategory, ncepVariable, ncepLevel);
	
	GRIB_CHECK(grib_get_long(h1, "totalLength", &sizeLong), 0);
	size = (size_t)sizeLong;

	// returns a pointer to the message (buffer) and its size.
	GRIB_CHECK(grib_get_message(h1, &buffer, &size), 0);
	
	// store the data into the buffer
	retBuffer = addGribRecordToArray(TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION, gribBuffers, buffer, size);
	
		
	// clean up
	grib_handle_delete(h1);
	grib_handle_delete(h2);	
	
	free(newValues);
	free(oldValues1);
	free(oldValues2);	
    free( ncepCategoricalSnowfallValues );
    free( ncepTotalRainfallValues );

	rewind(aFile);

//	printf("\nDone : loadDataForTiSFWE\n\n");

	// h should now be set,
	//    so we want to add it to the grib handle array, and return it.
	//return addGribRecordToArray(gribBuffers, buffer);
	return retBuffer;
	}
		// END load data routine
*/
