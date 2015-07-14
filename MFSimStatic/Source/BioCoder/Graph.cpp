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
/*---------------------------Implementation Details-----------------------------*
 * Source: assayProtocol.cpp													*
 * Original Code Author(s): Chris Curtis										*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Originally taken and heavily modified from BioCoder:				*
 * http://research.microsoft.com/en-us/um/india/projects/biocoder/				*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/BioCoder/BioCoderStructs.h"
#include "../../Headers/BioCoder/Graph.h"
#include <stdlib.h>
#include <stdio.h>

// the next unique ID that should be assigned to a node
static int MAX_NODE_ID = 0;

// list of all nodes




graphNode::graphNode()
{
	id=0;
	time=0;
	unit="";
	type=0;
	name="";
	pcr_or_centrifuge=0;
	in=NULL;
	out=NULL;
}
graphNode::graphNode(const graphNode &n)
:id(n.id),time(n.time),unit(n.unit),type(n.type),name(n.name),pcr_or_centrifuge(n.pcr_or_centrifuge),in(n.in),out(n.out)
{
}

graphNode* graphNodelist:: create_node(char* name) {
	return create_node_with_type(name, CONTAINER);
}



// create a new node, with the given name and type, and assign it a unique id
graphNode* graphNodelist:: create_node_with_type(char* name, fluid_type type) {
	graphNode* node = (graphNode*)calloc(1, sizeof(graphNode));
	add_to_list(node);
	node->in = (graphNodelist*)calloc(1, sizeof(graphNodelist));
	node->out = (graphNodelist*)calloc(1, sizeof(graphNodelist));
	node->name = name;
	node->id = MAX_NODE_ID++;
	node->type = type;
	node->time = 0;
	node->pcr_or_centrifuge = 0;
	return node;
}


graphNodelist::graphNodelist()
:node(NULL),next(NULL)
{}

graphNodelist ::graphNodelist(const graphNodelist &n)
:node(n.node),next(n.next)
{}
// adds the given node to the list of nodes.
void graphNodelist:: add_to_list(graphNode* node) {
	// if list is empty, point it to new item
	if (this->node == NULL) {
		this->node = node;
	} else {
		graphNodelist* last;
		graphNodelist* temp=this;
		// advance to end of list
		while (temp->next != NULL) {
			temp = temp->next;
		}
		// add node to end
		last = (graphNodelist*)calloc(1, sizeof(graphNodelist));
		last->node = node;
		temp->next = last;
	}
}
// adds an edge from node1 to node2
void graphNodelist:: create_edge(graphNode* node1, graphNode* node2) {
	node1->out->add_to_list(node2);
	node2->in->add_to_list(node1);
}
// adds an edge from x1 to x2, where x1 and x2 are Fluids, Samples, or Operations
// if nodes do not yet exist for these quantities, then create them
void graphNodelist:: create_edge_from_fluids(void* x1, void* x2) {
	Fluid* f1 = (Fluid*)x1;
	Fluid* f2 = (Fluid*)x2;
	// create nodes if needed
	if (f1->node == NULL) {
		f1->node = create_node_with_type(f1->original_name, f1->type);
	}
	if (f2->node == NULL) {
		f2->node = create_node_with_type(f2->original_name, f2->type);
	}
	// connect the nodes
	create_edge(f1->node, f2->node);
}
void graphNodelist:: create_edge_from_fluid_to_container(void* x1, void* x2) {
	Fluid* f1 = (Fluid*)x1;
	Container* f2 = (Container*)x2;
	// create nodes if needed
	if (f1->node == NULL) {
		f1->node = create_node_with_type(f1->original_name, f1->type);
	}
	if (f2->node == NULL) {
		f2->node = create_node_with_type(f2->name, f2->type);
	}
	// connect the nodes
	create_edge(f1->node, f2->node);
}

void graphNodelist:: create_edge_from_containers(void* x1, void* x2) {
	Container* f1 = (Container*)x1;
	Container* f2 = (Container*)x2;
	// create nodes if needed
	if (f1->node == NULL) {
		f1->node = create_node_with_type(f1->name, f1->type);
	}
	if (f2->node == NULL) {
		f2->node = create_node_with_type(f2->name, f2->type);
	}
	// connect the nodes
	create_edge(f1->node, f2->node);
}
void graphNodelist :: create_edge_from_container_to_fluid(void* x1, void* x2) {
	Container* f1 = (Container*)x1;
	Fluid* f2 = (Fluid*)x2;
	// create nodes if needed
	if (f1->node == NULL) {
		f1->node = create_node_with_type(f1->name, f1->type);
	}
	if (f2->node == NULL) {
		f2->node = create_node_with_type(f2->original_name, f2->type);
	}
	// connect the nodes
	create_edge(f1->node, f2->node);
}
