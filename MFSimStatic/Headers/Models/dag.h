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
 * Name: DAG (Directed Acyclic Graph)											*
 *																				*
 * Details: This class represents a DAG, which represents an assay.  A DAG is	*
 * composed of a number AssayNodes.												*
 *-----------------------------------------------------------------------------*/
#ifndef _DAG_H
#define _DAG_H

using namespace std;
#include "assay_node.h"
#include "../Resources/enums.h"
#include "../Testing/claim.h"
#include <stdio.h>
#include <vector>
#include <math.h>
#include <map>

class AssayNode;

class DAG : public Entity
{
	protected:
		// Variables
		static int next_id;
		vector<AssayNode*> allNodes;
		vector<AssayNode*> heads;
		vector<AssayNode*> tails;
		vector<AssayNode*> mixes;
		vector<AssayNode*> dilutes;
		vector<AssayNode*> splits;
		vector<AssayNode*> heats;
		vector<AssayNode*> detects;
		vector<AssayNode*> storage;
		vector<AssayNode*> storageHolders;
		vector<AssayNode*> others;

		unsigned long long freqInHz;
		static int orderNum;
		string name;

		// Methods
		void AddNodeToDAG(AssayNode *node);
		void RemoveNodeFromDAG(AssayNode *node);
		//AssayNode * AddStorageNode();
		void InsertNode(AssayNode *p, AssayNode *c, AssayNode *insert);

	public:
		// Constructors
		DAG();
		virtual ~DAG();

		// Methods
		AssayNode * AddDispenseNode(string inputWell, double volume, string nodeName);
		AssayNode * AddMixNode(int numDropsBefore, double seconds, string nodeName);
		AssayNode * AddDiluteNode(int numDropsBefore, double seconds, string nodeName);
		AssayNode * AddSplitNode(bool isFaultTolerant, int numDropsAfter, double seconds, string nodeName);
		AssayNode * AddHeatNode(double seconds, string nodeName);
		AssayNode * AddDetectNode(int numDropsIO, double seconds, string nodeName);
		AssayNode * AddOutputNode(string outputSink, string nodeName);
		AssayNode * AddStorageNode(string nodeName);
		AssayNode * AddGeneralNode(string nodeName);
		AssayNode * AddWashNode(string nodeName);

		AssayNode * AddDispenseNode(string inputWell, double volume);
		AssayNode * AddMixNode(int numDropsBefore, double seconds);
		AssayNode * AddDiluteNode(int numDropsBefore, double seconds);
		AssayNode * AddSplitNode(bool isFaultTolerant, int numDropsAfter, double seconds);
		AssayNode * AddHeatNode(double seconds);
		AssayNode * AddDetectNode(int numDropsIO, double seconds);
		AssayNode * AddOutputNode(string outputSink);
		AssayNode * AddStorageNode();
		AssayNode * AddStorageHolderNode();
		AssayNode * AddGeneralNode();
		AssayNode * AddWashNode();

		void ParentChild(AssayNode *p, AssayNode *c);
		//void ValidateAndOrder();

		// Getters/Setters
		void setFreq(unsigned long long freqHz);
		void setName(string n) { name = n; }
		string getName() { return name; }
		vector<AssayNode *> getAllNodes() { return allNodes; }
		vector<AssayNode *> getAllInputNodes() { return heads; }
		vector<AssayNode *> getAllOutputNodes() { return tails; }
		vector<AssayNode *> getAllStorageHolders() { return storageHolders; }
		int getNumNonIoNodes() { return (allNodes.size() - heads.size() - tails.size()); }
		bool requiresHeater() { return heats.size() > 0; }
		bool requiresDetector() { return !detects.empty(); }

		// Print/Debug
		void PrintSchedule();
		void PrintParChildRelationships();
		void OutputGraphFile(string filename, bool color, bool fullStats);
		string GetPrintableName();

		// Transformation Functions - Used only once to convert assays in various ways
		void ConvertMixSplitsToDilutes();
		void ConvertMixesToDiluteWithSecondDropletOutputting();

		// Friend Classes
		friend class RealTimeEvalListScheduler;
		friend class GrissomFppcPathScheduler;
		friend class GrissomFppcScheduler;
		friend class GrissomFppcLEBinder;
		friend class GenetPathScheduler;
		friend class GrissomPathBinder;
		friend class RickettScheduler;
		friend class GrissomLEBinder;
		friend class GenetScheduler;
		friend class KamerLlPlacer;
		friend class ListScheduler;
		friend class PathScheduler;
		friend class FDLScheduler;
		friend class ILPScheduler;
		friend class PinMapper;
		friend class Priority;
		friend class Analyze;
		friend class Test;
};
#endif
