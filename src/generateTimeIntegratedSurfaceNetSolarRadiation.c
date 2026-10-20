/* generate Time-Integrated Surface Net Solar Radiation
**
**  Generates a valid tigge file for Time-Integrated Surface Net Solar Radiation
**
** Steven Anthony, 2007
**
*/
#include "ncdcTigge.h"

void generateTimeIntegratedSurfaceNetSolarRadiation(int yyyy, int mm, int dd, int hh, int em, void** resource){
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
	
	double* tiggeTimeIntegratedSurfaceNetSolarRadiationValues = NULL;
	
	char fileName[256];
	// check if the space was allocated correctly.
	if(runningSum == NULL){
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
	
	grib_handle* ncepGribRecord = NULL;
	// for each forecast  hour.
	for(fff = 0; fff<=FORECAST_HOURS; fff+=6){
		
	        // populate tmEnd with forecast time offset, add to initial time 
	        tmEnd.yr = 0;
		tmEnd.mo = 0; 
		tmEnd.dy = 0;  
		tmEnd.hr = fff;  
		tmEnd.mn = 0;  
		timeAdd (&tmInit, &tmEnd);

		ncepGribRecord = grib_handle_new_from_message(NULL, resource[fff/6], bytesPerRecord[TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION][fff/6]);
		GRIB_CHECK(grib_get_long(ncepGribRecord, "parameterNumber", &temp), 0);
		
		// read the values.
		ncepValues = calloc(ncepValuesSize, sizeof(double));
		
		GRIB_CHECK(grib_get_double_array(ncepGribRecord, "values", ncepValues, &ncepValuesSize), 0);
				
			// set the fileName
		memset(fileName, 0, 128); //reset the string.
		generateFileName(TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION, yyyy, mm, dd, hh, fff, em, fileName);

			//printf("%s\n", fileName);
			// reset the current fff's values.
		tiggeTimeIntegratedSurfaceNetSolarRadiationValues = calloc(ncepValuesSize, sizeof(double));		

		if(fff == 0){
			//printf("fff == 0\n");
			for(i=0; i<ncepValuesSize; i++){
				//runningSum[i] = ncepValues[i];
				runningSum[i] = 0.0;
				tiggeTimeIntegratedSurfaceNetSolarRadiationValues[i] = 0.0;
			}
		}else
			{
			for(i=0; i<ncepValuesSize; i++)
				{
				runningSum[i] += ncepValues[i];
			//tiggeTimeIntegratedSurfaceNetSolarRadiationValues[i] = (runningSum[i] * (fff*3600.0));
				tiggeTimeIntegratedSurfaceNetSolarRadiationValues[i] = (runningSum[i] * 21600.0 );


/*  An explanation for the above change made Mar.10.2014
from  Richard Wobus  (NCEP GEFS staff) 

~~~ In effect (in reference to the old commented out line)
you are integrating over time in both the flux and
the time factor.

For the 0-24 case you can multiply the average flux:

( Net(0-6) + Net(6-12) + Net(12-18) + Net(18-24) ) / 4

with the total time in seconds:

3600 * 6 * 4

or, canceling out the number of periods, you can
multiply the running sum:


( Net(0-6) + Net(6-12) + Net(12-18) + Net(18-24) )

with the time per period in seconds:

3600 * 6

Looking at these examples in terms of units, the
first method multiplies (watts per meter squared)
with (seconds), while the second method multiplies
(watts x periods per meter squared) with
(seconds per period), and the number of periods
cancels out.

Dick

*/

			/*	Enable for diagnostic output for point 30000

			if( i == 30000 )
                        	{
	                        printf("[%d] Added : %lf   running Sum = %lf   Unit Adjusted = %lf  ::  ens.mem / fff = %d - %d \n",i, ncepValues[i], runningSum[i], tiggeTimeIntegratedSurfaceNetSolarRadiationValues[i] ,em, fff );
	                        }
			*/


				}
			}
		
		
		// now create a new grib_handle to store the data.
		gh = newGribRecord(yyyy, mm, dd, hh, fff, em, n_lat, n_lon);
		
		GRIB_CHECK(grib_set_long(gh, "productDefinitionTemplateNumber", 11), 0);
		
		GRIB_CHECK(grib_set_long(gh, "parameterCategory", 4), 0);
		GRIB_CHECK(grib_set_long(gh, "parameterNumber", 9), 0);
		
		GRIB_CHECK(grib_set_long(gh, "typeOfFirstFixedSurface", 1), 0);
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
	
		// store the data into the grib file
		GRIB_CHECK( grib_set_double_array(gh, "values", tiggeTimeIntegratedSurfaceNetSolarRadiationValues, ncepValuesSize),  0);
		writeGribToFile(gh, fileName);


		// clean up after ourselves.
		grib_handle_delete(gh);
		free(ncepValues);
		free(tiggeTimeIntegratedSurfaceNetSolarRadiationValues);
		grib_handle_delete(ncepGribRecord);
	} //end forecast hours
	

	free(runningSum);
}






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
void** loadDataForTimeIntegratedSurfaceNetSolarRadiation(void** gribBuffers, int n_lat, int n_lon){

	// h1 and h2 will be added to each other.
	grib_handle* h1 = NULL;
	grib_handle* h2 = NULL;
	int err;
	void** retBuffer = NULL;
	const void* buffer = NULL;
	size_t size = 0;
	size_t ncepValuesSize = n_lat * n_lon;
	long sizeLong = 0;
	long ncepDiscipline, ncepCategory, ncepVariable, ncepLevel;
	int i;
	int found1, found2;
	
	double* newValues = NULL;
	double* oldValues1 = NULL;
	double* oldValues2 = NULL;
	size_t oldValues1Size = ncepValuesSize;
	size_t oldValues2Size = ncepValuesSize;

	// test if this is the first one.
	if(!numberOfSavedRecords[TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION]){
		numberOfSavedRecords[TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION] = 0;
	}
	
	// seek to the correct grib_handle we need (first one)

	found1 = found2 = 0;
	ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;

	long ncepVariableFirst   = -1;
	long ncepVariableSecond  = -1;

	// optomized for 2 variables, hopefully this will speed things up quite a bit.
	// while either is not found, continue looking.
	while(!found1 || !found2){
		if( (h1 != NULL) && !found1 ) grib_handle_delete(h1);
		if( (h2 != NULL) && !found2 ) grib_handle_delete(h2);
		
		if( !found1 && !found2 ) {  	// search for the first one.


			h1 = grib_handle_new_from_file(0, sFile, &err);
                        if( h1 == NULL && ignoreMissingInput != 0 )	// fill-in or croak?
                                { h1 = newBlankGribRecord( 0, 4, 192, 1, n_lat, n_lon ); }
                        if (h1 == NULL && ignoreMissingInput == 0 )
                                {
                                printf("Error (TI net solar flux I) unable to create handle from file. error code: %d \n", err);
                                return(NULL);
                                }


			GRIB_CHECK(grib_get_long(h1, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(h1, "parameterCategory", &ncepCategory), 0);
			GRIB_CHECK(grib_get_long(h1, "parameterNumber", &ncepVariable), 0);
			GRIB_CHECK(grib_get_long(h1, "typeOfFirstFixedSurface", &ncepLevel), 0);
			
			// if it is either one of the variables.
			if( ((ncepDiscipline == 0) && (ncepCategory == 4) && (ncepVariable == 193) && (ncepLevel == 1)) ||
				((ncepDiscipline == 0) && (ncepCategory == 4) && (ncepVariable == 192) && (ncepLevel == 1)) ){
				found1 = 1;
				ncepVariableFirst = ncepVariable;	// remember what var# was found first.
				ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
			}
		
		}else if( found1 && !found2){
		// search for the second one.



			h2 = grib_handle_new_from_file(0, sFile, &err);
			if( h2 == NULL && ignoreMissingInput != 0 )
				{ h2 = newBlankGribRecord( 0, 4, 193, 1, n_lat, n_lon ); }
			if (h2 == NULL && ignoreMissingInput == 0 )
				{
				printf("Error (TI net solar flux II) unable to create handle from file. error code: %d \n", err);
				return(NULL);
				}


			GRIB_CHECK(grib_get_long(h2, "discipline", &ncepDiscipline), 0);
			GRIB_CHECK(grib_get_long(h2, "parameterCategory", &ncepCategory), 0);
			GRIB_CHECK(grib_get_long(h2, "parameterNumber", &ncepVariable), 0);
			GRIB_CHECK(grib_get_long(h2, "typeOfFirstFixedSurface", &ncepLevel), 0);
			
			// if it is either one of the variables
			if( ((ncepDiscipline == 0) && (ncepCategory == 4) && (ncepVariable == 193) && (ncepLevel == 1)) ||
				((ncepDiscipline == 0) && (ncepCategory == 4) && (ncepVariable == 192) && (ncepLevel == 1)) ){
				found2 = 1;
				ncepVariableSecond = ncepVariable;       // remember second var#
				ncepDiscipline = ncepCategory = ncepVariable = ncepLevel = -1;
			}
		}
	}

	

	// array for the new values to be stored.
	newValues = calloc(ncepValuesSize, sizeof(double));
	oldValues1 = calloc(ncepValuesSize, sizeof(double));
	oldValues2 = calloc(ncepValuesSize, sizeof(double));

		// This operation must always be done in the order:   Net = [Down - Up]
		// Given the order may be arbituary, make sure its always var# (192 - 193)

	if( (ncepVariableFirst == 192) && (ncepVariableSecond == 193) ) 
		{
		GRIB_CHECK(grib_get_double_array(h1, "values", oldValues1, &oldValues1Size), 0);
		GRIB_CHECK(grib_get_double_array(h2, "values", oldValues2, &oldValues2Size), 0);
		}
	else
		{
		GRIB_CHECK(grib_get_double_array(h2, "values", oldValues1, &oldValues1Size), 0);
		GRIB_CHECK(grib_get_double_array(h1, "values", oldValues2, &oldValues2Size), 0);
		}

//	GRIB_CHECK(grib_get_double_array(h1, "values", oldValues1, &oldValues1Size), 0);
//	GRIB_CHECK(grib_get_double_array(h2, "values", oldValues2, &oldValues2Size), 0);

	for(i = 0; i < ncepValuesSize; i++)
		{
		newValues[i] = oldValues1[i] - oldValues2[i];

		/* Enable for diagnostic output for point 30000

		if( i == 30000 ) 
			{	
			printf("%lf (DSWRF) - %lf (USWRF) = %lf (net)\n", oldValues1[i], oldValues2[i], newValues[i]); 
			}
		*/

		//printf(".");
	}


                // Before reinserting altered values --
                // Need to standardize the decimalScaleFactor or subtle value errors emerge
        // GRIB_CHECK(grib_set_long(h1,"decimalScaleFactor",0),0);
        // GRIB_CHECK(grib_set_long(h1,"binaryScaleFactor",24),0);

		// make an empty temporary grib handle to store calculations
		// necessary b/c the binary and decimal scale factors will be new and clean
	grib_handle* ghTemp;
	ghTemp = newGribRecord(1000, 1, 1, 0, 0, 0, n_lat, n_lon);

	//printf("\n");
	// write the values back into the grib handle.
	GRIB_CHECK( grib_set_double_array(ghTemp, "values", newValues, ncepValuesSize),  0);


	//printf("%ld, %ld, %ld, %ld\n", ncepDiscipline, ncepCategory, ncepVariable, ncepLevel);
	
	GRIB_CHECK(grib_get_long(ghTemp, "totalLength", &sizeLong), 0);
	size = (size_t)sizeLong;

	// returns a pointer to the message (buffer) and its size.
	GRIB_CHECK(grib_get_message(ghTemp, &buffer, &size), 0);
	
	// store the data into the buffer
	retBuffer = addGribRecordToArray(TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION, gribBuffers, buffer, size);
	
		
	// clean up
	grib_handle_delete(h1);
	grib_handle_delete(h2);	
	grib_handle_delete(ghTemp);
	
	free(newValues);
	free(oldValues1);
	free(oldValues2);
	
	rewind(sFile);
	// h should now be set, so we want to add it to the grib handle array, and return it.
	//return addGribRecordToArray(gribBuffers, buffer);
	return retBuffer;
}
