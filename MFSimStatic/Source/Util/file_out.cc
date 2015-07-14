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
 * Source: file_out.cc															*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 17, 2013							*
 *																				*
 * Details: N/A																	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/
#include "../../Headers/Util/file_out.h"
#include "../../Headers/Util/util.h"

///////////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////////
FileOut::FileOut() {}

///////////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////////
FileOut::~FileOut(){}

////////////////////////////////////////////////////////////////////////////
// Writes the DAG to a text file so it can be read and processed by any
// scheduler later.
////////////////////////////////////////////////////////////////////////////
void FileOut::WriteDagToFile(DAG *dag, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write DAG to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	os << "// DAG Specification for " << dag->getName() << endl;
	if (!dag->getName().empty())
		os << "DAGNAME (" << dag->getName() << ")" << endl;
	else
		os << "DAGNAME (DAG)" << endl;

	for (unsigned i = 0; i < dag->getAllNodes().size(); i++)
	{
		AssayNode *node = dag->getAllNodes().at(i);
		os << "NODE (";

		if (node->GetType() == DISPENSE) // Id, Type, FluidName, Volume, Name
			os << node->id << ", DISPENSE, "  << node->portName <<  ", " << node->volume << ", " << node->name << ")\n";
		else if (node->GetType() == MIX) // Id, Type, NumDropsBefore, Time (s), Name
			os << node->id << ", MIX, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ")\n";
		else if (node->GetType() == DILUTE) // Id, Type, NumDropsBefore, Time (s), Name
			os << node->id << ", DILUTE, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ")\n";
		else if (node->GetType() == SPLIT) // Id, Type, NumDropsAfter, Time (s), Name
			os << node->id << ", SPLIT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ")\n";
		else if (node->GetType() == HEAT)// Id, Type, Time (s), Name
			os << node->id << ", HEAT, " << node->seconds << ", " << node->name << ")\n";
		else if (node->GetType() == DETECT) // Id, Type, NumDropsIo, Time (s), Name
			os << node->id << ", DETECT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ")\n";
		else if (node->GetType() == OUTPUT) // Id, Type, SinkName, Name
			os << node->id << ", OUTPUT, " << node->portName << ", " << node->name << ")\n";
		else if (node->GetType() == STORAGE) // Id, Type, Name
			os << node->id << ", STORAGE, " << node->name << ")\n";
		else // Id, Type, Name
			os << node->id << ", GENERAL, " << node->name << ")\n";

		for (unsigned c = 0; c < node->children.size(); c++)
			os << "EDGE (" << node->id << ", " << node->children.at(c)->id << ")\n";
	}
	os.close();
}

////////////////////////////////////////////////////////////////////////////
// Writes the DAG to a text file so it can be read and processed by any
// scheduler later.
////////////////////////////////////////////////////////////////////////////
void FileOut::WriteScheduledDagAndArchToFile(DAG *dag, DmfbArch *arch, Scheduler *scheduler, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Scheduled DAG/Arch to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DAG *d = dag;
	DmfbArch *a = arch;

	// First output the architecture
	os << "// ARCHITECTURE Specification" << endl;
	os << "ARCHNAME (" << a->getName() << ")" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		os << "EXTERNAL (" << (int)fm->resourceType << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		/*if (fm->resourceType == D_RES)
			os << "EXTERNAL (DETECT, " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNAL (HEAT, " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");*/
	}
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << "NORTH, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << "SOUTH, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << "EAST, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << "WEST, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}
	os << "FREQ (" << a->freqInHz << ")" << endl;
	os << "TIMESTEP (" << a->secPerTS << ")" << endl << endl;

	os << "SCHEDTYPE (" << FileOut::GetKeyFromSchedType(scheduler->getType()) << ")" << endl;
	os << "PINMAPTYPE (" << FileOut::GetKeyFromPinMapType(arch->getPinMapper()->getType()) << ")" << endl;
	os << "RESOURCEALLOCATIONTYPE (" << FileOut::GetKeyFromResourceAllocationType(arch->getPinMapper()->getResAllocType()) << ")" << endl;
	os << "MAXSTORAGEDROPSPERMOD (" << scheduler->getMaxStoragePerModule() << ")" << endl << endl;

	os << "PINMAP (";
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			os << arch->getPinMapper()->getPinMapping()->at(x)->at(y);
			if (!(x == arch->getNumCellsX()-1 && y == arch->getNumCellsY()-1))
				os << ", ";
		}
	}
	os << ")" << endl;

	arch->getPinMapper()->flattenSpecialPurposePins();
	if (arch->getPinMapper()->getSpecialPurposePins()->size() > 0)
	{
		os << "SPECIALPINS (" << arch->getPinMapper()->getSpecialPurposePins()->at(0);
		for (unsigned i = 1; i < arch->getPinMapper()->getSpecialPurposePins()->size(); i++)
			os << ", " << arch->getPinMapper()->getSpecialPurposePins()->at(i);
		os << ")" << endl;
	}

	os << "RESOURCECOUNT (" << arch->getPinMapper()->getAvailResCount()->at(0);
	for (int i = 1; i <= RES_TYPE_MAX; i++)
		os << ", " << arch->getPinMapper()->getAvailResCount()->at(i);
	os << ")" << endl;

	os << endl << "// Locations of the resources listed above (if any)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;


	// Now output scheduled DAG with resource type
	os << endl << "// DAG Specification for " << d->getName() << endl;
	if (!d->getName().empty())
		os << "DAGNAME (" << d->getName() << ")" << endl;
	else
		os << "DAGNAME (DAG)" << endl;

	for (unsigned i = 0; i < d->getAllNodes().size(); i++)
	{
		AssayNode *node = d->getAllNodes().at(i);

		os << "NODE (";

		if (node->GetType() == DISPENSE) // Id, Type, FluidName, Volume, Name, StartTS, EndTS
			os << node->id << ", DISPENSE, " << node->portName <<  ", " << node->volume << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ")\n";
		else if (node->GetType() == MIX) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ResourceType
			os << node->id << ", MIX, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else if (node->GetType() == DILUTE) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ResourceType
			os << node->id << ", DILUTE, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else if (node->GetType() == SPLIT) // Id, Type, NumDropsAfter, Time (s), Name, StartTS, EndTS, ResourceType
			os << node->id << ", SPLIT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else if (node->GetType() == HEAT)// Id, Type, Time (s), Name, StartTS, EndTS, ResourceType
			os << node->id << ", HEAT, " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else if (node->GetType() == DETECT) // Id, Type, NumDropsIo, Time (s), Name, StartTS, EndTS, ResourceType
			os << node->id << ", DETECT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else if (node->GetType() == OUTPUT) // Id, Type, SinkName, Name, StartTS, EndTS
			os << node->id << ", OUTPUT, " << node->portName << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ")\n";
		else if (node->GetType() == STORAGE) // Id, Type, Name, StartTS, EndTS, ResourceType
			os << node->id << ", STORAGE, " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else // Id, Type, Name, StartTS, EndTS, ResourceType
			os << node->id << ", GENERAL, " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";

		for (unsigned c = 0; c < node->children.size(); c++)
			os << "EDGE (" << node->id << ", " << node->children.at(c)->id << ")\n";
	}

	// Now output scheduled STORAGE_HOLDER nodes with resource type
	for (unsigned i = 0; i < d->getAllStorageHolders().size(); i++)
	{
		AssayNode *node = d->getAllStorageHolders().at(i);

		os << "NODE (";

		if (node->GetType() == STORAGE_HOLDER) // Type, Id, StartTS, EndTS, ResourceType
			os << node->id << ", STORAGE_HOLDER, " << node->startTimeStep << ", " << node->endTimeStep << ", " << (int)node->boundedResType << ")\n";
		else
			claim(false, "Invalid type for STORAGE_HOLDER node.");

		//for (int c = 0; c < node->children.size(); c++)
		//	os << "EDGE (" << node->id << ", " << node->children.at(c)->id << ")\n";
	}
	os.close();
}

