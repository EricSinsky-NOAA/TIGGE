package ncdcTigge;

use 5.008000;
use strict;
use warnings;

use File::Basename;
use File::Path;

require Exporter;

our $gefs_filepattern = qr(^ge[cp](\d\d)\.t(\d\d)z\.pgrb2([ab])(\.0p50\.)?f(\d{2,3})$);

our @ISA = qw(Exporter);

our %EXPORT_TAGS = ( 'all' => 	[
   qw( %RequiredVariablesA %RequiredVariablesB $GRIB_API_BIN $gefs_filepattern &cleanupLocation &mergeRecords &qcGensFile &qcGensCycle &qcOutput &runLatest &getDTGfromGensFileName &archiveOutput &packCycle &FCT_HOURS &FCT_INC) 
								] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our $VERSION = "1.5";
our $REVISION_DATE = "20190315";

# Preloaded methods go here.

        # Important! sets the location where ECMWF GRIB API executables can be found
our $GRIB_API_BIN = "$ENV{TIGGE_TOOLS}/grib-api/bin";
if(  ( !(-d ${ncdcTigge::GRIB_API_BIN}) || !(-r ${ncdcTigge::GRIB_API_BIN}) || !(-l ${ncdcTigge::GRIB_API_BIN}) ) &&  (-d "$ENV{TIGGE_TOOLS}/../grib-api/bin") )
        {  $ncdcTigge::GRIB_API_BIN = "$ENV{TIGGE_TOOLS}/../grib-api/bin";  }
print STDOUT "    GRIB_API_BIN is [$ncdcTigge::GRIB_API_BIN]\n";


sub get_n_members($) {
    my ($cycle) = @_;
    if ($cycle >= 2020092312) {
        return 31;
    } else {
        return 21;
    }
}

sub getQcNumPoints($) {
    my ($cycle) = @_;
    if ($cycle >= 2020092312) {
        return 259920;   # Number of datapoints contained in 0.5-deg Grid
    } else {
        return 65160;    # Number of datapoints contained in GENS-3 Grid
    }
}


sub BEGIN 
{
print STDOUT "\n ** Using ncdcTigge PERL Module ** \n";
$ncdcTigge::startTime = time;
}

# our $GRIB_API_BIN = "$ENV{TIGGE_TOOLS}/grib-api/bin";

	# PACKAGE LEVEL "Public" (our scope) VARIABLES / CONSTANTS
# our $ENS_MEMBERS    = 20;		# Maximum number of members
our $FCT_HOURS      = 384;		# Maximum forecast hours
our $FCT_INC        = 6;		# Default forecast hour increment

	# For QC -- Record counts per file-type and date
our $LOOSENESS = 1;
our %aAnlRecCount;
our %bAnlRecCount;
our %aFctRecCount;
our %bFctRecCount;

$aAnlRecCount{"0000000000"} = 45;
$bAnlRecCount{"0000000000"} = 283;
$aFctRecCount{"0000000000"} = 51;
$bFctRecCount{"0000000000"} = 288;

$aAnlRecCount{"2010022312"} = 73;
$aAnlRecCount{"2015120212"} = 69;
$bAnlRecCount{"2010022312"} = 258;
$aFctRecCount{"2010022312"} = 79;
$bFctRecCount{"2010022312"} = 262;



	# FTP download variables
our $ftpSrcSite		= "ftpprd.ncep.noaa.gov";		# Gens Download location
our $ftpSrcLogin	= "anonymous";
our $ftpSrcPasswd	= "NOMADS.ncdc\@noaa.gov";
our $ftpTimeout     = 30;		# Timeout setting (seconds)
our $ftpDebug       = 0;		# ??? not sure what values for this do what
our $ftpGetDelay	= 1;		# Seconds to wait between transfers
our $ftpXferGiveUp  = 10;		# Terminate on # number of consecutive failures

	# FTP Upload variables
our $ftpInfoFile = "ftpUploadInfo.txt";
our $ftpDestSite	= "";
our $ftpDestLogin	= "";
our $ftpDestPasswd	= "";
our $ftpDestPath	= "";

	# Set this to 'yes' to disable certain checks
our $forceOption	= "no";

# These 2 variables need to be implemented!
our $ftpRetry		= 3;		# Number of retry attempts upon trapped errors
our $ftpRetryDelay	= 20;		# Seconds to wait between retry

#	For Input QC
our $qcCenterCode 	= "kwbc";	# Accepted originating center code (NWS/NCEP)
our $qcRecByteMin	= 128;		# Minimum byte size for ANY grib2 record
our $qcGensProcessIdNum = 4;	# GRIB2 Generating process ID for GENS model

our $qcGensAnlARecCount	= 45;
our $qcGensFctARecCount	= 51;
our $qcGensAnlBRecCount	= 283;
our $qcGensFctBRecCount	= 288;

our %RequiredVariablesA = (
	"Categorical Snow",			"0,1,95,,,,",
	"Total Precipitation",		"0,1,8,,,,",
	"Tot Precipitable Water",	"0,1,3,200,,,",
	"Accumulated Precip",		"0,1,195,,,,"
	);

our %RequiredVariablesB = (
    "Pressure PV2000",			"3,0,109,2000,,",
	"Temperature PV2000",		"0,0,0,109,2000,,",
	"Soil Temperature 10",		"0,0,0,106,0,106,10",
	"Soil Temperature 40",		"0,0,0,106,10,106,40",
	"Soil Moisture 10",			"2,0,192,106,0,106,10",
	"Soil Moisture 40",			"2,0,192,106,10,106,40",
	"Upward LW Radiation",		"0,5,193,1,,,",
	"Upward LW radiation",		"0,5,192,1,,,",
	"Sfc Latent Heat Flux",		"0,0,10,1,,,",
	"Upward SW radiation",		"0,4,193,1,,,",
	"Downward SW radiation",	"0,4,192,1,,,",
	"Sfc Sensible Heat Flux",	"0,0,11,1,,,",
	"Total Cloud water",		"0,6,6,200,,,"
    );

our @gribLsKeys = (
        "identifier",
        "editionNumber",
        "totalLength",
        "identificationOfOriginatingGeneratingCentre",
        "typeOfGeneratingProcess",
        "perturbationNumber",
        "forecastTime",
        "numberOfDataPoints",
        "parameterCategory",
        "parameterNumber",
        "typeOfFirstFixedSurface",
        "scaledValueOfFirstFixedSurface",
        "typeOfSecondFixedSurface",
        "scaledValueOfSecondFixedSurface",
        "level"
        );

our $Segmentor = "###*****###";

# For output QC
our @tiggeVarCodes = qw(pt sf sm st ttr slhf ssr str sshf tcw tp);







#					cleanupLocation
#===========================================================================

sub cleanupLocation($$;@)
{
my $headline = "$0:ncdcTigge::cleanupLocation(@_) : ";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )     { $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n";

my $curTime = time;
my $location = shift(@_);
my $targetAge = shift(@_);
undef my @EXCLUDE;
push(@EXCLUDE,"\^\\.");
if( $#_ > -1 ) { @EXCLUDE = @_; }

        # Verify Input
if( $targetAge !~ m/^[\d\.]*$/ ) 
{ return("### $headline : Expire Age (ARG2) is not numeric \& positive!\n\n"); }
if( !(-d $location) ) 
{ return("### $headline : $location is not a directory!\n\n"); }
if( !(-r $location) ) 
{ return("### $headline $location not readable by effective user!\n\n"); }

# Precaution: If you can't write it-- you can't delete it :p
if( !(-w $location) ) 
{ return("### $headline $location not writable by effective user!\n\n"); }

$ncdcTigge::startTime = time;
print STDOUT "\n$headline Run on $ENV{'HOSTNAME'} \@ ".localtime($ncdcTigge::startTime)."\n";

my $EXCLUDEpattern = "(".join('|',@EXCLUDE).")";

print "   Cleaning up : $location\n\n";
opendir(INDIR,$location) || return("### ${0}:ncdcTigge::cleanupLocation : Failed opendir $location\n\n");
my @targets = sort grep(!/$EXCLUDEpattern/,readdir(INDIR));
closedir(INDIR);

if( $#targets == -1 )
	{ return("$headline Exiting - No targets found \n"); }

$curTime = time;

my $localPath = "";
undef my @nextLevel;
my $filesRemoved = 0;
my $bytesRemoved = 0;
my $iterationBreaker = 1;
my $timeBreaker = time + 1600;
my $maxElements = $#targets;

	# Give this loop 30 mins or 5 iterations to finish
while ( ($#targets > -1) && ($iterationBreaker < 5 ) && (time < $timeBreaker) )
    {
	if( $maxElements < $#targets ) { $maxElements = $#targets; }
    my $thisTarget = shift(@targets);
	if( $thisTarget =~ m/^\./ ) { next; }
    my $thisTargetPath = "${location}/${localPath}/${thisTarget}";
    $thisTargetPath =~ s/\/*\//\//g;

# print STDOUT "DEBUG : Target : $thisTargetPath\n\n";
    if( (-d $thisTargetPath) )
        {
        my $tmpDir = "${location}/$thisTarget";
#       print "D\t$tmpDir\n";
        opendir(DIR,$tmpDir);
        my @tmpDirFiles = grep(!/$EXCLUDEpattern/,readdir(DIR));
        closedir(DIR);
        foreach my $f ( @tmpDirFiles )
            { push(@nextLevel,"${thisTarget}/${f}"); }
        }
    else
        {
        my @filestat = stat($thisTargetPath);
	if( !(@filestat) || ($#filestat != 12) ) 
		{
		print STDOUT " ! $headline Stat failed on $thisTargetPath\n";
		next;
		}
        my $modTime = $filestat[9];
        my $age = ($curTime - $modTime) / 86400;
        if( $age !~ m/^[\d\.]*$/ )
            {
            print STDOUT "*** Age parse is not numeric on $thisTargetPath";
            next;
            }
        if( $age >= $targetAge )
            {
            $filesRemoved++;
            $bytesRemoved += $filestat[7];
            printf("%.3f\tExpired -> %s\n",$age,$thisTarget);
            unlink($thisTargetPath);
            }
        }

    if( ($#targets == -1) && ($#nextLevel > -1) )
        {
        print "  going to next level  : $#nextLevel entries\n\n";
        @targets = @nextLevel;
        undef @nextLevel;
		$iterationBreaker++;
        }
#	$iterationBreaker++;
    }

my $MbytesRemoved = sprintf("%d",$bytesRemoved/1000000);

# print STDOUT "  DEBUG : Loop took 
#  $iterationBreaker iterations to complete
#  ".int($timeBreaker-time)." seconds to finish.
#  and \@targets once had $maxElements elements in it.\n\n\n";

print STDOUT "\tRemoved $filesRemoved Files\n";
print STDOUT "\t$MbytesRemoved MB\n\n";
return("$headline Returned OK\n\n");
}

#       End			cleanupLocation
#===========================================================================




#					qcGensFile
#=========================================================================

sub qcGensFile($;$)
{
undef my @messages;               # information to return
undef my %RecsPerFctHr;
    # Keep track of records for each forecast hour (should only be 1 fcthr)

# my @rtn2 = (`/bin/ls $ENV{home}`);

        # Inport module level QC Settings
my $CENTER 			= $ncdcTigge::qcCenterCode;
my $REC_BYTE_MIN	= $ncdcTigge::qcRecByteMin;
my $GEN_PROCESS_NUM	= $ncdcTigge::qcGensProcessIdNum;
# my $NUM_POINTS 		= $ncdcTigge::qcNumPoints;
my $MAX_FCT 		= $ncdcTigge::FCT_HOURS;
my $GRIB_LS = "${ncdcTigge::GRIB_API_BIN}/grib_ls";

my $ArecCountAnl = $ncdcTigge::qcGensAnlARecCount;
my $ArecCountFct = $ncdcTigge::qcGensFctARecCount;
my $BrecCountAnl = $ncdcTigge::qcGensAnlBRecCount;
my $BrecCountFct = $ncdcTigge::qcGensFctBRecCount;

# start
my ($inFile, $cycle) = @_;
$inFile =~ s/\/*\//\//g;
my $inFileName = $inFile;
$inFileName =~ s/^.*\///g;
if( !(-e $inFile) )
    { return(1,"File related error","File \"$inFile\" does not exist!\n\n"); }
if( !(-r $inFile) )
    { return(1,"File related error","File \"$inFile\" cannot be read!\n\n"); }
if( !(-e $GRIB_LS) )
    { return(2,"dependancy error","\"$GRIB_LS\" : does not exist!\n\n"); }
if( !(-x $GRIB_LS) )
    { return(2,"dependancy error","$GRIB_LS : cannot be executed!\n\n"); }

# my ($inFileParseName,$inFileParseGrid,$inFileParseDTG,$inFileParseCycle,$inFileParseFct,$inFileParseMem) =
#   $inFileName =~ m/^(gens-.)_(\d)_(\d{8})_(\d{4})_(\d{3})_(\d{2})\.(grb2)$/i;
my ( $inFileParseMem, $inFileParseCycle, $gensFileType, $inFileParseRes, $inFileParseFct ) =
  $inFileName =~ m/$gefs_filepattern/i;

my $gribLsKeysString = join(',',@gribLsKeys);
my $gribLsKeyCount = $#gribLsKeys +1;

my @dtgi = ncdcTigge::getDTGfromGensFileName($inFile);
my $dtg = $dtgi[0];
if( $dtg eq "ERROR" ) 
	{
	return(4,"file problem",
		"DTG be determined by filename {$inFile}::{$dtgi[2]}\n");
	}

# my @dtgi = split(/\_/,$inFile);
# my $dtg = $dtgi[2].substr($dtgi[3],0,2);
# if( $dtg !~ m/^\d{4}[0-1]\d[0-3]\d[0-2]\d$/ ) 
# 	{ 
# 	return(4,"file problem",
# 		"DTG be determined by filename {$inFile}::{$dtg}\n");
# 	}

	# use the right key here depending on the date
foreach my $k ( sort keys (%aAnlRecCount ) )
	{
	if( $dtg ge $k ) 
		{ $ArecCountAnl = $ncdcTigge::aAnlRecCount{$k}; }
	}
foreach my $k ( sort keys (%bAnlRecCount) )
        {
        if( $dtg ge $k )
                { $BrecCountAnl = $ncdcTigge::bAnlRecCount{$k}; }
        }
foreach my $k ( sort keys (%aFctRecCount ) )
	{
	if( $dtg ge $k )
		{ $ArecCountFct = $ncdcTigge::aFctRecCount{$k}; }
	}
foreach my $k ( sort keys (%bFctRecCount ) )
	{
	if( $dtg ge $k )
		{ $BrecCountFct = $ncdcTigge::bFctRecCount{$k}; }
	}

my @varArray;       # Holds list of category,varNum / for all records
my @rtn = (`$GRIB_LS -p $gribLsKeysString $inFile `);
undef my @fileData;
# my @rtn2 = (`/bin/ls $ENV{home}`);

if( $#rtn == -1 ) 
	{
	print STDERR "Somethings wrong, command GRIB_LS returned nothing.
$GRIB_LS -p $gribLsKeysString $inFile\n\n[rtn2]\n\n";

#	print STDOUT (`$GRIB_LS -p $gribLsKeysString $inFile`);
	print STDOUT "\nSYSOUT : " . system "${ncdcTigge::GRIB_API_BIN}/grib_ls";
    print STDOUT "\nSYSOUT : " .`/bin/ls $ENV{TIGGE_TOOLS}`;

    push(@messages,
     "\n\tQC Error: GRIB_LS call failed");
	}

my $numLine = 0;
my $numRec = 0;
foreach my $line ( @rtn )
    {
    $numLine++;
    chomp($line);
    if( !defined($line) || $line eq "" ) { next; }      # Blank line
    $line =~ s/\s*$//g;
    $line =~ s/\s*\s/,/g;
    my @lineData = split(/,/,$line);
    my $lineDataCount = $#lineData +1;

        # Ignore Blank lines & Known Header/Footer lines

    if( ($lineDataCount == 1) && ($lineData[0] eq $inFile) )
        { next; }
    if( $line eq $gribLsKeysString )
        { next; }
    if( ($lineDataCount == 7) && ($lineData[1] eq "of") )
        { next; }
    if( ($lineDataCount == 9) && ($lineData[1] eq "of") )
        { next; }

            # Catch and stop on critical errors

    if( (($lineDataCount == $gribLsKeyCount) &&
            ($lineData[0] eq "GRIB") && ($lineData[1] ne "2")) )
        {
        push(@messages,
        "\n\tQC Error:$numLine  GRIB version $lineData[1] is not GRIB 2\n");
        next;
        }
    if( !(($lineDataCount == $gribLsKeyCount) &&
            ($lineData[0] eq "GRIB") && ($lineData[1] eq "2")) )
        {
        push(@messages,
        "\n\tQC Error:$numLine  grib_ls returned unidentified info: $line\n");
        next;
        }

    $numRec++;
    my $thisVar = "$lineData[8],$lineData[9]";
    push(@varArray,$thisVar);

            # Non-Critical errors -> throw messages and proceed

    if( (($lineDataCount == $gribLsKeyCount) &&
            (int($lineData[2]) < $REC_BYTE_MIN ) ) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Record Size = $lineData[2] < $REC_BYTE_MIN B");
        }
    if( (($lineDataCount == $gribLsKeyCount) &&
            ($lineData[3] ne $CENTER) ) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Orig.Center $lineData[3] is not $CENTER");
        }
    if( (($lineDataCount == $gribLsKeyCount) &&
            ($lineData[4] ne $GEN_PROCESS_NUM) ) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Gen.Process ID $lineData[4] is not $GEN_PROCESS_NUM");
        }
    if( (($lineDataCount == $gribLsKeyCount) &&
            ($lineData[5] < 0 || $lineData[5] >= get_n_members($cycle) ) ) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Invalid mem. number ($lineData[5])");
        }
    if( ($lineDataCount == $gribLsKeyCount) &&
            (($lineData[6] > $MAX_FCT) || ($lineData[7] < 0)) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Invalid forecast hour ($lineData[6])");
        }
    if( ($lineDataCount == $gribLsKeyCount) &&
            ($lineData[7] != getQcNumPoints($cycle)) )
        {
        push(@messages,
        "\n\tQC Warning:$numLine  Wrong #data-points ($lineData[7])");
        }

    $RecsPerFctHr{$lineData[6]}++;

#   print STDOUT " [$lineDataCount] -> $line\n";    }
    }

my @fctHrs = keys(%RecsPerFctHr);
my $fctHrsCount = $#fctHrs+1;

	# Ignore this check on forecast 192.  its different for some reason
if( $fctHrsCount > 2 && ($inFileParseFct != 192) )
    {
    push(@messages,
      "\n\tQC Warning:  More than 2 different forecast hours present!");
    }

    # Determine the primary forecast hour, hr which occurs most
    #   needed in order ot filter out statistical parameters which report
    #  a T-006 hr forecastTime.

my $priFct = "";
my $priFctCount = 0;
foreach my $K ( keys(%RecsPerFctHr) )
    {
    if( $RecsPerFctHr{$K} > $priFctCount )
        { $priFct = $K; $priFctCount = $RecsPerFctHr{$K};  }
    # print STDOUT "DEBUG : $K [$RecsPerFctHr{$K}]\n";
    }

    # Specific checks for A files
if( $gensFileType eq "a" )
    {
    if( ($priFct == 0) && ($numRec < $ArecCountAnl) )
        {
        push(@messages,
        "\n\tQC Warning:  Expected $ArecCountAnl Records, found $numRec");
        }
    if( ($priFct > 0) && ($numRec < $ArecCountFct) )
        {
        push(@messages,
        "\n\tQC Warning:  Expected $ArecCountFct Records, found $numRec");
        }
    }

    # Specific checks for B files
if( $gensFileType eq "b" )
    {
    if( ($priFct == 0) && ($numRec < ($BrecCountAnl-$LOOSENESS)) )
        {
        push(@messages,
        "\n\tTEST QC Warning:  Expected $ArecCountAnl Records, found $numRec\n");
        }
    if( ($priFct > 0) && ($numRec < ($BrecCountFct-$LOOSENESS)) )
        {
        push(@messages,
        "\n\tTEST QC Warning:  Expected $BrecCountFct Records, found $numRec\n");
        }
    }

# print STDOUT "\n\nValid GENS\-$gensFileType file  [$inFileName]\n";
# print STDOUT "Done:  Read $numLine lines, found $numRec Valid Records.\n\n";

return(0,"OK",@messages);
}
#       End         qcGensFile
#===========================================================================


#					qcGensCycle
#=========================================================================

sub qcGensCycle($;$;$)
{
my $headline = "$0:ncdcTigge::qcGensCycle : ";
my $MEM_FILES = 65;
if( $ncdcTigge::FCT_INC != 0 ) 
	{ $MEM_FILES = int($ncdcTigge::FCT_HOURS / $ncdcTigge::FCT_INC) +1; }

my $inDir = shift(@_);
my $cycle = shift(@_);
my $quietOpt = 0;
if( scalar grep(/(1|-q|-quiet)/,@_) ) 
	{ $quietOpt = 1; }

	# Given ambiguous DTG, assume its in TIGGE_INPUT
if( $inDir =~ m/^\d{4}[0-1]\d[0-3]\d[0-2]\d$/ )
	{ $inDir = "$ENV{'TIGGE_INPUT'}/${inDir}"; }
if( !(-e $inDir) )
    { return("$headline Directory \"${inDir}\" does not exist!\n\n"); }
if( !(-r $inDir) )
    { return("$headline Directory \"${inDir}\" cannot be read!\n\n"); }
if( !(-d $inDir) )
    { return("$headline \"${inDir}\" is not a directory!\n\n"); }

print STDOUT "\n$headline Started : ".localtime(time)."\n";
print STDOUT "\tChecking Directory : $inDir...\n\n";

	# Tagfile, do not repeat effort if this cycle has already checked out
my $qcTagFile = "${inDir}/.qcGensCycle.passed";
if( (-e $qcTagFile) && !(-z $qcTagFile) ) 
	{
	open(TAG,"<",$qcTagFile);
	my @tagData = readline(TAG);
	close(TAG);
	my @flagStat = stat($qcTagFile);
	my $flagDate = scalar localtime($flagStat[9]);
	return("$headline $inDir *-*-PASSED-*-* tagFile exists $qcTagFile:
		Already Completed on $flagDate\n\n");
	}

opendir(DIR,$inDir) || return("$headline Error opening $inDir\n");
my @dirAll = readdir(DIR);
closedir(DIR);
my @dirGrb = sort grep(/$gefs_filepattern/i,@dirAll);
my @dirTags = sort grep(/^CURRENTLY\-DOWNLOADING\-\d{1,8}$/i,@dirAll);

	# Check for stale tags.. 
foreach my $dltag ( @dirTags )
	{
	my $tagPath = "$inDir/$dltag";
	my $tagAge = (-M $tagPath);
	if( $tagAge > 0.5 ) 
		{
		print STDOUT "$headline: Releasing stale tag: $tagPath : $tagAge Days old.\n"; 
		unlink($tagPath);
		}
	}

	# rescan

opendir(DIR,$inDir) || return("$headline Error opening $inDir\n");
@dirAll = readdir(DIR);
closedir(DIR);
@dirGrb = sort grep(/$gefs_filepattern/i,@dirAll);
@dirTags = sort grep(/^CURRENTLY\-DOWNLOADING\-\d{1,8}$/i,@dirAll);


	# If any downloads are occuring, take a 15-min nap
if ( $#dirTags > -1 ) 
	{
	print STDOUT "$headline A download is in progress, sleeping 15-minutes -\n";
	sleep(900);
	print STDOUT "$headline   Woke up -- Rescanning dir.\n\n";

	opendir(DIR,$inDir) || return("$headline Error opening $inDir\n");
	@dirAll = readdir(DIR);
	closedir(DIR);
	@dirGrb = sort grep(/$gefs_filepattern/i,@dirAll);
	}

undef my %filesPerMember;
undef my %fileFound;
foreach my $thisFile ( @dirGrb ) {
    my $thisPath = "${inDir}/$thisFile";
    if(  (-z $thisPath) ) { next; }
    if( !(-r $thisPath) ) { next; }
    my @fnParts = split(/[\.\-\_]/,$thisFile);
    my ( $inFileParseMem, $inFileParseCycle, $inFileParseType, $inFileParseFct ) = 
         $thisFile =~ m/$gefs_filepattern/i;
    my $memberCode = "$inFileParseMem-$inFileParseType";
    $filesPerMember{$memberCode}++;
    my $hourCode = "$inFileParseMem-$inFileParseType-$inFileParseFct";
    $fileFound{$hourCode} = 1;
}

for( my $m = 0; $m < get_n_members($cycle); $m++ ) {
    my $aKey = sprintf("%02d-%1s",$m,"a");
    my $bKey = sprintf("%02d-%1s",$m,"b");
	if( !defined($filesPerMember{$aKey}) ) 
		{ return("### $headline FAILED Member \# ${m}-a No files found!\n"); }
	if( !defined($filesPerMember{$bKey}) )
		{ return("### $headline FAILED Member \# ${m}-b No files found!\n"); }
    my $aCount = $filesPerMember{$aKey};
    my $bCount = $filesPerMember{$bKey};
    if( $aCount != $MEM_FILES) {
        for ( my $f=0; $f < $ncdcTigge::FCT_HOURS; $f+=$ncdcTigge::FCT_INC ) {
            my $hourCode = sprintf("%02-a-%02",$m,$f);
            if( !$fileFound{$hourCode} ) {
              print STDERR " ! MISSING A FILE: $inDir Member ${m} / Fct $f \n";
            }
        }
        return("### $headline FAILED Member \# ${m}-a Does not contain $MEM_FILES files, ($aCount Found) \n");
    }
    if( $bCount != $MEM_FILES) {
        for ( my $f=0; $f < $ncdcTigge::FCT_HOURS; $f+=$ncdcTigge::FCT_INC ) {
            my $hourCode = sprintf("%02-b-%02",$m,$f);
            if( !$fileFound{$hourCode} ) {
              print STDERR " ! MISSING B FILE: $inDir Member ${m} / Fct $f \n";
            }
        }
        return("### $headline FAILED Member \# ${m}-b Does not contain $MEM_FILES.\n files, ($bCount Found) \n");
    }
}
    # Execute Slow, deep file scan on every file.
    #   accumulate errors/warnings in @tmp
$| = 1;
undef my @tmp;
my $scanNum = 0;
my $scanTotal = $#dirGrb+1;
foreach my $thisFile ( sort @dirGrb )
    {
    $scanNum++;
	if( !$quietOpt ) 
    	{ printf STDOUT (" %5d/%-5d : $thisFile  ",$scanNum,$scanTotal); }
    my $thisPath = "${inDir}/${thisFile}";
	if( (-z $thisPath) )
		{
		my $errString = "$thisPath :: Zero Byte file";
		push(@tmp,$errString);
		next;
		}

    my @rtn = qcGensFile($thisPath,$cycle);
    my $rtnCode = shift(@rtn);
    my $rtnDesc = shift(@rtn);

    if( $rtnCode )
        {
        my $errString = "$thisPath :: $rtnDesc :: ".join('|',@rtn);
        push(@tmp,$errString);
		if( !$quietOpt )
        	{ print STDOUT "FAIL :return code: $rtnCode @rtn \n"; }
        next;
        }
    if( ($rtnCode == 0) && @rtn )
        {
        my $errString = "$thisPath :: ".join('|',@rtn);
        push(@tmp,$errString);
		if( !$quietOpt )
        	{ print STDOUT "FAIL :Msgs: @rtn \n"; }
        next;
        }
	if( !$quietOpt )
		{ print STDOUT "OK\n"; }
    }
$| = 0;

print STDOUT "\n$headline Finished : ".localtime(time)."\n";
	# Write the TagFile
open(TAGOUT,">",$qcTagFile);
print TAGOUT localtime(time)."\n$headline";
close(TAGOUT);

    # Raise any errors/warnings through return.
if(@tmp)
    {
    my $cnt = $#tmp+1;
    if( -f $qcTagFile )  { unlink($qcTagFile); }
    return("### $headline $cnt problems detected!",@tmp);
    }
return("$headline $inDir *-*-PASSED-*-* ".localtime(time)."\n");
}

#       End         qcGensCycle
#===========================================================================


#                   qcOutput
#=========================================================================

sub qcOutput($)
{
my $headline = "$0:ncdcTigge::qcOutput(@_)";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )     { $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n";

my $cycle = shift(@_);
if( $cycle !~ m/^\d{4}[0-1]\d[0-3]\d[0-2]\d$/ ) 
	{ return("FAIL","$headline : Invalid DTG Argument (@_)"); }
$ENV{'TIGGE_OUTPUT'} || return("FAIL","$headline : ENV VAR TIGGE_OUTPUT, is not defined!\n");

my $tagFile = "$ENV{'TIGGE_OUTPUT'}/OUTPUT.QC.PASS.$cycle";

if( (-f $tagFile) )
   { return("PASS"," Cycle passed previous QC run.\n"); }

if( !(-d $ENV{'TIGGE_OUTPUT'}) )
	{ return("FAIL","$headline : tigge/Output directory does not exist!! \"$ENV{'TIGGE_OUTPUT'}\" \n"); }
if( !(-r $ENV{'TIGGE_OUTPUT'}) )
	{ return("FAIL","$headline : tigge/Output directory is not readable!! \"$ENV{'TIGGE_OUTPUT'}\" \n"); }


my $numVars = $#ncdcTigge::tiggeVarCodes + 1;
my $expectedVarCount = get_n_members($cycle) * ($ncdcTigge::FCT_HOURS / $ncdcTigge::FCT_INC +1);
my $expectedCycleCount = $expectedVarCount * $numVars;

print STDOUT " * Scanning dir; [$ENV{'TIGGE_OUTPUT'}] \n";
opendir(OUTDIR,$ENV{'TIGGE_OUTPUT'}) || 
	return("FAIL","$headline : Couldn't open $ENV{'TIGGE_OUTPUT'} \n");
my @fileList = sort grep(/\.grib$/,readdir(OUTDIR));
closedir(OUTDIR);
my @cycleFileList = grep(/_${cycle}/,@fileList);

# print STDOUT " DEBUG 
# my numVars = $#ncdcTigge::tiggeVarCodes + 1;
# my expectedVarCount = $ncdcTigge::ENS_MEMBERS * $ncdcTigge::FCT_HOURS;
# numVars				$numVars
# expectedVarCount 	$expectedVarCount
# expectedCycleCount	$expectedCycleCount
# [$#fileList] [$#cycleFileList] \n";

my $cycFileCount = $#cycleFileList+1;
if( $cycFileCount < $expectedCycleCount ) 
	{ return("FAIL","$headline : for cycle $cycle : $expectedCycleCount Expected, found $cycFileCount (\*.grib)\n"); }

	# Run basic file-count / var check
foreach my $tVar ( @ncdcTigge::tiggeVarCodes ) 
	{
	my $ptn = "_${tVar}.grib\$";
	my @matches = grep(/$ptn/,@cycleFileList);
	if( int($#matches+1) != $expectedVarCount ) 
		{ return("FAIL","$headline : ".int($#matches+1)." files found for variable $tVar, expected $expectedVarCount\n\n"); }
	}

	# Run tigge_check scanner / var
my $TIGGE_CHECK = "${ncdcTigge::GRIB_API_BIN}/tigge_check";
if( !(-e $TIGGE_CHECK ) )
	{ return("FAIL","$headline : $TIGGE_CHECK executable was not found!\n\n"); }
if( !(-x $TIGGE_CHECK ) )
    { return("FAIL","$headline : $TIGGE_CHECK is not executable!\n\n"); }
undef my @checkErrors;
foreach my $tVar ( @ncdcTigge::tiggeVarCodes )
	{
	my $ptn = "_${tVar}.grib\$";
	my @matches = grep(/$ptn/,@cycleFileList);
	foreach my $f ( @matches ) 
		{
		my $thisPath = "$ENV{'TIGGE_OUTPUT'}/$f";
		my @rtn = (`$TIGGE_CHECK $thisPath`);
		if ( $#rtn > -1 ) 
			{ push(@checkErrors,@rtn); }
		}
	}

### Ignore errors involving marsStartStep
@checkErrors = grep(!/marsStartStep/,@checkErrors);

if( @checkErrors ) 
	{ return("FAIL","$headline : tigge_check found the following problems:\n@checkErrors\n\n"); }

open(TAG,">",$tagFile) || return("ERROR","$headline Could not write the tagfile! $tagFile\n\n"); 
print TAG "PASS - $headline";
close(TAG);

return("PASS",$headline,"Set the tagfile : $tagFile");
}

#       End         qcOutput
#===========================================================================



sub runCycle($;$) {
    my $headline = "$0:ncdcTigge::runCycle(@_) : ";
    my $latestRun = shift(@_);

    my $thisHOST = "Unknown Host";
    if( defined($ENV{'HOSTNAME'}) )     { $thisHOST = $ENV{'HOSTNAME'}; }
    print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n\n";
    print STDOUT " ENV:
     TIGGE_TOOLS:  $ENV{TIGGE_TOOLS}
     TIGGE_INPUT:  $ENV{TIGGE_INPUT}
     TIGGE_OUTPUT: $ENV{TIGGE_OUTPUT}\n\n\n";

    my $QC = "yes";
    if ( scalar grep(/^-noQC/,@_) )
        {  $QC = "no";  }

    if( !defined $ENV{'TIGGE_INPUT'}) {
        print STDOUT "### $headline : Required ENV VAR : TIGGE_INPUT, is not set!\n\n";
        return(1);
    }
    if (!defined $ENV{'TIGGE_TOOLS'}) {
        print STDOUT "### $headline : Required ENV VAR : TIGGE_TOOLS, is not set!\n\n";
        return(1);
    }

    if( !(-d $ENV{'TIGGE_INPUT'}) ) {
        print STDOUT "### $headline : \$TIGGE_INPUT is not set to a valid directory!\n\n";
        return(2);
    }
    if( !(-r $ENV{'TIGGE_INPUT'}) ) {
        print STDOUT "### $headline : \$TIGGE_INPUT is not a readable directory!\n\n";
        return(2);
    }

    if( !(-d $ENV{'TIGGE_OUTPUT'})) {
        mkdir $ENV{'TIGGE_OUTPUT'} || die "Could not create output directory $ENV{'TIGGE_OUTPUT'}";
    }

     # Cannot allow the same cycle to be run by two different processes at once.
    my $runLatestLOCKFILE = "$ENV{'TIGGE_OUTPUT'}/runLatest.LOCKFILE.$latestRun";
    if( -e $runLatestLOCKFILE ) 
    {
        print STDOUT "\n *ABORTED*  This processes is currnetly running!\n";
        print STDOUT "   If this persists, remove lock.  rm $runLatestLOCKFILE\n";
        return(3);
    }

    my $latestRunPath = "$ENV{'TIGGE_INPUT'}/$latestRun";
    my $tagFile = "${latestRunPath}/.completed";

        # Begin work.  LOCK this cycle here.
    # anything that breaks this loop must also unlink this LOCKFILE
    open(my $theLOCKFILE,">",$runLatestLOCKFILE);
    print $theLOCKFILE "$$\n".localtime(time);
    close($theLOCKFILE);

    if( -d "$ENV{'TIGGE_OUTPUT'}/$latestRun" ) {
        print STDOUT qq(Removing output directory $ENV{'TIGGE_OUTPUT'}/$latestRun\n);
        rmtree("$ENV{'TIGGE_OUTPUT'}/$latestRun");
    }

    if( -d "$ENV{'TIGGE_OUTPUT'}/archive/$latestRun" ) {
        print STDOUT qq(Removing archive directory $ENV{'TIGGE_OUTPUT'}/archive/$latestRun\n);
        rmtree("$ENV{'TIGGE_OUTPUT'}/archive/$latestRun");
    }

    unlink("$ENV{'TIGGE_OUTPUT'}/archive/tigge-kwbc-${latestRun}.tar");

    print STDOUT "---------------------------------------------------\n";
    print STDOUT "$latestRun\tLock: $runLatestLOCKFILE\n\n";
    print STDOUT "$headline Checking : $latestRunPath for errors ...\n ";

        # The QC is a long process and may be turned off with -noQC
    if( $QC eq "yes" )
    {
        my @QCresults = ncdcTigge::qcGensCycle($latestRunPath,$latestRun);
        if( $QCresults[0] !~ m/\*\-\*\-PASSED\-\*\-\*/ )
            {
            print STDOUT "$headline : Cycle $latestRun : Input QC did not pass!\n";
            print STDOUT "  Returned information:\n".join("\n",@QCresults)."\n\n";
            print "$headline Moving to next cycle.\n\n";
            unlink($runLatestLOCKFILE);
            print STDOUT "Input failed QC";
            return(10);
            }
    }


    # QC Passed, throw the switch!
    # Note - the ncdcTigge application needs to be run with cwd ".", 
    #   so a chdir needs to be called first

    # This is the header conversion portion that NCAR used to do.
    # it is a standalone process -- for now.
#   print STDOUT (`perl run_ncep_convert.pl $latestRun`);
    my $output = `perl run_ncep_convert.pl $latestRun`;
    print STDOUT $output;
    if ($? != 0) {
        die "CRITICAL: run_ncep_convert.pl failed! Stopping run_tigge.\n";
    }

    print STDOUT (" $ENV{TIGGE_TOOLS}/bin/ncdcTigge $latestRun all
    Is currently running\n\n");
    chdir("$ENV{TIGGE_TOOLS}") || print STDOUT "### $headline chdir($ENV{TIGGE_TOOLS}) failed!\n\n";
    my @execute = (`./bin/ncdcTigge $latestRun all`);

    print STDOUT ("  Processor Finished.\n\n".localtime(time)."\n\noutput:\n");
    print STDOUT join("\n",@execute)."\n\n";

    # QC the results, and move to output/archive directory
    print STDOUT "$headline Checking $latestRun Run output...\n\n";
    my @qcOutputResults =  ncdcTigge::qcOutput($latestRun);
    if( $qcOutputResults[0] ne "PASS" )
        {
        print STDOUT "### $0: Detected issues with output data\n";
        print STDOUT "### $0: qcOutput Returned the following notices:\n";
        print STDOUT join("\n",@qcOutputResults)."\n\n";
        print STDOUT "Output QC failed";
        return(12);
        }
    if( $qcOutputResults[0] eq "PASS" )
        {
            # Output is OK, proceeed with the final steps
        print STDOUT "$0 : QC Passed.  Sending output to its archive directory...\n\n";
        print STDOUT ncdcTigge::archiveOutput("_${latestRun}0000_",
            "\^OUTPUT.QC.PASS.${latestRun}\$");
        print STDOUT ncdcTigge::packCycle($latestRun);

            # Mark a completed run with hidden file
        open(TAG,">",$tagFile)
          || print STDOUT "### $headline Error writing tag file!\n";
        print TAG "$$\n".localtime(time);
        close(TAG);
        }

    unlink($runLatestLOCKFILE);

    print STDOUT "$headline Completed & Unlocked cycle $latestRun!\n\n\n";
    print STDOUT "\n ===---===---===---===---===---===---===---===\n\n";
    return(0);
}


#                   runLatest
#===========================================================================

sub runLatest($;$)
{
my $headline = "$0:ncdcTigge::runLatest(@_) : ";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )     { $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n\n";
print STDOUT " ENV:
 TIGGE_TOOLS:  $ENV{TIGGE_TOOLS}
 TIGGE_INPUT:  $ENV{TIGGE_INPUT}
 TIGGE_OUTPUT: $ENV{TIGGE_OUTPUT}\n\n\n";

my $NUMRUNS = shift(@_);
if( $NUMRUNS !~ m/^\-?\d*$/ ) 
	{ return("### invalid argument for num of cycles"); }
my $QC = "yes";
if ( scalar grep(/^-noQC/,@_) )
    {  $QC = "no";  }

$ENV{'TIGGE_INPUT'} ||
    return("### $headline : Required ENV VAR : TIGGE_INPUT, is not set!\n\n");
$ENV{'TIGGE_TOOLS'} ||
    return("### $headline : Required ENV VAR : TIGGE_TOOLS, is not set!\n\n");

if( !(-d $ENV{'TIGGE_INPUT'}) )
    { return("### $headline : \$TIGGE_INPUT is not set to a valid directory!\n\n"); }
if( !(-r $ENV{'TIGGE_INPUT'}) )
    { return("### $headline : \$TIGGE_INPUT is not a readable directory!\n\n"); }

# Ensure input & output symlinks are in place
if( !(-e "$ENV{'TIGGE_TOOLS'}/input") )
	{ symlink("$ENV{'TIGGE_INPUT'}","$ENV{'TIGGE_TOOLS'}/input"); }
if( !(-e "$ENV{'TIGGE_TOOLS'}/output") )
	{ symlink("$ENV{'TIGGE_OUTPUT'}","$ENV{'TIGGE_TOOLS'}/output"); }

print STDOUT "$headline Finding latest GENS cycle(s)...\n";
opendir(IN,$ENV{'TIGGE_INPUT'});
my @directories = reverse sort grep(/^\d{4}[0-1]\d[0-3]\d(00|06|12|18)$/,readdir(IN));
close(IN);

	# Clear out empty-expired directories
foreach my $d ( sort @directories ) 
	{
	my $subDir = "$ENV{'TIGGE_INPUT'}/$d";
	if( -l $subDir ) { next; } 	# Ignore symbolic links as potential rerun attempts.
	my $subDirAge = (-M $subDir);
	opendir(MYDIR,$subDir);
	my @contents = grep(!/^\.{1,2}$/,readdir(MYDIR));
	closedir(MYDIR);
	my @gribContents = grep(/$gefs_filepattern/,@contents);
	if( ($#gribContents == -1) && ($#contents == -1) && ($subDirAge > 2.0 ) ) 
		{ print STDOUT "   Removed: $subDir\n"; rmdir($subDir); }
	elsif( ($#gribContents == -1) && ($#contents > -1) && ($subDirAge > 0.0 ) )
		{ 
		foreach my $f ( @contents ) 
			{
			my $target = "$subDir/$f";
			if( (-f $target) ) 
				{ 
				print STDOUT "        Removed  $target\n";
				unlink($target); 
				}
			}
		print STDOUT "   Removed: $subDir\n";
		rmdir($subDir);
		}
	}


	# Look for completion flags in each dir, to avoid re-processing
	#	Look in TIGGE_INPUT First to see which DTG /CAN/ be processed
	#	Then look at TIGGE_OUTPUT to ensure it needs to be

undef my @uncompletedRuns;
foreach my $dir ( @directories )
    {
    my $thisPath = "$ENV{'TIGGE_INPUT'}/${dir}";
    opendir( my $T, $thisPath ) || next;
    my @testList = readdir( $T );
    closedir( $T );
    my $tagFile = "${thisPath}/.completed";
       # invalid input directory does not contain enough files ... do not consider it
    if( $#testList < 10 )       { next; }
    if( !(-e $tagFile) )	{ push(@uncompletedRuns,$dir); }
    }

@uncompletedRuns = reverse sort @uncompletedRuns;

if( $NUMRUNS < 0 )      # Negative number reverses direction of runs- (oldest to newest)
	{
	$NUMRUNS = $NUMRUNS * -1;
	@uncompletedRuns = reverse @uncompletedRuns;
	}

if( $#uncompletedRuns == -1 )
    { return("$headline : I have nothing left to do!\n\n"); }

print STDOUT "\n$headline Found ".int($#uncompletedRuns+1)." Unprocessed runs:\n";
my $tmpCounter = 0;
foreach my $dir ( @uncompletedRuns )
    { print STDOUT " [$tmpCounter] $dir\n"; $tmpCounter++; }

print STDOUT "$headline Verifying uncompleted runs...\n\n";
foreach my $run ( sort @uncompletedRuns ) 
	{
	my $pathToOutTag = "$ENV{'TIGGE_OUTPUT'}/archive/$run/OUTPUT.QC.PASS.$run";
	my $runLatestLOCKFILE = "$ENV{'TIGGE_OUTPUT'}/runLatest.LOCKFILE.$run";
	if(-f $pathToOutTag) 
		{ next; }
	if( -e $runLatestLOCKFILE ) 
		{
		print STDOUT " DTG $run : Currently Processing! \n";
		next;
		}
	print STDOUT " DTG $run : Hasn't been completed! \n";
	}

print STDOUT "\n\n$headline Processing...\n\n";

    # Run through last N incomplete cycles

for( my $r = 0; $r < $NUMRUNS; $r++ )
    {
    my $latestRun = $uncompletedRuns[$r];
    if( !defined($latestRun) ) { next; }
    runCycle($latestRun);
	}

return("\n$headline returned OK\n\n");
}

#       End         runLatest
#===========================================================================


#                   archiveOutput
#===========================================================================

sub archiveOutput(;@)
{
my $headline = "$0:ncdcTigge::archiveOutput(@_) : ";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )		{ $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n";

my $outputDir = "$ENV{'TIGGE_OUTPUT'}";
my $archiveDir = "${outputDir}/archive";

if( !(-d $archiveDir) ) { mkdir($archiveDir); }

opendir(DIR,$outputDir);
my @allFiles = grep(!/(^\.|^archive|\.tar\.gz$)/i,readdir(DIR));
closedir(DIR);

	# apply filters
if( @_ ) 
	{
	my $ptrn = "(".join('|',@_).")";
	@allFiles = grep(/$ptrn/,@allFiles);
	}

$| = 1;
undef my %summary;
foreach my $file ( @allFiles ) 
	{
	my @parts = split(/[\_\.]/,$file);
	my $sourcePath = "${outputDir}/${file}";
	my $partsCount = $#parts + 1;

	if( ($parts[0] eq "OUTPUT") && ($parts[1] eq "QC") ) 
		{
		my $yyyymmddhh = $parts[3];
		my $destDir = "${archiveDir}/${yyyymmddhh}";

			# Verify the archive directory can be created.
		if( (-e $destDir) && !(-d $destDir) ) 
			{ unlink($destDir); }
		if( !(-d $destDir) ) 
			{ mkdir($destDir); }
		if( !(-d $destDir) )
			{ 
			print STDOUT "$headline Cannot creat earchive dir : $destDir\nAbort!\n";
			return;
			}
		my $destPath = "${destDir}/${file}";

		# clean this up later
		print STDOUT (`mv $sourcePath $destDir`);

		next;
		}

	my $dtg = $parts[4];
	my $yyyymmddhh = substr($dtg,0,10);

	if( $partsCount != 14 )
		{ print STDOUT "### $headline Invalid output filename: $file\n"; }
	if( $yyyymmddhh !~ m/^\d{4}[0-1]\d[0-3]\d(00|06|12|18)$/ ) 
		{ print STDOUT "### $headline Parsed DTG is invalid [$yyyymmddhh]\n"; }
	
	my $bufferData;
	my $destDir = "${archiveDir}/${yyyymmddhh}";
	my $destPath = "${destDir}/${file}";

	if( !(-d $destDir) ) 
		{ mkdir($destDir) || return("### $headline Couldn't create directory to archive output!  $destDir\n\n"); }

	my @inStat = stat($sourcePath);
	my $srcSize = $inStat[7];
	open( INSRC, "<", $sourcePath) || return("### $headline Could not read directory:  $sourcePath\n");
	binmode( INSRC );
	my $verif = read( INSRC, $bufferData, $srcSize );
	close( INSRC );

	open( OUT, ">", $destPath );
	binmode(OUT);
	print OUT $bufferData;
	close(OUT);

	my @copyStat = stat($destPath);
	my $copySize = $copyStat[7];
	if( $copySize == $srcSize ) 
		{ unlink($sourcePath); }
	else
		{
		print STDERR "$headline Copy failed on $sourcePath >> $destPath\n\n";
		unlink($destPath);
		}
	$summary{$yyyymmddhh}++;
	}
$| = 0;
print STDOUT "$headline : Finished OK\n\n";
return;
}

#       End         archiveOutput
#===========================================================================




#                   mergeCycleMembers
#===========================================================================

sub mergeCycleMembers($)
{
my $headline = "$0:ncdcTigge::mergeCycleMembers(@_) : ";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )     { $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n";

$ENV{'TIGGE_OUTPUT'} || return("### $headline Essential ENV VAR TIGGE_OUTPUT is not defined!\n\n");

my $cycle = shift(@_);
my $outputDir 			= "$ENV{'TIGGE_OUTPUT'}";
my $archiveDir 			= "${outputDir}/archive";
my $archiveTargetDir 	= "${outputDir}/archive/${cycle}";

if( !(-d $archiveTargetDir) ) 
	{ return("### $headline No such archive directory: $archiveTargetDir\n"); }
if( !(-r $archiveTargetDir) )
	{ return("### $headline Can't read archive directory: $archiveTargetDir\n"); }
if( !(-w $archiveTargetDir) )
    { return("### $headline Can't write to directory: $archiveTargetDir\n"); }

print STDOUT "Scanning directory $archiveTargetDir\n\n";
opendir(DIR,$archiveTargetDir) || print STDOUT "\n### $headline Failed to opendir $archiveTargetDir\n\n";
my @cycleFiles = grep(!/^\./,readdir(DIR));
closedir(DIR);
if( $#cycleFiles == -1 ) 
	{ return("\n$headline : Nothing found in $archiveTargetDir\n\n"); }

my $yyyymmdd = substr($cycle,0,8);
my $hh00 = substr($cycle,8,2)."00";

for( my $m = 0; $m < get_n_members($cycle); $m++ )
	{
	my $memberFileName = sprintf("gens-ncdc_3_%08d_%04d_%02d.grb2",$yyyymmdd,$hh00,$m);
	my $memberFilePath = "${archiveTargetDir}/${memberFileName}";
	my $memberKey = sprintf("%03d",$m);
	print STDOUT "Merging Member ${m} ->\t";
	my @memberFiles = grep(/^z_tigge_c_kwbc_${cycle}0000_[a-z]{4}_[a-z]{4}_[a-z]{2}_[a-z]{2}_${memberKey}\.grib$/,@cycleFiles);
	# my @memberFiles = grep(/^z_tigge_c_kwbc_${cycle}0000_[a-z]*_[a-z]*_[a-z]*_[a-z]*_\d{4}_${memberKey}_(\d|\?){4}_[a-z]*\.grib$/,@cycleFiles);
	# print "    my \@memberFiles = grep(/\^z_tigge_c_kwbc_\${cycle}0000_[a-z]*_[a-z]*_[a-z]*_[a-z]*_\d{4}_\${memberKey}_(\d|\?){4}_[a-z\]\*\.grib\$\/,\@cycleFiles); ";
	if( $#memberFiles == -1 ) 
		{ print STDOUT " ! $0 Member $memberKey : No files!\n"; next; }

	print STDOUT " (".int($#memberFiles+1).") ".int($#cycleFiles+1)."  ";

		# Consolidate the files
	open(OUT,">",$memberFilePath);
	foreach my $f ( @memberFiles ) 
		{
		print OUT "\n$ncdcTigge::Segmentor $f\n";
		my $thisPath = "${archiveTargetDir}/${f}";
		my @statF = stat($thisPath);

		my $bufferData;
		open(IN,"<",$thisPath);
		binmode(IN);
		read(IN,$bufferData,$statF[7]);
		close(IN);

		print OUT $bufferData;
		undef $bufferData;
		}
	close(OUT);
	print STDOUT "$memberFileName\n";
	}

return("$headline Done  ".localtime(time)."\n\n");
}

#       End         mergeCycleMembers
#===========================================================================




#                   packCycle
#===========================================================================

sub packCycle($)
{
my $headline = "$0:ncdcTigge::packCycle(@_) : ";
my $thisHOST = "Unknown Host";
if( defined($ENV{'HOSTNAME'}) )		{ $thisHOST = $ENV{'HOSTNAME'}; }
print STDOUT "\n$headline Run on $thisHOST \@ ".localtime(time)."\n";

$ENV{'TIGGE_OUTPUT'} || return("### $headline Essential ENV VAR TIGGE_OUTPUT is not defined!\n\n");

my $cycle = shift(@_);
my $outputDir         = "$ENV{'TIGGE_OUTPUT'}";
my $archiveDir        = "${outputDir}/archive";
my $archiveTargetDir  = "${outputDir}/archive/${cycle}";
my $statusFileAux     = "${outputDir}/OUTPUT.QC.PASS.${cycle}";
my $statusFile        = "${archiveTargetDir}/OUTPUT.QC.PASS.${cycle}";
my $masterArchive     = "${archiveDir}/tigge-kwbc-${cycle}.tar";

if( !(-d $archiveTargetDir) )
    { return("### $headline No such archive directory: $archiveTargetDir\n"); }
if( !(-r $archiveTargetDir) )
    { return("### $headline Can't read archive directory: $archiveTargetDir\n"); }
if( !(-w $archiveTargetDir) )
    { return("### $headline Can't write to directory: $archiveTargetDir\n"); }

	# for some reason the status file may have been placed in the tigge/output directory
	# instead of tigge/output/archive/ area... attempt to move it.
if( (-f $statusFileAux) && !(-f $statusFile) ) 
	{ rename($statusFileAux,$statusFile); }
if( !(-f $statusFile) )
    { return("### $headline No status file: QC did not pass on this cycle!  $statusFile \n"); }
if( !(-r $statusFile) )
    { return("### $headline Can't read status file: $statusFile\n"); }
if( !(-w $statusFile) )
    { return("### $headline Can't write to directory: $statusFile\n"); }

open(STS,"<",$statusFile);
my @statusInfo = readline(STS);
close(STS);

if( scalar grep(/^DATA_PACKAGED_SUCCESSFULLY/,@statusInfo) ) 
	{ return("$headline Cycle has already packaged.\n"); }

chdir( $archiveTargetDir );
print STDOUT "$headline in $archiveTargetDir Tarring files into $archiveDir...\n";
opendir( DIR,$archiveTargetDir );
closedir(DIR);

ncdcTigge::mergeRecords( $archiveTargetDir, $archiveTargetDir, $cycle );
print STDOUT "Merging into master archive: $masterArchive\n\n";
unlink($masterArchive);
my @tarRtn =  (`tar -cf $masterArchive z_tigge_c_kwbc_${cycle}0000_*.grib`);

if( @tarRtn ) 
	{
	print STDOUT "$headline $masterArchive Returned @tarRtn\n\n";
	return("### $headline  Returned from tar $masterArchive:  @tarRtn\n\n");
	}

# my @rtn = ncdcTigge::mergeCycleMembers($cycle);
# print STDOUT "ncdcTigge::mergeCycleMembers Call returned:\n@rtn\n\n";

# Flag the status file as sucesssfully processed
push(@statusInfo,"\nDATA_PACKAGED_SUCCESSFULLY ".localtime(time)."\n");
open(STSOUT,">",$statusFile);
print STSOUT @statusInfo;
close(STSOUT);

print STDOUT "$headline : Updated Status file : $statusFile\n";
return("\n$headline Finished  ".localtime(time)."\n\n");
}

#       End			packCycle
#===========================================================================


#
#	Checks a cycle of input data for basic file existance.
#	If missing, attempts recovery from NCEP-NOMADS component.
#	Recovery depends on wget
#

# sub ncepNomadsBackupSource($;$)
# 	{
# 	my $cycle = shift(@_);
# 	my $force = 0;
# 	if( ($#ARGV > -1) && ($ARGV[0] == 1) )
# 		{ $force = 1; }
# 	my $directory = "$ENV{TIGGE_INPUT}/$cycle";

# 	if( $cycle !~ m/^\d{10}$/ ) 
# 		{ return("### invalid input ($cycle) need 10 digit INT"); }
# 	if( ! -d $directory ) 
# 		{ return( "### not a directory: $directory" ); }

# 	my $localAge = int( (-C $directory) * 24);
# 	if( $localAge < 4 ) 
# 		{ return( "### $directory changed within 4 hours, ignoring for safety." ); }

# 	my $ymd = substr($cycle,0,8);
# 	my $cyc = substr($cycle,8,2);


# 	# internal recovery subroutine, ARGS:
# 	# dtg,fct,mem,type,outputReNamer (full path)

# 	sub attemptRecovery($$$$$)
# 		{
# 		my $dtg  = shift(@_);
# 		my $fct  = shift(@_);
# 		my $mem  = shift(@_);
# 		my $type = lc(shift(@_));
# 		my $output = shift(@_);

# 		my $ymd = substr($dtg,0,8);
# 		my $cyc = substr($dtg,8,2);
# 		my $prefix = "gec";
# 		if( int($mem) > 0 ) { $prefix = "gep"; }

# 		my $baseURL = "http://nomads.ncep.noaa.gov/pub/data/nccf/com/gens/prod";
# 		my $subURL = sprintf("gefs.${ymd}/${cyc}/pgrb2${type}/${prefix}%02d.t${cyc}z.pgrb2${type}f%02d",
# 			$mem,$fct);
# 		my $fullURL = "$baseURL/$subURL";
		
# 		print STDOUT "auto-recovering: $fullURL > $output \n";
# 		my @capture = (`wget --tries=3 -O $output $fullURL >&/dev/null`);

# 			# verify
# 		if( (-e $output) && (-s $output) )  { return(1); }
# 		else { return(0); }
# 		}

# 	if( !(-r $directory) )
# 		{ return("### Cannot read directory : $cycle ($directory)"); }

# 	my $ok = 0;
# 	my $notok = 0;
# 	my $recovered = 0;
# 	for( my $mem = 0; $mem <= 20; $mem += 1 )
# 		{
# 		for( my $fct = 0; $fct <= 384; $fct += 6 ) 
# 			{
# 			my $localFileNameA = sprintf("gens-a_3_${ymd}_${cyc}00_%03d_%02d.grb2",
# 				$fct,$mem);
# 			my $localFileNameB = sprintf("gens-b_3_${ymd}_${cyc}00_%03d_%02d.grb2",
# 				$fct,$mem);
# 			my $localFilePathA = "${directory}/${localFileNameA}";
# 			my $localFilePathB = "${directory}/${localFileNameB}";

# 			if( (-e $localFilePathA) && (-z $localFilePathA) ) 
# 				{ unlink($localFilePathA); }
# 			if( (-e $localFilePathB) && (-z $localFilePathB) )
# 				{ unlink($localFilePathB); }

# 			if( !(-e $localFilePathA) ) 
# 				{ 
# 				$notok++;
# 				#print STDOUT " Need < $localFilePathA\n";
# 				$recovered += attemptRecovery($cycle,$fct,$mem,"a",$localFilePathA);
# 				}
# 			else
# 				{ $ok++; }

# 			if( !(-e $localFilePathB) )
# 				{ 
# 				$notok++;
# 				# print STDOUT " Need < $localFilePathB\n";
# 				$recovered += attemptRecovery($cycle,$fct,$mem,"b",$localFilePathB);
# 				}
# 			else { $ok++;  }

# #			print STDOUT "$localFileNameA $localFileNameB\n";

# 			# gens-a_3_20130819_1800_384_00.grb2
# 			}
# 		}

# 	return(" ( ok $ok / not ok $notok ) recovered $recovered ");
# 	}


#
#	Concatenates records into type-member conglomerates (Requested Feb.2018)
#
sub mergeRecords($$$)
	{
	my $dir = shift(@_);
	my $dirOut = shift(@_);
	my $cyc = shift(@_);
	my @lvs = qw(sl pl pt pv);

	if( $cyc !~ m/^\d{10}$/ )      { return(1); }
	if( !(-d $dir) || !(-r $dir) ) 
		{ return(2); }

	if( $dirOut eq "" ) 
		{  $dirOut = $dir;  }
	if( !(-d $dirOut) )  { mkdir($dirOut); }
        if( !(-d $dirOut) )  { $dirOut = $dir; }

	opendir( DIR,$dir );
	my @allFiles = sort grep(/\.grib$/,readdir(DIR));
	closedir( DIR );

#print STDOUT "||DEBUG|||  $dir $#allFiles (@allFiles)\n\n";
	if( $#allFiles <= -1 )   { return(-16); } 

	for( my $mb = 0; $mb < get_n_members($cyc); $mb++ )
		{
		foreach my $lv ( @lvs ) 
			{
			my $m = sprintf( "%03d",$mb );
			my $ofn = "z_tigge_c_kwbc_${cyc}0000_glob_prod_pf_${lv}_${m}.grib";
			if( int( $mb ) == 0 ) 
				{ $ofn = "z_tigge_c_kwbc_${cyc}0000_glob_prod_cf_${lv}_${m}.grib"; }
			my $patt = "_${lv}_\\d{4}_${m}.*\\.grib\$";
			my @set = grep( /$patt/, @allFiles );
			my $OUTFILE = "${dirOut}/${ofn}";

			open( OUTF, ">>", $OUTFILE ) || return(4);
			binmode(OUTF);

			my $sizetest = 0;
			if( -e $OUTFILE )   # outfile is being concatenated onto, so adjust.
				{ $sizetest = (-s $OUTFILE);  }
			foreach my $inf ( sort @set ) 
				{
				$sizetest += (-s "${dir}/$inf");
				open( INF, "<", "${dir}/$inf" ) || next;
				binmode(INF);
				print OUTF while <INF>;
				close(INF);
				}

			close( OUTF );
			if( -z $OUTFILE ) 
				{ unlink( $OUTFILE ); }

			if( (-s $OUTFILE) == $sizetest ) 
				{
				foreach my $inf ( sort @set )
					{ unlink( "${dir}/$inf" ); }
				}
			else { return(8); }		# I/O Error of some sort.

			print STDOUT " $patt ($#set) $sizetest b >> $OUTFILE \n";
			}
		}
	return(0);
	}


#
#       Utility function - obtains dates from gens filenames
#
sub getDTGfromGensFileName($$)
    {
    my $fName = shift(@_);
    my($basename,$path) = fileparse($fName);
    my $result = basename($path);

    if( $result !~ m/^\d{4}[0-1]\d[0-3]\d([0-2]\d)?([0-5]\d)?$/ )
            { return("ERROR",2,"bad return data $result\n"); }
    return($result);
    }


#   END -- Preloaded methods go here.

        # FILE LEVEL "Private" VARIABLES
my $yyyymmdd;		# Holds model initialization Date
my $cycle;			# Holds Model initialization time (cycle)
my $ftpHandle;		# For NET::FTP Object Handler
my $startTime;		# For timers
my $endTime;		#  ''
my $gensType;		# Type of GENS :  A or B 

1;

# Autoload methods go after =cut, and are processed by the autosplit program.

__END__

=head1 NAME

ncdcTigge - Self contained perl module

=head1 SYNOPSIS

  use ncdcTigge;

NCDC-NOMADS Tigge Project - Phase 1
PERL module that contains generalized routines used for/to :
1)  Obtain GENS a & b data from ftpprd
2)  Verify downloaded files, then executes the ncdcTigge C code that 
	processes the tigge data.
3)  Verify the TIGGE output records.
4)  Ships the data to an FTP server at NCAR

=head1 DESCRIPTION

This package contains subroutines which accomplish the following tasks--

1)  Verify downloaded files, then executes the ncdcTigge C code that
        processes the tigge data.
2)  Verify the TIGGE output records.


(Deprecated)
X)  Obtain GENS a & b data from ftpprd
X)  Ships the data to an FTP server at NCAR

        These has been moved to module ncdcTiggeFTP.pm




		--- Routine documentation Section ---

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub	cleanupLocation($$;@)
			@ v.0.84
		Will remove EVERYTHING UNDER a target directory <ARG1> older than
		<ARG2> days.  (test against modification time)
		Use [ARG3+] RegEx strings to -EXCLUDE match from deletion

       ARGS   (-> input, <- output, <= returns)  Required, [optional]
	 1  -> Target Directory (string PATH)
	 2  -> Age Threshold (positive float in days)
	[3+]-> Optional RegEx match EXCLUDE strings
		<- STDOUT : Progress, list of files removed, stats
		<- STDERR : Not used
		<* Message (string) on success or failure

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub runLatest($;$)
			@ v.0.84
		Scan TIGGE_INPUT for GENS cycles which have passed QC.
		Will run ncdcTigge executable on the latest ARG1(#) cycles 
		Runs output QC on the results
		Depending on QC results, will execute the archive & upload processes

	 1	->	Number of cycles to process, in time order form current
	[2]	->	-noQC option.  Will disable input QC checks and just run the cycles
		<-	STDOUT: Progress
		<-	STDERR: Not used
		<*	returned OK, or ### <error> string

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub runCycle($;$)
		Similar to runLatest(),  but exact cycle to be run is specified in ARG1
		SUBROUTINE NOT COMPLETED, as not necessarily needed

	 1	->	YYYYMMDDHH (cycle) to run in TIGGE_INPUT
	[2]	->	-noQC option, turns off input QC and force a run
		<-	STDOUT: Progress
		<-	STDERR: Not used
		<*	...

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub archiveOutput(;@)
		Moves TIGGE_OUTPUT files from the pool of output files into
		TIGGE_OUTPUT/archive/yyyymmddhh directories to avoid congestion.

	[1]	->	REGEX pattern match to target certain files
		<-	STDOUT - Progress and errors
		<*	"Returned OK" else "### Error msg"	

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub mergeCycleMembers($)
		For output files, merges the individual GRIB records into a
		21 files, one for each memeber -- for easier handling

	 1	The cycle (yyyymmddhh) under TIGGE_OUTPUT to merge
		<-	STDOUT - Progress
		<*	"Done", or "### Error msg" on problems

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub packCycle($)
		Analyses status of given cycle and runs tar command from the shell
		to package the cycle into 2 compressed tar files.  One for NCAR, and 
		the other for ncdc archive.
		Uses status files to avoid re-processing

	 1	->	Cycle (yyyymmddhh), must exist under ENV TIGGE_OUTPUT
		<-	STDOUT - progress
		<*	string "Finished", or "### Error message" on failure

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub qcGensFile($)
		Invokes grib_ls utility from GRIB API on a target gens file (arg1)
		Scans output and performs a basic QC check to ensure validity

		->	PATH of file to check
		<*	array "OK", followed by any messages, "### Error message" on error

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    sub qcGensCycle($)
        Scans target output cycle(yyyymmddhh) directory <arg1>
        for all input files required to run a cycle.
        Runs qcGensFile on each file required.

        ->  1   Cycle to check  YYYYMMDDHH, must be present in TIGGE_INPUT
        -> [2]  1=Silence most output
        <-  STDOUT - Progress/Messages
        <*  Unique string "*-*-PASSED-*-*" if success, "### Error" if failure

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	sub qcOutput($)
		Runs tigge_check from GRIB_API to validate output files from ncdcTigge
		application.  Also counts files to make sure they are all there.

		->	Cycle (YYYYMMDDHH) to validate
		<-	No STDOUT/STDERR output
		<*	array : "PASS" or "FAIL", followed by string messages

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        sub mergeRecords($$$)
		Will concatenate all records in a given directory for a given cycle
		into level-type, member conglomerates.

                1->      PATH   individual file input directory
                2->      PATH   concatenated file output directory
                3->      String  Cycle "yyyymmddhh" to target within input directory.
		<-	INT Return code   non-zero means the operation aborted.

 - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



=head2 EXPORT

None by default.

=head1 SEE ALSO

NET::FTP
http://search.cpan.org/~gbarr/libnet-1.22/Net/FTP.pm

Support / feedback:
NOMADS.ncdc@noaa.gov

Related Web sites:
http://nomads.ncdc.noaa.gov/
http://tigge.ucar.edu/home/home.htm

=head1 AUTHOR

Dan.Swank, <lt>dan.swank@noaa.gov<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2007 by Dan Swank

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.8 or,
at your option, any later version of Perl 5 you may have available.


=cut

