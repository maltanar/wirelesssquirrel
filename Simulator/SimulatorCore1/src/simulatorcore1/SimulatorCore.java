/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;


/**
 *
 * @author maltanar
 */
public class SimulatorCore 
{

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) 
    {
        PhysicalEnvironment environment = new PhysicalEnvironment(100, 100);
        environment.createNode();
        environment.createNode();
        environment.createNode();
        
        System.out.println("Node count: " + 
                            Integer.toString(environment.getNodeCount()));
        
        Integer d1 = environment.calculateDistance(1, 2);
        Integer d2 = environment.calculateDistance(1, 3);
        Integer d3 = environment.calculateDistance(2, 3);
        
        System.out.printf("Distances: %d %d %d \n", d1, d2, d3);
        
    }
    
}
