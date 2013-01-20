/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Point;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import javax.swing.BorderFactory;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.Border;

/**
 *
 * @author GeorgeSong
 */
public class EnvironmentDisplay extends JPanel {

    private int length;
    private int width;
    private Border blackline = BorderFactory.createLineBorder(Color.BLACK);
    private HashMap<Integer, NodeDisplay> m_sensorDisplayList;
    private PhysicalEnvironment m_environment;
    private Point m_circleCoords = new Point(0, 0);

    public EnvironmentDisplay(PhysicalEnvironment env) {
        m_sensorDisplayList = new HashMap<Integer, NodeDisplay>();
        m_environment = env;
        setSize(env.getWidth(), env.getHeight());
        //setTitle("Sensor Environment Display");
        setBackground(Color.WHITE);
        this.setLayout(null);
    }

    public void checkForNewNodes() {
        for (int i = 0; i < m_environment.getNodeCount(); i++) {
            SensorNode node = m_environment.getNode(i + 1);
            if (!m_sensorDisplayList.containsKey(i + 1)) {
                addSensor(node);
            }
        }
        updateAll();
    }

    /*public void paintSensor(List<SensorNode> SN) {

     for (int i = 0; i < SN.size(); i++) {
     NodeDisplay sensor = new NodeDisplay(SN.get(i));
     sensor.setLocation(SN.get(i).getPosition().x, SN.get(i).getPosition().y);
     sensor.setSize(50, 30);

     // Display the node id
     Integer sensorID = SN.get(i).getNodeID();
     String colon = " : ";
     String bitfield = SN.get(i).getPresenceData();

     String displayData = sensorID.toString() + colon + bitfield;

     sensor.setText(displayData);


     if ((i + 1) % 2 == 0) {
     sensor.setBackground(Color.RED);// Even node
     } else {
     sensor.setBackground(Color.BLUE);// Odd node
     }
     sensor.setBorder(blackline);
     sensors.add(sensor);
     }

     for (int i = 0; i < SN.size(); i++) {
     this.add(sensors.get(i));
     }

     this.repaint();
     }*/
    public void updateAll() {
        Iterator<NodeDisplay> itr = m_sensorDisplayList.values().iterator();

        while (itr.hasNext()) {
            NodeDisplay nxt = itr.next();
            nxt.updateData();
        }

    }

    public void addSensor(SensorNode node) {
        NodeDisplay display = new NodeDisplay(node);
        //sensor.setLocation(node.getPosition().x, node.getPosition().y);
        //display.setSize(50, 30);

        // Display the node id
        /*Integer sensorID = node.getNodeID();
         String colon = " : ";
         String bitfield = node.getPresenceData();

         String displayData = sensorID.toString() + colon + bitfield;

         sensor.setText(displayData);*/

        display.setBorder(blackline);
        m_sensorDisplayList.put(node.getNodeID(), display);

        this.add(display);

        this.repaint();
    }

    public void removeAllSensors() {
        Iterator<NodeDisplay> itr = m_sensorDisplayList.values().iterator();

        while (itr.hasNext()) {
            this.remove(itr.next());
        }

        m_sensorDisplayList.clear();
        this.repaint();
    }

    public void encircle(NodeDisplay d) {
        
        if(d == null) 
            m_circleCoords.x = m_circleCoords.y = 0;
        else 
            m_circleCoords = d.getLocation();
        repaint();
    }

    @Override
    public void paintComponent(Graphics g) {
        super.paintComponent(g);

        //  draw circle indicating range around selected node
        // TODO fix size and position

        if (m_circleCoords.x != 0 && m_circleCoords.y != 0) {
            g.setColor(Color.red);
            g.drawOval(m_circleCoords.x, m_circleCoords.y, 100, 100);
        }

        //g.drawString("testing testing", 20, 20);
    }
}
