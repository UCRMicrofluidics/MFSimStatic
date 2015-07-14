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
 * Source: AboutUs.java (About Us)												*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Basic GUI window that shows an about us dialog.						*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package dmfbSimVisualizer.views;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.FlowLayout;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import java.awt.Toolkit;
import java.awt.Dimension;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JTextArea;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.SystemColor;
import javax.swing.JEditorPane;
import javax.swing.JTextPane;
import java.awt.Font;

public class AboutUs extends JDialog {

	private final JPanel contentPanel = new JPanel();
	private JPanel pnlAboutInfo;

	/**
	 * Create the dialog.
	 */
	public AboutUs() {
		setResizable(false);
		setTitle("About Us");
		setIconImage(Toolkit.getDefaultToolkit().getImage(AboutUs.class.getResource("/dmfbSimVisualizer/resources/logo_32.png")));
		setBounds(100, 100, 461, 273);
		getContentPane().setLayout(new BorderLayout());
		contentPanel.setBorder(new EmptyBorder(5, 5, 5, 5));
		getContentPane().add(contentPanel, BorderLayout.CENTER);

		
		JPanel pnlAboutImage1 = new ImagePanel(new ImageIcon(Main.class.getResource("/dmfbSimVisualizer/resources/logo_64.png")));
		JPanel pnlAboutImage2 = new ImagePanel(new ImageIcon(Main.class.getResource("/dmfbSimVisualizer/resources/dmfb_64.png")));
		JPanel pnlAboutImage3 = new ImagePanel(new ImageIcon(Main.class.getResource("/dmfbSimVisualizer/resources/driver_64.png")));
		
		pnlAboutInfo = new JPanel();
		
		GroupLayout gl_contentPanel = new GroupLayout(contentPanel);
		gl_contentPanel.setHorizontalGroup(
				gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
						.addContainerGap()
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING)
								.addComponent(pnlAboutImage3, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)
								.addGroup(gl_contentPanel.createParallelGroup(Alignment.LEADING)
										.addComponent(pnlAboutImage1, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)
										.addComponent(pnlAboutImage2, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)))
										.addPreferredGap(ComponentPlacement.RELATED)
										.addComponent(pnlAboutInfo, GroupLayout.DEFAULT_SIZE, 361, Short.MAX_VALUE)
										.addGap(4))
				);
		gl_contentPanel.setVerticalGroup(
				gl_contentPanel.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_contentPanel.createSequentialGroup()
						.addGroup(gl_contentPanel.createParallelGroup(Alignment.TRAILING, false)
								.addComponent(pnlAboutInfo, Alignment.LEADING, 0, 0, Short.MAX_VALUE)
								.addGroup(Alignment.LEADING, gl_contentPanel.createSequentialGroup()
										.addComponent(pnlAboutImage1, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)
										.addPreferredGap(ComponentPlacement.RELATED)
										.addComponent(pnlAboutImage2, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)
										.addPreferredGap(ComponentPlacement.RELATED)
										.addComponent(pnlAboutImage3, GroupLayout.PREFERRED_SIZE, 64, GroupLayout.PREFERRED_SIZE)))
										.addContainerGap(43, Short.MAX_VALUE))
				);

		JLabel lblUniversity = new JLabel("University of California, Riverside");		
		JLabel lblDepartment = new JLabel("Department of Computer Engineering and Science");		
		JLabel lblTitle = new JLabel("DMFB Static Simulator Visualizer");
		lblTitle.setFont(new Font("Tahoma", Font.BOLD, 13));		
		JLabel lblVersion = new JLabel("Version 3.0");		
		JLabel lblLab = new JLabel("Microfluidics Lab");		
		JLabel lblWeb = new JLabel("www.microfluidics.cs.ucr.edu");


		GroupLayout gl_pnlAboutInfo = new GroupLayout(pnlAboutInfo);
		gl_pnlAboutInfo.setHorizontalGroup(
			gl_pnlAboutInfo.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlAboutInfo.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_pnlAboutInfo.createParallelGroup(Alignment.LEADING)
						.addComponent(lblDepartment)
						.addComponent(lblUniversity)
						.addComponent(lblTitle)
						.addComponent(lblLab)
						.addComponent(lblVersion)
						.addComponent(lblWeb))
					.addContainerGap(112, Short.MAX_VALUE))
		);
		gl_pnlAboutInfo.setVerticalGroup(
			gl_pnlAboutInfo.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlAboutInfo.createSequentialGroup()
					.addGap(29)
					.addComponent(lblTitle)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblVersion)
					.addGap(29)
					.addComponent(lblUniversity)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblDepartment)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblLab)
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(lblWeb)
					.addContainerGap(36, Short.MAX_VALUE))
		);
		pnlAboutInfo.setLayout(gl_pnlAboutInfo);
		contentPanel.setLayout(gl_contentPanel);
		{
			JPanel buttonPane = new JPanel();
			getContentPane().add(buttonPane, BorderLayout.SOUTH);
			buttonPane.setLayout(new FlowLayout(FlowLayout.CENTER, 5, 5));
			{
				JButton okButton = new JButton("OK");
				okButton.addActionListener(new ActionListener() {
					public void actionPerformed(ActionEvent e) {
						dispose();
					}
				});
				okButton.setActionCommand("OK");
				buttonPane.add(okButton);
				getRootPane().setDefaultButton(okButton);
			}
		}
	}
}
