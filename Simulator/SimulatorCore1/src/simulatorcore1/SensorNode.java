package simulatorcore1;

// TODO add stochastic movement model to sensor nodes

public class SensorNode implements SimulationItem
{
    // member variables ******************************************************
    private Integer m_nodeID;   // unique node identifier
    private SensorPosition m_position;  // sensor node position in environment
    private SensorState m_state;    // node state
    private SensorConfig m_config;  // node config
    private SensorConfig m_newConfig;   // new node config, takes effect after
                                        // reset
    
    // internal type definitions *********************************************
    
    // possible states that a sensor node can be in
    private enum SensorState {
        STATE_INIT,         // sensor has just been powered up
        STATE_CYCLE_SLEEP,  // sensor is sleeping and waiting for the next cycle
        STATE_TRANSMIT,     // sensor is sending presence bitfield data
        STATE_RECEIVE       // sensor is listening for presence bitfield data
    }
    
    // member methods *********************************************************
    
    // constructor for the SensorNode - initialize member variables
    public SensorNode(Integer nodeID, SensorPosition position,
                      SensorConfig config)
    {
        m_nodeID = nodeID;
        m_position = position;
        m_config = m_newConfig = config;
        m_state = SensorState.STATE_INIT;
        
        System.out.println("Node with id " + nodeID + " created at " + 
                            position);
    }
    
    public void resetNode()
    {
        // TODO implement full reset functionality here
        m_config = m_newConfig;
        m_state = SensorState.STATE_INIT;
    }
    
    // note: config setting only takes place after node reset!
    public void setNodeConfig(SensorConfig newConfig)
    {
        m_newConfig = newConfig;
    }
    
    public SensorConfig getNodeConfig()
    {
        return m_config;
    }

    public Integer getNodeID() {
        return m_nodeID;
    }

    public SensorPosition getPosition() {
        return m_position;
    }

    public void setPosition(SensorPosition m_position) {
        this.m_position = m_position;
    }
    
    public void moveBy(SensorPosition movementDelta)
    {
        this.m_position.move(movementDelta);
    }
    
    // returns the current current consumption estimate by this node in mA
    // this depends on the current state (sleeping, transmitting, etc.)
    public double getCurrentConsumption() {
        double i = 0.0;
        // TODO get accurate values from real devices for these
        switch(m_state)
        {
            case STATE_INIT:
                i = 10.0;
                break;
            case STATE_CYCLE_SLEEP:
                i = 0.001;
                break;
            case STATE_TRANSMIT:
                i = 30;
                break;
            case STATE_RECEIVE:
                i = 20;
                break;
            default:
                // unrecognized device state
                System.out.println("Unrecognized sensor state!");
        }
                
        return i;
    }
    
    // methods implemented from SimulationItem interface **********************
    
    @Override
    public void timePassed(double passedTimeMs) 
    {
        System.out.println("SensorNode::timePassed not implemented yet");
    }
            
}
