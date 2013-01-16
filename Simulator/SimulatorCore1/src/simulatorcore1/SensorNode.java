package simulatorcore1;

// TODO add stochastic movement model to sensor nodes
import java.util.LinkedList;

public class SensorNode implements SimulationItem {
    // member variables ******************************************************

    private Integer m_nodeID;   // unique node identifier
    private SensorPosition m_position;  // sensor node position in environment
    private SensorState m_state;    // node state
    private SensorConfig m_config;  // node config
    private SensorConfig m_newConfig;   // new node config, takes effect after
    // reset
    private PresenceData m_presenceData;    // local presence data
    private Boolean m_isRxActive;
    private Boolean m_isTxActive;
    private LinkedList<String> m_rfMessageQueueRx, m_rfMessageQueueTx;
    private double m_timeCounter;   // internal timekeeping variable
    private int m_cycleCounter;

    // internal type definitions *********************************************
    // possible states that a sensor node can be in
    private enum SensorState {

        STATE_INIT, // sensor has just been powered up
        STATE_CYCLE_SLEEP, // sensor is sleeping and waiting for the next cycle
        STATE_CYCLE_ACTIVE     // sensor is rxing or txing presence bitfield data
    }

    // member methods *********************************************************
    // constructor for the SensorNode - initialize member variables
    public SensorNode(Integer nodeID, SensorPosition position,
            SensorConfig config, int maxNetworkSize) {
        m_nodeID = nodeID;
        m_position = position;
        m_config = m_newConfig = config;
        m_isTxActive = Boolean.FALSE;
        m_isRxActive = true;
        m_state = SensorState.STATE_INIT;
        m_presenceData = new PresenceData(maxNetworkSize);
        m_rfMessageQueueRx = new LinkedList<String>();
        m_rfMessageQueueTx = new LinkedList<String>();
        m_timeCounter = 0;
        m_cycleCounter = 0;

        System.out.println("Node with id " + nodeID + " created at "
                + position);
    }

    public void resetNode() {
        // TODO implement full reset functionality here
        m_config = m_newConfig;
        m_state = SensorState.STATE_INIT;
        m_isTxActive = Boolean.FALSE;
        m_isRxActive = true;
        m_rfMessageQueueRx.clear();
        m_rfMessageQueueTx.clear();
        m_presenceData.resetAll();
        m_timeCounter = 0;
        m_cycleCounter = 0;
    }

    // note: config setting only takes place after node reset!
    public void setNodeConfig(SensorConfig newConfig) {
        m_newConfig = newConfig;
    }

