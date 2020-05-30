import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.TimeUnit;

public class RouterSimulator {

	
	static int routerPort=1500;
	public static void main(String[] args) {
	
		
		while(true)
		{
		ServerSocket ssock = null;
			try {
				//ssock.close();
				ssock = new ServerSocket(routerPort);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		      System.out.println("Listening");
		      
		    
		         Socket sock = null;
				try {
					sock = ssock.accept();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				 try {
			         PrintStream pstream = new PrintStream(sock.getOutputStream());
			         InputStream input = sock.getInputStream();
			         BufferedReader reader = new BufferedReader(new InputStreamReader(input));
			         String line = reader.readLine();   
			         String [] elements=line.split(";");
			         
			         if (elements[0]!=null && elements[0].equals("0"))
			         {
			        	 try {
							TimeUnit.SECONDS.sleep(2);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
			        	 String answer="Someone asking to get ressource";
			        	 System.out.println(answer +" to" +line);
			        	 pstream.println("OKs");
 
			         }
			         if (elements[0]!=null && elements[0].equals("1"))
			         {
			        	 try {
								TimeUnit.SECONDS.sleep(2);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
				        	 String answer="Someone asking to release ressource";
				        	 System.out.println(answer +" to" +line);

				        	 pstream.println("OK");
 
				         }
			         pstream.close();
			         sock.close();
			         ssock.close();
			      } catch (IOException e) {
			         System.out.println(e);
			      }
		
		}
		}
	}

