package simulatorcore1;

import java.util.HashMap;
import java.util.Iterator;

/**
 *
 * @author maltanar
 */
public class PhysicalEnvironment implements SimulationItem, RadioInterface {

    private HashMap<Integer, SensorNode> m_sensorNodes;
    private int m_width, m_height;
    private int m_maxNetworkSize;
    private SensorConfig m_sensorConfig;

    public PhysicalEnvironment(int width, int height, int maxNetworkSize) {
        m_sensorNodes = new HashMap<Integer, SensorNode>();
        m_width = width;
        m_height = height;
        m_maxNetworkSize = maxNetworkSize;
        m_sensorConfig = new SensorConfig();
        // default config for sensors
        // algorithm parameters
        m_sensorConfig.arbitrationMode = SensorConfig.ArbitrationMode.ARBITRATE_ODDEVEN;
        m_sensorConfig.cycleCount = 4;
        m_sensorConfig.sleepDurationS = 1;
        m_sensorConfig.rxDurationMs = 1000;
        m_sensorConfig.txDurationMs = 1000;
        // device parameters - suggested values from CC2430 datasheet
        m_sensorConfig.txPowerdBm = -0.4;
        m_sensorConfig.rxSensitivitydBm = -92;
    }
    
    public int getWidth() {
        return m_width;
    }
    
    public int getHeight() {
        return m_height;
    }

    public SensorNode createNode() {
        if (getNodeCount() >= m_maxNetworkSize) {
            System.out.printf("PhysicalEnvironment: Allowed node count exceeded!"
                    + " (%d) \n", m_maxNetworkSize);
            return null;
        }
        // new node ID is next available integer ID
        Integer newNodeID = m_sensorNodes.size() + 1;

        // create a new node with randomized location 
        SensorNode newNode = new SensorNode(newNodeID,
                generateRandomPosition(),
                getSensorConfig(), this, getMaxNetworkSize());
        // add the new node to our list
        m_sensorNodes.put(newNodeID, newNode);

        return newNode;
    }
    
    public void removeAllNodes() {
        m_sensorNodes.clear();
    }

    public SensorConfig getSensorConfig() {
        return m_sensorConfig;
    }

    public void setSensorConfig(SensorConfig m_sensorConfig) {
        this.m_sensorConfig = m_sensorConfig;
        
        // propagate config change to all sensors
        Iterator<SensorNode> nodes = m_sensorNodes.values().iterator();

        while (nodes.hasNext()) {
            nodes.next().setNodeConfig(m_sensorConfig);
        }
    }

    // return number of present nodes in the environment
    public int getNodeCount() {
        return m_sensorNodes.size();
    }

    // return a node given its unique ID
    public SensorNode getNode(Integer nodeID) {
        return m_sensorNodes.get(nodeID);
    }

    // calculate the distance given two node IDs
    public double calculateDistance(Integer nodeID1, Integer nodeID2) {
        return calculateDistance(getNode(nodeID1), getNode(nodeID2));
    }

    // calculate the distance given two nodes
    public double calculateDistance(SensorNode n1, SensorNode n2) {
        return calculateDistance(n1.getPosition(), n2.getPosition());
    }

    // calculate the distance given two positions
    public double calculateDistance(SensorPosition p1, SensorPosition p2) {
        // we have a simple Cartesian distance formula for the moment
        double dx = Math.abs(p1.x - p2.x);
        double dy = Math.abs(p1.y - p2.y);

        return Math.sqrt((dx * dx) + (dy * dy));
    }

    // generate a random position within allowed size
    private SensorPosition generateRandomPosition() {
        SensorPosition randomPos = new SensorPosition();
        randomPos.x = generateRandomNumber(0, m_width);
        randomPos.y = generateRandomNumber(0, m_height);

        return randomPos;
    }

    // return a pseudo-random number in the given range
    private int generateRandomNumber(int min, int max) {
        return min + (int) (Math.random() * ((max - min) + 1));
    }

    public int getMaxNetworkSize() {
        return m_maxNetworkSize;
    }

    public void setMaxNetworkSize(int m_maxNetworkSize) {
        this.m_maxNetworkSize = m_maxNetworkSize;
    }

    // special: propagate a field of all-0 values to all nodes to get the
    // network started. this functionality will be handled by the readout
    // device in the physical implementation
    public void makeSyncBroadcast() {
        Iterator<SensorNode> nodes = m_sensorNodes.values().iterator();
        // create an all-zeroes PresenceData field for use in sync
        PresenceData syncData = new PresenceData(m_maxNetworkSize);
        syncData.resetAll();

        while (nodes.hasNext()) {
            SensorNode node = nodes.next();
            node.receiveRadioWaves(syncData.toString(), 0);
        }
    }

    @Override
    public void propagateRadioWaves(String message, SensorNode origin,
            double txPower) {
        // TODO could use precomputed node distance table here
        Iterator<SensorNode> nodes = m_sensorNodes.values().iterator();
        double distance = 0;
        double rxPower = 0;
        // wavelength = speed of light / 2.4 GHz
        double wavelength = (300.0 / 2445.0);   // 3*10^8 / 2445 * 10^6

        while (nodes.hasNext()) {
            SensorNode node = nodes.next();

            // only attempt to propagate broadcast to other nodes
            if (node.getNodeID() != origin.getNodeID()) {
                // calculate distance in meters
                distance = calculateDistance(origin, node);
                // use Friis equation to calculate rx power on the destination node
                // antenna gains assumed to be 0 dB
                rxPower = txPower + 20 * Math.log10(wavelength
                        / (4 * Math.PI * distance));
                System.out.printf("Distance btw %d and %d is %f, rxp %f \n",
                                origin.getNodeID(), node.getNodeID(),
                                distance, rxPower);
                // expose target node to radio waves
                node.receiveRadioWaves(message, rxPower);
            }
        }
    }
    
    public double getMaxRange(double Pt, double Pr) {
        double wavelength = (300.0 / 2445.0);   // 3*10^8 / 2445 * 10^6
        // calculate max transmission range given tx power and rx sensitivity
        // using the Friis equation
        // zero-dBm antenna gains assumed
        double max_range = wavelength / (4.0 * Math.PI * 
                            Math.pow(10, (Pr - Pt) / 20.0));
        
        return max_range;
    }
    
    public double getMaxRange()
    {
        return getMaxRange(m_sensorConfig.txPowerdBm, 
                           m_sensorConfig.rxSensitivitydBm);
    }

    @Override
    public void timePassed(double passedTimeMs) {
        System.out.println("PhysicalEnvironment::timePassed not implemented yet");
    }
}
