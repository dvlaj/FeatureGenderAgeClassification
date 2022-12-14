///////////////////////////////////////////////////////////////////////
// Author: Damjan Vlaj
// Copyright (c) 2022 DSP LAB, University of Maribor, Maribor, Slovenia
///////////////////////////////////////////////////////////////////////

////////////////////
// File Inclusions
////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hmm.h"
#include "FeatureGenderAgeClassification.h"

char gmmFileName[999],
        pitchWeightsFileName[999],
        featureFileName[999],
        mlfFileName[999],
        decisionKind[30] = "prob";

FILE *gmmFile = NULL,
        *pitchWeightsFile = NULL,
        *featureFile = NULL,
        *mlfFile = NULL;

int HangoverSpecified = FALSE,
        HangbeforeSpecified = FALSE,
        SpeechFrameSpecified = FALSE,
        PitchPositionSpecified = FALSE,
        NormaliseWeightsSpecified = FALSE,
        PitchThresholdSpecified = TRUE,
        DecisionKindSpecified = FALSE,
        DifferenceBetweenProbabilitiesSpecified = TRUE,
        hangOver = 0,
        hangBefore = 0,
        differenceBetweenProbabilities = 5,
        pitchPosition = 0,
        decisionPriority = FALSE;

float pitchThreshold = 4.0;

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: main
//
// PURPOSE:       main function
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {

    if (ParseCommLine(argc - 1, argv + 1)) {

        GenderDecision();

        if (gmmFile)
            fclose(gmmFile);
        if (pitchWeightsFile)
            fclose(pitchWeightsFile);
        if (featureFile)
            fclose(featureFile);
        if (mlfFile)
            fclose(mlfFile);
    } else {
        ReportUsage();
        return (1);
    }

    return (0);


}


//////////////////////////////////////////////////////////////////////
//                          FUNCTIONS                                     
//////////////////////////////////////////////////////////////////////                                   

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ParseCommLine
//
// PURPOSE:       Parse command line
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

