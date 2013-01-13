/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.util.HashMap;

/**
 *
 * @author maltanar
 */
public class PhysicalEnvironment implements SimulationItem
{
    private HashMap<Integer, SensorNode> m_sensorNodes;
    private int m_width, m_height;
    
    public PhysicalEnvironment(int width, int height)
    {
        m_sensorNodes = new HashMap<Integer, SensorNode>();
        m_width = width;
        m_height = height;
    }
    
    public int createNode()
    {
        // new node ID is next available integer ID
        Integer newNodeID = m_sensorNodes.size() + 1;
        
        // create default config for new sensors
        // TODO make this customizable
        SensorConfig cfg = new SensorConfig();
        cfg.arbitrationMode = SensorConfig.ArbitrationMode.ARBITRATE_ODDEVEN;
        cfg.cycleCount = 3;
        cfg.rxDurationMs = 1000;
        cfg.txDurationMs = 1000;
        
        // create a new node with randomized location 
        SensorNode newNode = new SensorNode(newNodeID, 
                                            generateRandomPosition(),
                                            cfg);
        // add the new node to our list
        m_sensorNodes.put(newNodeID, newNode);
        
        return newNodeID;
    }
    
    // return number of present nodes in the environment
    public int getNodeCount()
    {
        return m_sensorNodes.size();
    }
    
    // return a node given its unique ID
    public SensorNode getNode(Integer nodeID)
    {
        return m_sensorNodes.get(nodeID);
    }
    
    // calculate the distance given two node IDs
    public int calculateDistance(Integer nodeID1, Integer nodeID2)
    {
        return calculateDistance(getNode(nodeID1), getNode(nodeID2));
    }
    
    // calculate the distance given two nodes
    public int calculateDistance(SensorNode n1, SensorNode n2)
    {
        return calculateDistance(n1.getPosition(), n2.getPosition());
    }
    
    // calculate the distance given two positions
    public int calculateDistance(SensorPosition p1, SensorPosition p2)
    {
        // we have a simple Cartesian distance formula for the moment
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        
        return (int) Math.sqrt(dx*dx + dy*dy);
    }
    
    // generate a random position within allowed size
    private SensorPosition generateRandomPosition()
    {
        SensorPosition randomPos = new SensorPosition();
        randomPos.x = generateRandomNumber(0, m_width);
        randomPos.y = generateRandomNumber(0, m_height);
        
        return randomPos;
    }
    
    // return a pseudo-random number in the given range
    private int generateRandomNumber(int min, int max)
    {
        return min + (int)(Math.random() * ((max - min) + 1));
    }

    
    @Override
    public void timePassed(double passedTimeMs) {
        System.out.println("PhysicalEnvironment::timePassed not implemented yet");
    }
}
