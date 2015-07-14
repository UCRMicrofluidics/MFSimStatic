/*------------------------------------------------------------------------------*
 *                   2012 by University of California, Riverside                *
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
/*-------------------------------Class Details----------------------------------*
 * Type: BioCoder Class															*
 * Name: AssayStep (Assay Step)													*
 *																				*
 * Original source code from the following paper:								*
 * Authors: Vaishnavi Ananthanarayanan and William Thies						*
 * Title: Biocoder: A programming language for standardizing and automating		*
 * 			biology protocols													*
 * Publication Details: In J. Biological Engineering, Vol. 4, No. 13, Nov 2010	*
 * 																				*
 * Details: UCR has modified and added to the original biocoder so that it 		*
 * could actually be used for synthesis on a DMFB.								*
 * 																				*
 * This class represents a single step in an assay and can be translated to an	*
 * assay operation inside a DAG.
 *-----------------------------------------------------------------------------*/
#ifndef ASSAYSTEP_H_
#define ASSAYSTEP_H_

#include "BioCoderStructs.h"

struct aDetect;
struct aDispense;
struct aStore;
struct aHeat;
struct aMix;
struct aOutput;
struct aSplit;
struct aTransIn;
struct aTransOut;

struct assayStep
{
	map<string,aDetect> detectList;
	map<string,aDispense> dispList;
	map<string,aSplit> splitList;
	map<string,aStore> storeList;
	map<string,aHeat> heatList;
	map<string,aMix> mixList;
	map<string,aOutput> outputList;
	map<string,aTransIn> transInList;
	map<string,aTransOut> transOutList;

	bool detect;
	bool dispence;
	bool mix;
	bool heat;
	bool split;
	bool store;
	bool output;
	bool transIn;
	bool transOut;
	assayStep();
};
#endif /* ASSAYSTEP_H_ */