static BOOLEAN ParseCommLine(int argc, char *argv[]) {

    int mark = 0;

    while (argc) {

        if (strcmp(argv[mark], "-norm") == 0) {
            NormaliseWeightsSpecified = TRUE;
            --argc;
            ++mark;
        } else if (strcmp(argv[mark], "-dk") == 0) {
            DecisionKindSpecified = TRUE;
            --argc;
            ++mark;
            strcpy(decisionKind, argv[mark]);
            --argc;
            ++mark;
        } else if (strcmp(argv[mark], "-diff") == 0) {
            DifferenceBetweenProbabilitiesSpecified = TRUE;
            --argc;
            ++mark;
            differenceBetweenProbabilities = atoi(argv[mark]);
            --argc;
            ++mark;
        } else if (strcmp(argv[mark], "-pp") == 0) {
            PitchPositionSpecified = TRUE;
            --argc;
            ++mark;
            pitchPosition = atoi(argv[mark]);
            --argc;
            ++mark;
        } else if (strcmp(argv[mark], "-pTh") == 0) {
            PitchThresholdSpecified = TRUE;
            --argc;
            ++mark;
            pitchThreshold = atof(argv[mark]);
            --argc;
            ++mark;
        } else if (argv[mark][0] == '-') {
            fprintf(stderr, "WARNING:  Un-recognized flag '%s' !\r\n", argv[mark]);
            --argc;
            ++mark;
        } else {
            if (mlfFile) // Fifth argument string ERROR!
            {
                fprintf(stderr, "ERROR:   Too many input arguments!\r\n");
                return FALSE;
            } else if (featureFile) // Forth argument string (output file - VAD file)
            {
                strcpy(mlfFileName, argv[mark]);
                mlfFile = fopen(mlfFileName, "wt");
                if (mlfFile == NULL) {
                    fprintf(stderr, "ERROR:   Could not open file '%s' !\r\n", mlfFileName);
                    return FALSE;
                }
            } else if (gmmFile) // Third argument string (input file - feature file)
            {
                strcpy(featureFileName, argv[mark]);
                featureFile = fopen(featureFileName, "rb");
                if (featureFile == NULL) {
                    fprintf(stderr, "ERROR:   Could not open file '%s' !\r\n", featureFileName);
                    return FALSE;
                }
            } else if (pitchWeightsFile) // Second argument string (input file - feature file)
            {
                strcpy(gmmFileName, argv[mark]);
                gmmFile = fopen(gmmFileName, "rt");
                if (gmmFile == NULL) {
                    fprintf(stderr, "ERROR:   Could not open file '%s' !\r\n", gmmFileName);
                    return FALSE;
                }
            } else // First argument string (input file - GMM file)
            {
                strcpy(pitchWeightsFileName, argv[mark]);
                pitchWeightsFile = fopen(pitchWeightsFileName, "rt");
                if (pitchWeightsFile == NULL) {
                    fprintf(stderr, "ERROR:   Could not open file '%s' !\r\n", pitchWeightsFileName);
                    return FALSE;
                }
            }
            --argc;
            ++mark;
        }

    }

    if (strcmp(decisionKind, "prob") &&
            strcmp(decisionKind, "pitchWeight") &&
            strcmp(decisionKind, "pitchCount") &&
            strcmp(decisionKind, "probPitchWeight") &&
            strcmp(decisionKind, "probPitchCountPitchWeight") &&
            strcmp(decisionKind, "probPitchCount")) {
        fprintf(stderr, "ERROR:   Invalid decision kind format '%s'!\r\n", decisionKind);
        return FALSE;
    }

    if (!gmmFile || !featureFile || !mlfFile || !pitchWeightsFile) /* Input and output files must be given */ {
        fprintf(stderr, "ERROR:   Input and output files and configurations flags must be given!\r\n");
        return FALSE;
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ReportUsage
//
// PURPOSE:       Usage report
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ReportUsage() {

    printf("\n");
    printf("\n");
    printf("   Usage: FeatureGenderAgeClassifcation [options] pitchWeightsFile gmmFile featureFile mlfFile\n");
    printf("\n");
    printf("   Options:\n");
    printf("      -dk   format        Decision kind format:                                                   (%s)\r\n", decisionKind);
    printf("                          prob                        - probability                    \r\n");
    printf("                          pitchWeight                 - pitch weight                   \r\n");
    printf("                          pitchCount                  - pitch count                     \r\n");
    printf("                          probPitchWeight             - probability with pitch weight   \r\n");
    printf("                          probPitchCount              - probability with pitch count    \r\n");
    printf("                          probPitchCountPitchWeight   - probability with pitch count and weight\r\n");
    printf("      -pp   index         index of the pitch position in feature file                                (%d)\r\n", pitchPosition);
    printf("      -diff value         difference between probabilities counters                                  (%d)\r\n", differenceBetweenProbabilities);
    printf("      -pTh  threshold     threshold of the pitch value                                             (%.1f)\r\n", pitchThreshold);
    printf("      -norm               normalise weights                                                      (%s)\r\n", NormaliseWeightsSpecified ? "TRUE" : "FALSE");
    printf("\n");
    printf("   (C)DSPLAB 2022 - Version 1.0 \n");
    printf("\n");

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: GenderDecision
//
// PURPOSE:       Gender an Age Decision
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void GenderDecision() {

    int i, j, genderDecision, genderPitchCountDecision,
            genderPitchWeightsDecision,
            pitchWeightDecision = 0, pitchCountDecision = 0, probDecision = 0,
            manCounter = 0, womanCounter = 0, childCounter = 0, silCounter = 0,
            manPitchCountCounter = 0, womanPitchCountCounter = 0, childPitchCountCounter = 0,
            silPitchCountCounter = 0, pitchCounter = 0;
    float *featureVector, probRatio = 0.0, pitchWeightRatio = 0.0,
            pitchCountRatio = 0.0;
    char gender[10], genderProb[10], genderPitchCount[10], genderPitchWeight[10];

    HMMStruct hmm;
    HTKFeatFile feat;

    ////////////////////////////////////////////////////
    // Read GMM file with sil and speech models
    ////////////////////////////////////////////////////

    ReadPitchWeightsFile(&hmm, pitchWeightsFile);

    ReadHMMFile(&hmm, gmmFile);
    InitHMMParameters(&hmm);
    ReadHMMParameters(&hmm);

    //////////////////////////
    // Read HTK feature file 
    //////////////////////////
    ReadHTKFeatureFile(&feat, featureFile);

    /////////////////
    // Gender Decision 
    /////////////////
    featureVector = new float[feat.sampSize / sizeof (float)];

    for (i = 0; i < feat.nSamples; i++) {

        for (j = 0; j < (int) (feat.sampSize / sizeof (float)); j++)
            featureVector[j] = feat.featureBuffer[i * feat.sampSize / sizeof (float)+j];

        genderDecision = GenderFrameDecision(&hmm, featureVector);
        genderPitchCountDecision = GenderPitchCountDecision(&hmm, featureVector, pitchPosition, NormaliseWeightsSpecified);

        if (genderPitchCountDecision == 1) {
            manPitchCountCounter++;
            pitchCounter++;
        } else if (genderPitchCountDecision == 2) {
            womanPitchCountCounter++;
            pitchCounter++;
        } else if (genderPitchCountDecision == 3) {
            childPitchCountCounter++;
            pitchCounter++;
        } else if (genderPitchCountDecision == 4) {
            silPitchCountCounter++;
        }

        if (pitchPosition == 0) {

            if (genderDecision == 1) {
                manCounter++;
            } else if (genderDecision == 2) {
                womanCounter++;
            } else if (genderDecision == 3) {
                childCounter++;
            } else {
                silCounter++;
            }

        } else {


            if (genderDecision == 1) {
                manCounter++;
            } else if (genderDecision == 2) {
                womanCounter++;
            } else if (genderDecision == 3) {
                childCounter++;
            } else {
                silCounter++;
            }

        }

    }

    genderPitchWeightsDecision = GenderPitchWeightsDecision(&hmm);

    if (genderPitchWeightsDecision == 1) {
        strcpy(genderPitchWeight, "man");
    } else if (genderPitchWeightsDecision == 2) {
        strcpy(genderPitchWeight, "woman");
    } else if (genderPitchWeightsDecision == 3) {
        strcpy(genderPitchWeight, "child");
    }


    printf("manProbCounter = %d\n", manCounter);
    printf("womanProbCounter = %d\n", womanCounter);
    printf("childProbCounter = %d\n", childCounter);
    printf("silProbCounter = %d\n", silCounter);

    printf("manPitchCount = %d\n", manPitchCountCounter);
    printf("womanPitchCount = %d\n", womanPitchCountCounter);
    printf("childPitchCount = %d\n", childPitchCountCounter);

    printf("manPitchSum = %.3f\n", hmm.sumOfPitchWeightsMan);
    printf("womanPitchSum = %.3f\n", hmm.sumOfPitchWeightsWoman);
    printf("childPitchSum = %.3f\n", hmm.sumOfPitchWeightsChild);

    if (!strcmp(decisionKind, "prob")) {

        if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
            strcpy(gender, "man");
        } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
            strcpy(gender, "woman");
        } else {
            strcpy(gender, "child");
        }

        WriteMLFFile(mlfFile, mlfFileName, gender);

    } else if (!strcmp(decisionKind, "pitchWeight")) {

        if ((hmm.sumOfPitchWeightsMan >= hmm.sumOfPitchWeightsWoman) && (hmm.sumOfPitchWeightsMan >= hmm.sumOfPitchWeightsChild)) {
            strcpy(gender, "man");
        } else if ((hmm.sumOfPitchWeightsWoman >= hmm.sumOfPitchWeightsMan) && (hmm.sumOfPitchWeightsWoman >= hmm.sumOfPitchWeightsChild)) {
            strcpy(gender, "woman");
        } else {
            strcpy(gender, "child");
        }

        WriteMLFFile(mlfFile, mlfFileName, gender);

    } else if (!strcmp(decisionKind, "pitchCount")) {

        if ((manPitchCountCounter >= womanPitchCountCounter) && (manPitchCountCounter >= childPitchCountCounter)) {
            strcpy(gender, "man");
        } else if ((womanPitchCountCounter >= manPitchCountCounter) && (womanPitchCountCounter >= childPitchCountCounter)) {
            strcpy(gender, "woman");
        } else {
            strcpy(gender, "child");
        }

        WriteMLFFile(mlfFile, mlfFileName, gender);

    } else if (!strcmp(decisionKind, "probPitchWeight")) {

        if (pitchCounter < 10) {

            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                strcpy(genderProb, "man");
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                strcpy(genderProb, "woman");
            } else {
                probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                strcpy(genderProb, "child");
            }

            WriteMLFFile(mlfFile, mlfFileName, genderProb);

        } else {

            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                strcpy(genderProb, "man");
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                strcpy(genderProb, "woman");
            } else {
                probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                strcpy(genderProb, "child");
            }

            if ((hmm.sumOfPitchWeightsMan >= hmm.sumOfPitchWeightsWoman) && (hmm.sumOfPitchWeightsMan >= hmm.sumOfPitchWeightsChild)) {
                pitchWeightRatio = hmm.sumOfPitchWeightsMan / (hmm.sumOfPitchWeightsWoman + hmm.sumOfPitchWeightsChild);
                strcpy(genderPitchWeight, "man");
            } else if ((hmm.sumOfPitchWeightsWoman >= hmm.sumOfPitchWeightsMan) && (hmm.sumOfPitchWeightsWoman >= hmm.sumOfPitchWeightsChild)) {
                pitchWeightRatio = hmm.sumOfPitchWeightsWoman / (hmm.sumOfPitchWeightsMan + hmm.sumOfPitchWeightsChild);
                strcpy(genderPitchWeight, "woman");
            } else {
                pitchWeightRatio = hmm.sumOfPitchWeightsChild / (hmm.sumOfPitchWeightsMan + hmm.sumOfPitchWeightsWoman);
                strcpy(genderPitchWeight, "child");
            }

            if (probRatio >= pitchWeightRatio)
                WriteMLFFile(mlfFile, mlfFileName, genderProb);
            else
                WriteMLFFile(mlfFile, mlfFileName, genderPitchWeight);
        }

    } else if (!strcmp(decisionKind, "probPitchCount")) {

        if (pitchCounter < 10) {

            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                strcpy(genderProb, "man");
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                strcpy(genderProb, "woman");
            } else {
                probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                strcpy(genderProb, "child");
            }

            WriteMLFFile(mlfFile, mlfFileName, genderProb);

        } else {
            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                strcpy(genderProb, "man");
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                strcpy(genderProb, "woman");
            } else {
                probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                strcpy(genderProb, "child");
            }

            if ((manPitchCountCounter >= womanPitchCountCounter) && (manPitchCountCounter >= childPitchCountCounter)) {
                pitchCountRatio = (float) manPitchCountCounter / (float) (womanPitchCountCounter + childPitchCountCounter);
                strcpy(genderPitchCount, "man");
            } else if ((womanPitchCountCounter >= manPitchCountCounter) && (womanPitchCountCounter >= childPitchCountCounter)) {
                pitchCountRatio = (float) womanPitchCountCounter / (float) (manPitchCountCounter + childPitchCountCounter);
                strcpy(genderPitchCount, "woman");
            } else {
                pitchCountRatio = (float) childPitchCountCounter / (float) (manPitchCountCounter + womanPitchCountCounter);
                strcpy(genderPitchCount, "child");
            }


            if (probRatio >= pitchCountRatio)
                WriteMLFFile(mlfFile, mlfFileName, genderProb);
            else
                WriteMLFFile(mlfFile, mlfFileName, genderPitchCount);
        }
    } else if (!strcmp(decisionKind, "probPitchCountPitchWeight")) {

        if (pitchCounter < 10) {

            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                strcpy(genderProb, "man");
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                strcpy(genderProb, "woman");
            } else {
                probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                strcpy(genderProb, "child");
            }

            WriteMLFFile(mlfFile, mlfFileName, genderProb);

        } else {

            if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                if (((manCounter - womanCounter) >= differenceBetweenProbabilities) && 
                        ((manCounter - childCounter) >= differenceBetweenProbabilities))
                    decisionPriority = TRUE;
                else
                    decisionPriority = FALSE;
            } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                if (((womanCounter - manCounter) >= differenceBetweenProbabilities) && 
                        ((womanCounter - childCounter) >= differenceBetweenProbabilities))
                    decisionPriority = TRUE;
                else
                    decisionPriority = FALSE;
            } else {
                if (((childCounter - manCounter) >= differenceBetweenProbabilities) && 
                        ((childCounter - womanCounter) >= differenceBetweenProbabilities))
                    decisionPriority = TRUE;
                else
                    decisionPriority = FALSE;
            }

            if (decisionPriority == TRUE) {

                if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                    probRatio = (float) manCounter / (float) (womanCounter + childCounter);
                    strcpy(genderProb, "man");
                } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                    probRatio = (float) womanCounter / (float) (manCounter + childCounter);
                    strcpy(genderProb, "woman");
                } else {
                    probRatio = (float) childCounter / (float) (manCounter + womanCounter);
                    strcpy(genderProb, "child");
                }

                WriteMLFFile(mlfFile, mlfFileName, genderProb);

            } else {

                if ((manCounter >= womanCounter) && (manCounter >= childCounter)) {
                    probDecision = 1;
                    strcpy(genderProb, "man");
                } else if ((womanCounter >= manCounter) && (womanCounter >= childCounter)) {
                    probDecision = 2;
                    strcpy(genderProb, "woman");
                } else {
                    probDecision = 3;
                    strcpy(genderProb, "child");
                }

                if (genderPitchWeightsDecision == 1) {
                    pitchWeightDecision = 1;
                } else if (genderPitchWeightsDecision == 2) {
                    pitchWeightDecision = 2;
                } else if (genderPitchWeightsDecision == 3) {
                    pitchWeightDecision = 3;
                }

                if ((manPitchCountCounter >= womanPitchCountCounter) && (manPitchCountCounter >= childPitchCountCounter)) {
                    pitchCountDecision = 1;
                } else if ((womanPitchCountCounter >= manPitchCountCounter) && (womanPitchCountCounter >= childPitchCountCounter)) {
                    pitchCountDecision = 2;
                } else {
                    pitchCountDecision = 3;
                }

                if ((probDecision == 1) && (pitchWeightDecision == 1) && (pitchCountDecision == 1)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 1) && (pitchCountDecision == 2)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 1) && (pitchCountDecision == 3)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 2) && (pitchCountDecision == 1)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 3) && (pitchCountDecision == 1)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 1) && (pitchCountDecision == 1)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 1) && (pitchCountDecision == 1)) {
                    strcpy(gender, "man");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 2) && (pitchCountDecision == 2)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 2) && (pitchCountDecision == 1)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 2) && (pitchCountDecision == 3)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 1) && (pitchCountDecision == 2)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 3) && (pitchCountDecision == 2)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 2) && (pitchCountDecision == 2)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 2) && (pitchCountDecision == 2)) {
                    strcpy(gender, "woman");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 3) && (pitchCountDecision == 3)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 3) && (pitchCountDecision == 1)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 3) && (pitchCountDecision == 2)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 1) && (pitchCountDecision == 3)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 3) && (pitchWeightDecision == 2) && (pitchCountDecision == 3)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 1) && (pitchWeightDecision == 3) && (pitchCountDecision == 3)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else if ((probDecision == 2) && (pitchWeightDecision == 3) && (pitchCountDecision == 3)) {
                    strcpy(gender, "child");
                    WriteMLFFile(mlfFile, mlfFileName, gender);
                } else {
                    WriteMLFFile(mlfFile, mlfFileName, genderProb);
                }
            }
        }
    }

    delete [] featureVector;

    ////////////////////////////////////////////////////
    // Release memory of the HMMStruct 
    ////////////////////////////////////////////////////	
    DelHMMParameters(&hmm);
    DelHMMHTKFeatureFile(&feat);

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ReadHTKFeatureFile
//
// PURPOSE:       Read HTK feature File
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ReadHTKFeatureFile(HTKFeatFile *feat, FILE *fp) {

    unsigned int lHead = 0, lFile = 0;

    fread(&(feat->nSamples), sizeof (long), 1, fp);
    fread(&(feat->sampPeriod), sizeof (long), 1, fp);
    fread(&(feat->sampSize), sizeof (short), 1, fp);
    fread(&(feat->sampKind), sizeof (short), 1, fp);

    lHead = ftell(fp);
    lFile = FileSize(fp);
    fseek(fp, lHead, SEEK_SET);

    feat->featureBuffer = new float[(lFile - lHead) / sizeof (float)];
    fread((void *) feat->featureBuffer, sizeof (float), (lFile - lHead) / sizeof (float), fp);

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: DelHMMHTKFeatureFile
//
// PURPOSE:       Delete HMM HTK feature file
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void DelHMMHTKFeatureFile(HTKFeatFile *feat) {

    ///////////////////
    // Memory release 
    delete [] feat->featureBuffer;

    return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION NAME: ExtractFileName
//
// PURPOSE:       Extract filename
//
// INPUT:
//
// OUTPUT
//
// RETURN VALUE
//
// Copyright (c) 2022 DSP-LAB, UM-FERI, Maribor, Slovenia
//////////////////////////////////////////////////////////////////////////////

void ExtractFileName(char *in, char *out) {

    char temp[199];
    int len1, len2, i;

    strcpy(temp, in); // "/clean1/MRK_336O198A.8k.l16"

    if (strlen(strrchr(temp, '/')) > 0)
        strcpy(temp, strrchr(temp, '/')); // "/MRK_336O198A.8k.l16"

    len1 = strlen(temp);
    if (temp[0] == '/') {
        for (i = 1; i < len1; i++)
            temp[i - 1] = temp[i];
        temp[len1 - 1] = '\0';
    } // "MRK_336O198A.8k.l16"

    len1 = strlen(temp);
    len2 = strlen(strchr(temp, '.'));
    if (len2 > 0)
        temp[len1 - len2] = '\0'; // "MRK_336O198A"

    strcpy(out, temp); // "MRK_336O198A"

    return;

}


