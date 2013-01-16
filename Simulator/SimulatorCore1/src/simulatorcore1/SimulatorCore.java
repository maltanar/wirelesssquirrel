package simulatorcore1;
import com.sun.corba.se.spi.orbutil.proxy.LinkedInvocationHandler;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Scanner;
import java.util.Set;


/**
 *
 * @author maltanar
 */
public class SimulatorCore 
{
    protected HashMap<String, SimulationItem> m_simulationItems;
    
    public SimulatorCore()
    {
        m_simulationItems = new HashMap<String, SimulationItem>();
    }
    
    public void addSimulationItem(String name, SimulationItem newItem)
    {
        m_simulationItems.put(name, newItem);
    }
    
    public Set<String> getSimulationItemNames()
    {
        return m_simulationItems.keySet();
    }
    
    public SimulationItem getSimulationItem(String name)
    {
        return m_simulationItems.get(name);
    }
    
    public void removeSimulationItem(String name)
    {
        m_simulationItems.remove(name);
    }
    
    public void passTime(double ms)
    {
        // propagate simulation of passage of time to every simulated item
        Collection<SimulationItem> items = m_simulationItems.values();
        Iterator<SimulationItem> itr = items.iterator();
        
        while(itr.hasNext())
        {
            SimulationItem currentItem = itr.next();
            currentItem.timePassed(ms);
        }
    }

    public static void main(String[] args) 
    {
        PhysicalEnvironment environment = new PhysicalEnvironment(100, 100, 4);
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
        }
        
    }
    
}
