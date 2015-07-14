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
 * Source: Controls3D.java (Controls 3D)										*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: Basic GUI window that shows a dialog with the 3D controls.			*
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
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;
import javax.swing.border.BevelBorder;
import javax.swing.border.EtchedBorder;

public class Controls3D extends JDialog {

	private final JPanel contentPanel = new JPanel();
	private JPanel pnlAboutInfo;
	private JTable table;

	/**
	 * Create the dialog.
	 */
	public Controls3D() {
		setResizable(false);
		setTitle("3D Controls");
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
		
		table = new JTable();
		table.setShowVerticalLines(false);
		table.setBorder(null);
		table.setBackground(SystemColor.menu);
		table.setModel(new DefaultTableModel(
			new Object[][] {
				{"KEY", "MOVEMENT"},
				{"Left Arrow (+ Alt)", "Rotate Left (Slide Left)"},
				{"Right Arrow (+ Alt)", "Rotate Right (Slide Right)"},
				{"Up Arrow", "Move Forward"},
				{"Down Arrow", "Move Backward"},
				{"PgUp (+ Alt)", "Tilt Down (Slide Up)"},
				{"PgDn (+ Alt)", "Tilt Up (Slide Down)"},
				{"=", "Return To Center Of Universe"},
			},
			new String[] {
				"Key", "Movement"
			}
		) {
			Class[] columnTypes = new Class[] {
				String.class, Object.class
			};
			public Class getColumnClass(int columnIndex) {
				return columnTypes[columnIndex];
			}
		});
		table.getColumnModel().getColumn(0).setResizable(false);
		table.getColumnModel().getColumn(0).setPreferredWidth(169);
		table.getColumnModel().getColumn(1).setPreferredWidth(249);


		GroupLayout gl_pnlAboutInfo = new GroupLayout(pnlAboutInfo);
		gl_pnlAboutInfo.setHorizontalGroup(
			gl_pnlAboutInfo.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlAboutInfo.createSequentialGroup()
					.addContainerGap()
					.addComponent(table, GroupLayout.DEFAULT_SIZE, 341, Short.MAX_VALUE)
					.addContainerGap())
		);
		gl_pnlAboutInfo.setVerticalGroup(
			gl_pnlAboutInfo.createParallelGroup(Alignment.TRAILING)
				.addGroup(Alignment.LEADING, gl_pnlAboutInfo.createSequentialGroup()
					.addGap(37)
					.addComponent(table, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
					.addContainerGap(39, Short.MAX_VALUE))
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
