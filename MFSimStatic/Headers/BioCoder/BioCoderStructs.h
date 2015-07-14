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
/*--------------------------------File Details----------------------------------*
 * Type: BioCoder Structs														*
 * Name: BioCoderStructs														*
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
 * Contains the necessary structures for BioCoder.								*
 *-----------------------------------------------------------------------------*/
#ifndef BIOCODERSTRUCTS_H_
#define BIOCODERSTRUCTS_H_
#include<list>
#include<map>
#include <iostream>
#include <string>
#include "assayProtocol.h"
#include "BioCoder.h"

using namespace std;
#include "TimeVolumeSpeed.h"
#include "Graph.h"

struct BioCoder;
//struct assayProtocol;

struct Fluid
{

	graphNode* node;/*!< The current graph node. */
	char* original_name;/*!< The name of the fluid. */
	char* new_name;/*!< The name of the fluid after renaming. Initially, original_name = new_name */
	container_type container;/*!< The container of the fluid. */
	char* state;/*!< The state of the fluid. */
	float volume;/*!< The volume of the fluid. */
	enum vol_unit unit;/*!< The unit of volume. */
	fluid_type type;/*!< The type of this resource. */
	int used;/*!< Used internally to track the usage of the fluid. */
	int usage_index;/*!< Used internally to track the usage of the fluid. */

	Fluid();

	Fluid(const Fluid & f);
};
struct Container
{
	//data memebers
	graphNode* node;/*!< The current graph node. */
	enum container_type id;/*!< The container's id. */
	Fluid contents;/*!< The contents of the container. */
	float volume;/*!< The volume of the contents of the container. Note: This is different from the content's volume i. e. container.volume != container.contents.volume */
	char* name;/*!< The container's label. */
	fluid_type type;/*!< The type of this resource. */
	int used;/*!< Used internally to track the usage of the container. */
	int usage_index;/*!< Used internally to track the usage of the container. */
	list<string> tLinkName;/*<translator Link Name:  Used to have an identifier for the links between the different nodes*/

	Container();
	Container(const Container & c);
};
/*
 *Name:		TranslatorNameBank
 *Purpose:  data member that holds the names for the links between the different structs
 *			of assayProtocol.
 *
 */
struct tNameBank
{
private:
	std::string nodeNames[11];
public:
	tNameBank( string);
	/*
	 * Name: increaseNodeName
	 * Purpose: increases the link name at specified index.
	 * 			i.e. increase D1 to D2
	 */

	void incNodeName(enum TLinkNames);
	/*
	 * Name: accessName
	 * Purpose: accesses the Link name for the particular name needed.
	 */
	std::string accessName(enum TLinkNames);
};

/*
 * Name: ADetect
 * Purpose: Data structure for detections.
 * 			used for keeping track for assayProtocol
 */
struct aDetect
{
	std::string detectType;
	Time time;
	double volume;

	aDetect();
	aDetect (std::string type, Time t);

};
/*
 * Name: ADispense
 * Purpose: Data structure for dispense.
 * 			used for keeping track for assayProtocol
 */
struct aDispense
{
	Volume vol;
	Fluid flu;

	aDispense();
	aDispense(Fluid f, Volume v);

};
/*
 * Name: AMixture
 * Purpose: Data structure for Mix.
 */
struct aMixture
{
	Time time; // DTG
	int listNum;
	char* name;
	char * container;
	double volume;
	Fluid list[100];
	aMixture();

	/*
	 * Name: find
	 * Purpose: finds the container within the completeMixList
	 * 			returns the mixture found otherwise an empty mixture
	 * 			size denotes the size of the list
	 */
	aMixture find(Container &con, int size);

	/*
	 * Name: addToMixList
	 * Purpose: Adds specified fluid to a list of active containers,
	 * 			used for reference as well as use in outputs.
	 * 			size denotes size of list to find the container in.
	 */
	aMixture addToMixList( Container & con, const Fluid& fluid,int& size);
	/*
	 * Name: addToMixList
	 * Purpose: Function that will add the mixture from the source
	 *  		container to the destination container.
	 *  		clears the container name field(it takes all of the
	 *  		fluids out, so it is reuseable)returns the list of
	 *  		aMixtures. Size denotes size of list
	 */

	aMixture addToMixList( Container & src, Container& dest,int& size);

	/*
	 * Name: changeMixName
	 * Purpose: finds specific container and changes name field to the name
	 * 			passed in
	 */
	void changeMixName(Container& con,char* name,int size);

	/*
	 * Name: getTotalVol
	 * Purpose: returns the total volume inside of a mixture.
	 */
	double getTotalVol();

	/*
	 * Name: add
	 * Purpose: 	add function that takes in 2 volumes and
	 * returns the combined volume returns volume in ul(right now)
	 */
	Volume add(Volume vol1,Volume vol2);

};

/*
 * Name: aMix
 * Purpose: Data structure for mix.
 * 			used for keeping track for assayProtocol.
 */
struct aMix
{
	aMixture component;
	string typeofmix;
	double volume;
	aMix();
	aMix(aMixture m, string n);
};

/*
 * Name: aHeat
 * Purpose: Data structure for Heat.
 * 			used for keeping track for assayProtocol.
 */
struct aHeat
{
	Time time;
	double temp;
	aMixture fluids;
	double volume;

	aHeat();
	aHeat(Time t,double tmp,aMixture mix);

};

/*
 * Name: aSplit
 * Purpose: Data structure for Split.
 * 			used for keeping track for assayProtocol.
 */
struct aSplit
{
	//Time time;
	int numSplit;
	double volume;
	bool CF;//stands for control flow True if application is a Control flow application. Defaults false.
	bool ensureMeasurement; // Indicates that measurement must be perfect, without fault

	aSplit();
	aSplit(int n);
};

/*
 * Name: aStore
 * Purpose: Data structure for Store.
 * 			used for keeping track for assayProtocol.
 */
struct aStore
{
	Time time;
	double temp;
	aMixture fluids;
	double volume;

	aStore();
	aStore(Time t,double tmp,aMixture mix);

};

/*
 * Name: aOutput
 * Purpose: Data structure for Output.
 * 			used for keeping track for assayProtocol.
 */
struct aOutput
{
	string sinkName;
	string nodeName;

	aOutput();
	aOutput(string sink, string node);
};

struct aTransOut
{
	//string linkName;
	Container con;

	aTransOut();
	aTransOut(Container & cont);

};

struct aTransIn
{
	string lName;
//	aTransOut source;

	aTransIn();
	aTransIn(string linkName);
};

#endif /* BIOCODERSTRUCTS_H_ */
