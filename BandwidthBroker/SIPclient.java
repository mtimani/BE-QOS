import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;
public class SIPclient {
	
	
	static String ip="localhost";
	static Integer port=1236;
	static Boolean isFinished=false;
	static Socket socket = null;
	static PrintStream output;

	
	static String request1="0;192.168.2.1;192.168.3.2;5000;30\n";
	static String request2="0;192.168.3.32;192.168.3.4;5000;60\n";
	static String request3="0;192.168.0.14;192.168.2.6;5000;10\n";
	static String request4="0;192.168.2.1;192.168.3.2;5000;35\n";
	static String request5="1;192.168.2.1;192.168.3.2;5000;30\n";
	static String request6="1;192.168.3.32;192.168.3.4;5000;60\n";
	static String request7="0;192.168.0.27;192.168.2.6;5000;90\n";
	static String request8="1;192.168.2.1;192.168.3.2;5000;35\n";
	
	public static String requestToLiberation(String request)
	{	
		String ret="";
		String[] elements=request.split(";");
		elements[0]="1";
		for (int i=0; i< elements.length-1;i++) {
			ret=ret.concat(elements[i]).concat(";");
		}
			return ret.concat(elements[elements.length-1]);
		}
	
	public static void SIPToBB(String ipBB, Integer portBB, String request)
	{
		
		

		try {
			socket = new Socket(ipBB, portBB);
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		try {
			synchronized(socket)
			{
			InputStream input = socket.getInputStream();
			BufferedReader reader = new BufferedReader(new InputStreamReader(input));
	        output= new PrintStream(socket.getOutputStream());

	        output.println(request);
	        
			String line = reader.readLine();
			
			System.out.println("Answer : "+line);
	         output.close();

			socket.close();
			isFinished=true;
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
			isFinished=true;
			
			System.out.println("Just read on a dead socket");
			 
			

		
		}
	}
	
	public static void main(String[] args) {

		/*
		TimerTask ft = new TimerTask(){
			public void run(){
			     if (!isFinished){
			       try {
				    output.close();

					socket.close();
					isFinished=true;
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			     }
			     System.out.println("Finished this thread");
			     
			   }
			   
			};

			//(new Timer()).schedule(ft, 5000);
			System.out.println("I am now here");

		
		try {
			socket = new Socket("localhost", 1236);
		} catch (UnknownHostException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		try {
			InputStream input = socket.getInputStream();
			BufferedReader reader = new BufferedReader(new InputStreamReader(input));
	        output= new PrintStream(socket.getOutputStream());

	        output.println(request7);
	        
			String line = reader.readLine();
			
			System.out.println("Answer : "+line);
	         output.close();

			socket.close();
			isFinished=true;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			//e.printStackTrace();
			isFinished=true;
			
			System.out.println("Just read on a dead socket");
			 
			

		}
		
		
		System.out.println("Main DONE");
*/
		
		SIPToBB(ip,port,request7);
	//	System.out.println(requestToLiberation(request8));
	}
	
}