/* generate Time-Integrated Outgoing Long Wave Radiation
**
**  Generates a valid tigge file for Time-Integrated Outgoing Long Wave Radiation
**
** Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generateTimeIntegratedOutgoingLongWaveRadiation(int yyyy, int mm, int dd, int hh, int em, void** resource){
	int fff = 0;
	int i = 0;
	
	long temp = 0;
	double* ncepValues = NULL;
	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,2);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,2);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,2);
	grib_handle* gh;

	double* runningSum = NULL;
	runningSum = calloc(ncepValuesSize, sizeof(double));

	double* tiggeTimeIntegratedOutgoingLongWaveRadiationValues = NULL;	
	

	char fileName[256];
	// check if the space was allocated correctly.
	if(runningSum == NULL){
		fprintf(stderr, "Failed to allocate memory for Time Integrated Outgoing Long Wave Radiation.  Exiting...\n");
		exit(1);
	}
	
	// create a date/time structure with the initial time
	struct dt tmInit,tmEnd;
	tmInit.yr = yyyy;
	tmInit.mo = mm; 
	tmInit.dy = dd;  
	tmInit.hr = hh;  
	tmInit.mn = 0;  

	grib_handle* ncepGribRecord = NULL;
	// for each forecast  hour.
	for(fff = 0; fff<=FORECAST_HOURS; fff+=6){

// printf("\nDEBUG : fct = %i\n",fff);
		
	        // populate tmEnd with forecast time offset, add to initial time 
	        tmEnd.yr = 0;
		tmEnd.mo = 0; 
		tmEnd.dy = 0;  
		tmEnd.hr = fff;  
		tmEnd.mn = 0;  
		timeAdd (&tmInit, &tmEnd);

if( ! resource[fff/6] )
	{ fprintf(stderr, "ERROR: resource (%i) is NULL\n", fff/6 );  continue;  }
if( ! bytesPerRecord[TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION][fff/6] )
	{ fprintf(stderr, "ERROR: bytesPerRecord[ %i ] is null!\n", fff/6 );  continue;  }

		ncepGribRecord = grib_handle_new_from_message(NULL, resource[fff/6], bytesPerRecord[TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION][fff/6]);
		GRIB_CHECK(grib_get_long(ncepGribRecord, "parameterNumber", &temp), 0);
		
		// read the values.
		ncepValues = calloc(ncepValuesSize, sizeof(double));
		tiggeTimeIntegratedOutgoingLongWaveRadiationValues = calloc(ncepValuesSize, sizeof(double));

		GRIB_CHECK(grib_get_double_array(ncepGribRecord, "values", ncepValues, &ncepValuesSize), 0);
				
		// set the fileName
		memset(fileName, 0, 128); //reset the string.
		generateFileName(TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION, yyyy, mm, dd, hh, fff, em, fileName);

		//printf("%s\n", fileName);
		

		if(fff == 0){
			//printf("fff == 0\n");
			for(i=0; i<ncepValuesSize; i++){
				// runningSum[i] = ncepValues[i];
				//  2014-03-10 must NOT count initial time step
				runningSum[i] = 0.0;
				tiggeTimeIntegratedOutgoingLongWaveRadiationValues[i] = 0.0;
			}
		}else{
			for(i=0; i<ncepValuesSize; i++){
				runningSum[i] += ncepValues[i];

				// Mar.2014 unit conversion fix.
				// Apr.2014 correction, the sign is reversed due to 
				//          "positive downward radiation" TIGGE convention
                                tiggeTimeIntegratedOutgoingLongWaveRadiationValues[i] = runningSum[i] * -21600.0;
//				tiggeTimeIntegratedOutgoingLongWaveRadiationValues[i] = (runningSum[i] / (fff*3600.0));
			}
		}
		
// printf("DEBUG : -01- %i \n", fff );

		
		// now create a new grib_handle to store the data.
		gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);
		
		GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 11), 0);
		
		GRIB_CHECK(grib_set_long(gh, "parameterCategory", 5), 0);
		GRIB_CHECK(grib_set_long(gh, "parameterNumber", 5), 0);
		
		GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 8), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfFirstFixedSurface"), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaledValueOfFirstFixedSurface"), 0);
	
		GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 255), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfSecondFixedSurface"), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaledValueOfSecondFixedSurface"), 0);
	
		
		GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", (long)tmEnd.yr), 0);
		GRIB_CHECK(grib_set_long(gh, "monthOfEndOfOverallTimeInterval", (long)tmEnd.mo), 0);
		GRIB_CHECK(grib_set_long(gh, "dayOfEndOfOverallTimeInterval", (long)tmEnd.dy), 0);
		GRIB_CHECK(grib_set_long(gh, "hourOfEndOfOverallTimeInterval", (long)tmEnd.hr), 0);
		GRIB_CHECK(grib_set_long(gh, "minuteOfEndOfOverallTimeInterval", 0), 0);
		GRIB_CHECK(grib_set_long(gh, "secondOfEndOfOverallTimeInterval", 0), 0);
		

		GRIB_CHECK(grib_set_long(gh, "typeOfStatisticalProcessing", 1), 0);
		GRIB_CHECK(grib_set_long(gh, "typeOfTimeIncrementBetweenSuccessiveFieldsUsedInTheStatisticalProcessing", 2), 0);
		GRIB_CHECK(grib_set_long(gh, "numberOfMissingInStatisticalProcess", 0), 0);  // can set if not 0
		
		GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitOfTimeRange", 1), 0);
		GRIB_CHECK(grib_set_long(gh, "lengthOfTimeRange", fff), 0);
		GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitForTimeIncrement", 255), 0);
		GRIB_CHECK(grib_set_long(gh, "timeIncrement", 0), 0);
		
		GRIB_CHECK(grib_set_long(gh, "startStep", 0), 0);
		GRIB_CHECK(grib_set_long(gh, "endStep", (long)fff ), 0);

// printf("DEBUG : -02- %i \n", fff );

		// store the data into the grib file
		GRIB_CHECK( grib_set_double_array(gh, "values", tiggeTimeIntegratedOutgoingLongWaveRadiationValues, ncepValuesSize),  0);
	

// printf("DEBUG: wrote %s", fileName  );

		writeGribToFile(gh, fileName);

		// clean up after ourselves.
		grib_handle_delete(gh);
		free(ncepValues);
		free(tiggeTimeIntegratedOutgoingLongWaveRadiationValues);
		grib_handle_delete(ncepGribRecord);
	} //end forecast hours
	
	free(runningSum);
}






/*
** loadDataForTimeIntegratedOutgoingLongWaveRadiation()
**  Loads the required data needed to calculate the ttr variable.
**
** Parameters:
**  void** gribBuffers    The array of pointers where the data is to be stored
**
** Returns:
**  void**     The array of pointers where the data was stored.
**
**
*/
void** loadDataForTimeIntegratedOutgoingLongWaveRadiation(void** gribBuffers, int n_lat, int n_lon){

	// seek to a variable.
	grib_handle* h = NULL;
	int err;
	void** retBuffer = NULL;
	const void* buffer = NULL;
	size_t size = 0;
	long sizeLong = 0;
	long ncepDiscipline, ncepCategory, ncepVariable, ncepLevel;

	// test if this is the first one.
	if(!numberOfSavedRecords[TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION]){
		numberOfSavedRecords[TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION] = 0;
	}
	
	// seek to the correct grib_handle we need
	
	ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1; 
	while( (ncepDiscipline != 0) || (ncepCategory != 5) || (ncepVariable != 193) || (ncepLevel != 8) ){
		if(h != NULL){
			grib_handle_delete(h);
		}
		h = grib_handle_new_from_file(0, sFile, &err);
		if (h == NULL && ignoreMissingInput != 0 ) 
			{
			h = newBlankGribRecord( 0, 5, 193, 8, n_lat, n_lon );
			}
		if (h == NULL && ignoreMissingInput == 0 )
			{
			printf("Error (generateTimeIntegratedOutgoingLongWaveRadiation) unable to create handle from file. error code: %d :: implanting blank grib record. \n", err);
			return(NULL);
			}

		GRIB_CHECK(grib_get_long(h, "discipline", &ncepDiscipline), 0);
		GRIB_CHECK(grib_get_long(h, "parameterCategory", &ncepCategory), 0);
		GRIB_CHECK(grib_get_long(h, "parameterNumber", &ncepVariable), 0);
		GRIB_CHECK(grib_get_long(h, "typeOfFirstFixedSurface", &ncepLevel), 0);
		}

	//printf("%ld, %ld, %ld, %ld\n", ncepDiscipline, ncepCategory, ncepVariable, ncepLevel);
	
	GRIB_CHECK(grib_get_long(h, "totalLength", &sizeLong), 0);
	size = (size_t)sizeLong;
	//printf("Size: %d\n", size);
	// returns a pointer to the message (buffer) and its size.
	GRIB_CHECK(grib_get_message(h, &buffer, &size), 0);
	
	// store the data into the buffer
	retBuffer = addGribRecordToArray(TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION, gribBuffers, buffer, size);
	
	grib_handle_delete(h);

        rewind(sFile);
	// h should now be set, so we want to add it to the grib handle array, and return it.
	//return addGribRecordToArray(gribBuffers, buffer);
	return retBuffer;
}
