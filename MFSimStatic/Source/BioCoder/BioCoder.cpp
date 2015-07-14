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
#include <iostream>
#include <assert.h>
#include <cstdio>
extern "C"
{
#include <stdarg.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sstream>
//#include "../../Headers/assay_node.h"
////#include "../../Headers/system.h"
//#include "../../Headers/enums.h"
//#include "../../Headers/dag.h"
#include "../../Headers/BioCoder/BioCoder.h"
#include "../../Headers/BioCoder/BioEnums.h"
#include "../../Headers/BioCoder/MCFlow.h"
#include"../../Headers/BioCoder/BioCoderStructs.h"
//#include"../../Headers/BioCoder/BioConditionalGroup.h"



int BioCoder :: get_ttr()
{
	return total_time_required;
}

BioCoder :: BioCoder(string name)
{

	total_time_required = 0;
	options_flag = 0;
	option_no = 1;
	print_reagents = 1;
	print_parameters = 1;
	first = 0;
	/*options_flag = 0;
 option_no = 1;*/
	sub_step_flag = 0;
	prev_cont = 1;
	//initiallization of number of containers, equipment, etc.
	equip_no = 0;
	microfuge_no = 1;
	centrifuge_no = 1;
	incubator_no = 1;
	electrophoresis_no = 1;
	mortar_no = 1;
	thermocycler_no = 1;
	electro_no = 1;
	shaker_no = 1;
	spectrophotometer_no = 1;
	cont_no = 0;

	container_count = new int[24];
	for(int i=0; i<24;++i)
		container_count[i] = 0;

	containers= new char*[20];
	equipments= new char*[20];


	//variables required for keeping track of usage of fluids and containers
	list_fluid_no = 0;
	list_container_no = 0;

	all_nodes = new graphNodelist;
	usage_list_fluids = new Fluid[10000];
	usage_list_containers = new Container[10000];
	proSteps= new assayProtocol(name);
	start_protocol(name);
}
BioCoder :: ~BioCoder()
{
	for(list<Time*>::iterator it =delTimePtrs.begin();it!=delTimePtrs.end();++it)
	{
		delete *it;
	}
	for(list<Speed*>::iterator it =delSpeedPtrs.begin();it!=delSpeedPtrs.end();++it)
	{
		delete *it;
	}
	for(list<Volume*>::iterator it =delVolumePtrs.begin();it!=delVolumePtrs.end();++it)
	{
		delete *it;
	}
	delete containers;
	delete equipments;
	delete container_count;
	delete proSteps;
	delete usage_list_containers;
	delete usage_list_fluids;
	delete all_nodes;
}

void BioCoder :: start_protocol(string name)
{
	filename = name;
	string fName = "Output/Bio_Instruc_";
	fName+= filename;
	fName+= ".htm";
	fp = fopen(fName.c_str(), "w");
	fprintf(fp, "<h1 style=\"font-size = 25px;\">%s</h1>", name.c_str());
}
void BioCoder :: export_graph(const char* filename) {
	char* color;
	float color_val, ttr;
	string fName = "Output/Bio_Instruc_";
	fName+= filename;
	fName+= ".dot";
	FILE* fp = fopen(fName.c_str(), "w");
	graphNodelist *list = all_nodes;
	int mul;
	// print header
	fprintf(fp, "digraph G {\n");
	fprintf(fp, "  size=\"8.5,10.25\";\n");
	// every node prints itself and its outgoing edges
	while (list->node != NULL) {
		graphNode *node = list->node;
		graphNodelist *out = node->out;
		switch(node->type){
		case 0:fprintf(fp, "%d [label=\"%s\" color=blue];\n", node->id, node->name);break;
		case 1:fprintf(fp, "%d [label=\"%s\" color=blue];\n", node->id, node->name);break;
		case 2:fprintf(fp, "%d [label=\"%s\" shape=octagon color=red];\n", node->id, node->name);break;
		case 3: if((node->unit== "sec")||(node->unit== "secs"))
			mul = 1;
		else if((node->unit== "min")||(node->unit== "mins"))
			mul = 60;
		else
			mul = 3600;
		ttr = get_ttr();
		color_val = (node->time*mul)/ttr*100;
		if (color_val <= 10)
			color = ".59166 .61 .7";
		else if ((color_val >10)&&(color_val <= 20))
			color = ".55833 .44 .81";
		else if ((color_val >20)&&(color_val <= 30))
			color = ".53888 .26 .91";
		else if ((color_val >30)&&(color_val <= 40))
			color = ".53333 .09 .97";
		else if ((color_val >40)&&(color_val <= 50))
			color = ".51694 .24 1.0";
		else if ((color_val >50)&&(color_val <= 60))
			color = ".11666 .43 .99";
		else if ((color_val >60)&&(color_val <= 70))
			color = ".07777 0.61 .99";
		else if ((color_val >70)&&(color_val <= 80))
			color = ".03888 .72 .95";
		else
			color = ".00555 .81 .84";

		if (node->pcr_or_centrifuge == 1)
		{
			if(node->time == 0)
				fprintf(fp, "%d [shape = box, style = filled, color = \"%s\",label = \"%s\"];\n", node->id, color, node->name);
			else
				//if (node->unit == "mins"||node->unit == "min") mul = 0.01;
				//else if (node->unit == "hrs"||node->unit == "hr") mul = 0.1;
				fprintf(fp, "%d [shape = box, style = filled, color = \"%s\", label = \"%s (Duration: %g %s)\"];\n", node->id, color, node->name, node->time, node->unit);
		}
		else{
			if(node->time == 0)
				fprintf(fp, "%d [label = \"%s\", shape = box];\n", node->id, node->name);
			else
			{
				//if (node->unit == "mins"||node->unit == "min") mul = 0.01;
				//else if (node->unit == "hrs"||node->unit == "hr") mul = 0.1;
				fprintf(fp, "%d [shape = box, style = filled, color = \"%s\", label = \"%s (Duration: %g %s)\"];\n", node->id, color, node->name, node->time, node->unit);
			}
		}
		break;
		default:break;
		}
		while (out->node != NULL) {
			fprintf(fp, "%d -> %d;\n", node->id, out->node->id);
			if (out->next == NULL) break;
			out = out->next;
		}
		if (list->next == NULL) break;
		list = list->next;
	}
	fprintf(fp, "}\n");
	fclose(fp);
}
//void BioCoder:: exportDagGraph(assayProtocol pro)
//{
//	char* temp1 = "";
//	temp1 = (char *)calloc(strlen(filename)+ 10, sizeof(char));
//	temp1 = strcat(temp1, filename);
//	temp1 = strcat(temp1, "DagGraph");
//	temp1 = strcat(temp1, ".dot");
//	FILE* fp = fopen(filename, "w");
//	int step =0;
//	fprintf(fp, "digraph G {\n");
//	fprintf(fp, "  size=\"8.5,10.25\";\n");
//	for( int i=0; i < pro.stepNum;++i)
//	{
//		if(pro.steps[i].dispence){
//
//		}
//	}
//
//}

void BioCoder :: combine_helper (int count, Container* container1)
{
	int i;
	Container sample, result1;
	char* plus = "+";
	char* str3 = "";
	char* str4 = "";
	Fluid o;
	// graph  maintenance
	{
		result1.node = all_nodes-> create_node("container with mixture");
		o = new_operation("combine");
		all_nodes-> create_edge_from_fluids(&o, &result1);
	}
	for(i=0; i<count; i++)
	{
		if (i == 0)
		{
			sample = container1[0];
			fprintf(fp, "Combine the %s", sample.contents.new_name);
			result1.contents.new_name = container1[0].contents.new_name;
			result1.volume = container1[0].volume;
		}
		else {
			proSteps-> assayPreMix(container1[i],container1[0]);
			sample =  container1[i];
			fprintf(fp, ",%s", sample.contents.new_name);
			str3 = (char *)calloc(strlen(str3) + strlen(sample.contents.new_name) + 2, sizeof(char));
			strcat(str3, plus);
			strcat(str3, sample.contents.new_name);
			result1.volume = result1.volume + sample.volume;
		}
		// graph maintenance
		{
			all_nodes-> create_edge_from_container_to_fluid(&sample, &o);
		}

	}

	str4=(char *)calloc(strlen(result1.contents.new_name) + strlen(str3) + 1, sizeof(char));
	strcat(str4, result1.contents.new_name);
	strcat(str4, str3);
	result1.contents.new_name = str4;

	container1[0] = result1;
	fprintf(fp, "<br>");
}


void BioCoder :: mix (Container &container1, enum mixing type, Time time1)
{
	string type1;
	aMixture mixStep;

	//	if(container1.contents.type!= SOLID)
	//		mixStep = proSteps->addtoMixtureList(container1,container1.contents);
	//	else
	mixStep = proSteps->findMix(container1);
	//proSteps.steps[proSteps.stepNum].mix=true;
	mixStep.time = time1;
	switch(type){
	case TAPPING: {
		fprintf(fp, "Gently tap the mixture for ");
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, " .<br>");
		type1 = "Tapping";
		break;
	}
	case STIRRING: {
		fprintf(fp, "Stir the mixture for ");
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, " .<br>");
		type1 = "Stirring";
		break;
	}
	case INVERTING: {
		fprintf(fp, "Close the tube tightly and invert the tube ");
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, " times.<br>");
		type1 = "Inverting";
		break;
	}
	case VORTEXING:{
		fprintf(fp, "Vortex the mixture for ");
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, " .<br>");
		type1 = "Vortexing";
		break;
	}
	type1 = "";
	fprintf(fp, "Invalid entry.<br>");
	}
	// graph maintenance
	{
		Fluid o = new_operation("mix");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container with contents mixed");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
	proSteps->assayMixManagement(container1 ,mixStep,type1);

}

void BioCoder :: mix (Container &container1, enum mixing type, int times)
{
	string type1;
	aMixture mixStep;
	mixStep =  proSteps->findMix(container1);
	switch(type){
	case INVERTING:
		fprintf(fp, "Close the tube tightly and invert the tube <b><font color=#357EC7>%d times</font></b>.<br>", times);
		type1 = "Inverting";
		break;
	default:
		type1 = "";
		fprintf(fp, "Invalid entry.<br>");
	}
	proSteps->assayMixManagement(container1 ,mixStep,type1);
	// graph maintenance
	{
		Fluid o = new_operation("mix");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container with contents mixed");
		all_nodes-> create_edge(o.node, container1.node);
	}
}

void BioCoder :: mix (Container &container1, enum mixing type, int min_times, int max_times)
{
	string type1;
	aMixture mixStep;
	//	if(container1.contents.type!= SOLID)
	//		mixStep = proSteps->addtoMixtureList(container1,container1.contents);
	//	else
	mixStep = proSteps->findMix(container1);
	//proSteps.steps[proSteps.stepNum].mix=true;
	switch(type){
	case INVERTING:
		type1 = "Inverting";

		break;
	default:
		type1 = "";

		fprintf(fp, "Invalid entry.<br>");
	}
	proSteps->assayMixManagement(container1 ,mixStep,type1);
	// graph maintenance
	{
		Fluid o = new_operation("mix");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes->create_node("container with contents mixed");
		all_nodes-> create_edge(o.node, container1.node);
	}
}

