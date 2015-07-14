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
/*-------------------------------Class Details----------------------------------*
 * Type: BioCoder Class															*
 * Name: TimeVolumeSpeed 														*
 *																				*
 * Original source code from the following paper:								*
 * Authors: Vaishnavi Ananthanarayanan and William Thies						*
 * Title: Biocoder: A programming language for standardizing and automating		*
 * 			biology protocols													*
 * Publication Details: In J. Biological Engineering, Vol. 4, No. 13, Nov 2010	*
 * 																				*
 * Details: UCR has modified and added to the original biocoder so that it 		*
 * could actually be used for synthesis on a DMFB.								*
 * 																				*
 * This class contains several simple classes detailing time, volume and speed. *
 *-----------------------------------------------------------------------------*/
#ifndef TIMEVOLUMESPEED_H_
#define TIMEVOLUMESPEED_H_
#include<cstdio>
#include "BioEnums.h"
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>TIME<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class Time
{
public:
	float value;
	int unit_choice;
	char* unit;
	int mul;
	Time(){}
	Time (float x, enum time_unit unit);
	double conv2secs();
	virtual void display_time (FILE * fp,int option_no,int options_flag, int & total_time_required);
	virtual ~Time();
};

class Minimum_time : public Time
{
public:
	Minimum_time(float x, enum time_unit unit);
	void display_time(FILE* fp,int option_no,int options_flag, int & total_time_required);
	~ Minimum_time();
};

class Maximum_time : public Time
{
public:
	Maximum_time(float x, enum time_unit unit);
	void display_time(FILE* fp,int option_no,int options_flag, int & total_time_required);
	~ Maximum_time();
};
class Approx_time : public Time
{
public:
	Approx_time(float x, enum time_unit unit);
	void display_time(FILE* fp,int option_no,int options_flag, int & total_time_required);
	~ Approx_time();
};

class Time_range : public Time
{
public:
	float value1;
	Time_range(float x, float y, enum time_unit unit);
	void display_time(FILE* fp,int option_no,int options_flag, int & total_time_required);
	~ Time_range();
};

typedef struct
{
	char* symbol;
	float value;
	enum time_unit unit;
}Symbol_t;
class Symbolic_time : public Time
{
public:
	Symbol_t s1;
	Symbolic_time(Symbol_t& s);
	void display_time(FILE* fp,int option_no,int options_flag, int & total_time_required);
	~ Symbolic_time();
};

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SPEED<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class Speed
{
public:
	float value;
	int unit_choice;
	char* unit;
	Speed(){}
	Speed (float x, enum speed_type unit);
	virtual void display_speed (FILE* fp);
	virtual ~Speed();
};
class Minimum_speed : public Speed
{
public:
	Minimum_speed(float x, enum speed_type unit);
	void display_speed(FILE* fp);
	~ Minimum_speed();
};

class Maximum_speed : public Speed
{
public:
	Maximum_speed(float x, enum speed_type unit);
	void display_speed(FILE* fp);
	~ Maximum_speed();
};
class Approx_speed : public Speed
{
public:
	Approx_speed(float x, enum speed_type unit);
	void display_speed(FILE* fp);
	~ Approx_speed();
};

class Speed_range : public Speed
{
public:
	float value1;
	Speed_range(float x, float y, enum speed_type unit);
	void display_speed(FILE* fp);
	~ Speed_range();
};
typedef struct
{
	char* symbol;
	float value;
	enum vol_unit unit;
}Symbol;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>VOLUME<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
class Volume
{
public:
	float value;
	char* unit;
	enum factor mul;
	enum vol_unit unit_choice;
	Volume(){}
	double conv2UL();
	Volume(float x, enum vol_unit unit);
	virtual void display_vol(FILE* fp);
	~Volume();
};

class Symbolic_vol : public Volume
{
public:
	Symbol s1;
	Symbolic_vol(){}
	Symbolic_vol(Symbol& s);
	void display_vol(FILE* fp);

};
class Volume_range : public Volume
{
public:
	float value1;
	Volume_range(){}
	Volume_range(float x, float y, enum vol_unit unit);
	void display_vol(FILE* fp);
};
class Approx_volume : public Volume
{
public:
	Approx_volume(){}
	Approx_volume(float x, enum vol_unit unit);
	void display_vol(FILE* fp);
};


#endif /* TIMEVOLUMESPEED_H_ */
