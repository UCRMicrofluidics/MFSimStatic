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
 * Source: Main.java (Main)														*
 * Original Code Author(s): Dan Grissom											*
 * Original Completion/Release Date: October 7, 2012							*
 *																				*
 * Details: GUI frame/window that shows the main controls of the program. This	*
 * program is just a simple java GUI wrapper that calls the MFSimStatic c++		*
 * program.																		*
 *																				*
 * Revision History:															*
 * WHO		WHEN		WHAT													*
 * ---		----		----													*
 * FML		MM/DD/YY	One-line description									*
 *-----------------------------------------------------------------------------*/

package mfSimStaticGUI.views;

import java.awt.BorderLayout;
import java.awt.EventQueue;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileFilter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.nio.channels.FileChannel;
import java.util.Arrays;
import java.util.Comparator;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import mfSimStaticGUI.common.MFError;
import mfSimStaticGUI.common.SimulationThread;

import javax.swing.ButtonGroup;
import javax.swing.UIManager;
import javax.swing.GroupLayout;
import javax.swing.GroupLayout.Alignment;
import javax.swing.ImageIcon;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import java.awt.Toolkit;
import javax.swing.JLabel;
import javax.swing.JComboBox;
import javax.swing.JButton;
import javax.swing.DefaultComboBoxModel;
import javax.swing.LayoutStyle.ComponentPlacement;
import javax.swing.JTextField;
import javax.swing.JSeparator;
import java.awt.Color;
import javax.swing.JTextPane;
import javax.swing.JTextArea;
import javax.swing.JScrollPane;
import javax.swing.border.TitledBorder;
import javax.swing.border.BevelBorder;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JTabbedPane;
import java.awt.Font;
import javax.swing.JRadioButton;

public class Main extends JFrame {

	String del = System.getProperty("file.separator");
	String progFileName = System.getProperty("user.dir") + del + "MFS.exe";
	private volatile boolean isSimulating = false;
	private Main mainPtr;
	private Process simProcess; // the external simulation process

