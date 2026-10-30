/* util.c
**
** This file provides basic utility functions for the NCDC Tigge Project.
**
*/
#include "ncdcTigge.h"
#include "stdlib.h"

/*
** variableShortName
**	returns the short name of the variable
**  Parameters:
** 	int 		the ID of the variable name.
**
**  Returns:
**	char* var	The parameter name. (the tigge short name), NULL on error.
**  	
*/
char* variableShortName(int id)
	{
	switch(id)
		{
		case POTENTIAL_TEMPERATURE: return "pt";
		case SNOW_FALL_WATER_EQUIVALENT: return "sf6";
		case SOIL_MOISTURE: return "sm";
		case SOIL_TEMPERATURE: return "st";
		case TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT: return "sf";
		case TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION: return "ttr";
		case TIME_INTEGRATED_SURFACE_LATENT_HEAT_FLUX: return "slhf";
		case TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION: return "ssr";
		case TIME_INTEGRATED_SURFACE_NET_THERMAL_RADIATION: return "str";
		case TIME_INTEGRATED_SURFACE_SENSIBLE_HEAT_FLUX: return "sshf";
		case TOTAL_COLUMN_WATER: return "tcw";
		case TOTAL_PRECIPITATION: return "tp";
		case TEST_VARIABLE: return "tv";
		case TEST_VARIABLE_TI: return "tvt";
		default: return NULL;
		}
	}



/*
** variableIdFromShortName
**	returns the ID of the variable based on its short name
**  Parameters:
**	char* var	The parameter name. (the tigge short name)
**
**  Returns:
**  	int 		the ID of the variable name. returns -1 on error.
*/
int variableIdFromShortName(char* var)
	{
	if(!strcmp("pt",   var))
			{ return POTENTIAL_TEMPERATURE; }
	if(!strcmp("sf6",   var))
			{ return SNOW_FALL_WATER_EQUIVALENT; }
	if(!strcmp("sf",  var))
			{ return TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT; }
	if(!strcmp("sm",   var))
			{ return SOIL_MOISTURE; }
	if(!strcmp("st",   var))
			{ return SOIL_TEMPERATURE; }
	if(!strcmp("ttr",  var))
			{ return TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION; }
	if(!strcmp("slhf", var))
			{ return TIME_INTEGRATED_SURFACE_LATENT_HEAT_FLUX; }
	if(!strcmp("ssr",  var))
			{ return TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION; }
	if(!strcmp("str",  var))
			{ return TIME_INTEGRATED_SURFACE_NET_THERMAL_RADIATION; }
	if(!strcmp("sshf", var))
			{ return TIME_INTEGRATED_SURFACE_SENSIBLE_HEAT_FLUX; }
	if(!strcmp("tcw",  var))
			{ return TOTAL_COLUMN_WATER; }
	if(!strcmp("tp",   var))
			{ return TOTAL_PRECIPITATION; }
	if(!strcmp("tv",   var))
			{ return TEST_VARIABLE; }
	if(!strcmp("tvt",  var))
			{ return TEST_VARIABLE_TI; }
	
	// not found
	return -1;
}



/*
** addVariableToProcessList()
**	Checks the parameters and adds them to the list of variables to process.
**  Parameters:
**	int i:   	The parameter number (used as the array index)
**	char* var	The parameter name. (the tigge short name)
**
**  Returns:
**  	int    status:  0=fine, 1=done, -1=failure
*/
int addVariableToProcessList(int i, char* var){
	// if all is listed.
	if(!strcmp(var, "all")){
		return 1;
	}
	// if it is not already in queue
	
	//printf("DEBUG:  processQueueContains(%s): %d\n", var, processQueueContains(var));

	if( !processQueueContains(var) ){
		if(variableIdFromShortName(var) != -1){ // test if it is a sane variable
			variablesToProcess[i] = variableIdFromShortName(var);
			return 0;
		}
	}

	return -1;
}


