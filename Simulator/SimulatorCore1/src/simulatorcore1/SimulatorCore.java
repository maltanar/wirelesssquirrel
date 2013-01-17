package simulatorcore1;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
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
}
