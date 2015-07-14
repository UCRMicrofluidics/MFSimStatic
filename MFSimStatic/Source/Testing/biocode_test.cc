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
/*---------------------------Implementation Details-----------------------------*
 * Source: biocode_test.cc														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Testing/biocode_test.h"


///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the PCR mixing stage detailed
// in Chakrabarty's benchmarks. DAG created using BIOCODER.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_B1_PCRMix(double mult)
{
	DAG *dag = new DAG();
	dag->setName("B1_PCR_Mix");
	vector<Container> tubes;

	BioCoder * bio= new BioCoder("B1_PCR_Mix");
	Volume dispVol = bio->vol(100, ML);
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3*mult, SECS);
	Fluid beosynucleotide = bio->new_fluid("beosynucleotide", dispVol);
	Fluid amplitag = bio->new_fluid("amplitag", dispVol);
	Fluid trishcl = bio->new_fluid("tris-hcl", dispVol);
	Fluid gelatin = bio->new_fluid("gelatin", dispVol);
	Fluid bovine = bio->new_fluid("bovine", dispVol);
	Fluid primer = bio->new_fluid("primer", dispVol);
	Fluid lambda = bio->new_fluid("lambda", dispVol);
	Fluid kcl = bio->new_fluid("kcl", dispVol);

	for (int i = 0; i < 4; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	bio->first_step();
	bio->measure_fluid(trishcl, dropVol, tubes[0]);
	bio->measure_fluid(kcl, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);

	bio->next_step();
	bio->measure_fluid(bovine, dropVol, tubes[1]);
	bio->measure_fluid(gelatin, dropVol, tubes[1]);
	bio->vortex(tubes[1], time);

	bio->next_step();
	bio->measure_fluid(primer, dropVol, tubes[2]);
	bio->measure_fluid(beosynucleotide, dropVol, tubes[2]);
	bio->vortex(tubes[2], time);

	bio->next_step();
	bio->measure_fluid(amplitag, dropVol, tubes[3]);
	bio->measure_fluid(lambda, dropVol, tubes[3]);
	bio->vortex(tubes[3], time);

	bio->next_step();
	bio->measure_fluid(tubes[1], tubes[0]);
	bio->vortex(tubes[0], time);

	bio->next_step();
	bio->measure_fluid(tubes[3], tubes[2]);
	bio->vortex(tubes[2], time);

	bio->next_step();
	bio->measure_fluid(tubes[2], tubes[0]);
	bio->vortex(tubes[0], time);

	bio->next_step();
	bio->drain(tubes[0], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCode Bench1_PCRMix CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the multiplexed in-vitro diagnostics
// assay detailed in Chakrabarty's benchmarks.  Plasma/Serum are assayed
// for glucose/lactate measurements. DAG created using BIOCODER.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_B2_InVitroDiag(int numSamples, int numReagents)
{
	// S1-S4 = Plasma, Serum, Saliva, Urine
	// R1-R4 = Glucose, Lactate, Pyruvate, Glutamate
	// Mixing Times: S1 = 5, S2 = 3, S3 = 4, S4 = 6
	// Detect Times: R1 = 5, R2 = 4, R3 = 6, R4 = 5

	// Bench Stats:
	// B1 - #S = 2, #R = 2, #Detectors = 2
	// B2 - #S = 2, #R = 3, #Detectors = 3
	// B3 - #S = 3, #R = 3, #Detectors = 3
	// B4 - #S = 3, #R = 4, #Detectors = 4
	// B5 - #S = 4, #R = 4, #Detectors = 4

	{	// Sanity check: We only have mix/detect data for the four samples/reagents
		stringstream msg;
		msg << "ERROR. Number of samples/reagents/detectors must each be between 1 and 7." << ends;
		claim(numSamples > 0 && numReagents > 0 && numSamples <= 7 && numReagents <= 7, &msg);
	}

	BioCoder * bio= new BioCoder("B2_InVitro_Mix");
	double sampleMixTimes[] = {5, 3, 4, 6, 3, 7, 6};
	double reagentDetectTimes[] = {5, 4, 6, 5, 8, 6, 7};
	char* sampleNames[] = {"Plasma", "Serum", "Saliva", "Urine", "S1", "S2", "S3"};
	char* reagentNames[] = {"Glucose", "Lactate", "Pyruvate", "Glutamate", "R1", "R2", "R3"};

	DAG *dag = new DAG();
	dag->setName("B2_InVitro_Mix");

	int tubeNum = 0;
	for (int s = 0; s < numSamples; s++)
	{
		for (int r = 0; r < numReagents; r++)
		{
			Container tube = bio->new_container(STERILE_MICROFUGE_TUBE2ML);
			Fluid S = bio->new_fluid(sampleNames[s] ,bio->vol(100,ML));
			Fluid R = bio->new_fluid(reagentNames[r] ,bio->vol(100,ML));

			if (s == 0 && r == 0)
				bio->first_step();
			else
				bio->next_step();

			bio->measure_fluid(S, bio->vol(10, UL), tube);
			bio->measure_fluid(R, bio->vol(10, UL), tube);
			bio->vortex(tube, bio->time(sampleMixTimes[s], SECS));

			bio->next_step();
			bio->measure_fluorescence(tube, bio->time(sampleMixTimes[s], SECS));

			bio->next_step();
			bio->drain(tube, "output");

			tubeNum++;
		}
	}
	bio->end_protocol();
	bio->Translator(dag);

    cout << "BioCoder Bench_InVitroDiag(Samples=" << numSamples << ", Reagents=" << numReagents << ")  CREATED" << endl;
    return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the protein assay (based on Bradford
// reaction) detailed in Chakrabarty's benchmarks. Specified in Biocoder.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_B3_Protein(double mult)
{
	DAG *dag = new DAG();
	dag->setName("B3_Protein_Mix");

	BioCoder * bio = new BioCoder("B3_Protein_Mix");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(1000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(1000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(1000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3*mult, SECS);

	vector<Container> tubes;
	int numOuts = 8;
	for (int i = 0; i < numOuts; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	// Level 1
	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tubes[0]);
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[4], false);

	// Level 2
	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[2], false);

	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[4]);
	bio->vortex(tubes[4], time);
	bio->measure_fluid(tubes[4], dropVol, tubes[6], false);

	// Level 3
	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[1], false);

	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[2]);
	bio->vortex(tubes[2], time);
	bio->measure_fluid(tubes[2], dropVol, tubes[3], false);

	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[4]);
	bio->vortex(tubes[4], time);
	bio->measure_fluid(tubes[4], dropVol, tubes[5], false);

	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tubes[6]);
	bio->vortex(tubes[6], time);
	bio->measure_fluid(tubes[6], dropVol, tubes[7], false);


	// Level 4-END
	for (int i = 0; i < numOuts; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[i]);
			bio->vortex(tubes[i], time);
		}
		bio->next_step();
		bio->measure_fluid(DsR, dropVol, tubes[i]);
		bio->vortex(tubes[i], time);

		bio->next_step();
		bio->measure_fluorescence(tubes[i], bio->time(30*mult, SECS));
	}

	bio->next_step();
	for (int i = 0; i < numOuts; i ++)
		bio->drain(tubes[i], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_Protein CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the protein assay (based on Bradford
// reaction) detailed in Chakrabarty's benchmarks. However, instead of doing
// the default 3 levels of splits, we can specify how many levels of splits
// we want to do.  Also, we can use eq1_inc2_dec3 to specify if the detects
// are equal (1), increasing (2) or decreasing (3) from left to right.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_B3_Protein_Levels(int splitLevels, int eq1_inc2_dec3, double mult)
{
	DAG *dag = new DAG();
	dag->setName("B3_Protein_Mix_Levels");

	BioCoder * bio = new BioCoder("B3_Protein_Mix");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(10000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(10000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(10000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3*mult, SECS);

	vector<Container> tubes;
	int numOuts = (int)pow((float)2, splitLevels);
	for (int i = 0; i < numOuts; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tubes[0]);
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[numOuts/2], false);

	for (int l = 1; l < splitLevels; l++)
	{
		int inc1 = (int)pow((float)2, splitLevels-l);
		int inc2 = (int)pow((float)2, splitLevels-l-1);

		for (int sNum = 0; sNum < (int)pow((float)2, l); sNum++)
		{
			int base = sNum * inc1;
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[base]);
			bio->vortex(tubes[base], time);
			//bio->measure_fluid(tubes[base], dropVol, tubes[base+inc2], false);
			bio->measure_fluid(tubes[base], 2, 1, tubes[base+inc2], false);
		}
	}

	// Level numLevels+1 to END
	for (int i = 0; i < numOuts; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[i]);
			bio->vortex(tubes[i], time);
		}
		bio->next_step();
		bio->measure_fluid(DsR, dropVol, tubes[i]);
		bio->vortex(tubes[i], time);

		bio->next_step();

		if (eq1_inc2_dec3 == 1)
			bio->measure_fluorescence(tubes[i], bio->time(30*mult, SECS));
		else if (eq1_inc2_dec3 == 2)
			bio->measure_fluorescence(tubes[i], bio->time(30+i, SECS));
		else if (eq1_inc2_dec3 == 3)
			bio->measure_fluorescence(tubes[i], bio->time(30+(numOuts-i-1), SECS));
		else
		{
			cerr << "Specified a value for eq1_inc2_dec3 that was not valid for B3_Protein_Levels()" << endl;
			exit(1);
		}
	}

	bio->next_step();
	for (int i = 0; i < numOuts; i ++)
		bio->drain(tubes[i], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_Protein_Levels CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// Simply shows how the dispense, mix, heat, detect, split, output operations
// are called
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_Operation_Demonstration_Bench()
{
	DAG *dag = new DAG();
	dag->setName("Operation_Demo_Benchmark");

	// BioCoder Assay Protocol
	BioCoder * bio = new BioCoder("Operation_Demo_Benchmark");
	Volume reservoirVol = bio->vol(100,ML);
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(1, SECS);
	Container tube1 = bio->new_container(STERILE_MICROFUGE_TUBE2ML);
	Container tube2 = bio->new_container(STERILE_MICROFUGE_TUBE2ML);
	Fluid reagent = bio->new_fluid("reagent", reservoirVol);
	Fluid sample = bio->new_fluid("sample", reservoirVol);

	bio->first_step("Dispense Fluids");
	bio->measure_fluid(sample, dropVol, tube1);
	bio->measure_fluid(reagent, dropVol, tube1);

	bio->next_step("Mix Fluids");
	bio->vortex(tube1, time);

	bio->next_step("Heat Fluids at 50 C");
	bio->store_for(tube1, 50, time);

	bio->next_step("Detect Fluorescence");
	bio->measure_fluorescence(tube1, time);

	bio->next_step("Split Fluid");
	bio->measure_fluid(tube1, dropVol, tube2, false);

	bio->next_step("Output Fluid");
	bio->drain(tube1, "waste");
	bio->drain(tube2, "output");

	bio->end_protocol();
	bio->Translator(dag);

    cout << "Operation_Demo_Benchmark CREATED" << endl;
    return dag;
}


///////////////////////////////////////////////////////////////////////////
// This benchmark details the DAG for the protein assay (based on Bradford
// reaction) detailed in Chakrabarty's benchmarks. However, instead of doing
// the default 3 levels of splits, it does 2 levels and then has 5s detect
// operations after each split.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_2LevelProtein_With_SplitVolumeDetects()
{
	int splitLevels = 2;

	DAG *dag = new DAG();
	dag->setName("B3_2LevelProtein_With_SplitVolumeDetects");

	BioCoder * bio = new BioCoder("B3_2LevelProtein_With_SplitVolumeDetects");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(10000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(10000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(10000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3, SECS);
	Time detTime = bio->time(5, SECS);

	vector<Container> tubes;
	int numOuts = (int)pow((float)2, splitLevels);
	for (int i = 0; i < numOuts; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tubes[0]);
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[numOuts/2], false);
	bio->measure_fluorescence(tubes[0], detTime);
	//bio->measure_fluorescence(tubes[numOuts/2], detTime); // For timings...really only need one detect

	for (int l = 1; l < splitLevels; l++)
	{
		int inc1 = (int)pow((float)2, splitLevels-l);
		int inc2 = (int)pow((float)2, splitLevels-l-1);

		for (int sNum = 0; sNum < (int)pow((float)2, l); sNum++)
		{
			int base = sNum * inc1;
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[base]);
			bio->vortex(tubes[base], time);
			//bio->measure_fluid(tubes[base], dropVol, tubes[base+inc2], false);
			bio->measure_fluid(tubes[base], 2, 1, tubes[base+inc2], false);
			bio->measure_fluorescence(tubes[base], detTime);
			//bio->measure_fluorescence(tubes[base+inc2], detTime); // For timings...really only need one detect
		}
	}

	// Level numLevels+1 to END
	for (int i = 0; i < numOuts; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[i]);
			bio->vortex(tubes[i], time);
		}
		bio->next_step();
		bio->measure_fluid(DsR, dropVol, tubes[i]);
		bio->vortex(tubes[i], time);

		bio->next_step();
		bio->measure_fluorescence(tubes[i], bio->time(30, SECS));

	}

	bio->next_step();
	for (int i = 0; i < numOuts; i ++)
		bio->drain(tubes[i], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_2LevelProtein_With_SplitVolumeDetects CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// Time-redundant recovery sub-graph for a failure occurring in first level.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_2LevelProtein_Lev1FailureSubGraph()
{
	int splitLevels = 1;

	DAG *dag = new DAG();
	dag->setName("B3_2LevelProtein_Lev1FailureSubGraph");

	BioCoder * bio = new BioCoder("B3_2LevelProtein_Lev1FailureSubGraph");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(10000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(10000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(10000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3, SECS);
	Time detTime = bio->time(5, SECS);

	vector<Container> tubes;
	int numOuts = (int)pow((float)2, splitLevels);
	for (int i = 0; i < numOuts; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tubes[0]);
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[numOuts/2], false);
	bio->measure_fluorescence(tubes[0], detTime);
	bio->measure_fluorescence(tubes[numOuts/2], detTime); // For timings...really only need one detect

	bio->next_step();
	for (int i = 0; i < numOuts; i ++)
		bio->drain(tubes[i], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_2LevelProtein_Lev1FailureSubGraph CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// Time-redundant recovery sub-graph for a single failure occurring in
// second level.
///////////////////////////////////////////////////////////////////////////
DAG *BiocodeTest::Create_2LevelProtein_Lev2SingleFailureSubGraph()
{
	int splitLevels = 2;

	DAG *dag = new DAG();
	dag->setName("B3_2LevelProtein_Lev2SingleFailureSubGraph");

	BioCoder * bio = new BioCoder("B3_2LevelProtein_Lev2SingleFailureSubGraph");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(10000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(10000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(10000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3, SECS);
	Time detTime = bio->time(5, SECS);

	Container tube0 = bio->new_container(STERILE_MICROFUGE_TUBE2ML);
	Container tube1 = bio->new_container(STERILE_MICROFUGE_TUBE2ML);
	Container tube2 = bio->new_container(STERILE_MICROFUGE_TUBE2ML);


	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tube0);
	bio->measure_fluid(DsB, dropVol, tube0);
	bio->vortex(tube0, time);
	bio->measure_fluid(tube0, dropVol, tube2, false);
	bio->measure_fluorescence(tube0, detTime);
	//bio->measure_fluorescence(tubes[numOuts/2], detTime); // For timings...really only need one detect
	bio->drain(tube2, "output");



	bio->next_step();
	bio->measure_fluid(DsB, dropVol, tube0);
	bio->vortex(tube0, time);
	//bio->measure_fluid(tubes[base], dropVol, tubes[base+inc2], false);
	bio->measure_fluid(tube0, 2, 1, tube1, false);
	bio->measure_fluorescence(tube0, detTime);
	bio->drain(tube0, "output");
	bio->drain(tube1, "output");


	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_2LevelProtein_Lev2SingleFailureSubGraph CREATED" << endl;
	return dag;
}

///////////////////////////////////////////////////////////////////////////
// Time-redundant recovery sub-graph for a double failure occurring in
// second level.
///////////////////////////////////////////////////////////////////////////
DAG * BiocodeTest::Create_2LevelProtein_Lev2DoubleFailureSubGraph()
{
	int splitLevels = 2;

	DAG *dag = new DAG();
	dag->setName("B3_2LevelProtein_Lev2DoubleFailureSubGraph");

	BioCoder * bio = new BioCoder("B3_2LevelProtein_Lev2DoubleFailureSubGraph");
	Fluid DsS = bio->new_fluid("DsS", bio->vol(10000,ML));
	Fluid DsB = bio->new_fluid("DsB", bio->vol(10000,ML));
	Fluid DsR = bio->new_fluid("DsR", bio->vol(10000,ML));
	Volume dropVol = bio->vol(10, UL);
	Time time = bio->time(3, SECS);
	Time detTime = bio->time(5, SECS);

	vector<Container> tubes;
	int numOuts = (int)pow((float)2, splitLevels);
	for (int i = 0; i < numOuts; i++)
		tubes.push_back(bio->new_container(STERILE_MICROFUGE_TUBE2ML));

	bio->first_step();
	bio->measure_fluid(DsS, dropVol, tubes[0]);
	bio->measure_fluid(DsB, dropVol, tubes[0]);
	bio->vortex(tubes[0], time);
	bio->measure_fluid(tubes[0], dropVol, tubes[numOuts/2], false);
	bio->measure_fluorescence(tubes[0], detTime);
	bio->measure_fluorescence(tubes[numOuts/2], detTime); // For timings...really only need one detect

	for (int l = 1; l < splitLevels; l++)
	{
		int inc1 = (int)pow((float)2, splitLevels-l);
		int inc2 = (int)pow((float)2, splitLevels-l-1);

		for (int sNum = 0; sNum < (int)pow((float)2, l); sNum++)
		{
			int base = sNum * inc1;
			bio->next_step();
			bio->measure_fluid(DsB, dropVol, tubes[base]);
			bio->vortex(tubes[base], time);
			//bio->measure_fluid(tubes[base], dropVol, tubes[base+inc2], false);
			bio->measure_fluid(tubes[base], 2, 1, tubes[base+inc2], false);
			bio->measure_fluorescence(tubes[base], detTime);
			bio->measure_fluorescence(tubes[base+inc2], detTime); // For timings...really only need one detect
		}
	}

	bio->next_step();
	for (int i = 0; i < numOuts; i ++)
		bio->drain(tubes[i], "output");

	bio->end_protocol();
	bio->Translator(dag);

	cout << "BioCoder B3_2LevelProtein_Lev2DoubleFailureSubGraph CREATED" << endl;
	return dag;
}


