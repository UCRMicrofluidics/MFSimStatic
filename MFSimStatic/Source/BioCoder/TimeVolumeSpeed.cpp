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
#include "../../Headers/BioCoder/TimeVolumeSpeed.h"
#include "../../Headers/BioCoder/BioCoder.h"
#include "../../Headers/BioCoder/BioEnums.h"


Time::Time(float x, enum time_unit unit)
{
	value = x;
	unit_choice = unit;
}
double Time :: conv2secs()
{
	double secs= value;
	double multi;
	switch (unit_choice){
	case SECS:
		multi =1;
		break;
	case MINS:
		multi = 60;
		break;
	case HRS:
		multi = 3600;
		break;
	}
	return secs*multi;
}

void Time::display_time(FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	float max_time_value = 0;
	switch(unit_choice)
	{
	case SECS:if (value == 1) unit = "sec"; else unit = "secs";mul = 1;break;
	case MINS:if (value == 1) unit = "min"; else unit = "mins";mul = 60;break;
	case HRS:if (value == 1) unit = "hr"; else unit = "hrs";mul = 3600;break;
	default:break;
	}
	if ((value == OVERNIGHT)&&(unit == "hrs"))
		fprintf (fp, "<b><font color=#357EC7>12 hrs</font></b>(overnight)");
	else if (value == 0)
		fprintf(fp, "<b>immediately</b>");
	else if (value == FOREVER)
		fprintf(fp, "<b><font color=#357EC7>forever</font></b>(until taken out from the thermocycler)");
	else if (value == XVAL)
		fprintf(fp, "<b><font color=#357EC7>X</font></b>");
	else
		fprintf (fp, "<b><font color=#357EC7>%g %s</font></b>", value, unit);
	if((option_no == 2) && (options_flag == 1))
		max_time_value = value * mul;
	else if(option_no > 2)
	{
		if(max_time_value > value*mul)
			total_time_required = total_time_required + max_time_value;
		else
		{
			total_time_required = total_time_required + value * mul;
			max_time_value = value*mul;
		}
	}
	else
		total_time_required = total_time_required + value * mul;
}
Time::~Time()
{

}

Minimum_time::Minimum_time (float x, enum time_unit unit)
{
	value = x;
	unit_choice = unit;
}
void Minimum_time::display_time(FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	float max_time_value = 0;
	switch(unit_choice)
	{
	case SECS:if (value == 1) unit = "sec"; else unit = "secs";mul = 1;break;
	case MINS:if (value == 1) unit = "min"; else unit = "mins";mul = 60;break;
	case HRS:if (value == 1) unit = "hr"; else unit = "hrs";mul = 3600;break;
	default:break;
	}
	fprintf(fp, "at least <b><font color=#357EC7>%g %s</font></b>", value, unit);
	if((option_no == 2) && (options_flag == 1))
		max_time_value = value * mul;
	else if(option_no > 2)
	{
		if(max_time_value > value*mul)
			total_time_required = total_time_required + max_time_value;
		else
		{
			total_time_required = total_time_required + value * mul;
			max_time_value = value*mul;
		}
	}
	else
		total_time_required = total_time_required + value * mul;
}
Minimum_time::~Minimum_time()
{

}

Maximum_time::Maximum_time(float x, enum time_unit unit)
{
	value = x;
	unit_choice = unit;
}
void Maximum_time::display_time(FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	float max_time_value = 0;
	switch(unit_choice)
	{
	case SECS:if (value == 1) unit = "sec"; else unit = "secs";mul = 1;break;
	case MINS:if (value == 1) unit = "min"; else unit = "mins";mul = 60;break;
	case HRS:if (value == 1) unit = "hr"; else unit = "hrs";mul = 3600;break;
	default:break;
	}
	fprintf(fp, "at most <b><font color=#357EC7>%g %s</font></b>", value, unit);
	if((option_no == 2) && (options_flag == 1))
		max_time_value = value * mul;
	else if(option_no > 2)
	{
		if(max_time_value > value*mul)
			total_time_required = total_time_required + max_time_value;
		else
		{
			total_time_required = total_time_required + value * mul;
			max_time_value = value*mul;
		}
	}
	else
		total_time_required = total_time_required + value * mul;
}
Maximum_time::~Maximum_time()
{

}