void BioCoder :: mix (Container &container1, enum mixing type)
{
	string type1;
	aMixture mixStep;

mixStep.volume=container1.volume;
//cout<<mixStep.volume<<endl;
//if(container1.contents.type!= SOLID)
	//mixStep = proSteps->addtoMixtureList(container1,container1.contents);
	//else
	mixStep = proSteps->findMix(container1);

	switch(type){
	case TAPPING: fprintf(fp, "Gently tap the mixture for a few secs.<br>");
	type1 = "Tapping";
	break;
	case STIRRING: fprintf(fp, "Stir the mixture for a few secs.<br>");
	type1 = "Stirring";
	break;
	case INVERTING: fprintf(fp, "Close the tube tightly and gently mix the contents by inverting the tube.<br>");
	type1 = "Inverting";
	break;
	case VORTEXING: fprintf(fp, "Vortex the mixture for a few secs.<br>");
	type1 = "Vortexing";
	break;
	case RESUSPENDING:fprintf(fp, "Resuspend pellet by vortexing/by shaking vigorously.<br>");
	type1 = "Resuspending";
	break;
	case DISSOLVING:fprintf(fp, "Dissolve the pellet in the solution.<br>");
	type1 = "Dissolving";
	break;
	case PIPETTING:fprintf(fp, "Mix solution by pipetting up and down several times.<br>");
	type1 = "Pipetting";
	break;
	default: fprintf(fp, "Invalid entry.<br>");
	type1 = "";
	}
	proSteps->assayMixManagement(container1 ,mixStep,type1);
	// graph maintenance
	{
		Fluid o = new_operation("mix");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container with contents mixed");
		all_nodes-> create_edge(o.node, container1.node);
	}
}

void BioCoder :: mix (Container &container1, enum until event1)
{
	string type1;
	aMixture mixStep;
	//	if(container1.contents.type!= SOLID)
	//		mixStep = proSteps->addtoMixtureList(container1,container1.contents);
	//	else
	mixStep = proSteps->findMix(container1);
	//proSteps.steps[proSteps.stepNum].mix=true;
	type1="";
	proSteps->assayMixManagement(container1 ,mixStep,type1);
	if (event1 == PPT_STOPS_STICKING)
		fprintf(fp, "Close the tube tightly and gently mix the contents by inverting the tube until precipitate stops sticking to walls of the tube.<br>");
	if (event1 == PELLET_DISLODGES)
		fprintf(fp, "Gently mix the contents of the tube until the pellet dislodges.<br>");
	// graph maintenance
	{
		Fluid o = new_operation("mix");
		all_nodes->create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes->create_node("container with contents mixed");
		all_nodes-> create_edge(o.node, container1.node);
	}
}

void BioCoder :: first_step()
{
	first = 1;
	fprintf(fp, "</ul><h2>Steps:</h2><ol>");
	fprintf(fp, "<p><li>");
}

string BioCoder::getName()
{
	return filename;
}
assayProtocol * BioCoder:: getProtocolData()
{
	return proSteps;
}
string BioCoder:: save_fluid(Container & con)
{
	string linkName= proSteps->assayTransOut(con);
	proSteps->addToLinkMap(con, linkName);
	proSteps->clearMixture(con);
	return linkName;
}
string BioCoder:: reuse_fluid(Container & con)
{
	string linkName = proSteps->assayTransIn();
	con.tLinkName.push_back(linkName);
	return linkName;
}
void BioCoder :: first_step(char* step_name)
{
	first = 1;
	fprintf(fp, "</ul><h2>Steps:</h2><ol>");
	fprintf(fp, "<p><li><b><font size=3>%s</font></b><br>", step_name);
}
void BioCoder :: next_step(char* step_name)
{
	proSteps->stepNum++;
	first = 0;
	if (sub_step_flag == 1)
	{
		fprintf(fp,"</li></p></ol>");
		sub_step_flag = 0;
	}
	fprintf(fp, "</li></p><p><li><b><font size=3>%s</font></b><br>", step_name);
}

void BioCoder :: next_step()
{
	proSteps->stepNum++;
	first = 0;
	if (sub_step_flag == 1)
	{
		fprintf(fp, "</li></p></ol>");
		sub_step_flag = 0;
	}
	fprintf(fp, "</li></p><p><li>");
}

void BioCoder :: parallel_step()
{
	first = 0;
	if (sub_step_flag == 1)
	{
		fprintf(fp, "</li></p></ol>");
		sub_step_flag = 0;
	}
	fprintf(fp, "</li></p><p><li><b>Meanwhile:</b><br>");
}

void BioCoder :: first_sub_step()
{
	first = 0;
	fprintf(fp, "<ol type=\"a\"><p><li>");
	sub_step_flag = 1;
}
void BioCoder :: next_sub_step()
{
	fprintf(fp, "</li></p><p><li>");
	sub_step_flag = 1;
}
/*void BioCoder :: repeat(int step_no)
{
	fprintf(fp, "Repeat Step %d. <br>", step_no);
}
 */
void BioCoder :: display_equip()
{
	int i;
	fprintf(fp, "<div style=\"top: 25px; margin-top: 50px; margin-left: 700px; position: absolute; z-index: 1; visibility: show;\">");
	fprintf(fp, "<h2>Equipment:</h2><ul type=\"circle\">");
	for(i=0; i<equip_no; i++)
		fprintf(fp, "<li>%s</li>", equipments[i]);
	for(i=0; i<cont_no; i++)
		fprintf(fp, "<li>%s</li>", containers[i]);
	fprintf(fp, "</ul></div>");
}
void BioCoder :: usage_details()
{
	int i;
	int count1 = 0;
	int count2 = 0;
	for(i=0;i<list_fluid_no;i++)
	{
		if (usage_list_fluids[i].used == 1)
			continue;
		if (count1 == 0)
		{
			fprintf(fp, "<font color = red>Warning: The following reagents/materials were declared but not used in the protocol:</font><ul>");
			fprintf(fp, "<li>%s</li>", usage_list_fluids[i].original_name);
			count1++;
		}
		else
			fprintf(fp, "<li>%s</li>", usage_list_fluids[i].original_name);
	}
	fprintf(fp, "</ul>");
	for(i=0;i<list_container_no;i++)
	{
		if (usage_list_containers[i].used == 1)
			continue;
		if (count2 == 0)
		{
			fprintf(fp, "<font color = red>Warning: The following equipment were declared but not used in the protocol:</font><ul>");
			fprintf(fp, "<li>%s</li>", usage_list_containers[i].name);
			count1++;
		}
		else
			fprintf(fp, "<li>%s</li>", usage_list_containers[i].name);
	}
	fprintf(fp, "</ul>");
}
void BioCoder :: timing_details()
{
	int time_hrs = total_time_required/3600;
	int time_mins = (total_time_required%3600)/60;
	if (time_hrs == 0)
		fprintf(fp, "<p><b>TOTAL TIME REQUIRED FOR THE COMPLETION OF THE PROTOCOL :<font color=#357EC7>~ %d mins</font></b></p>", time_mins);
	else if (time_hrs == 1)
		fprintf(fp, "<p><b>TOTAL TIME REQUIRED FOR THE COMPLETION OF THE PROTOCOL :<font color=#357EC7>~ %d hr, %d mins</font></b></p>", time_hrs, time_mins);
	else
		fprintf(fp, "<p><b>TOTAL TIME REQUIRED FOR THE COMPLETION OF THE PROTOCOL :<font color=#357EC7>~ %d hrs, %d mins</font></b></p>", time_hrs, time_mins);
	total_time_required = 0;
}
void BioCoder :: end_protocol()
{
	proSteps->collectOutput();
	//int i;
	export_graph(filename.c_str());
	fprintf(fp, "</li></p></ol>");
	display_equip();
	usage_details();
	//timing_details();
	print_reagents = 1;
	print_parameters = 1;
	option_no = 1;
	equip_no = 0;
	microfuge_no = 1;
	centrifuge_no = 1;
	incubator_no = 1;
	electrophoresis_no = 1;
	mortar_no = 1;
	thermocycler_no = 1;
	electro_no = 1;
	shaker_no = 1;
	spectrophotometer_no = 1;
	cont_no = 0;
	prev_cont = 1;
	list_fluid_no = 0;
	total_time_required = 0;
	list_container_no = 0;
	fclose(fp);
}

void BioCoder :: check_container(Container& container1)
{
	if ((first == 1) || (prev_cont == 1)||(prev_container != container1.name))
	{
		prev_container = container1.name;
		fprintf(fp, "%s", container1.name);
	}
}
void BioCoder :: to_do(char* clarify)
{
	fprintf(fp, "<font color = \"red\"><i>%s</i></font><br>", clarify);
}
void BioCoder :: comment(char* comm)
{
	fprintf(fp, "<font color = \"#800517\"><i>%s</i></font><br>", comm);
}