	private JPanel contentPane;
	private JComboBox cbBins;
	private JButton btnSelectBinary;
	private JComboBox cbSynthesis;
	private JComboBox cbScheduler;
	private JComboBox cbPlacer;
	private JComboBox cbRouter;
	private JComboBox cbCompType;
	private JComboBox cbProcEngType;
	private JComboBox cbExType;
	private JTextField txtSchedOut;
	private JTextField txtPlaceIn;
	private JTextField txtPlaceOut;
	private JTextField txtRouteIn;
	private JComboBox cbAssay;
	private JComboBox cbDmfbArch;
	private JButton btnSynthesize;
	public JTextArea txtOutput;
	private JComboBox cbPinMapping;
	private JPanel pnlGeneral;
	private JPanel pnlScheduling;
	private JPanel pnlPlacement;
	private JPanel pnlRouting;
	private JPanel pnlExecute;
	private JTabbedPane tpSynthesis;
	private JComboBox cbResAlloc;
	private JComboBox cbWireRouteType;
	private JSpinner spnHorizTracks;
	private JSpinner spnVertTracks;
	private JTextField txtWireRouteIn;
	private JSpinner spnModSpace;
	private JSpinner spnStorageDrops;
	private JRadioButton btnWashNo;
	private JRadioButton btnWashYes;
	private ButtonGroup grpWashDroplet;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Throwable e) {
			e.printStackTrace();
		}
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					Main frame = new Main();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	public Main() {
		setIconImage(Toolkit.getDefaultToolkit().getImage(Main.class.getResource("/mfSimStaticGUI/resources/logo_32.png")));
		setTitle("DMFB Static Simulator GUI");		
		initComponents();
		createEvents();
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// initComponents()
	// Created using Eclipse WindowBuilder Pro Plugin. Creates the windows and components
	// and lays them out accordingly. 	 
	//////////////////////////////////////////////////////////////////////////////////////
	private void initComponents()
	{		
		mainPtr = this;

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 674, 424);

		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);

		JMenu mnFile = new JMenu("File");
		menuBar.add(mnFile);

		JMenuItem mntmExit = new JMenuItem("Exit");
		mnFile.add(mntmExit);
		mntmExit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				dispose();
			}
		});
		mntmExit.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/exit_16.png")));

		JMenu mnHelp = new JMenu("Help");
		menuBar.add(mnHelp);

		JMenuItem mntmAboutUs = new JMenuItem("About Us...");
		mnHelp.add(mntmAboutUs);
		mntmAboutUs.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				AboutUs about = new AboutUs();
				about.setModalityType(java.awt.Dialog.ModalityType.APPLICATION_MODAL);
				about.setVisible(true);
			}
		});
		mntmAboutUs.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/about_16.png")));



		contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);

		tpSynthesis = new JTabbedPane(JTabbedPane.TOP);

		pnlGeneral = new JPanel();
		tpSynthesis.addTab("General", null, pnlGeneral, null);

		JLabel lblBinaryLocation = new JLabel("Binary Location:");

		cbBins = new JComboBox();

		btnSelectBinary = new JButton("Select Binary");
		btnSelectBinary.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/bin_16.png")));

		JLabel lblSynthesisType = new JLabel("Synthesis Type:");

		cbSynthesis = new JComboBox();
		cbSynthesis.setModel(new DefaultComboBoxModel(new String[] {"Entire Flow", "Schedule", "Place", "Route", "Wire Route"}));
		cbSynthesis.setEnabled(false);

		JLabel lblPinMapping = new JLabel("Pin Mapping:");

		cbPinMapping = new JComboBox();
		cbPinMapping.setEnabled(false);
		GroupLayout gl_pnlGeneral = new GroupLayout(pnlGeneral);
		gl_pnlGeneral.setHorizontalGroup(
			gl_pnlGeneral.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlGeneral.createSequentialGroup()
					.addGap(21)
					.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.TRAILING)
						.addComponent(lblPinMapping)
						.addComponent(lblBinaryLocation)
						.addComponent(lblSynthesisType))
					.addGap(18)
					.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.LEADING)
						.addComponent(cbSynthesis, Alignment.TRAILING, 0, 396, Short.MAX_VALUE)
						.addComponent(cbPinMapping, 0, 396, Short.MAX_VALUE)
						.addComponent(cbBins, Alignment.TRAILING, 0, 396, Short.MAX_VALUE))
					.addPreferredGap(ComponentPlacement.RELATED)
					.addComponent(btnSelectBinary)
					.addContainerGap())
		);
		gl_pnlGeneral.setVerticalGroup(
			gl_pnlGeneral.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlGeneral.createSequentialGroup()
					.addGap(7)
					.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlGeneral.createSequentialGroup()
							.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.BASELINE)
								.addComponent(cbBins, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(btnSelectBinary))
							.addGap(10)
							.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.BASELINE)
								.addComponent(cbSynthesis, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
								.addComponent(lblSynthesisType)))
						.addComponent(lblBinaryLocation))
					.addGap(18)
					.addGroup(gl_pnlGeneral.createParallelGroup(Alignment.BASELINE)
						.addComponent(cbPinMapping, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblPinMapping))
					.addGap(227))
		);
		pnlGeneral.setLayout(gl_pnlGeneral);

		pnlScheduling = new JPanel();
		tpSynthesis.addTab("Scheduling", null, pnlScheduling, null);
		tpSynthesis.setEnabledAt(1, false);

		JLabel lblScheduler = new JLabel("Scheduler:");

		cbScheduler = new JComboBox();
		cbScheduler.setEnabled(false);

		JLabel lblAssay = new JLabel("Assay:");

		cbAssay = new JComboBox();
		cbAssay.setEnabled(false);

		JLabel lblDmfbArch = new JLabel("Dmfb Arch:");

		cbDmfbArch = new JComboBox();
		cbDmfbArch.setEnabled(false);

		JLabel lblSchedOut = new JLabel("Output:");

		txtSchedOut = new JTextField();
		txtSchedOut.setEnabled(false);
		txtSchedOut.setColumns(10);
		
		JLabel lblResAlloc = new JLabel("Res. Alloc. Type:");
		
		cbResAlloc = new JComboBox();
		cbResAlloc.setEnabled(false);
		
		JLabel label = new JLabel("Drops Per Storage Mod:");
		
		spnStorageDrops = new JSpinner();
		spnStorageDrops.setEnabled(false);
		
		JLabel lblMisc = new JLabel("Misc:");
		GroupLayout gl_pnlScheduling = new GroupLayout(pnlScheduling);
		gl_pnlScheduling.setHorizontalGroup(
			gl_pnlScheduling.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlScheduling.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.TRAILING)
						.addComponent(lblScheduler)
						.addComponent(lblResAlloc)
						.addComponent(lblAssay)
						.addComponent(lblDmfbArch)
						.addComponent(lblSchedOut)
						.addComponent(lblMisc))
					.addGap(18)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlScheduling.createSequentialGroup()
							.addComponent(label, GroupLayout.PREFERRED_SIZE, 115, GroupLayout.PREFERRED_SIZE)
							.addGap(4)
							.addComponent(spnStorageDrops, GroupLayout.PREFERRED_SIZE, 31, GroupLayout.PREFERRED_SIZE))
						.addComponent(cbAssay, 0, 523, Short.MAX_VALUE)
						.addComponent(cbDmfbArch, 0, 523, Short.MAX_VALUE)
						.addComponent(txtSchedOut, GroupLayout.DEFAULT_SIZE, 523, Short.MAX_VALUE)
						.addComponent(cbResAlloc, 0, 523, Short.MAX_VALUE)
						.addComponent(cbScheduler, 0, 523, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_pnlScheduling.setVerticalGroup(
			gl_pnlScheduling.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlScheduling.createSequentialGroup()
					.addGap(5)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
						.addComponent(cbScheduler, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblScheduler))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
						.addComponent(cbResAlloc, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblResAlloc))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
						.addComponent(cbAssay, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblAssay))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
						.addComponent(cbDmfbArch, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblDmfbArch))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
						.addComponent(txtSchedOut, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblSchedOut))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlScheduling.createSequentialGroup()
							.addGap(3)
							.addGroup(gl_pnlScheduling.createParallelGroup(Alignment.BASELINE)
								.addComponent(label)
								.addComponent(lblMisc)))
						.addComponent(spnStorageDrops, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(175, Short.MAX_VALUE))
		);
		pnlScheduling.setLayout(gl_pnlScheduling);

		pnlPlacement = new JPanel();
		tpSynthesis.addTab("Placement", null, pnlPlacement, null);
		tpSynthesis.setEnabledAt(2, false);

		JLabel lblPlacer = new JLabel("Placer:");

		cbPlacer = new JComboBox();
		cbPlacer.setEnabled(false);

		JLabel lblPlaceIn = new JLabel("Input:");

		txtPlaceIn = new JTextField();
		txtPlaceIn.setEnabled(false);
		txtPlaceIn.setColumns(10);

		JLabel lblPlaceOut = new JLabel("Output:");

		txtPlaceOut = new JTextField();
		txtPlaceOut.setEnabled(false);
		txtPlaceOut.setColumns(10);
		
		JLabel label_1 = new JLabel("Cells Between Modules:");
		
		spnModSpace = new JSpinner();
		spnModSpace.setEnabled(false);
		
		JLabel lblMisc_1 = new JLabel("Misc:");
		GroupLayout gl_pnlPlacement = new GroupLayout(pnlPlacement);
		gl_pnlPlacement.setHorizontalGroup(
			gl_pnlPlacement.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlPlacement.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.TRAILING)
						.addComponent(lblPlacer)
						.addComponent(lblPlaceIn)
						.addComponent(lblPlaceOut)
						.addComponent(lblMisc_1))
					.addGap(10)
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlPlacement.createSequentialGroup()
							.addComponent(label_1, GroupLayout.PREFERRED_SIZE, 113, GroupLayout.PREFERRED_SIZE)
							.addGap(4)
							.addComponent(spnModSpace, GroupLayout.PREFERRED_SIZE, 31, GroupLayout.PREFERRED_SIZE))
						.addComponent(cbPlacer, 0, 567, Short.MAX_VALUE)
						.addComponent(txtPlaceIn, GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE)
						.addComponent(txtPlaceOut, GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_pnlPlacement.setVerticalGroup(
			gl_pnlPlacement.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlPlacement.createSequentialGroup()
					.addGap(5)
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblPlacer)
						.addComponent(cbPlacer, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.BASELINE)
						.addComponent(txtPlaceIn, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblPlaceIn))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.BASELINE)
						.addComponent(txtPlaceOut, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblPlaceOut))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlPlacement.createSequentialGroup()
							.addGap(3)
							.addGroup(gl_pnlPlacement.createParallelGroup(Alignment.BASELINE)
								.addComponent(label_1)
								.addComponent(lblMisc_1)))
						.addComponent(spnModSpace, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(209, Short.MAX_VALUE))
		);
		pnlPlacement.setLayout(gl_pnlPlacement);

		pnlRouting = new JPanel();
		tpSynthesis.addTab("Routing", null, pnlRouting, null);
		tpSynthesis.setEnabledAt(3, false);

		JLabel lblRouter = new JLabel("Router:");

		cbRouter = new JComboBox();
		cbRouter.setEnabled(false);

		JLabel lblInput = new JLabel("Input:");

		txtRouteIn = new JTextField();
		txtRouteIn.setEnabled(false);
		txtRouteIn.setColumns(10);

		JLabel lblCompactionType = new JLabel("Compaction Type:");

		cbCompType = new JComboBox();
		cbCompType.setEnabled(false);

		JLabel lblProcessingEngineType = new JLabel("Processing Engine Type:");

		cbProcEngType = new JComboBox();
		cbProcEngType.setEnabled(false);

		JLabel lblExecutionType = new JLabel("Execution Type:");

		cbExType = new JComboBox();
		cbExType.setEnabled(false);
		
		JLabel lblWashDroplets = new JLabel("Wash Droplets:");		
		btnWashYes = new JRadioButton("Yes");		
		btnWashNo = new JRadioButton("No");
		btnWashNo.setSelected(true);		
		grpWashDroplet = new ButtonGroup();
		grpWashDroplet.add(btnWashYes);
		grpWashDroplet.add(btnWashNo);		
		
		GroupLayout gl_pnlRouting = new GroupLayout(pnlRouting);
		gl_pnlRouting.setHorizontalGroup(
			gl_pnlRouting.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlRouting.createSequentialGroup()
					.addContainerGap()
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.TRAILING)
						.addComponent(lblWashDroplets)
						.addComponent(lblExecutionType)
						.addComponent(lblProcessingEngineType)
						.addComponent(lblCompactionType)
						.addComponent(lblInput)
						.addComponent(lblRouter))
					.addGap(18)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.LEADING)
						.addComponent(cbRouter, 0, 488, Short.MAX_VALUE)
						.addComponent(txtRouteIn, GroupLayout.DEFAULT_SIZE, 488, Short.MAX_VALUE)
						.addComponent(cbCompType, 0, 488, Short.MAX_VALUE)
						.addComponent(cbProcEngType, 0, 488, Short.MAX_VALUE)
						.addComponent(cbExType, 0, 488, Short.MAX_VALUE)
						.addGroup(gl_pnlRouting.createSequentialGroup()
							.addComponent(btnWashYes)
							.addPreferredGap(ComponentPlacement.UNRELATED)
							.addComponent(btnWashNo)))
					.addContainerGap())
		);
		gl_pnlRouting.setVerticalGroup(
			gl_pnlRouting.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlRouting.createSequentialGroup()
					.addGap(5)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblRouter)
						.addComponent(cbRouter, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblInput)
						.addComponent(txtRouteIn, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblCompactionType)
						.addComponent(cbCompType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblProcessingEngineType)
						.addComponent(cbProcEngType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addPreferredGap(ComponentPlacement.UNRELATED)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblExecutionType)
						.addComponent(cbExType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(18)
					.addGroup(gl_pnlRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblWashDroplets)
						.addComponent(btnWashYes)
						.addComponent(btnWashNo))
					.addContainerGap(146, Short.MAX_VALUE))
		);
		pnlRouting.setLayout(gl_pnlRouting);
		
		JPanel pnlWireRouting = new JPanel();
		tpSynthesis.addTab("Wire Routing", null, pnlWireRouting, null);
		tpSynthesis.setEnabledAt(4, false);
		
		JLabel lblWireRoutingType = new JLabel("Wire Routing Type:");
		
		cbWireRouteType = new JComboBox();
		cbWireRouteType.setEnabled(false);
		
		JLabel lblNumberRoutingTracks = new JLabel("Number Routing Tracks:");
		
		JLabel lblHorizontal = new JLabel("Horizontal:");
		
		spnHorizTracks = new JSpinner();
		spnHorizTracks.setModel(new SpinnerNumberModel(3, 1, 10, 1));
		
		JLabel lblVeritcal = new JLabel("Veritcal:");
		
		spnVertTracks = new JSpinner();
		spnVertTracks.setModel(new SpinnerNumberModel(3, 1, 10, 1));
		
		JLabel lblWrInput = new JLabel("Input:");
		
		txtWireRouteIn = new JTextField();
		txtWireRouteIn.setEnabled(false);
		txtWireRouteIn.setColumns(10);
		GroupLayout gl_pnlWireRouting = new GroupLayout(pnlWireRouting);
		gl_pnlWireRouting.setHorizontalGroup(
			gl_pnlWireRouting.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlWireRouting.createSequentialGroup()
					.addGap(43)
					.addGroup(gl_pnlWireRouting.createParallelGroup(Alignment.TRAILING)
						.addComponent(lblWrInput)
						.addComponent(lblWireRoutingType)
						.addComponent(lblNumberRoutingTracks))
					.addGap(18)
					.addGroup(gl_pnlWireRouting.createParallelGroup(Alignment.LEADING)
						.addGroup(gl_pnlWireRouting.createSequentialGroup()
							.addComponent(lblHorizontal)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(spnHorizTracks, GroupLayout.PREFERRED_SIZE, 46, GroupLayout.PREFERRED_SIZE)
							.addGap(18)
							.addComponent(lblVeritcal)
							.addPreferredGap(ComponentPlacement.RELATED)
							.addComponent(spnVertTracks, GroupLayout.PREFERRED_SIZE, 47, GroupLayout.PREFERRED_SIZE))
						.addComponent(txtWireRouteIn, GroupLayout.DEFAULT_SIZE, 441, Short.MAX_VALUE)
						.addComponent(cbWireRouteType, 0, 441, Short.MAX_VALUE))
					.addContainerGap())
		);
		gl_pnlWireRouting.setVerticalGroup(
			gl_pnlWireRouting.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlWireRouting.createSequentialGroup()
					.addGap(23)
					.addGroup(gl_pnlWireRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblWireRoutingType)
						.addComponent(cbWireRouteType, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(18)
					.addGroup(gl_pnlWireRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblWrInput)
						.addComponent(txtWireRouteIn, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addGap(18)
					.addGroup(gl_pnlWireRouting.createParallelGroup(Alignment.BASELINE)
						.addComponent(lblNumberRoutingTracks)
						.addComponent(lblHorizontal)
						.addComponent(spnHorizTracks, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE)
						.addComponent(lblVeritcal)
						.addComponent(spnVertTracks, GroupLayout.PREFERRED_SIZE, GroupLayout.DEFAULT_SIZE, GroupLayout.PREFERRED_SIZE))
					.addContainerGap(214, Short.MAX_VALUE))
		);
		pnlWireRouting.setLayout(gl_pnlWireRouting);

		pnlExecute = new JPanel();
		tpSynthesis.addTab("Execute", null, pnlExecute, null);
		tpSynthesis.setEnabledAt(5, false);

		JLabel lblOutput = new JLabel("Output:");

		JScrollPane scrollPane = new JScrollPane();

		txtOutput = new JTextArea();
		txtOutput.setEditable(false);
		scrollPane.setViewportView(txtOutput);

		btnSynthesize = new JButton("Synthesize");
		btnSynthesize.setEnabled(false);
		btnSynthesize.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/play_16.png")));

		JLabel lblnoteNotAll = new JLabel("(Note: Not all synthesis sub-methods and configurations are compatible with each other)");
		lblnoteNotAll.setFont(new Font("Tahoma", Font.BOLD, 11));
		GroupLayout gl_pnlExecute = new GroupLayout(pnlExecute);
		gl_pnlExecute.setHorizontalGroup(
				gl_pnlExecute.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlExecute.createSequentialGroup()
						.addContainerGap()
						.addGroup(gl_pnlExecute.createParallelGroup(Alignment.LEADING)
								.addGroup(gl_pnlExecute.createSequentialGroup()
										.addComponent(btnSynthesize)
										.addPreferredGap(ComponentPlacement.UNRELATED)
										.addComponent(lblnoteNotAll))
										.addGroup(gl_pnlExecute.createSequentialGroup()
												.addComponent(lblOutput)
												.addPreferredGap(ComponentPlacement.UNRELATED)
												.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 655, Short.MAX_VALUE)))
												.addContainerGap())
				);
		gl_pnlExecute.setVerticalGroup(
				gl_pnlExecute.createParallelGroup(Alignment.LEADING)
				.addGroup(gl_pnlExecute.createSequentialGroup()
						.addGap(10)
						.addGroup(gl_pnlExecute.createParallelGroup(Alignment.BASELINE)
								.addComponent(btnSynthesize)
								.addComponent(lblnoteNotAll))
								.addPreferredGap(ComponentPlacement.UNRELATED)
								.addGroup(gl_pnlExecute.createParallelGroup(Alignment.LEADING)
										.addComponent(lblOutput)
										.addComponent(scrollPane, GroupLayout.DEFAULT_SIZE, 366, Short.MAX_VALUE))
										.addContainerGap())
				);
		pnlExecute.setLayout(gl_pnlExecute);
		GroupLayout gl_contentPane = new GroupLayout(contentPane);
		gl_contentPane.setHorizontalGroup(
				gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addComponent(tpSynthesis, Alignment.TRAILING, GroupLayout.PREFERRED_SIZE, 628, Short.MAX_VALUE)
				);
		gl_contentPane.setVerticalGroup(
				gl_contentPane.createParallelGroup(Alignment.LEADING)
				.addComponent(tpSynthesis, Alignment.TRAILING, GroupLayout.DEFAULT_SIZE, 355, Short.MAX_VALUE)
				);
		contentPane.setLayout(gl_contentPane);

		getLocalBinaries();		
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Gets any Binaries available in the local director, and in the Debug and Release
	// folders and gives the locations/binaries as options in the binary combo box.
	//////////////////////////////////////////////////////////////////////////////////////
	private void getLocalBinaries()
	{
		String [] searchFolders = {"Debug","Release",System.getProperty("user.dir")};

		cbBins.removeAllItems();

		for (int i=0; i < searchFolders.length; i++)
		{
			File currFolder = new File(searchFolders[i]);
			File[] filteredFiles = currFolder.listFiles(new FileFilter()
			{
				@Override
				public boolean accept(File file)
				{
					return file.canExecute() && file.toString().matches(".*MFSimStatic(\\.exe)?$");
				}
			});
			if (filteredFiles != null)
				for (int j=0; j < filteredFiles.length; j++)
					cbBins.insertItemAt(filteredFiles[j].toString(), cbBins.getItemCount());

		}


		if (cbBins.getItemCount() > 0)
			cbBins.setSelectedIndex(0);
		else
			cbBins.insertItemAt("No Binaries Found", 0);
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Copies the source file into the destination file location
	//////////////////////////////////////////////////////////////////////////////////////
	public static void copyFile(File sourceFile, File destFile) throws IOException {
		if(!destFile.exists()) {
			destFile.createNewFile();
		}

		FileChannel source = null;
		FileChannel destination = null;

		try {
			source = new FileInputStream(sourceFile).getChannel();
			destination = new FileOutputStream(destFile).getChannel();
			destination.transferFrom(source, 0, source.size());
		}
		finally {
			if(source != null) {
				source.close();
			}
			if(destination != null) {
				destination.close();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Given a properly formatted sub-string from the executable usage string, returns
	// the command line parameter key-string
	//////////////////////////////////////////////////////////////////////////////////////
	private String getParameter(String selection)
	{
		String key = "";

		if (selection.indexOf("--") > 0)
		{
			key = selection.substring(0, selection.indexOf("--"));
			key = key.replaceAll("\"", "");
			key = key.trim();
		}
		else
			MFError.DisplayError("The following line is not formatted properly to parse a parameter:\n" + selection);

		return key;
	}


	//////////////////////////////////////////////////////////////////////////////////////
	// Just changes the look of the button to show as synthesis (as opposed to a stop
	// button)
	//////////////////////////////////////////////////////////////////////////////////////
	public void setButtonAsSynthesize(boolean isReadyToSyn)
	{
		if (isReadyToSyn)
		{
			btnSynthesize.setText("Synthesize");
			btnSynthesize.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/play_16.png")));
		}
		else
		{
			btnSynthesize.setText("Stop");
			btnSynthesize.setIcon(new ImageIcon(Main.class.getResource("/mfSimStaticGUI/resources/stop_16.png")));
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// createEvents()
	// All event handlers are found here.
	//////////////////////////////////////////////////////////////////////////////////////
	private void createEvents()
	{
		/////////////////////////////////////////////////////////
		// Behavior for when the synthesis box selection is changed
		/////////////////////////////////////////////////////////
		cbSynthesis.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if (cbSynthesis.getSelectedItem().toString() == "Entire Flow")
				{
					tpSynthesis.setEnabledAt(1, true); // Scheduling tab
					tpSynthesis.setEnabledAt(2, true); // Placement tab
					tpSynthesis.setEnabledAt(3, true); // Routing tab
					tpSynthesis.setEnabledAt(4, true); // Wire-Route tab
					tpSynthesis.setEnabledAt(5, true); // Execution tab

					cbScheduler.setEnabled(true);
					cbPlacer.setEnabled(true);
					cbRouter.setEnabled(true);
					cbPinMapping.setEnabled(true);
					cbCompType.setEnabled(true);
					cbProcEngType.setEnabled(true);
					cbExType.setEnabled(true);
					cbResAlloc.setEnabled(true);

					cbAssay.setEnabled(true);
					cbDmfbArch.setEnabled(true);
					txtSchedOut.setEnabled(false);
					txtPlaceIn.setEnabled(false);
					txtPlaceOut.setEnabled(false);
					txtRouteIn.setEnabled(false);
					txtWireRouteIn.setEnabled(false);

					spnStorageDrops.setEnabled(true);
					spnModSpace.setEnabled(true);
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Schedule")
				{
					tpSynthesis.setEnabledAt(1, true); // Scheduling tab
					tpSynthesis.setEnabledAt(2, false); // Placement tab
					tpSynthesis.setEnabledAt(3, false); // Routing tab
					tpSynthesis.setEnabledAt(4, false); // Wire-Route tab
					tpSynthesis.setEnabledAt(5, true); // Execution tab
					
					cbScheduler.setEnabled(true);
					cbPlacer.setEnabled(false);
					cbRouter.setEnabled(false);
					cbPinMapping.setEnabled(true);
					cbCompType.setEnabled(false);
					cbProcEngType.setEnabled(false);
					cbExType.setEnabled(false);
					cbResAlloc.setEnabled(true);

					cbAssay.setEnabled(true);
					cbDmfbArch.setEnabled(true);
					txtSchedOut.setEnabled(true);
					txtPlaceIn.setEnabled(false);
					txtPlaceOut.setEnabled(false);
					txtRouteIn.setEnabled(false);
					txtWireRouteIn.setEnabled(false);

					spnStorageDrops.setEnabled(true);
					spnModSpace.setEnabled(false);
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Place")
				{
					tpSynthesis.setEnabledAt(1, false); // Scheduling tab
					tpSynthesis.setEnabledAt(2, true); // Placement tab
					tpSynthesis.setEnabledAt(3, false); // Routing tab
					tpSynthesis.setEnabledAt(4, false); // Wire-Route tab
					tpSynthesis.setEnabledAt(5, true); // Execution tab
					
					cbScheduler.setEnabled(false);
					cbPlacer.setEnabled(true);
					cbRouter.setEnabled(false);
					cbPinMapping.setEnabled(true);
					cbCompType.setEnabled(false);
					cbProcEngType.setEnabled(false);
					cbExType.setEnabled(false);
					cbResAlloc.setEnabled(false);

					cbAssay.setEnabled(false);
					cbDmfbArch.setEnabled(false);
					txtSchedOut.setEnabled(false);
					txtPlaceIn.setEnabled(true);
					txtPlaceOut.setEnabled(true);
					txtRouteIn.setEnabled(false);
					txtWireRouteIn.setEnabled(false);

					spnStorageDrops.setEnabled(true);
					spnModSpace.setEnabled(true);
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Route")
				{
					tpSynthesis.setEnabledAt(1, false); // Scheduling tab
					tpSynthesis.setEnabledAt(2, false); // Placement tab
					tpSynthesis.setEnabledAt(3, true); // Routing tab
					tpSynthesis.setEnabledAt(4, false); // Wire-Route tab
					tpSynthesis.setEnabledAt(5, true); // Execution tab
					
					cbScheduler.setEnabled(false);
					cbPlacer.setEnabled(false);
					cbRouter.setEnabled(true);
					cbPinMapping.setEnabled(true);
					cbCompType.setEnabled(true);
					cbProcEngType.setEnabled(true);
					cbExType.setEnabled(true);
					cbResAlloc.setEnabled(false);

					cbAssay.setEnabled(false);
					cbDmfbArch.setEnabled(false);
					txtSchedOut.setEnabled(false);
					txtPlaceIn.setEnabled(false);
					txtPlaceOut.setEnabled(false);
					txtRouteIn.setEnabled(true);
					txtWireRouteIn.setEnabled(false);

					spnStorageDrops.setEnabled(false);
					spnModSpace.setEnabled(true);
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Wire Route")
				{
					tpSynthesis.setEnabledAt(1, false); // Scheduling tab
					tpSynthesis.setEnabledAt(2, false); // Placement tab
					tpSynthesis.setEnabledAt(3, false); // Routing tab
					tpSynthesis.setEnabledAt(4, true); // Wire-Route tab
					tpSynthesis.setEnabledAt(5, true); // Execution tab
					
					cbScheduler.setEnabled(false);
					cbPlacer.setEnabled(false);
					cbRouter.setEnabled(true);
					cbPinMapping.setEnabled(true);
					cbCompType.setEnabled(true);
					cbProcEngType.setEnabled(true);
					cbExType.setEnabled(true);
					cbResAlloc.setEnabled(false);

					cbAssay.setEnabled(false);
					cbDmfbArch.setEnabled(false);
					txtSchedOut.setEnabled(false);
					txtPlaceIn.setEnabled(false);
					txtPlaceOut.setEnabled(false);
					txtRouteIn.setEnabled(true);
					txtWireRouteIn.setEnabled(true);
					

					spnStorageDrops.setEnabled(false);
					spnModSpace.setEnabled(false);
				}
			}
		});

		/////////////////////////////////////////////////////////
		// Calls the external binary for execution
		/////////////////////////////////////////////////////////
		btnSynthesize.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {

				String cmd = "";
				if (cbSynthesis.getSelectedItem().toString() == "Entire Flow")
				{
					cmd = progFileName + " ef ";
					cmd = cmd.concat(getParameter(cbScheduler.getSelectedItem().toString()) + " "); // Scheduler
					cmd = cmd.concat(getParameter(cbPlacer.getSelectedItem().toString()) + " "); // Placer
					cmd = cmd.concat(getParameter(cbRouter.getSelectedItem().toString()) + " "); // Router
					if (btnWashYes.isSelected())
						cmd = cmd.concat("TRUE ");
					else
						cmd = cmd.concat("FALSE ");
					cmd = cmd.concat(getParameter(cbResAlloc.getSelectedItem().toString()) + " "); // Resource Allocation
					cmd = cmd.concat(getParameter(cbPinMapping.getSelectedItem().toString()) + " "); // PinMapper
					cmd = cmd.concat(getParameter(cbWireRouteType.getSelectedItem().toString()) + " "); // WireRouting
					cmd = cmd.concat(getParameter(cbCompType.getSelectedItem().toString()) + " "); // Compaction Type
					cmd = cmd.concat(getParameter(cbProcEngType.getSelectedItem().toString()) + " "); // Processing Engine Type
					cmd = cmd.concat(getParameter(cbExType.getSelectedItem().toString()) + " "); // Execution Type
					cmd = cmd.concat(cbAssay.getSelectedItem().toString() + " "); // Assay
					cmd = cmd.concat(cbDmfbArch.getSelectedItem().toString() + " "); // DMFB Architecture
					cmd = cmd.concat(spnStorageDrops.getValue().toString() + " "); // Number droplets per storage module
					cmd = cmd.concat(spnModSpace.getValue().toString() + " "); // Number cells between module IRs
					cmd = cmd.concat(spnHorizTracks.getValue().toString() + " "); // Number horizontal tracks under each cell
					cmd = cmd.concat(spnVertTracks.getValue().toString() + " "); // Number vertical tracks under each cell

				}
				else if (cbSynthesis.getSelectedItem().toString() == "Schedule")
				{
					cmd = progFileName + " s ";
					cmd = cmd.concat(getParameter(cbScheduler.getSelectedItem().toString()) + " "); // Scheduler
					cmd = cmd.concat(getParameter(cbResAlloc.getSelectedItem().toString()) + " "); // Resource Allocation
					cmd = cmd.concat(getParameter(cbPinMapping.getSelectedItem().toString()) + " "); // PinMapper
					cmd = cmd.concat(cbAssay.getSelectedItem().toString() + " "); // Assay
					cmd = cmd.concat(cbDmfbArch.getSelectedItem().toString() + " "); // DMFB Architecture
					cmd = cmd.concat(txtSchedOut.getText() + " "); // Scheduler Output
					cmd = cmd.concat(spnStorageDrops.getValue().toString() + " "); // Number droplets per storage module
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Place")
				{
					cmd = progFileName + " p ";
					cmd = cmd.concat(getParameter(cbPlacer.getSelectedItem().toString()) + " "); // Placer
					cmd = cmd.concat(txtPlaceIn.getText() + " "); // Placer Input
					cmd = cmd.concat(txtPlaceOut.getText() + " "); // Placer Output
					cmd = cmd.concat(spnModSpace.getValue().toString() + " "); // Number cells between module IRs
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Route")
				{
					cmd = progFileName + " r ";
					cmd = cmd.concat(getParameter(cbRouter.getSelectedItem().toString()) + " "); // Router
					if (btnWashYes.isSelected())
						cmd = cmd.concat("TRUE ");
					else
						cmd = cmd.concat("FALSE ");
					cmd = cmd.concat(getParameter(cbCompType.getSelectedItem().toString()) + " "); // Compaction Type
					cmd = cmd.concat(getParameter(cbProcEngType.getSelectedItem().toString()) + " "); // Processing Engine Type
					cmd = cmd.concat(getParameter(cbExType.getSelectedItem().toString()) + " "); // Execution Type
					cmd = cmd.concat(txtRouteIn.getText() + " "); // Router Input
				}
				else if (cbSynthesis.getSelectedItem().toString() == "Wire Route")
				{
					cmd = progFileName + " wr ";
					cmd = cmd.concat(getParameter(cbWireRouteType.getSelectedItem().toString()) + " "); // Wire Router
					cmd = cmd.concat(txtWireRouteIn.getText() + " "); // Wire Router Input
					cmd = cmd.concat(spnHorizTracks.getValue().toString() + " "); // Number horizontal tracks under each cell
					cmd = cmd.concat(spnVertTracks.getValue().toString() + " "); // Number vertical tracks under each cell
				}



				if (!isSimulating)
				{
					isSimulating = true;

					Thread thread1 = new Thread(new SimulationThread("simThread", mainPtr, cmd), "thread");
					thread1.start();
					setButtonAsSynthesize(false);
				}
				else
				{
					isSimulating = false;
					setButtonAsSynthesize(true);
					if (simProcess != null)
						simProcess.destroy();
					txtOutput.setText(txtOutput.getText() + "\n\n***SIMULATION HALTED BY USER***");
				}
			}
		});


		/////////////////////////////////////////////////////////
		// Runs the selected binary to get the parameters options
		/////////////////////////////////////////////////////////
		btnSelectBinary.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {

				// Clear all the combo-boxes
				cbScheduler.removeAllItems();
				cbPlacer.removeAllItems();
				cbRouter.removeAllItems();
				cbPinMapping.removeAllItems();
				cbCompType.removeAllItems();
				cbProcEngType.removeAllItems();
				cbExType.removeAllItems();			       
				cbAssay.removeAllItems();
				cbDmfbArch.removeAllItems();
				cbResAlloc.removeAllItems();
				cbWireRouteType.removeAllItems();

				File f = new File((String)cbBins.getSelectedItem());
				if (!f.exists())
				{
					MFError.DisplayError("The binary located at " + (String)cbBins.getSelectedItem() + " does not exist!");
					return;
				}

				File nf = new File(progFileName);
				try { copyFile(f,nf); } catch (IOException e1) { MFError.DisplayError(e1.getMessage()); }


				String stdOut = "";
				String stdErr = "";				
				try {
					String line;
					Process p = Runtime.getRuntime().exec( progFileName );

					BufferedReader in = new BufferedReader(new InputStreamReader(p.getInputStream()));
					String state = "none";
					while ((line = in.readLine()) != null)
					{
						line = line.trim();
						if (line.startsWith("[") && line.endsWith("]"))
							state = line;
						else if (line.startsWith("\"") && line.contains("--"))
						{	
							// Contains a parameter option - Add to options boxes
							if (state.contains("[schedulerType]"))
								cbScheduler.insertItemAt(line, cbScheduler.getItemCount());
							else if (state.contains("[placementType]"))
								cbPlacer.insertItemAt(line, cbPlacer.getItemCount());
							else if (state.contains("[routerType]"))
								cbRouter.insertItemAt(line, cbRouter.getItemCount());
							else if (state.contains("[pinMapType]"))
								cbPinMapping.insertItemAt(line, cbPinMapping.getItemCount());
							else if (state.contains("[wireRouteType]"))
								cbWireRouteType.insertItemAt(line, cbWireRouteType.getItemCount());
							else if (state.contains("[compactionType]"))
								cbCompType.insertItemAt(line, cbCompType.getItemCount());
							else if (state.contains("[processingEngineType]"))
								cbProcEngType.insertItemAt(line, cbProcEngType.getItemCount());
							else if (state.contains("[executionType]"))
								cbExType.insertItemAt(line, cbExType.getItemCount());
							else if (state.contains("[resourceAllocationType]"))
								cbResAlloc.insertItemAt(line, cbResAlloc.getItemCount());
						}
						else
							state = "none";
					}

					in.close();

					// Enable/disable appropriate controls
					cbSynthesis.setEnabled(true);
					cbScheduler.setEnabled(true);
					cbPlacer.setEnabled(true);
					cbRouter.setEnabled(true);
					cbPinMapping.setEnabled(true);
					cbWireRouteType.setEnabled(true);
					cbCompType.setEnabled(true);
					cbProcEngType.setEnabled(true);
					cbExType.setEnabled(true);			       
					cbAssay.setEnabled(true);
					cbDmfbArch.setEnabled(true);
					cbResAlloc.setEnabled(true);
					txtSchedOut.setEnabled(false);
					txtPlaceIn.setEnabled(false);
					txtPlaceOut.setEnabled(false);
					txtRouteIn.setEnabled(false);
					btnSynthesize.setEnabled(true);
					spnStorageDrops.setEnabled(true);
					spnModSpace.setEnabled(true);
					
					// Enable all tabs
					for (int i = 0; i < tpSynthesis.getTabCount(); i++)
						tpSynthesis.setEnabledAt(i, true);

					// Get Assay files and DMFB Archs up to 3 levels
					String assayDir = "Assays";
					File da = new File(assayDir);
					File[] afiles = da.listFiles();
					for (int i = 0; i < afiles.length; i++)
					{
						if (afiles[i].isFile())
							cbAssay.insertItemAt(afiles[i].getPath(), cbAssay.getItemCount());
						else if (afiles[i].isDirectory() && !afiles[i].isHidden())
						{
							File lev2Dir = afiles[i];
							for (int j = 0; j < lev2Dir.listFiles().length; j++)
							{
								if (lev2Dir.listFiles()[j].isFile())
									cbAssay.insertItemAt(lev2Dir.listFiles()[j].getPath(), cbAssay.getItemCount());
								else if (lev2Dir.listFiles()[j].isDirectory() && !lev2Dir.listFiles()[j].isHidden())
								{
									File lev3Dir = lev2Dir.listFiles()[j];
									for (int k = 0; k < lev3Dir.listFiles().length; k++)
										if (lev3Dir.listFiles()[k].isFile())
											cbAssay.insertItemAt(lev3Dir.listFiles()[k].getPath(), cbAssay.getItemCount());
								}
							}
						}
					}

					String dmfbDir = "DmfbArchs";
					File dd = new File(dmfbDir);
					File[] dfiles = dd.listFiles();
					for (int i = 0; i < dfiles.length; i++)
					{
						if (dfiles[i].isFile())
							cbDmfbArch.insertItemAt(dfiles[i].getPath(), cbDmfbArch.getItemCount());
						else if (dfiles[i].isDirectory() && !dfiles[i].isHidden())
						{
							File lev2Dir = dfiles[i];
							for (int j = 0; j < lev2Dir.listFiles().length; j++)
							{
								if (lev2Dir.listFiles()[j].isFile())
									cbDmfbArch.insertItemAt(lev2Dir.listFiles()[j].getPath(), cbDmfbArch.getItemCount());
								else if (lev2Dir.listFiles()[j].isDirectory() && !lev2Dir.listFiles()[j].isHidden())
								{
									File lev3Dir = lev2Dir.listFiles()[j];
									for (int k = 0; k < lev3Dir.listFiles().length; k++)
										if (lev3Dir.listFiles()[k].isFile())
											cbDmfbArch.insertItemAt(lev3Dir.listFiles()[k].getPath(), cbDmfbArch.getItemCount());
								}
							}
						}
					}
					

					// Initialize selections
					cbScheduler.insertItemAt("SELECT SCHEDULER", 0);
					cbPlacer.insertItemAt("SELECT PLACER", 0);
					cbRouter.insertItemAt("SELECT ROUTER", 0);
					cbPinMapping.insertItemAt("SELECT PIN-MAPPER", 0);
					cbWireRouteType.insertItemAt("SELECT WIRE-ROUTER", 0);
					cbCompType.insertItemAt("SELECT COMPACTION TYPE", 0);
					cbProcEngType.insertItemAt("SELECT PROCESSING ENGINE TYPE", 0);
					cbExType.insertItemAt("SELECT EXECUTION TYPE", 0);
					cbAssay.insertItemAt("SELECT ASSAY", 0);
					cbDmfbArch.insertItemAt("SELECT DMFB ARCHITECTURE", 0);
					cbResAlloc.insertItemAt("SELECT RESOURCE ALLOCATION TYPE", 0);
					cbScheduler.setSelectedIndex(0);
					cbPlacer.setSelectedIndex(0);
					cbRouter.setSelectedIndex(0);
					cbPinMapping.setSelectedIndex(0);
					cbWireRouteType.setSelectedIndex(0);
					cbCompType.setSelectedIndex(0);
					cbProcEngType.setSelectedIndex(0);
					cbExType.setSelectedIndex(cbExType.getItemCount()-1);
					cbAssay.setSelectedIndex(0);
					cbDmfbArch.setSelectedIndex(0);
					cbResAlloc.setSelectedIndex(0);
					txtSchedOut.setText("Output" + del + "1_SCHED_to_PLACE.txt");
					txtPlaceIn.setText("Output" + del + "1_SCHED_to_PLACE.txt");
					txtPlaceOut.setText("Output" + del + "2_PLACE_to_ROUTE.txt");
					txtRouteIn.setText("Output" + del + "2_PLACE_to_ROUTE.txt");
					txtWireRouteIn.setText("Output" + del + "3_ROUTE_to_SIM.txt");

				}
				catch (Exception e1) { MFError.DisplayError(e1.getMessage()); }

			}
		});
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// getters/setters
	//////////////////////////////////////////////////////////////////////////////////////
	public boolean isSimulating() {
		return isSimulating;
	}
	public void setSimulating(boolean isSim) {
		this.isSimulating = isSim;
	}

	public Process getSimProcess() {
		return simProcess;
	}
	public void setSimProcess(Process simProcess) {
		this.simProcess = simProcess;
	}
}
