///////////////////////////////////////////////////////////////////////
// Author: Damjan Vlaj
// Copyright (c) 2022 DSP LAB, University of Maribor, Maribor, Slovenia
///////////////////////////////////////////////////////////////////////

#define	BOOLEAN int
#define	FALSE 0
#define	TRUE (!FALSE)

//Structure HTK feature file
typedef struct 
{
    long nSamples;              /* Structure for HTK header */
    long sampPeriod;
    short sampSize;
    short sampKind;
	float *featureBuffer;
} HTKFeatFile;

//////////////////////////////////////////////////////////////////////
//                          FUNCTIONS                                     
//////////////////////////////////////////////////////////////////////                                   
static BOOLEAN ParseCommLine (int argc, char *argv[]);
void ReportUsage();
void GenderDecision();
void ReadHTKFeatureFile(HTKFeatFile *feat, FILE *fp);
void DelHMMHTKFeatureFile(HTKFeatFile *feat);
void ExtractFileName (char *in, char *out);