Fluid BioCoder :: new_fluid(char *name)
{
	Fluid result;
	result.state = "";
	result.original_name = name;
	result.new_name = name;
	result.type = FLUID;
	result.volume = DEFAULT;
	result.unit = L;
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	fprintf(fp, "<li>%s</li>", name);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid(char*name, float temp)
{
	char* t1="";
	Fluid result;
	result.original_name = name;
	result.new_name = name;
	result.state="";
	result.type = FLUID;
	result.volume = DEFAULT;
	result.unit = L;
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	if(temp == ICE_COLD)
	{
		fprintf(fp, "<li>ice-cold %s</li>", name, temp);
		t1 = (char *)calloc(strlen(t1) + strlen(name) + 10,	sizeof(char));
		strcat(t1, "ice-cold ");
		strcat(t1, name);
		result.new_name = t1;
	}
	else if(temp == RT)
		fprintf(fp, "<li>%s stored at room temperature</li>", name);
	else
		fprintf(fp, "<li>%s stored at %g%cC</li>", name, temp, 0x00B0);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid(char*name, char* state)
{
	char* t1 = "";
	Fluid result;
	result.original_name = name;
	result.new_name = name;
	result.state= state;
	result.type = FLUID;
	result.volume = DEFAULT;
	result.unit = L;
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	fprintf(fp, "<li> <a name=\"%s\">%s <i><br><tab><div style=\"margin-right: 600px;\">(%s)</div></i></a></li>", name, name, state);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid (char* name, char* state, float temp)
{
	char* t1="";
	Fluid result;
	result.original_name = name;
	result.new_name = name;
	result.state= state;
	result.type = FLUID;
	result.volume = DEFAULT;
	result.unit = L;
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	if(temp == ICE_COLD)
	{
		t1 = (char *)calloc(strlen(t1) + strlen(name) + 10, sizeof(char));
		strcat(t1, "ice-cold ");
		strcat(t1, name);
		result.new_name = t1;
		fprintf(fp, "<li> <a name=\"%s\"> %s <i><br><tab><div style=\"margin-right: 600px;\">(%s)</div></i></a></li>", result.new_name, result.new_name, state);
	}
	else if(temp >= 35)
		fprintf(fp, "<li> <a name=\"%s\">%s preheated in a water bath set at <b><font color=#357EC7>%g%cC</font></b> <i>%s</i></a></li>", name, name, temp, 0x00B0, state);
	else
		fprintf(fp, "<li> <a name=\"%s\">%s <i>%s</i> </a> at <b><font color=#357EC7>%g%cC</font></b></li>", name, name, state, temp, 0x00B0);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid(char *name, Volume volume1)
{
	char* t1 = "";
	char* t2 = "";
	Fluid result;
	result.state = "";
	result.type = FLUID;
	result.unit = vol_unit(volume1.unit_choice);
	result.volume = volume1.value * volume1.mul;
	result.original_name = name;
	result.new_name = name;
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	fprintf(fp, "<li>%s</li>", name);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid(char*name, float temp, Volume volume1)
{
	char* t1 = "";
	char* t2 = "";
	Fluid result;
	result.state="";
	result.type = FLUID;
	result.original_name = name;
	result.new_name = name;
	result.volume = volume1.value * volume1.mul;
	result.unit = vol_unit(volume1.unit_choice);
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	if(temp == ICE_COLD)
	{
		fprintf(fp, "<li>ice-cold %s</li>", name, temp);
		t1 = (char *)calloc(strlen(t1) + strlen(name) + 10,	sizeof(char));
		strcat(t1, "ice-cold ");
		strcat(t1, name);
		result.new_name = t1;
	}
	else if(temp == RT)
		fprintf(fp, "<li>%s stored at room temperature</li>", name);
	else
		fprintf(fp, "<li>%s stored at %g%cC</li>", name, temp, 0x00B0);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid(char*name, char* state, Volume volume1)
{
	char* t1 = "";
	char* t2 = "";
	Fluid result;
	result.state= state;
	result.original_name = name;
	result.new_name = name;
	result.type = FLUID;
	result.volume = volume1.value * volume1.mul;
	result.unit = vol_unit(volume1.unit_choice);
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	fprintf(fp, "<li> <a name=\"%s\">%s <i><br><tab><div style=\"margin-right: 600px;\">(%s)</div></i></a></li>", name, name, state);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}

Fluid BioCoder :: new_fluid (char* name, char* state, float temp, Volume volume1)
{
	char* t1 = "";
	char* t2 = "";
	Fluid result;
	result.state= state;
	result.original_name = name;
	result.new_name = name;
	result.type = FLUID;
	result.volume = volume1.value * volume1.mul;
	result.unit = vol_unit(volume1.unit_choice);
	if(print_reagents == 1)
	{
		fprintf(fp, "<h2 style=\"margin-top:50px;\">Solutions/reagents:</h2><ul type=\"circle\">");
		print_reagents++;
	}
	if(temp == ICE_COLD)
	{
		t1 = (char *)calloc(strlen(t1) + strlen(name) + 10, sizeof(char));
		strcat(t1, "ice-cold ");
		strcat(t1, name);
		result.new_name = t1;
		fprintf(fp, "<li> <a name=\"%s\"> %s <i><br><tab><div style=\"margin-right: 600px;\">(%s)</div></i></a></li>", result.new_name, result.new_name, state);
	}
	else if(temp >= 35)
		fprintf(fp, "<li> <a name=\"%s\">%s preheated in a water bath set at <b><font color=#357EC7>%g%cC</font></b> <i>%s</i></a></li>", name, name, temp, 0x00B0, state);
	else
		fprintf(fp, "<li> <a name=\"%s\">%s <i>%s</i> </a> at <b><font color=#357EC7>%g%cC</font></b></li>", name, name, state, temp, 0x00B0);
	result.used = 0;
	usage_list_fluids[list_fluid_no] = result;
	result.usage_index = list_fluid_no;
	list_fluid_no++;
	return result;
}
Fluid BioCoder :: new_operation(char* name)
{
	Fluid result;
	result.new_name = name;
	result.original_name = name;
	result.type = OPERATION;
	return result;
}
void BioCoder :: store_container_names (int i, char* name)
{
	if(container_count[i] == 0)
	{
		containers[cont_no] = name;
		cont_no++;
	}
}

Container BioCoder :: new_container(enum container_type cont_id)
{
	Container result;
	result.type = CONTAINER;
	result.volume = 0;
	result.contents.new_name = "";
	char* temp;
	char* temp1 = (char *)calloc(50, sizeof(char));
	char* temp2 = (char *)calloc(50, sizeof(char));
	result.id = cont_id;
	switch(cont_id){
	case STERILE_MICROFUGE_TUBE:sprintf(temp2, " (%d)", container_count[0]+1); temp1 = strcat(temp1, "sterile 1.5-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Sterile 1.5-ml microcentrifuge tubes"; store_container_names(0, temp); container_count[0]++;break;
	case CENTRIFUGE_TUBE_15ML:sprintf(temp2, " (%d)", container_count[1]+1); temp1 = strcat(temp1, "sterile 15-ml centrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Sterile 15-ml centrifuge tubes";store_container_names(1, temp);container_count[1]++;break;
	case FLASK:sprintf(temp2, " (%d)", container_count[2]+1); temp1 = strcat(temp1, "flask");temp1 = strcat(temp1, temp2);result.name = temp1; temp = "Flasks of appropriate volumes";store_container_names(2, temp);container_count[2]++;break;
	case CENTRIFUGE_BOTTLE:sprintf(temp2, " (%d)", container_count[3]+1); temp1 = strcat(temp1, "centrifuge bottle");temp1 = strcat(temp1, temp2);result.name = temp1; temp = "Centrifuge bottles";store_container_names(3, temp);container_count[3]++;break;
	case GRADUATED_CYLINDER:result.name = "a graduated cylinder"; temp = "Graduated cylinders";store_container_names(4, temp);container_count[4]++; break;
	case RXN_TUBE:sprintf(temp2, " (%d)", container_count[5]+1); temp1 = strcat(temp1, "reaction tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Reaction tubes"; store_container_names(5, temp);container_count[5]++; break;
	case FRESH_COLL_TUBE:sprintf(temp2, " (%d)", container_count[6]+1); temp1 = strcat(temp1, "fresh collection tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Collection tubes";store_container_names(6, temp);container_count[6]++; break;
	case LIQUID_NITROGEN:result.name = "a container with liquid nitrogen"; temp = "Container with liquid nitrogen";store_container_names(7, temp);container_count[7]++; break;
	case PLG:result.name = "a 50- ml PLG tube"; temp = "50-ml PLG tubes";store_container_names(8, temp);container_count[8]++; break;
	case OAKRIDGE:sprintf(temp2, " (%d)", container_count[9]+1); temp1 = strcat(temp1, "Oakridge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Oakridge tubes";store_container_names(9, temp); container_count[9]++;break;
	case QIA_CARTRIDGE:result.name= "a QIAfilter cartridge"; temp = "QIAfilter cartridge";store_container_names(10, temp);container_count[10]++;break;
	case CUVETTE_ICE:result.name= "a cuvette stored on ice"; temp = "Cuvettes stored on ice";store_container_names(11, temp);container_count[11]++;break;
	case SPEC_CUVETTE:result.name= "a spectrometry cuvette"; temp = "Spectrometry cuvettes";store_container_names(12, temp); container_count[12]++;break;
	case STOCK_PLATE_96:result.name = "a 96-well stock plate"; temp = "96-well stock plates";store_container_names(13, temp);container_count[13]++;break;
	case WELL_BLOCK_96:result.name = "a 96-well block"; temp = "96-well blocks";store_container_names(14, temp);container_count[14]++;break;
	case PCR_PLATE:result.name = "a 96-well PCR plate"; temp = "96-well PCR plates";store_container_names(15, temp); container_count[15]++;break;
	case LIQUID_BLOCK:result.name = "a 96-well liquid block"; temp = "96-well liquid blocks";store_container_names(16, temp);container_count[16]++; break;
	case CELL_CULT_CHAMBER:result.name = "a cell culture chamber"; temp = "cell culture chamber";store_container_names(17, temp);container_count[17]++; break;
	case EPPENDORF:sprintf(temp2, " (%d)", container_count[18]+1); temp1 = strcat(temp1, "Eppendorf tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Eppendorf tubes";store_container_names(18, temp); container_count[18]++; break;
	case STERILE_MICROFUGE_TUBE2ML:sprintf(temp2, " (%d)", container_count[19]+1); temp1 = strcat(temp1, "sterile 2-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Sterile 2-ml microcentrifuge tubes"; store_container_names(19, temp); container_count[19]++;break;
	case STERILE_PCR_TUBE:sprintf(temp2, " (%d)", container_count[20]+1); temp1 = strcat(temp1, "sterile 0.6-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Sterile 0.6-ml tubes"; store_container_names(20, temp);container_count[20]++;break;
	case CENTRI_TUBE_50ML:sprintf(temp2, " (%d)", container_count[21]+1); temp1 = strcat(temp1, "50-ml centrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "50-ml centrifuge tubes"; store_container_names(21, temp); container_count[21]++;break;
	case CRYO_VIAL:result.name = "a screw-topped cryo vial"; temp = "screw-topped cryo vials"; store_container_names(22, temp);container_count[22]++; break;
	case SCREW_CAP_TUBE:sprintf(temp2, " (%d)", container_count[23]+1); temp1 = strcat(temp1, "screw-cap tube");temp1 = strcat(temp1, temp2);result.name = temp1;temp = "Screw-cap tubes of appropriate volume"; store_container_names(23, temp); container_count[23]++;break;
	default:break;
	}
	result.used = 0;
	usage_list_containers[list_container_no] = result;
	result.usage_index = list_container_no;
	list_container_no++;

	return result;
}

Container BioCoder :: new_container(enum container_type cont_id, Fluid& fluid1)
{
	Container result;
	result.type = CONTAINER;
	result.contents.new_name = "";
	char* temp1 = (char *)calloc(50, sizeof(char));
	char* temp2 = (char *)calloc(50, sizeof(char));
	result.id = cont_id;
	switch(cont_id){
	case STERILE_MICROFUGE_TUBE:sprintf(temp2, " (%d)", container_count[0]+1);temp1 = strcat(temp1, "sterile 1.5-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[0]++;break;
	case CENTRIFUGE_TUBE_15ML:sprintf(temp2, " (%d)", container_count[1]+1);temp1 = strcat(temp1, "sterile 15-ml centrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[1]++;break;
	case FLASK:sprintf(temp2, " (%d)", container_count[2]+1);temp1 = strcat(temp1, "flask");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[2]++;break;
	case CENTRIFUGE_BOTTLE:sprintf(temp2, " (%d)", container_count[3]+1);temp1 = strcat(temp1, "centrifuge bottle");temp1 = strcat(temp1, temp2);result.name = temp1;break;
	case GRADUATED_CYLINDER:result.name = "a graduated cylinder";break;
	case RXN_TUBE:sprintf(temp2, " (%d)", container_count[5]+1);temp1 = strcat(temp1, "reaction tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[5]++;break;
	case FRESH_COLL_TUBE:sprintf(temp2, " (%d)", container_count[6]+1);temp1 = strcat(temp1, "fresh collection tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[6]++;break;
	case LIQUID_NITROGEN:result.name = "a container with liquid nitrogen";break;
	case PLG:result.name = "a 50- ml PLG tube"; break;
	case OAKRIDGE:sprintf(temp2, " (%d)", container_count[9]+1);temp1 = strcat(temp1, "Oakridge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[9]++;break;
	case QIA_CARTRIDGE:result.name= "a QIAfilter cartridge";break;
	case CUVETTE_ICE:result.name= "a cuvette stored on ice";break;
	case SPEC_CUVETTE:result.name= "a spectrometry cuvette";break;
	case STOCK_PLATE_96:result.name = "a 96-well stock plate";break;
	case WELL_BLOCK_96:result.name = "a 96-well block";break;
	case PCR_PLATE:result.name = "a 96-well PCR plate";break;
	case LIQUID_BLOCK:result.name = "a 96-well liquid block";break;
	case CELL_CULT_CHAMBER:result.name = "a cell culture chamber";break;
	case EPPENDORF:sprintf(temp2, " (%d)", container_count[18]+1);temp1 = strcat(temp1, "Eppendorf tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[18]++;break;
	case STERILE_MICROFUGE_TUBE2ML:printf(temp2, " (%d)", container_count[19]+1);temp1 = strcat(temp1, "sterile 2-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[19]++;break;
	case STERILE_PCR_TUBE:sprintf(temp2, " (%d)", container_count[20]+1);temp1 = strcat(temp1, "sterile 0.6-ml microcentrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[20]++;break;
	case CENTRI_TUBE_50ML:sprintf(temp2, " (%d)", container_count[21]+1);temp1 = strcat(temp1, "50-ml centrifuge tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[21]++;break;
	case CRYO_VIAL:result.name = "a screw-topped cryo vial";break;
	case SCREW_CAP_TUBE:sprintf(temp2, " (%d)", container_count[23]+1);temp1 = strcat(temp1, "screw-cap tube");temp1 = strcat(temp1, temp2);result.name = temp1;container_count[23]++;break;
	default:break;
	}
	result.used = 0;
	usage_list_containers[list_container_no] = result;
	result.usage_index = list_container_no;
	list_container_no++;
	set_container(fluid1, result);
	return result;
}
void BioCoder :: set_container(Fluid& fluid1, Container& container1)
{
	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
	{
		usage_list_fluids[fluid1.usage_index].used = 1;
	}
	else
	{
		fluid1.used = 1;
		usage_list_fluids[list_fluid_no] = fluid1;
		fluid1.usage_index = list_fluid_no;
		list_fluid_no++;
	}
	fluid1.container = container1.id;
	if(container1.contents.original_name)
	{
		proSteps->addtoMixtureList(container1,container1.contents);
	}
	container1.contents = fluid1;
	container1.volume = container1.contents.volume;
}

void BioCoder :: measure_fluid(Fluid& fluid1, Container& container1)
{
	string name = proSteps->assayDispense(fluid1,Volume(fluid1.volume,fluid1.unit));
	container1.tLinkName.push_back(name);

	// graph maintenance
	{
		Fluid o = new_operation("measure fluid");
		all_nodes->create_edge_from_fluids(&fluid1, &o);
		all_nodes->create_edge_from_fluid_to_container(&o, &container1);
	}
	if (fluid1.volume == 0)
		fprintf(fp, "<font color = red>Warning: You are out of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", fluid1.new_name);
	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
		usage_list_fluids[fluid1.usage_index].used = 1;
	else
	{
		fluid1.used = 1;
		usage_list_fluids[list_fluid_no] = fluid1;
		fluid1.usage_index = list_fluid_no;
		list_fluid_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	if (first == 1)
	{
		prev_container = container1.name;
		if (container1.contents.new_name == "")
		{
			if(fluid1.state == "")
				fprintf(fp, "Measure out %s into %s.<br>", fluid1.new_name, container1.name);
			else
				fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
			first = 0;
			prev_cont++;
		}
		else
		{
			if(fluid1.state == "")
				fprintf(fp, "Measure out %s into %s.<br>", fluid1.new_name, container1.contents.new_name);
			else
				fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.contents.new_name);
			first = 0;
			prev_cont++;
		}
	}
	else if (prev_cont == 1)
	{
		prev_container = container1.name;
		if(fluid1.state == "")
			fprintf(fp, "Measure out %s into %s.<br>", fluid1.new_name, container1.name);
		else
			fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
		prev_cont++;
	}
	else if(prev_container == container1.name)
	{
		fprintf(fp, "Add <font color=#357EC7>%s</font> to %s.<br>", fluid1.new_name, container1.contents.new_name);
		prev_container = container1.name;
	}
	else if(fluid1.state == "")
	{
		fprintf(fp, "Measure out %s into %s.<br>", fluid1.new_name, container1.name);
		prev_container = container1.name;
	}
	else
	{
		fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
		prev_container = container1.name;
	}
	fluid1.container = container1.id;
	if(container1.contents.original_name)
	{
		proSteps->addtoMixtureList(container1,container1.contents);
	}
	container1.contents = fluid1;
	container1.volume = container1.volume + fluid1.volume;
}

void BioCoder :: measure_fluid(Container & container, Container & container1)
{
	proSteps->assayPreMix(container,container1);
	//Graph maintenance
	{
		Fluid o = new_operation("measure fluid");
		all_nodes-> create_edge_from_container_to_fluid(&container, &o);
		all_nodes-> create_edge_from_fluid_to_container(&o, &container1);
	}
	if (container.volume == 0)
		fprintf(fp, "<font color = red>Warning: You are out of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", container.contents.new_name);
	if (usage_list_containers[container.usage_index].name == container.name)
		usage_list_containers[container.usage_index].used = 1;
	else
	{
		container.used = 1;
		usage_list_containers[list_container_no] = container;
		container.usage_index = list_container_no;
		list_container_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	if (first == 1)
	{
		prev_container = container1.name;
		if (container1.contents.new_name == "")
		{
			if(container.contents.state == "")
				fprintf(fp, "Measure out %s into %s.<br>", container.contents.new_name, container1.name);
			else
				fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", container.contents.new_name, container.contents.new_name, container1.name);
			first = 0;
			prev_cont++;
		}
		else
		{
			if(container.contents.state == "")
				fprintf(fp, "Measure out %s into %s.<br>", container.contents.new_name, container1.contents.new_name);
			else
				fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", container.contents.new_name, container.contents.new_name, container1.contents.new_name);
			first = 0;
			prev_cont++;
		}
	}
	else if (prev_cont == 1)
	{
		prev_container = container1.name;
		if(container.contents.state == "")
			fprintf(fp, "Measure out %s into %s.<br>", container.contents.new_name, container1.name);
		else
			fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", container.contents.new_name, container.contents.new_name, container1.name);
		prev_cont++;
	}
	else if(prev_container == container1.name)
	{
		fprintf(fp, "Add <font color=#357EC7>%s</font> to %s.<br>", container.contents.new_name, container1.contents.new_name);
		prev_container = container1.name;
	}
	else if(container.contents.state == "")
	{
		fprintf(fp, "Measure out %s into %s.<br>", container.contents.new_name, container1.name);
		prev_container = container1.name;
	}
	else
	{
		fprintf(fp, "Measure out <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", container.contents.new_name, container.contents.new_name, container1.name);
		prev_container = container1.name;
	}
	container.contents.container = container1.id;
	//proSteps.addToMixtureList(container,container1);
	container1.contents = container.contents;
	container1.volume = container1.volume + container.volume;
	container.volume=0;
	container.tLinkName.clear();
}

//void BioCoder :: measure_fluid(Fluid& fluid1, Volume* volume1)
//{
//	string name = proSteps->assayDispence(fluid1,*volume1);
//	//container1.tLinkName[container1.linkNum++]=name.c_str();
//	// graph maintenance
//	{
//		Fluid o = new_operation("measure fluid");
//		all_nodes-> create_edge_from_fluids(&fluid1, &o);
//	}
//	if ((fluid1.volume == 0)|| (fluid1.volume < (volume1->value*volume1->mul)))
//		fprintf(fp, "<font color = red>Warning: You are out of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", fluid1.new_name);
//	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
//		usage_list_fluids[fluid1.usage_index].used = 1;
//	else
//	{
//		fluid1.used = 1;
//		usage_list_fluids[list_fluid_no] = fluid1;
//		fluid1.usage_index = list_fluid_no;
//		list_fluid_no++;
//	}
//	fprintf(fp, "Measure out ");
//	volume1->display_vol(fp);
//	if(fluid1.state == "")
//		fprintf(fp, " of %s.<br>", fluid1.new_name);
//	else
//		fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", fluid1.new_name, fluid1.new_name);
//	fluid1.volume = fluid1.volume - volume1->value * volume1->mul;
//}

void BioCoder :: measure_fluid(Fluid &fluid1, Volume volume1, Container& container1)
{

	// graph maintenance
	{
		Fluid o = new_operation("measure fluid");
		all_nodes-> create_edge_from_fluids(&fluid1, &o);
		all_nodes-> create_edge_from_fluid_to_container(&o, &container1);
	}
	if ((fluid1.volume == 0)|| (fluid1.volume < (volume1.value*volume1.mul)))
		fprintf(fp, "<font color = red>Warning: You are out of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", fluid1.new_name);
	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
		usage_list_fluids[fluid1.usage_index].used = 1;
	else
	{
		fluid1.used = 1;
		usage_list_fluids[list_fluid_no] = fluid1;
		fluid1.usage_index = list_fluid_no;
		list_fluid_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	if (first == 1)
	{
		fluid1.volume =volume1.value;
		fluid1.unit=volume1.unit_choice;
		prev_container = container1.name;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if (container1.contents.new_name == "")
		{
			if(fluid1.state == "")
				fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", fluid1.new_name, container1.name);
			else
				fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
			first = 0;
			prev_cont++;
		}
		else
		{
			if(fluid1.state == "")
				fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", fluid1.new_name, container1.contents.new_name);
			else
				fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.contents.new_name);
			first = 0;
			prev_cont++;
		}
	}
	else if (prev_cont == 1)
	{
		fluid1.volume =volume1.value;
		fluid1.unit=volume1.unit_choice;
		prev_container = container1.name;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if(fluid1.state == "")
			fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", fluid1.new_name, container1.name);
		else
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
		prev_cont++;
	}
	else if (prev_container == container1.name)
	{
		fluid1.volume = volume1.value;
		fluid1.unit=volume1.unit_choice;
		if(fluid1.state == "")
		{
			fprintf(fp, "Add ");
			volume1.display_vol(fp);
			fprintf(fp, " of <font color=#357EC7>%s</font>.<br>", fluid1.new_name);
		}
		else
		{
			fprintf(fp, "Add ");
			volume1.display_vol(fp);
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", fluid1.new_name, fluid1.new_name);
		}
	}
	else
	{
		fluid1.volume = volume1.value;
		fluid1.unit=volume1.unit_choice;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if(fluid1.state == "")
		{
			fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", fluid1.new_name, container1.name);
			prev_container = container1.name;
		}
		else
		{
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", fluid1.new_name, fluid1.new_name, container1.name);
			prev_container = container1.name;
		}
	}
	fluid1.container = container1.id;
	//THIS JUST REWRITES OVER THE OLD FLUID SO THERE IS NO ACCESS TO THE PREVIOUS FLUID THAT WAS THERE
	//MUST STORE PREVIOUS FLUID IF THERE AND WAIT FOR MIX TO BE CALLED ON THAT CONTAINER.
	if(container1.contents.original_name)
	{
		if(container1.contents.type!= SOLID)
			proSteps->addtoMixtureList(container1,container1.contents);
	}

	container1.contents = fluid1;
	container1.volume = container1.volume + volume1.value * volume1.mul;
//cout<<container1.volume<<endl;
	string name = proSteps->assayDispense(fluid1,volume1);
	container1.tLinkName.push_back(name);
}

void BioCoder :: measure_fluid(Container& con, Volume volume1, Container& container1, bool ensureMeasurement)
{//Does not maintain the links for the DAG correctly

	//cout<< con.tLinkName.front();
	string linkName = proSteps->assaySplit(volume1,ensureMeasurement);//takes volume1 from con and moves it to container1.
	proSteps->addToLinkMap(con,linkName);
	proSteps->addToLinkMap(container1,linkName);


	proSteps->completeMixList->addToMixList(container1,con.contents,proSteps->completeMixNum);


	//proSteps->splitManagment(con,container1,volume1, ensureMeasurement);
	// graph maintenance
	{
		Fluid o = new_operation("measure fluid");
		all_nodes-> create_edge_from_container_to_fluid(&con, &o);
		all_nodes-> create_edge_from_fluid_to_container(&o, &container1);
	}
	if ((con.volume == 0)|| (con.volume < (volume1.value*volume1.mul)))
		fprintf(fp, "<font color = red>Warning: You are measuing out more than the available volume of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", con.contents.new_name);
	if (usage_list_containers[con.usage_index].name == con.name)
		usage_list_containers[con.usage_index].used = 1;
	else
	{
		con.used = 1;
		usage_list_containers[list_container_no] = con;
		con.usage_index = list_container_no;
		list_container_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	if (first == 1)
	{
		con.contents.volume = con.contents.volume - volume1.value * volume1.mul;
		prev_container = container1.name;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if (container1.contents.new_name == "")
		{
			if(con.contents.state == "")
				fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", con.contents.new_name, container1.name);
			else
				fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", con.contents.new_name, con.contents.new_name, container1.name);
			first = 0;
			prev_cont++;
		}
		else
		{
			if(con.contents.state == "")
				fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", con.contents.new_name, container1.contents.new_name);
			else
				fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", con.contents.new_name, con.contents.new_name, container1.contents.new_name);
			first = 0;
			prev_cont++;
		}
	}
	else if (prev_cont == 1)
	{
		con.contents.volume = con.contents.volume - volume1.value * volume1.mul;
		prev_container = container1.name;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if(con.contents.state == "")
			fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", con.contents.new_name, container1.name);
		else
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", con.contents.new_name, con.contents.new_name, container1.name);
		prev_cont++;
	}
	else if (prev_container == container1.name)
	{
		con.contents.volume = con.contents.volume + volume1.value * volume1.mul;
		if(con.contents.state == "")
		{
			fprintf(fp, "Add ");
			volume1.display_vol(fp);
			fprintf(fp, " of <font color=#357EC7>%s</font>.<br>", con.contents.new_name);
		}
		else
		{
			fprintf(fp, "Add ");
			volume1.display_vol(fp);
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", con.contents.new_name, con.contents.new_name);
		}
	}
	else
	{
		con.contents.volume = con.contents.volume - volume1.value * volume1.mul;
		fprintf(fp, "Measure out ");
		volume1.display_vol(fp);
		if(con.contents.state == "")
		{
			fprintf(fp, " of <font color=#357EC7>%s</font> into %s.<br>", con.contents.new_name, container1.name);
			prev_container = container1.name;
		}
		else
		{
			fprintf(fp, " of <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", con.contents.new_name, con.contents.new_name, container1.name);
			prev_container = container1.name;
		}
	}
	con.contents.container = container1.id;
	container1.contents = con.contents;
	container1.volume = container1.volume + volume1.value * volume1.mul;
	con.volume-= volume1.value*volume1.mul;
}

void BioCoder:: measure_fluid(Container& source, int NumSplit, int piecesToDest, Container& Dest, bool ensureMeasurement)
{
	cout<<source.tLinkName.front();
	if(NumSplit < 2)
		return;
	else if(NumSplit ==2){
		string linkName= proSteps->assaySplit(2,ensureMeasurement);
		proSteps->addToLinkMap(source,linkName);
		proSteps->addToLinkMap(Dest,linkName);
	}
	else
	{
		string linkName = proSteps->assaySplit(NumSplit,ensureMeasurement);
		proSteps->addToLinkMap(source,linkName);
		proSteps->addToLinkMap(Dest,linkName);
		if(piecesToDest==1)
			for(int i=0;i<NumSplit-1;++i)
				source.tLinkName.push_back(linkName);
		else
		{
			for(int i=0; i<(NumSplit-piecesToDest)-1;++i)
				source.tLinkName.push_back(linkName);
			for(int i=0; i<piecesToDest-1;++i)
				Dest.tLinkName.push_back(linkName);
		}
	}
}

void BioCoder :: set_temp(Container& container1, float temp)
{
	string name = proSteps->assayHeat(container1, temp,Time(0,SECS));
	proSteps-> addToLinkMap(container1, name);
	// graph maintenance
	{
		Fluid o = new_operation("set temp");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes->create_node("container with contents at new temperature");
		all_nodes->create_edge(o.node, container1.node);
	}
	if (temp >= 35)
		fprintf(fp, "Pre-heat %s in a water bath set at <b><font color=#357EC7>65%cC</font></b>.<br>", container1.contents.new_name, 0x00B0);
	else
		fprintf(fp, "Set the temperature of %s to <b><font color=#357EC7>%g%cC</font></b>.<br>", container1.contents.new_name, temp, 0x00B0);
}
//void BioCoder :: measure_prop(Container& dest, Container& source, float prop)
//{
//	// graph maintenance
//	{
//		Fluid o = new_operation("measure prop");
//		all_nodes-> create_edge_from_container_to_fluid(&source, &o);
//		all_nodes-> create_edge_from_fluid_to_container(&o, &dest);
//	}
//	if ((source.contents.volume == 0)|| (source.contents.volume < (dest.volume * prop)))
//		fprintf(fp, "<font color = red>Warning: You are measuing out more than the available volume of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", source.contents.new_name);
//	if (usage_list_containers[dest.usage_index].name == dest.name)
//		usage_list_containers[dest.usage_index].used = 1;
//	else
//	{
//		dest.used = 1;
//		usage_list_containers[list_container_no] = dest;
//		dest.usage_index = list_container_no;
//		list_container_no++;
//	}
//	if (usage_list_containers[source.usage_index].name == source.name)
//		usage_list_containers[source.usage_index].used = 1;
//	else
//	{
//		source.used = 1;
//		usage_list_containers[list_container_no] = source;
//		source.usage_index = list_container_no;
//		list_container_no++;
//	}
//	if (first == 1)
//	{
//		prev_container = dest.name;
//		if(source.contents.state == "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font> into %s.<br>", prop, source.contents.new_name, dest.contents.new_name);
//			else
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font> into %s.<br>", prop, source.contents.new_name, dest.contents.new_name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", prop, source.contents.new_name, source.contents.new_name, dest.contents.new_name);
//			else
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", prop, source.contents.new_name, source.contents.new_name, dest.contents.new_name);
//		}
//	}
//	else if (prev_container == dest.name)
//	{
//		dest.contents.volume = dest.contents.volume +  prop * dest.contents.volume;
//		if(source.contents.state != "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", prop, source.contents.new_name, source.contents.new_name);
//			else
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", prop, source.contents.new_name, source.contents.new_name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font>.<br>", prop, source.contents.new_name);
//			else
//				fprintf( fp,"Add  <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font>.<br>", prop, source.contents.new_name);
//		}
//	}
//	else
//	{
//		dest.contents.volume = dest.contents.volume -  prop * dest.contents.volume;
//		if(source.contents.state != "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a> to %s.<br>", prop, source.contents.new_name, source.contents.new_name, dest.name, dest.name);
//			else
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a> to %s.<br>", prop, source.contents.new_name, source.contents.new_name, dest.name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font> to %s.<br>", prop, source.contents.new_name, dest.name);
//			else
//				fprintf( fp,"Add  <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font> to %s.<br>", prop, source.contents.new_name, dest.name);
//		}
//	}
//	prev_container = dest.name;
//	dest.volume = dest.volume +  prop * dest.volume;
//}
//
//void BioCoder :: measure_prop(Container& dest, Fluid& source, float prop)
//{
//
//	// graph maintenance
//	{
//		Fluid o = new_operation("measure prop");
//		all_nodes-> create_edge_from_fluids(&source, &o);
//		all_nodes-> create_edge_from_fluid_to_container(&o, &dest);
//	}
//	if ((source.volume == 0)|| (source.volume < (dest.volume*prop)))
//		fprintf(fp, "<font color = red>Warning: You are measuing out more than the available volume of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", source.new_name);
//	if (usage_list_fluids[source.usage_index].original_name == source.original_name)
//		usage_list_fluids[source.usage_index].used = 1;
//	else
//	{
//		source.used = 1;
//		usage_list_fluids[list_fluid_no] = source;
//		source.usage_index = list_fluid_no;
//		list_fluid_no++;
//	}
//	if (usage_list_containers[dest.usage_index].name == dest.name)
//		usage_list_containers[dest.usage_index].used = 1;
//	else
//	{
//		dest.used = 1;
//		usage_list_containers[list_container_no] = dest;
//		dest.usage_index = list_container_no;
//		list_container_no++;
//	}
//	if (first == 1)
//	{
//		prev_container = dest.name;
//		if(source.state == "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font> into %s.<br>", prop, source.new_name, dest.contents.new_name);
//			else
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font> into %s.<br>", prop, source.new_name, dest.contents.new_name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", prop, source.new_name, source.new_name, dest.contents.new_name);
//			else
//				fprintf(fp, "Measure out <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a> into %s.<br>", prop, source.new_name, source.new_name, dest.contents.new_name);
//		}
//	}
//	else if (prev_container == dest.name)
//	{
//		dest.contents.volume = dest.contents.volume +  prop * dest.contents.volume;
//		if(source.state != "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", prop, source.new_name, source.new_name);
//			else
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a>.<br>", prop, source.new_name, source.new_name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font>.<br>", prop, source.new_name);
//			else
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font>.<br>", prop, source.new_name);
//		}
//	}
//	else
//	{
//		dest.contents.volume = dest.contents.volume -  prop * dest.contents.volume;
//		if(source.state != "")
//		{
//			if (prop > 1)
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volumes <a href=\"#%s\" ><font color=#357EC7>%s</font></a> to %s.<br>", prop, source.new_name, source.new_name, dest.name);
//			else
//				fprintf(fp, "Add <b><font color=#357EC7>%g</font></b> volume <a href=\"#%s\" ><font color=#357EC7>%s</font></a> to %s.<br>", prop, source.new_name, source.new_name, dest.name);
//		}
//		else
//		{
//			if (prop > 1)
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volumes <font color=#357EC7>%s</font> to %s.<br>", prop, source.new_name, dest.name);
//			else
//				fprintf(fp, "Add  <b><font color=#357EC7>%g</font></b> volume <font color=#357EC7>%s</font> to %s.<br>", prop, source.new_name, dest.name);
//		}
//	}
//
//	prev_container = dest.name;
//	dest.volume = dest.volume +  prop * dest.volume;
//}

void BioCoder :: combine (int count, Container * container1)
{
	combine_helper(count, container1);
}
void BioCoder :: transfer (Container &container1, Container& container2)
{
	proSteps->addToMixtureList(container1, container2);
	if (container2.tLinkName.front().substr(0,1)=="O"||container2.tLinkName.front().substr(0,1)=="W")
	{
		container2.tLinkName.clear();
		container2.tLinkName= container1.tLinkName;
	}
	else
	{
		for(list<string> ::iterator it=container1.tLinkName.begin(); it != container1.tLinkName.end();++it)
			container2.tLinkName.push_back(*it);
	}
	// graph maintenance
	{
		Fluid o = new_operation("transfer");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		all_nodes-> create_edge_from_fluid_to_container(&o, &container2);
	}
	if (container1.volume == 0)
		fprintf(fp, "<font color = red>Warning: You are out of %s! Please make sure you have enough before carrying on with the protocol.<br></font>", container1.contents.new_name);
	if (usage_list_containers[container2.usage_index].name == container2.name)
		usage_list_containers[container2.usage_index].used = 1;
	else
	{
		container2.used = 1;
		usage_list_containers[list_container_no] = container2;
		container2.usage_index = list_container_no;
		list_container_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	fprintf(fp, "Transfer %s into %s.<br>" , container1.contents.new_name, container2.name);
	container2.contents = container1.contents;
	container2.contents.container = container2.id;
	container2.volume = container1.volume;
	container1.volume = 0;
}
void BioCoder :: discard(Container& container1, string outputSink)
{
	// graph maintenance
	{
		Fluid o = new_operation("discard");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("contents of container discarded");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp, "Discard %s.<br>", container1.contents.new_name);
	container1.volume = 0;

	proSteps-> assayDrain(container1, outputSink);
}


void BioCoder :: tap (Container& container1, enum until event1)
{
	mix(container1, event1);
}

void BioCoder :: tap(Container &container1, Time time1)
{
	mix(container1, TAPPING, time1);
}

void BioCoder :: tap(Container &container1)
{
	mix(container1, TAPPING);
}

void BioCoder :: stir(Container& container1)
{
	mix(container1, STIRRING);
}

void BioCoder :: stir(Container& container1, Time time1)
{
	mix(container1, STIRRING, time1);
}

void BioCoder :: invert(Container &container1, int times)
{
	mix(container1, INVERTING, times);
}

void BioCoder :: invert(Container &container1, int min_times, int max_times)
{
	mix(container1, INVERTING, min_times, max_times);
}

void BioCoder :: invert(Container &container1)
{
	mix(container1, INVERTING);
}

void BioCoder :: invert(Container& container1, enum until event1)
{
	mix(container1, event1);
}

void BioCoder :: vortex(Container &container1, Time time1)
{
	mix(container1, VORTEXING, time1);
}

void BioCoder :: vortex(Container &container1)
{
	mix(container1, VORTEXING);
}

void BioCoder :: resuspend(Container &container1)
{
	mix(container1, RESUSPENDING);
}

void BioCoder :: dissolve (Container &container1)
{
	mix(container1, DISSOLVING);
}

void BioCoder :: pipet (Container &container1)
{
	mix(container1, PIPETTING);
}
void BioCoder :: wait(Container& container1, Time time1)
{
	string name;
	name = proSteps->assayStore(container1,RT,time1);
	proSteps->addToLinkMap(container1, name);

	fprintf(fp, "Keep %s aside for ", container1.contents.new_name);
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ".<br>");
	// graph maintenance
	{
		Fluid o = new_operation("wait");
		all_nodes->create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container with contents after waiting");
		all_nodes->create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
}
void BioCoder :: combine_and_mix (enum mixing type, Time time1, int count, Container *container1 )
{
	//va_list ap;
	//va_start(ap, container1);
	combine_helper(count, container1);
	//va_end(ap);
	mix(container1[0], type, time1);
}

void BioCoder :: combine_and_mix (enum mixing type, int count, Container *container1 )
{
	//va_list ap;
	//va_start(ap, container1);
	combine_helper(count, container1);
	//va_end(ap);
	mix(container1[0], type);
}




void BioCoder :: store(Container& container1, float temp)
{
	// graph maintenance
	string name;
	name = proSteps->assayStore(container1,temp,Time(FOREVER,SECS));
	proSteps->addToLinkMap(container1, name);
	{
		Fluid o = new_operation("store");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container stored at requiassayStoreemperature");
		all_nodes-> create_edge(o.node, container1.node);
	}
	if(temp == ON_ICE)
	{
		fprintf(fp, "Store ");
		check_container(container1);
		fprintf(fp, " <b><font color=#357EC7>on ice</font></b>.<br>");
	}
	else if(temp == BOILING_WATER)
		fprintf(fp, "Immerse the tube in boiling water.");
	else
	{
		fprintf(fp, "Store ");
		check_container(container1);
		fprintf(fp," at <b><font color=#357EC7>%g%cC</font></b>.<br>", temp, 0x00B0);
	}
}


void BioCoder :: store_for(Container& container1, float temp, Time time1)
{

	if(temp == ON_ICE)
	{
		fprintf(fp, "Store ");
		check_container(container1);
		fprintf(fp, " <b><font color=#357EC7>on ice</font></b> for ");
	}
	else if(temp == BOILING_WATER)
	{
		fprintf(fp, "Immerse ");
		check_container(container1);
		fprintf(fp, " in boiling water for ");
	}
	else if (temp == RT)
	{
		fprintf(fp, "Store ");
		check_container(container1);
		fprintf(fp, " at <b><font color=#357EC7><b><font color=#357EC7>room temperature</font></b></font></b> for ");
	}
	else
	{
		fprintf(fp, "Store ");
		check_container(container1);
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
	}
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ".<br>");
	// graph maintenance
	{
		Fluid o = new_operation("store for");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container stored at required temperature");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
	string name = proSteps->assayHeat(container1,temp,time1);
	proSteps->addToLinkMap(container1, name);
}

void BioCoder :: store_for(Container& sample1, float temp, Time time1, enum func function)
{

	if(function == 1)
	{
		fprintf(fp, "Denature ");
		check_container(sample1);
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
	}
	else if (function == 2)
	{
		fprintf(fp, "Perform enzyme inactivation by storing ");
		check_container(sample1);
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
	}
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ".<br>");
	// graph maintenance
	{
		Fluid o = new_operation("store for");
		all_nodes-> create_edge_from_container_to_fluid(&sample1, &o);
		sample1.node = all_nodes-> create_node("container stored at required temperature");
		all_nodes-> create_edge(o.node, sample1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
	string name = proSteps->assayStore(sample1,temp,time1);
	proSteps->addToLinkMap(sample1, name);
}


void BioCoder :: store_until(Container& container1, float temp, enum until type)
{
	//proSteps.store(container1,temp, FOREVER);
	// graph maintenance
	{
		Fluid o = new_operation("store until");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container stored at required temperature");
		all_nodes-> create_edge(o.node, container1.node);
	}
	switch(type)
	{
	case ETHANOL_EVAP:if (temp == RT)
		fprintf(fp, "Store %s at <b><font color=#357EC7>room temperature</font></b> until the ethanol has evaporated and no fluid is visible in the tube.<br>", container1.contents.new_name);
	else
		fprintf(fp, "Store %s at <b><font color=#357EC7>%g%cC</font></b> until the ethanol has evaporated and no fluid is visible in the tube.<br>", container1.contents.new_name, temp, 0x00B0);break;
	case OD:fprintf(fp, "Incubate %s at <b><font color=#357EC7>%g%cC</font></b> until the O.D.600 reaches 0.6.<br>", container1.contents.new_name, temp, 0x00B0);break;
	case THAW:fprintf(fp, "Allow %s to thaw at <b><font color=#357EC7>room temperature</font></b>.<br>", container1.contents.new_name);break;
	case COOLED:fprintf(fp, "Keep %s at <b><font color=#357EC7>room temperature</font></b> until cooled.<br>", container1.contents.new_name);break;
	case COLOUR_DEVELOPS:fprintf(fp, "Wait for the colour to develop.<br>");break;
	case THAW_ICE:fprintf(fp, "Allow %s to thaw on <b><font color=#357EC7>ice</font></b>.<br>", container1.contents.new_name);break;
	default:break;
	}
}

void BioCoder :: store_until(Container& container1, float temp, enum until type, Time time1)
{
	string name = proSteps->assayStore(container1,temp,time1);
	proSteps->addToLinkMap(container1, name);
	switch(type)
	{
	case ETHANOL_EVAP:if (temp == RT)
		fprintf(fp, "Store %s at <b><font color=#357EC7>room temperature</font></b> until the ethanol has evaporated and no fluid is visible in the tube (~", container1.contents.new_name);
	else
		fprintf(fp, "Store %s at <b><font color=#357EC7>%g%cC</font></b> until the ethanol has evaporated and no fluid is visible in the tube (~", container1.contents.new_name);break;
	case OD:fprintf(fp, "Incubate %s at <b><font color=#357EC7>%g%cC</font></b> until the O.D.600 reaches 0.6 (~", container1.contents.new_name);break;
	case THAW:fprintf(fp, "Allow %s to thaw at <b><font color=#357EC7>room temperature</font></b> (~", container1.contents.new_name);break;
	case COOLED:fprintf(fp, "Keep %s at <b><font color=#357EC7>room temperature</font></b> until cooled (~", container1.contents.new_name);break;
	case COLOUR_DEVELOPS:fprintf(fp, "Wait for the colour to develop (~", container1.contents.new_name);break;
	default:break;
	}
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ").<br>");
	// graph maintenance
	{
		Fluid o = new_operation("store until");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container stored at required temperature");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
}

void BioCoder :: incubate(Container& container1, float temp, Time time1, int rpm)
{
	string name = proSteps->assayHeat(container1,temp,time1);
	proSteps-> addToLinkMap(container1, name);
	if(temp == RT)
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp," at <b><font color=#357EC7><b><font color=#357EC7>room temperature</font></b></font></b> for ");
	}
	else if((temp == ON_ICE)||(temp == 0))
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp, " on <b><font color=#357EC7><b><font color=#357EC7>ice</font></b></font></b> for ");
	}
	else
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
	}
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, " with shaking at %d rpm.<br>", rpm);
	if(incubator_no == 1)
	{
		equipments[equip_no] = "Incubator";
		equip_no++;
		incubator_no++;
	}
	// graph maintenance
	{
		Fluid o = new_operation("incubate");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes->create_node("container after incubation");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
}

void BioCoder :: incubate(Container& container1, float temp, Time time1)
{

	string name = proSteps->assayHeat(container1, temp, time1);

	proSteps->addToLinkMap(container1, name);
	if(temp == 28)
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp, " at <b><font color=#357EC7><b><font color=#357EC7>room temperature</font></b></font></b> for ");
	}
	else if((temp == ON_ICE)||(temp == 0))
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp, " on <b><font color=#357EC7><b><font color=#357EC7>ice</font></b></font></b> for ");
	}
	else
	{
		fprintf(fp, "Incubate ");
		check_container(container1);
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
	}
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ".<br>");

	if(incubator_no == 1)
	{
		equipments[equip_no] = "Incubator";
		equip_no++;
		incubator_no++;
	}
	// graph maintenance
	{
		Fluid o = new_operation("incubate");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container after incubation");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
}

void BioCoder :: incubate_and_mix(Container& container1, float temp, Time time1, Time time_mix, enum mixing type)
{

	string type1;
	aMixture mixStep=proSteps->findMix(container1);
	fprintf(fp, "Incubate ");
	check_container(container1);
	switch (type){
	case STIRRING:	{
		type1 = "Stirring";
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ", mixing gently by stirring the tube every ");
		time_mix.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ". <br>");
		break;
	}
	case INVERTING:	{
		type1 = "Inverting";
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ", mixing gently by inverting the tube every ");
		time_mix.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ". <br>");
		break;
	}
	case VORTEXING:	{
		type1 = "Vortexing";
		fprintf(fp, " at <b><font color=#357EC7>%g%cC</font></b> for ", temp, 0x00B0);
		time1.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ", mixing gently by vortexing the tube every ");
		time_mix.display_time(fp,option_no,options_flag,total_time_required);
		fprintf(fp, ". <br>");
		break;
	}
	default:
		type1 ="";
		break;
	}
	proSteps->assayMixManagement(container1 ,mixStep,type1);
	if(incubator_no == 1)
	{
		equipments[equip_no] = "Incubator";
		equip_no++;
		incubator_no++;
	}
	// graph maintenance
	{
		Fluid o = new_operation("incubate and mix");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("container after incubation with mixing at periodic intervals");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
	type1= proSteps->assayHeat(container1,temp,time1);
	proSteps->addToLinkMap(container1, type1);
}
void BioCoder :: name_sample(Container& container1, char* new_name)
{
	proSteps->completeMixList->changeMixName(container1,new_name,proSteps->completeMixNum);
	container1.contents.new_name = new_name;
	container1.contents.state = "";
	container1.contents.used = 1;
	usage_list_fluids[list_fluid_no] = container1.contents;
	container1.contents.usage_index = list_fluid_no;
	list_fluid_no++;
}
void BioCoder :: name_container(Container& container1, char* name)
{
	fprintf(fp, "Set aside a fresh %s. Call it %s. <br>", container1.name, name);
	container1.name = name;
	usage_list_containers[container1.usage_index].name = name;
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
}
string BioCoder :: ce_detect (Container& container1, float length, float volt_per_cm, Fluid& fluid1)
{
	string linkName = proSteps->assayDetect("ce_detect",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("ce detect");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		all_nodes-> create_edge_from_fluids(&fluid1, &o);
		container1.node = all_nodes->create_node("results of capillary electrophoresis");
		all_nodes->create_edge(o.node, container1.node);
	}
	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
		usage_list_fluids[fluid1.usage_index].used = 1;
	else
	{
		fluid1.used = 1;
		usage_list_fluids[list_fluid_no] = fluid1;
		fluid1.usage_index = list_fluid_no;
		list_fluid_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	fprintf(fp, "Detect/separate %s by capillary electrophoresis with the following settings - <b><font color=#357EC7>%g</font></b> cm at <b><font color=#357EC7>%g</font></b> V/cm using %s.<br>", container1.contents.new_name, length, volt_per_cm, fluid1.new_name);
	name_sample(container1, "separated flow");
return linkName;
}

string BioCoder :: ce_detect (Container& container1, float length, float volt_per_cm, Fluid& fluid1, Time time1)
{
	string linkName = proSteps->assayDetect("ce_detect",time1,container1);
	proSteps->addToLinkMap(container1, linkName);
	if (usage_list_fluids[fluid1.usage_index].original_name == fluid1.original_name)
		usage_list_fluids[fluid1.usage_index].used = 1;
	else
	{
		fluid1.used = 1;
		usage_list_fluids[list_fluid_no] = fluid1;
		fluid1.usage_index = list_fluid_no;
		list_fluid_no++;
	}
	if (usage_list_containers[container1.usage_index].name == container1.name)
		usage_list_containers[container1.usage_index].used = 1;
	else
	{
		container1.used = 1;
		usage_list_containers[list_container_no] = container1;
		container1.usage_index = list_container_no;
		list_container_no++;
	}
	fprintf(fp, "Detect/separate %s by capillary electrophoresis with the following settings - <b><font color=#357EC7>%g</font></b> cm at <b><font color=#357EC7>%g</font></b> V/cm using %s for ", container1.contents.new_name, length, volt_per_cm, fluid1.new_name);
	time1.display_time(fp,option_no,options_flag,total_time_required);
	fprintf(fp, ".<br>");
	name_sample(container1, "separated flow");
	// graph maintenance
	{
		Fluid o = new_operation("ce detect");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		all_nodes-> create_edge_from_fluids(&fluid1, &o);
		container1.node = all_nodes-> create_node("results of capillary electrophoresis");
		all_nodes-> create_edge(o.node, container1.node);
		o.node->time = time1.value;
		o.node->unit = time1.unit;
	}
	return linkName;
}

string BioCoder :: measure_fluorescence (Container& container1, Time time1)
{
	string linkName = proSteps->assayDetect("measure fluorescence",time1,container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("measure fluorescence");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("results of fluorescence measurement");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp, "Measure the fluorescence of %s.<br>", container1.contents.new_name);
return linkName;
}


void BioCoder :: drain(Container& container1, string outputSink)
{

	// graph maintenance
	{
		Fluid o = new_operation("drain column");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("drained column");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp,"Drain %s.<br>", container1.name);
	container1.volume = 0;

	proSteps->assayDrain(container1, outputSink);
}
string BioCoder :: electrophoresis(int no_samples, Container container_array[], float agar_conc, Volume pdt_vol, Volume loading_buff_vol)
{
	string linkName;
	for(int i=0; i<no_samples;++i){
		linkName = proSteps->assayDetect("electrophoresis",Time(0,SECS),container_array[i]);
		proSteps->addToLinkMap(container_array[i], linkName);

	}
	int i;
	// graph maintenance
	{
		Fluid o = new_operation("electrophoresis");
		for(i=0; i<no_samples; i++)
			all_nodes->create_edge_from_container_to_fluid(&container_array[i], &o);
		container_array[1].node = all_nodes-> create_node("results of electrophoresis");
		all_nodes-> create_edge(o.node, container_array[1].node);
	}
	fprintf(fp,"Perform %g&#37 agarose gel electrophoresis of ",agar_conc);
	if(no_samples == 2)
		fprintf(fp, "%s and %s (", container_array[0].contents.new_name, container_array[1].contents.new_name);
	else
	{
		for(i=0; i<no_samples;i++)
		{
			if(i=no_samples-1)
				fprintf(fp, "%s (", container_array[i].contents.new_name);
			else if(i=no_samples-2)
				fprintf(fp, "%s and ", container_array[i].contents.new_name);
			else
				fprintf(fp, "%s, ", container_array[i].contents.new_name);
			container_array[i].contents.volume = container_array[i].contents.volume - pdt_vol.value * pdt_vol.mul;
		}
	}
	pdt_vol.display_vol(fp);
	fprintf(fp, " sample and ");
	loading_buff_vol.display_vol(fp);
	fprintf(fp, " loading buffer), mixed with ethidium bromide and visualize with UV transilluminator to confirm the presence of required product.<br>");
	if(electrophoresis_no==1)
	{
		equipments[equip_no]="Electrophoretic unit";
		equip_no++;
		electrophoresis_no++;
	}
	return linkName;
}

string BioCoder :: electrophoresis(Container& container1,float agar_conc)
{
	string linkName = proSteps->assayDetect("electrophoresis",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("electrophoresis");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("results of electrophoresis");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp,"Perform %g&#37 agarose gel electrophoresis of appropriate quantity of  %s mixed with ethidium bromide and visualize with UV transilluminator to confirm the presence of required product.<br>",agar_conc, container1.contents.new_name);
	if(electrophoresis_no==1)
	{
		equipments[equip_no]="Electrophoretic unit";
		equip_no++;
		electrophoresis_no++;
	}
	return linkName;
}

string BioCoder :: electrophoresis(Container& container1)
{
	string linkName = proSteps->assayDetect("electrophoresis",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("electrophoresis");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("results of electrophoresis");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp,"Perform agarose gel electrophoresis of appropriate quantity of  %s mixed with ethidium bromide and visualize with UV transilluminator to confirm the presence of required product.<br>", container1.contents.new_name);
	if(electrophoresis_no==1)
	{
		equipments[equip_no]="Electrophoretic unit";
		equip_no++;
		electrophoresis_no++;
	}
	return linkName;
}


string BioCoder :: sequencing(Container& container1)
{
	string linkName = proSteps->assayDetect("sequencing",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("sequencing");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("results of sequencing");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp,"Dilute %s to <font color=#357EC7>100ng/ µl</font> and send <font color=#357EC7>1 µg (10 µL)</font> for sequencing.<br>", container1.contents.new_name);
	return linkName;
}

string BioCoder :: weigh(Container& container1)
{
	string linkName = proSteps->assayDetect("weigh",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("weigh");
		all_nodes->create_edge_from_container_to_fluid(&container1, &o);
		container1.node =all_nodes->create_node("results of weighing");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp, "Weigh the amount of %s present.<br>", container1.contents.new_name);
	return linkName;
}
string BioCoder :: facs(Container& container1)
{
	string linkName = proSteps->assayDetect("fluorescence activated cell sorting",Time(0,SECS),container1);

	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("facs");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes-> create_node("results of facs");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp, "FACS: sort %s based on fluorescence.", container1.contents.new_name);
	return linkName;
}
string BioCoder :: cell_culture(Container& cells,Fluid& medium, int centri_speed, float temp, float time, float percent_CO2, Fluid& for_wash_valves, Fluid& for_wash_chambers, Fluid& for_trypsinization, float for_feeding)
{
	string linkName = proSteps->assayDetect("cell culture",Time(time,SECS),cells);
	proSteps->addToLinkMap(cells, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("cell culture");
		all_nodes-> create_edge_from_container_to_fluid(&cells, &o);
		cells.node = all_nodes-> create_node("cultured cell");
		all_nodes-> create_edge(o.node, cells.node);
	}
	fprintf(fp,"Perform cell culture with the specified parameters.");
	return linkName;
}


string BioCoder :: transfection(Container& container1, Fluid& medium, Fluid& dna)
{
	string linkName = proSteps->assayDetect("transfection",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("transfection");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		all_nodes-> create_edge_from_fluids(&medium, &o);
		all_nodes-> create_edge_from_fluids(&dna, &o);
		container1.node =all_nodes-> create_node("transfected cell");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp,"Transfect %s with %s.", container1.contents.new_name, dna.new_name);
	return linkName;
}

string BioCoder :: electroporate (Container& container1, float voltage, int no_pulses)
{
	string linkName = proSteps->assayDetect("electroporate",Time(0,SECS),container1);
	proSteps->addToLinkMap(container1, linkName);
	// graph maintenance
	{
		Fluid o = new_operation("electroporate");
		all_nodes-> create_edge_from_container_to_fluid(&container1, &o);
		container1.node = all_nodes->create_node("container with contents electroporated");
		all_nodes-> create_edge(o.node, container1.node);
	}
	fprintf(fp, "Set the electroporator to deliver <b><font color=#357EC7>%g V</font></b>, and then press the PULSE button <b><font color=#357EC7>%d</font></b> times. <br>", voltage, no_pulses);
	if(electro_no==1)
	{
		equipments[equip_no]="Electroporator";
		equip_no++;
		electro_no++;
	}
	return linkName;
}
Symbol BioCoder :: new_symbol(char* symbol, char* def)
{
	Symbol s1;
	s1.value = DEFAULT;
	s1.symbol = symbol;
	s1.unit = UL;
	if(print_parameters == 1)
	{
		fprintf(fp, "</ul><h2>Parameters:</h2><ul type=\"circle\">");
		print_parameters++;
	}
	fprintf(fp, "<li>%s - %s</li>", symbol, def);
	return s1;
}


Symbol_t BioCoder ::  new_symbol_t(char* symbol, char* def)
{
	Symbol_t s1;
	s1.value = DEFAULT;
	s1.symbol = symbol;
	s1.unit = SECS;
	if(print_parameters == 1)
	{
		fprintf(fp, "</ul><h2>Parameters:</h2><ul type=\"circle\">");
		print_parameters++;
	}
	fprintf(fp, "<li>%s - %s</li>", symbol, def);
	return s1;
}
void BioCoder :: set_value(Symbol& symbol1, float value, enum vol_unit unit)
{
	symbol1.value = value;
	symbol1.unit = unit;
}

void BioCoder :: set_value(Symbol_t& symbol1, float value, enum time_unit unit)
{
	symbol1.value = value;
	symbol1.unit = unit;
}

void BioCoder :: assign(Symbol& s1, Symbol& s2)
{
	fprintf(fp, "Let %s = %s.\n", s1.symbol, s2.symbol);
	s1.value = s2.value;
}

void BioCoder :: assign(Symbol_t& s1, Symbol_t& s2)
{
	fprintf(fp, "Let %s = %s.\n", s1.symbol, s2.symbol);
	s1.value = s2.value;
}

Symbol BioCoder :: subtract(Symbolic_vol* s1, Volume vol1)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(s1->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(s1->s1.symbol)+ 10, sizeof(char));
	strcat(temp1, s1->s1.symbol);
	strcat(temp1, "-");
	sprintf(temp2, "%g", vol1.value);
	strcat(temp1, temp2);
	y.symbol= temp1;
	if ((vol1.value != DEFAULT)&&(s1->value != DEFAULT))
		y.value = s1->value - vol1.value;
	else
		y.value = DEFAULT;
	return y;
}

Symbol BioCoder :: subtract(Volume s1, Symbolic_vol* vol1)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	sprintf(temp2, "%g", s1.value);
	strcat(temp1, temp2);
	strcat(temp1, "-");
	strcat(temp1, vol1->s1.symbol);
	y.symbol= temp1;
	if ((s1.value != DEFAULT)&&(vol1->value != DEFAULT))
		y.value = s1.value - vol1->value;
	else
		y.value = DEFAULT;
	return y;
}

Symbol BioCoder :: subtract(Symbolic_vol* vol1, Symbolic_vol* vol2)
{
	Symbol y;
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ strlen(vol2->s1.symbol), sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "-");
	strcat(temp1, vol2->s1.symbol);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2->value != DEFAULT))
		y.value = vol1->value - vol2->value;
	else
		y.value = DEFAULT;
	return y;
}


Symbol BioCoder :: add(Symbolic_vol* vol1, Volume vol2)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "+");
	sprintf(temp2, "%g", vol2.value);
	strcat(temp1, temp2);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2.value != DEFAULT))
		y.value = vol1->value + vol2.value;
	else
		y.value = DEFAULT;
	return y;
}


