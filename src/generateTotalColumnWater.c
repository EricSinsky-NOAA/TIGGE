/* generate Total Column Water
**
**  Generates a valid tigge file for Total Column Water
**
** Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generateTotalColumnWater(int yyyy, int mm, int dd, int hh, int fff, int em){
	grib_handle* gh = NULL;
	grib_handle* ncepTotalPrecipWater = NULL;
	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,2);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,2);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,2);
	double* ncepTotalPrecipWaterValues = NULL;
	
	grib_handle* ncepTotalCloudWater = NULL;
	double* ncepTotalCloudWaterValues = NULL;
	
	double* tiggeTotalColumnWaterValues = NULL;	

	char fileName[256];
	
	long ncepDiscipline, ncepParameterCategory, ncepParameterNumber, ncepLevel;
	int i;
	int errorCode;
	
	/*
	** Section 1.  read the A file (from NCEP)
	*/
	
	// first, is the file here?
        if(sFile == NULL){
                fprintf(stderr, "Error, could not read the required file.  Exiting generateTotalColumnWater()\n");
                return;
        }
	
	// first create the new skeleton file.
	gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);

	// name format from:   http://tigge.ecmwf.int/ldm_protocol.html
	generateFileName(TOTAL_COLUMN_WATER, yyyy, mm, dd, hh, fff, em, fileName);



	// start plugging in the metadata.
	// product definition template number, this means that it conforms to template 4.XX, where XX is this number */
	GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 1), 0);
	
	GRIB_CHECK(grib_set_long(gh, "parameterCategory", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterNumber", 51), 0);

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

		// Vertical level info
	GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 1), 0);
	GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 8), 0);


	GRIB_CHECK(grib_set_long(gh,"scaledValueOfFirstFixedSurface",4294967295),0);
	GRIB_CHECK(grib_set_long(gh,"scaleFactorOfFirstFixedSurface",255), 0);
	GRIB_CHECK(grib_set_long(gh,"scaledValueOfSecondFixedSurface",4294967295),0);
	GRIB_CHECK(grib_set_long(gh,"scaleFactorOfSecondFixedSurface",255), 0);


	GRIB_CHECK(grib_set_long(gh, "marsStartStep", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "forecastTime",(long)fff ), 0);

	// open up the data files from NCEP
	ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepLevel = -1;
	
	// TotalPrecipWater from the pgrb-a file.
	// while there is no match on the ncep variable.
	while( (ncepDiscipline != 0) || (ncepParameterCategory != 1) || (ncepParameterNumber != 3) || (ncepLevel != 200) ){
		if(ncepTotalPrecipWater != NULL){
			grib_handle_delete(ncepTotalPrecipWater);
			ncepTotalPrecipWater = NULL;
		}
		ncepTotalPrecipWater = grib_handle_new_from_file(0, sFile, &errorCode);
		if(ncepTotalPrecipWater == NULL){
			fprintf(stderr, "Error (ncepTotalPrecipWater) There was a problem reading the grib_handle.  Exiting generateSnowFallWaterEquivalent(%d)\n", errorCode);
			return;
		}
		// set the 3 numbers up for determining the product.
		GRIB_CHECK(grib_get_long(ncepTotalPrecipWater, "discipline", &ncepDiscipline), 0);
		GRIB_CHECK(grib_get_long(ncepTotalPrecipWater, "parameterCategory", &ncepParameterCategory), 0);
		GRIB_CHECK(grib_get_long(ncepTotalPrecipWater, "parameterNumber", &ncepParameterNumber), 0);
		GRIB_CHECK(grib_get_long(ncepTotalPrecipWater, "typeOfFirstFixedSurface", &ncepLevel), 0);
	}	

	GRIB_CHECK(grib_get_size(ncepTotalPrecipWater, "values", &ncepValuesSize), 0);
	
	ncepTotalPrecipWaterValues = calloc(ncepValuesSize, sizeof(double));
	if(ncepTotalPrecipWaterValues == NULL){
		fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Precipital Water.  Exiting...\n");
		exit(1);
	}
	GRIB_CHECK(grib_get_double_array(ncepTotalPrecipWater, "values", ncepTotalPrecipWaterValues, &ncepValuesSize), 0);

	
	//
	// now that the ncepTotalPrecipWater is filled. we must fill the cloud Water array.
	//
	
	ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepLevel =  -1;
	while( (ncepDiscipline != 0) || (ncepParameterCategory != 6) || (ncepParameterNumber != 6) || (ncepLevel != 200) ){
		if(ncepTotalCloudWater != NULL){
			grib_handle_delete(ncepTotalCloudWater);
			ncepTotalCloudWater = NULL;
		}
		
		ncepTotalCloudWater = grib_handle_new_from_file(0, sFile, &errorCode);
		if(ncepTotalCloudWater == NULL){
			fprintf(stderr, "Error (ncepTotalCloudWater) There was a problem reading the grib_handle.  Exiting generateTotalColumnWater()\n");
			return;
		}
		// set the 3 numbers up for determining the product.
		GRIB_CHECK(grib_get_long(ncepTotalCloudWater, "discipline", &ncepDiscipline), 0);
		GRIB_CHECK(grib_get_long(ncepTotalCloudWater, "parameterCategory", &ncepParameterCategory), 0);
		GRIB_CHECK(grib_get_long(ncepTotalCloudWater, "parameterNumber", &ncepParameterNumber), 0);
		GRIB_CHECK(grib_get_long(ncepTotalCloudWater, "typeOfFirstFixedSurface", &ncepLevel), 0);
	}

	// we have the grib_handle now.
	GRIB_CHECK(grib_get_size(ncepTotalCloudWater, "values", &ncepValuesSize), 0);
	ncepTotalCloudWaterValues = calloc(ncepValuesSize, sizeof(double));
	
	if(ncepTotalCloudWaterValues == NULL){
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generateTotalColumnWater()\n");
		exit(1);
	}	
	
	GRIB_CHECK(grib_get_double_array(ncepTotalCloudWater, "values", ncepTotalCloudWaterValues, &ncepValuesSize), 0);	
	

	// both are now stored off.

	
	tiggeTotalColumnWaterValues = calloc(ncepValuesSize, sizeof(double));
	if(tiggeTotalColumnWaterValues == NULL){
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generateTotalColumnWater()\n");
		exit(1);
	}
	
	// for each point in the grid.
	for(i=0; i<ncepValuesSize; i++){
		tiggeTotalColumnWaterValues[i] = ncepTotalPrecipWaterValues[i] + ncepTotalCloudWaterValues[i];
	}
	


	// store the data into the grib file
	GRIB_CHECK( grib_set_double_array(gh, "values", tiggeTotalColumnWaterValues, ncepValuesSize),  0);
	
	
	if(writeGribToFile(gh, fileName) != 0){
		printf("ERROR, did not write the file \"%s\" out successfully.\n", fileName);
	}

	// free allocated memory
	free(ncepTotalPrecipWaterValues);
	free(ncepTotalCloudWaterValues);
	free(tiggeTotalColumnWaterValues);
		
	
	// delete grib_handles
	grib_handle_delete(gh);
	grib_handle_delete(ncepTotalPrecipWater);
	grib_handle_delete(ncepTotalCloudWater);
	// rewind the file
	rewind(sFile);

}
