/* generate Potential Temperature
**
**  Generates a valid tigge file for potential temperature (pt) on the potential vorticity
**  level 2 PVU.
**
**  pt = T*(P0/P)**K	; see http://amsglossary.allenpress.com/glossary/search?id=potential-temperature1
**  
**  P0 = 1000 hPa = 100000 Pa
**  P = Pressure (Pa); pgrbb-record 184
**  T = Temperature (K); pgrbb-record 182
**  K = Poisson constant = 0.2854 for dry air; see http://amsglossary.allenpress.com/glossary/search?id=poisson-constant1
**  
**
** Danny Brinegar & Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generatePotentialTemperature(int yyyy, int mm, int dd, int hh, int fff, int em){
	grib_handle* gh = NULL;
	grib_handle* ncepPressureNorth = NULL;
	grib_handle* ncepPressureSouth = NULL;
	grib_handle* ncepTemperatureNorth = NULL;
	grib_handle* ncepTemperatureSouth = NULL;

	grib_handle* temp = NULL;
	int pressureNorthFound, pressureSouthFound, temperatureNorthFound, temperatureSouthFound;

	size_t ncepValuesSize = get_n_points(yyyy,mm,dd,hh,0);
	int n_lat = get_n_lat(yyyy,mm,dd,hh,0);
	int n_lon = get_n_lon(yyyy,mm,dd,hh,0);
	
	double* ncepPressureNorthValues = NULL;
	double* ncepPressureSouthValues = NULL;
	double* ncepTemperatureNorthValues = NULL;
	double* ncepTemperatureSouthValues = NULL;

	double* tiggePotentialTemperatureValues = NULL;

	double levValue;

	char fileName[256];
	long ncepDiscipline, ncepParameterCategory, ncepParameterNumber,ncepTypeOfFirstFixedSurface, ncepScaledValueOfFirstFixedSurface;
	int i;
	int errorCode;
	int skipFlag = 0;
	double tempPressureValue, tempTemperatureValue;
	
	pressureNorthFound = pressureSouthFound = temperatureNorthFound = temperatureSouthFound = 0;
	

	/*
	** Section 1.  read the B file (from NCEP)
	*/
	
	// first, is the file here?
	if(bFile == NULL){
		fprintf(stderr, "Error, could not read the required file.  Exiting generatePotentialTemperature()\n");
		return;
	}
	
	// first create the new skeleton file.
	gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);

	// name format from:   http://tigge.ecmwf.int/ldm_protocol.html
	// Check if cccc=kwne is correct.
	// Check if tt=cf is correct.
	// Check if ll=sl is correct for potential vorticity surface variable.
	// Check if llll=0000 is correct for potential vorticity surface variable.
	generateFileName(POTENTIAL_TEMPERATURE, yyyy, mm, dd, hh, fff, em, fileName);

	// start plugging in the metadata.
	/* Set all the stuff up that defines the potential temperature variable */
	// product definition template number, this means that it conforms to template 4.XX, where XX is this number 
	GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 1), 0);
	
	GRIB_CHECK(grib_set_long(gh, "parameterCategory", 0), 0);
	GRIB_CHECK(grib_set_long(gh, "parameterNumber", 2), 0);
	
	
	GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 109), 0);
	GRIB_CHECK(grib_set_long(gh, "scaleFactorOfFirstFixedSurface", 9), 0);
	GRIB_CHECK(grib_set_long(gh, "scaledValueOfFirstFixedSurface", 2000), 0);
	
	GRIB_CHECK(grib_set_long(gh, "typeOfSecondFixedSurface", 255), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaleFactorOfSecondFixedSurface"), 0);
	GRIB_CHECK(grib_set_missing(gh, "scaledValueOfSecondFixedSurface"), 0);
	

	// Initialize variables.
	ncepDiscipline = ncepParameterCategory = ncepParameterNumber = ncepTypeOfFirstFixedSurface = ncepScaledValueOfFirstFixedSurface = -1;


	// Find and Get data for Pressure.

	int recCount = 0;
	while(	(pressureNorthFound != 1) || (pressureSouthFound != 1) ||
			(temperatureNorthFound != 1) || (temperatureSouthFound != 1) )
		{
		recCount++;

//		printf(" -> Record %d flags(%d %d %d %d)\n",recCount,pressureNorthFound,
//			 pressureSouthFound, temperatureNorthFound, temperatureSouthFound);

			// NULL out any existing record in preperation for next record.
		if( (temp != NULL) && (skipFlag == 0) ){
			grib_handle_delete(temp);
			temp = NULL;
		}
		skipFlag = 0;


		temp = grib_handle_new_from_file(0, bFile, &errorCode);
		if(temp == NULL){
			fprintf(stderr, "Error (generatePotentialTemperature.temp) There was a problem reading the grib_handle.  Exiting generatePotentialTemperature()   Likely ran out of records while looking for required parameters!\n");
			return;	
		}
		// set the 5 numbers up for determining the product.
		GRIB_CHECK(grib_get_long(temp, "discipline", &ncepDiscipline), 0);
		GRIB_CHECK(grib_get_long(temp, "parameterCategory", &ncepParameterCategory), 0);
		GRIB_CHECK(grib_get_long(temp, "parameterNumber", &ncepParameterNumber), 0);
		GRIB_CHECK(grib_get_long(temp, "typeOfFirstFixedSurface", &ncepTypeOfFirstFixedSurface), 0);
//		GRIB_CHECK(grib_get_long(temp, "scaledValueOfFirstFixedSurface", &ncepScaledValueOfFirstFixedSurface), 0);
        GRIB_CHECK(grib_get_double(temp, "scaledValueOfFirstFixedSurface", &levValue), 0);


//		printf("\tR.Meta: %d %d %d %d (%f)\n", ncepDiscipline, 
//			ncepParameterCategory,ncepParameterNumber, 
//			ncepTypeOfFirstFixedSurface,levValue);
	
		// do the tests now.
		// N. Hemisphere pressure
		if( (pressureNorthFound == 0) && (ncepDiscipline == 0) && 
			(ncepParameterCategory == 3) && (ncepParameterNumber == 0) && 
			(ncepTypeOfFirstFixedSurface == 109) && (levValue == 2000.0) )
// (ncepScaledValueOfFirstFixedSurface == 2000) ){
				{
				pressureNorthFound = 1;
				ncepPressureNorth = temp;
				skipFlag = 1;
				continue;
				}
		
		// S. Hemisphere pressure
		if( (pressureSouthFound == 0) && (ncepDiscipline == 0) && 
			(ncepParameterCategory == 3) && (ncepParameterNumber == 0) && 
			(ncepTypeOfFirstFixedSurface == 109) && 
            ((levValue == 2147485648.0) || (levValue = -2147485648.0)) )
// (ncepScaledValueOfFirstFixedSurface == -2147481648) )
				{
				pressureSouthFound = 1;
				ncepPressureSouth = temp;
				skipFlag = 1;
				continue;
				}
		
		// N. Hemisphere temperature
		if( (temperatureNorthFound == 0) && (ncepDiscipline == 0) && 
			(ncepParameterCategory == 0) && (ncepParameterNumber == 0) && 
			(ncepTypeOfFirstFixedSurface == 109) && (levValue == 2000.0) )
// (ncepScaledValueOfFirstFixedSurface == 2000) )
				{
				temperatureNorthFound = 1;
				ncepTemperatureNorth = temp;
				skipFlag = 1;
				continue;
				}
		
		// S. Hemisphere temperature
		if( (temperatureSouthFound == 0) && (ncepDiscipline == 0) && 
			(ncepParameterCategory == 0) && (ncepParameterNumber == 0) && 
			(ncepTypeOfFirstFixedSurface == 109) && 
			((levValue == 2147485648.0) || (levValue = -2147485648.0)) )

// && (ncepScaledValueOfFirstFixedSurface == -2147481648) )

			{
			temperatureSouthFound = 1;
			ncepTemperatureSouth = temp;
			skipFlag = 1;
			continue;
			}

	}


	// extract the values:  pressure North
	GRIB_CHECK(grib_get_size(ncepPressureNorth, "values", &ncepValuesSize), 0);
	ncepPressureNorthValues = calloc(ncepValuesSize, sizeof(double));
	if(ncepPressureNorthValues == NULL){
		fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Pressure, Northern Hemisphere.   Exiting...\n");
		exit(1);
	}
	GRIB_CHECK(grib_get_double_array(ncepPressureNorth, "values", ncepPressureNorthValues, &ncepValuesSize), 0);

	// extract the values:  pressure South
	GRIB_CHECK(grib_get_size(ncepPressureSouth, "values", &ncepValuesSize), 0);
	ncepPressureSouthValues = calloc(ncepValuesSize, sizeof(double));
	if(ncepPressureSouthValues == NULL){
		fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Pressure, Southern Hemisphere.   Exiting...\n");
		exit(1);
	}
	GRIB_CHECK(grib_get_double_array(ncepPressureSouth, "values", ncepPressureSouthValues, &ncepValuesSize), 0);

	// extract the values:  Temperature North
	GRIB_CHECK(grib_get_size(ncepTemperatureNorth, "values", &ncepValuesSize), 0);
	ncepTemperatureNorthValues = calloc(ncepValuesSize, sizeof(double));
	if(ncepTemperatureNorthValues == NULL){
		fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Temperature, Northern Hemisphere.   Exiting...\n");
		exit(1);
	}
	GRIB_CHECK(grib_get_double_array(ncepTemperatureNorth, "values", ncepTemperatureNorthValues, &ncepValuesSize), 0);

	// extract the values:  temperature South
	GRIB_CHECK(grib_get_size(ncepTemperatureSouth, "values", &ncepValuesSize), 0);
	ncepTemperatureSouthValues = calloc(ncepValuesSize, sizeof(double));
	if(ncepTemperatureSouthValues == NULL){
		fprintf(stderr, "ERROR.  Could not allocate the space for the ncep variable: Temperature, Southern Hemisphere.   Exiting...\n");
		exit(1);
	}
	GRIB_CHECK(grib_get_double_array(ncepTemperatureSouth, "values", ncepTemperatureSouthValues, &ncepValuesSize), 0);



	// set up the new values array.
	tiggePotentialTemperatureValues = calloc(ncepValuesSize, sizeof(double));
	if(tiggePotentialTemperatureValues == NULL){
		fprintf(stderr, "Error, There was a problem Allocating the memory for storing the data. Exiting generatePotentialTemperature() 001\n");
		exit(1);
	}
	

	// Turn on the bitmap (this is a land mask)
	GRIB_CHECK( grib_set_long(gh, "bitMapIndicator", 0), 0);


	// for each point in the grid calculate potential temperature.
	for(i=0; i<ncepValuesSize; i++){
		tempPressureValue = 9999.0;
		tempTemperatureValue = 9999.0;
	
		// merge the values if they are both there
		if( (ncepPressureSouthValues[i] != 9999.0) && (ncepPressureNorthValues[i] != 9999.0) ){
			tempPressureValue = (ncepPressureSouthValues[i] + ncepPressureNorthValues[i]) / 2.0;
		}
		if( (ncepTemperatureSouthValues[i] != 9999.0) && (ncepTemperatureNorthValues[i] != 9999.0) ){
			tempTemperatureValue = (ncepTemperatureSouthValues[i] + ncepTemperatureNorthValues[i]) / 2.0;
		}

		// set them to the correct side if only one is filled out. (pressure)
		if( (ncepPressureSouthValues[i] == 9999.0) && (ncepPressureNorthValues[i] != 9999.0) ) {
			tempPressureValue = ncepPressureNorthValues[i];
		}else if( (ncepPressureSouthValues[i] != 9999.0) && (ncepPressureNorthValues[i] == 9999.0) ){
			tempPressureValue = ncepPressureSouthValues[i];
		}else{
			// neither of them exist.
			tempPressureValue = 9999.0;
		}

		// set them to the correct side if only one is filled out. (temperature)
		if( (ncepTemperatureSouthValues[i] == 9999.0) && (ncepTemperatureNorthValues[i] != 9999.0) ) {
			tempTemperatureValue = ncepTemperatureNorthValues[i];
		}else if( (ncepTemperatureSouthValues[i] != 9999.0) && (ncepTemperatureNorthValues[i] == 9999.0) ){
			tempTemperatureValue = ncepTemperatureSouthValues[i];
		}else{
			// neither of them exist.
			tempTemperatureValue = 9999.0;
		}

		
		// now set up the array.
		if( (tempPressureValue == 9999.0) || (tempTemperatureValue == 9999.0) ){
			// we have a missing value somewhere.
			tiggePotentialTemperatureValues[i] = 9999.0;
		}else{
			tiggePotentialTemperatureValues[i] = tempTemperatureValue * pow(100000/tempPressureValue, 0.2854);
		}

	}
	



	// store the data into the grib file
	GRIB_CHECK( grib_set_double_array(gh, "values", tiggePotentialTemperatureValues, ncepValuesSize),  0);
	
	
	if(writeGribToFile(gh, fileName) != 0){
		printf("ERROR, did not write the file \"%s\" out successfully.\n", fileName);
	}

	// free allocated memory
	free(ncepPressureNorthValues);
	free(ncepPressureSouthValues);
	free(ncepTemperatureNorthValues);
	free(ncepTemperatureSouthValues);
	free(tiggePotentialTemperatureValues);

	// delete grib_handles
	grib_handle_delete(gh);
	grib_handle_delete(ncepPressureNorth);
	grib_handle_delete(ncepPressureSouth);
	grib_handle_delete(ncepTemperatureNorth);
	grib_handle_delete(ncepTemperatureSouth);
	
	// rewind the file
	rewind(aFile);
	rewind(bFile);
}
