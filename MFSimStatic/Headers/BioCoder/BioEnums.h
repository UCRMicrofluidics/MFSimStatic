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
/*--------------------------------File Details----------------------------------*
 * Type: BioCoder Enums															*
 * Name: BioEnums																*
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
 * Contains the necessary enumerations for BioCoder.							*
 *-----------------------------------------------------------------------------*/
#ifndef BIOENUMS_H_
#define BIOENUMS_H_

/*! Types of containers. For use in new_container(). */
enum container_type{STERILE_MICROFUGE_TUBE/*!< Sterile 1.5-ml microfuge tube */, CENTRIFUGE_TUBE_15ML/*!< 15ml centrifuge tube */, FLASK/*!< Flask */, CENTRIFUGE_BOTTLE/*!< Centrifuge bottle */, GRADUATED_CYLINDER/*!< Graduated cylinder */, HUMIDIFIED_CHAMBER/*!< container that can be placed in a humidified chamber */, RXN_TUBE/*!< Reaction tube */, FRESH_COLL_TUBE/*!< Fresh collection tube */, LIQUID_NITROGEN/*!< container with liquid nitrogen */, PLG/*!< 50ml PLG tube */, OAKRIDGE/*!< Oakridge tube */, QIA_CARTRIDGE/*!< Qiacartridge */, CUVETTE_ICE/*!< Cuvette stored on ice */, SPEC_CUVETTE/*!< spectrometry cuvette */, STOCK_PLATE_96/*!< 96-well stock plate */, WELL_BLOCK_96/*!< 96-well block */, PCR_PLATE/*!< 96-well PCR plate */, LIQUID_BLOCK/*!< 96-well liquid block */, CELL_CULT_CHAMBER/*!< Cell culture chamber */, EPPENDORF/*!< Eppendorf tube */, STERILE_MICROFUGE_TUBE2ML/*!< Sterile 2 ml microcentrifuge tube */, STERILE_PCR_TUBE/*!< Sterile 0.6-ml tube */, CENTRI_TUBE_50ML/*!< 50-ml centrifuge tube */, CRYO_VIAL/*!< screw-topped cryo vial */, SCREW_CAP_TUBE/*!< Screw-cap tube of appropriate volume */};
/*!Specifies whether drying has to be performed in air or in vacuum. */
enum drying{IN_AIR, IN_VACUUM};
/// @cond
/*! Used internally for conversion between volume units. */
enum factor{MICRO=1/*!< Conversion value = 1 */, MILLI=1000/*!< Conversion value = 1000 */, LITRE=1000000/*!< Conversion value = 1000000 */};
/*! Specifies the type of the subtance. Used internally. */
enum fluid_type {FLUID, SOLID, CONTAINER, OPERATION} ;
/// @endcond
/*!Specifies the kind of operation performed by a given instruction. */
enum func{DEWAX/*!< dewaxing */, DENATURE/*!< denaturation */, ENZYME_INAC/*!< enzyme inactivation */};
/*! Types of mixing. For use in incubate_and_mix(). */
enum mixing {TAPPING, STIRRING, INVERTING, VORTEXING, RESUSPENDING, DISSOLVING, PIPETTING};
/*! Specifies the type of PCR being performed. For use in thermocycler(). */
enum pcr_type{NORMAL/*!< Regular PCR */, GRADIENT/*!< Gradient PCR */, COLONY/*!< Colony PCR */};
/*! Specifies the unit of the speed of the rotor in a centrifuge instruction. */
enum speed_type{RPM, G};
/*! Specifies the type of time constraint being employed. For use in time_constraint(). */
enum time_constraint_type{CURRENT_FLUID/*!< if \c time_constraint() is applicable to the subsequent use of the contents of a particular container. */, NEXTSTEP/*!< if \c time_constraint() is applicable between a pair of successive steps that use the contents of a container. */};
/*! Time units supported. */
enum time_unit{SECS/*!< seconds */, MINS/*!< minutes */, HRS/*!< hours */};
/*! Specifies certain conditions that might have to be satisfied for the completion of a step. For use in store_until(), vortex(), etc. */
enum until{ETHANOL_EVAP/*!< "until all the ethanol has evaporated and no fluid is visible in the tube".*/, OD/*!< "until the O.D.600 reaches 0.6". */, THAW/*!< "until the sample has thawed". */, COOLED/*!< "until cooled". */, COLOUR_DEVELOPS/*!< "until the colour develops". */, PPT_STOPS_STICKING/*!< "until the precipitate stops sticking to the walls of the tube". */, PELLET_DISLODGES/*!< "until the pellet dislodges". */, THAW_ICE/*!< "keep on ice until the sample has thawed" */};
/*! Volume units supported. */
enum vol_unit{UL/*!< microliter */, ML/*!< milliliter */, L/*!< liter */};
/*! Types of washing. For use in wash_slide(). */
enum wash_type{WASHING,RINSING};
/*! Weight units supported. For use in measure_solid(). */
enum weight_unit{UG/*!< microgram */, MG/*!< milligram */, GR/*!< gram */};

enum TLinkNames{DIS,MX,HT,SPLT,DCT,OT,STR,WST,DG,TrN,TrO};

enum XMLOPS{X_DIS, X_MX, X_HT, X_SPLT, X_DCT, X_OT, X_STR, X_DG, X_TrN, X_TrO};
enum XMLMIX{X_INVERT, X_MIX, X_STIR, X_TAP, X_VORTEX};

#define DEFAULT 25000000

#define BOILING_WATER 101
/*! \def BOILING_WATER
* \brief For use in store(), store_for() and store_until().
*/
#define ICE_COLD 3
/*! \def ICE_COLD
* \brief For use in declaring the state of a fluid in new_fluid().
*/
#define FOREVER -100
/*! \def FOREVER
* \brief For use in pcr_final_ext().
*/
#define NA -1
/*! \def NA
* \brief "Not Applicable". For use in mixing_table() and mixing_table_pcr() where "--" needs to be printed.
*/
#define ON_ICE 3
/*! \def ON_ICE
* \brief For use in store(), store_for() and store_until().
*/
#define OVERNIGHT 12
/*! \def OVERNIGHT
* \brief For use in incubate(), incubate_and_mix(), inoculation(), store(), store_for() and store_until().
*/
#define RT 28
/*! \def RT
* \brief Room Temperature.
*/
#define SPEED_MAX 20000
/*! \def SPEED_MAX
* \brief Maximum speed value to be used in centrifugation instructions.
*/
#define XVAL -2
/*! \def XVAL
* \brief 'X'(unknown) value. For use in mixing_table() and mixing_table_pcr() where "X" needs to be printed.
*/


#endif /* BIOENUMS_H_ */
