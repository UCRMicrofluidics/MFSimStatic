package dmfbSimVisualizer.common;

public class WireSegment {
	public enum SEGTYPE { LINE, ARC } 
	public int pinNo;
	public int layer;
	public SEGTYPE segmentType;	
	
	// Start/stop relative cell locations locations	
	public int sourceGridCellX;
	public int sourceGridCellY;
	public int destGridCellX;
	public int destGridCellY;

	public WireSegment()
	{
		sourceGridCellX = sourceGridCellY = -1;
		destGridCellX = destGridCellY = -1;
	}
	

}
