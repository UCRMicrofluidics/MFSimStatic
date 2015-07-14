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
/*------------------------Class/Implementation Details--------------------------*
 * Source: Placed3dViewerApp.java 												*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: This class will display a rotating 3D model of the placement stage.	*
 * 																				*
 * WARNING: If this file causes all kinds of errors, even after installing		*
 * Java3D properly, you may need to set the "Forbidden reference (access		*
 * rules)" under the "Deprecated and restricted API" from "Error" to "Warning",	*
 * as described at:																*
 * http://blog.js-development.com/2008/11/type-x-is-not-accessible-due-to.html	*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.views;

import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.Frame;
import java.awt.GraphicsConfiguration;

import javax.media.j3d.Alpha;
import javax.media.j3d.AmbientLight;
import javax.media.j3d.Appearance;
import javax.media.j3d.BoundingSphere;
import javax.media.j3d.BranchGroup;
import javax.media.j3d.Canvas3D;
import javax.media.j3d.ColoringAttributes;
import javax.media.j3d.DirectionalLight;
import javax.media.j3d.Font3D;
import javax.media.j3d.FontExtrusion;
import javax.media.j3d.Geometry;
import javax.media.j3d.GeometryArray;
import javax.media.j3d.LineArray;
import javax.media.j3d.Material;
import javax.media.j3d.Node;
import javax.media.j3d.RotationInterpolator;
import javax.media.j3d.Shape3D;
import javax.media.j3d.Text3D;
import javax.media.j3d.Transform3D;
import javax.media.j3d.TransformGroup;
import javax.media.j3d.TransparencyAttributes;
import javax.vecmath.Color3f;
import javax.vecmath.Point3d;
import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import com.sun.j3d.utils.applet.MainFrame;
import com.sun.j3d.utils.behaviors.keyboard.KeyNavigatorBehavior;
import com.sun.j3d.utils.geometry.Box;
import com.sun.j3d.utils.universe.SimpleUniverse;
import com.sun.j3d.utils.universe.ViewingPlatform;

import dmfbSimVisualizer.common.*;
import dmfbSimVisualizer.parsers.PlacedParser;

public class Placed3dViewerApp extends Applet {

	//////////////////////////////////////////////////////////////////////////////////////
	// Constructor
	//////////////////////////////////////////////////////////////////////////////////////
	public Placed3dViewerApp(String placedFName) {
		setLayout(new BorderLayout());
		GraphicsConfiguration config =
				SimpleUniverse.getPreferredConfiguration();

		Canvas3D canvas3D = new Canvas3D(config);
		add("Center", canvas3D);

		// SimpleUniverse is a Convenience Utility class
		SimpleUniverse simpleU = new SimpleUniverse(canvas3D);
		BranchGroup scene = createSceneGraph(simpleU, placedFName);
		simpleU.addBranchGraph(scene);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Causes the 3D shapes to rotate 360 degrees around a central axis
	//////////////////////////////////////////////////////////////////////////////////////
	private TransformGroup Rotate(Node node,Alpha alpha){

		TransformGroup xformGroup = new TransformGroup();
		xformGroup.setCapability(
				TransformGroup.ALLOW_TRANSFORM_WRITE);

		//Create an interpolator for rotating the node.
		RotationInterpolator interpolator = new RotationInterpolator(alpha,xformGroup);

		//Establish the animation region for this
		// interpolator.
		interpolator.setSchedulingBounds(new BoundingSphere(new Point3d(0.5,0.0,0.0),1.0));

		//Populate the xform group.
		xformGroup.addChild(interpolator);
		xformGroup.addChild(node);

		return xformGroup;

	}//end rotate

	//////////////////////////////////////////////////////////////////////////////////////
	// Some Java3D magic that just needs to be done.
	//////////////////////////////////////////////////////////////////////////////////////
	Shape3D createLand(){
		LineArray landGeom = new LineArray(44, GeometryArray.COORDINATES
				| GeometryArray.COLOR_3);
		float l = -50.0f;
		for(int c = 0; c < 44; c+=4){

			landGeom.setCoordinate( c+0, new Point3f( -50.0f, 0.0f,  l ));
			landGeom.setCoordinate( c+1, new Point3f(  50.0f, 0.0f,  l ));
			landGeom.setCoordinate( c+2, new Point3f(   l   , 0.0f, -50.0f ));
			landGeom.setCoordinate( c+3, new Point3f(   l   , 0.0f,  50.0f ));
			l += 10.0f;
		}

		Color3f c = new Color3f(0.1f, 0.8f, 0.1f);
		for(int i = 0; i < 44; i++) landGeom.setColor( i, c);

		return new Shape3D(landGeom);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// This function reads the input file, creates shapes which make-up the 3D placement
	// visualization, and places them at the appropriate places.
	//////////////////////////////////////////////////////////////////////////////////////
	public BranchGroup createSceneGraph(SimpleUniverse su, String placedFName) {
		// Create the root of the branch graph
		TransformGroup vpTrans = null;

		BranchGroup objRoot = new BranchGroup();

		Vector3f translate = new Vector3f();
		Transform3D T3D = new Transform3D();
		TransformGroup TG = null;

		//objRoot.addChild(createLand());
	
		PlacedParser pp = new PlacedParser(placedFName);
		//Set default display parameters
		int spinSpeed = 10000;
		int detail = 1;
		int numCellsX = pp.getNumXcells();
		int numCellsY = pp.getNumYcells();
		
		float increaseref = -1.0f/2.0f;
		float elecRad=Math.max((3f/(10f*numCellsX)), (3f/(10f*numCellsY)));
		float spacing=elecRad*1.5f;
		float cell = elecRad + spacing;
		//float spacing= .045f;

		// Set up colors
		Color3f black = new Color3f(0.0f, 0.0f, 0.0f);
		Color3f white = new Color3f(1.0f, 1.0f, 1.0f);
		Color3f red = new Color3f(0.7f, .15f, .15f);
		Color3f green = new Color3f(.1f, 0.65f, .1f);
		Color3f blue = new Color3f(.15f, .15f, 0.7f);
		Color3f gray = new Color3f(.2f, .2f, .2f);
		Color3f yellow = new Color3f(0.9f, 1.0f, 0.0f);
		Color3f tanSub = new Color3f(0.86f, 0.85f, 0.76f);

		// For transparency
		TransparencyAttributes ta = new TransparencyAttributes();
		ta.setTransparencyMode (ta.BLENDED);
		ta.setTransparency (0.5f);
		
		//set up the material
		Appearance apYellow = new Appearance();
		apYellow.setMaterial(new Material(yellow, black, yellow, black, 1.0f));
		Appearance apBlack = new Appearance();
		apBlack.setMaterial(new Material(black, black, black, black, 1.0f));
		Appearance apRed = new Appearance();
		apRed.setMaterial(new Material(red, black, red, black, 1.0f));
		Appearance apTanSub = new Appearance();
		apTanSub.setMaterial(new Material(tanSub, black, tanSub, black, 1.0f));
		apRed.setTransparencyAttributes (ta);

		Appearance apBlue = new Appearance();
		apBlue.setMaterial(new Material(blue, black, blue, black, 1.0f));
		Appearance apWhite = new Appearance();
		apWhite.setMaterial(new Material(white, black, white, black, 1.0f));
		Appearance apGray = new Appearance();
		apGray.setMaterial(new Material(gray, black, gray, black, 1.0f));
		Appearance apGreen = new Appearance();
		apGreen.setMaterial(new Material(green, black, green, black, 1.0f));
		Appearance apLine = new Appearance();
		ColoringAttributes ca = new ColoringAttributes(gray, ColoringAttributes.SHADE_FLAT);
		apLine.setColoringAttributes(ca);

		// Draw bottom substrate
		Box bSub;
		float baseX = ((float)(numCellsX-1) * cell)/2f; // X Offset
		float baseY = ((float)(numCellsY-1) * cell)/2f; // Y Offset
		float baseRadX = baseX + cell;
		float baseRadY = baseY + cell;
		bSub = new Box(baseRadX, 1f/50f, baseRadY, apTanSub);
		TransformGroup tgSub = new TransformGroup();
		tgSub.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
		Transform3D t3dSub = new Transform3D();
		Vector3f vSub = new Vector3f(0, (-1f/1.7f), 0);
		t3dSub.setTranslation(vSub);
		tgSub.setTransform(t3dSub);
		tgSub.addChild(bSub);
		objRoot.addChild(Rotate(tgSub, new Alpha(-1, spinSpeed)));		
	
		// Draw Electrodes				
		for (int x = 0; x < numCellsX; x++)
		{
			for (int y = 0; y < numCellsY; y++)
			{
				Box bElec;
				
				if (x == 0 && y == 0)
					bElec = new Box(elecRad, (1f/250f), elecRad, apYellow);
				else
					bElec = new Box(elecRad, (1f/250f), elecRad, apBlack);

				TransformGroup tgElec = new TransformGroup();
				tgElec.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
				Transform3D t3dElec = new Transform3D();				
				Vector3f vElec = new Vector3f(x*(cell)-baseX, (-1f/1.8f), y*(cell)-baseY);			
				t3dElec.setTranslation(vElec);
				tgElec.setTransform(t3dElec);
				tgElec.addChild(bElec);
				objRoot.addChild(Rotate(tgElec, new Alpha(-1, spinSpeed)));

			}
		}

		
		// Draw Reconfigurable Modules
		int maxTS = 0;
		for (int i = 0; i < pp.getReconfigAreas().size(); i++)
		{
			ReconfigArea ra = pp.getReconfigAreas().get(i);
			if (ra.stop_TS > maxTS)
				maxTS = (int)ra.stop_TS;
			float modTopHeight = (increaseref+((float)ra.stop_TS)/4f);
			float modBottomHeight = (increaseref+((float)ra.start_TS)/4f);
			float modHeight = Math.abs(modTopHeight-modBottomHeight);
			
			float xCenter = Math.abs((((float)ra.br_x-ra.tl_x)*cell)/2 + (((float)ra.tl_x)*cell));
			float yCenter = Math.abs((((float)ra.br_y-ra.tl_y)*cell)/2 + (((float)ra.tl_y)*cell));
			float xRad = (((float)ra.br_x)*cell) - xCenter + elecRad;
			float yRad = (((float)ra.br_y)*cell) - yCenter + elecRad;
			
			Box bMod;
			
			Color3f modColor = null;
			if (ra.opType.equals(OperationType.MIX))
				modColor = new Color3f(0.554f, 0.703f, 0.886f); // Blue
			else if (ra.opType.equals(OperationType.DILUTE))
				modColor = new Color3f(0.781f, 0.746f, 0.902f); // Purple
			else if (ra.opType.equals(OperationType.SPLIT))
				modColor = new Color3f(0.714f, 0.867f, 0.906f); // Light Blue
			else if (ra.opType.equals(OperationType.HEAT))
				modColor = new Color3f(0.964f, 0.585f, 0.273f); // Orange
			else if (ra.opType.equals(OperationType.DETECT))
				modColor = new Color3f(0.765f, 0.738f, 0.589f); // Tan
			else if (ra.opType.equals(OperationType.STORAGE))
				modColor = new Color3f(1.0f, 1.0f, 0.717f); // Light Yellow
					
			//set up the material & transparency
			Appearance apMod = new Appearance();
			apMod.setMaterial(new Material(modColor, black, modColor, black, 1.0f));
			TransparencyAttributes taMod = new TransparencyAttributes();
			taMod.setTransparencyMode (taMod.BLENDED);
			taMod.setTransparency (0.3f);
			apMod.setTransparencyAttributes (taMod);			
			
			bMod = new Box(xRad, modHeight/2, yRad, apMod);
			TransformGroup tgMod = new TransformGroup();
			tgMod.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
			Transform3D t3dMod = new Transform3D();
			Vector3f vMod = new Vector3f(xCenter-baseX, modBottomHeight + modHeight/2, yCenter-baseY);
			t3dMod.setTranslation(vMod);
			tgMod.setTransform(t3dMod);
			tgMod.addChild(bMod);
			objRoot.addChild(Rotate(tgMod, new Alpha(-1, spinSpeed)));
			
			// Draw the module Name
			Font my2DFont = new Font("Arial", Font.PLAIN, 1 );
			FontExtrusion myExtrude = new FontExtrusion( );			
			Font3D fTS = new Font3D(my2DFont, myExtrude);
			Text3D tTS = new Text3D(fTS, ra.name);
			Shape3D tsName = new Shape3D(tTS, apMod);
			TransformGroup tgTS = new TransformGroup();
			tgTS.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
			Transform3D t3dTS = new Transform3D();
			t3dTS.setScale(.075);
			Vector3f vTS = new Vector3f(xCenter-baseX-xRad/2, modBottomHeight + modHeight/2, yCenter-baseY);
			t3dTS.setTranslation(vTS);
			tgTS.setTransform(t3dTS);
			tgTS.addChild(tsName);
			objRoot.addChild(Rotate(tgTS, new Alpha(-1, spinSpeed)));
		}

		// Draw time-step planes
		for (int i = 0 ; i <= maxTS ; i++)
		{
			float tsHeight = (increaseref+((float)i)/4f);
			//System.out.println("TS " + i + " is at height: " + tsHeight);		
			
			
			// Add TS plane
			Box bTS = new Box(baseRadX, 1f/1000f, baseRadY, apRed);		
			TransformGroup tgTS = new TransformGroup();
			tgTS.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
			Transform3D t3dTS = new Transform3D();
			Vector3f vTS = new Vector3f(0, tsHeight, 0);
			t3dTS.setTranslation(vTS);
			tgTS.setTransform(t3dTS);
			tgTS.addChild(bTS);
			objRoot.addChild(Rotate(tgTS, new Alpha(-1, spinSpeed)));
			
			// Add TS number
			Font my2DFont = new Font("Arial", Font.PLAIN, 1 );
			FontExtrusion myExtrude = new FontExtrusion( );			
			Font3D fTS = new Font3D(my2DFont, myExtrude);
			Text3D tTS = new Text3D(fTS, Integer.toString(i));
			Shape3D tsName = new Shape3D(tTS);
			tgTS = new TransformGroup();
			tgTS.setCapability(TransformGroup.ALLOW_TRANSFORM_WRITE);//allows movement
			t3dTS = new Transform3D();
			t3dTS.setScale(.1);
			vTS = new Vector3f(baseX, tsHeight, 0);
			t3dTS.setTranslation(vTS);
			tgTS.setTransform(t3dTS);
			tgTS.addChild(tsName);
			objRoot.addChild(Rotate(tgTS, new Alpha(-1, spinSpeed)));
			
		}
		
		// Create/add light
		Color3f lightColor = new Color3f(1f, 1f, 1f);
		BoundingSphere bounds = new BoundingSphere(new Point3d(0.0,0.0,0.0), 100.0);
		Vector3f lightDirection = new Vector3f(4.0f, -7.0f, -12.0f);
		DirectionalLight light = new DirectionalLight(lightColor, lightDirection);
		light.setInfluencingBounds(bounds);
		objRoot.addChild(light);

		//Create/add ambient light
		AmbientLight ambientLight = new AmbientLight(new Color3f(.5f,.5f,.5f));
		ambientLight.setInfluencingBounds(bounds);
		objRoot.addChild(ambientLight);
		
		// Set navigational controls
		vpTrans = su.getViewingPlatform().getViewPlatformTransform();
		translate.set( 0.0f, 0.3f, 0.0f);
		T3D.setTranslation(translate);
		vpTrans.setTransform(T3D);
		KeyNavigatorBehavior keyNavBeh = new KeyNavigatorBehavior(vpTrans);
		keyNavBeh.setSchedulingBounds(new BoundingSphere(new Point3d(),1000.0));
		objRoot.addChild(keyNavBeh);

		// Let Java 3D perform optimizations on this scene graph.
		objRoot.compile();

		return objRoot;
	} // end of CreateSceneGraph method of KeyNavigatorApp

} // end of class KeyNavigatorApp