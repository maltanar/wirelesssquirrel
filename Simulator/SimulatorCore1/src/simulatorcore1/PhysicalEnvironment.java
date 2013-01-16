package simulatorcore1;

import java.util.HashMap;
import java.util.Iterator;

/**
 *
 * @author maltanar
 */
public class PhysicalEnvironment implements SimulationItem, RadioInterface
{
    private HashMap<Integer, SensorNode> m_sensorNodes;
    private int m_width, m_height;
    private int m_maxNetworkSize;
    
    public PhysicalEnvironment(int width, int height, int maxNetworkSize)
    {
        m_sensorNodes = new HashMap<Integer, SensorNode>();
        m_width = width;
        m_height = height;
        m_maxNetworkSize = maxNetworkSize;
    }
    
    public SensorNode createNode()
    {
        if(getNodeCount() >= m_maxNetworkSize)
        {
            System.out.printf("PhysicalEnvironment: Allowed node count exceeded!"
                               + " (%d) \n", m_maxNetworkSize);
            return null;
        }
        // new node ID is next available integer ID
        Integer newNodeID = m_sensorNodes.size() + 1;
        
        // create default config for new sensors
        // TODO make this customizable
        SensorConfig cfg = new SensorConfig();
        cfg.arbitrationMode = SensorConfig.ArbitrationMode.ARBITRATE_ODDEVEN;
        cfg.cycleCount = 3;
        cfg.sleepDurationS = 1;
        cfg.rxDurationMs = 1000;
        cfg.txDurationMs = 1000;
        cfg.txPowerdBm = -0.4;         // suggested value from CC2430 datasheet
        cfg.rxSensitivitydBm = -92;    // suggested value from CC2430 datasheet
        
        // create a new node with randomized location 
        SensorNode newNode = new SensorNode(newNodeID, 
                                            generateRandomPosition(),
                                            cfg, this, getMaxNetworkSize());
        // add the new node to our list
        m_sensorNodes.put(newNodeID, newNode);
        
        return newNode;
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

    public int getMaxNetworkSize() {
        return m_maxNetworkSize;
    }

    public void setMaxNetworkSize(int m_maxNetworkSize) {
        this.m_maxNetworkSize = m_maxNetworkSize;
    }
    
    public void propagateRadioWaves(String message, SensorNode origin, 
                                   double txPower)
    {
        // TODO how should we use txPower to make the calculation more
        // realistic?
        // TODO use precomputed node distance table here
        Iterator<SensorNode> nodes = m_sensorNodes.values().iterator();
        double rxPower = 0;
        double wavelength = (3*10^8 / (2445*10^6));
        int distance = 0;
        while(nodes.hasNext())
        {
            SensorNode node = nodes.next();
            
            // only attempt to propagate broadcast to other nodes
            if(node == origin)
                continue;
            
            // calculate distance in meters
            distance = calculateDistance(origin, node);
            // use Friis equation to calculate rx power on the destination node
            // antenna gains assumed to be 0 dB
            rxPower = txPower + 20 * Math.log10(wavelength 
                                                / (4 * Math.PI * distance));
            // expose target node to radio waves
            node.receiveRadioWaves(message, rxPower);
        }
    }
    
    @Override
    public void timePassed(double passedTimeMs) {
        System.out.println("PhysicalEnvironment::timePassed not implemented yet");
    }
}
