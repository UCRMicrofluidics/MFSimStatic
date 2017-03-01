#include "../../Headers/Scheduler/ilp_scheduler.h"
#include "../../lp_lib.h"
#include "../../Headers/Util/util.h"
#include <cstring>
#include <errno.h>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#ifdef __linux__
	#include <dlfcn.h>
#endif

ILPScheduler::ILPScheduler() {

}

ILPScheduler::~ILPScheduler() {

}

string ILPScheduler::convertFloatToString(float number) {
	//convert float to string
	ostringstream buff;
	buff << number;
	return buff.str();
}

int ILPScheduler::resetAvailResourcesForGrissomFixedPlacer(DmfbArch *arch) {
	//returns the number of modules on the chip to their original values.

	int modWidth = 4;	//2;
	int modHeight = 3;	//5;
	int modIntWidth = modWidth + 2; // For interference region
	int modIntHeight = modHeight + 2; // For interference region

	for (int i = 0; i <= RES_TYPE_MAX; i++)
		availRes[i] = 0;

	int maxX = arch->getNumCellsX();
	int maxY = arch->getNumCellsY();
	int x;
	int y;

	// Create a map of the fixed-module layout
	OperationType cellType[maxX][maxY];
	for (x = 0; x < maxX; x++)
		for (y = 0; y < maxY; y++)
			cellType[x][y] = GENERAL;
	for (unsigned i = 0; i < arch->getExternalResources()->size(); i++) {
		FixedModule *fm = arch->getExternalResources()->at(i);
		for (x = fm->getLX(); x <= fm->getRX(); x++) {
			for (y = fm->getTY(); y <= fm->getBY(); y++) {
				claim(x < maxX && y < maxY,
						"A fixed module has coordinates that do not fit on the array.");
				claim(cellType[x][y] == BASIC_RES,
						"A cell has already been augmented with a fixed module; cannot augment with two fixed modules.");
				if (fm->getResourceType() == H_RES)
					cellType[x][y] = HEAT;
				else if (fm->getResourceType() == D_RES)
					cellType[x][y] = DETECT;
				else{
					cout<<"Unknown cellType is: "<<flush<<endl;
					cout<<fm->getResourceType()<<flush<<endl;
				}
			}
		}
	}

	x = 1;
	y = 1;
	while (y + modIntHeight < maxY) {
		if ((x + modIntWidth <= maxX) && (y + modIntHeight <= maxY)) {
			bool canHeat = false;
			bool canDetect = false;

			for (int xMod = x + 1; xMod < x + 1 + modWidth; xMod++) {
				for (int yMod = y + 1; yMod < y + 1 + modHeight; yMod++) {
					if (cellType[xMod][yMod] == DETECT)
						canDetect = true;
					else if (cellType[xMod][yMod] == HEAT)
						canHeat = true;
				}
			}

			if (canDetect && canHeat)
				availRes[DH_RES]++;
			else if (canDetect)
				availRes[D_RES]++;
			else if (canHeat)
				availRes[H_RES]++;
			else
				availRes[BASIC_RES]++;
		}
		x = x + modIntWidth + 1;
		if (x + modIntWidth >= maxX) {
			x = 1;
			y = y + modIntHeight + 1;
		}
	}

	// Now reset dispense/output resources
	while (!dispRes->empty()) {
		IoResource *r = dispRes->back();
		dispRes->pop_back();
		delete r;
	}
	while (!outRes->empty()) {
		IoResource *r = outRes->back();
		outRes->pop_back();
		delete r;
	}
	for (int i = 0; i < arch->getIoPorts()->size(); i++) {
		IoPort *iop = arch->getIoPorts()->at(i);
		if (iop->isAnInput()) {
			IoResource *dr = new IoResource();
			dr->port = iop;
			dr->name = iop->getPortName();
			dr->lastEndTS = 0;
			dr->durationInTS = ceil(
					(double) iop->getTimeInSec()
							/ ((double) arch->getSecPerTS()));
			dispRes->push_back(dr);
			delete dr; //Kenneth added 2/1/2016
		} else {
			IoResource *outR = new IoResource();
			outR->port = iop;
			outR->name = iop->getPortName();
			outR->lastEndTS = 0;
			outR->durationInTS = ceil(
					(double) iop->getTimeInSec()
							/ ((double) arch->getSecPerTS()));
			outRes->push_back(outR);
			delete outR; //Kenneth added 2/1/2016
		}
	}

	int numModules = 0;
	for (int i = 0; i <= RES_TYPE_MAX; i++)
		numModules += availRes[i];
	return numModules;
}

///////Dag Reinitialize (set dag to default vals for scheduling)///////
void ILPScheduler::dagReinitialize(DAG * dag) {
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		AssayNode *n = dag->allNodes.at(i);
		n->boundedResType = UNKNOWN_RES;
		n->startTimeStep = 0;
		n->endTimeStep = 0;
		n->SetStatus(UNBOUND_UNSCHED);
	}

	vector<AssayNode*> storage;
	for (int i = dag->allNodes.size() - 1; i >= 0; i--) {
		if (dag->allNodes.at(i)->type == STORAGE) {
			storage.push_back(dag->allNodes.at(i));
			dag->allNodes.erase(dag->allNodes.begin() + i);
		}
	}
	dag->storage.clear();
	for (unsigned i = 0; i < dag->storageHolders.size(); i++) {
		AssayNode *n = dag->storageHolders.at(i);
		delete n;
	}
	dag->storageHolders.clear();

	for (int i = storage.size() - 1; i >= 0; i--) {
		AssayNode *s = storage.at(i);
		AssayNode *p = s->parents.front();
		AssayNode *c = s->children.front();

		// Remove the storage from the parent
		for (unsigned k = 0; k <= p->children.size(); k++) {
			if (p->children.at(k) == s) {
				p->children.erase(p->children.begin() + k);
				break;
			}
		}
		p->children.push_back(c);

		// Remove the storage from the child
		for (unsigned k = 0; k <= c->parents.size(); k++) {
			if (c->parents.at(k) == s) {
				c->parents.erase(c->parents.begin() + k);
				break;
			}
		}
		c->parents.push_back(p);
		delete s;
	}
	storage.clear();
}

///////Creating M (Upper Bound on Maximal time step completion time///////
pair<unsigned, unsigned> ILPScheduler::ALAP(DAG * dag, DmfbArch *arch) {
	unsigned Num_Ops = 0;
	Num_Ops = dag->allNodes.size();

	dagReinitialize(dag);

	for (unsigned i = 0; i < dag->tails.size(); i++) //schedule tails (outputs) first
			{
		dag->tails.at(i)->status = SCHEDULED;
		dag->tails.at(i)->startTimeStep = 0;
		dag->tails.at(i)->endTimeStep = 1;
	}
	vector<AssayNode*> Not_finished;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) //nodes not finished yet
			{
		if (!(dag->allNodes.at(i)->status == SCHEDULED)) {
			Not_finished.push_back(dag->allNodes.at(i));
		}
	}
	unsigned TS = 0;
	while (!Not_finished.empty()) // while nodes to schedule
	{
		for (unsigned i = 0; i < dag->allNodes.size(); i++) // for each node left to schedule
				{
			bool status = true;
			if (!(dag->allNodes.at(i)->status == SCHEDULED)) {
				for (unsigned j = 0; j < dag->allNodes.at(i)->children.size();
						j++) // for all the children at the current node
						{
					//if not finished set flag
					if (!(dag->allNodes.at(i)->children.at(j)->status
							== SCHEDULED
							&& dag->allNodes.at(i)->children.at(j)->endTimeStep
									< TS)) {
						status = false;
					}
				}
				//flag not set, so can schedule
				if (status == true) {
					dag->allNodes.at(i)->status = SCHEDULED;
					dag->allNodes.at(i)->startTimeStep = TS;
					dag->allNodes.at(i)->endTimeStep = TS
							+ ceil(
									(float) dag->allNodes.at(i)->cycles
											/ (float) arch->getFreqInHz()
											/ arch->getSecPerTS());
					//Not_finished.erase(Not_finished.begin()+i);
					for (unsigned k = 0; k < Not_finished.size(); k++) {
						if (Not_finished.at(k)->name
								== dag->allNodes.at(i)->name) {
							Not_finished.erase(Not_finished.begin() + k);
						}
					}
				}
			}
		}
		TS++;
	}

	unsigned Maxts = 0;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) //find max time step
			{
		if (dag->allNodes.at(i)->endTimeStep > Maxts) {
			Maxts = dag->allNodes.at(i)->startTimeStep; //16 bet so far

		}

	}
	return pair<unsigned, unsigned>(Num_Ops, Maxts);
	///num_ops = number of operations
	///MaxTS = Maximum timestep allowed
}

//retrieving Variable max values (Max Time, and NumOps)//
pair<unsigned, unsigned> ILPScheduler::getVarVals(DmfbArch * arch, DAG* dag, unsigned MaxTS) {
	unsigned MaxTimeStep = MaxTS;
	unsigned NumbOps = dag->allNodes.size();
	return pair<unsigned, unsigned>(NumbOps, MaxTimeStep);
}

