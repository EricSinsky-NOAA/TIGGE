/* ncdcTigge.h
**
** The main header file for the project.
** defines global enum's and #define's
*/

#ifndef ncdc_tigge_H
#define ncdc_tigge_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <grib_api.h>
#include <mcheck.h>
#include <time.h>
#include <math.h>

// #define ENSEMBLE_MEMBERS 21
#define FORECAST_HOURS 384
#define FORECAST_HOURS_HR 240
#define NUMBER_OF_VARIABLES 13
#define QUEUE_MAX 11


FILE* aFile;
FILE* bFile;
FILE* sFile;
	// when true, will allow missing input to pass by inserting a zero-ed data record in its place
int ignoreMissingInput;


// for saving off some information
int numberOfSavedRecords[NUMBER_OF_VARIABLES];
long bytesPerRecord[NUMBER_OF_VARIABLES][((FORECAST_HOURS/6)+1)*2];
int variablesToProcess[NUMBER_OF_VARIABLES];

//enumerate the variable names so we can refer to them
enum tiggeVariableNames { POTENTIAL_TEMPERATURE,
							SNOW_FALL_WATER_EQUIVALENT, 
							TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT,
							SOIL_MOISTURE, 
							SOIL_TEMPERATURE, 
							TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION,
							TIME_INTEGRATED_SURFACE_LATENT_HEAT_FLUX,
							TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION,
							TIME_INTEGRATED_SURFACE_NET_THERMAL_RADIATION,
							TIME_INTEGRATED_SURFACE_SENSIBLE_HEAT_FLUX,
							TOTAL_COLUMN_WATER,
							TOTAL_PRECIPITATION,
							TEST_VARIABLE,
							TEST_VARIABLE_TI };
							

/* Date/time structure */
struct dt {
  int yr;
  int mo;
  int dy;
  int hr;
  int mn;
};

/* from util.c */
int variableIdFromShortName(char*);
char* variableShortName(int);
int addVariableToProcessList(int, char*);
int processQueueContains(char*);
void fillQueueWithDefault();
void fillQueueWithBogusData();
int writeGribToFile(grib_handle*, char*);
int generateFileName(int, int, int, int, int, int, int, char*);
void generateInputFilename(int, int, int, int, int, int, int, char*);
int get_n_members(int, int, int, int);
int get_n_points(int, int, int, int, int);
int get_n_lat(int, int, int, int, int);
int get_n_lon(int, int, int, int, int);
int get_gen_proc_id(int, int, int, int);
void timadd (struct dt *, struct dt *);
int qleap (int);
void timeAdd (struct dt *dtim, struct dt *dto);
int qleap (int year);

/* from newGribRecord.c */
grib_handle* newGribRecord(int, int, int, int, int, int, int, int );
grib_handle* newBlankGribRecord( long, long, long, long, int, int );

/* from generateVariable.c */
int generateVariable(int, int, int, int, int, int, int, void**);

/* from loadDataForVariable.c */
void** addGribRecordToArray(int, void**, const void*, size_t);
void** loadDataForVariable(int, void**, int, int);
void freeDataForVariable(int, void**);

/* from generateSnowFallWaterEquivalent.c */
void generateSnowFallWaterEquivalent(int, int, int, int, int, int);

/* from generateTimeIntegratedSnowFallWaterEquivalent.c */
void generateTimeIntegratedSnowFallWaterEquivalent(int, int, int, int, int, int, void**);
void** loadDataForTimeIntegratedSnowFallWaterEquivalent(void**, int, int);

/* from generateTimeIntegratedOutgoingLongWaveRadiation.c */
void generateTimeIntegratedOutgoingLongWaveRadiation(int, int, int, int, int, void**);
void** loadDataForTimeIntegratedOutgoingLongWaveRadiation(void**, int, int);

/* from generateTimeIntegratedSurfaceLatentHeatFlux.c */
void generateTimeIntegratedSurfaceLatentHeatFlux(int, int, int, int, int, void**);
void** loadDataForTimeIntegratedSurfaceLatentHeatFlux(void**, int, int);

/* from generateTimeIntegratedSurfaceLatentHeatFlux.c */
void generateTimeIntegratedSurfaceSensibleHeatFlux(int, int, int, int, int, void**);
void** loadDataForTimeIntegratedSurfaceSensibleHeatFlux(void**, int, int);

/* from generateTimeIntegratedSurfaceNetSolarRadiation.c */
void generateTimeIntegratedSurfaceNetSolarRadiation(int, int, int, int, int, void**);
void** loadDataForTimeIntegratedSurfaceNetSolarRadiation(void**, int, int);

/* from generateTimeIntegratedSurfaceNetSolarRadiation.c */
void generateTimeIntegratedSurfaceNetThermalRadiation(int, int, int, int, int, void**);
void** loadDataForTimeIntegratedSurfaceNetThermalRadiation(void**, int, int);

/* from generateTotalPrecipitation.c */
void generateTotalPrecipitation(int, int, int, int, int, void**);
void** loadDataForTotalPrecipitation(void**, int, int);

/* from generateTotalColumnWater.c */
void generateTotalColumnWater(int, int, int, int, int, int);

/* from generatePotentialTemperature.c */
void generatePotentialTemperature(int, int, int, int, int, int);

/* from generateSoilMoisture.c */
void generateSoilMoisture(int, int, int, int, int, int);

/* from generateSoilTemperature.c */
void generateSoilTemperature(int, int, int, int, int, int);

#endif

