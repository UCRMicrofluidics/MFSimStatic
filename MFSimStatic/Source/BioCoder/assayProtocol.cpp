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
#include "../../Headers/BioCoder/assayProtocol.h"

#include <fstream>
#include <string>

assayProtocol::assayProtocol(string FileName)
{
	dagDropRestrict = 100000; // The value of 13 causes a problem when 32+ outputs for protein
	stepNum=0;
	completeMixNum=0;
	nodes= new tNameBank(FileName);
	steps= new assayStep[100000];
	completeMixList= new aMixture[10000];
}

assayProtocol::~assayProtocol()
{
	delete nodes;
	delete completeMixList;
	delete steps;
}
int assayProtocol :: gcd( int a, int b)
{
	if(b==0)
		return a;
	else
		return gcd(b, a%b);
}

bool assayProtocol:: isInt(float f)
{
	int i =f;
	return(f==static_cast<float>(i));
}

void assayProtocol:: deciRatio2IntRatio(float &f,float &f2)
{
	int countf=0;
	int countf2=0;
	while(!isInt(f))
	{
		countf++;
		f*=10;
	}
	while(!isInt(f2))
	{
		countf2++;
		f2*=10;
	}
	if(countf > countf2)
		for(int i=0;i<countf-countf2;++i)
			f2*=10;
	else
		for(int i=0;i<countf2-countf;++i)
			f*=10;
}
bool assayProtocol:: ckSplit(int target ,int total, list<int> & splits,int dropletRestrict)
{
	int numDrop=dropletRestrict;
	/*int gcdTemp=gcd(total,target);
	if(gcdTemp>1){
		target/=gcdTemp;
		total/=gcdTemp;
	}*/ //MOVE THIS TO OUTSIDE ATTEMPT3 INTO SPLIT MANAGEMENT!!!!!!
	if(total<=13){
		splits.push_back(total);
		return true;
	}
	for (int i=numDrop;i>=2;--i){
		if(total%i==0&& (total/i)<=numDrop){
			splits.push_back(i);
			splits.push_back(total/i);
			return true;
		}
		else if(total%i==0){
			//there needs to be some check on target to find out how much is needed.
			splits.push_back(i);
			if(ckSplit(target,total/i,splits,dropletRestrict))
				return true;
			else
				splits.remove(total/i);
		}
	}
	return false;
}
int assayProtocol:: numPiecesUsed(int target, int pieces, int value)
{
	for (int i=1; i<=pieces;++i)
		if(target< i*value)
			return i-1;
	return pieces;
}
void assayProtocol:: splitCalls(Container & src,Container & dest, map<int,pair<int,string> > splitInst, map <int , string> mixDest, bool ensureMeasurement)
{ //Do Not Use.
	cout<<"FUNCTION REMOVED!"<<endl;
	/*string targetLinkName,tempLinkName;

	Container temp;
	Container target;
	map<int,string>::iterator mixDestIt;
	map<int,pair<int,string> >::iterator it=splitInst.begin();
	string linkName=assaySplit(it->second.first, ensureMeasurement);
	addToLinkMap(src,linkName);
	++it;
	for(;it!= splitInst.end();++it)
	{

		if(it->second.second=="s"){
			string linkName=assaySplit(it->second.first, ensureMeasurement);
			tempLinkName=linkName;
			addToLinkMap(temp,linkName);
		}
		if(it->second.second=="m"){
			mixDestIt=mixDest.find(it->first);
			if(mixDestIt!=mixDest.end())
			{

				if(mixDestIt->second=="dest"){
					for (int i=0;i<it->second.first;++i){
						target.tLinkName.push_back(linkName);
					}
					assayMixManagement(target,aMixture(),"mix");
				}
				else {
					for (int i=0;i<it->second.first-1;++i){
						src.tLinkName.push_back(linkName);
					}
					assayMixManagement(target,aMixture(),"mix");
					temp.tLinkName.push_back(tempLinkName);
				}
			}
		}
	}
	assayPreMix(target,dest);
	assayMixManagement(dest,aMixture(),"mix");
	//	linkName = assayMix(aMixture(),"mix");
	//	addToLinkMap(dest,linkName);
	temp.tLinkName.pop_front();
	assayPreMix(temp,src);
	assayMixManagement(src,aMixture(),"mix");
	if(conLookUp.find(target.name)!= conLookUp.end())
		conLookUp.erase(conLookUp.find(target.name));
	if(conLookUp.find(temp.name)!= conLookUp.end())
		conLookUp.erase(conLookUp.find(temp.name));
*/}
void assayProtocol:: splitManagment(Container & src,Container & dest,Volume targetVolume, bool ensureMeasurement)
{
	map<int,string> mixDestinations;
	map<int, pair<int,string> > splitInstructions;
	pair<int,pair<int,string> > map;
	pair<int ,string> helper;
	int stepNo=0;
	int dropCount=getDropCount();
	list<int> splitList;
	int value;
	int pieces;
	int largest=0;
	float totalVol = src.volume;
	float targetVol= targetVolume.conv2UL();
	deciRatio2IntRatio(totalVol,targetVol);
	int total=totalVol;
	int target=targetVol;
	int tot=totalVol;
	int tar=targetVol;
	int gcdTemp=gcd(total,target);
	if(gcdTemp>1){
		target/=gcdTemp;
		total/=gcdTemp;
	}
	if(! ckSplit(target,total,splitList,dagDropRestrict-dropCount))
	{
		cerr <<"Ratio of "<< target<< " / "<< total<< " is not possible to"
				<<" obtain, exiting program"<<endl;
		exit(-1);
	}
	//if true then splitList will have the necessary splits for the protocol.
	int piecesLeft;
	int debug = 0; //dtg debug
	while(!splitList.empty())
	{
		pieces=splitList.front();
		splitList.pop_front();

		if(largest<pieces)
			largest=pieces;
		dropCount+=(pieces-1);
		if (dropCount > dagDropRestrict)
		{
			total=totalVol/gcdTemp;
			target=targetVol/gcdTemp;
			int tot=totalVol;
			int tar=targetVol;
			splitList.clear();
			mixDestinations.clear();
			splitInstructions.clear();
			stepNo=0;
			if(! ckSplit(target,total,splitList,largest-1))
			{
				cerr <<"Ratio of "<< target<< " / "<< total<< " is not possible to"
						<<" obtain given number of drops, exiting program"<<endl;
				exit(-1);
			}
		}
		else{

			value=tot/pieces;
			int numUsed= numPiecesUsed(tar,pieces,value);

			helper.first=pieces;//this group splits the total into parts
			helper.second="s";
			map.first=stepNo++;
			map.second=helper;
			splitInstructions.insert(map);

			helper.first=stepNo;		//this group gives a tag for the mix for the destination
			helper.second="dest";
			mixDestinations.insert(helper);

			helper.first=numUsed;		// this is where the information is gathered for the mix
			helper.second="m";
			map.first=stepNo++;
			map.second=helper;
			splitInstructions.insert(map);
			tar-=numUsed*value;
			dropCount-=(numUsed-1);

			helper.first=stepNo;
			helper.second="src";
			mixDestinations.insert(helper);

			if(tar!=0){
				helper.first=pieces-(numUsed+1);
				helper.second="m";
				map.first=stepNo++;
				map.second=helper;
				splitInstructions.insert(map);

				dropCount-=(pieces- numUsed+1);

				total=value;
			}
			else{
				helper.first=pieces-(numUsed);
				helper.second="m";
				map.first=stepNo++;
				map.second=helper;
				splitInstructions.insert(map);

				dropCount-=(pieces- numUsed+1);

				tot=value;

			}
		}
	}
	//now that the split calls can be done correctly this will call all mixes and splits
	splitCalls(src, dest, splitInstructions,mixDestinations, ensureMeasurement);
}


