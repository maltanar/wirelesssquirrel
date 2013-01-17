/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Scanner;

/**
 *
 * @author maltanar
 */
public class main {
    public static void main(String[] args) 
    {
        /*GUI g = new GUI();
        g.repaint();*/
        PhysicalEnvironment environment = new PhysicalEnvironment(500, 500, 5);
        SimulatorCore simulator = new SimulatorCore();
        
        EnvironmentDisplay dp = new EnvironmentDisplay(environment);
        dp.setVisible(true);
        
        ControlPanel cp = new ControlPanel(simulator, environment, dp);
        cp.setVisible(true);
        
        
        
        
        
        
        /*PhysicalEnvironment environment = new PhysicalEnvironment(100, 100, 4);
        SimulatorCore simulator = new SimulatorCore();
        
        // enter the number of sensor to be simulated 
        System.out.println("Please enter the number of nodes to be simulated: ");
        System.out.println("note: only integer valid");
        
        Scanner input = new Scanner(System.in);
        
        int N = input.nextInt(); // number of nodes to be simulated
        
        // Create N nodes according to the number user entered and add them
        // to the simulation
        for (int i =1; i <= N; i++)
        {
            SensorNode newNode = environment.createNode();
            if(newNode != null)
                simulator.addSimulationItem(Integer.toString(newNode.getNodeID()),
                                        newNode);
        }
        
        environment.makeSyncBroadcast();
        
        for(int i=0; i < 100; i++)
           simulator.passTime(100);
        
        for (int i =1; i <= N; i++)
        {
            System.out.printf("%d : %s \n", i,
            environment.getNode(i).getPresenceData().toString());
        }
        
        //environment.createNode();
        //environment.createNode();
        
        System.out.println("Node count: " + 
                            Integer.toString(environment.getNodeCount()));
        
        // Calculate the distance between each node (party handshake problem)
        int[][] distance_array = new int[N][N];
        for(int i = 0; i < distance_array.length; i++)
        {
            for(int j = 0; j < distance_array[i].length; j++)
            {
                distance_array[i][j] = environment.calculateDistance(i+1, j+1);
            }
        }
        //Integer d1 = environment.calculateDistance(1, 2);
        //Integer d2 = environment.calculateDistance(1, 3);
        //Integer d3 = environment.calculateDistance(2, 3);
        
        //System.out.printf("Distances: %d %d %d \n", d1, d2, d3);
        
        for(int i = 0; i < distance_array.length; i++)
        {
            for(int j = 0; j < distance_array[i].length; j++)
            {
                System.out.println("Distance: " + "(" + Integer.toString(i) + " , " + Integer.toString(j) + ") "+ distance_array[i][j]);
            }
        }*/
    }
}
