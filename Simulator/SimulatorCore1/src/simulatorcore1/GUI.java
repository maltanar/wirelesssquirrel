/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.Font;
//import java.awt.List;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

/**
 *
 * @author GeorgeSong
 */
public class GUI extends JFrame {

    private JButton createBtn;
    private JButton clearBtn;
    private JButton runBtn;
    private SensorDisplay displayPanel;
    private JTextField size;
    private JLabel lblNumber;
    private List<SensorNode> nodeList = new LinkedList<SensorNode>();

    public GUI() {
        // SET WINDOW PROPERTIES
        this.setSize(830, 720);
        this.setTitle("Simulator");
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setVisible(true);
        getContentPane().setLayout(null);
        
        // Initialise the physical environment
        

        // add sensor display panel
        displayPanel = new SensorDisplay(1, 1, 500, 500);
        getContentPane().add(displayPanel);

        size = new JTextField();
        size.setFont(new Font("Tahoma", Font.PLAIN, 14));
        size.setHorizontalAlignment(SwingConstants.RIGHT);
        size.setSize(58, 30);
        size.setEditable(true);
        size.setLocation(510, 80);
        getContentPane().add(size);

        // add control components: Show sensor button
        createBtn = new JButton("Create");
        createBtn.setSize(100, 30);
        createBtn.setLocation(600, 80);
        createBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                // call run method
                createSensors();
            }
        });
        getContentPane().add(createBtn);
        
        // add label: enter the number of sensor nodes
        lblNumber = new JLabel("Enter the number of nodes simulated");
        lblNumber.setFont(new Font("Serif", Font.ITALIC, 14));
        lblNumber.setBounds(510, 50, 300, 30);
        getContentPane().add(lblNumber);

        // add control components: Clear screen button
        clearBtn = new JButton("Clear");
        clearBtn.setSize(100, 30);
        clearBtn.setLocation(600, 115);
        clearBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                clearScreen();
            }
        });
        getContentPane().add(clearBtn);
        
        // add control components: run simulation button
        runBtn  = new JButton("Run");
        runBtn.setLocation(600, 150);
        runBtn.setSize(100, 30);
        runBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                runAndUpdate();
            }
            
        });
        getContentPane().add(runBtn);
    }

    // Method showSensors: show the simulated sensors on the screen based on the value user entered
    public void createSensors() {
        //clearScreen();
        // get number of nodes entered by the user
        
        PhysicalEnvironment environment = new PhysicalEnvironment(100, 100, 4);
        SimulatorCore simulator = new SimulatorCore();
        
        // enter the number of sensor to be simulated 
        //System.out.println("Please enter the number of nodes to be simulated: ");
        //System.out.println("note: only integer valid");
        
        //Scanner input = new Scanner(System.in);
        
        //int N = input.nextInt(); // number of nodes to be simulated
        
        // Create N nodes according to the number user entered and add them
        // to the simulation
        
        int nodesNumber = Integer.parseInt(size.getText());
        
        for (int i =1; i <= nodesNumber; i++)
        {
            SensorNode newNode = environment.createNode();
            if(newNode != null)
                simulator.addSimulationItem(Integer.toString(newNode.getNodeID()),
                                        newNode);
        }
        
        simulator.passTime(500);
        
        SensorNode coordNode = environment.createNode();

        // simulate the sync message
        environment.propagateRadioWaves("0000", coordNode, -0.4);
        
        
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        simulator.passTime(500);
        
                
        
        //environment.createNode();
        //environment.createNode();
        
        System.out.println("Node count: "
                + Integer.toString(environment.getNodeCount()));

        // Create N nodes according to the number user entered
        for (int i = 1; i <= nodesNumber; i++) {
            nodeList.add(environment.createNode());
        }

        // populate the nodeList for later call
        //for (int i = 1; i <= nodesNumber; i++) {
         //   nodeList.add(environment.getNode(i));
        //}

        displayPanel.paintSensor(nodeList);

        // Calculate the distance between each node
        int[][] distance_array = new int[nodesNumber][nodesNumber];
        for (int i = 0; i < distance_array.length; i++) {
            for (int j = 0; j < distance_array[i].length; j++) {
                distance_array[i][j] = environment.calculateDistance(i + 1, j + 1);
            }
        }
        //Integer d1 = environment.calculateDistance(1, 2);
        //System.out.printf("Distances: %d %d %d \n", d1, d2, d3);

        for (int i = 0; i < distance_array.length; i++) {
            for (int j = 0; j < distance_array[i].length; j++) {
                System.out.println("Distance: " + "(" + Integer.toString(i) + " , " + Integer.toString(j) + ") " + distance_array[i][j]);
            }
        }

    }

    public void clearScreen() {
        // clear the sensors and textfield for new input
        nodeList.clear();
        if(getContentPane().getComponentAt(1, 1) instanceof SensorDisplay){
            getContentPane().remove(getContentPane().getComponentAt(1, 1));
        }
        else{
            System.err.println("Error! Please close the program and try again");
        }
        
        displayPanel = new SensorDisplay(1,1, 500, 500);
        getContentPane().add(displayPanel);
        this.repaint();
    }
    
    public void runAndUpdate(){
        // run the simulation and update the bit field
        
    }
}