////////////////////////////////////////////////////////////////////////////
// Writes the DAG to a text file so it can be read and processed by any
// router later.
////////////////////////////////////////////////////////////////////////////
void FileOut::WritePlacedDagAndArchToFile(DAG *dag, DmfbArch *arch, Placer *placer, vector<ReconfigModule *> *rModules, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Placed DAG/Arch to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DAG *d = dag;
	DmfbArch *a = arch;

	// First output the architecture
	os << "// ARCHITECTURE Specification" << endl;
	os << "ARCHNAME (" << a->getName() << ")" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		os << "EXTERNAL (" << (int)fm->resourceType << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
	}
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << iop->id << ", NORTH, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << iop->id << ", SOUTH, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << iop->id << ", EAST, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << iop->id << ", WEST, " << iop->posXY << ", " << iop->seconds << ", " << iop->pinNo << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}
	os << "FREQ (" << a->freqInHz << ")" << endl;
	os << "TIMESTEP (" << a->secPerTS << ")" << endl << endl;

	os << "SCHEDTYPE (" << FileOut::GetKeyFromSchedType(placer->getPastSchedType()) << ")" << endl;
	os << "PINMAPTYPE (" << FileOut::GetKeyFromPinMapType(arch->getPinMapper()->getType()) << ")" << endl;
	os << "PLACERTYPE (" << FileOut::GetKeyFromPlaceType(placer->getType()) << ")" << endl;
	os << "RESOURCEALLOCATIONTYPE (" << FileOut::GetKeyFromResourceAllocationType(arch->getPinMapper()->getResAllocType()) << ")" << endl;
	os << "HCELLSBETWEENMODIR (" << placer->getHCellsBetweenModIR() << ")" << endl;
	os << "vCELLSBETWEENMODIR (" << placer->getVCellsBetweenModIR() << ")" << endl << endl;

	os << "PINMAP (";
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			os << arch->getPinMapper()->getPinMapping()->at(x)->at(y);
			if (!(x == arch->getNumCellsX()-1 && y == arch->getNumCellsY()-1))
				os << ", ";
		}
	}
	os << ")" << endl;

	arch->getPinMapper()->flattenSpecialPurposePins();
	if (arch->getPinMapper()->getSpecialPurposePins()->size() > 0)
	{
		os << "SPECIALPINS (" << arch->getPinMapper()->getSpecialPurposePins()->at(0);
		for (unsigned i = 1; i < arch->getPinMapper()->getSpecialPurposePins()->size(); i++)
			os << ", " << arch->getPinMapper()->getSpecialPurposePins()->at(i);
		os << ")" << endl;
	}

	os << "RESOURCECOUNT (" << arch->getPinMapper()->getAvailResCount()->at(0);
	for (int i = 1; i <= RES_TYPE_MAX; i++)
		os << ", " << arch->getPinMapper()->getAvailResCount()->at(i);
	os << ")" << endl;

	os << endl << "// Locations of the resources listed above (if any)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;


	// Now output the reconfigurable modules
	os << endl << "// Reconfigurable Modules" << endl;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->at(i);

		string opType = "";
		if (rm->boundNode == NULL)
			opType = GetOperationString(GENERAL);
		else
			opType = GetOperationString(rm->boundNode->type);

		os << "RECONFIG (" << rm->id << ", " << opType << ", " << (int)rm->resourceType << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << rm->startTimeStep << ", " << rm->endTimeStep << ", " << rm->tiledNum << ")" << endl;
	}
	os << endl;

	// Now output scheduled DAG with resource bindings to specific IoPorts or Reconfigurable Modules
	os << "// DAG Specification for " << d->getName() << endl;
	if (!d->getName().empty())
		os << "DAGNAME (" << d->getName() << ")" << endl;
	else
		os << "DAGNAME (DAG)" << endl;

	for (unsigned i = 0; i < d->getAllNodes().size(); i++)
	{
		AssayNode *node = d->getAllNodes().at(i);

		string resType;
		if (node->boundedResType == BASIC_RES)
			resType = "B";
		else if (node->boundedResType == D_RES)
			resType = "D";
		else if (node->boundedResType == H_RES)
			resType = "H";
		else if (node->boundedResType == DH_RES)
			resType = "DH";

		os << "NODE (";

		if (node->GetType() == DISPENSE) // Id, Type, FluidName, Volume, Name, StartTS, EndTS, IoPortId
			os << node->id << ", DISPENSE, " << node->portName <<  ", " << node->volume << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->ioPort->id << ")\n";
		else if (node->GetType() == MIX) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", MIX, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else if (node->GetType() == DILUTE) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", DILUTE, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else if (node->GetType() == SPLIT) // Id, Type, NumDropsAfter, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", SPLIT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else if (node->GetType() == HEAT)// Id, Type, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", HEAT, " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else if (node->GetType() == DETECT) // Id, Type, NumDropsIo, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", DETECT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else if (node->GetType() == OUTPUT) // Id, Type, SinkName, Name, StartTS, EndTS, IoPortId
			os << node->id << ", OUTPUT, " << node->portName << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->ioPort->id << ")\n";
		else if (node->GetType() == STORAGE) // Id, Type, StartTS, EndTS, ReconfigModId
			os << node->id << ", STORAGE, " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else // Id, Type, Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", GENERAL, " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";

		for (unsigned c = 0; c < node->children.size(); c++)
			os << "EDGE (" << node->id << ", " << node->children.at(c)->id << ")\n";
	}
	os.close();
}