    public SensorConfig getNodeConfig() {
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

    public void moveBy(SensorPosition movementDelta) {
        this.m_position.move(movementDelta);
    }

    // returns the current current consumption estimate by this node in mA
    // this depends on the current state (sleeping, transmitting, etc.)
    public double getCurrentConsumption() {
        double i = 0.0;
        // TODO get accurate values from real devices for these
        switch (m_state) {
            case STATE_CYCLE_SLEEP:
                i = 0.001;
                break;

            case STATE_INIT:
            case STATE_CYCLE_ACTIVE:
                // base active current with 32 MHz XOSC
                i = 10;
                // add radio currents depending whether it is enabled
                if (m_isTxActive) {
                    i += 30;
                }
                if (m_isRxActive) {
                    i += 20;
                }
                break;

            default:
                // unrecognized device state
                System.out.println("Unrecognized sensor state!");
        }

        return i;
    }

    // returns whether the node is actively listening for messages
    public Boolean getRxActive() {
        return m_isRxActive;
    }

    // returns whether the node is actively sending messages
    public Boolean getTxActive() {
        return m_isTxActive;
    }

    public void receiveRadioWaves(String data, double rxPower) {
        // do not receive anything if rx is not active
        if (!m_isRxActive) {
            return;
        }

        // cannot receive message if signal is too weak
        if (rxPower < m_config.rxSensitivitydBm) {
            return;
        }

        // otherwise, receive successful - add to a virtual "queue" of messages
        // TODO here would be a good place to implement dropping messages
        // if they are coming in too quickly
        m_rfMessageQueueRx.add(data);

        System.out.printf("Node %d received %s \n", m_nodeID, data);
    }

    public void sendRadioWaves(String data) {
        // do not transmit anything if tx is not active
        if (!m_isTxActive) {
            return;
        }

        m_rfMessageQueueTx.add(data);
    }

    // return and remove first element of the queue if there are pending tx
    // messages, empty string otherwise
    // not inteded to be used by the class itself - will be called by the
    // PhysicalEnvironment to get pending tx messages
    public String getFromTxQueue() {
        if (m_rfMessageQueueTx.size() > 0) {
            return m_rfMessageQueueTx.removeFirst();
        } else {
            return "";
        }
    }

    // return and remove first element of the queue if there are pending rx
    // messages, empty string otherwise
    public String getFromRxQueue() {
        if (m_rfMessageQueueRx.size() > 0) {
            return m_rfMessageQueueRx.removeFirst();
        } else {
            return "";
        }
    }

    // methods implemented from SimulationItem interface **********************
    @Override
    public void timePassed(double passedTimeMs) {
        double newTime = m_timeCounter + passedTimeMs;

        switch (m_state) {
            case STATE_INIT:
                m_isRxActive = true;
                m_isTxActive = false;

                if (waitForSyncMessage()) {
                    // transition from init to sleep state
                    m_state = SensorState.STATE_CYCLE_SLEEP;
                    m_isRxActive = false;
                    m_isTxActive = false;
                    newTime = 0;
                }
                break;

            case STATE_CYCLE_SLEEP:
                m_isRxActive = false;
                m_isTxActive = false;

                if (newTime >= m_config.sleepDurationS * 1000) {
                    // transition from sleep to active state
                    m_state = SensorState.STATE_CYCLE_ACTIVE;
                    // initialize local presence data to all zeroes except
                    // our own node ID
                    m_presenceData.resetAll();
                    m_presenceData.setPresence(m_nodeID);
                    // reset the cycle counter every time we start a new cycle
                    // set, i.e after waking up from sleep
                    m_cycleCounter = 0;
                    // timecounter will be used internally to switch between
                    // tx and rx modes
                    newTime = 0;
                }


                break;

            case STATE_CYCLE_ACTIVE:
                // carry out active state functions depending on elapsed cycle
                // count
                if (isMyTurnToSend()) {
                    m_isRxActive = false;
                    m_isTxActive = true;
                    // TODO maybe add finite tx count limitation here
                    // broadcast current presence data
                    sendRadioWaves(m_presenceData.toString());
                    // check to see if we have completed the current tx cycle
                    if(newTime >= m_config.txDurationMs)
                    {
                        m_cycleCounter++;
                        newTime = 0;
                    }
                } else {
                    m_isRxActive = true;
                    m_isTxActive = false;
                    // handle any received messages
                    String received = "";
                    PresenceData newData = new PresenceData(m_presenceData.getSize());
                    do {
                        received = getFromRxQueue();
                        if (received.length() == newData.getSize()) {
                            // parse the received data
                            newData.fromString(received);
                            // combine with local presence data
                            m_presenceData.combineWith(newData);
                        }
                    } while (!received.isEmpty());
                    // check to see if we have completed the current rx cycle
                    if(newTime >= m_config.rxDurationMs)
                    {
                        m_cycleCounter++;
                        newTime = 0;
                    }
                }
                
                // when we have gone through the tx-rx cycles the desired num
                // of times, go back to sleep
                if(m_cycleCounter == m_config.cycleCount)
                {
                    m_state = SensorState.STATE_CYCLE_SLEEP;
                    m_isRxActive = m_isTxActive = false;
                    newTime = 0;
                }

                break;
        }
        
        m_timeCounter = newTime;
    }

    protected boolean waitForSyncMessage() {
        // to keep the local clock synchronized with the rest of the network,
        // the node will wait until it hears a broadcast to go into active mode
        if (!getFromRxQueue().equals("")) // nonempty value = broadcast rcvd
        {
            return true;
        } else {
            return false;   // no broadcast, keep waiting
        }
    }

    protected boolean isMyTurnToSend() {
        // the node determines whether it will send or receive using
        // the arbitration method
        if (m_config.arbitrationMode
                == SensorConfig.ArbitrationMode.ARBITRATE_ODDEVEN) // odd nodes send in one cycle, even ones in the other
        {
            return (m_nodeID % 2) == (m_cycleCounter % 2);
        } else // TODO implement other arbitration modes
        {
            return false;
        }
    }
}
