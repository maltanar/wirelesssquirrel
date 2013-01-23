/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.MouseEvent;
import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.event.MouseInputListener;

/**
 *
 * @author GeorgeSong
 */
public class NodeDisplay extends JPanel implements MouseInputListener {

    private boolean pressed = false;
    private int oldX, oldY;
    private SensorNode m_node;
    private final int width = 100;
    private final int height = 30;
    private String m_text;

    public NodeDisplay(SensorNode attachedNode) {
        m_node = attachedNode;
        updateData();
        addMouseListener(this);
        addMouseMotionListener(this);
    }

    public void updateData() {
        // update content of GUI element from simulated SensorNode
        m_text = m_node.getPresenceData().toString();
        // synchronize position
        this.setLocation(m_node.getPosition().x, m_node.getPosition().y);
        this.setSize(width, height);
        repaint();
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);

        g.drawString(m_text, 15, 20);
        
        if(m_node.getNodeID() % 2 == 0)
            g.setColor(Color.yellow);
        else
            g.setColor(Color.red);

        g.fillOval(0, 0, 15, 15);
        
        // Draw ID
        g.setColor(Color.black);
        g.drawString(Integer.toString(m_node.getNodeID()), 5, 13);
        
        if(m_node.getRxActive())
            g.drawString("Rx", 80, 20);
        else if(m_node.getTxActive())
            g.drawString("Tx", 80, 20);
        else
            g.drawString("Zz", 80, 20);
        
    }

    @Override
    public void mouseClicked(MouseEvent e) {
    }

    @Override
    public void mousePressed(MouseEvent e) {
        oldX = e.getXOnScreen();
        oldY = e.getYOnScreen();
        ((EnvironmentDisplay)this.getParent()).encircle(this);
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        // update sim location from gui location
        SensorPosition newPosition = new SensorPosition();
        newPosition.x = this.getLocation().x;
        newPosition.y = this.getLocation().y;
        m_node.setPosition(newPosition);
        ((EnvironmentDisplay)this.getParent()).encircle(null);
    }

    @Override
    public void mouseEntered(MouseEvent e) {
    }

    @Override
    public void mouseExited(MouseEvent e) {
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        Point currentLocation = this.getLocation();

        currentLocation.translate(e.getXOnScreen() - oldX,
                e.getYOnScreen() - oldY);

        this.setLocation(currentLocation);

        oldX = e.getXOnScreen();
        oldY = e.getYOnScreen();
        
        ((EnvironmentDisplay)this.getParent()).encircle(this);
    }

    @Override
    public void mouseMoved(MouseEvent e) {
    }
}