///////////////////////////////////////////////////////////////
// Writes the DMFB architecture which can be read in for wire
// routing.
///////////////////////////////////////////////////////////////
void FileOut::WriteArchForWireRouting(DmfbArch *arch, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Arch to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DmfbArch *a = arch;

	// First output the architecture
	os << "=======================Initialization=======================" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	os << endl << "// IO Ports from " << a->getName() << endl;
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << iop->id  << ", top, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << iop->id  << ", bottom, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << iop->id  << ", right, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << iop->id  << ", left, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}

	os << endl << "PINMAPTYPE (" << FileOut::GetKeyFromPinMapType(arch->getPinMapper()->getType()) << ")" << endl << endl;

	os << "PINMAP (";
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			os << arch->getPinMapper()->getPinMapping()->at(x)->at(y);
			if (!(x == arch->getNumCellsX()-1 && y == arch->getNumCellsY()-1))
				os << ", ";
		}
	}
	os << ")" << endl;

	os << endl << "// Non-reconfigurable Resources from " << a->getName() << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		if (fm->resourceType == D_RES)
			os << "EXTERNALDETECTOR (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNALHEATER (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");
	}



	os << endl << "// Locations of the resources listed above (if any)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes the final simulation results to an output file that
// can be read by the Java visualizer tools
///////////////////////////////////////////////////////////////
void FileOut::WriteRoutedDagAndArchToFile(DAG *dag, DmfbArch *arch, Router *router, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<vector<RoutePoint*> *> *dirtyCells, vector<vector<int> *> *pinActivations, vector<unsigned long long> *tsBeginningCycle, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Cyclic Routed DAG to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DmfbArch *a = arch;

	// First output the architecture
	os << "=======================Initialization=======================" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	os << endl << "// IO Ports from " << a->getName() << endl;
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << iop->id  << ", top, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << iop->id  << ", bottom, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << iop->id  << ", right, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << iop->id  << ", left, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}

	os << endl << "PINMAPTYPE (" << FileOut::GetKeyFromPinMapType(arch->getPinMapper()->getType()) << ")" << endl << endl;

	os << "PINMAP (";
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			os << arch->getPinMapper()->getPinMapping()->at(x)->at(y);
			if (!(x == arch->getNumCellsX()-1 && y == arch->getNumCellsY()-1))
				os << ", ";
		}
	}
	os << ")" << endl;

	os << endl << "// Non-reconfigurable Resources from " << a->getName() << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		if (fm->resourceType == D_RES)
			os << "EXTERNALDETECTOR (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNALHEATER (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");
	}

	//DTG Debug
	//for (int i = 0; i < 200; i++)
	//	tsBeginningCycle->push_back(12345);


	// Are module occurrance lengths dictated by time-steps or cycles
	os << endl << "// Reconfigurable module delta type (cycles or time-steps)" << endl;
	os << "MODULEDELTATYPE (";
	// So far, only the routing-based synthesis router uses cycles as it's module-length delta
	if (router->getType() == SKYCAL_R)
		os << C_MDT << ")" << endl;
	else
		os << TS_MDT << ")" << endl;

	os << endl << "// Reconfigurable Resources" << endl;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->at(i);
		unsigned long long begTS = rm->startTimeStep;//tsBeginningCycle->at(rm->startTimeStep);
		unsigned long long endTS = rm->endTimeStep;//tsBeginningCycle->at(rm->endTimeStep);
		if (rm->boundNode->type == MIX)
			os << "RECONFIGMIXER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == DILUTE)
			os << "RECONFIGDILUTER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == SPLIT)
			os << "RECONFIGSPLITTER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == HEAT)
			os << "RECONFIGHEATER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == DETECT)
			os << "RECONFIGDETECTOR (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == STORAGE || rm->boundNode->type == STORAGE_HOLDER)
			os << "RECONFIGSTORAGE (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == GENERAL)
			os << "RECONFIGGENERAL (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else
			claim(false, "Unsupported reconfigurable resource type.");
	}

	os << endl << "// Locations of the resources listed above (if any)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;

	os << endl << "// Time-step to cycle conversions" << endl;
	for (unsigned i = 0; i < tsBeginningCycle->size(); i++)
	{
		unsigned long long lastTSEnd = 0;
		if (i > 0)
			lastTSEnd = tsBeginningCycle->at(i-1) + (int)(a->freqInHz*a->secPerTS);

		os << "ROUTETO (" << i << ", " << lastTSEnd << ", " << tsBeginningCycle->at(i) << ")" << endl;
		os << "TSRANGE (" << i << ", " << tsBeginningCycle->at(i) << ", " << tsBeginningCycle->at(i) + (int)(a->freqInHz*a->secPerTS) << ")" << endl;
	}

	os << "=======================Init Done=======================" << endl;

	//map<Droplet *, vector<RoutePoint *> *> *routes
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	unsigned long long firstCycle = 10000000000000;
	unsigned long long lastCycle = 0;
	while (it != routes->end())
	{
		if (!it->second->empty() && it->second->front()->cycle < firstCycle)
			firstCycle = it->second->front()->cycle;
		if (!it->second->empty() && it->second->back()->cycle > lastCycle)
			lastCycle = it->second->back()->cycle;
		it++;
	}

	// Create string stream for each cycle
	vector<stringstream *> cycleStrings;
	for (unsigned i = firstCycle; i <= lastCycle+1; i++)
	{
		stringstream *ss = new stringstream();
		(*ss) << "=======================Commit Cycle " << i << "=======================" << endl;
		cycleStrings.push_back(ss);
	}

	// Add routing points to appropriate cycle
	for (it = routes->begin(); it != routes->end(); it++)
	{
		vector<RoutePoint *> *route = it->second;
		for (unsigned i = 0; i < route->size(); i++)
		{
			RoutePoint *rp = route->at(i);

			/*(*cycleStrings.at(rp->cycle - cycle)) << "Droplet " << it->first->getId() << ", Cell: (" << rp->x << ", " << rp->y << ")";

			if (rp->dStatus == DROP_PROCESSING)
				(*cycleStrings.at(rp->cycle - cycle)) << "--PROCESSING";
			else if (rp->dStatus == DROP_MERGING)
				(*cycleStrings.at(rp->cycle - cycle)) << "--MERGING";
			else if (rp->dStatus == DROP_SPLITTING)
				(*cycleStrings.at(rp->cycle - cycle)) << "--SPLITTING";
			else if (rp->dStatus == DROP_OUTPUT)
				(*cycleStrings.at(rp->cycle - cycle)) << "--OUTPUTTING";
			else if (rp->dStatus == DROP_WAIT)
				(*cycleStrings.at(rp->cycle - cycle)) << "--PROCESS WAIT";
			else if (rp->dStatus == DROP_INT_WAIT)
				(*cycleStrings.at(rp->cycle - cycle)) << "--INTERFERENCE WAIT";
			else if (rp->dStatus == DROP_WASH)
				(*cycleStrings.at(rp->cycle - cycle)) << "--WASHING";
			else if (rp->dStatus == DROP_WASTE)
				(*cycleStrings.at(rp->cycle - cycle)) << "--WASTING";
			(*cycleStrings.at(rp->cycle - cycle)) << endl;*/

			(*cycleStrings.at(rp->cycle - firstCycle)) << "C (";
			if (rp->dStatus == DROP_PROCESSING)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_PROC";
			else if (rp->dStatus == DROP_MERGING)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_MERGE";
			else if (rp->dStatus == DROP_SPLITTING)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_SPLIT";
			else if (rp->dStatus == DROP_OUTPUT)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_OUT";
			else if (rp->dStatus == DROP_WAIT)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_PROC_WAIT";
			else if (rp->dStatus == DROP_INT_WAIT)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_INT_WAIT";
			else if (rp->dStatus == DROP_WASH)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_WASH";
			else if (rp->dStatus == DROP_WASTE)
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_WASTE";
			else
				(*cycleStrings.at(rp->cycle - firstCycle)) << "D_NORM";
			(*cycleStrings.at(rp->cycle - firstCycle)) << ", " << it->first->getId() << ", " << rp->x << ", " << rp->y << ")" << endl;
		}
	}

	for (unsigned i = 0; i < cycleStrings.size(); i++)
	{
		// cycleStrings might not start at the same cycle as dirtyCells
		if (i+firstCycle < dirtyCells->size())
		{
			vector<RoutePoint *> *cells = dirtyCells->at(i+firstCycle);
			if (cells->size() > 0)
			{
				(*cycleStrings.at(i)) << "Dirty (";
				for (unsigned j = 0; j < cells->size(); j++)
				{
					RoutePoint *rp = cells->at(j);
					(*cycleStrings.at(i)) << ((rp->y*arch->getNumCellsX()) + rp->x) << ", " << rp->droplet->getId();
					//cout << ((rp->y*arch->getNumCellsX()) + rp->x) << ", " << rp->droplet->getId();
					if (j != cells->size()-1)
						(*cycleStrings.at(i)) << ", ";
					//(*cycleStrings.at(i-firstCycle)) << "C (";
					//(*cycleStrings.at(i-firstCycle)) << "C_DIRTY";
					//(*cycleStrings.at(i-firstCycle)) << ", " << rp->droplet->getId() << ", " << rp->x << ", " << rp->y << ")" << endl;
				}
				(*cycleStrings.at(i)) << ")" << endl;
			}
			else
				(*cycleStrings.at(i)) << "Dirty (-)" << endl;
		}
		else
			(*cycleStrings.at(i)) << "Dirty (-)" << endl;
	}

	// Equalize lengths of vectors
	for (unsigned i = pinActivations->size(); i < cycleStrings.size()+firstCycle; i++)
		pinActivations->push_back(new vector<int>());

	for (unsigned i = 0; i < cycleStrings.size(); i++)
	{
		if (pinActivations->at(i+firstCycle)->size() > 0)
		{
			(*cycleStrings.at(i)) << "ActivePins (";
			for (unsigned j = 0; j < pinActivations->at(i+firstCycle)->size(); j++)
			{
				(*cycleStrings.at(i)) << pinActivations->at(i+firstCycle)->at(j);
				if (j < pinActivations->at(i+firstCycle)->size()-1)
					(*cycleStrings.at(i)) << ", ";
			}
			(*cycleStrings.at(i)) << ")" << endl;
		}
		else
			(*cycleStrings.at(i)) << "ActivePins (-)" << endl;
	}

	for (unsigned i = firstCycle; i <= lastCycle+1; i++)
		os << cycleStrings.at(i - firstCycle)->str();
		//os << cycleStrings.at(i - firstCycle)->str() << "Number of pins to activate = " << pinActivations->at(i)->size() << endl;
	//os << cycleStrings.at(i - cycle)->str() << "Number of pins to activate = 1" << endl;

	// Cleanup stringstreams
	while (!cycleStrings.empty())
	{
		stringstream *ss = cycleStrings.back();
		cycleStrings.pop_back();
		delete ss;
	}

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes an architecture file that can be input to the
// scheduler as an input file.
///////////////////////////////////////////////////////////////
void FileOut::WriteInputtableDmfbArchToFile(DmfbArch *arch, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write architecture to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DmfbArch *a = arch;

	// Output the architecture
	os << "ARCHNAME (" << a->name << ")" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;

	// External Fixtures
	os << endl << "// External fixtures" << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		if (fm->resourceType == D_RES)
			os << "EXTERNAL (DETECT, " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNAL (HEAT, " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");
	}

	// I/O Ports
	os << endl << "// IO Ports from " << a->getName() << endl;
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << "north, " << iop->posXY << ", " << iop->seconds << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << "south, " << iop->posXY << ", " << iop->seconds << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << "east, " << iop->posXY << ", " << iop->seconds << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << "west, " << iop->posXY << ", " << iop->seconds << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}

	os << endl << "FREQ (" << a->freqInHz << ")" << endl;
	os << "TIMESTEP (" << a->secPerTS << ")" << endl;

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes the hardware description file, along with information
// for wire routing to an output file that can be read by the
// Java visualizer tools
///////////////////////////////////////////////////////////////
void FileOut::WriteHardwareFileWithWireRoutes(DmfbArch *arch, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Hardware Description (with Wire-routes) to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DmfbArch *a = arch;

	// Output the architecture
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	os << endl << "// IO Ports from " << a->getName() << endl;
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << iop->id  << ", top, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << iop->id  << ", bottom, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << iop->id  << ", right, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << iop->id  << ", left, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}


	os << "PINMAP (";
	for (int y = 0; y < arch->getNumCellsY(); y++)
	{
		for (int x = 0; x < arch->getNumCellsX(); x++)
		{
			os << arch->getPinMapper()->getPinMapping()->at(x)->at(y);
			if (!(x == arch->getNumCellsX()-1 && y == arch->getNumCellsY()-1))
				os << ", ";
		}
	}
	os << ")" << endl;

	os << endl << "// Non-reconfigurable Resources from " << a->getName() << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		if (fm->resourceType == D_RES)
			os << "EXTERNALDETECTOR (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNALHEATER (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");
	}


	os << endl << "// Locations of the module locations (if fixed/applicable)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;


	//NUMVTRACKS (3)

	os << endl << "// Wire-segment descriptions" << endl;
	os << "WIREGRIDDIM (" << a->getWireRouter()->getModel()->getWireGridXSize() << ", " << a->getWireRouter()->getModel()->getWireGridYSize() << ")" << endl;
	os << "NUMHTRACKS (" << a->getWireRouter()->getNumHorizTracks() << ")" << endl;
	os << "NUMVTRACKS (" << a->getWireRouter()->getNumVertTracks() << ")" << endl;
	//int tSize = a->getWireRouter()->getModel()->getTileGridSize();
	//os << "WIREGRIDDIM (" << ((a->getNumCellsX()-1)*(tSize-1))+1 << ", " << ((a->getNumCellsY()-1)*(tSize-1))+1 << ")" << endl;
	vector< vector<WireSegment *> *> *wires = a->getWireRouter()->getWireRoutesPerPin();
	for (unsigned i = 0; i < wires->size(); i++)
	{
		vector<WireSegment *> *wire = wires->at(i);
		for (unsigned j = 0; j < wire->size(); j++)
		{
			WireSegment *ws = wire->at(j);

			if (ws->segmentType == LINE_WS)
				os << "RELLINE (";
			else
				claim(false, "Unknown wire-segment type.");

			os << ws->pinNo << ", " << ws->layer << ", " << ws->sourceWireCellX << ", " << ws->sourceWireCellY << ", " << ws->destWireCellX << ", " << ws->destWireCellY << ")" << endl;

		}
	}

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes compacted simulation results to file so that they
// can be displayed by the Java visualization tools.
///////////////////////////////////////////////////////////////
void FileOut::WriteCompactedRoutesToFile(DAG *dag, DmfbArch *arch, vector<ReconfigModule *> *rModules, map<Droplet *, vector<RoutePoint *> *> *routes, vector<unsigned long long> *tsBeginningCycle, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write Compacted Routed DAG to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	DAG *d = dag;
	DmfbArch *a = arch;

	// First output the architecture
	os << "=======================Initialization=======================" << endl;
	os << "DIM (" << a->getNumCellsX() << ", " << a->getNumCellsY() << ")" << endl;
	os << endl << "// IO Ports from " << a->getName() << endl;
	for (unsigned i = 0; i < a->ioPorts->size(); i++)
	{
		IoPort *iop = a->ioPorts->at(i);
		string washStatus = iop->containsWashFluid ? "TRUE" : "FALSE";

		if (iop->isInput)
			os << "INPUT (";
		else
			os << "OUTPUT (";

		if (iop->ioSide == NORTH)
			os << iop->id  << ", top, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == SOUTH)
			os << iop->id  << ", bottom, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == EAST)
			os << iop->id  << ", right, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else if (iop->ioSide == WEST)
			os << iop->id  << ", left, " << iop->posXY << ", " << iop->portName << ", " << washStatus << ")" << endl;
		else
			claim(false, "Unknown IO side direction specified while outputting to file.");
	}

	os << endl << "// Non-reconfigurable Resources from " << a->getName() << endl;
	for (unsigned i = 0; i < a->externalResources->size(); i++)
	{
		FixedModule *fm = a->externalResources->at(i);
		if (fm->resourceType == D_RES)
			os << "EXTERNALDETECTOR (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else if (fm->resourceType == H_RES)
			os << "EXTERNALHEATER (" << fm->id << ", " << fm->leftX << ", " << fm->topY << ", " << fm->rightX << ", " << fm->bottomY << ")" << endl;
		else
			claim(false, "Unknown EXTERNAL module type being ouput to file.");
	}

	os << endl << "// Reconfigurable Resources" << endl;
	for (unsigned i = 0; i < rModules->size(); i++)
	{
		ReconfigModule *rm = rModules->at(i);
		unsigned long long begTS = rm->startTimeStep;
		unsigned long long endTS = rm->endTimeStep;
		if (rm->boundNode->type == MIX)
			os << "RECONFIGMIXER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == DILUTE)
			os << "RECONFIGDILUTER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == SPLIT)
			os << "RECONFIGSPLITTER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == HEAT)
			os << "RECONFIGHEATER (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == DETECT)
			os << "RECONFIGDETECTOR (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == STORAGE || rm->boundNode->type == STORAGE_HOLDER)
			os << "RECONFIGSTORAGE (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else if (rm->boundNode->type == GENERAL)
			os << "RECONFIGGENERAL (" << rm->id << ", " << rm->leftX << ", " << rm->topY << ", " << rm->rightX << ", " << rm->bottomY << ", " << begTS << ", " << endTS << ")" << endl;
		else
			claim(false, "Unsupported reconfigurable resource type.");
	}

	os << endl << "// Locations of the resources listed above (if any)" << endl;
	vector<vector<FixedModule *> *> *ar = arch->getPinMapper()->getAvailRes();
	for (unsigned i = 0; i < ar->size(); i++)
		for (unsigned j = 0; j < ar->at(i)->size(); j++)
			os << "RESOURCELOCATION (" << (int)ar->at(i)->at(j)->resourceType << ", " << ar->at(i)->at(j)->leftX << ", " << ar->at(i)->at(j)->topY << ", " << ar->at(i)->at(j)->rightX << ", " << ar->at(i)->at(j)->bottomY << ", " << ar->at(i)->at(j)->tiledNum << ")" << endl;

	// Now output scheduled DAG with resource bindings to specific IoPorts or Reconfigurable Modules
	os << endl << "// DAG Specification for " << d->getName() << endl;
	if (!d->getName().empty())
		os << "DAGNAME (" << d->getName() << ")" << endl;
	else
		os << "DAGNAME (DAG)" << endl;

	for (unsigned i = 0; i < d->getAllNodes().size(); i++)
	{
		AssayNode *node = d->getAllNodes().at(i);

		string resType;
		if (node->boundedResType == BASIC_RES)
			resType = "B";
		else if (node->boundedResType == D_RES)
			resType = "D";
		else if (node->boundedResType == H_RES)
			resType = "H";
		else if (node->boundedResType == DH_RES)
			resType = "DH";

		int rmId = -1;
		if (node->reconfigMod)
			rmId = node->reconfigMod->getId();

		os << "NODE (";

		if (node->GetType() == DISPENSE) // Id, Type, FluidName, Volume, Name, StartTS, EndTS, IoPortId
			os << node->id << ", DISPENSE, " << node->portName <<  ", " << node->volume << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->ioPort->id << ")\n";
		else if (node->GetType() == MIX) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", MIX, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << rmId << ")\n";
		else if (node->GetType() == DILUTE) // Id, Type, NumDropsBefore, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", DILUTE, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << rmId << ")\n";
		else if (node->GetType() == SPLIT) // Id, Type, NumDropsAfter, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", SPLIT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << rmId << ")\n";
		else if (node->GetType() == HEAT)// Id, Type, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", HEAT, " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << rmId << ")\n";
		else if (node->GetType() == DETECT) // Id, Type, NumDropsIo, Time (s), Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", DETECT, " << node->numDrops <<  ", " << node->seconds << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << rmId << ")\n";
		else if (node->GetType() == OUTPUT) // Id, Type, SinkName, Name, StartTS, EndTS, IoPortId
			os << node->id << ", OUTPUT, " << node->portName << ", " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->ioPort->id << ")\n";
		else if (node->GetType() == STORAGE) // Id, Type, StartTS, EndTS, ReconfigModId
			os << node->id << ", STORAGE, " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";
		else // Id, Type, Name, StartTS, EndTS, ReconfigModId
			os << node->id << ", GENERAL, " << node->name << ", " << node->startTimeStep << ", " << node->endTimeStep << ", " << node->reconfigMod->id << ")\n";

		for (unsigned c = 0; c < node->children.size(); c++)
			os << "EDGE (" << node->id << ", " << node->children.at(c)->id << ")\n";
	}


	os << "=======================Init Done=======================" << endl;


	// Create string stream for each routing phase
	vector<stringstream *> rpStrings;
	for (unsigned i = 0; i <= tsBeginningCycle->size(); i++)
	{
		stringstream *ss = new stringstream();
		(*ss) << "=======================Routing to TimeStep " << i << "=======================" << endl;
		rpStrings.push_back(ss);
	}

	// Add routes to appropriate routing phases
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	for (it = routes->begin(); it != routes->end(); it++)
	{
		vector<RoutePoint *> *route = it->second;
		int rIndex = 0; // Route index
		int beginTS = 0; // Routing to (routing phase) index

		bool hasBegun = false;
		while (rIndex < route->size() && beginTS < tsBeginningCycle->size())
		{
			// Compute routing borders
			unsigned long long routeBeginCycle = 0;
			if (beginTS > 0)
				routeBeginCycle = tsBeginningCycle->at(beginTS-1) + (unsigned long long)(a->secPerTS * a->freqInHz);
			unsigned long long routeEndCycle = tsBeginningCycle->at(beginTS);


			RoutePoint *rp = route->at(rIndex);
			if (rp->cycle >= routeBeginCycle && rp->cycle <= routeEndCycle && (rp->dStatus == DROP_NORMAL || rp->dStatus == DROP_OUTPUT))
			{
				if (hasBegun == false)
					(*rpStrings.at(beginTS)) << "Droplet " << it->first->getId() << endl;
				hasBegun = true;

				(*rpStrings.at(beginTS)) << "(" << rp->x << ", " << rp->y << ")" << endl;
			}
			else
			{
				if (hasBegun == true)
					(*rpStrings.at(beginTS)) << "End Route" << endl;
				hasBegun = false;
			}
			if (rp->cycle <= routeEndCycle+1)
				rIndex++;
			else
				beginTS++;
		}
	}

	// Write each routing phase
	for (unsigned i = 0; i < rpStrings.size(); i++)
		os << rpStrings.at(i)->str() << "Number of droplets routed = " << Util::CountSubstring(Util::Util::StringToUpper(rpStrings.at(i)->str()), "DROPLET") << endl;

	// Cleanup stringstreams
	while (!rpStrings.empty())
	{
		stringstream *ss = rpStrings.back();
		rpStrings.pop_back();
		delete ss;
	}

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes file that has the bare-minimum information needed
// to actually execute the assay on a real-life DMFB (i.e.
// contains just the electrode activations)
///////////////////////////////////////////////////////////////
void FileOut::WriteDmfbProgramToFile(map<Droplet *, vector<RoutePoint *> *> *routes, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write DMFB Program to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	// Get min/max cycle
	map<Droplet *, vector<RoutePoint *> *>::iterator it = routes->begin();
	unsigned long long cycle = 10000000000000;
	unsigned long long lastCycle = 0;
	while (it != routes->end())
	{
		if (!it->second->empty() && it->second->front()->cycle < cycle)
			cycle = it->second->front()->cycle;
		if (!it->second->empty() && it->second->back()->cycle > lastCycle)
			lastCycle = it->second->back()->cycle;
		it++;
	}

	// Create string stream for each cycle
	vector<stringstream *> cycleStrings;
	for (unsigned i = cycle; i <= lastCycle+1; i++)
	{
		stringstream *ss = new stringstream();
		(*ss) << i << ":";
		cycleStrings.push_back(ss);
	}

	// Add routing points to appropriate cycle
	for (it = routes->begin(); it != routes->end(); it++)
	{
		vector<RoutePoint *> *route = it->second;
		for (unsigned i = 0; i < route->size(); i++)
		{
			RoutePoint *rp = route->at(i);
			(*cycleStrings.at(rp->cycle - cycle)) << " (" << rp->x << "," << rp->y << ")";
		}
	}

	// Write each cycle
	for (unsigned i = cycle; i <= lastCycle+1; i++)
		os << cycleStrings.at(i - cycle)->str() << endl;

	// Cleanup stringstreams
	while (!cycleStrings.empty())
	{
		stringstream *ss = cycleStrings.back();
		cycleStrings.pop_back();
		delete ss;
	}

	os.close();
}

