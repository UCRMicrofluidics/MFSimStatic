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
 * Name: AssayProtocol (Assay Protocol)											*
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
 * This class represents an assay written in BioCoder and can be translated		*
 * to a DAG (usable by the DMFB simulator).										*
 *-----------------------------------------------------------------------------*/
#ifndef ASSAYPROTOCOL_H_
#define ASSAYPROTOCOL_H_

#include "../Models/assay_node.h"
#include "../Resources/enums.h"
#include "../Models/dag.h"


#include <map>
#include <list>
#include <iostream>
#include <fstream>
#include "assayStep.h"
#include "BioCoder.h"
#include "BioEnums.h"
#include "BioCoderStructs.h"
#include "TimeVolumeSpeed.h"
#include "MCFlow.h"

//forward declarations
struct Container;
struct Fluid;
struct tNameBank;
struct aDetect;
struct aDispense;
struct aStore;
struct aHeat;
struct aMix;
struct aMixture;
struct aOutput;
struct assayStep;
struct aTransOut;
struct aTransIn;
struct MCFlow;


struct BioCoder;
struct BioConditionalGroup;

struct assayProtocol
{

	int dagDropRestrict;					//defaults at 13 Given by Dan's simulator.
	map<string,Container> conLookUp;	//this contains the status of all containers used, and is updated in real time.
	map<string,int> nodeLookUp;			//this data member holds the Node names and the specific step they are located in.
	map<string, list <string> > links;	//this data member holds the edges between the Nodes with the syntax of <child, parents>
	tNameBank* nodes;					//Name bank to find out the next available name using the name standards of (D1,D2,D3...)

	list<aDispense> inputList;			//Initialization list of inputs
	list<aOutput> outputList;			////Initialization list of outputs

	//record of every mixture that was used in the protocol
	aMixture* completeMixList;
	int completeMixNum;
	//keeps track of every step
	assayStep* steps;
	int stepNum;

	/*
	 *Name: Greatest Common Divisor
	 *Purpose: takes in two numbers and returns the greatest common divisor
	 */
	int gcd(int a, int b);

	/*
	 * Name: Is Int
	 * Purpose: tests whether the float is an int.
	 */
	bool isInt(float f);

	/*
	 * Name: deciRatio2IntRatio
	 * Purpose: takes in a floating point ratio and converts the ratio into
	 * 			an int ratio.
	 */
	void deciRatio2IntRatio(float& f, float& f2);//** WARNING ** CAN NOT USE SAME VARIABLE FOR BOTH SLOTS WILL RESULT IN INVALAID RESULTS

	/*
	 * Name: ckSplit
	 * Purpose: checks to see if the split is possible to obtain
	 * 			if it is true then the divisions will be stored inside of int list(splits)
	 */
	bool ckSplit(int target,int total, list<int>&splits,int dropletRetrict);

	/*
	 * Name: NumPiecesUsed
	 * Purpose: returns the number of pieces used given some target and value of the pieces
	 * 			this function is used for the cascading division to solve the split problem.
	 */
	int numPiecesUsed(int target, int pieces, int value);

	/*
	 * Name Split Calls
	 * Purpose: this is the actual calls on the data structure. this calls both mix
	 * 			 and split instructions as necessary.
	 */
	void splitCalls(Container & src,Container & dest,map<int,pair<int,string> > splitInstructions, map <int , string> mixDest, bool ensureMeasurement);


	assayProtocol(string FileName);
	~assayProtocol();
//>>>>>>>>>>>Shell calls for the different structs and their connections<<<<<<<<<<<<<
	/*Name: Split Management
	 * Purpose: Calls all necessary functions required to figure out to see
	 *  		if it is possible to split a fluid into necessary part for the ratio
	 *  		and links everything needed within this function
	 */
	void splitManagment(Container & src,Container & dest,Volume targetVolume, bool ensureMeasurement);

	/*
	 * Name: assay Mix Management
	 * Purpose: calls both functions necessary to connect to the data structure,
	 * 			but it does checking on the container to see if there actually
	 * 			is something to mix, if not then it will not execute the functions
	 */
	void assayMixManagement(Container & con,aMixture & mixture, string typeofmix);

	/*
	 * Name: Print Assay
	 * Purpose: prints each step of the assay and how it plugs into the Dag
	 * 		  	shows the link name to the side of the struct
	 */
	void printassay(ostream & out);

	/*
	 * Name: Print Links
	 * Purpose: get a visual of the links it outputs each link and its parents.
	 */
	void printLinks(ostream & out);
//	void printMixtures(ostream &out);