void assayProtocol :: printassay(ostream & out)
{
	for (int i=0;i<=stepNum;++i){
		out<<"step " << i << ":"<<endl;
		if(steps[i].dispence){
			out<<"  Dispense:\n";
			for(map<string,aDispense>::iterator it= steps[i].dispList.begin();
					it!= steps[i].dispList.end();++it){
				out<<"     "<<it->second.flu.new_name<<" ";
				out<<it->second.vol.value<<", ";
				switch(it->second.vol.unit_choice){
				case 0:out<< "uL "; break;
				case 1:out<< "mL "; break;
				case 2:out<< "L ";  break;
				}
				out<< " ("<< it->first<<")"<<endl;

			}
		}
		if(steps[i].mix){
			for( map<string,aMix>::iterator it= steps[i].mixList.begin();
					it !=steps[i].mixList.end();++it)
			{
				out<< "  Mix: (" << it->first<< ") ";
				out<< "total Volume: "<< it->second.volume;
				out<<endl<<"     "<<it->second.typeofmix<<endl;
			}
		}
		if(steps[i].heat){
			out<< "  Heat: "<<endl;
			for( map<string,aHeat>::iterator it= steps[i].heatList.begin();
					it !=steps[i].heatList.end();++it)
			{
				out <<"     "<< it->second.fluids.name << " for ";
				out << it->second.time.value<< " ";
				switch(it->second.time.unit_choice){
				case 0: out<<"S"; break;
				case 1: out<<"Min(s)"; break;
				case 2: out<< "Hr(s)"; break;
				}
				out << " at " << it->second.temp <<" degrees C";
				out<< " ("<< it->first<<"), Volume: "<< it->second.volume<< endl;
			}
		}
		if(steps[i].store){
			out<< "  Store: "<<endl;
			for( map<string,aStore>::iterator it= steps[i].storeList.begin();
					it !=steps[i].storeList.end();++it)
			{
				out << it->second.fluids.name << " for ";
				out << it->second.time.value<< " ";
				switch(it->second.time.unit_choice){
				case 0: out<<"S"; break;
				case 1: out<<"Min(s)"; break;
				case 2: out<< "Hr(s)"; break;
				}
				out << " at " << it->second.temp <<" degrees C";
				out<< " ("<< it->first<<")"<<endl;
			}
		}
		if(steps[i].split){
			out<< "  Split: "<<endl;
			for(map<string,aSplit>::iterator it= steps[i].splitList.begin();
								it != steps[i].splitList.end();++it)
			{
				if(it->second.CF)
				{
					out<<"     ("<<it->first<<") Taking "<<it->second.volume<< "uL out."<<endl;
				}
				else
				{
					out<<"     ("<<it->first<<")Splitting into " << it->second.numSplit << " parts."<<endl;
				}
			}
		}
		if(steps[i].detect){
			out<< "  Detect: " <<endl;
			for(map<string,aDetect>::iterator it= steps[i].detectList.begin();
					it != steps[i].detectList.end();++it)
			{
				out<<"      "<<it->second.detectType<< "for ";
				out << it->second.time.value<< " ";
				switch(it->second.time.unit_choice){
				case 0: out<<"Sec(s)"; break;
				case 1: out<<"Min(s)"; break;
				case 2: out<< "Hr(s)"; break;
				}
				out<<" ("<<it->first<<"), Volume: "<<it->second.volume<<"uL "<<endl;
			}
		}
		if(steps[i].output){
			out<< "  Output: "<<endl;
			for(map<string,aOutput>::iterator it= steps[i].outputList.begin();
					it != steps[i].outputList.end();++it)
				out<<"     "<<"Sink Name: "<< it->second.sinkName<<endl;
		}
	}
	out<<endl;
}
void assayProtocol:: printLinks(ostream & out)
{
	for( map<string,list <string> > :: iterator it =  links.begin();
			it != links.end() ; ++it)
	{
		out << "child: "<<it->first<<endl << "Parent(s): \n";

		for (list <string>::iterator j = it->second.begin(); j != it->second.end(); ++j)
			out << *j << endl;
		out <<endl;
	}

}
void assayProtocol :: addToInitInputList(aDispense & dis)
{
	inputList.push_back(dis);
}