///////////////////////////////////////////////////////////////
// Writes file that has the bare-minimum information needed
// to actually execute the assay on a real-life DMFB (i.e.
// contains just the electrode activations). Similar to
// WriteDmfbProgramToFile(), except this function produces
// output in the form of a binary bit-vectors.
///////////////////////////////////////////////////////////////
void FileOut::WriteDmfbBinaryProgramToFile(DmfbArch *arch, vector<vector<int> *> *pinActivations, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	{
		stringstream str;
		str << "Failed to properly write DMFB Binary Program to file: " << fileName << endl;
		claim (os.good(), &str);
	}

	int numPins = arch->getPinMapper()->getNumUniquePins();
	os << "// Number states/cycles" << endl << pinActivations->size() << endl << endl;
	os << "// Vector length (# pins)" << endl << numPins << endl << endl;

	// Output a bit-vector for each cycle
	os << "// Cyclic Bit-Vectors (0 = OFF, 1 = ON)" << endl;
	for (unsigned i = 0; i < pinActivations->size(); i++)
	{
		int pin = 0;
		vector<int> * activationsThisCycle = pinActivations->at(i);
		sort(activationsThisCycle->begin(), activationsThisCycle->end());
		for (unsigned j = 0; j < activationsThisCycle->size(); j++)
		{
			int activePin = activationsThisCycle->at(j);
			while (pin <= activePin)
			{
				if (pin == activePin)
					os << 1;
				else
					os << 0;
				pin++;
			}
		}

		// Write 0's for the remaining pins
		while (pin < numPins)
		{
			os << 0;
			pin++;
		}
		os << endl;
	}

	os.close();
}

