package simulatorcore1;


public class SensorConfig {
    public double   sleepDurationS; // how many seconds will be spent in sleep
    public double   txDurationMs;   // how many ms will be spent transmitting
    public double   rxDurationMs;   // how many ms will be spent receiving
    public int      cycleCount;     // how many times the rx-tx cycle will
                                    // be repeated
    
    // possible values for the arbitration node
    public enum ArbitrationMode {
        ARBITRATE_ODDEVEN,  // node sends every other cycle, receives otherwise
        ARBITRATE_MOD3,     // node sends every third cycle, receives otherwise
        ARBITRATE_MOD4      // node sends every fourth cycle, receives otherwise
    }
    
    public ArbitrationMode arbitrationMode; // which arbitration mode to use
   
}
