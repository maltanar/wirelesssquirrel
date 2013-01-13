package simulatorcore1;

// base interface for all items to be simulated
// any object that is to be simulated has to have a way to reacting to the
// simulated passage of time, and this is what this interface encapsulates
// TODO extend this interface with the necessary methods
public interface SimulationItem {
    void timePassed(double passedTimeMs);
}