//Writing to .mod file all variable initialization and bounding//
void ILPScheduler::setVarVals(DmfbArch * arch, DAG* dag,
		pair<unsigned, unsigned> varVals, vector<DelayVal> delays,
		vector<DependencyConstr> Childs, ofstream &fs) {

	//note: varVals.first = numOps
	//note: VarVals.second = maxTS;
	if (!fs) {
		std::cerr << "Cannot open the output file in setVarVals" << std::endl;
		exit(1);
	}

	char buffer[5], buffer2[5];
	string ParamV = "param V, integer, >=0, default ";
	string ParamT = "param T, integer, >= 0, default ";

	fs << ParamV << Util::itoa(varVals.first, buffer, 10) << ";" << "\n" << "\n";
	fs << ParamT <<Util::itoa(varVals.second, buffer2, 10) << ";" << "\n" << "\n";

	fs
			<< "#M is a large constant used for indicator variable in storage restrictions"
			<< "\n";
	fs << "param M, integer, >= 0, default 10000;" << "\n" << "\n";

	fs << "#binary variable x[i,j] signifying op i scheduled at time j" << "\n";
	fs << "var x{1..V, 0..T}, binary;" << "\n" << "\n";
	fs << "var m{1..V, 0..T}, binary;" << "\n" << "\n";

	fs << "var y, binary;" << "\n";
	fs << "#start time variables" << "\n";

	for (unsigned i = 1; i < varVals.first + 1; i++) {
		char buffer[5];
		fs << "var s" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}

	fs << "\n" << "#Num Mix @ j variables" << "\n";
	for (unsigned i = 0; i < varVals.second + 1; i++) {
		char buffer[5];
		fs << "var nm" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}

	fs << "\n" << "#Num Detect @ j variables" << "\n";
	for (unsigned i = 0; i < varVals.second + 1; i++) {
		char buffer[5];
		fs << "var nd" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}

	fs << "\n" << "#Num Heat @ j variables" << "\n";
	for (unsigned i = 0; i < varVals.second + 1; i++) {
		char buffer[5];
		fs << "var nh" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}

	fs << "\n" << "#Num Split @ j variables" << "\n";
	for (unsigned i = 0; i < varVals.second + 1; i++) {
		char buffer[5];
		fs << "var nsplit" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}

	fs << "\n" << "#Num Store @ j variables" << "\n";
	for (unsigned i = 0; i < varVals.second + 1; i++) {
		char buffer[5];
		fs << "var ns" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}
	fs << "\n";

	//End Times
	for (unsigned i = 1; i < varVals.first + 1; i++) {
		char buffer[5];
		fs << "var end" << Util::itoa(i, buffer, 10) << ", integer, >= 0;" << "\n";
	}
	fs << "\n";

	for (unsigned i = 1; i < varVals.first + 1; i++) {
		char buffer[5];
		fs << "s.t. a" << Util::itoa(i, buffer, 10) << ": sum{j in 0..T} x["
				<< Util::itoa(i, buffer, 10) << ",j] = 1;" << "\n";
	}
	fs << "\n";

	//Start times
	fs << "#start time si = sum from j =1 to maxTS, j*x[i,j]" << "\n";
	for (unsigned i = 1; i < varVals.first + 1; i++) {
		char buffer[5];
		fs << "s.t. b" << Util::itoa(i, buffer, 10) << ": s" << Util::itoa(i, buffer, 10)
				<< " = sum{j in 0..T} (j*x[" << Util::itoa(i, buffer, 10) << ",j]);"
				<< "\n";
	}
	fs << "\n";

	//finish times
	fs << "#end time endi = start time + delay(i) time" << "\n";
	for (unsigned i = 1; i < varVals.first + 1; i++) {
		char buffer[5];
		char buffer2[5];

		fs << "s.t. fin" << Util::itoa(i, buffer, 10) << ": end"
				<< Util::itoa(i, buffer, 10) << " = s" << Util::itoa(i, buffer, 10) << " + "
				<< Util::itoa(delays.at(i - 1).delay, buffer2, 10) << ";" << "\n";
	}
	fs << "\n";
}

//Getting input constraints
vector<MaxInput> ILPScheduler::getInputs(DmfbArch * arch, DAG * dag) {
	//FIND ALL OF THE INPUT PORTS /////////
	vector<pair<string, vector<unsigned> > > ports;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		if (dag->allNodes.at(i)->type == 3) //input type is type 3
				{
			if (ports.size() == 0) {
				vector<unsigned> temp;
				temp.push_back(i + 1);
				ports.push_back(make_pair(dag->allNodes.at(i)->portName, temp));
			} else {
				for (unsigned j = 0; j < ports.size(); j++) {
					bool found_port = false;
					bool found_loc = false;
					for (unsigned k = 0; k < ports.size(); k++) {
						if (dag->allNodes.at(i)->portName
								== ports.at(k).first) {
							found_port = true;
							for (unsigned l = 0; l < ports.at(k).second.size();
									l++) {
								if (ports.at(k).second.at(l) == i + 1) {
									found_loc = true;
								}
							}
							if (found_loc == false) {
								ports.at(k).second.push_back(i + 1);
							}
						}
					}
					if (found_port == false) {
						vector<unsigned> temp;
						ports.push_back(
								make_pair(dag->allNodes.at(i)->portName, temp));
					}
				}
			}
		} else {
			continue;
		}
	}

	//NOW THAT ALL INPUTS HAVE BEEN FOUND, ESTABLISH LOCATIONS AND DELAYS /////
	vector<MaxInput> MaxI;
	for (unsigned i = 0; i < ports.size(); i++) {
		MaxInput Tempor;
		vector<pair<unsigned, double> > locations;
		for (unsigned j = 0; j < ports.at(i).second.size(); j++) {
			double OpSecs = 0;
			double SecPerOp = arch->getSecPerTS();
			double delay = 0;
			for (unsigned k = 0; k < arch->getIoPorts()->size(); k++) {
				if (ports.at(i).first
						== arch->getIoPorts()->at(k)->getPortName()) {
					OpSecs = arch->getIoPorts()->at(k)->getTimeInSec();
				}
			}
			delay = OpSecs / SecPerOp;
			locations.push_back(make_pair(ports.at(i).second.at(j), delay));
		}
		Tempor.portName = ports.at(i).first;
		Tempor.locations = locations;
		MaxI.push_back(Tempor);
	}

	return MaxI;
}

//Collect the max number of timesteps we are going to allow by running FDLS and List_S///////
pair<vector<unsigned>, unsigned> ILPScheduler::getMaxs(DmfbArch * arch,
		DAG * dag) {
	unsigned numModules = getAvailResources(arch);
	unsigned numDetect = availRes[D_RES];
	unsigned numHeat = availRes[H_RES];
	unsigned numDetHeat = availRes[DH_RES];
	unsigned numOutput = 1;

	///Establishing upper bound on TS. /////
	int lsTime = 0;

	vector<int> ScheduleTimes;
	ListScheduler ls;
	FDLScheduler fds;
	PathScheduler ps;
	dagReinitialize(dag);

	if(false){
	dag->PrintParChildRelationships();
	}

	ls.setMaxStoragePerModule(getMaxStoragePerModule());
	lsTime = ls.schedule(arch, dag);
	cerr<<"ls time is: "<<lsTime<<flush<<endl;
	ScheduleTimes.push_back(lsTime);

	pair<unsigned, unsigned> ALAPRes = ALAP(dag, arch);
	unsigned alapSchedTime = ALAPRes.second;
	cerr<<"ALAP time is: "<<alapSchedTime<<flush<<endl;
	//ScheduleTimes.push_back(alapSchedTime);
	//exit(1);


	//dag->PrintParChildRelationships();
	//determining minimum, used if more than one scheduled pushed above
	int M = ScheduleTimes.at(0);
	for (unsigned i = 0; i < ScheduleTimes.size(); i++) {
		if (ScheduleTimes.at(i) < M) {
			M = ScheduleTimes.at(i);
		}
	}

	vector<unsigned> temp;
	dagReinitialize(dag);
	temp.push_back(numModules); // temp holds numModules in first
	temp.push_back(numDetect); // temp holds num detect in second
	temp.push_back(numHeat); // temp holds num heat in third pos
	temp.push_back(numDetHeat); // temp holds numDetHeat in fourth position
	temp.push_back(numOutput); // temp holds maxnum outputs in fifth position
	return pair<vector<unsigned>, unsigned>(temp, M);

}

vector<DelayVal> ILPScheduler::getDelays(DmfbArch* arch, DAG* dag,
		RestrictionVals Mix, RestrictionVals Detect, RestrictionVals Heat,
		RestrictionVals Split, RestrictionVals Output,
		vector<MaxInput> Input) {
	vector<DelayVal> DelayVals;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		DelayVal Temp;
		Temp.location = i + 1;
		Temp.type = dag->allNodes.at(i)->type;
		DelayVals.push_back(Temp);
	}
	for (unsigned i = 0; i < DelayVals.size(); i++) {
		for (unsigned j = 0; j < Mix.locations.size(); j++) {
			if (DelayVals.at(i).location == Mix.locations.at(j).first) {
				DelayVals.at(i).delay = Mix.locations.at(j).second;
			}
		}
		for (unsigned k = 0; k < Detect.locations.size(); k++) {
			if (DelayVals.at(i).location == Detect.locations.at(k).first) {
				DelayVals.at(i).delay = Detect.locations.at(k).second;
			}
		}
		for (unsigned l = 0; l < Heat.locations.size(); l++) {
			if (DelayVals.at(i).location == Heat.locations.at(l).first) {
				DelayVals.at(i).delay = Heat.locations.at(l).second;
			}
		}
		for (unsigned m = 0; m < Split.locations.size(); m++) {
			if (DelayVals.at(i).location == Split.locations.at(m).first) {
				DelayVals.at(i).delay = Split.locations.at(m).second;
			}
		}
		for (unsigned n = 0; n < Output.locations.size(); n++) {
			if (DelayVals.at(i).location == Output.locations.at(n).first) {
				DelayVals.at(i).delay = Output.locations.at(n).second;
			}
		}
		for (unsigned p = 0; p < Input.size(); p++) {
			for (unsigned q = 0; q < Input.at(p).locations.size(); q++) {
				if (DelayVals.at(i).location
						== Input.at(p).locations.at(q).first) {
					DelayVals.at(i).delay = Input.at(p).locations.at(q).second;
				}
			}
		}
	}
	return DelayVals;
}

