package simulatorcore1;

// simple class for representing sensor position data
// for now we use integer coordinates along two axes
public class SensorPosition {

    public int x;
    public int y;

    public SensorPosition() {
        x = 0;
        y = 0;
    }

    @Override
    public String toString() {
        return "(" + Integer.toString(x) + ", " + Integer.toString(y) + ")";
    }

    void move(SensorPosition delta) {
        x += delta.x;
        y += delta.y;
    }
}
