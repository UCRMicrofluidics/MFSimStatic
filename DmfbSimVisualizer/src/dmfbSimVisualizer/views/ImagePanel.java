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
 * Source: ImagePanel.java (Image Panel)										*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Basic utility GUI window that is used to display an image.			*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.views;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;

import javax.swing.ImageIcon;
import javax.swing.JPanel;

import dmfbSimVisualizer.common.MFError;

public class ImagePanel extends JPanel {

	private Image img;

	/**
	 * Create the panel.
	 */

	/**
	 * @wbp.parser.constructor
	 */
	//////////////////////////////////////////////////////////////////////////////////////
	// Constructors
	//////////////////////////////////////////////////////////////////////////////////////
	public ImagePanel(String img) {
		this(new ImageIcon(img).getImage());
	}	
	public ImagePanel(ImageIcon imIcon) {
		this(imIcon.getImage());
	}
	public ImagePanel(Image img) {
		this.img = img;
		Dimension size = new Dimension(img.getWidth(null), img.getHeight(null));
		setPreferredSize(new Dimension(img.getWidth(null), img.getHeight(null)));
		setMinimumSize(size);
		setMaximumSize(size);
		setSize(size);
		setBounds(0, 0, img.getWidth(null), img.getHeight(null));
		setLayout(null);
		repaint();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////
	// Updates the image in this panel
	//////////////////////////////////////////////////////////////////////////////////////
	public void updateImage(Image img)
	{
		this.img = img;
	}
	public void updateImage(ImageIcon imIcon)
	{
		updateImage(imIcon.getImage());
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Does the painting of the image.  Draws the image, keeping it's aspect ratio. If
	// container is bigger than image, does not stretch. If container is smaller than
	// image, then it should reduce in size, while keeping aspect ratio.
	//////////////////////////////////////////////////////////////////////////////////////
	public void paintComponent(Graphics g) {
		Dimension d = getSize();		
		
		// Fill entire box, not keeping original aspect ratio
		int imageWidth = Math.min(d.width, img.getWidth(null));
		int imageHeight = Math.min(d.height, img.getHeight(null));
		
		// Keep proper aspect ratio	
		if (d.width > img.getWidth(null) && d.height > img.getHeight(null))
		{
			imageWidth = img.getWidth(null);
			imageHeight = img.getHeight(null);
		}	
		else if (img.getHeight(null) > img.getWidth(null))
		{
			double ratio = (double)img.getWidth(null) / (double)img.getHeight(null);
			int newWidth = (int)((double)d.height * ratio);

			if (newWidth <= d.width)
			{				
				imageWidth = newWidth;
				imageHeight = d.height;
			}
			else
			{
				ratio = 1 / ratio;
				int newHeight = (int)((double)d.width * ratio);
				imageWidth = d.width;
				imageHeight = newHeight;
			}
		}
		else
		{
			double ratio = (double)img.getHeight(null) / (double)img.getWidth(null);
			int newHeight = (int)((double)d.width * ratio);

			if (newHeight <= d.height)
			{
				imageWidth = d.width;
				imageHeight = newHeight;
			}
			else
			{
				ratio = 1 / ratio;
				int newWidth = (int)((double)d.height * ratio);
				imageWidth = newWidth;
				imageHeight = d.height;

			}
		}
		g.drawImage(img, 0, 0, imageWidth, imageHeight, null);
	}

}
