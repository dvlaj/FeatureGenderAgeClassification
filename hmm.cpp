//////////////////////////////////////////////////////////////////////////////
//
// GMM Classification
//
// FILE NAME: hmm.c
// 
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hmm.h"

#define MAX_PATHNAME_LEN 999

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: InitHMMParameters
//
// PURPOSE:       Read HMM file with sil and speech models
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ReadHMMFile(HMMStruct *hmm, FILE *fp) {

    unsigned int hmmFile;

    hmm->vecsize = 0;

    ////////////////////////////////
    // open HMM file and read models
    hmmFile = FileSize(fp);
    hmm->hmmBuffer = new char[hmmFile / sizeof (char)];
    fread((void *) hmm->hmmBuffer, sizeof (char), hmmFile / sizeof (char), fp);

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: InitHMMParameters
//
// PURPOSE:       Read HMM file with sil and speech models
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ReadPitchWeightsFile(HMMStruct *hmm, FILE *fp) {

    int countLine, countNumber, i;
    char line[256], delims[] = "\t";
    char *result;
    float maxValue;

    countLine = 0;
    while (fgets(line, sizeof (line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';

        countNumber = 0;
        result = strtok(line, delims);
        while (result != NULL) {
            countNumber++;
            if (countNumber == 2)
                hmm->pitchManWeight[countLine] = (float) atof(result);
            else if (countNumber == 3)
                hmm->pitchWomanWeight[countLine] = (float) atof(result);
            else if (countNumber == 4)
                hmm->pitchChildWeight[countLine] = (float) atof(result);
            result = strtok(NULL, delims);
        }
        countLine++;
    }

    maxValue = 0.0;
    for (i = 0; i < 59; i++) {
        if (hmm->pitchManWeight[i] > maxValue)
            maxValue = hmm->pitchManWeight[i];
        if (hmm->pitchWomanWeight[i] > maxValue)
            maxValue = hmm->pitchWomanWeight[i];
        if (hmm->pitchChildWeight[i] > maxValue)
            maxValue = hmm->pitchChildWeight[i];
    }

    for (i = 0; i < 59; i++) {
        hmm->pitchManNormWeight[i] = hmm->pitchManWeight[i] / maxValue;
        hmm->pitchWomanNormWeight[i] = hmm->pitchWomanWeight[i] / maxValue;
        hmm->pitchChildNormWeight[i] = hmm->pitchChildWeight[i] / maxValue;
    }

    return;

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: InitHMMParameters
//
// PURPOSE:       Read HMM file with sil and speech models
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void InitHMMParameters(HMMStruct *hmm) {

    char FindString[999], FindModel[999];

    strcpy(FindString, "<VECSIZE>");
    hmm->vecsize = (int) SearchForValue(hmm->hmmBuffer, FindString, 1);

    strcpy(FindModel, "man");
    strcpy(FindString, "<NUMMIXES>");
    hmm->numMixturesMan = (int) SearchForValueInModel(hmm->hmmBuffer, FindModel, FindString, 1);
    hmm->usedNumMixturesMan = 0;

    strcpy(FindModel, "woman");
    strcpy(FindString, "<NUMMIXES>");
    hmm->numMixturesWoman = (int) SearchForValueInModel(hmm->hmmBuffer, FindModel, FindString, 1);
    hmm->usedNumMixturesWoman = 0;

    strcpy(FindModel, "child");
    strcpy(FindString, "<NUMMIXES>");
    hmm->numMixturesChild = (int) SearchForValueInModel(hmm->hmmBuffer, FindModel, FindString, 1);
    hmm->usedNumMixturesChild = 0;

    strcpy(FindModel, "sil");
    strcpy(FindString, "<NUMMIXES>");
    hmm->numMixturesSil = (int) SearchForValueInModel(hmm->hmmBuffer, FindModel, FindString, 1);
    hmm->usedNumMixturesSil = 0;

    // Memory allocation 
    hmm->mixtureNoMan = new float[hmm->numMixturesMan];
    hmm->mixtureNoWoman = new float[hmm->numMixturesWoman];
    hmm->mixtureNoChild = new float[hmm->numMixturesChild];
    hmm->mixtureNoSil = new float[hmm->numMixturesSil];
    hmm->mixtureWeightMan = new float[hmm->numMixturesMan];
    hmm->mixtureWeightWoman = new float[hmm->numMixturesWoman];
    hmm->mixtureWeightChild = new float[hmm->numMixturesChild];
    hmm->mixtureWeightSil = new float[hmm->numMixturesSil];
    hmm->meanMan = new float[hmm->vecsize * hmm->numMixturesMan];
    hmm->meanWoman = new float[hmm->vecsize * hmm->numMixturesWoman];
    hmm->meanChild = new float[hmm->vecsize * hmm->numMixturesChild];
    hmm->meanSil = new float[hmm->vecsize * hmm->numMixturesSil];
    hmm->varianceMan = new float[hmm->vecsize * hmm->numMixturesMan];
    hmm->varianceWoman = new float[hmm->vecsize * hmm->numMixturesWoman];
    hmm->varianceChild = new float[hmm->vecsize * hmm->numMixturesChild];
    hmm->varianceSil = new float[hmm->vecsize * hmm->numMixturesSil];
    hmm->gconstMan = new float[hmm->numMixturesMan];
    hmm->gconstWoman = new float[hmm->numMixturesWoman];
    hmm->gconstChild = new float[hmm->numMixturesChild];
    hmm->gconstSil = new float[hmm->numMixturesSil];

    hmm->sumOfPitchWeightsMan = (float) 0.0;
    hmm->sumOfPitchWeightsWoman = (float) 0.0;
    hmm->sumOfPitchWeightsChild = (float) 0.0;

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: DelHMMParameters
//
// PURPOSE:       Read HMM file with sil and speech models
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void DelHMMParameters(HMMStruct *hmm) {

    ///////////////////
    // Memory release 
    delete [] hmm->hmmBuffer;
    delete [] hmm->mixtureNoMan;
    delete [] hmm->mixtureNoWoman;
    delete [] hmm->mixtureNoChild;
    delete [] hmm->mixtureNoSil;
    delete [] hmm->mixtureWeightMan;
    delete [] hmm->mixtureWeightWoman;
    delete [] hmm->mixtureWeightChild;
    delete [] hmm->mixtureWeightSil;
    delete [] hmm->meanMan;
    delete [] hmm->meanWoman;
    delete [] hmm->meanChild;
    delete [] hmm->meanSil;
    delete [] hmm->varianceMan;
    delete [] hmm->varianceWoman;
    delete [] hmm->varianceChild;
    delete [] hmm->varianceSil;
    delete [] hmm->gconstMan;
    delete [] hmm->gconstWoman;
    delete [] hmm->gconstChild;
    delete [] hmm->gconstSil;

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ReadHMMFile
//
// PURPOSE:       Read HMM file with sil and speech models
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ReadHMMParameters(HMMStruct *hmm) {

    //	int i, j;
    char FindString[999], FindModel[999];

    /////////////////////////////////////////////////////////////////////////////////
    // man model
    /////////////////////////////////////////////////////////////////////////////////
    strcpy(FindModel, "man");
    strcpy(FindString, "<MIXTURE>");
    hmm->usedNumMixturesMan = (int) SearchForMixtureNoInModel(hmm->hmmBuffer, hmm->mixtureNoMan, FindModel, FindString, hmm->numMixturesMan, 1);

    strcpy(FindModel, "man");
    strcpy(FindString, "<MIXTURE>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->mixtureWeightMan, FindModel, FindString, hmm->numMixturesMan, 2);

    strcpy(FindModel, "man");
    strcpy(FindString, "<GCONST>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->gconstMan, FindModel, FindString, hmm->numMixturesMan, 1);

    strcpy(FindModel, "man");
    strcpy(FindString, "<MEAN>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->meanMan, hmm->vecsize, FindModel, FindString, hmm->numMixturesMan);

    strcpy(FindModel, "man");
    strcpy(FindString, "<VARIANCE>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->varianceMan, hmm->vecsize, FindModel, FindString, hmm->numMixturesMan);

    /////////////////////////////////////////////////////////////////////////////////
    // woman model
    /////////////////////////////////////////////////////////////////////////////////
    strcpy(FindModel, "woman");
    strcpy(FindString, "<MIXTURE>");
    hmm->usedNumMixturesWoman = (int) SearchForMixtureNoInModel(hmm->hmmBuffer, hmm->mixtureNoWoman, FindModel, FindString, hmm->numMixturesWoman, 1);

    strcpy(FindModel, "woman");
    strcpy(FindString, "<MIXTURE>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->mixtureWeightWoman, FindModel, FindString, hmm->numMixturesWoman, 2);

    strcpy(FindModel, "woman");
    strcpy(FindString, "<GCONST>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->gconstWoman, FindModel, FindString, hmm->numMixturesWoman, 1);

    strcpy(FindModel, "woman");
    strcpy(FindString, "<MEAN>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->meanWoman, hmm->vecsize, FindModel, FindString, hmm->numMixturesWoman);

    strcpy(FindModel, "woman");
    strcpy(FindString, "<VARIANCE>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->varianceWoman, hmm->vecsize, FindModel, FindString, hmm->numMixturesWoman);

    
    /////////////////////////////////////////////////////////////////////////////////
    // child model
    /////////////////////////////////////////////////////////////////////////////////
    strcpy(FindModel, "child");
    strcpy(FindString, "<MIXTURE>");
    hmm->usedNumMixturesChild = (int) SearchForMixtureNoInModel(hmm->hmmBuffer, hmm->mixtureNoChild, FindModel, FindString, hmm->numMixturesChild, 1);

    strcpy(FindModel, "child");
    strcpy(FindString, "<MIXTURE>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->mixtureWeightChild, FindModel, FindString, hmm->numMixturesChild, 2);

    strcpy(FindModel, "child");
    strcpy(FindString, "<GCONST>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->gconstChild, FindModel, FindString, hmm->numMixturesChild, 1);

    strcpy(FindModel, "child");
    strcpy(FindString, "<MEAN>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->meanChild, hmm->vecsize, FindModel, FindString, hmm->numMixturesChild);

    strcpy(FindModel, "child");
    strcpy(FindString, "<VARIANCE>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->varianceChild, hmm->vecsize, FindModel, FindString, hmm->numMixturesChild);

    
    /////////////////////////////////////////////////////////////////////////////////
    // sil model
    /////////////////////////////////////////////////////////////////////////////////
    strcpy(FindModel, "sil");
    strcpy(FindString, "<MIXTURE>");
    hmm->usedNumMixturesSil = (int) SearchForMixtureNoInModel(hmm->hmmBuffer, hmm->mixtureNoSil, FindModel, FindString, hmm->numMixturesSil, 1);

    strcpy(FindModel, "sil");
    strcpy(FindString, "<MIXTURE>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->mixtureWeightSil, FindModel, FindString, hmm->numMixturesSil, 2);

    strcpy(FindModel, "sil");
    strcpy(FindString, "<GCONST>");
    SearchForValuesInModel(hmm->hmmBuffer, hmm->gconstSil, FindModel, FindString, hmm->numMixturesSil, 1);

    strcpy(FindModel, "sil");
    strcpy(FindString, "<MEAN>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->meanSil, hmm->vecsize, FindModel, FindString, hmm->numMixturesSil);

    strcpy(FindModel, "sil");
    strcpy(FindString, "<VARIANCE>");
    SearchForLineInModel(hmm->hmmBuffer, hmm->varianceSil, hmm->vecsize, FindModel, FindString, hmm->numMixturesSil);

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: SearchForValue
//
// PURPOSE:       Search for value of the variables
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

float SearchForValue(char *buffer, char *StringToFind, int NoString) {

    int StringDim, lFile;
    float value;
    char variable[80];
    long i, j, k;

    value = -1.0;
    lFile = strlen(buffer);
    StringDim = strlen(StringToFind);

    i = 0;
    variable[0] = '\0';
    while ((strcmp(variable, StringToFind) != 0)&&(i < lFile - StringDim)) {
        for (j = i; j < i + StringDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += StringDim - 1;

    for (k = 0; k < NoString; k++) {

        while (((buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\n'))&&(i < lFile))
            i++;

        j = 0;
        while (((buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\n'))&&(i < lFile))
            variable[j++] = buffer[i++];

        variable[j] = '\0';
        value = (float) atof(variable);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: SearchForValueInModel
//
// PURPOSE:       Search for value of the variables
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

float SearchForValueInModel(char *buffer, char *Model, char *StringToFind, int NoString) {

    int StringDim, ModelNameDim, lFile;
    float value;
    char variable[80];
    long i, j, k;

    value = -1.0;
    lFile = strlen(buffer);
    StringDim = strlen(StringToFind);
    ModelNameDim = strlen(Model);

    i = 0;
    variable[0] = '\0';
    while ((strcmp(variable, Model) != 0)&&(i < lFile - ModelNameDim)) {
        for (j = i; j < i + ModelNameDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += ModelNameDim - 1;

    while ((strcmp(variable, StringToFind) != 0)&&(i < lFile - StringDim)) {
        for (j = i; j < i + StringDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += StringDim - 1;

    for (k = 0; k < NoString; k++) {

        while (((buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\n'))&&(i < lFile))
            i++;

        j = 0;
        while (((buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\n'))&&(i < lFile))
            variable[j++] = buffer[i++];

        variable[j] = '\0';
        value = (float) atof(variable);
    }

    return value;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: SearchForValuesInModel
//
// PURPOSE:       Search for value of the variables
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void SearchForValuesInModel(char *buffer, float *resultBuffer, char *Model, char *StringToFind, int NoOfValues, int NoString) {

    int StringDim, ModelNameDim, lFile;
    char variable[80];
    long i, j, k, l;

    lFile = strlen(buffer);
    StringDim = strlen(StringToFind);
    ModelNameDim = strlen(Model);

    i = 0;
    variable[0] = '\0';
    while ((strcmp(variable, Model) != 0)&&(i < lFile - ModelNameDim)) {
        for (j = i; j < i + ModelNameDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += ModelNameDim - 1;

    for (l = 0; l < NoOfValues; l++) {
        while ((strcmp(variable, StringToFind) != 0)&&(i < lFile - StringDim)) {
            for (j = i; j < i + StringDim; j++) {
                variable[j - i] = buffer[j];
            }
            variable[j - i] = '\0';
            i++;
        }
        i += StringDim - 1;

        for (k = 0; k < NoString; k++) {

            while (((buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\n'))&&(i < lFile))
                i++;

            j = 0;
            while (((buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\n'))&&(i < lFile))
                variable[j++] = buffer[i++];

            variable[j] = '\0';
            resultBuffer[l] = (float) atof(variable);
        }
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: SearchForMixtureNoInModel
//
// PURPOSE:       Search for value of the variables
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE		used number of mixtures
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

int SearchForMixtureNoInModel(char *buffer, float *resultBuffer, char *Model, char *StringToFind, int NoOfValues, int NoString) {

    int StringDim, ModelNameDim, lFile;
    char variable[80];
    long i, j, k, l;

    lFile = strlen(buffer);
    StringDim = strlen(StringToFind);
    ModelNameDim = strlen(Model);

    i = 0;
    variable[0] = '\0';
    while ((strcmp(variable, Model) != 0)&&(i < lFile - ModelNameDim)) {
        for (j = i; j < i + ModelNameDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += ModelNameDim - 1;

    for (l = 0; l < NoOfValues; l++) {
        while ((strcmp(variable, StringToFind) != 0)&&(i < lFile - StringDim)) {
            for (j = i; j < i + StringDim; j++) {
                variable[j - i] = buffer[j];
            }
            variable[j - i] = '\0';
            i++;
        }
        i += StringDim - 1;

        for (k = 0; k < NoString; k++) {

            while (((buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\n'))&&(i < lFile))
                i++;

            j = 0;
            while (((buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\n'))&&(i < lFile))
                variable[j++] = buffer[i++];

            variable[j] = '\0';

            if (((int) atoi(variable) - 1) == l)
                resultBuffer[l] = (float) atof(variable);
            else {
                while (l != ((int) atoi(variable) - 1)) {
                    resultBuffer[l] = 0.0;
                    l++;
                }
                resultBuffer[l] = (float) atof(variable);
            }
        }
    }

    i = 0;
    for (j = 0; j < NoOfValues; j++)
        if (resultBuffer[j] != 0.0)
            i++;

    return i;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: SearchForLineInModel
//
// PURPOSE:       Search for value of the variables
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void SearchForLineInModel(char *buffer, float *resultBuffer, int vecSize, char *Model, char *StringToFind, int NoOfLines) {

    int StringDim, ModelNameDim, lFile;
    char variable[80];
    long i, j, k, l;

    lFile = strlen(buffer);
    StringDim = strlen(StringToFind);
    ModelNameDim = strlen(Model);

    i = 0;
    variable[0] = '\0';
    while ((strcmp(variable, Model) != 0)&&(i < lFile - ModelNameDim)) {
        for (j = i; j < i + ModelNameDim; j++) {
            variable[j - i] = buffer[j];
        }
        variable[j - i] = '\0';
        i++;
    }
    i += ModelNameDim - 1;

    for (l = 0; l < NoOfLines; l++) {
        while ((strcmp(variable, StringToFind) != 0)&&(i < lFile - StringDim)) {
            for (j = i; j < i + StringDim; j++) {
                variable[j - i] = buffer[j];
            }
            variable[j - i] = '\0';
            i++;
        }
        i += StringDim - 1;

        while ((buffer[i] != '\n')&&(i < lFile))
            i++;

        for (k = 0; k < vecSize; k++) {

            while (((buffer[i] == ' ') || (buffer[i] == '\t') || (buffer[i] == '\n'))&&(i < lFile))
                i++;

            j = 0;
            while (((buffer[i] != ' ') && (buffer[i] != '\t') && (buffer[i] != '\n'))&&(i < lFile))
                variable[j++] = buffer[i++];

            variable[j] = '\0';
            resultBuffer[vecSize * l + k] = (float) atof(variable);

        }
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: FileSize
//
// PURPOSE:       Calculate the size of the configuration file 
//
// INPUT:
//   fp_conf      Pointer to the configuration file
//
// OUTPUT
//   none            
//
// RETURN VALUE
//   lFile        Size of the configuration file
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

unsigned int FileSize(FILE *fp) {

    unsigned int lFile = 0;

    fseek(fp, 0L, SEEK_END);
    lFile = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    return lFile;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: GenderPitchCountDecision
//
// PURPOSE:       
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

int GenderPitchCountDecision(HMMStruct *hmm, float *FeatVec, int pitchPosition, int NormaliseWeightsSpecified) {

    int gender;
    float pitch = 0.0;

    pitch = 10.0 * FeatVec[pitchPosition];

    if ((pitch >= 80.0) && (pitch < 90.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[1];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[1];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[1];
    } else if ((pitch >= 90.0) && (pitch < 100.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[2];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[2];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[2];
    } else if ((pitch >= 100.0) && (pitch < 110.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[3];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[3];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[3];
    } else if ((pitch >= 110.0) && (pitch < 120.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[4];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[4];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[4];
    } else if ((pitch >= 120.0) && (pitch < 130.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[5];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[5];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[5];
    } else if ((pitch >= 130.0) && (pitch < 140.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[6];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[6];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[6];
    } else if ((pitch >= 140.0) && (pitch < 150.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[7];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[7];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[7];
    } else if ((pitch >= 150.0) && (pitch < 160.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[8];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[8];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[8];
    } else if ((pitch >= 160.0) && (pitch < 170.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[9];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[9];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[9];
    } else if ((pitch >= 170.0) && (pitch < 180.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[10];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[10];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[10];
    } else if ((pitch >= 180.0) && (pitch < 190.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[11];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[11];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[11];
    } else if ((pitch >= 190.0) && (pitch < 200.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[12];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[12];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[12];
    } else if ((pitch >= 200.0) && (pitch < 110.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[13];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[13];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[13];
    } else if ((pitch >= 210.0) && (pitch < 220.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[14];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[14];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[14];
    } else if ((pitch >= 220.0) && (pitch < 230.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[15];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[15];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[15];
    } else if ((pitch >= 230.0) && (pitch < 240.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[16];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[16];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[16];
    } else if ((pitch >= 240.0) && (pitch < 250.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[17];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[17];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[17];
    } else if ((pitch >= 250.0) && (pitch < 260.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[18];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[18];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[18];
    } else if ((pitch >= 260.0) && (pitch < 270.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[19];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[19];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[19];
    } else if ((pitch >= 270.0) && (pitch < 280.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[20];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[20];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[20];
    } else if ((pitch >= 280.0) && (pitch < 290.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[21];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[21];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[21];
    } else if ((pitch >= 290.0) && (pitch < 300.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[22];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[22];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[22];
    } else if ((pitch >= 300.0) && (pitch < 310.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[23];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[23];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[23];
    } else if ((pitch >= 310.0) && (pitch < 320.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[24];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[24];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[24];
    } else if ((pitch >= 320.0) && (pitch < 330.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[25];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[25];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[25];
    } else if ((pitch >= 330.0) && (pitch < 340.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[26];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[26];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[26];
    } else if ((pitch >= 340.0) && (pitch < 350.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[27];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[27];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[27];
    } else if ((pitch >= 350.0) && (pitch < 360.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[28];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[28];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[28];
    } else if ((pitch >= 360.0) && (pitch < 370.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[29];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[29];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[29];
    } else if ((pitch >= 370.0) && (pitch < 380.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[30];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[30];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[30];
    } else if ((pitch >= 380.0) && (pitch < 390.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[31];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[31];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[31];
    } else if ((pitch >= 390.0) && (pitch < 400.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[32];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[32];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[32];
    } else if ((pitch >= 400.0) && (pitch < 410.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[33];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[33];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[33];
    } else if ((pitch >= 410.0) && (pitch < 4120.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[34];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[34];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[34];
    } else if ((pitch >= 420.0) && (pitch < 430.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[35];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[35];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[35];
    } else if ((pitch >= 430.0) && (pitch < 440.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[36];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[36];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[36];
    } else if ((pitch >= 440.0) && (pitch < 450.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[37];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[37];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[37];
    } else if ((pitch >= 450.0) && (pitch < 460.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[38];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[38];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[38];
    } else if ((pitch >= 460.0) && (pitch < 470.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[39];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[39];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[39];
    } else if ((pitch >= 470.0) && (pitch < 480.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[40];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[40];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[40];
    } else if ((pitch >= 480.0) && (pitch < 490.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[41];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[41];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[41];
    } else if ((pitch >= 490.0) && (pitch < 500.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[42];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[42];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[42];
    } else if ((pitch >= 500.0) && (pitch < 510.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[43];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[43];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[43];
    } else if ((pitch >= 510.0) && (pitch < 520.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[44];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[44];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[44];
    } else if ((pitch >= 520.0) && (pitch < 530.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[45];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[45];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[45];
    } else if ((pitch >= 530.0) && (pitch < 540.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[46];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[46];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[46];
    } else if ((pitch >= 540.0) && (pitch < 550.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[47];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[47];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[47];
    } else if ((pitch >= 550.0) && (pitch < 560.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[48];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[48];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[48];
    } else if ((pitch >= 560.0) && (pitch < 570.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[49];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[49];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[49];
    } else if ((pitch >= 570.0) && (pitch < 580.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[50];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[50];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[50];
    } else if ((pitch >= 580.0) && (pitch < 590.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[51];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[51];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[51];
    } else if ((pitch >= 590.0) && (pitch < 600.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[52];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[52];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[52];
    } else if ((pitch >= 600.0) && (pitch < 610.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[53];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[53];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[53];
    } else if ((pitch >= 610.0) && (pitch < 620.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[54];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[54];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[54];
    } else if ((pitch >= 620.0) && (pitch < 630.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[55];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[55];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[55];
    } else if ((pitch >= 630.0) && (pitch < 640.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[56];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[56];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[56];
    } else if ((pitch >= 640.0) && (pitch < 650.0)) {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[57];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[57];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[57];
    } else {
        hmm->currentPitchManWeight = hmm->pitchManNormWeight[58];
        hmm->currentPitchWomanWeight = hmm->pitchWomanNormWeight[58];
        hmm->currentPitchChildWeight = hmm->pitchChildNormWeight[58];
    }

    hmm->sumOfPitchWeightsMan += hmm->currentPitchManWeight;
    hmm->sumOfPitchWeightsWoman += hmm->currentPitchWomanWeight;
    hmm->sumOfPitchWeightsChild += hmm->currentPitchChildWeight;

    if (pitch <= 40.0) {
        gender = 4; // sil
    } else {
        if ((hmm->currentPitchManWeight >= hmm->currentPitchWomanWeight) && (hmm->currentPitchManWeight >= hmm->currentPitchChildWeight))
            gender = 1; // man
        else if ((hmm->currentPitchWomanWeight >= hmm->currentPitchManWeight) && (hmm->currentPitchWomanWeight >= hmm->currentPitchChildWeight))
            gender = 2; // woman
        else
            gender = 3; // child
    }

    return gender;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: GenderPitchWeightsDecision
//
// PURPOSE:       
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

int GenderPitchWeightsDecision(HMMStruct *hmm) {

    int gender;

    if ((hmm->sumOfPitchWeightsMan >= hmm->sumOfPitchWeightsWoman) && (hmm->sumOfPitchWeightsMan >= hmm->sumOfPitchWeightsChild))
        gender = 1; // man
    else if ((hmm->sumOfPitchWeightsWoman >= hmm->sumOfPitchWeightsMan) && (hmm->sumOfPitchWeightsWoman >= hmm->sumOfPitchWeightsChild))
        gender = 2; // woman
    else
        gender = 3; // child

    return gender;

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: GenderFrameDecision
//
// PURPOSE:       
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

int GenderFrameDecision(HMMStruct *hmm, float *FeatVec) {

    int i, j, gender;
    double tmpProb;

    //Probability calculation for man

    hmm->probMan = (float) 0.0;

    for (j = 0; j < hmm->usedNumMixturesMan; j++) {
        tmpProb = (double) 0.0;

        for (i = 0; i < (int) hmm->vecsize; i++)
            tmpProb += ((double) FeatVec[i]-(double) hmm->meanMan[j * hmm->vecsize + i])*
            ((double) FeatVec[i]-(double) hmm->meanMan[j * hmm->vecsize + i]) /
            (double) hmm->varianceMan[j * hmm->vecsize + i];

        tmpProb = (double) exp((double(-tmpProb)) / 2.0);
        tmpProb = (double) tmpProb / (double) sqrt((double) exp((double) hmm->gconstMan[j]));
        tmpProb = (double) tmpProb * (double) hmm->mixtureWeightMan[j];

        hmm->probMan += (float) tmpProb;
    }

    //Probability calculation for woman

    hmm->probWoman = (float) 0.0;

    for (j = 0; j < hmm->usedNumMixturesWoman; j++) {
        tmpProb = (double) 0.0;

        for (i = 0; i < (int) hmm->vecsize; i++)
            tmpProb += ((double) FeatVec[i]-(double) hmm->meanWoman[j * hmm->vecsize + i])*
            ((double) FeatVec[i]-(double) hmm->meanWoman[j * hmm->vecsize + i]) /
            (double) hmm->varianceWoman[j * hmm->vecsize + i];

        tmpProb = (double) exp((double(-tmpProb)) / 2.0);
        tmpProb = (double) tmpProb / (float) sqrt((double) exp((double) hmm->gconstWoman[j]));
        tmpProb = (double) tmpProb * (double) hmm->mixtureWeightWoman[j];

        hmm->probWoman += (float) tmpProb;
    }

    //Probability calculation for child

    hmm->probChild = (float) 0.0;

    for (j = 0; j < hmm->usedNumMixturesChild; j++) {
        tmpProb = (double) 0.0;

        for (i = 0; i < (int) hmm->vecsize; i++)
            tmpProb += ((double) FeatVec[i]-(double) hmm->meanChild[j * hmm->vecsize + i])*
            ((double) FeatVec[i]-(double) hmm->meanChild[j * hmm->vecsize + i]) /
            (double) hmm->varianceChild[j * hmm->vecsize + i];

        tmpProb = (double) exp((double(-tmpProb)) / 2.0);
        tmpProb = (double) tmpProb / (float) sqrt((double) exp((double) hmm->gconstChild[j]));
        tmpProb = (double) tmpProb * (double) hmm->mixtureWeightChild[j];

        hmm->probChild += (float) tmpProb;
    }

    //Probability calculation for sil

    hmm->probSil = (float) 0.0;

    for (j = 0; j < hmm->usedNumMixturesSil; j++) {
        tmpProb = (double) 0.0;

        for (i = 0; i < (int) hmm->vecsize; i++)
            tmpProb += ((double) FeatVec[i]-(double) hmm->meanSil[j * hmm->vecsize + i])*
            ((double) FeatVec[i]-(double) hmm->meanSil[j * hmm->vecsize + i]) /
            (double) hmm->varianceSil[j * hmm->vecsize + i];

        tmpProb = (double) exp((double(-tmpProb)) / 2.0);
        tmpProb = (double) tmpProb / (float) sqrt((double) exp((double) hmm->gconstSil[j]));
        tmpProb = (double) tmpProb * (double) hmm->mixtureWeightSil[j];

        hmm->probSil += (float) tmpProb;
    }
    
    if ((hmm->probMan >= hmm->probWoman) && (hmm->probMan >= hmm->probChild) && (hmm->probMan >= hmm->probSil))
        gender = 1; // man
    else if ((hmm->probWoman >= hmm->probMan) && (hmm->probWoman >= hmm->probChild) && (hmm->probWoman >= hmm->probSil))
        gender = 2; // woman
    else if ((hmm->probChild >= hmm->probMan) && (hmm->probChild >= hmm->probWoman) && (hmm->probChild >= hmm->probSil))
        gender = 3; // child
    else
        gender = 4; // sil

    return gender;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: WriteMLFFile
//
// PURPOSE:       
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void WriteMLFFile(FILE *fp, char *filename, char *gender) {

    char path[999],
            base[999],
            ext[10];

    GetFileParts(filename, path, base, ext);

    fprintf(fp, "\"*/%s.rec\"\n", base);
    fprintf(fp, "%s\n", gender);
    fprintf(fp, ".\n");

    printf("Final decision: %s\n", gender);
    
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: GetFileParts
//
// PURPOSE:       
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void GetFileParts(char *path, char *path_, char *base_, char *ext_) {
    char *base;
    char *ext;
    char nameKeep[MAX_PATHNAME_LEN];
    char pathKeep[MAX_PATHNAME_LEN];
    char pathKeep2[MAX_PATHNAME_LEN]; //preserve original input string
    char File_Ext[40];
    char baseK[40];
    int lenFullPath, lenExt_, lenBase_;
    char sDelim[5];
    int iDelim = 0;
    int rel = 0, i;

    if (path) { //determine type of path string (C:\\, \\, /, ./, .\\)
        if ((strlen(path) > 1) &&

                (
                ((path[1] == ':') &&
                (path[2] == '\\')) ||

                (path[0] == '\\') ||

                (path[0] == '/') ||

                ((path[0] == '.') &&
                (path[1] == '/')) ||

                ((path[0] == '.') &&
                (path[1] == '.') &&
                (path[2] == '/')) ||

                ((path[0] == '.') &&
                (path[1] == '\\'))
                )
                ) {
            //sDelim = (char*)calloc(5, sizeof (char));
            /*  //   */if (path[0] == '\\') iDelim = '\\', strcpy(sDelim, "\\");
            /*  c:\\ */if (path[1] == ':') iDelim = '\\', strcpy(sDelim, "\\"); // also satisfies path[2] == '\\'
            /*  /    */if (path[0] == '/') iDelim = '/', strcpy(sDelim, "/");
            /* ./    */if ((path[0] == '.')&&(path[1] == '/')) iDelim = '/', strcpy(sDelim, "/");
            /* ../   */if ((path[0] == '.')&&(path[1] == '.')&&(path[2] == '/')) iDelim = '/', strcpy(sDelim, "/");
            /* .\\   */if ((path[0] == '.')&&(path[1] == '\\')) iDelim = '\\', strcpy(sDelim, "\\");
            /*  \\\\ */if ((path[0] == '\\')&&(path[1] == '\\')) iDelim = '\\', strcpy(sDelim, "\\");
            if (path[0] == '.') {
                rel = 1;
                path[0] = '*';
            }

            if (!strstr(path, ".")) // if no filename, set path to have trailing delim,
            { //set others to "" and return
                lenFullPath = strlen(path);
                if (path[lenFullPath - 1] != iDelim) {
                    strcat(path, sDelim);
                    path_[0] = 0;
                    base_[0] = 0;
                    ext_[0] = 0;
                }
            } else {
                nameKeep[0] = 0; //works with C:\\dir1\file.txt
                pathKeep[0] = 0;
                pathKeep2[0] = 0; //preserves *path
                File_Ext[0] = 0;
                baseK[0] = 0;

                //Get lenth of full path
                lenFullPath = strlen(path);

                strcpy(nameKeep, path);
                strcpy(pathKeep, path);
                strcpy(pathKeep2, path);
                strcpy(path_, path); //capture path

                //Get length of extension:
                for (i = lenFullPath - 1; i >= 0; i--) {
                    if (pathKeep[i] == '.') break;
                }
                lenExt_ = (lenFullPath - i) - 1;

                base = strtok(path, sDelim);
                while (base) {
                    strcpy(File_Ext, base);
                    base = strtok(NULL, sDelim);
                }


                strcpy(baseK, File_Ext);
                lenBase_ = strlen(baseK) - lenExt_;
                baseK[lenBase_ - 1] = 0;
                strcpy(base_, baseK);

                path_[lenFullPath - lenExt_ - lenBase_ - 1] = 0;

                ext = strtok(File_Ext, ".");
                ext = strtok(NULL, ".");
                if (ext) strcpy(ext_, ext);
                else strcpy(ext_, "");
            }
            memset(path, 0, lenFullPath);
            strcpy(path, pathKeep2);
            if (rel)path_[0] = '.'; //replace first "." for relative path
            //free(sDelim);
        }
    }
}



