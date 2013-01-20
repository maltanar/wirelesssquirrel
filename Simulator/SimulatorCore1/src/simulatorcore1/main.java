/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.util.Scanner;

/**
 *
 * @author maltanar
 */
public class main {
    public static void main(String[] args) 
    {
        // enter the number of sensor to be simulated 
        System.out.println("Please enter the max number of nodes to be simulated: ");
        System.out.println("note: only integer valid");
        
        Scanner input = new Scanner(System.in);
        
        int N = input.nextInt(); // max number of nodes allowed in network
        
        PhysicalEnvironment environment = new PhysicalEnvironment(500, 500, N);
        SimulatorCore simulator = new SimulatorCore();
        
        EnvironmentDisplay dp = new EnvironmentDisplay(environment);
        dp.setVisible(true);
        
        ControlPanel cp = new ControlPanel(simulator, environment, dp);
        cp.setVisible(true);
        
    }
}