Symbol BioCoder :: add(Symbolic_vol* vol1, Symbolic_vol* vol2)
{
	Symbol y;
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ strlen(vol2->s1.symbol), sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "+");
	strcat(temp1, vol2->s1.symbol);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2->value != DEFAULT))
		y.value = vol1->value + vol2->value;
	else
		y.value = DEFAULT;
	return y;
}

Symbol BioCoder :: multiply(Symbolic_vol* vol1, Volume vol2)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "*");
	sprintf(temp2, "%g", vol2.value);
	strcat(temp1, temp2);
	if ((vol1->value != DEFAULT)&&(vol2.value != DEFAULT))
		y.value = vol1->value * vol2.value;
	else
		y.value = DEFAULT;
	y.symbol= temp1;
	return y;
}

Symbol BioCoder :: multiply(Symbolic_vol* vol1, Symbolic_vol* vol2)
{
	Symbol y;
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ strlen(vol2->s1.symbol), sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "*");
	strcat(temp1, vol2->s1.symbol);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2->value != DEFAULT))
		y.value = vol1->value * vol2->value;
	else
		y.value = DEFAULT;
	return y;
}
Symbol BioCoder :: divide(Symbolic_vol* vol1, Volume vol2)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(vol1->s1.symbol)+ 10, sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "/");
	sprintf(temp2, "%g", vol2.value);
	strcat(temp1, temp2);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2.value != DEFAULT))
		y.value = vol1->value / vol2.value;
	else
		y.value = DEFAULT;
	return y;
}


