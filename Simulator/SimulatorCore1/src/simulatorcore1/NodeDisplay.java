/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

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
    private final int width = 300;
    private final int length = 50;
    private String m_text;

    public NodeDisplay(SensorNode attachedNode) {
        m_node = attachedNode;
        updateData();
        addMouseListener(this);
        addMouseMotionListener(this);
    }

    public void updateData() {
        // update content of GUI element from simulated SensorNode
        String newText = "ID: " + m_node.getNodeID();
        newText += " Flags: " + m_node.getPresenceData().toString();
        m_text = newText;
        // synchronize position
        this.setLocation(m_node.getPosition().x, m_node.getPosition().y);
        this.setSize(width, length);
        repaint();
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);

        // Draw Text
        g.drawString(m_text, 10, 20);
    }

    @Override
    public void mouseClicked(MouseEvent e) {
    }

    @Override
    public void mousePressed(MouseEvent e) {
        System.out.println("pressed");
        oldX = e.getXOnScreen();
        oldY = e.getYOnScreen();
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        // update sim location from gui location
        SensorPosition newPosition = new SensorPosition();
        newPosition.x = this.getLocation().x;
        newPosition.y = this.getLocation().y;
        m_node.setPosition(newPosition);
    }

    @Override
    public void mouseEntered(MouseEvent e) {
    }

    @Override
    public void mouseExited(MouseEvent e) {
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        System.out.println("dragged");
        Point currentLocation = this.getLocation();

        currentLocation.translate(e.getXOnScreen() - oldX,
                e.getYOnScreen() - oldY);

        this.setLocation(currentLocation);

        oldX = e.getXOnScreen();
        oldY = e.getYOnScreen();
    }

    @Override
    public void mouseMoved(MouseEvent e) {
    }
}
