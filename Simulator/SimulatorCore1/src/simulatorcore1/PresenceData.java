/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

// encapsulate the data and functionality we expect from presence data that
// will be stored locally and broadcasted across nodes
// this is a simple bitfield implementation that we also use in the real life
// implementation of the project
public class PresenceData 
{
    // member variables
    private Boolean[] m_presenceFlags;
    private int m_size;
    
    
    public PresenceData(int size)
    {
        // TODO size range check: must be non-negative non-zero
        m_size = size;
        m_presenceFlags = new Boolean[size];
     
        // construct presence data with all set to 0
        for(int i = 0; i < m_size; i++)
            m_presenceFlags[i] = Boolean.FALSE;
    }
    
    public int getSize()
    {
        return m_size;
    }
    
    public void combineWith(PresenceData data)
    {
        // use logical OR to combine local and remote presence fields
        for(int i = 0; i < m_size; i++)
            m_presenceFlags[i] = m_presenceFlags[i] || data.getPresence(i);
    }
    
    public Boolean getPresence(int index)
    {
        return m_presenceFlags[index];
    }
    
    public void setPresence(int index)
    {
        m_presenceFlags[index] = Boolean.TRUE;
    }
    
    // helper function - set all presence fields to 0
    public void resetAll()
    {
        for(int i = 0; i < m_size; i++)
            m_presenceFlags[i] = Boolean.FALSE;
    }
    
    // helper function - set all presence fields to 1
    public void setAll()
    {
        for(int i = 0; i < m_size; i++)
            m_presenceFlags[i] = Boolean.TRUE;
    }
    
    // produce string of 0 and 1s to get debug print from presence data
    @Override
    public String toString()
    {
        String result = "";
        
        for(int i = 0; i < m_size; i++)
            result += getPresence(i) ? "1 " : "0 ";
        
        // remove trailing space
        result.trim();
        
        return result;
    }
}