Symbol BioCoder :: divide(Volume vol1, Symbolic_vol* vol2)
{
	Symbol y;
	char* temp2 = "";
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol2->s1.symbol)+ 10, sizeof(char));
	temp2 = (char *)calloc(strlen(vol2->s1.symbol)+ 10, sizeof(char));
	sprintf(temp2, "%g", vol1.value);
	strcat(temp1, temp2);
	strcat(temp1, "/");
	strcat(temp1, vol2->s1.symbol);
	y.symbol= temp1;
	if ((vol1.value != DEFAULT)&&(vol2->value != DEFAULT))
		y.value = vol1.value / vol2->value;
	else
		y.value = DEFAULT;
	return y;
}


Symbol BioCoder :: divide(Symbolic_vol* vol1, Symbolic_vol* vol2)
{
	Symbol y;
	char* temp1="";
	temp1 = (char *)calloc(strlen(vol1->s1.symbol)+ strlen(vol2->s1.symbol), sizeof(char));
	strcat(temp1, vol1->s1.symbol);
	strcat(temp1, "/");
	strcat(temp1, vol2->s1.symbol);
	y.symbol= temp1;
	if ((vol1->value != DEFAULT)&&(vol2->value != DEFAULT))
		y.value = vol1->value / vol2->value;
	else
		y.value = DEFAULT;
	return y;
}