Approx_time::Approx_time(float x, enum time_unit unit)
{
	value = x;
	unit_choice = unit;
}

void Approx_time::display_time(FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	float max_time_value = 0;
	switch(unit_choice)
	{
	case SECS:if (value == 1) unit = "sec"; else unit = "secs";mul = 1;break;
	case MINS:if (value == 1) unit = "min"; else unit = "mins";mul = 60;break;
	case HRS:if (value == 1) unit = "hr"; else unit = "hrs";mul = 3600;break;
	default:break;
	}
	fprintf(fp, "~<b><font color=#357EC7>%g %s</font></b>", value, unit);
	if((option_no == 2) && (options_flag == 1))
		max_time_value = value * mul;
	else if(option_no > 2)
	{
		if(max_time_value > value*mul)
			total_time_required = total_time_required + max_time_value;
		else
		{
			total_time_required = total_time_required + value * mul;
			max_time_value = value*mul;
		}
	}
	else
		total_time_required = total_time_required + value * mul;
}
Approx_time ::~Approx_time()
{
}

Time_range::Time_range(float x, float y, enum time_unit unit)
{
	value1 = x;
	value = y;
	unit_choice = unit;
}

void Time_range::display_time (FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	float max_time_value = 0;
	switch(unit_choice)
	{
	case SECS:if (value1 == 1) unit = "sec"; else unit = "secs";mul = 1;break;
	case MINS:if (value1 == 1) unit = "min"; else unit = "mins";mul = 60;break;
	case HRS:if (value1 == 1) unit = "hr"; else unit = "hrs";mul = 3600;break;
	default:break;
	}
	if ((value1 == 12) && (unit == "hrs"))
		fprintf (fp, "<b><font color=#357EC7>%g(overnight) - %g %s</font></b>", value1, value, unit);
	else if ((value == 12) && (unit == "hrs"))
		fprintf (fp, "<b><font color=#357EC7>%g - %g %s(overnight)</font></b>", value1, value, unit);
	else
		fprintf (fp, "<b><font color=#357EC7>%g - %g %s</font></b>", value1, value, unit);
	if((option_no == 2) && (options_flag == 1))
		max_time_value = value * mul;
	else if(option_no > 2)
	{
		if(max_time_value > value*mul)
			total_time_required = total_time_required + max_time_value;
		else
		{
			total_time_required = total_time_required + value * mul;
			max_time_value = value*mul;
		}
	}
	else
		total_time_required = total_time_required + value * mul;
}
Time_range::~Time_range()
{

}

Symbolic_time::Symbolic_time(Symbol_t& s)
{
	s1.symbol = s.symbol;
	s1.unit = s.unit;
	s1.value = s.value;
	value = s1.value;
}

void Symbolic_time:: display_time(FILE* fp,int option_no,int options_flag, int & total_time_required)
{
	Time time1;
	if (s1.value == DEFAULT)
		fprintf(fp,"<b><font color=#357EC7>%s secs</font></b>", s1.symbol);
	else
	{
		//*time1 = Time(s1.value, s1.unit);
		time1 = Time(s1.value, s1.unit);
		time1.display_time(fp,option_no,options_flag,total_time_required);
	}
}

Symbolic_time::~Symbolic_time()
{

}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>SPEED<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//class Speed
Speed::Speed(float x, enum speed_type unit)
{
	value = x;
	unit_choice = unit;
}
void Speed::display_speed(FILE* fp)
{
	switch(unit_choice)
	{
	case RPM: unit = "rpm";break;
	case G: unit = "Xg";break;
	default:break;
	}
	if (value == SPEED_MAX)
		fprintf (fp, "<font color=#357EC7>maximum speed</font>");
	else
		fprintf (fp, "a speed of <font color=#357EC7>%g %s</font>", value, unit);
}
Speed::~Speed()
{

}

Minimum_speed::Minimum_speed (float x, enum speed_type unit)
{
	value = x;
	unit_choice = unit;
}

