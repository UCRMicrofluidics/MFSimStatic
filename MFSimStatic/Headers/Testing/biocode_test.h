/*------------------------------------------------------------------------------*
 *                       (c)2014, All Rights Reserved.     						*
 *       ___           ___           ___     									*
 *      /__/\         /  /\         /  /\    									*
 *      \  \:\       /  /:/        /  /::\   									*
 *       \  \:\     /  /:/        /  /:/\:\  									*
 *   ___  \  \:\   /  /:/  ___   /  /:/~/:/        								*
 *  /__/\  \__\:\ /__/:/  /  /\ /__/:/ /:/___     UCR DMFB Synthesis Framework  *
 *  \  \:\ /  /:/ \  \:\ /  /:/ \  \:\/:::::/     www.microfluidics.cs.ucr.edu	*
 *   \  \:\  /:/   \  \:\  /:/   \  \::/~~~~ 									*
 *    \  \:\/:/     \  \:\/:/     \  \:\     									*
 *     \  \::/       \  \::/       \  \:\    									*
 *      \__\/         \__\/         \__\/    									*
 *-----------------------------------------------------------------------------*/
/*--------------------------------Class Details---------------------------------*
 * Name: Biocode Test (BioCode Test)											*
 *																				*
 * Details: Creates DAGs for several assays using the BioCoder language.		*
 *-----------------------------------------------------------------------------*/
#ifndef _BIOCODE_TEST_H
#define _BIOCODE_TEST_H

using namespace std;
#include "../../Headers/BioCoder/BioCoder.h"
#include "../../Headers/Models/assay_node.h"
#include "../../Headers/Resources/enums.h"
#include "../../Headers/Models/dag.h"
#include <stdlib.h>
#include <stdio.h>
#include<list>
#include <map>

class BiocodeTest
{
    public:
        // Constructors
		BiocodeTest();
        virtual ~BiocodeTest ();

        // Benchmarks from http://people.ee.duke.edu/~fs/Benchmark.pdf
        static DAG *Create_B1_PCRMix(double mult);
        static DAG *Create_B2_InVitroDiag(int numSamples, int numReagents);
        static DAG *Create_B3_Protein(double mult);
        static DAG *Create_B3_Protein_Levels(int splitLevels, int eq1_inc2_dec3, double mult);

        // Other homemade benchmarks and tests
        static DAG *Create_Operation_Demonstration_Bench();
        static DAG *Create_2LevelProtein_With_SplitVolumeDetects(); // Used for JETC journal
        static DAG *Create_2LevelProtein_Lev1FailureSubGraph(); // Used for JETC journal
        static DAG *Create_2LevelProtein_Lev2SingleFailureSubGraph(); // Used for JETC journal
        static DAG *Create_2LevelProtein_Lev2DoubleFailureSubGraph(); // Used for JETC journal
};
#endif