void assayProtocol :: addToInitOutputList(aOutput & out)
{
	outputList.push_back(out);
}

string assayProtocol:: assayDispense(Fluid fluid1,Volume volume1)
{
	string linkName=nodes->accessName(DIS);//index zero is the location of
	nodes->incNodeName(DIS);				  //the dispence link name

	aDispense dis(fluid1,Volume(volume1));
	addToInitInputList(dis);

	pair<string,int> b(linkName,stepNum);
	nodeLookUp.insert(b);

	pair<string,aDispense> a (linkName, dis);
	steps[stepNum].dispence=true;
	steps[stepNum].dispList.insert(a);

	return linkName;
}
void assayProtocol :: assayPreMix(Container & Src, Container & Dest)
{
	addToMixtureList(Src, Dest);
	for(list <string>::iterator srcPtr =Src.tLinkName.begin(); srcPtr != Src.tLinkName.end(); ++srcPtr)
		Dest.tLinkName.push_back(*srcPtr);
	Src.tLinkName.clear();

}

//void assayProtocol::addToCondtionalMaps(BioConditionalGroup *bcg)
//{
//	for (int i = 0; i < bcg->getConditions()->size(); i++)
//	{
//		BioCondition *bCond = bcg->getConditions()->at(i);
//		if (bCond->statement->operandType == OP_ONE_SENSOR)
//		{
//			//			BioCondition *bCond = bcg->getConditions()->at(i);
//			pair<string,BioConditionalGroup*> condtion(bCond->statement->sensor1, bcg);
//			conditions.insert(condtion);
//			pair<string, BioCoder*> link(bCond->statement->sensor1, bCond->branchIfTrue);
//			cfgLink.insert(link);
//		}
//		else if (bCond->statement->operandType == OP_TWO_SENSORS)
//		{
//			//			BioCondition *bCond = bcg->getConditions()->at(i);
//			pair<string,BioConditionalGroup*> condtion(bCond->statement->sensor2, bcg);
//			conditions.insert(condtion);
//			pair<string, BioCoder*> link(bCond->statement->sensor2, bCond->branchIfTrue);
//			cfgLink.insert(link);
//		}
//		else if(bCond->statement->operationType == OP_UNCOND)
//		{
//			pair<string,BioConditionalGroup*> condtion("UNCOND", bcg);
//			conditions.insert(condtion);
//			pair<string, BioCoder*> link("UNCOND", bCond->branchIfTrue);
//			cfgLink.insert(link);
//		}
//	}
//
//
//
//
//}