/*
** processQueueContains()
**	Determines if a variable name has already been requested.
**  Parameters:
**	char* var	The parameter name. (the tigge short name)
**
**  Returns:
**  	int    1=yes, the process queue contains this variable,  0=the process queue does not contain this variable, -1= error
*/
int processQueueContains(char* str){
	int i;
	// check if it is a valid short name.
	if( variableIdFromShortName(str) == -1){
		return -1;
	}

	for(i=0; i<NUMBER_OF_VARIABLES; i++){
		// do the sanity check, the array is set to all -1's to begin with.
		// this is the end of the array.
		if(variablesToProcess[i] == -1){
			return 0;
		}
		if( strcmp(str, variableShortName(variablesToProcess[i]) ) == 0 ){
			return 1;
		}
	}
	// if there was no match.
	return 0;

}



/*
** fillQueueWithDefault()
**	fills the "variables to process" queue with the default (everything) order.
**  Parameters:
**	none
**
*/
void fillQueueWithDefault(){
	variablesToProcess[0] = POTENTIAL_TEMPERATURE;
	variablesToProcess[2] = SOIL_MOISTURE;
	variablesToProcess[3] = SOIL_TEMPERATURE;
	variablesToProcess[4] = TIME_INTEGRATED_OUTGOING_LONG_WAVE_RADIATION;
	variablesToProcess[5] = TIME_INTEGRATED_SURFACE_LATENT_HEAT_FLUX;
	variablesToProcess[6] = TIME_INTEGRATED_SURFACE_NET_SOLAR_RADIATION;
	variablesToProcess[7] = TIME_INTEGRATED_SURFACE_NET_THERMAL_RADIATION;
	variablesToProcess[8] = TIME_INTEGRATED_SURFACE_SENSIBLE_HEAT_FLUX;
    variablesToProcess[1] = TIME_INTEGRATED_SNOW_FALL_WATER_EQUIVALENT;
	variablesToProcess[9] = TOTAL_COLUMN_WATER;
	variablesToProcess[10] = TOTAL_PRECIPITATION;
	variablesToProcess[11] = TEST_VARIABLE;
	variablesToProcess[12] = TEST_VARIABLE_TI;
}

/*
** fillQueueWithBogusData()
**	fills the "variables to process" queue with -1's to start off.
**  Parameters:
**	none
**
*/
void fillQueueWithBogusData(){
	variablesToProcess[0] = -1;
	variablesToProcess[1] = -1;
	variablesToProcess[2] = -1;
	variablesToProcess[3] = -1;
	variablesToProcess[4] = -1;
	variablesToProcess[5] = -1;
	variablesToProcess[6] = -1;
	variablesToProcess[7] = -1;
	variablesToProcess[8] = -1;
	variablesToProcess[9] = -1;
	variablesToProcess[10] = -1;
	variablesToProcess[11] = -1;
	variablesToProcess[12] = -1;
}




/* writeGribToFile()
**   Writes a grib_handle out to a file.
**  
**  Parameters:
**    grib_handle* gh    This is the pointer to the grib_handle structure to be written
**    char* filename     This is the filename that will be written to.
**
**  Returns:
**    int    0 = ok,  anything else = error
**
*/

int writeGribToFile(grib_handle* gh, char* filename)
	{
	// Dan Swank Feb.2016 - Handle possiblity for null pointers reaching here.
	if( gh == NULL || ! gh ) 
		{ return(1); }

	char totalFilename[256];
	const void* buffer;
	char* outputFilenamePrefix = getenv("TIGGE_OUTPUT");

	sprintf(totalFilename, "%s/%s", outputFilenamePrefix, filename);
	// printf("totalFilename: %s\n", totalFilename);
	
	FILE *outGribFile = fopen(totalFilename, "w");
	
        size_t dataSize = 65160;

	// save the message out to the buffer.
	GRIB_CHECK(grib_get_message(gh, &buffer, &dataSize), 0);
		
	// write the buffer to the file.
	if(fwrite(buffer, 1, dataSize, outGribFile) != dataSize){
		printf("Cannot write file out!\n");
		return -1;
	}
	fclose(outGribFile);
	return 0;
}

