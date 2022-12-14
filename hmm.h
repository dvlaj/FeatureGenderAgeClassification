//////////////////////////////////////////////////////////////////////////////
//
// GMM Classification
//
// FILE NAME: hmm.h
// 
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

//Structure for HMM
typedef struct 
{
    char *hmmBuffer;
    int vecsize;
    int numMixturesMan;
    int numMixturesWoman;
    int numMixturesChild;
    int numMixturesSil;
    int usedNumMixturesMan;
    int usedNumMixturesWoman;
    int usedNumMixturesChild;
    int usedNumMixturesSil;
    float *mixtureNoMan;
    float *mixtureNoWoman;
    float *mixtureNoChild;
    float *mixtureNoSil;
    float *mixtureWeightMan;
    float *mixtureWeightWoman;
    float *mixtureWeightChild;
    float *mixtureWeightSil;
    float *meanMan;           
    float *meanWoman;           
    float *meanChild;           
    float *meanSil;           
    float *varianceMan;
    float *varianceWoman;
    float *varianceChild;
    float *varianceSil;
    float *gconstMan;
    float *gconstWoman;
    float *gconstChild;
    float *gconstSil;
    float pitchManWeight[59];
    float pitchWomanWeight[59];
    float pitchChildWeight[59];
    float pitchManNormWeight[59];
    float pitchWomanNormWeight[59];
    float pitchChildNormWeight[59];
    float currentPitchManWeight;
    float currentPitchWomanWeight;
    float currentPitchChildWeight;
    float sumOfPitchWeightsMan;
    float sumOfPitchWeightsWoman;
    float sumOfPitchWeightsChild;
    float probMan;
    float probWoman;
    float probChild;
    float probSil;
    int ManFrame;
    int WomanFrame;
    int ChildFrame;
    int SilFrame;
    int hangover;
} HMMStruct;

void InitHMMParameters(HMMStruct *hmm);
void ReadPitchWeightsFile(HMMStruct *hmm, FILE *fp);
void ReadHMMFile(HMMStruct *hmm, FILE *fp);
void ReadHMMParameters(HMMStruct *hmm);
void DelHMMParameters(HMMStruct *hmm);
float SearchForValue(char *buffer, char *StringToFind, int NoString);
float SearchForValueInModel(char *buffer, char *Model, char *StringToFind, int NoString);
void SearchForValuesInModel(char *buffer, float *resultBuffer, char *Model, char *StringToFind, int NoOfValues, int NoString);
int SearchForMixtureNoInModel(char *buffer, float *resultBuffer, char *Model, char *StringToFind, int NoOfValues, int NoString);
void SearchForLineInModel(char *buffer, float *resultBuffer, int vecSize, char *Model, char *StringToFind, int NoOfLines);
unsigned int FileSize ( FILE *fp_conf );
int GenderPitchCountDecision(HMMStruct *hmm, float *FeatVec, int pitchPosition, int NormaliseWeightsSpecified);
int GenderPitchWeightsDecision(HMMStruct *hmm);
int GenderFrameDecision (HMMStruct *hmm, float *FeatVec);
void WriteMLFFile(FILE *fp, char *filename, char *gender);
void GetFileParts(char *path, char *path_, char *base_, char *ext_);