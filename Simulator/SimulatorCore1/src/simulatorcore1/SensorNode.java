package simulatorcore1;

public class SensorNode 
{
    // member variables
    private Integer m_nodeID;   // unique node identifier
    private SensorPosition m_position;  // 
    private SensorState m_state;
    
    // internal type definitions
    
    // possible states that a sensor node can be in
    private enum SensorState {
        STATE_INIT,         // sensor has just been powered up
        STATE_CYCLE_SLEEP,  // sensor is sleeping and waiting for the next cycle
        STATE_TRANSMIT,     // sensor is sending presence bitfield data
        STATE_RECEIVE       // sensor is listening for presence bitfield data
    }
    
    
    // constructor for the SensorNode - initialize member variables
    public SensorNode(Integer nodeID, SensorPosition position)
    {
        m_nodeID = nodeID;
        m_position = position;
        m_state = SensorState.STATE_INIT;
        
        System.out.println("Node with id " + nodeID + " created at " + 
                            position);
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
}