void BioCoder :: drainAndFillWith(Container & drainCon, string outputSink, Container & filler)
{
	if(!(drainCon.volume == 0 && (drainCon.tLinkName.front().substr(0,1)=="O" ||drainCon.tLinkName.front().substr(0,1)=="W" )))
		drain(drainCon, outputSink);
	transfer(filler,drainCon);
}
//functions used to call time, speed and volume
Time BioCoder :: time (float x, enum time_unit unit)
{
	return Time(x, unit);
}
Time_range  BioCoder :: time_range (float x, float y, enum time_unit unit)
{
	return Time_range(x, y, unit);

}

Minimum_time BioCoder :: min_time(float x, enum time_unit unit)
{
	return Minimum_time(x, unit);

}
Maximum_time  BioCoder :: max_time(float x, enum time_unit unit)
{
	return Maximum_time(x, unit);
}
Approx_time BioCoder :: approx_time(float x, enum time_unit unit)
{
	return Approx_time(x, unit);
}
Speed BioCoder :: speed (float x, enum speed_type unit)
{
	return Speed(x, unit);
}
Speed_range BioCoder :: speed_range(float x, float y, enum speed_type unit)
{
	return Speed_range(x, y, unit);
}
Minimum_speed BioCoder :: min_speed(float x, enum speed_type unit)
{
	return Minimum_speed(x, unit);
}
Maximum_speed BioCoder :: max_speed(float x, enum speed_type unit)
{
	return Maximum_speed(x, unit);

}
Approx_speed BioCoder :: approx_speed(float x, enum speed_type unit)
{
	return Approx_speed(x, unit);

}