///////Get the parents for each node in the Dag///
vector<DependencyConstr> ILPScheduler::getParents(DmfbArch* arch, DAG* dag) {
	vector<DependencyConstr> temp;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		DependencyConstr Dep;
		if (dag->allNodes.at(i)->parents.size() != 0) {
			Dep.nodeLocation = i;
			for (unsigned j = 0; j < dag->allNodes.at(i)->parents.size(); j++) {
				Dep.parentLocation.push_back(
						dag->allNodes.at(i)->parents.at(j)->id - 1);
				//Dep.parentLocation.push_back(dag->allNodes.at(i)->parents.at(j)->id);
			}
			temp.push_back(Dep);
		}
	}
	return temp;
}

//////Get the children of each node in the Dag ///////////////////
vector<DependencyConstr> ILPScheduler::getChildren(DmfbArch* arch, DAG* dag) {
	vector<DependencyConstr> childs;
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		DependencyConstr Dep;
		if (dag->allNodes.at(i)->children.size() != 0) {
			Dep.nodeLocation = i; //parent loc
			for (unsigned j = 0; j < dag->allNodes.at(i)->children.size();
					j++) {
				Dep.parentLocation.push_back(
						dag->allNodes.at(i)->children.at(j)->id - 1);
			}
			childs.push_back(Dep);
		}
	}
	return childs;
}

/////Write the dependency constraints to the .mod for lp_solve/////
void ILPScheduler::setDependent(DmfbArch* arch, DAG* dag,
		vector<DependencyConstr> DepCons, vector<DelayVal> delays,
		ofstream &fs) {
	//dependencyConstr contains nodeLocation, vector<parentLocation> as unsigned index vals for dag->allNodes
	//DelayVal contains node type and the types associated delay.
	unsigned count = 1;
	for (unsigned i = 0; i < DepCons.size(); i++) {
		for (unsigned j = 0; j < DepCons.at(i).parentLocation.size(); j++) {
			char buffer1[5];
			char buffer2[5];
			char buffer3[5];
			fs << "s.t. c" << Util::itoa(count, buffer1, 10) << " : s"
					<< Util::itoa(DepCons.at(i).nodeLocation + 1, buffer2, 10) << " >= s"
					<< Util::itoa(DepCons.at(i).parentLocation.at(j) + 1, buffer3, 10)
					<< "+";
			for (unsigned k = 0; k < delays.size(); k++)
				if (dag->allNodes.at(DepCons.at(i).parentLocation.at(j))->id
						== (int)delays.at(k).location) {
					char buffer4[5];
					fs << Util::itoa(delays.at(k).delay, buffer4, 10) << ";" << "\n"; //Previously
				}
			count++;
		}
	}
}

//simply returns the number of mix operations in combination with its type for later identification in
//the usage of constraint generation.
RestrictionVals ILPScheduler::getNumMix(DmfbArch* arch, DAG* dag,
		pair<vector<unsigned>, unsigned> Maxs) {
	string tempor;
	unsigned count = 0;
	vector<pair<unsigned, double> > locations;
	RestrictionVals Mix_restrict;
	for (unsigned temp = 0; temp < dag->allNodes.size(); temp++) {
		if (dag->allNodes.at(temp)->type == MIX) //mix is of type 4
				{
			count++;
			double OpSecs = dag->allNodes.at(temp)->GetNumSeconds();
			double SecPerOp = arch->getSecPerTS();
			double MixTime;
			MixTime = OpSecs / SecPerOp;
			locations.push_back(make_pair(temp + 1, MixTime));
		}
	}
	Mix_restrict.type = 4;
	Mix_restrict.numberOf = count;
	Mix_restrict.locations = locations;
	Mix_restrict.maximumOf = Maxs.first.at(0);
	return Mix_restrict;
}

