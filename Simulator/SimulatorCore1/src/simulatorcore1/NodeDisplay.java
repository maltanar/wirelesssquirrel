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
            pressed = false;
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
