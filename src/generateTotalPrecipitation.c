/* generate Total Precipitation
**
**  Generates a valid tigge file for Total Precipitation
**
** Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generateTotalPrecipitation(int yyyy, int mm, int dd, int hh, int em, void** resource){
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

	double* tiggeTotalPrecipitationValues = NULL;	
	

	char fileName[256];
	// check if the space was allocated correctly.
	if(runningSum == NULL){
		fprintf(stderr, "Failed to allocate memory for Total Precipitation.  Exiting...\n");
		exit(1);
	}	

	grib_handle* ncepGribRecord = NULL;
	// for each forecast  hour.
	for(fff = 0; fff<=FORECAST_HOURS; fff+=6){
		
		if(fff != 0){
			ncepGribRecord = grib_handle_new_from_message(NULL, resource[fff/6], bytesPerRecord[TOTAL_PRECIPITATION][fff/6]);
			GRIB_CHECK(grib_get_long(ncepGribRecord, "parameterNumber", &temp), 0);
		}

		// read the values.
		ncepValues = calloc(ncepValuesSize, sizeof(double));
		tiggeTotalPrecipitationValues = calloc(ncepValuesSize, sizeof(double));
		
		if(fff != 0){
			GRIB_CHECK(grib_get_double_array(ncepGribRecord, "values", ncepValues, &ncepValuesSize), 0);
		}
		
		// set the fileName
		memset(fileName, 0, 128); //reset the string.
		generateFileName(TOTAL_PRECIPITATION, yyyy, mm, dd, hh, fff, em, fileName);

		//printf("%s\n", fileName);
		

		if(fff == 0){
			//printf("fff == 0\n");
			for(i=0; i<ncepValuesSize; i++){
				runningSum[i] = ncepValues[i];
				tiggeTotalPrecipitationValues[i] = 0.0;
			}
		}else{
			for(i=0; i<ncepValuesSize; i++){
				runningSum[i] += ncepValues[i];
				tiggeTotalPrecipitationValues[i] = runningSum[i];
			}
		}
	

		// now create a new grib_handle to store the data.
		gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);
		
		GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 11), 0);

		GRIB_CHECK(grib_set_long(gh, "tablesVersion", 5), 0);
		GRIB_CHECK(grib_set_long(gh, "grib2LocalSectionPresent", 0), 0);
		
		GRIB_CHECK(grib_set_long(gh, "parameterCategory", 1), 0);
		GRIB_CHECK(grib_set_long(gh, "parameterNumber", 52), 0);
		
		GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 1), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfFirstFixedSurface"), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaledValueOfFirstFixedSurface"), 0);
	
		GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 255), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfSecondFixedSurface"), 0);
		GRIB_CHECK(grib_set_missing(gh, "scaledValueOfSecondFixedSurface"), 0);
	
		
		GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", yyyy), 0);
		GRIB_CHECK(grib_set_long(gh, "monthOfEndOfOverallTimeInterval", mm), 0);
		GRIB_CHECK(grib_set_long(gh, "dayOfEndOfOverallTimeInterval", dd), 0);
		GRIB_CHECK(grib_set_long(gh, "hourOfEndOfOverallTimeInterval", hh), 0);
		GRIB_CHECK(grib_set_long(gh, "minuteOfEndOfOverallTimeInterval", 0), 0);
		GRIB_CHECK(grib_set_long(gh, "secondOfEndOfOverallTimeInterval", 0), 0);
		

		GRIB_CHECK(grib_set_long(gh, "typeOfStatisticalProcessing", 1), 0);
		GRIB_CHECK(grib_set_long(gh, "typeOfTimeIncrementBetweenSuccessiveFieldsUsedInTheStatisticalProcessing", 2), 0);
		GRIB_CHECK(grib_set_long(gh, "numberOfMissingInStatisticalProcess", 0), 0);  // can set if not 0
		
		GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitOfTimeRange", 1), 0);
		GRIB_CHECK(grib_set_long(gh, "lengthOfTimeRange", fff), 0);
		GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitForTimeIncrement", 255), 0);
		GRIB_CHECK(grib_set_long(gh, "timeIncrement", 0), 0);
		
		// set marsStartStep to make it stop yelling.
		GRIB_CHECK(grib_set_long(gh, "startStep", 0), 0);
		GRIB_CHECK(grib_set_long(gh, "endStep", (long) fff), 0);
		
		// store the data into the grib file
		GRIB_CHECK( grib_set_double_array(gh, "values", tiggeTotalPrecipitationValues, ncepValuesSize),  0);
		
		writeGribToFile(gh, fileName);

		// clean up after ourselves.
		grib_handle_delete(gh);
		free(ncepValues);
		free(tiggeTotalPrecipitationValues);
		grib_handle_delete(ncepGribRecord);
	} //end forecast hours
	
	free(runningSum);
}


/*
** loadDataForTotalPrecipitation()
**  Loads the required data needed to calculate the pt variable.
**
** Parameters:
**  void** gribBuffers    The array of pointers where the data is to be stored
**
** Returns:
**  void**     The array of pointers where the data was stored.
**
**
*/
void** loadDataForTotalPrecipitation(void** gribBuffers, int n_lat, int n_lon)
	{
	// seek to a variable.
	grib_handle* h = NULL;
	int err;
	void** retBuffer = NULL;
	const void* buffer = NULL;
	size_t size = 0;
	size_t ncepValuesSize = n_lat * n_lon;
	long sizeLong = 0;
	long ncepDiscipline, ncepCategory, ncepVariable, ncepLevel;
	int first = 0;

	// test if this is the first one.
	if(!numberOfSavedRecords[TOTAL_PRECIPITATION]){
		numberOfSavedRecords[TOTAL_PRECIPITATION] = 0;
		first = 1;
	}	
	
	if(!first){
		ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1; 
		while( (ncepDiscipline != 0) || (ncepCategory != 1) || (ncepVariable != 8) ){
			if(h != NULL){
				grib_handle_delete(h);
			}
			h = grib_handle_new_from_file(0, sFile, &err);
			if (h == NULL) {
				printf("Error (generateTotalPrecipitation) unable to create handle from file. error code: %d \n", err);
				return NULL;
			}
			GRIB_CHECK(grib_get_long(h, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(h, "parameterCategory", &ncepCategory), 0);
			GRIB_CHECK(grib_get_long(h, "parameterNumber", &ncepVariable), 0);
			GRIB_CHECK(grib_get_long(h, "typeOfFirstFixedSurface", &ncepLevel), 0);
		}
	
		GRIB_CHECK(grib_get_long(h, "totalLength", &sizeLong), 0);
		size = (size_t)sizeLong;

		// returns a pointer to the message (buffer) and its size.
		GRIB_CHECK(grib_get_message(h, &buffer, &size), 0);
	}else{
		// if we are on the 0h forecast
		buffer = calloc(ncepValuesSize, sizeof(double));
		size = ncepValuesSize;
	}	

	// store the data into the buffer
	retBuffer = addGribRecordToArray(TOTAL_PRECIPITATION, gribBuffers, buffer, size);
	
	if( h != NULL )
		{
		grib_handle_delete(h);
		}

//	if(first && (buffer != NULL))
//		{
//		free(buffer);
//		}

	rewind(sFile);

	return retBuffer;
}