string assayProtocol :: assayDetect(string typeDetect,Time time,Container &con)
{
	steps[stepNum].detect=true;
	string linkName = nodes->accessName(DCT);
	nodes->incNodeName(DCT);

	/*if(e!=NULL){
		if(!bcg)
			bcg=new BioConditionalGroup();
		//if(e->operationType!=OP_UNCOND)
		if(e->operandType==OP_TWO_SENSORS)
			e->setSensor2(linkName);
		else
			e->setSensor1(linkName);
		bcg->addNewCondition(e,BioPtr);
		//pair<string,BioConditionalGroup*> condtion(linkName, bcg);
		//conditions.insert(condtion);
		//pair<string, BioCoder*> link(linkName, BioPtr);
		//cfgLink.insert(link);
	}*/
	pair<string ,int> b(linkName,stepNum);
	nodeLookUp.insert(b);

	aDetect detect(typeDetect, time);
	detect.volume=con.volume;
	pair<string, aDetect> a (linkName, detect);
	steps[stepNum].detectList.insert(a);

	return linkName;
}
string assayProtocol :: assayStore(Container & con, float temp1, Time time1)
{
	steps[stepNum].store=true;
	string linkName=nodes->accessName(STR); //index 6 holds the place
	nodes->incNodeName(STR);				   //of store link name

	pair<string,int> b(linkName,stepNum);
	nodeLookUp.insert(b);

	aStore str(time1,temp1,findMix(con));
	pair<string, aStore> a(linkName,str);
	steps[stepNum].storeList.insert(a);

	return linkName;
}
string assayProtocol :: assayHeat(Container &con, float temp1, Time time1)
{
	steps[stepNum].heat=true;
	string linkName=nodes->accessName(HT); //index 2 holds the place
	nodes->incNodeName(HT);				   //of heat link name

	pair<string,int> b(linkName,stepNum);
	nodeLookUp.insert(b);

	aHeat str(time1,temp1,findMix(con));
	str.volume=con.volume;
	pair<string, aHeat> a(linkName,str);
	steps[stepNum].heatList.insert(a);

	return linkName;
}

string assayProtocol :: assayMix(aMixture mixture,string typeofmix,Container& con)
{
	steps[stepNum].mix=true;
	string linkName=nodes->accessName(MX);
	nodes->incNodeName(MX);

	pair<string,int> b(linkName,stepNum);
	nodeLookUp.insert(b);

	aMix mix(mixture, typeofmix);
	mix.volume=con.volume;
	pair<string,aMix> a ( linkName,mix);
	steps[stepNum].mixList.insert(a);

	return linkName;
}
void assayProtocol:: assayMixManagement(Container & con, aMixture & mixture, string typeofmix)
{
	//cout<<mixture.volume<<endl;
	if(con.tLinkName.size() > 1){
		string linkName=assayMix(mixture,typeofmix, con);
		addToLinkMap(con,linkName);
	}
}
string assayProtocol :: assayOutput(string sinkName)
{
	steps[stepNum].output=true;
	string linkName= nodes->accessName(OT);
	nodes->incNodeName(OT);

	pair<string, int> b(linkName, stepNum);
	nodeLookUp.insert(b);

	aOutput out(sinkName, linkName);
	addToInitOutputList(out);

	pair<string, aOutput> a(linkName, out);
	steps[stepNum].outputList.insert(a);

	return linkName;
}
string assayProtocol :: assayWasteOutput(string sinkName)
{
	steps[stepNum].output=true;
	string linkName= nodes->accessName(OT);
	nodes->incNodeName(OT);

	pair<string, int> b(linkName, stepNum);
	nodeLookUp.insert(b);

	aOutput out(sinkName, linkName);
	addToInitOutputList(out);

	pair<string, aOutput> a(linkName, out);
	steps[stepNum].outputList.insert(a);

	return linkName;
}
string assayProtocol:: assaySplit(int n, bool ensureMeasurement)
{// This should be used for Droplet Applications ONLY not control flow applications.
	steps[stepNum].split=true;
	string linkName=nodes->accessName(SPLT);
	nodes->incNodeName(SPLT);

	pair<string, int> b (linkName, stepNum);
	nodeLookUp.insert(b);

	aSplit split(n);
	split.ensureMeasurement = ensureMeasurement;
	pair<string, aSplit>a(linkName,split);
	steps[stepNum].splitList.insert(a);

	return linkName;

}
string assayProtocol:: assaySplit (Volume & v1,bool ensureMeasurement)
{
	steps[stepNum].split=true;
	string linkName=nodes->accessName(SPLT);
	nodes->incNodeName(SPLT);

	pair<string, int> b (linkName, stepNum);
	nodeLookUp.insert(b);
//this will just do integer division as a way to short cut.
	aSplit split(0);
	split.CF=true;
	split.volume=v1.conv2UL();
	split.ensureMeasurement = ensureMeasurement;
	pair<string, aSplit>a(linkName,split);
	steps[stepNum].splitList.insert(a);

	return linkName;
}
string assayProtocol:: assayTransIn()
{
	steps[stepNum].transIn=true;
	string linkName =nodes->accessName(TrN);
	nodes->incNodeName(TrN);

	pair<string ,int> b(linkName, stepNum);
	nodeLookUp.insert(b);

	aTransIn transIn(linkName);
	pair<string,aTransIn> a(linkName, transIn);
	steps[stepNum].transInList.insert(a);

	return linkName;
}

