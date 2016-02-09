///lpsolve_V8
//lpSolve_DuringStore.h Backup ///
/*
 * lpsolve.h
 *
 *  Created on: Oct 24, 2012
 *      Author: Kenneth O'Neal
 */

#ifndef ILP_SCHEDULER_H_
#define ILP_SCHEDULER_H_


#include "../../Headers/Scheduler/scheduler.h"
#include "../Scheduler/priority.h"
//#include "../structs.h"
#include "../../Headers/Scheduler/list_scheduler.h"
#include "../../Headers/Scheduler/force_directed_list_scheduler.h"
#include "../../Headers/Scheduler/path_scheduler.h"
#include "../../lp_lib.h"

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <time.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <math.h>

class AssayNode;

//struct for returning of information needed for establishing constraints
//this struct works for mix, split, detect, heat.
struct RestrictionVals{
	unsigned type;
	unsigned numberOf;
	unsigned maximumOf;
	vector< pair <unsigned, double> > locations; //first in pair = node location, second = node delay
};

struct MaxInput{
	string portName;
	vector< pair<unsigned, double> > locations;
};

struct InputRestrictVals{
	unsigned maximumOf;
	vector< unsigned > locations; //location, delay pair
	string portName;
	double delay;
};

struct DependencyConstr{
	unsigned nodeLocation;
	vector<unsigned> parentLocation;
};

//used in get delays, will be used to calculate(setJbounds) summation bounds for storage
struct DelayVal{
	unsigned location;
	unsigned type;
	unsigned delay;
};


class ILPScheduler : public Scheduler
{

private:

public:
////constructors
	ILPScheduler();
	virtual ~ILPScheduler();

////Utils:
	//converts float to string
	string convertFloatToString(float number);
	//Alternative for MaxTS based on ALAP scheduling
	pair< unsigned, unsigned> ALAP(DAG * dag, DmfbArch *arch);

////Getters: (max values for scheduling)
	//Computes Max TS value we want to have based on List Scheduling
	pair< vector < unsigned >, unsigned > getMaxs(DmfbArch * arch, DAG * dag);
	//Determine max number of inputs that can occur simultaneously of type k
	vector< MaxInput > getInputs(DmfbArch * arch, DAG * dag);
	//Get operation delays
	vector<DelayVal> getDelays(DmfbArch* arch, DAG* dag, RestrictionVals Mix, RestrictionVals Detect, RestrictionVals Heat, RestrictionVals Split, RestrictionVals Output, vector<MaxInput> Input );
	//Get number of mix operations
	RestrictionVals getNumMix(DmfbArch* arch, DAG* dag, pair< vector <unsigned> , unsigned >);
	//Get number of detect operations
	RestrictionVals getNumDetect(DmfbArch* arch, DAG* dag, pair< vector <unsigned> , unsigned > );
	//Get number of heat operations
	RestrictionVals getNumHeat(DmfbArch* arch, DAG* dag, pair< vector <unsigned> , unsigned > );
	//Get number of split operations
	RestrictionVals getNumSplit(DmfbArch* arch, DAG* dag, pair< vector <unsigned> , unsigned > );
	//Get number of output operations
	RestrictionVals getNumOutput(DmfbArch* arch, DAG* dag, pair< vector <unsigned> , unsigned >);
	//Get constraint variable values (MaxTS, number of operations)
	pair<unsigned, unsigned> getVarVals(DmfbArch * arch, DAG* dag, unsigned MaxTS);
	//Get parent dependencies
	vector<DependencyConstr> getParents(DmfbArch* arch, DAG* dag);
	//Get children dependencies
	vector<DependencyConstr> getChildren(DmfbArch* arch, DAG* dag);

////Setters:
	//Write variable max constraints for dag types to mod file
	void setRestVarMaxs(DmfbArch * arch, DAG * dag, unsigned MaxTS, unsigned MixMax, unsigned HeatMax, unsigned DetectMax, unsigned SplitMax, ofstream &fs);
	//Write General resource constraints to mod file
	void setGeneralRestricts(DmfbArch * arch, DAG * dag, unsigned MaxTS, unsigned MixMax, ofstream &fs);
	//Write Store constraints to mod file
	void setStoreRestrictsUpd(DmfbArch * arch, DAG * dag, unsigned MaxTS, vector<DelayVal> Delays, vector<DependencyConstr> Childs, ofstream &fs);
	//Write Mix constraints to mod file
	void setMixRestrict(DmfbArch * arch, DAG* dag, RestrictionVals MixRestricts, unsigned MaxTS, ofstream &fs);
	//Write detect constraints to mod file
	void setDetectRestrict(DmfbArch* arch, DAG* dag, RestrictionVals DetectRestricts, unsigned MaxTS, ofstream &fs);
	//write heat constraints to mod file
	void setHeatRestrict(DmfbArch* arch, DAG* dag, RestrictionVals HeatRestrict, unsigned MaxTS, ofstream &fs);
	//write split constraints to mod file
	void setSplitRestrict(DmfbArch* arch, DAG* dag, RestrictionVals SplitRestrict, unsigned MaxTS, ofstream &fs);
	//write output constraints to mod file (For consistency with other schedulers, do not use)
	void setOutputRestrict(DmfbArch* arch, DAG* dag, RestrictionVals OutRestrict, unsigned MaxTS, ofstream &fs);
	//Write variables T and V for constaint usage in mod file
	void setVarVals(DmfbArch * arch, DAG* dag, pair<unsigned, unsigned> varVals, vector<DelayVal> delays, vector<DependencyConstr> Childs, ofstream &fs);
	//Write operation dependencies for mod file
	void setDependent(DmfbArch* arch, DAG* dag, vector<DependencyConstr>, vector<DelayVal> delays, ofstream &fs);
	//write objective for mod file
	void setObjective(unsigned NumOps, ofstream &fs);
	//set timing in dag ops to "write" schedule. Achieved via ILP solution result parsing
	void setTiming(DmfbArch * arch, DAG * dag, vector<DelayVal> delays, vector<double> StartTimes, int objective_val);

	//The following two functions must be reimplimented for correctness!!!!//////
	//Get maximum inputs allowed
	vector< pair<string, unsigned> > getMaxInput(DmfbArch* arch, DAG* dag);
	//Set input constraints in mod file
	void setInputRestrict(DmfbArch* arch, DAG* dag, vector<MaxInput> InpRestricts, unsigned MaxTS, vector< pair<string, unsigned> > PnameCount, ofstream &fs);

////Dag Operations and Prep

	//reinitialize the dag for subsequent scheduling
	void dagReinitialize(DAG* dag);
	//reset resource availability for subsequent scheduling
	int resetAvailResourcesForGrissomFixedPlacer(DmfbArch *arch);
	//Bind resources to operations to perform scheduling
	void bindResources(DmfbArch * arch, DAG * dag, vector<DelayVal> delays, vector<double> StartTimes, int objective_val);
	//find Ready dispense well for input operations
	IoResource * getReadyDispenseWell(string fluidName, unsigned long long schedTS);
	//Insert storage nodes in dag
	int storageNodeInsertion(DmfbArch * arch, DAG * dag, vector<DelayVal> delays, vector<double> StartTimes);
	//Bind storage nodes
	void storageNodeBinding(DmfbArch * arch, DAG * dag, vector<DelayVal> delays, vector<double> StartTimes, int objective_val, int dropsInStorage);

////Schedule
	//Perform overall scheduling process
	unsigned long long schedule(DmfbArch *arch, DAG *dag);

};

#endif /* ILP_SCHEDULER_H_ */