Volume BioCoder :: vol(float x, enum vol_unit unit)
{
	return Volume(x, unit);

}

Symbolic_vol  BioCoder :: s_vol(Symbol s)
{
	return Symbolic_vol(s);
}

Symbolic_time BioCoder :: s_time(Symbol_t s)
{
	return Symbolic_time(s);

}

Volume_range  BioCoder :: vol_range(float x, float y, enum vol_unit unit)
{
	return Volume_range(x, y, unit);
}
Approx_volume BioCoder :: approx_vol(float x, enum vol_unit unit)
{
	return Approx_volume(x, unit);

}
int BioCoder ::getDropRestriction()
{
	return proSteps->dagDropRestrict;
}
void BioCoder :: setDropRestriction(int n)
{
	proSteps->dagDropRestrict=n;
}

void BioCoder :: printAssaySteps()
{
	proSteps->printassay(cout);
}
void BioCoder :: printLinks()
{
	proSteps->printLinks(cout);
}

void BioCoder :: Translator(DAG *dag)
{
	bool all=true;
	bool color=true;
	proSteps->Translator(filename, dag, color,all);
}

string BioCoder :: getFile(){
	string path ="./Assays";
	ifstream xmlfile;
	DIR * directory;
	struct dirent *entry;
	if(directory=opendir(path.c_str())){
		while( entry = readdir(directory)){
			if(strcmp(entry->d_name, "text.xml") ==0){
				string filepath= path+"/"+ entry->d_name;
				xmlfile.open(filepath.c_str());
				if(xmlfile.is_open())
					return filepath;
				else
					cout<<"FILE NOT FOUND!!!"<<endl;
				xmlfile.close();
			}
		}

	}
	closedir(directory);
	return "";
}
void BioCoder :: TransWebApp ()
{
	//TODO: Throw error if container/fluid/ oulet list is empty
	map<int, XMLOPS> OpMap;
	map<int, XMLMIX> MixMap;
	map<int, string> ConMap;// map containing the container list
	map<int, string> FluMap;// map containing the fluid list
	map<int, string> OutMap;// map containing the Output list
	ifstream xmlfile;
	string line;
	string Filepath = getFile();
	xmlfile.open(Filepath.c_str());
	//	xmlfile>> line;
	//cout<< line;
	if(xmlfile.is_open()){
		while (xmlfile.good()){
			xmlfile >> line;
			if(line=="<Operations>")
			{
				while(line != "</Operations>")
				{
				//	cout<<"entering O"<<endl;
					xmlfile >> line;
					if(line != "</Operations>"){
						int n= line.find(">");
						line=line.substr(n);
						n=line.find("<");
						int num = atoi(line.substr(1,n-1).c_str());
					//	cout<<line << "num: " <<num<< endl;
						switch (num){
						case 0:
							OpMap.insert(pair<int,XMLOPS>(num,X_DIS));
							break;
						case 1:
							OpMap.insert(pair<int, XMLOPS>(num, X_DCT));
							break;
						case 2:
							OpMap.insert(pair<int,XMLOPS>(num, X_MX));
							break;
						case 3:
							OpMap.insert(pair<int, XMLOPS>(num, X_SPLT));
							break;
						case 4:
							OpMap.insert(pair<int, XMLOPS>(num, X_HT));
							break;
						case 5:
							OpMap.insert(pair<int, XMLOPS>(num,X_OT));
							break;
						default:
							cerr<< "Unknown operation found!"<<endl;
						}
					}
				}
				//cout<<"exiting O"<<endl;
			}
			else if(line == "<Containers>")
			{
			//	cout<<"entering C"<<endl;
				while(line != "</Containers>" )
				{
					xmlfile>>line;
					if(line != "</Containers>"){
						int n= line.find(">");
						string name = line.substr(1,n-1);
						line=line.substr(n);
						n=line.find("<");
						int num = atoi(line.substr(1,n-1).c_str());
					//	cout<<line<<  "num: " <<num<< " NAME: "<< name<<endl;//santity check to make sure that substr is working correctly
						ConMap.insert(pair<int,string>(num, name));
					}
				}
				//cout<<"exiting C"<<endl;
			}
			else if (line == "<Fluids>")
			{
				//cout<<"entering F"<<endl;
				while(line !=  "</Fluids>")
				{
					xmlfile>>line;
					if(line != "</Fluids>"){
					int n= line.find(">");
					string name = line.substr(1,n-1);
					line=line.substr(n);
					n=line.find("<");
					int num = atoi(line.substr(1,n-1).c_str());
			//		cout<< "num: " <<num<< " NAME: "<< name<<endl;//santity check to make sure that substr is working correctly
					FluMap.insert(pair<int, string>(num, name));
					}
				}
		//		cout<<"exit F"<<endl;
			}
			else if(line=="<Mix_Type>")
			{
		//		cout<<"entering M"<<endl;
				while(line != "</Mix_Type>")
				{
					xmlfile >> line;
					if(line != "</Mix_Type>"){
						int n= line.find(">");
						line=line.substr(n);
						n=line.find("<");
						int num = atoi(line.substr(1,n-1).c_str());
//						cout<<line << "num: " <<num<< endl;
						switch (num){
						case 0:
							MixMap.insert(pair<int,XMLMIX>(num,X_INVERT));
							break;
						case 1:
							MixMap.insert(pair<int,XMLMIX>(num,X_MIX));
							break;
						case 2:
							MixMap.insert(pair<int,XMLMIX>(num,X_STIR));
							break;
						case 3:
							MixMap.insert(pair<int,XMLMIX>(num,X_TAP));
							break;
						case 4:
							MixMap.insert(pair<int,XMLMIX>(num,X_VORTEX));
							break;
						default:
							cerr<< "Unknown MIX found"<<endl;
						}
					}
				}
	//			cout<<"exit M"<<endl;
			}
			else if(line == "<Outlets>")
			{
			//	cout<<"entering Out"<<endl;
				while(line != "</Outlets>" )
				{
					xmlfile>>line;
					if(line != "</Outlets>"){
						int n= line.find(">");
						string name = line.substr(1,n-1);
						line=line.substr(n);
						n=line.find("<");
						int num = atoi(line.substr(1,n-1).c_str());
				//		cout<<line<< "num: " <<num<< " NAME: "<< name<<endl;//santity check to make sure that substr is working correctly
						OutMap.insert(pair<int,string>(num, line));
					}
				}
				//cout<<"exit OUt"<<endl;
			}
			else if(line == "<Test_Assay>")
			{
				while(line!= "</Test_Assay>")
				{
					xmlfile>>line;
					if(line != "</Test_Assay>")
					{
;//						CallBio(xmlfile);
					}

				}
			}
			else
				cout<<"Case Not Caught: "<<line<<endl;

		}
	}
	else
		cout<<"FILE NOT FOUND!!!"<<endl;
	xmlfile.close();
}
void BioCoder::CFTranslator(string s){
		proSteps->Translator(s);
}