string assayProtocol:: assayTransOut(Container &con)
{
	steps[stepNum].transOut=true;
	string linkName =nodes->accessName(TrO);
	nodes->incNodeName(TrO);

	pair<string ,int> b(linkName, stepNum);
	nodeLookUp.insert(b);

	aTransOut transOut(con);
	pair<string,aTransOut> a(linkName, transOut);
	steps[stepNum].transOutList.insert(a);

	return linkName;
}


//assayProtocol assayProtocol::mergeSteps()
//{
//	assayProtocol;
//	for (int i =0; i<stepNum;++i)
//
//}

void assayProtocol :: collectOutput()
{
	for(int i=0 ; i<completeMixNum ;++i){
		map<string, Container>::iterator it = conLookUp.find(completeMixList[i].container);
		if(it!= conLookUp.end()){
			string linkName = assayOutput(completeMixList[i].container);
			pair<string,list <string> > a (linkName , it->second.tLinkName);
			links.insert(a);
		}
	}
}
void assayProtocol :: addToLinkMap(Container & con, string linkName)
{
	if(linkName.substr(0,1)!= "M" && con.tLinkName.size()>1)
	{
		string newLinkName =nodes->accessName(MX);
		nodes->incNodeName(MX);
		addToLinkMap(con,newLinkName);
	}
	//cout<< linkName<<" "<< con.tLinkName.front()<<endl;
	pair<string,list <string> > a (linkName , con.tLinkName);
	links.insert(a);
	con.tLinkName.clear();
	con.tLinkName.push_back(linkName);

	//this keeps track of the containers given for output list.
	//if the container doesnt exist in the list, it adds the container to the list.
	//otherwise it updates the old containter with the new contents.
	if(conLookUp.count(con.name)!=0){
		conLookUp.erase(conLookUp.find(con.name));
	}
	pair<string, Container> contIndex(con.name,con);
	conLookUp.insert(contIndex);


}
void assayProtocol :: assayDrain(Container & con, string outputSink)
{
	//empty the container
	con.volume=0;
	con.contents.volume=0;
	con.contents.original_name="";
	con.contents.new_name="";

	//assay link management
	//con.name;
	string linkName =assayWasteOutput(outputSink);
	addToLinkMap(con, linkName);
	clearMixture(con);
}

void assayProtocol :: clearMixture(Container& con)
{
	for( int i=0 ; i <completeMixNum;++i)
		if(completeMixList[i].container== con.name)
			completeMixList[i].container="";
}

aMixture assayProtocol:: findMix(Container &con)
{
	return completeMixList->find(con,completeMixNum);
}


aMixture assayProtocol:: addtoMixtureList(Container& con, Fluid& fluid)
{
	return completeMixList->addToMixList(con,fluid,completeMixNum);
}

aMixture assayProtocol:: addToMixtureList(Container& src, Container& dest)
{
	return completeMixList->addToMixList ( src,dest, completeMixNum);
}

