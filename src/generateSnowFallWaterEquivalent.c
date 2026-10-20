/* generate Snow Fall Water Equivalent
**
**  Generates a valid tigge file for snow-fall water equivalent.
**
** Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generateSnowFallWaterEquivalent(int yyyy, int mm, int dd, int hh, int fff, int em){
	grib_handle* gh = NULL;
	grib_handle* ncepCategoricalSnowfall = NULL;
	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,2);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,2);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,2);
	double* ncepCategoricalSnowfallValues = NULL;
	
	grib_handle* ncepTotalRainfall = NULL;
	double* ncepTotalRainfallValues = NULL;
	
	double* tiggeSnowfallWaterEquivalentValues = NULL;	

	char fileName[256];
	
	long ncepDiscipline, ncepParameterCategory, ncepParameterNumber;
	int i;
	int errorCode;
	
	time_t rawTime;
	struct tm* timeStructure;

	time(&rawTime); // store the real time into rawTime
	timeStructure = gmtime(&rawTime); // take the raw time and convert it to a time structure. (struct tm from time.h)
	
	timeStructure->tm_year = yyyy - 1900; // store the year
	timeStructure->tm_mon = mm - 1;  //month
	timeStructure->tm_mday = dd;  //day
	timeStructure->tm_hour = hh;  //hour
	/*printf("Testing:\n");
	printf("year (untouched):  %d \n", timeStructure->tm_year+1900);
	timeStructure->tm_mon += 12;
	mktime(timeStructure);
	printf("year (12 mon later):  %d \n", timeStructure->tm_year+1900);
	*/

	/* for scaling info */
	//long ncepRainfallDecimalScaleFactor;
	//long ncepRainfallBinaryScaleFactor;
	//long ncepRainfallReferenceValue;
	
	/*
	** Section 1.  read the A file (from NCEP)
	*/
	
	// first, is the file here?
	if(sFile == NULL){
		fprintf(stderr, "Error, could not read the required file.  Exiting generateSnowFallWaterEquivalent()\n");
		return;
	}
	
	
	// first create the new skeleton file.
	gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);

	// name format from:   http://tigge.ecmwf.int/ldm_protocol.html
	generateFileName(SNOW_FALL_WATER_EQUIVALENT, yyyy, mm, dd, hh, fff, em, fileName);



	// start plugging in the metadata.
	/* Set all the stuff up that defines the "snow fall water equivalent" variable */
	// product definition template number, this means that it conforms to template 4.XX, where XX is this number */
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
	GRIB_CHECK(grib_set_long(gh, "startStep", (long)fff ), 0);
	GRIB_CHECK(grib_set_long(gh, "endStep", (long)fff ), 0);

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
	
	// only do the search if it is not the 000 forecast hour.
	if(fff != 0){
		// while there is no match on the ncep variable.
		while( (ncepDiscipline != 0) || (ncepParameterCategory != 1) || (ncepParameterNumber != 195) ){
			if(ncepCategoricalSnowfall != NULL){
				grib_handle_delete(ncepCategoricalSnowfall);
				ncepCategoricalSnowfall = NULL;
			}
			ncepCategoricalSnowfall = grib_handle_new_from_file(0, sFile, &errorCode);
			if(ncepCategoricalSnowfall == NULL){
				fprintf(stderr, "Error (ncepCategoricalSnowfall) There was a problem reading the grib_handle.  Exiting generateSnowFallWaterEquivalent()\n");
				return;	
			}
			// set the 3 numbers up for determining the product.
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepCategoricalSnowfall, "parameterNumber", &ncepParameterNumber), 0);
			
		}
		
		// test the data.
		GRIB_CHECK(grib_get_size(ncepCategoricalSnowfall, "values", &ncepValuesSize), 0);
		//printf("ncepCategoricalSnowfallSize = %d\n", ncepCategoricalSnowfallSize);
		
		//printf("%ld - %ld - %ld\n", ncepDiscipline, ncepParameterCategory, ncepParameterNumber);

		ncepCategoricalSnowfallValues = calloc(ncepValuesSize, sizeof(double));
		if(ncepCategoricalSnowfallValues == NULL){
			fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Categorical Rainfall.   Exiting...\n");
			exit(1);
		}
		GRIB_CHECK(grib_get_double_array(ncepCategoricalSnowfall, "values", ncepCategoricalSnowfallValues, &ncepValuesSize), 0);

		
		//
		// now that the ncepCategoricalSnowfall is filled. we must fill the total rainfall array.
		//		
		rewind(sFile); /* reset it to the beginning of the file */
		ncepDiscipline = ncepParameterCategory = ncepParameterNumber = -1;
		while( (ncepDiscipline != 0) || (ncepParameterCategory != 1) || (ncepParameterNumber != 8) ){
			if(ncepTotalRainfall != NULL){
				grib_handle_delete(ncepTotalRainfall);
				ncepTotalRainfall = NULL;
			}
			
			ncepTotalRainfall = grib_handle_new_from_file(0, sFile, &errorCode);
			if(ncepTotalRainfall == NULL){
				fprintf(stderr, "Error (ncepTotalRainfall) There was a problem reading the grib_handle.  Exiting generateSnowFallWaterEquivalent()\n");
				return;
			}
			// set the 3 numbers up for determining the product.
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepTotalRainfall, "parameterNumber", &ncepParameterNumber), 0);
			//printf("Discipline = %ld   Category = %ld   Number = %ld\n", ncepDiscipline, ncepParameterCategory, ncepParameterNumber);  
		}
		// we have the grib_handle now.
		GRIB_CHECK(grib_get_size(ncepTotalRainfall, "values", &ncepValuesSize), 0);
		ncepTotalRainfallValues = calloc(ncepValuesSize, sizeof(double));
		if(ncepTotalRainfallValues == NULL){
			fprintf(stderr, "Error (ncepTotalRainfall) There was a problem Allocating the memory for storing the data. Exiting generateSnowFallWaterEquivalent()\n");
			exit(1);
		}	
	
		GRIB_CHECK(grib_get_double_array(ncepTotalRainfall, "values", ncepTotalRainfallValues, &ncepValuesSize), 0);
		

		
		// save off some scaling information.
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "decimalScaleFactor", &ncepRainfallDecimalScaleFactor), 0);
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "binaryScaleFactor", &ncepRainfallBinaryScaleFactor), 0);;
		//GRIB_CHECK(grib_get_long(ncepTotalRainfall, "referenceValue", &ncepRainfallReferenceValue), 0);
		
	}
	
	tiggeSnowfallWaterEquivalentValues = calloc(ncepValuesSize, sizeof(double));
	if(tiggeSnowfallWaterEquivalentValues == NULL){
		fprintf(stderr, "Error (tiggeSnowfallWaterEquivalentValues) There was a problem Allocating the memory for storing the data. Exiting generateSnowFallWaterEquivalent()\n");
		exit(1);
	}
	
	
	// for each point in the grid.
	for(i=0; i<ncepValuesSize; i++){
		if(fff == 0){
			tiggeSnowfallWaterEquivalentValues[i] = 0.0;
		}
		else{
			if(ncepCategoricalSnowfallValues[i] == 1){
			// if it's snowing
				tiggeSnowfallWaterEquivalentValues[i] = ncepTotalRainfallValues[i];
				//value = ( ncepRainfallReferenceValue + (0.0 + ncepTotalRainfallValues[i]) * pow(2.0, ncepRainfallBinaryScaleFactor) ) / pow(10.0, ncepRainfallDecimalScaleFactor);
				//printf("Unscaled Value: %f    Scaled Value: %f\n", ncepTotalRainfallValues[i], value);
			}else{
			// if it's not snowing
				tiggeSnowfallWaterEquivalentValues[i] = 0.0;
			}
		}
	}
	



	// store the data into the grib file
	GRIB_CHECK( grib_set_double_array(gh, "values", tiggeSnowfallWaterEquivalentValues, ncepValuesSize),  0);
	
	
	if(writeGribToFile(gh, fileName) != 0){
		printf("ERROR, did not write the file \"%s\" out successfully.\n", fileName);
	}

	// free allocated memory
	free(ncepCategoricalSnowfallValues);
	free(ncepTotalRainfallValues);
	free(tiggeSnowfallWaterEquivalentValues);
		
	
	// delete grib_handles
	grib_handle_delete(gh);
	grib_handle_delete(ncepCategoricalSnowfall);
	grib_handle_delete(ncepTotalRainfall);
	// rewind the file
	rewind(sFile);

}