///Writes the previously ascertained mix restrictions to the .mod for lp_solve/////
void ILPScheduler::setMixRestrict(DmfbArch * arch, DAG* dag,
		RestrictionVals MixRestricts, unsigned MaxTS, ofstream &fs) {
	fs << "\n" << "# mix restrictions " << "\n";
	if (MixRestricts.locations.size() == 1) {
		fs << "s.t. MIX1: nm1 = ";
		char buffer1[5], buffer2[5];
		for (unsigned i = 0; i <= MaxTS; i++) {
			for (unsigned j = 0; j < MixRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(MixRestricts.locations.at(j).first, buffer1, 10)
						<< "," << Util::itoa(i, buffer2, 10) << "]+";
			}
		}
		fs << "0;" << "\n";
	} else if (MixRestricts.locations.at(0).second < 1) {
		fs << "\n" << "#out size less one why doing this?" << "\n";
		unsigned count = 0;
		char buffer1[5], buffer2[5], buffer3[5];
		for (unsigned i = 0; i < MaxTS; i++) {
			fs << "s.t. MIX" << Util::itoa(count, buffer1, 10) << ": nm"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < MixRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(MixRestricts.locations.at(j).first, buffer2, 10)
						<< "," << Util::itoa(i, buffer3, 10) << "]+";
			}
			fs << "0;" << "\n";
			count++;
		}
	} else {
		for (unsigned i = 0; i < MaxTS; i++) {
			char buffer1[5], buffer2[5], buffer3[5];
			fs << "s.t. MIX" << Util::itoa(i, buffer1, 10) << ": nm"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < MixRestricts.locations.size(); j++) {
				if (i < MixRestricts.locations.at(j).second) {
					if (j != MixRestricts.locations.size() - 1) {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												MixRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
						} else {
							fs << "x["
									<< Util::itoa(MixRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
						}
					} else {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												MixRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
							fs << "0;" << "\n";
						} else {
							fs << "x["
									<< Util::itoa(MixRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
							fs << "0;" << "\n";
						}
					}
				} else if (i >= MixRestricts.locations.at(j).second) {
					if (j != MixRestricts.locations.size() - 1) {
						for (unsigned k = i;
								k > i - MixRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(MixRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
					} else {
						for (unsigned k = i;
								k > i - MixRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(MixRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
						fs << "0;" << "\n";
					}
				}
			}
		}
	}
}

////Find the number od detect operations present, and the number of detectors available ////
RestrictionVals ILPScheduler::getNumDetect(DmfbArch* arch, DAG* dag,
		pair<vector<unsigned>, unsigned> Maxs) {
	string tempor;
	unsigned count = 0;
	vector<pair<unsigned, double> > locations;
	RestrictionVals Detect_restrict;
	for (unsigned temp = 0; temp < dag->allNodes.size(); temp++) {
		if (dag->allNodes.at(temp)->type == DETECT) {
			count++;
			double OpSecs = dag->allNodes.at(temp)->GetNumSeconds();
			double SecPerOp = arch->getSecPerTS();
			double DetectTime;
			DetectTime = OpSecs / SecPerOp;
			locations.push_back(make_pair(temp + 1, DetectTime));
		}
	}
	Detect_restrict.type = 7;
	Detect_restrict.numberOf = count;
	Detect_restrict.locations = locations;
	Detect_restrict.maximumOf = Maxs.first.at(1) + Maxs.first.at(3);//.at(1) is # detect modules, .at(3) is num detectHeat Modules
	return Detect_restrict;
}

void ILPScheduler::setDetectRestrict(DmfbArch* arch, DAG* dag,
		RestrictionVals DetectRestricts, unsigned MaxTS, ofstream &fs) {
	fs << "\n" << "# Detect restrictions " << "\n";
	if (DetectRestricts.locations.size() == 1) {
		fs << "s.t. DET1: nd1 = ";
		char buffer1[5], buffer2[5];
		for (unsigned i = 0; i <= MaxTS; i++) {
			for (unsigned j = 0; j < DetectRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(DetectRestricts.locations.at(j).first, buffer1,
								10) << "," << Util::itoa(i, buffer2, 10) << "]+";
			}
		}
		fs << "0;" << "\n";
	} else if (DetectRestricts.locations.at(0).second < 1) {
		fs << "\n" << "#out size less one why doing this?" << "\n";

		unsigned count = 0;
		char buffer1[5], buffer2[5], buffer3[5];
		for (unsigned i = 0; i < MaxTS; i++) {
			fs << "s.t. DET" << Util::itoa(count, buffer1, 10) << ": nd"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < DetectRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(DetectRestricts.locations.at(j).first, buffer2,
								10) << "," << Util::itoa(i, buffer3, 10) << "]+";
			}
			fs << "0;" << "\n";
			count++;
		}
	} else {
		for (unsigned i = 0; i < MaxTS; i++) {
			char buffer1[5], buffer2[5], buffer3[5];
			fs << "s.t. DET" << Util::itoa(i, buffer1, 10) << ": nd"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < DetectRestricts.locations.size(); j++) {
				if (i < DetectRestricts.locations.at(j).second) {
					if (j != DetectRestricts.locations.size() - 1) {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												DetectRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
						} else {
							fs << "x["
									<< Util::itoa(
											DetectRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
						}
					} else {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												DetectRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
							fs << "0;" << "\n";
						} else {
							fs << "x["
									<< Util::itoa(
											DetectRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
							fs << "0;" << "\n";
						}
					}
				} else if (i >= DetectRestricts.locations.at(j).second) {
					if (j != DetectRestricts.locations.size() - 1) {
						for (unsigned k = i;
								k > i - DetectRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(
											DetectRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
					} else {
						for (unsigned k = i;
								k > i - DetectRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(
											DetectRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
						fs << "0;" << "\n";
					}
				}
			}
		}
	}
}

RestrictionVals ILPScheduler::getNumHeat(DmfbArch* arch, DAG* dag,
		pair<vector<unsigned>, unsigned> Maxs) {
	string tempor;
	unsigned count = 0;
	vector<pair<unsigned, double> > locations;
	RestrictionVals Heat_restrict;
	for (unsigned temp = 0; temp < dag->allNodes.size(); temp++) {
		if (dag->allNodes.at(temp)->type == HEAT) {
			count++;
			double OpSecs = dag->allNodes.at(temp)->GetNumSeconds();
			double SecPerOp = arch->getSecPerTS();
			double HeatTime;
			HeatTime = OpSecs / SecPerOp;
			locations.push_back(make_pair(temp + 1, HeatTime));
		}
	}
	Heat_restrict.type = 6;
	Heat_restrict.numberOf = count;
	Heat_restrict.locations = locations;
	Heat_restrict.maximumOf = Maxs.first.at(2) + Maxs.first.at(3);
	return Heat_restrict;
}

void ILPScheduler::setHeatRestrict(DmfbArch* arch, DAG* dag,
		RestrictionVals HeatRestricts, unsigned MaxTS, ofstream &fs) {
	fs << "\n" << "# Heat restrictions " << "\n";
	if (HeatRestricts.locations.size() == 1) {
		fs << "s.t. HEAT1: nh1 = ";
		char buffer1[5], buffer2[5];
		for (unsigned i = 0; i <= MaxTS; i++) {
			for (unsigned j = 0; j < HeatRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(HeatRestricts.locations.at(j).first, buffer1,
								10) << "," << Util::itoa(i, buffer2, 10) << "]+";
			}
		}
		fs << "0;" << "\n";
	} else if (HeatRestricts.locations.at(0).second < 1) {
		fs << "\n" << "#out size less one why doing this?" << "\n";
		unsigned count = 0;
		char buffer1[5], buffer2[5], buffer3[5];
		for (unsigned i = 0; i < MaxTS; i++) {
			fs << "s.t. HEAT" << Util::itoa(count, buffer1, 10) << ": nh"
					<< Util::itoa(count, buffer1, 10) << " = ";
			for (unsigned j = 0; j < HeatRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(HeatRestricts.locations.at(j).first, buffer2,
								10) << "," << Util::itoa(i, buffer3, 10) << "]+";
			}
			fs << "0;" << "\n";
			count++;
		}
	} else {
		for (unsigned i = 0; i < MaxTS; i++) {
			char buffer1[5], buffer2[5], buffer3[5];

			fs << "s.t. Heat" << Util::itoa(i, buffer1, 10) << ": nh"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < HeatRestricts.locations.size(); j++) {
				if (i < HeatRestricts.locations.at(j).second) {
					if (j != HeatRestricts.locations.size() - 1) {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												HeatRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
						} else {
							fs << "x["
									<< Util::itoa(HeatRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
						}
					} else {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												HeatRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
							fs << "0;" << "\n";
						} else {
							fs << "x["
									<< Util::itoa(HeatRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(0, buffer3, 10) << "]+";
							fs << "0;" << "\n";
						}
					}
				} else if (i >= HeatRestricts.locations.at(j).second) {
					if (j != HeatRestricts.locations.size() - 1) {
						for (unsigned k = i;
								k > i - HeatRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(HeatRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
					} else {
						for (unsigned k = i;
								k > i - HeatRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(HeatRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
						fs << "0;" << "\n";
					}
				}
			}
		}
	}
}

////Get the max number of inputs that can be done///////
vector<pair<string, unsigned> > ILPScheduler::getMaxInput(DmfbArch* arch,
		DAG* dag) {
	vector<pair<string, unsigned> > PnameCount;
	vector<IoPort*>* IP = arch->getIoPorts();
	for (unsigned i = 0; i < IP->size(); i++) {
		if (PnameCount.size() == 0) {
			unsigned val = 1;
			PnameCount.push_back(make_pair(IP->at(i)->getPortName(), val));
		} else {
			bool found = false;
			for (unsigned j = 0; j < PnameCount.size(); j++) {
				if (IP->at(i)->getPortName() == PnameCount.at(j).first) {
					PnameCount.at(j).second = PnameCount.at(j).second + 1;
					found = true;
				}
			}
			if (found == false) {
				unsigned val = 1;
				PnameCount.push_back(make_pair(IP->at(i)->getPortName(), val));
			}
		}

	}
	return PnameCount;
}

//////set the restrictions on input from GetMaxINputs//////////
void ILPScheduler::setInputRestrict(DmfbArch* arch, DAG* dag,
		vector<MaxInput> InpVals, unsigned MaxTS,
		vector<pair<string, unsigned> > PnameCount, ofstream &fs) {

	vector<InputRestrictVals> restricts;
	for (unsigned i = 0; i < InpVals.size(); i++) {
		for (unsigned j = 0; j < PnameCount.size(); j++) {
			if (InpVals.at(i).portName == PnameCount.at(j).first) {
				InputRestrictVals temp;
				string Name = InpVals.at(i).portName;
				temp.portName = Name;
				double delay;
				vector<unsigned> Indexes;
				for (unsigned k = 0; k < InpVals.at(i).locations.size(); k++) {
					Indexes.push_back(InpVals.at(i).locations.at(k).first);
				}
				temp.locations = Indexes;
				delay = InpVals.at(i).locations.at(0).second;
				temp.delay = delay;
				unsigned maximumOf;
				maximumOf = PnameCount.at(j).second;
				temp.maximumOf = maximumOf;
				restricts.push_back(temp);
			}
		}
	}

	fs << "\n" << "# Input restrictions " << "\n";
	unsigned count = 0;
	for (unsigned i = 0; i < restricts.size(); i++) {
		if (restricts.at(i).locations.size() == 1)// || restricts.at(i).delay < 1)
				{
			char buffer1[5], buffer2[5], buffer3[5], buffer4[5];
			fs << "s.t. INP" << Util::itoa(count, buffer1, 10) << ": ";
			for (unsigned j = 0; j < MaxTS; j++) {
				for (unsigned k = 0; k < restricts.at(i).locations.size();
						k++) {
					fs << "x["
							<< Util::itoa(restricts.at(i).locations.at(k), buffer2,
									10) << "," << Util::itoa(j, buffer3, 10) << "]+";
				}
			}
			count = count + 1;
			fs << "0 <= " << Util::itoa(restricts.at(i).maximumOf, buffer4, 10) << ";"
					<< "\n";
		} else if (restricts.at(i).delay < 1) {
			fs << "\n" << "#out size less one why doing this?" << "\n";
			unsigned count = 0;
			char buffer1[5], buffer2[5], buffer3[5], buffer4[5];
			for (unsigned j = 0; j < MaxTS; j++) {
				fs << "s.t. INP" << Util::itoa(count, buffer1, 10) << ": ";
				for (unsigned k = 0; k < restricts.at(i).locations.size();
						k++) {
					fs << "x["
							<< Util::itoa(restricts.at(i).locations.at(k), buffer2,
									10) << "," << Util::itoa(j, buffer3, 10) << "]+";
				}
				fs << "0 <= " << Util::itoa(restricts.at(i).maximumOf, buffer4, 10) << ";"
						<< "\n";
				count++;
			}
		} else {
			for (unsigned j = 0; j < MaxTS; j++) {
				char buffer1[5], buffer2[5], buffer3[5], buffer4[5];
				fs << "s.t. INP" << Util::itoa(count, buffer1, 10) << ": ";
				for (unsigned k = 0; k < restricts.at(i).locations.size();
						k++) {
					if (j < restricts.at(i).delay) {
						if (k != restricts.at(i).locations.size() - 1) {
							if (j != 0) {
								for (int l = j; l > -1; l--) {
									fs << "x["
											<< Util::itoa(
													restricts.at(i).locations.at(
															k), buffer2, 10)
											<< "," << Util::itoa(l, buffer3, 10)
											<< "]+";
								}
							} else if (j == 0) {
								fs << "x["
										<< Util::itoa(restricts.at(i).locations.at(k),
												buffer2, 10) << ",0]+";
							}
						} else {
							if (j != 0) {
								for (int l = j; l > -1; l--) {
									fs << "x["
											<< Util::itoa(
													restricts.at(i).locations.at(
															k), buffer2, 10)
											<< "," << Util::itoa(l, buffer3, 10)
											<< "]+";
								}
								fs << "0 <= "
										<< Util::itoa(restricts.at(i).maximumOf, buffer4,
												10) << ";" << "\n";
							} else if (j == 0) {
								fs << "x["
										<< Util::itoa(restricts.at(i).locations.at(k),
												buffer2, 10) << ",0]+";
								fs << "0 <= "
										<< Util::itoa(restricts.at(i).maximumOf, buffer4,
												10) << ";" << "\n";
							}
						}
					} else if (j >= restricts.at(i).delay) {
						if (k != restricts.at(i).locations.size() - 1) {
							for (unsigned l = j; l > j - restricts.at(i).delay;
									l--) {
								fs << "x["
										<< Util::itoa(restricts.at(i).locations.at(k),
												buffer2, 10) << ","
										<< Util::itoa(l, buffer3, 10) << "]+";
							}
						} else {
							for (unsigned l = j; l > j - restricts.at(i).delay;
									l--) {
								fs << "x["
										<< Util::itoa(restricts.at(i).locations.at(k),
												buffer2, 10) << ","
										<< Util::itoa(l, buffer3, 10) << "]+";
							}
							fs << "0 <= "
									<< Util::itoa(restricts.at(i).maximumOf, buffer4, 10)
									<< ";" << "\n";
						}
					}
				}
				count = count + 1;
			}
		}
	}
}

////Get the number of Split actions taking place//////
RestrictionVals ILPScheduler::getNumSplit(DmfbArch* arch, DAG* dag,
		pair<vector<unsigned>, unsigned> Maxs) {
	string tempor;
	unsigned count = 0;
	vector<pair<unsigned, double> > locations;
	RestrictionVals Split_restrict;
	for (unsigned temp = 0; temp < dag->allNodes.size(); temp++) {
		if (dag->allNodes.at(temp)->type == SPLIT) {
			count++;
			double OpSecs = dag->allNodes.at(temp)->GetNumSeconds();
			double SecPerOp = arch->getSecPerTS();
			double SplitTime;
			SplitTime = OpSecs / SecPerOp;
			locations.push_back(make_pair(temp + 1, SplitTime));
		}
	}
	Split_restrict.type = 5;
	Split_restrict.numberOf = count;
	Split_restrict.locations = locations;
	Split_restrict.maximumOf = Maxs.first.at(0);
	return Split_restrict;
}

//////setting the restrictions on split in .mod for lp_solve //////
void ILPScheduler::setSplitRestrict(DmfbArch* arch, DAG* dag,
		RestrictionVals SplitRestricts, unsigned MaxTS, ofstream &fs) {
	fs << "\n" << "# Split restrictions " << "\n";
	if (SplitRestricts.locations.size() == 1) {
		fs << "s.t. SPLIT1: nsplit1 = ";
		char buffer1[5], buffer2[5];
		for (unsigned i = 0; i <= MaxTS; i++) {
			for (unsigned j = 0; j < SplitRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(SplitRestricts.locations.at(j).first, buffer1,
								10) << "," << Util::itoa(i, buffer2, 10) << "]+";
			}
		}
		fs << "0;" << "\n";
	} else if (SplitRestricts.locations.at(0).second < 1) {
		fs << "\n" << "#split size less one why doing this?" << "\n";
		unsigned count = 0;
		char buffer1[5], buffer2[5], buffer3[5];
		for (unsigned i = 0; i < MaxTS; i++) {
			fs << "s.t. SPLIT" << Util::itoa(count, buffer1, 10) << ": nsplit"
					<< Util::itoa(count, buffer1, 10) << " = ";
			for (unsigned j = 0; j < SplitRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(SplitRestricts.locations.at(j).first, buffer2,
								10) << "," << Util::itoa(i, buffer3, 10) << "]+";
			}
			fs << "0;" << "\n";
			count++;
		}
	} else {
		for (unsigned i = 0; i < MaxTS; i++) {
			char buffer1[5], buffer2[5], buffer3[5];
			fs << "s.t. Split" << Util::itoa(i, buffer1, 10) << ": nsplit"
					<< Util::itoa(i, buffer1, 10) << " = ";
			for (unsigned j = 0; j < SplitRestricts.locations.size(); j++) {
				if (i < SplitRestricts.locations.at(j).second) {
					if (j != SplitRestricts.locations.size() - 1) {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												SplitRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
						} else if (i == 0) {
							fs << "x["
									<< Util::itoa(
											SplitRestricts.locations.at(j).first,
											buffer2, 10) << ",0]+";
						}
					} else {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												SplitRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
							fs << "0;" << "\n";
						} else if (i == 0) {
							fs << "x["
									<< Util::itoa(
											SplitRestricts.locations.at(j).first,
											buffer2, 10) << ",0]+";
							fs << "0;" << "\n";
						}
					}
				} else if (i >= SplitRestricts.locations.at(j).second) {
					if (j != SplitRestricts.locations.size() - 1) {
						for (unsigned k = i;
								k > i - SplitRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(
											SplitRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
					} else {
						for (unsigned k = i;
								k > i - SplitRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(
											SplitRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
						fs << "0;" << "\n";
					}
				}
			}
		}
	}
}

//Calculating number of output for .mod in lpSolve//////
RestrictionVals ILPScheduler::getNumOutput(DmfbArch* arch, DAG* dag,
		pair<vector<unsigned>, unsigned> Maxs) {
	string tempor;
	unsigned count = 0;
	vector<pair<unsigned, double> > locations;
	RestrictionVals Output_restrict;
	for (unsigned temp = 0; temp < dag->allNodes.size(); temp++) {
		if (dag->allNodes.at(temp)->type == OUTPUT) {
			count++;
			double OpSecs = dag->allNodes.at(temp)->GetNumSeconds(); //Todo:: Is this correct? or calc with n->cycles?
			//[Kenneth] 2/1/2016 : alternative is manual calculation like so:
			//ceil((double)n->cycles/(double)arch->getFreqInHz()
			double SecPerOp = arch->getSecPerTS();
			double OutTime;
			OutTime = OpSecs / SecPerOp;
			if (OutTime == 0) {
				OutTime = 1;
			}
			locations.push_back(make_pair(temp + 1, OutTime));
		}
	}
	Output_restrict.type = 8;
	Output_restrict.numberOf = count;
	Output_restrict.locations = locations;
	Output_restrict.maximumOf = Maxs.first.at(4);
	return Output_restrict;
}

//////setting the output restrictions for .mod///////
void ILPScheduler::setOutputRestrict(DmfbArch* arch, DAG* dag,
		RestrictionVals OutRestricts, unsigned MaxTS, ofstream &fs) {
	fs << "\n" << "# Out restrictions " << "\n";
	if (OutRestricts.locations.size() == 1) {
		fs << "s.t. OUT1: ";
		char buffer1[5], buffer2[5], buffer3[5];
		for (unsigned i = 0; i <= MaxTS; i++) {
			for (unsigned j = 0; j < OutRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(OutRestricts.locations.at(j).first, buffer1, 10)
						<< "," << Util::itoa(i, buffer2, 10) << "]+";
			}
		}
		fs << "0 <= " << Util::itoa(OutRestricts.maximumOf, buffer3, 10) << ";" << "\n";
	} else if (OutRestricts.locations.at(0).second < 1) {
		fs << "\n" << "#out size less one why doing this?" << "\n";
		unsigned count = 0;
		char buffer1[5], buffer2[5], buffer3[5], buffer4[5];
		for (unsigned i = 0; i < MaxTS; i++) {
			fs << "s.t. OUT" << Util::itoa(count, buffer1, 10) << ": ";
			for (unsigned j = 0; j < OutRestricts.locations.size(); j++) {
				fs << "x["
						<< Util::itoa(OutRestricts.locations.at(j).first, buffer2, 10)
						<< "," << Util::itoa(i, buffer3, 10) << "]+";
			}
			fs << "0 <= " << Util::itoa(OutRestricts.maximumOf, buffer4, 10) << ";"
					<< "\n";
			count++;
		}
	} else {
		for (unsigned i = 0; i < MaxTS; i++) {
			char buffer1[5], buffer2[5], buffer3[5], buffer4[5]; //buffer6[5], buffer7[5], buffer8[5], buffer9[5], buffer10[5];

			fs << "s.t. Out" << Util::itoa(i, buffer1, 10) << ": ";
			for (unsigned j = 0; j < OutRestricts.locations.size(); j++) {
				if (i < OutRestricts.locations.at(j).second) {
					if (j != OutRestricts.locations.size() - 1) {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												OutRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
						} else if (i == 0) {
							fs << "x["
									<< Util::itoa(OutRestricts.locations.at(j).first,
											buffer2, 10) << ",0]+";
						}
					} else {
						if (i != 0) {
							for (int k = i; k > -1; k--) {
								fs << "x["
										<< Util::itoa(
												OutRestricts.locations.at(j).first,
												buffer2, 10) << ","
										<< Util::itoa(k, buffer3, 10) << "]+";
							}
							fs << "0 <= "
									<< Util::itoa(OutRestricts.maximumOf, buffer4, 10)
									<< ";" << "\n";
						} else if (i == 0) {
							fs << "x["
									<< Util::itoa(OutRestricts.locations.at(j).first,
											buffer2, 10) << ",0]+";
							fs << "0 <= "
									<< Util::itoa(OutRestricts.maximumOf, buffer4, 10)
									<< ";" << "\n";
						}
					}
				} else if (i >= OutRestricts.locations.at(j).second) {
					if (j != OutRestricts.locations.size() - 1) {
						for (unsigned k = i;
								k > i - OutRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(OutRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
					} else {
						for (unsigned k = i;
								k > i - OutRestricts.locations.at(j).second;
								k--) {
							fs << "x["
									<< Util::itoa(OutRestricts.locations.at(j).first,
											buffer2, 10) << ","
									<< Util::itoa(k, buffer3, 10) << "]+";
						}
						fs << "0 <= " << Util::itoa(OutRestricts.maximumOf, buffer4, 10)
								<< ";" << "\n";
					}
				}
			}
		}
	}
}

//sets store restricts in .mod file. Remove call from schedule to remove storage constraints
void ILPScheduler::setStoreRestrictsUpd(DmfbArch * arch, DAG * dag,
		unsigned MaxTS, vector<DelayVal> Delays,
		vector<DependencyConstr> Childs, ofstream &fs) {
	int NumChild = 0;
	int ChildCount = 0;
	for (unsigned t = 0; t < MaxTS; t++) //for each timestpe
			{
		char buff[5];
		fs << "s.t. STORE" << Util::itoa(t, buff, 10) << ": ns" << Util::itoa(t, buff, 10)
				<< " = "
						"(";
		for (unsigned i = 0; i < Childs.size(); i++) //for each child
				{
			NumChild = Childs.at(i).parentLocation.size(); //The number of children to the current parent (change name)
			while (ChildCount != NumChild) {
				int delayNodeLoc = Childs.at(i).nodeLocation; // represents the parent node index in dag (Change name of CHilds)

				fs << " (";

				char buffer[5], buffer1[5], buffer2[5], buffer3[5]; //buffers for Util::itoa conversion
				unsigned Jdelay = 0;

				fs << " (";

				if (Delays.at(delayNodeLoc).delay >= t) //if delay at parent loc is greater then current ts, then we use t directly
						{
					Jdelay = t;
				} //end if
				else //otherwise, make sure we don't go out of time boundary
				{
					Jdelay = t - Delays.at(delayNodeLoc).delay;
				} // end else

				for (unsigned j = 0; j <= Jdelay; j++) //TERM1: for (j = 0 up to  J - parents delay)
						{
					fs << "x[" << Util::itoa(Childs.at(i).nodeLocation + 1, buffer, 10)
							<< "," << Util::itoa(j, buffer1, 10) << "] + "; //nodeLocation == parentLocationation, plus one accounts for  index from 0, index from 1 mismatch in lpsolve
				} // end for

				fs << "0) - (";
				for (unsigned k = 0; k <= t; k++) //for dependent child, go from time 0 to current time t
						{
					fs << "x["
							<< Util::itoa(Childs.at(i).parentLocation.at(ChildCount) + 1,
									buffer2, 10) << "," << Util::itoa(k, buffer3, 10)
							<< "] + ";
				} //end for
				fs << "0) ) + ";
				ChildCount = ChildCount + 1;
			}
			ChildCount = 0;
		}
		fs << "0); \n";
	}
	return;
}

////set the variable maxs for .mod for lp_solve//////
void ILPScheduler::setRestVarMaxs(DmfbArch * arch, DAG * dag, unsigned MaxTS,
		unsigned MixMax, unsigned HeatMax, unsigned DetectMax,
		unsigned SplitMax, ofstream &fs) {
	float numSPM = getMaxStoragePerModule();
	float ratio = 1 / numSPM;

	fs << "\n";
	fs << "#Restrict variable Maxs" << "\n";
	for (unsigned i = 0; i < MaxTS; i++) {
		if (dag->mixes.size() != 0) {
			char buffer[5];
			char buffer1[5];
			fs << "s.t. MM" << Util::itoa(i, buffer, 10) << ": nm"
					<< Util::itoa(i, buffer, 10) << "  <= "
					<< Util::itoa(MixMax, buffer1, 10) << ";" << "\n";
		}
		if (dag->detects.size() != 0) {
			char buffer[5];
			char buffer1[5];
			fs << "s.t. MD" << Util::itoa(i, buffer, 10) << ": nd"
					<< Util::itoa(i, buffer, 10) << "  <= "
					<< Util::itoa(DetectMax, buffer1, 10) << ";" << "\n";
		}
		if (dag->heats.size() != 0) {
			char buffer[5];
			char buffer1[5];
			fs << "s.t. MH" << Util::itoa(i, buffer, 10) << ": nh"
					<< Util::itoa(i, buffer, 10) << "  <= "
					<< Util::itoa(HeatMax, buffer1, 10) << ";" << "\n";
		}
		if (dag->splits.size() != 0) {
			char buffer[5];
			char buffer1[5];
			fs << "s.t. MSPLIT" << Util::itoa(i, buffer, 10) << ": nsplit"
					<< Util::itoa(i, buffer, 10) << "  <= "
					<< Util::itoa(SplitMax, buffer1, 10) << ";" << "\n";
		}
		//storage addition
		char buffer1[5], buffer2[5];
		fs << "s.t. MS" << Util::itoa(i, buffer1, 10) << ": (" << convertFloatToString(ratio)
				<< " * ns" << Util::itoa(i, buffer1, 10) << ") <= "
				<< Util::itoa(MixMax, buffer2, 10) << ";" << "\n";
	}
	fs << "\n";
}

//Establish the general restrictions, i.e. #mix < #MixMods //////
void ILPScheduler::setGeneralRestricts(DmfbArch * arch, DAG * dag, unsigned MaxTS,
		unsigned MixMax, ofstream &fs) {
	//This function is designed to collect from the arch/dag the number of modules there are
	//It then writes these to the .mod file for lp_solve.
	float numSPM = getMaxStoragePerModule();
	float ratio = 1 / numSPM;
	fs << " \n";
	fs << "#setting global device constraints" << "\n";
	int mix_size = dag->mixes.size();
	int detect_size = dag->detects.size();
	int split_size = dag->splits.size();
	int heat_size = dag->heats.size();

	for (unsigned i = 0; i < MaxTS; i++) {
		char buffer[5];
		char buffer1[5];
		fs << "s.t. GEN" << Util::itoa(i, buffer, 10) << ": ";
		if (mix_size != 0) {
			fs << "nm" << Util::itoa(i, buffer, 10) << " + ";
		}
		if (detect_size != 0) {
			fs << "nd" << Util::itoa(i, buffer, 10) << " + ";
		}
		if (heat_size != 0) {
			fs << "nh" << Util::itoa(i, buffer, 10) << " + ";
		}
		if (split_size != 0) {
			fs << "nsplit" << Util::itoa(i, buffer, 10) << " + ";
		}

		//storage addition
		fs << "(" << convertFloatToString(ratio) << " * ns" << Util::itoa(i, buffer, 10) << ") + ";
		fs << "0 <= " << Util::itoa(MixMax, buffer1, 10) << ";" << "\n";
	}
	fs << "\n";
}

//////set the objective of the ILP solution /////////////
void ILPScheduler::setObjective(unsigned NumOps, ofstream &fs) {
	fs << "\n";
	fs << "#objective constraints" << "\n";
	fs << "var c;" << "\n";
	fs << "minimize t: c;" << "\n";

	for (unsigned i = 1; i < NumOps + 1; i++) //this is the new way, using end times
			{
		char buffer[5];
		fs << "s.t. e" << Util::itoa(i, buffer, 10) << ": c >= end"
				<< Util::itoa(i, buffer, 10) << ";" << "\n";
	}
	fs << "\n";
	fs << "solve;" << "\n";
	fs << "end;" << "\n";
}

///// Basics of scheduling, set the timing for the dag //////////
void ILPScheduler::setTiming(DmfbArch * arch, DAG * dag, vector<DelayVal> delays,
		vector<double> StartTimes, int objective_val) {
	for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		dag->allNodes.at(i)->startTimeStep = StartTimes.at(i);
		dag->allNodes.at(i)->endTimeStep = StartTimes.at(i)
				+ delays.at(i).delay;
		dag->allNodes.at(i)->status = SCHEDULED;
		dag->allNodes.at(i)->cycles = (dag->allNodes.at(i)->GetEndTS()
				- dag->allNodes.at(i)->GetStartTS())
				* (arch->getFreqInHz() * arch->getSecPerTS());
	}
}

///////For each operation scheduled, bind it to a resource ///////////
void ILPScheduler::bindResources(DmfbArch * arch, DAG * dag,
		vector<DelayVal> delays, vector<double> StartTimes, int objective_val) {
	getAvailResources(arch);
	commissionDAG(dag);
	unsigned schedTS = 0;

	vector<AssayNode*>::iterator it = dag->allNodes.begin();
	vector<AssayNode*>::iterator it2 = dag->allNodes.begin();
	while ((int)schedTS <= objective_val) {
		it = dag->allNodes.begin();
		for (; it != dag->allNodes.end(); it++) {
			AssayNode *node = *it;
			if (node->GetEndTS() == schedTS) {
				if (node->boundedResType == BASIC_RES) {
					availRes[BASIC_RES]++;
				} else if (node->boundedResType == D_RES) {
					availRes[D_RES]++;
				} else if (node->boundedResType == H_RES) {
					availRes[H_RES]++;
				}
				else if (node->boundedResType == DH_RES) {
					availRes[DH_RES]++;
				}
			}
		}
		it = dag->allNodes.begin();
		for (; it != dag->allNodes.end(); it++) {
			AssayNode *n = *it;
			if (n->GetStartTS() == schedTS) {
				if ((n->type == SPLIT || n->type == MIX)
						&& (availRes[BASIC_RES] + availRes[D_RES]
								+ availRes[H_RES] + availRes[DH_RES]) > 0) {
					//std::cout<<"assay node being bound in SPLIT is: "<<n->name
					//<<", of type: "<<n->type<<"with status: "<<n->status<<flush<<endl; //KNLO Debug
					if (availRes[BASIC_RES] > 0) {
						availRes[BASIC_RES]--;
						n->boundedResType = BASIC_RES;
					} else if (availRes[H_RES] > 0) {
						availRes[H_RES]--;
						n->boundedResType = H_RES;
					} else if (availRes[D_RES] > 0) {
						availRes[D_RES]--;
						n->boundedResType = D_RES;
					} else if (availRes[DH_RES] >0) {
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					} else {
						//cout<<"No available res in SPLIT MIX branch"<<flush<<endl; //KNLO Debug
					}
				} else if (n->type == DETECT
						&& (availRes[D_RES] + availRes[DH_RES]) > 0) {
					//std::cout<<"assay node being bound in DETECT is: "<<n->name
					//<<", of type: "<<n->type<<"with status: "<<n->status<<flush<<endl; //KNLO Debug
					if (availRes[D_RES] > 0) {
						availRes[D_RES]--;
						n->boundedResType = D_RES;
					} else if (availRes[DH_RES] > 0) {
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					} else {
						//cout<<"No available res in DETECT branch"<<flush<<endl; //KNLO Debug
					}
				} else if (n->type == HEAT
						&& (availRes[H_RES] + availRes[DH_RES]) > 0) {
					//std::cout<<"assay node being bound in HEAT is: "<<n->name
					//<<", of type: "<<n->type<<"with status: "<<n->status<<flush<<endl; //KNLO Debug
					if (availRes[H_RES] > 0) {
						availRes[H_RES]--;
						n->boundedResType = H_RES;
					} else if (availRes[DH_RES] > 0) {
						availRes[DH_RES]--;
						n->boundedResType = DH_RES;
					} else {
						//cout<<"No available res in HEAT branch"<<flush<<endl; //KNLO Debug
					}
				} else if (n->type == OUTPUT){
					//std::cout<<"assay node being bound in OUTPUT is: "<<n->name
					//<<", of type: "<<n->type<<"with status: "<<n->status<<", and res type" <<n->boundedResType<<flush<<endl; //KNLO Debug
					//std::cout<<"output detected in bind resources; node: "<<n->name<<flush<<endl; //KNLO Debug
				}
				else{
					//std::cout<<"assay node being bound in ELSE is: "<<n->name
					//<<", of type: "<<n->type<<"with status: "<<n->status<<", and res type" << n->boundedResType<<flush<<endl; //KNLO Debug
					//std::cout<<"unhandled typed detected in bind resources; node: "<<n->name
					//<<"with type: "<<n->type<<flush<<endl; //KNLO Debug
				}
			}
		}
		schedTS = schedTS + 1;
	}

	/*for (unsigned i = 0; i < dag->allNodes.size(); i++) {
		if (dag->allNodes.at(i)->status != SCHEDULED) {
			cout << "node at i = " << i << "not scheduled. Node = ";
			dag->allNodes.at(i)->Print();
			cout << endl;
		}
	} */ //KNLO Debug

}

////////Ready the Dispense wells for input scheduling//////////
IoResource * ILPScheduler::getReadyDispenseWell(string fluidName,
		unsigned long long schedTS) {
	for (unsigned i = 0; i < dispRes->size(); i++) {
		IoResource *dr = dispRes->at(i);
		if (strcmp(Util::StringToUpper(fluidName).c_str(),
				Util::StringToUpper(dr->name).c_str()) == 0) {
			if (dr->lastEndTS + dr->durationInTS <= schedTS)
				return dr;
		}
	}
	return NULL;
}

////////Insert the storage nodes!//////////
int ILPScheduler::storageNodeInsertion(DmfbArch * arch, DAG * dag,
		vector<DelayVal> delays, vector<double> StartTimes) {

	int dropsInStorage = 0;
	vector<AssayNode*> temp = dag->allNodes;
	vector<AssayNode*>::iterator it = temp.begin();
	for (; it != temp.end(); it++) {
		vector<AssayNode*> pInsert;
		vector<AssayNode*> sInsert;
		AssayNode *n = *it;
		if (n->GetType() == DISPENSE) {
			it++;
			n = *it;
		}

		AssayNode *parentPush;
		int scount = 0;
		for (unsigned p = 0; p < n->GetParents().size(); p++) {
			AssayNode *parent = n->GetParents().at(p);
			unsigned long long tempTS = parent->endTimeStep;
			//std::cout<<"parent is: "<<parent->name<<", for node: "<<n->name<<flush<<endl; //KNLO Debug

			if (tempTS < ((n->startTimeStep))) {
				parentPush = n->GetParents().at(p);
				AssayNode *store = dag->AddStorageNode();
				//std::cout<<"storage node added, n = "<<parentPush->name<<flush<<endl; //KNLO Debug
				store->status = SCHEDULED;
				store->startTimeStep = tempTS; //MinPTS;
				store->endTimeStep = n->startTimeStep;
				//store->boundedResType = D_RES;
				if (availRes[BASIC_RES] > 0)
				{
					availRes[BASIC_RES]--;
					store->boundedResType = BASIC_RES;
				}
				else if (availRes[H_RES] > 0)
				{
					availRes[H_RES]--;
					store->boundedResType = H_RES;
				}
				else if (availRes[D_RES] > 0)
				{
					availRes[D_RES]--;
					store->boundedResType = D_RES;
				}
				else
				{
					availRes[DH_RES]--;
					store->boundedResType = DH_RES;
				}
				pInsert.push_back(parentPush); // Insert later so we don't mess up for loop
				sInsert.push_back(store);
				scount = scount + 1;

				/*
				 * 		while (dis > 0)
		{
			//AssayNode *node = dag->AddStorageHolderNode();
			//node->startTimeStep = schedTS;
			//node->endTimeStep = schedTS + 1;
			//node->cycles = (node->GetEndTS()-node->GetStartTS())* (arch->getFreqInHz() * arch->getSecPerTS());
			if (dis >= getMaxStoragePerModule())
				dis -= getMaxStoragePerModule();
			else
				dis -= dis;

			// Reserve a resource type, but don't need to assign here (will do in placer/binder)
			if (ar[BASIC_RES] > 0)
			{
				ar[BASIC_RES]--;
				//node->boundedResType = BASIC_RES;
			}
			else if (ar[H_RES] > 0)
			{
				ar[H_RES]--;
				//node->boundedResType = H_RES;
			}
			else if (ar[D_RES] > 0)
			{
				ar[D_RES]--;
				//node->boundedResType = D_RES;
			}
			else
			{
				ar[DH_RES]--;
				//node->boundedResType = DH_RES;
			}
		}
				 *
				 */
			}

		}

		// Now do actual insert of any necessary storage nodes
		for (unsigned s = 0; s < pInsert.size(); s++) {
			dag->InsertNode(pInsert.at(s), n, sInsert.at(s));
			char buffer[5];
			string stringer = "STORE";
			string tempor = Util::itoa(dropsInStorage, buffer, 10);
			stringer.append(tempor);
			sInsert.at(s)->name = stringer;
			dropsInStorage++;
		}
	}
	return dropsInStorage;
}

////FIX THIS: ADD parameter = nodes in storage
void ILPScheduler::storageNodeBinding(DmfbArch * arch, DAG * dag,
		vector<DelayVal> delays, vector<double> StartTimes, int objective_val,
		int dropsInStorage) {

	int schedTS = 0;
	// Create nodes for storage
	while (schedTS <= objective_val) {
		int ar[RES_TYPE_MAX + 1];
		for (int i = 0; i <= RES_TYPE_MAX; i++)
			ar[i] = availRes[i];

		for (unsigned i = 0; i < dag->storage.size(); i++) {
			if (dag->storage.at(i)->startTimeStep == (unsigned)schedTS) {
				AssayNode *node = dag->AddStorageHolderNode();
				node->startTimeStep = dag->storage.at(i)->startTimeStep;
				node->endTimeStep = dag->storage.at(i)->endTimeStep;
				node->cycles = (node->GetEndTS() - node->GetStartTS())
						* (arch->getFreqInHz() * arch->getSecPerTS());

				// Assign resource type
				if (ar[BASIC_RES] > 0) {
					ar[BASIC_RES]--;
					node->boundedResType = BASIC_RES;
				} else if (ar[H_RES] > 0) {
					ar[H_RES]--;
					node->boundedResType = H_RES;
				} else if (ar[D_RES] > 0) {
					ar[D_RES]--;
					node->boundedResType = D_RES;
				} else {
					ar[DH_RES]--;
					node->boundedResType = DH_RES;
				}
			}
		}
		schedTS = schedTS + 1;
	}
}
unsigned long long ILPScheduler::schedule(DmfbArch *arch, DAG *dag) {
	////Get the current path(OS independent) for filename setting/////
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
		return errno;
	}
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; //not necessary

	string str;
	//str = "C:\\Users\\Kenneth O'Neal\\Documents\\Proj2\\Microfluidics\\MFSIMMER_BACKUP\\MFSIMMER_RunWORKS\\MFSimStatic\\";
	str = cCurrentPath;
	str = str + "\\TestOutput";
	str = str + "\\" + dag->getName();
	str = str + ".mod";

	char * fname = new char[str.size() + 1];
	copy(str.begin(), str.end(), fname);
	fname[str.size()] = '\0';
	std::ofstream fs(fname);

	dagReinitialize(dag);

	pair<vector<unsigned>, unsigned> ModuleInfo = getMaxs(arch, dag);

	vector<unsigned> mod_info = ModuleInfo.first;
	unsigned MaxTS = ModuleInfo.second; //upper bound on time constrain (set by MLS)
	unsigned NumDetect = mod_info[1]; //number of detect reservoirs available on board
	unsigned NumHeat = mod_info[2]; //number of heat reservoirs available on board
	unsigned NumDetectHeat = mod_info[3]; //number of Detect/Heat Reservoirs available on board
	unsigned NumModules = mod_info[0]; //NM is number of general modules on board
	//unsigned NumOutput = mod_info[4]; //Only used when output restrictions accounted for

	//Determine dependency and parent/childs
	vector<DependencyConstr> DepCons2;
	vector<DependencyConstr> Childs;
	DepCons2 = getParents(arch, dag);
	Childs = getChildren(arch, dag);

	//Get the input values for constraint building///
	vector<MaxInput> temp = getInputs(arch, dag);

	//Establish the max inputs for constraints//
	vector<pair<string, unsigned> > pmet = getMaxInput(arch, dag);

	////Establish the mix values for constraints////
	RestrictionVals MixRestricts = getNumMix(arch, dag, ModuleInfo);
	RestrictionVals DetRestricts = getNumDetect(arch, dag, ModuleInfo);
	RestrictionVals HeatRestricts = getNumHeat(arch, dag, ModuleInfo);
	RestrictionVals SplitRestricts = getNumSplit(arch, dag, ModuleInfo);
	RestrictionVals OutRestricts = getNumOutput(arch, dag, ModuleInfo);
	pair<unsigned, unsigned> VarVals = getVarVals(arch, dag, MaxTS);

	vector<DelayVal> delays = getDelays(arch, dag, MixRestricts, DetRestricts,
			HeatRestricts, SplitRestricts, OutRestricts, temp);

	setVarVals(arch, dag, VarVals, delays, Childs, fs);

	setDependent(arch, dag, DepCons2, delays, fs);

	setInputRestrict(arch, dag, temp, MaxTS, pmet, fs);

	if (MixRestricts.numberOf != 0) {
		setMixRestrict(arch, dag, MixRestricts, MaxTS, fs);
	}

	if (DetRestricts.numberOf != 0) {
		setDetectRestrict(arch, dag, DetRestricts, MaxTS, fs);
	}

	if (HeatRestricts.numberOf != 0) {
		setHeatRestrict(arch, dag, HeatRestricts, MaxTS, fs);
	}

	if (SplitRestricts.numberOf != 0) {
		setSplitRestrict(arch, dag, SplitRestricts, MaxTS, fs);
	}

	setStoreRestrictsUpd(arch, dag, MaxTS, delays, Childs, fs);

	setRestVarMaxs(arch, dag, MaxTS, NumModules, NumHeat,
			(NumDetect + NumDetectHeat), NumModules, fs);

	ILPScheduler::setGeneralRestricts(arch, dag, MaxTS, NumModules, fs);

	setObjective(dag->allNodes.size(), fs);

	fs.close();

	//Initialize values
	lprec * lp;

	//Explicit linking of lpsolve library
	//prototype functions defined and set to address
	#if defined(__WIN32) || defined(WINDOWS)
		HINSTANCE lpsolve;
		delete_lp_func * _delete_lp;
		read_XLI_func * _read_XLI;
		set_timeout_func * _set_timeout;
		get_timeout_func * _get_timeout;
		get_Ncolumns_func * _get_Ncolumns;
		solve_func * _solve;
		get_objective_func * _get_objective;
		get_variables_func * _get_variables;
		get_col_name_func * _get_col_name;

		lpsolve = LoadLibrary("lpsolve55.dll");

		if (lpsolve == NULL) {
			cerr << "Unable to load lpsolve shared library\n";
			exit(1);
		}

		_delete_lp = (delete_lp_func *) GetProcAddress(lpsolve, "delete_lp");
		_read_XLI = (read_XLI_func *) GetProcAddress(lpsolve, "read_XLI");
		_set_timeout = (set_timeout_func *) GetProcAddress(lpsolve, "set_timeout");
		_get_timeout = (get_timeout_func *) GetProcAddress(lpsolve, "get_timeout");
		_get_Ncolumns = (get_Ncolumns_func *) GetProcAddress(lpsolve, "get_Ncolumns");
		_solve = (solve_func *) GetProcAddress(lpsolve, "solve");
		_get_objective = (get_objective_func *) GetProcAddress(lpsolve, "get_objective");
		_get_variables = (get_variables_func *) GetProcAddress(lpsolve, "get_variables");
		_get_col_name = (get_col_name_func *) GetProcAddress(lpsolve, "get_col_name");
	// end explicit linking, use proto functions to point to functions in lp_lib
	#else
		void * lpsolve;
		delete_lp_func * _delete_lp;
		read_XLI_func * _read_XLI;
		set_timeout_func * _set_timeout;
		get_timeout_func * _get_timeout;
		get_Ncolumns_func * _get_Ncolumns;
		solve_func * _solve;
		get_objective_func * _get_objective;
		get_variables_func * _get_variables;
		get_col_name_func * _get_col_name;

		lpsolve = dlopen("lpsolve55.dll", RTLD_LAZY);

		if (lpsolve == NULL) {
			cerr << "Unable to load lpsolve shared library\n";
			exit(1);
		}

		_delete_lp = (delete_lp_func *) dlsym(lpsolve, "delete_lp");
		_read_XLI = (read_XLI_func *) dlsym(lpsolve, "read_XLI");
		_set_timeout = (set_timeout_func *) dlsym(lpsolve, "set_timeout");
		_get_timeout = (get_timeout_func *) dlsym(lpsolve, "get_timeout");
		_get_Ncolumns = (get_Ncolumns_func *) dlsym(lpsolve, "get_Ncolumns");
		_solve = (solve_func *) dlsym(lpsolve, "solve");
		_get_objective = (get_objective_func *) dlsym(lpsolve, "get_objective");
		_get_variables = (get_variables_func *) dlsym(lpsolve, "get_variables");
		_get_col_name = (get_col_name_func *) dlsym(lpsolve, "get_col_name");
	#endif


	//Begin launching lp_solver//
	//associate solver with mathprog language interface
	lp = _read_XLI("xli_MathProg", fname, NULL, "", IMPORTANT);
	if (lp == NULL) {
		fprintf(stderr, "Unable to read model\n");
		return (1);
	}

	delete fname;

	int timeoutVal = 14400;
	_set_timeout(lp, timeoutVal); //sets timeout of model to 14400S (4 hours).
	cerr<<"the timeout of lp is: "<<_get_timeout(lp)<<" seconds"<<std::flush<<std::endl;

	///////Solving the model//////////
	double Nvar[_get_Ncolumns(lp)];

	(lp, PRESOLVE_ROWS + PRESOLVE_COLS + PRESOLVE_LINDEP, 0);

	unsigned temp_solve = _solve(lp);


	if (temp_solve == 0) {
		cout << "successful OPTIMAL solve of LPSolve Model" << endl;
		//cerr << "successful OPTIMAL solve of LPSolve Model" << endl; //KNLO Debug
	} else if (temp_solve == 1) {
		cout << "Feasible solution found" << flush << endl;
		//cerr << "Feasible solution found" << flush << endl; //KNLO Debug
	} else {
		cout << "TEMP SOLVE: " << temp_solve
				<< ", failure to solve LPSolve Model with optimal or feasible solution"
				<< endl;
		//cerr << "TEMP SOLVE: " << temp_solve
				//<< ", failure to solve LPSolve Model with optimal or feasible solution"
				//<< endl; //KNLO Debug
		exit(1);
	}


	double scheduleTime = _get_objective(lp);
	//getting the variables of the model, will be used to perform framework details
	_get_variables(lp, Nvar);





	// establish start times for set timing
	vector<double> StartTimes; //operations start times vector
	int startIndex = 0;

	for (int i = 0; i < _get_Ncolumns(lp); i++) {
		if(string(_get_col_name(lp, i+1)) == "s1")

	{
		startIndex = i;
		break;
	}
	}

	for(unsigned i = startIndex; i < startIndex + dag->allNodes.size(); i++)
	{
		StartTimes.push_back(Nvar[i]);
	}

	if (false) { //KNLO DEBUG
		for (int i = startIndex; i < get_Ncolumns(lp); i++) {
			cout << "Schedule: StartTimesCol post solve i = " << i << ", "
					<< get_col_name(lp, i + 1) << " : " << Nvar[i] << endl;
		}
	}

	setTiming(arch, dag, delays, StartTimes, scheduleTime + 1);

	//TODO:: currently storageNodeInsertion will cause proper placement, but segfault in routing
	//TODO:: No storage node insertion causes improper binding in placement
	storageNodeInsertion(arch, dag, delays, StartTimes);

	bindResources(arch, dag, delays, StartTimes, scheduleTime + 1);
	cout<<" LPSOLVE TS:  "<<scheduleTime<<flush<<endl;
	//cerr<<"LPSOLVE TS: "<<scheduleTime<<flush<<endl; //KNLO Debug

	_delete_lp(lp);

	//dag->PrintSchedule(); //KNLO Debug

	return 0;
}