void assayProtocol:: Translator(string filename ,DAG * dag, bool color, bool all)
{
//	vector<BioConditionalGroup *> BCGList;
	ofstream out;
	string temp = "Output/Bio_DMFB_";
	temp+= filename;
	temp+= ".dot";

	out.open(temp.c_str());


	int step =0;
	out<<"digraph G {\n";
	out<<"  size=\"8.5,10.25\";\n";


	double splitTime = 2;
	vector<AssayNode*> assays;

	//running through the steps
	for(int i=0 ; i <= stepNum; ++i)
	{
		if(steps[i].detect){
			for( map<string,aDetect>::iterator it= steps[i].detectList.begin();
					it != steps[i].detectList.end();++it)
			{
				float time;
				if(it->second.time.value == 0)
					time=.1;//default length of time to detect
				else
					time=it->second.time.value;

				AssayNode *detect =dag->AddDetectNode(1, time, it->first);
				//cout<< "AssayNode *Disp =dag->AddDetectNode("<< time<<" ,"<< it->first <<");"<<endl;


				out<< assays.size() <<" [label = \"Detect\"";
				if(color)
					out<<" fillcolor= orange, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(detect);

				/*run2 collection of condtions*/
//				for(map<string,BioConditionalGroup*>::iterator ExprIterator =conditions.begin();
//						ExprIterator != conditions.end();
//						++ExprIterator)
//				{
//					//cout<<it->first<<" "<<ExprIterator->first<<endl;
//					if(it->first== ExprIterator->first)
//					{
//						ConditionalGroup * cg;
//
//
//						for(int i=0; i<ExprIterator->second->getConditions()->size();++i)/*going through the conditions*/
//						{
//							if(ExprIterator->second->getConditions()->at(i)->statement->operandType==OP_TWO_SENSORS)/*if the condition is a 2 sensor readings*/
//							{
//								//cycles through every Dag that has been Created and finds correct node.
//								for (map<BioCoder *, DAG*>::iterator dagCycle = BioDag.begin(); dagCycle != BioDag.end(); ++dagCycle)
//								{
//									for (int j = 0 ; j < dagCycle->second->getAllNodes().size(); j++)//finds sensor1
//									{
//										//cout<< dagCycle->second->getAllNodes().at(j)->GetName() <<" "<< ExprIterator->second->getConditions()->at(i)->statement->sensor1<<endl;
//										if(dagCycle->second->getAllNodes().at(j)->GetName() == ExprIterator->second->getConditions()->at(i)->statement->sensor1)/*find sensor1*/
//										{
//											ExprIterator->second->getConditions()->at(i)->statement->s1=dagCycle->second->getAllNodes().at(j);//assigns sensor1 into the expression
//											break;
//											//sensor1 = assays.at(j);
//										}
//									}
//								}
//							}
//						}
//						cg=ExprIterator->second->convt2CG(this,detect);
//						CGList.push_back(cg);
//					}
//				}
			}
		}
		if(steps[i].dispence){
			for( map<string,aDispense>::iterator it= steps[i].dispList.begin();
					it != steps[i].dispList.end(); ++it)
			{
				string fluid= it->second.flu.original_name;
				double vol= it->second.vol.conv2UL();
				AssayNode *Disp =dag->AddDispenseNode(fluid,vol ,it->first);
				//cout<< "AssayNode *Disp =dag->AddDispenseNode("<< fluid<<", "<<vol<<" ,"<< it->first <<");"<<endl;
				out<<assays.size() <<" [label = \""<<fluid;
				if(all)
					out<< " "<< it->second.vol.conv2UL()<<"UL \"";
				else
					out<<"\"";
				if(color)
					out<<" fillcolor=cyan, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Disp);
			}
		}
		if(steps[i].mix){
			for( map<string,aMix>::iterator it= steps[i].mixList.begin();
					it !=steps[i].mixList.end();++it)
			{
				int numDrops= links.find(it->first)->second.size();
				//				for(list<string>::iterator it2=links.find(it->first)->second.begin(); it2!=links.find(it->first)->second.end();++it2)
				//					cout<< *it2<<endl;
				AssayNode* Mix= dag->AddMixNode(numDrops,it->second.component.time.conv2secs(),it->first);
				//cout << "AssayNode* Mix= dag->AddMixNode( " <<numDrops << "," << it->second.component.time.conv2secs() << "," << it->first << ");"<<endl;
				out<<assays.size() <<" [label = \"Mix\"";
				if(color)
					out<<" fillcolor=yellow, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Mix);
			}
		}
		if(steps[i].heat){
			for( map<string,aHeat>::iterator it= steps[i].heatList.begin();
					it !=steps[i].heatList.end();++it)
			{
				double time= it->second.time.conv2secs();
				//				cout<<"TIME: ----->>>>>>>>"<<time<<endl;
				AssayNode * Heat= dag->AddHeatNode(time,it->first);
				//cout<<"AssayNode * Heat= dag->AddHeatNode("<<time<<","<<it->first<<");"<<endl;
				out<<assays.size() <<" [label = \"Heat";
				if(all)
					out<<" ("<<it->second.temp<< "C, "<<time<<" s)\"";
				else
					out<<"\"";
				if(color)
					out<<" fillcolor=lightcoral, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Heat);
			}
		}
		if(steps[i].split){
			for(map<string,aSplit>::iterator it = steps[i].splitList.begin();
					it!= steps[i].splitList.end();++it)
			{

				AssayNode * Split = dag->AddSplitNode(it->second.ensureMeasurement, it->second.numSplit,splitTime,it->first);
				//cout<< "AssayNode * Split = dag->AddSplitNode("<<it->second.numSplit<<","<<splitTime<<","<<it->first<<")"<<endl;

				out<<assays.size() <<" [label = \"Split\"";
				if(color)
					out<<" fillcolor=seagreen1, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Split);
			}
		}
		if(steps[i].store){
			//			cout<< "Store: "<<endl;
			for( map<string,aStore>::iterator it= steps[i].storeList.begin();
					it !=steps[i].storeList.end();++it)
			{
				AssayNode* Store= dag->AddStorageNode(it->first);
				//cout<<"AssayNode* Store= dag->AddStorageNode("<< it->first <<");"<<endl;
				out<<assays.size() <<" [label = \"Store\"";
				if(color)
					out<<" fillcolor=burlywood, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Store);
			}
		}
		if(steps[i].output){
			//			cout<< "Output: "<<endl;
			for( map<string,aOutput> :: iterator it= steps[i].outputList.begin();
					it !=steps[i].outputList.end();++it)
			{
				aOutput foo = it->second;

				AssayNode* Output= dag->AddOutputNode(it->second.sinkName, it->first);
				//cout<<"AssayNode* Output= dag->AddOutputNode("<< it->second.sinkName<<", "<< it->first<<");"<<endl;
				out<<assays.size() <<" [label = \"" << it->second.sinkName << "\"";
				if(color)
					out<<" fillcolor=orchid1, style=filled];\n";
				else
					out<<"];\n";
				assays.push_back(Output);
			}
		}
	}
//	map<string,BioConditionalGroup*>::iterator ExprIterator =conditions.find("UNCOND");
//	if(ExprIterator!=conditions.end())
//	{
////		if(ExprIterator->first == "UNCOND"){
//			ConditionalGroup * cg;
//			cg=ExprIterator->second->convt2CG(this,assays.at(0));
//			CGList.push_back(cg);
////		}
//	}


	int graphNum=-1;
	int c;
	//Establising the links between the nodes
	for(map<string,list<string> >::iterator it= links.begin();
			it != links.end(); ++it)
	{
		AssayNode *child;
		for (unsigned i = 0 ; i < assays.size(); i++)
		{
			if(assays.at(i)->GetName() == it->first)
			{
				c = i;
				child = assays.at(i);
			}
		}

		for (list<string> :: iterator j=it->second.begin(); j != it->second.end(); ++j)
		{
			AssayNode *parent;
			for (unsigned i = 0 ; i < assays.size(); i++)
			{
				if(assays.at(i)->GetName() == *j)
				{
					graphNum=i;
					parent = assays.at(i);
				}
			}
			dag->ParentChild(parent,child);
			out << graphNum <<" -> "<< c <<";\n";
			//cout<<"dag->ParentChild(parent,child);"<<endl;
			//cout<< "parent: "<< parent->GetName()<<endl;
			//cout<< "child: "<< child ->GetName()<<endl;

		}
	}

	out<<"}\n";
//	return BCGList;
}

