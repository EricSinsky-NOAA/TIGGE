/* generate Soil Moisture
** 
**  Generates the Soil Moisture variable and writes out the file.
**
**  Danny Brinegar & Steven Anthony, 2007
*/
#include "ncdcTigge.h"

void generateSoilMoisture(int yyyy, int mm, int dd, int hh, int fff, int em){
	grib_handle* gh = NULL;
	grib_handle* ncepUpperSoilMoisture = NULL;
	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,2);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,2);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,2);
	double* ncepUpperSoilMoistureValues = NULL;
	
	grib_handle* ncepLowerSoilMoisture = NULL;
	double* ncepLowerSoilMoistureValues = NULL;
	
	double* tiggeSoilMoistureValues = NULL;	
	
	char fileName[256];
	
	long ncepDiscipline, ncepParameterCategory, ncepParameterNumber, ncepLevel, ncepLevelValue;
	int i, found1, found2;
	int errorCode;
	
	/*
	** Section 1.  read the A file (from NCEP)
	*/
	
	// first, is the file here?
	if(sFile == NULL){
		fprintf(stderr, "Error, could not read the required file.  Exiting generateSoilMoisture()\n");
		return;
	}
	
	// first create the new skeleton file.
	gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);

	// name format from:   http://tigge.ecmwf.int/ldm_protocol.html
	generateFileName(SOIL_MOISTURE, yyyy, mm, dd, hh, fff, em, fileName);



	// start plugging in the metadata.
	// product definition template number, this means that it conforms to template 4.XX, where XX is this number */
	GRIB_CHECK(grib_set_long(gh, "discipline", 2), 0);
	GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 1), 0);
	
	GRIB_CHECK(grib_set_long(gh, "parameterCategory", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterNumber", 22), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 106), 0);
	GRIB_CHECK(grib_set_long(gh, "scaleFactorOfFirstFixedSurface", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "scaledValueOfFirstFixedSurface", 0), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 106), 0);
	GRIB_CHECK(grib_set_long(gh, "scaleFactorOfSecondFixedSurface", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "scaledValueOfSecondFixedSurface", 2), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", yyyy), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "yearOfEndOfOverallTimeInterval", yyyy), 0);
	//GRIB_CHECK(grib_set_long(gh, "monthOfEndOfOverallTimeInterval", mm), 0);
	//GRIB_CHECK(grib_set_long(gh, "dayOfEndOfOverallTimeInterval", dd), 0);
	//GRIB_CHECK(grib_set_long(gh, "hourOfEndOfOverallTimeInterval", hh), 0);
	//GRIB_CHECK(grib_set_long(gh, "minuteOfEndOfOverallTimeInterval", 0), 0);
	//GRIB_CHECK(grib_set_long(gh, "secondOfEndOfOverallTimeInterval", 0), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "numberOfMissingInStatisticalProcess", 0), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "typeOfStatisticalProcessing", 1), 0);
	//GRIB_CHECK(grib_set_long(gh, "typeOfTimeIncrementBetweenSuccessiveFieldsUsedInTheStatisticalProcessing", 2), 0);
	
	//GRIB_CHECK(grib_set_long(gh, "lengthOfTimeRange", 0), 0);
	//GRIB_CHECK(grib_set_long(gh, "indicatorOfUnitForTimeIncrement", 255), 0);
	//GRIB_CHECK(grib_set_long(gh, "timeIncrement", 0), 0);
	
	// set marsStartStep to make it stop yelling.
	GRIB_CHECK(grib_set_long(gh, "marsStartStep", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "forecastTime",(long)fff ), 0);

	
	found1 = found2 = 0;
	ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepLevel = ncepLevelValue = -1;

	// optomized for 2 variables, hopefully this will speed things up quite a bit.
	// while either is not found, continue looking.
	int counter = 0;
	while(!found1 || !found2){
		counter++;
		if( (ncepUpperSoilMoisture != NULL) && !found1 ) grib_handle_delete(ncepUpperSoilMoisture);
		if( (ncepLowerSoilMoisture != NULL) && !found2 ) grib_handle_delete(ncepLowerSoilMoisture);
		
		if( !found1 && !found2 ){
		// search for the first one.
			ncepUpperSoilMoisture = grib_handle_new_from_file(0, sFile, &errorCode);
			if (ncepUpperSoilMoisture == NULL) {
				printf("Error (ncepUpperSoilMoisture) unable to create handle from file. error code: %d \n", errorCode);
				return;
			}
			GRIB_CHECK(grib_get_long(ncepUpperSoilMoisture, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepUpperSoilMoisture, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepUpperSoilMoisture, "parameterNumber", &ncepParameterNumber), 0);
			GRIB_CHECK(grib_get_long(ncepUpperSoilMoisture, "typeOfFirstFixedSurface", &ncepLevel), 0);
			GRIB_CHECK(grib_get_long(ncepUpperSoilMoisture, "scaledValueOfFirstFixedSurface", &ncepLevelValue), 0);


//	printf(" >> %d %d:%d dcp (%d %d %d) (%d - %d) \n",counter, found1, found2,
//		(int)ncepDiscipline,(int)ncepParameterCategory,(int)ncepParameterNumber,
//		(int)ncepLevel,(int)ncepLevelValue );

			// if it is either one of the variables.
			if( ((ncepDiscipline == 2) && (ncepParameterCategory == 0) && (ncepParameterNumber == 192) && (ncepLevel == 106) && (ncepLevelValue == 0) ) ||
				((ncepDiscipline == 2) && (ncepParameterCategory == 0) && (ncepParameterNumber == 192) && (ncepLevel == 106) && (ncepLevelValue == 10) ) ){
				found1 = 1;
				ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepLevel = ncepLevelValue = -1;
			}
		
		}else if( found1 && !found2){
		// search for the second one.
			ncepLowerSoilMoisture = grib_handle_new_from_file(0, sFile, &errorCode);
			if (ncepLowerSoilMoisture == NULL) {
				printf("Error (ncepLowerSoilMoisture) unable to create handle from file. error code: %d \n", errorCode);
				return;
			}
			GRIB_CHECK(grib_get_long(ncepLowerSoilMoisture, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(ncepLowerSoilMoisture, "parameterCategory", &ncepParameterCategory), 0);
			GRIB_CHECK(grib_get_long(ncepLowerSoilMoisture, "parameterNumber", &ncepParameterNumber), 0);
			GRIB_CHECK(grib_get_long(ncepLowerSoilMoisture, "typeOfFirstFixedSurface", &ncepLevel), 0);
			GRIB_CHECK(grib_get_long(ncepLowerSoilMoisture, "scaledValueOfFirstFixedSurface", &ncepLevelValue), 0);
			
			// if it is either one of the variables
			if( ((ncepDiscipline == 2) && (ncepParameterCategory == 0) && (ncepParameterNumber == 192) && (ncepLevel == 106) && (ncepLevelValue == 0) ) ||
				((ncepDiscipline == 2) && (ncepParameterCategory == 0) && (ncepParameterNumber == 192) && (ncepLevel == 106) && (ncepLevelValue == 10) ) ){
				found2 = 1;
				ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepLevel = ncepLevelValue = -1;
			}
		}
	}
	
	ncepUpperSoilMoistureValues = calloc(ncepValuesSize, sizeof(double));
	ncepLowerSoilMoistureValues = calloc(ncepValuesSize, sizeof(double));
	GRIB_CHECK(grib_get_double_array(ncepUpperSoilMoisture, "values", ncepUpperSoilMoistureValues, &ncepValuesSize), 0);
	GRIB_CHECK(grib_get_double_array(ncepLowerSoilMoisture, "values", ncepLowerSoilMoistureValues, &ncepValuesSize), 0);


	// both are now stored off.
	tiggeSoilMoistureValues = calloc(ncepValuesSize, sizeof(double));


	if(tiggeSoilMoistureValues == NULL){
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generateSoilMoisture()\n");
		exit(1);
	}
	// Turn on the bitmap (this is a land mask)
	GRIB_CHECK( grib_set_long(gh, "bitMapIndicator", 0), 0);

	// for each point in the grid.
	for(i=0; i<ncepValuesSize; i++){
		if( (ncepUpperSoilMoistureValues[i] == 9999.0) || (ncepLowerSoilMoistureValues[i] == 9999.0) ) {
			tiggeSoilMoistureValues[i] = 9999.0;
		}else{
			tiggeSoilMoistureValues[i] = ((ncepUpperSoilMoistureValues[i] + ncepLowerSoilMoistureValues[i]) / 2.0) * 1000.0;
		}
	}

	// store the data into the grib file
	GRIB_CHECK( grib_set_double_array(gh, "values", tiggeSoilMoistureValues, ncepValuesSize),  0);
	

	if(writeGribToFile(gh, fileName) != 0){
		printf("ERROR, did not write the file \"%s\" out successfully.\n", fileName);
	}

	// free allocated memory
	free(ncepUpperSoilMoistureValues);
	free(ncepLowerSoilMoistureValues);
	free(tiggeSoilMoistureValues);
		
	
	// delete grib_handles
	grib_handle_delete(gh);
	grib_handle_delete(ncepUpperSoilMoisture);
	grib_handle_delete(ncepLowerSoilMoisture);
	// rewind the file
	rewind(sFile);

}
