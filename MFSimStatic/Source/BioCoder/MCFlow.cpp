/*
 * MCFlow.cpp
 *
 *  Created on: Apr 23, 2013
 *      Author: Chris
 */

#include "../../Headers/BioCoder/MCFlow.h"


void MCFlow:: fluidInstr(string instruct, string outputName, string input1, double vol1, string input2,double vol2, double time){
	out << instruct<<" "<< outputName << " " << input1 << " " << vol1 << " " << input2 << " " << vol2 << " " << time << endl;
}
void MCFlow:: fluidInstr(string instruct, string outputName, string input1, double vol1, double time){
	out << instruct<<" "<< outputName << " " << input1 << " " << vol1 << " " << time << endl;
}

void MCFlow::createNewLine(){
	out<<endl;
}
infoNode MCFlow:: find(string n){

		for(unsigned i=0; i<nodes.size();++i){
			if(nodes[i].name==n){
				return nodes[i];
			}
		}

		return infoNode();
	}