	/*
	 * Name: Add To Initialization input List
	 * Purpose: takes a dispense struct and adds it to a list used for the initializationIO function
	 */
	void addToInitInputList(aDispense & dis);

	/*
	 * Name: Add To Initialization output List
	 * Purpose: takes an output struct and adds it to a list used for the initializationIO function
	 */
	void addToInitOutputList(aOutput & out);

	/*
	 * Name: assay Detect
	 * Purpose: 1) creates the Dectect node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayDetect(string typeDetect,Time time,Container & con);

	/*
	 * Name: assay Store
	 * Purpose: 1) creates the Store Node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayStore(Container & container, float temper, Time);

	/*
	 * Name: assay Heat
	 * Purpose: 1) creates the Heat node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayHeat(Container &container, float temp, Time);

	/*
	 * Name: assay Dispense
	 * Purpose: 1) creates the Dispense node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayDispense(Fluid fluid1, Volume volume1);

	/*
	 * Name: assay Mix
	 * Purpose: 1) creates the Mix node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayMix(aMixture mixture,string typeofmix,Container & con);

	/*
	 * Name: assay Output
	 * Purpose: 1) creates the Output Node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayOutput(string sinkName);

	/*
	 * Name: assay Waste Output
	 * Purpose: 1) creates Output Node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 * 	**NOTE**
	 * 	this function is the same as assayOutput except distinguishes between output and wastes!
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assayWasteOutput(string sinkName);

	/*
	 * Name: assay Split
	 * Purpose: 1) creates the Split Node and adds the node to a look up list to have quick access.
	 * 			2) sets flag in step to know that this node is happening
	 * 			3) adds the struct to the step for the data structure
	 *
	 * 	Return: the linkName of the Node that was created.
	 */
	string assaySplit(int n, bool ensureMeasurement);

	string assaySplit(Volume & volume, bool ensureMeasurement);

	string assayTransIn();

	string assayTransOut(Container &con);





	/*
	 * NOT IMPLEMENTED YET
	 * Name: merge Steps
	 * Purpose: It may be useful to have all Nodes with all the information
	 * 			in one place instead of haveing to iterate through the steps.
	 */
//	assayProtocol mergeSteps();

	/*
	 * Name: Collect Output
	 * Purpose: at the end of a protocol this function identifies the containers
	 * 			that are still active, and creates a node for them.
	 *
	 * 	Note** that this gets called inside of endprotocol and should not be used anywhere else.
	 */
	void collectOutput();

	/*
	 * Name: assayPreMix
	 * Purpose: this takes all the links stored in SRC and moves them over to DEST
	 * 			after it moves over it clears the links from SRC.
	 */
	void assayPreMix(Container & Src, Container & Dest);

	/*
	 * Name: addToLinkMap
	 * Purpose: 1) takes the linkName and pushes it onto a map of child/parents.
	 * 			   if the are two parents it does an intermediate step of automatically
	 * 			   adding a mix step and then proceeds to adding the new links
	 * 			2) tracks containers and manages them for the use of extracting the outputs at the end.
	 */
	void addToLinkMap(Container & con, string linkName);

	/*
	 * Name: assayDrain
	 * Purpose: creates a waste Node and then clears the container and resets
	 * 			it to be able to be used again if needed
	 */
	void assayDrain(Container & con, string outputSink);

	/*
	 * Name: clear Mixture
	 * Purpose: clears the mixture associated with the mixture.
	 */
	void clearMixture(Container &con);

	/*
	 * Name: find Mix
	 * Purpose: finds a mixture inside of a list of mixtures
	 */
	aMixture findMix(Container &con);

	/*
	 * Name: add to Mixture List
	 * Purpose: adds a fluid to the mixtuer already contained in CON
	 */
	aMixture addtoMixtureList(Container& con, Fluid& fluid);

	/*
	 * Name: add to Mixture List
	 * Purpose: adds all fluid to the mixture already contained in SRC from the container DEST
	 */
	aMixture addToMixtureList(Container& src, Container& dest);

	//=============================================================================
	//Translator from the BioCoder to Microfluidic Droplet Scheduler/Placer
	//=============================================================================
	void Translator(string filename, DAG *dag,bool color, bool all);
	//=============================================================================
	//Translator from the BioCoder to MicroFluidic Control Flow System.
	//=============================================================================

	void Translator(string file);

	/*
	 * returns the number of active containers to be able to tell if the sim is at max or not.
	 */
	int getDropCount();

};


#endif /* ASSAYPROTOCOL_H_ */