/* generateFileName()
**   generates the right file name for the tigge variable
**  
**  Parameters:
**    int variableName  The variable id
**    int yyyy          The year
**    int mm            The month
**    int dd            The day
**    int hh            The run-time hour
**    int fff           The current forecast hour
**    int em            The ensemble member number
**    char* fileName    The filename pointer that will be written to. 
**
**  Returns:
**    int    0 = ok,  anything else = error
**
*/
int generateFileName(int variableName, int yyyy, int mm, int dd, int hh, int fff, int em, char* fileName)
	{
	// can go back and return any error codes if need be.
	//char fileName[256];

	char* prefix = "z_tigge_c";
	char* cccc = "kwbc";
	char date[15];
	char* mmmm = "glob";
	char* vvvv = "prod";
	char tt[3];
	char ll[3];
	char ssss[5];
	char nnn[4];
	char llll[5];
	char* param = NULL;
	char* suffix = ".grib";
	
	// date
	sprintf(date, "%04d%02d%02d%02d0000", yyyy, mm, dd, hh);
	
	// type of forecast
	if(em == 0)
		{
		strcpy(tt, "cf");
		}
	else{
		strcpy(tt, "pf");
		}

	// level.
	if( (variableName == POTENTIAL_TEMPERATURE) )
		{
		strcpy(ll, "pv");
		strcpy(llll, "0002");
		}
	else{
		strcpy(ll, "sl");
		strcpy(llll, "0000");
		}

	//forecast hour timestep
	sprintf(ssss, "%04d", fff);
	
	//ensemble member number
	sprintf(nnn, "%03d", em);
	
	// parameter name
	param = variableShortName(variableName);
	
	// putting it all together
	sprintf(fileName, "%s_%s_%s_%s_%s_%s_%s_%s_%s_%s_%s%s", 
	  prefix, cccc, date, mmmm, vvvv, tt, ll, ssss, nnn, llll, param, suffix);

	return 0;
	}

/* generateInputFileName()
**   generates the right ncep file name to be read to generate variables
**  
**  Parameters:
**    int fileNumber    0 = a-File,  1 = b-File
**    int yyyy          The year
**    int mm            The month
**    int dd            The day
**    int hh            The run-time hour
**    int fff           The current forecast hour
**    int em            The ensemble member number
**    char* fileName    The filename pointer that will be written to. 
**
**  Returns:
**    int    0 = ok,  anything else = error
**
*/

void generateInputFilename(int fileNumber, int yyyy, int mm, int dd, int hh, int fff, int em, char* fileName) {

	char* newFileNamePrefix = getenv("TIGGE_INPUT");
	char directoryString[12];
	char modelEnsembleMember[3];
	char fileLetter[3];
	char resString[5];

    sprintf(directoryString, "%04d%02d%02d%02d", yyyy, mm, dd, hh);

    if (fileNumber == 0) {
        strcpy(fileLetter, "a");
        strcpy(resString, "0p50");
    } else if (fileNumber == 1) {
        strcpy(fileLetter, "b");
        strcpy(resString, "0p50");
    } else if (fileNumber == 2) {
        strcpy(fileLetter, "st");
        strcpy(resString, "0p25");
    } else {
        strcpy(fileLetter, "a"); // Fallback safety default
        strcpy(resString, "0p50");
    }

    if (em == 0)
        sprintf(modelEnsembleMember, "c%02d", em);
    else
        sprintf(modelEnsembleMember, "p%02d", em);

    if (strcmp(directoryString, "2020092312") < 0) {
        sprintf(fileName, "%s/%s/ge%s.t%02dz.pgrb2%sf%02d",
                newFileNamePrefix, directoryString, modelEnsembleMember, hh, fileLetter, fff);
    } else {
        sprintf(fileName, "%s/%s/ge%s.t%02dz.pgrb2%s.%s.f%03d",
                newFileNamePrefix, directoryString, modelEnsembleMember, hh, fileLetter, resString, fff);
    }

	return;
}

