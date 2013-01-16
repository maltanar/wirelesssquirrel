/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.Point;
import java.awt.event.MouseEvent;
import javax.swing.JButton;
import javax.swing.JPanel;

/**
 *
 * @author GeorgeSong
 */
public class NodeDisplay extends JButton {
    private boolean pressed = false;
    private int oldX, oldY;
    private SensorNode m_node;
    
    public void setSensorNode(SensorNode node) {
        m_node = node;
    }
    
    @Override
    protected void processMouseEvent(MouseEvent e)
    {
        if(e.getButton() == MouseEvent.BUTTON1) // mouse press
        {
            pressed = true;
            oldX = e.getXOnScreen();
            oldY = e.getYOnScreen();
        }
        else
        {
            pressed = false;
            SensorPosition newPosition = new SensorPosition();
            newPosition.x = this.getLocation().x;
            newPosition.y = this.getLocation().y;
            m_node.setPosition(newPosition);
        }
    }
    
    @Override
    protected void processMouseMotionEvent(MouseEvent e)
    {
        if(pressed)
        {
            Point currentLocation = this.getLocation();
            
            currentLocation.translate(e.getXOnScreen() - oldX, 
                    e.getYOnScreen() - oldY);
            
            this.setLocation(currentLocation);
                    
            oldX = e.getXOnScreen();
            oldY = e.getYOnScreen();
        }
    }
            
            
}
