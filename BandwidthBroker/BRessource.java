import java.time.Instant;

public class BRessource {

	Integer bandwidth; 
	Instant timeLived;
	public Integer getBandwidth() {
		return bandwidth;
	}
	public void setBandwidth(Integer bandwidth) {
		this.bandwidth = bandwidth;
	}
	public Instant getTimeLived() {
		return timeLived;
	}
	public void setTimeLived(Instant timeLived) {
		this.timeLived = timeLived;
	}
	public BRessource(Integer bandwidth, Instant timeLived) {
		this.bandwidth = bandwidth;
		this.timeLived = timeLived;
	} 
	
	public String toString()
	{
		return "Bandwidth " + bandwidth+ " TimeLived " + timeLived;
		
	}
	
}