////////////////////////////////////////////////////////////////////////////
// Writes (overwrites) the given string to a file specified by fileName.
////////////////////////////////////////////////////////////////////////////
void FileOut::WriteStringToFile(string s, string fileName)
{
	ofstream os;
	os.open(fileName.c_str());

	os << s;

	os.close();
}

////////////////////////////////////////////////////////////////////////////
// Returns the string name of the operation type
////////////////////////////////////////////////////////////////////////////
string FileOut::GetOperationString(OperationType ot)
{
	if (ot == MIX)
		return "MIX";
	else if (ot == DILUTE)
		return "DILUTE";
	else if (ot == SPLIT)
		return "SPLIT";
	else if (ot == HEAT)
		return "HEAT";
	else if (ot == DETECT)
		return "DETECT";
	else if (ot == STORAGE)
		return "STORAGE";
	else if (ot == STORAGE_HOLDER)
		return "STORAGE_HOLDER";
	else if (ot == GENERAL)
		return "GENERAL";
	else
	{
		claim(false, "Unknown operation type.");
		return "UNKNOWN"; // Unreachable, Eliminate Warning
	}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Given an enumeration type, the following "GetKeyFrom...Type" functions
// search the appropriate library to find and return the requested key.
//////////////////////////////////////////////////////////////////////////////
string FileOut::GetKeyFromSchedType(SchedulerType sType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.sEnums.size(); i++)
		if (sType == c.sEnums.at(i))
			return c.sKeys.at(i);

	cout << "Invalid parameter: " << sType << " is not a valid scheduler type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromPlaceType(PlacerType pType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.pEnums.size(); i++)
		if (pType == c.pEnums.at(i))
			return c.pKeys.at(i);

	cout << "Invalid parameter: " << pType << " is not a valid placer type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromRouteType(RouterType rType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.rEnums.size(); i++)
		if (rType == c.rEnums.at(i))
			return c.rKeys.at(i);

	cout << "Invalid parameter: " << rType << " is not a valid router type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromResourceAllocationType(ResourceAllocationType raType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.raEnums.size(); i++)
		if (raType == c.raEnums.at(i))
			return c.raKeys.at(i);

	cout << "Invalid parameter: " << raType << " is not a valid resource-allocation type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromPinMapType(PinMapType pmType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.pmEnums.size(); i++)
		if (pmType == c.pmEnums.at(i))
			return c.pmKeys.at(i);

	cout << "Invalid parameter: " << pmType << " is not a valid pin-mapper type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromWrType(WireRouteType wrType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.wrEnums.size(); i++)
		if (wrType == c.wrEnums.at(i))
			return c.wrKeys.at(i);

	cout << "Invalid parameter: " << wrType << " is not a valid wire-routing type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromCompType(CompactionType cType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.cEnums.size(); i++)
		if (cType == c.cEnums.at(i))
			return c.cKeys.at(i);

	cout << "Invalid parameter: " << cType << " is not a valid compaction type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromPeType(ProcessEngineType peType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.peEnums.size(); i++)
		if (peType == c.peEnums.at(i))
			return c.peKeys.at(i);

	cout << "Invalid parameter: " << peType << " is not a valid processing engine type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
string FileOut::GetKeyFromEtType(ExecutionType etType)
{
	CmdLine c;
	for (unsigned i = 0; i < c.etEnums.size(); i++)
		if (etType == c.etEnums.at(i))
			return c.etKeys.at(i);

	cout << "Invalid parameter: " << etType << " is not a valid execution type. Please add to the command line library if you have not done so." << endl;
	c.ForceCorrectUsage();
	return "Usage"; // Unreachable, Eliminate Warning
}
