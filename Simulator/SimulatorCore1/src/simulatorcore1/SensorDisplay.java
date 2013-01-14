/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package simulatorcore1;

import java.awt.Color;
import java.util.LinkedList;
import java.util.List;
import javax.swing.BorderFactory;
import javax.swing.JPanel;
import javax.swing.border.Border;

/**
 *
 * @author GeorgeSong
 */
public class SensorDisplay extends JPanel {

    private int length;
    private int width;
    private Border blackline = BorderFactory.createLineBorder(Color.BLACK);

    public SensorDisplay(int x, int y, int l, int w) {
        this.length = l;
        this.width = w;
        this.setSize(width, length);
        this.setLocation(x, y);
        this.setLayout(null);
        this.setBackground(Color.WHITE);
        this.setBorder(blackline);
    }

    public void paintSensor(List<SensorNode> SN) {

        List<NodeDisplay> sensors = new LinkedList<NodeDisplay>();

        for (int i = 0; i < SN.size(); i++) {
            NodeDisplay sensor = new NodeDisplay();
            sensor.setLocation(SN.get(i).getPosition().x, SN.get(i).getPosition().y);
            sensor.setSize(10, 10);

            
            if((i+1) % 2 ==0)
                sensor.setBackground(Color.RED);// Even node
            else
                sensor.setBackground(Color.BLUE);// Odd node
            
            sensor.setBorder(blackline);
            sensors.add(sensor);
        }
        
        for (int i = 0; i < SN.size(); i++){
            this.add(sensors.get(i));
        }
        
        this.repaint();
    }

}
