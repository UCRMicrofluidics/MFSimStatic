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
#include<cstdlib>
#include "../../Headers/BioCoder/BioCoderStructs.h"
#include <iostream>
#include <sstream>
#include "../../Headers/BioCoder/TimeVolumeSpeed.h"
#include "../../Headers/BioCoder/BioEnums.h"

Fluid::Fluid()
:node(NULL),original_name(NULL),new_name(NULL),state(""),volume(0),unit(L),type(FLUID), used(0),usage_index(0)
{}
Fluid::Fluid(const Fluid & f)
:node(f.node),original_name(f.original_name),new_name(f.new_name),container(f.container),state(f.state),volume(f.volume),unit(f.unit), type(f.type),used(f.used),usage_index(f.usage_index)
{}

Container::Container()
:node(NULL),contents(), volume(0), name(""),type(FLUID),used(0),usage_index(0)
{}

Container::Container(const Container & c)
:node(c.node),id(c.id),contents(c.contents),volume(c.volume),name(c.name),type(c.type),used(c.used),usage_index(c.usage_index),tLinkName(c.tLinkName)
{}
tNameBank:: tNameBank(string fileName)
{
	nodeNames[0] = "DIS1"; /*Dispense*/
	nodeNames[1] = "MIX1"; /*Mix*/
	nodeNames[2] = "HT_1"; /*Heat*/
	nodeNames[3] = "SLT1"; /*Split*/
	string detect= "DCT1";
	nodeNames[4] = detect; /*Detect*/
	nodeNames[5] = "OUT1"; /*Output*/
	nodeNames[6] = "STR1"; /*Store*/
	nodeNames[7] = "WST1"; /*Waste*/
	string dags= "DAG1";
	nodeNames[8] = dags;/*dags*/
	nodeNames[9] = "TIN1";/*transfer in Node*/
	nodeNames[10]= "TOT1";/*transfer Out Node*/

}
void tNameBank:: incNodeName(TLinkNames t)
{

	string node= nodeNames[t];
	string letter= node.substr(0,3);
	string numbers =node.substr(3);
	const char * b(numbers.c_str());
	int num=atoi(b);
	stringstream ss;
	ss<<++num;
	numbers=ss.str();
	node=node.substr(0,3)+numbers;
	nodeNames[t]=node;
}

string tNameBank:: accessName(TLinkNames t)
{
	return nodeNames[t];
}


aDetect:: aDetect()
:detectType(""), time(Time(0,SECS)), volume(0)
{}
aDetect:: aDetect (string type, Time t)
:detectType(type), time(t), volume(0)
{}

aDispense :: aDispense()
:vol(),flu()
{}

aDispense :: aDispense(Fluid f, Volume v)
:vol(v),flu(f)
{}
aMixture::aMixture()
:listNum(0),name(""),container(""), volume(0)
{
}

aMixture aMixture:: find(Container& con, int size)
{
	for(int i=0;i<size;++i)
	{
		if(this[i].container==con.name)
			return this[i];
	}
	aMixture a;
	return a;
}
aMixture aMixture:: addToMixList( Container &con,const Fluid& fluid,int& size)
{
	for(int i=0;i<size;i++)
	{
		if(this[i].container==con.name)
		{
			for(int j=0;j<this[i].listNum;++j)
			{
				if(this[i].list[j].original_name==fluid.original_name)
				{
					//setVol();
					return this[i];
				}
			}
			this[i].list[this[i].listNum++]=Fluid(fluid);
			//setVol();
			return this[i];
		}
	}
	this[size].container=con.name;
	this[size].list[this[size].listNum]=Fluid(fluid);
	this[size].list[this[size++].listNum++].volume;
	//		setVol();
	return this[size-1];
}
aMixture aMixture:: addToMixList(Container & src, Container& dest,int& size)
{
	aMixture  contSrc,contDest;
	if(src.contents.type!= SOLID)
		addToMixList(src, src.contents,size);
	if(dest.contents.type!= SOLID)
		addToMixList(dest,dest.contents,size);
	for(int i=0;i<size;++i)
	{
		if (this[i].container==src.name)
		{
			contSrc=this[i];
			this[i].container="";
			break;
		}
	}

	for (int i=0;i< contSrc.listNum;++i)
	{
		addToMixList(dest,contSrc.list[i],size);
	}
	return *this;
}
void aMixture:: changeMixName(Container& con,char* name,int size)
{
	for (int i=0;i<size;++i)
	{
		if(this[i].container == con.name)
		{
			this[i].name=name;
			return;
		}
	}
}

double aMixture::getTotalVol()
{
	return volume;
}

Volume aMixture::add(Volume vol1,Volume vol2)
{
	double v1=0;
	double v2=0;
	v1=vol1.mul*vol1.value;
	v2=vol2.mul*vol2.value;
	return Volume((v1+v2), UL);
}

aMix::aMix()
:typeofmix(""), volume(0)
{}

aMix::aMix(aMixture m, string n)
:component(m),typeofmix(n), volume(0)
{}

aHeat :: aHeat()
:time(Time(0,SECS)),temp(25000000), volume(0)
{}

aHeat::aHeat(Time t,double tmp,aMixture mix)
:time(t),temp(tmp),fluids(mix), volume(0)
{}

aSplit::aSplit()
:numSplit(0), volume(0), CF(false)
{}

aSplit::aSplit(int n)
:numSplit(n), volume(0), CF(false)
{}

aStore::aStore()
:time(Time(0,SECS)),temp(25000000), volume(0)
{}

aStore::aStore(Time t,double tmp,aMixture mix)
:time(t),temp(tmp),fluids(mix), volume(0)
{}

aOutput :: aOutput()
:sinkName(""),nodeName("")
{}

aOutput :: aOutput(string sink, string node)
:sinkName(sink),nodeName(node)
{}

aTransOut::aTransOut()
:con(Container())
{}

aTransOut::aTransOut(Container &cont)
:con(cont)
{}

aTransIn::aTransIn()
: lName("")
 {}
aTransIn::aTransIn(string linkName)
:lName(linkName)
	{}