int get_n_points(int yyyy, int mm, int dd, int hh, int fileNumber) {
    return get_n_lat(yyyy, mm, dd, hh, fileNumber) * get_n_lon(yyyy, mm, dd, hh, fileNumber);
}

int get_n_lat(int yyyy, int mm, int dd, int hh, int fileNumber) {
    char dateString[12];
    sprintf(dateString, "%04d%02d%02d%02d", yyyy, mm, dd, hh);

    if (strcmp(dateString, "2020092312") < 0) {
        return 181;
    } else {
        if (fileNumber == 2) {
            return 721; // 0.25-degree grid
        } else {
            return 361; // 0.50-degree grid ('a' and 'b' files)
        }
    }
}

int get_n_lon(int yyyy, int mm, int dd, int hh, int fileNumber) {
    char dateString[12];
    sprintf(dateString, "%04d%02d%02d%02d", yyyy, mm, dd, hh);

    if (strcmp(dateString, "2020092312") < 0) {
        return 360;
    } else {
        if (fileNumber == 2) {
            return 1440; // 0.25-degree grid
        } else {
            return 720;  // 0.50-degree grid ('a' and 'b' files)
        }
    }
}

int get_n_members(int yyyy, int mm, int dd, int hh) {
	char dateString[12];
	sprintf(dateString, "%04d%02d%02d%02d", yyyy, mm, dd, hh);

	if( strcmp(dateString, "2020092312") < 0 )
		return 21;
	else
		return 31;
}

int get_gen_proc_id(int yyyy, int mm, int dd, int hh) {
	char dateString[12];
	sprintf(dateString, "%04d%02d%02d%02d", yyyy, mm, dd, hh);

	if( strcmp(dateString, "2020092312") < 0 )
		return 80;
	else
		return 107;
}

/* timeAdd()
**  Adds a forecast offset (dto) to a time (dtim). Result is overwritten to dto. 
**  Originally written for GrADS, contributed by Jennifer Adams (jma@iges.org).
*/

void timeAdd (struct dt *dtim, struct dt *dto) {
  int i,cont;
  int mosiz[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

  // Add months and years.  
  dto->mo += dtim->mo;
  dto->yr += dtim->yr;

  // Normalize as needed.
  while (dto->mo>12) {
    dto->mo -= 12;
    dto->yr++;
  }

  // Add minutes, hours, and days.
  dto->mn += dtim->mn;
  dto->hr += dtim->hr;
  dto->dy += dtim->dy;

  // Normalize to days 
  if (dto->mn > 59) {
    i = dto->mn / 60;
    dto->hr += i;
    dto->mn = dto->mn - (i*60);
  }
  if (dto->hr > 23) {
    i = dto->hr / 24;
    dto->dy += i;
    dto->hr = dto->hr - (i*24);
  }

  // Normalize extra days to months/years
  cont = 1;
  while (dto->dy > mosiz[dto->mo] && cont) {
    if (dto->mo==2 && qleap(dto->yr)) {
      if (dto->dy == 29) cont=0;
      else {
        dto->dy -= 29;
        dto->mo++;
      }
    } else {
      dto->dy -= mosiz[dto->mo];
      dto->mo++;
    }
    while (dto->mo > 12) {dto->mo-=12; dto->yr++;}
  }
}

/* Test for leap year.  Rules are:
**  Divisible by 4, it is a leap year, unless....
**  Divisible by 100, it is not a leap year, unless...
**  Divisible by 400, it is a leap year.                           
**  Originally written for GrADS, contributed by Jennifer Adams (jma@iges.org).
*/
int qleap (int year)  {
int i,y;
  y = year;

  i = y / 4;
  i = (i*4) - y;
  if (i!=0) return (0);

  i = y / 100;
  i = (i*100) - y;
  if (i!=0) return (1);

  i = y / 400;
  i = (i*400) - y;
  if (i!=0) return (0);

  return (1);
}