void assayProtocol:: Translator(string file){
	cout<<"Translating to CF System"<<endl;
	MCFlow *CFSys;
	double defaultNum=5;
	vector<infoNode> instructs;

	if(file=="")
	{
		CFSys=new MCFlow();
	}
	else
	{
		CFSys=new MCFlow(file);
	}

	for (int i=0;i<=stepNum;++i){
		if(steps[i].mix || steps[i].heat || steps[i].detect){
			CFSys->createNewLine();
			cout<<endl;

		}
		if(steps[i].dispence){
			for(map<string,aDispense>::iterator it= steps[i].dispList.begin();
					it!= steps[i].dispList.end();++it){
				CFSys->nodes.push_back(infoNode(it->first,it->second.vol.conv2UL(),0));
			}
		}
		if(steps[i].mix){
			for( map<string,aMix>::iterator it= steps[i].mixList.begin();
					it !=steps[i].mixList.end();++it)
			{
				double mTime;
				if(it->second.component.time.conv2secs()>0)
				{
					mTime=it->second.component.time.conv2secs();
				}
				else{
					mTime=defaultNum;
				}
				CFSys->nodes.push_back(infoNode(it->first,it->second.volume,mTime));
				map<string,list <string> > :: iterator it2 =links.find(it->first);//finds name of parents
				for (list <string>::iterator j = it2->second.begin(); j != it2->second.end(); ++j){
					infoNode temp= CFSys->find(*j);
					while(temp.name.substr(0,3)=="SLT"){
						double tVol= temp.vol;
						temp= CFSys->find(temp.sName);
						temp.vol=tVol;
					}
					if(temp.name!=""){
						instructs.push_back(temp);
					}
					else
						cout<<*j<<": Node Not found!"<<endl;
				}
				cout<<"mix "<<it->first<<" "<<instructs[0].name<<" "<<instructs[0].vol<<" "<<instructs[1].name<<" "<<instructs[1].vol<<" "<<mTime<<endl;
				CFSys->fluidInstr("mix", it->first, instructs[0].name, instructs[0].vol, instructs[1].name, instructs[1].vol, mTime);
				instructs.clear();//Maintenance of the instruction vector.
			}
		}
			if(steps[i].heat){
				for( map<string,aHeat>::iterator it= steps[i].heatList.begin();
						it !=steps[i].heatList.end();++it)
				{
					double hTime; //makes sure time is not zero
					if(it->second.time.conv2secs()==0){
						cout<<"Heat time:"<<it->second.time.conv2secs();
						//hTime=defaultNum;
					}
					else{
						hTime=it->second.time.conv2secs();
						//cout<<"Heat time:"<<it->second.time.conv2secs();

					}
					CFSys->nodes.push_back(infoNode(it->first,it->second.volume,hTime));
					map<string,list <string> > :: iterator it2 =links.find(it->first);//finds name of parents
					for (list <string>::iterator j = it2->second.begin(); j != it2->second.end(); ++j){
						infoNode temp= CFSys->find(*j);
						while(temp.name.substr(0,3)=="SLT"){
							double tVol= temp.vol;
							temp= CFSys->find(temp.sName);
							temp.vol=tVol;
						}
						if(temp.name!=""){
							instructs.push_back(temp);
						}
						else
							cout<<*j<<": Node Not found!"<<endl;
					}
					cout<<"heat "<<it->first<<" "<<instructs[0].name<<" "<<it->second.volume<<" "<<hTime<<endl;
					CFSys->fluidInstr("heat",it->first,instructs[0].name,it->second.volume,hTime);
					instructs.clear();//Maintenance of the instruction vector.
				}

			}
			if(steps[i].store){
				//RIGHT NOW THIS DOESNT TRANSLATE RIGHT NOW.

				/*out<< "  Store: "<<endl;
				for( map<string,aStore>::iterator it= steps[i].storeList.begin();
						it !=steps[i].storeList.end();++it)
				{
					out << it->second.fluids.name << " for ";
					out << it->second.time.value<< " ";
					switch(it->second.time.unit_choice){
					case 0: out<<"S"; break;
					case 1: out<<"Min(s)"; break;
					case 2: out<< "Hr(s)"; break;
					}
					out << " at " << it->second.temp <<" degrees C";
					out<< " ("<< it->first<<")"<<endl;
				}*/
			}
			if(steps[i].split){

				for(map<string,aSplit>::iterator it= steps[i].splitList.begin();
						it != steps[i].splitList.end();++it)
				{

					map<string,list <string> > :: iterator it2 =links.find(it->first);//finds name of parents
					for (list <string>::iterator j = it2->second.begin(); j != it2->second.end(); ++j){
						infoNode temp= CFSys->find(*j);
						if(temp.name!=""){
							infoNode t2;
							t2.name= it->first;
							t2.sName =*j;
							t2.vol=it->second.volume;
							CFSys->nodes.push_back(t2);
						//	cout<< it->first << " -> " << *j<<endl;
						}
						else
							cout<<*j<<": Node Not found!"<<endl;
					}
				}
				instructs.clear();//Maintenance of the instruction vector.
			}

			if(steps[i].detect){

				for(map<string,aDetect>::iterator it= steps[i].detectList.begin();
						it != steps[i].detectList.end();++it)
				{
					double dTime; //makes sure time is not zero
					if(it->second.time.conv2secs()==0)
						dTime=defaultNum;
					else
						dTime=it->second.time.conv2secs();

					CFSys->nodes.push_back(infoNode(it->first,it->second.volume,dTime));
					map<string,list <string> > :: iterator it2 =links.find(it->first);//finds name of parents
					for (list <string>::iterator j = it2->second.begin(); j != it2->second.end(); ++j){
						infoNode temp= CFSys->find(*j);
						while(temp.name.substr(0,3)=="SLT"){
							double tVol= temp.vol;
							temp= CFSys->find(temp.sName);
							temp.vol=tVol;
						}
						if(temp.name!=""){
							instructs.push_back(temp);
						}
						else
							cout<<*j<<": Node Not found!"<<endl;
					}
					cout<<it->second.detectType<<" "<<it->first<<" "<<instructs[0].name<<" "<<it->second.volume<<" "<<dTime<<endl;
					CFSys->fluidInstr(it->second.detectType,it->first,instructs[0].name,it->second.volume,dTime);
					instructs.clear();//Maintenance of the instruction vector.
				}
			}
			if(steps[i].output){
				/*out<< "  Output: "<<endl;
				for(map<string,aOutput>::iterator it= steps[i].outputList.begin();
						it != steps[i].outputList.end();++it)
					out<<"     "<<"Sink Name: "<< it->second.sinkName<<endl;
			*/}
		}
		//out<<endl;

}

int assayProtocol :: getDropCount()
{
	int count=0;
	for(int i=0 ; i<completeMixNum ;++i){
		map<string, Container>::iterator it = conLookUp.find(completeMixList[i].container);
		if(it!= conLookUp.end()){
			count++;
		}
	}
	return count;
}


