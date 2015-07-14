/*
 * MCFlow.h
 *
 *  Created on: Apr 23, 2013
 *      Author: Chris
 */

#ifndef MCFLOW_H_
#define MCFLOW_H_
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>


using namespace std;
struct infoNode{
	string name;
	string sName;
	double vol;
	double time;
	infoNode(){
		name="";
		sName="";
		vol=0;
		time=0;
	}
	infoNode(string n, double v,double t){
		name=n;
		vol=v;
		time=t;
	}
};
class MCFlow{

	ofstream out;
public:
	vector<infoNode> nodes;
	MCFlow(){
		out.open("BioCoderOutPut.abs");
	}
	MCFlow(string s){
		string end=".abs";
		string file=s+end;
		out.open(file.c_str());
	}

	void fluidInstr(string instruct,string outputName, string input1, double vol1, string input2, double vol2, double time);
	void fluidInstr(string instruct,string outputName, string input1, double vol1, double time);
	void createNewLine();
	infoNode find(string n);
};

#endif /* MCFLOW_H_ */