void Minimum_speed::display_speed(FILE* fp)
{
	switch(unit_choice)
	{
	case RPM: unit = "rpm";break;
	case G: unit = "Xg";break;
	default:break;
	}
	fprintf(fp, "a speed of at least <font color=#357EC7>%g %s</font>", value, unit);
}
Minimum_speed::~Minimum_speed()
{
}

Maximum_speed::Maximum_speed(float x, enum speed_type unit)
{
	value = x;
	unit_choice = unit;
}
void Maximum_speed::display_speed(FILE* fp)
{
	switch(unit_choice)
	{
	case RPM: unit = "rpm";break;
	case G: unit = "Xg";break;
	default:break;
	}
	fprintf(fp, "a speed of at most <font color=#357EC7>%g %s</font>", value, unit);
}
Maximum_speed::~Maximum_speed()
{
}

Approx_speed::Approx_speed(float x, enum speed_type unit)
{
	value = x;
	unit_choice = unit;
}

void Approx_speed::display_speed(FILE* fp)
{
	switch(unit_choice)
	{
	case RPM: unit = "rpm";break;
	case G: unit = "Xg";break;
	default:break;
	}
	fprintf(fp, "a speed of ~<font color=#357EC7>%g %s</font>", value, unit);
}
Approx_speed::~Approx_speed()
{

}

Speed_range::Speed_range(float x, float y, enum speed_type unit)
{
	value1 = x;
	value = y;
	unit_choice = unit;
}
void Speed_range::display_speed (FILE* fp)
{
	switch(unit_choice)
	{
	case RPM: unit = "rpm";break;
	case G: unit = "Xg";break;
	default:break;
	}
	fprintf (fp, "a speed of <font color=#357EC7>%g - %g %s</font>", value1, value, unit);
}
Speed_range::~Speed_range()
{

}
//class Volume
Volume::Volume(float x, enum vol_unit unit1)
{
	value = x;
	unit_choice = unit1;
	switch(unit_choice)
	{
	case UL: unit = "µl";mul = MICRO;break;
	case ML: unit = "ml";mul = MILLI;break;
	case L: unit = "l";mul = LITRE;break;
	default : unit = "µl";mul = MICRO;break;
	}
}
void Volume::display_vol(FILE* fp)
{
	fprintf(fp, "<b><font color=#357EC7>%g %s</font></b>", value, unit);
}
double Volume :: conv2UL()
{
	double micro= value;
	double multi;

	switch(unit_choice){
	case UL:
		multi=1;
		break;
	case ML:
		multi= 1000;
		break;
	case L:
		multi= 1000000;
		break;
	}
	return micro*multi;
}
Volume::~Volume()
{}

Symbolic_vol::Symbolic_vol(Symbol& s)
{
	s1.symbol = s.symbol;
	s1.unit = s.unit;
	s1.value = s.value;
	value = s1.value;
}

void Symbolic_vol:: display_vol(FILE* fp)
{
	Volume vol1;
	if (s1.value == DEFAULT)
		fprintf(fp,"<b><font color=#357EC7>%s µl</font></b>", s1.symbol);
	else
	{
		vol1 = Volume(s1.value, s1.unit);
		vol1.display_vol(fp);
	}
}

Volume_range::Volume_range(float x, float y, enum vol_unit unit1)
{
	value1 = x;
	value = y;
	unit_choice = unit1;
	switch(unit_choice){
	case UL: unit = "µl";mul = MICRO;break;
	case ML: unit = "ml";mul = MILLI;break;
	case L: unit = "l";mul = LITRE;break;
	default : unit = "µl";mul = MICRO;break;
	}
}
void Volume_range:: display_vol(FILE* fp)
{
	fprintf(fp, "<b><font color=#357EC7>%g - %g %s</font></b>", value1, value, unit);
}

Approx_volume::Approx_volume(float x, enum vol_unit unit1)
{
	value = x;
	unit_choice = unit1;
	switch(unit_choice){
	case UL: unit = "µl";mul = MICRO;break;
	case ML: unit = "ml";mul = MILLI;break;
	case L: unit = "l";mul = LITRE;break;
	default : unit = "µl";mul = MICRO;break;
	}
}
void Approx_volume::display_vol(FILE* fp)
{
	fprintf(fp, "<b><font color=#357EC7>~%g %s</font></b>", value, unit);
};

