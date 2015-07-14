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
 * Name: Graph	 																*
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
 * This class represents a graph class used to store an intermediate version	*
 * of the assay inbetween the BioCoder code and the final DAG.					*
 *-----------------------------------------------------------------------------*/
#ifndef __GRAPH__
#define __GRAPH__
//#define NULL 0
#include "BioCoder.h"
#include "BioEnums.h"




struct graphNode {
	int id;
	float time;
	char* unit;
	int type;
	char* name;
	int pcr_or_centrifuge;
	struct graphNodelist* in;
	struct graphNodelist* out;
graphNode();
graphNode(const graphNode & n);
};



struct graphNodelist {
	graphNode* node;
	graphNodelist* next;

	graphNodelist();
	graphNodelist(const graphNodelist & n);
// create a new graphNode, with the given name, and assign it a unique id
graphNode* create_node(char* name);

graphNode* create_node_with_type(char* name, fluid_type type);
// adds the given graphNode to the list of graphNodes.
void add_to_list(graphNode* node);
// adds an edge from node1 to node2
void create_edge(graphNode* node1, graphNode* node2);

// adds an edge from x1 to x2, where x1 and x2 are Fluids, Samples, or Operations
// if nodes do not yet exist for these quantities, then create them
void create_edge_from_fluids(void* x1, void* x2);

void create_edge_from_containers(void* x1, void* x2);

void create_edge_from_fluid_to_container(void* x1, void* x2);

void create_edge_from_container_to_fluid(void* x1, void* x2);


};

#endif
