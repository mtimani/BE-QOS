import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;
import java.sql.Time;
import java.time.Instant;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;


public class BandwidthBroker implements Runnable  {

	public static class CleanRessources extends TimerTask{

		@Override
		public void run() {
			
			ArrayList<String> mySet=new ArrayList<String>();
			Instant now=Instant.now();
			synchronized (ressourcesUsed)
			{
				synchronized(ongoingCalls)
				  
				   {
					for (String e:ongoingCalls.keySet()) {
						if(ongoingCalls.get(e).getTimeLived().plusSeconds(CLEANCONST).isBefore(now))
						{
							mySet.add(e);
						}
					}
					showongoingCalls();
				   }
			           System.out.println("Cleaning ");  
			           
			           
			           
					   for (String name: mySet){
			           
			         
								System.out.println("We removed" + name);
								removeCall(name.split("/")[0],name.split("/")[1]);
								synchronized(requestHistory)
								{	
									showRequestHistory();
									String line=null;
									line=requestHistory.get(name);
									if (line!=null)
									{	
										//modify lines
										
										
										if (test)
											{BBToRouter(routerIP, routerPort, requestToLiberation(line));	
											}
										else
										{
											
								          BBToRouter(ipToRouter(line.split(";")[1]), routerPort, line);
								          BBToRouter(ipToRouter(line.split(";")[2]), routerPort, line);
									         
										}
										requestHistory.remove(name);
									}
								
								
							}					   
					   
					   } 		
				   
			}
		
		}
	   
	}	
	static boolean test=true;
	static final int CLEANCONST=4;// cleanup time in seconds.
	static int routerPort=1500;
	static String routerIP="localhost";
	static String testResponse="OK";
	static int number=5;
		Socket csocket;
	   public BandwidthBroker(Socket csocket) {
	      this.csocket = csocket;
	      //number--;
	   }
	   static Socket socket = null;
		static PrintStream output;
	   static HashMap<String,Integer> sla=new HashMap<String,Integer>();
	   static HashMap<String,Integer> ressourcesUsed=new HashMap<String,Integer>(); 
	   static HashMap <String,BRessource> ongoingCalls=new HashMap<String,BRessource>();
	   static HashMap <String,String> requestHistory= new HashMap<String,String>();
	   CleanRessources cR=new CleanRessources();
		static Boolean isFinished=false;

	   
	   static void addToSla(String ip,Integer res)
	   {	
		   synchronized (sla)
		   {
			   sla.put(ip,res);
		   }
	   }   
	   
	
	   static boolean reduceRessource(String ip,Integer band)
	   {	
		   synchronized(ressourcesUsed)
			  {
		   Integer aux=ressourcesUsed.get(ip);

		   if (aux-band>0)
		   {
		   ressourcesUsed.replace(ip,aux-band);
		   return true;
		   }
		   else
		   {
			   ressourcesUsed.remove(ip);
			   return true;

		   }
			  }
	   }
	   static boolean addRessource(String ip,Integer band)
	   {
		   
		   synchronized (sla)
			  {
				  synchronized(ressourcesUsed)
				  {	
					  if (sla.containsKey(ip) && !ressourcesUsed.containsKey(ip))
					   {
						   ressourcesUsed.put(ip,band);
						   return true;
					   }
					  else if (sla.containsKey(ip))
					  {
					  
					   Integer aux=ressourcesUsed.get(ip);
					   if (aux+band<sla.get(ip))
					   {
					   ressourcesUsed.replace(ip,aux+band);
					   return true;
					   }
					  }
				  }
			  }
		   
		return false;
	   }
	   
	
	   
	  public static boolean addCall(String ip, Integer band,String port)
	  {
		  synchronized (ongoingCalls)
		  {
			 if (ongoingCalls.containsKey(ip.concat("/").concat(port)))
			 {
				 ongoingCalls.replace(ip.concat("/").concat(port), new BRessource(band,Instant.now()));
				 return true;
			 }
			 else 
				 
				 if (sla.containsKey(ipToNetwork(ip)))
				 {	
					 
					 Boolean ret=addRessource(ipToNetwork(ip),band);
					 if (ret) {
						 ongoingCalls.put(ip.concat("/").concat(port), new BRessource(band,Instant.now()));
					 }


					 return ret;
				 }
			 	
			  
		  }


		  return false;
	  }
	  
	  
	  public static boolean removeCall(String ip,String port)
	  {

		  synchronized (ongoingCalls)
		  {
			if (ongoingCalls.containsKey(ip.concat("/").concat(port)))
			{
				
			boolean ans= reduceRessource(ipToNetwork(ip),  ongoingCalls.get(ip.concat("/").concat(port)).getBandwidth());
			ongoingCalls.remove(ip.concat("/").concat(port));
			return ans;
			}
			return false;
			  
		  }
	  }
	  
	  
	   
	   
	   
	   //Timer task thread to remove , check how to synchronize methods/variables
	   
	   
	   
	   public void run() {
		      try {
		         PrintStream pstream = new PrintStream(csocket.getOutputStream());
		         InputStream input = csocket.getInputStream();
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
		        	 System.out.println( answer);
		        	 if (addCall(elements[1],Integer.parseInt(elements[4]),elements[3])&& addCall(elements[2],Integer.parseInt(elements[4]),elements[3]))
		        		 {
			        		 synchronized(requestHistory)
				        	 {
			        		 requestHistory.put(elements[1].concat("/").concat(elements[3]), line);
				        	 
				        	 }
			        		 pstream.println("OK");

		        		 }
		        	 else 
		        	 {
			        	 pstream.println("KO");

		        	 }
		        		 
			        
		        	 
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
				         System.out.println(answer);
				         if (removeCall(elements[1],elements[3])&&removeCall(elements[2],elements[3]))
		        		 {
				        	 synchronized(requestHistory)
				        	 {
				        	 requestHistory.remove(elements[1].concat("/").concat(elements[4]));
				        	 }
				        	 pstream.println("OK");

		        		 }
			        	 else 
			        	 {
				        	 pstream.println("KO");
	
			        	 }
			         }
		         	
		         if (test)
		         {		        	 
		        	 if  (!BBToRouter(routerIP, routerPort, line));
		        	 {
		        		 removeCall(elements[1],elements[3]);
		        		 removeCall(elements[2],elements[3]);
		        	}
		        	 }
		         else
		         {
			          BBToRouter(ipToRouter(line.split(";")[1]), routerPort, line);
			          BBToRouter(ipToRouter(line.split(";")[2]), routerPort, line);
			          if  (!BBToRouter(ipToRouter(line.split(";")[1]), routerPort, line));
			        	 {
			        		 removeCall(elements[1],elements[3]);
			        		 removeCall(elements[2],elements[3]);
			        	}
			        	 if  (!BBToRouter(routerIP, routerPort, line));
			        	 {
			        		 removeCall(elements[1],elements[3]);
			        		 removeCall(elements[2],elements[3]);
			        	 }
		         }
		         // BBToRouter(routerIP, routerPort, line);
		         pstream.close();
		         csocket.close();
		      } catch (IOException e) {
		         System.out.println(e);
		      }
		   }
	   public static String ipToNetwork(String ip) 
		  {
			 
			  String res="";
			  String [] ipParts=ip.split("\\.");
			  for (int i=0;i<3;i++)
			  {	

				  res=res.concat(ipParts[i]+".");
			  }
	 
			  return res.concat("0") ;
		  }
	   public static String ipToRouter(String ip) 
		  {
			 
			  String res="";
			  String [] ipParts=ip.split("\\.");
			  for (int i=0;i<3;i++)
			  {	

				  res=res.concat(ipParts[i]+".");
			  }
	 
			  return res.concat("254") ;
		  }
	
	   public static void showSla()
	   {	
		   
            

           synchronized(sla)
           {
        	   System.out.println("\n SLA\n "); 
		   for (String name: sla.keySet()){
	            String key = name.toString();
	            String value = sla.get(name).toString();  
	            System.out.println(key + " " + value);  
		   } 
           		}
	   }
	   public static void showRessources()
	   {	
          

		   synchronized(ressourcesUsed)
		   {
			   System.out.println("\n Ressources Used\n");  
		   for (String name: ressourcesUsed.keySet()){
	            String key = name.toString();
	            String value = ressourcesUsed.get(name).toString();  
	            System.out.println(key + " " + value);  
		   		} 
		   }
	   }
	   
	   public static void showRequestHistory()
	   {	
          

		   synchronized(requestHistory)
		   {
			   System.out.println("\n Request History\n");  
		   for (String name: requestHistory.keySet()){
	            String key = name.toString();
	            String value = requestHistory.get(name);  
	            System.out.println(key + " " + value);  
		   		} 
		   }
	   }
	   public static void showongoingCalls()
	   {	

		   synchronized(ongoingCalls)
		  
		   {
	           System.out.println("\n OngoingCalls\n");  

			   for (String name: ongoingCalls.keySet()){
	            String key = name.toString();
	            String value = ongoingCalls.get(name).toString();  
	            System.out.println(key + " " + value);  
		   		} 		
		   }
	   }
	   
		public static synchronized boolean BBToRouter(String ipBB, Integer portBB, String request)
		{	
			boolean ret=false;
			
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
				InputStream input = socket.getInputStream();
				BufferedReader reader = new BufferedReader(new InputStreamReader(input));
		        output= new PrintStream(socket.getOutputStream());

		        output.println(request);
		        
				String line = reader.readLine();
				
				System.out.println("Answer : "+line);
				ret=line.equals("OK");
		         output.close();

				socket.close();
				isFinished=true;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
				isFinished=true;
				
				System.out.println("Just read on a dead socket");
				 
				

			}  
			
			return ret;
			
		}
	   
	public static void clean()
	{
			Timer timerClean=new Timer();
			timerClean.schedule(new CleanRessources(),1000,10000);
			
			
	}
	public static String requestToLiberation(String request)
	{	
		String ret="";
		String[] elements=request.split(";");
		elements[0]="1";
		for (int i=0; i< elements.length-1;i++) {
			ret=ret.concat(elements[i]).concat(";");
		}
		System.out.println("About to ask to release "+ret.concat(elements[elements.length-1]));

			return ret.concat(elements[elements.length-1]);
		}

		
		public static void main(String[] args) {
		
		addToSla("192.168.0.0",300);
		addToSla("192.168.1.0",300);
		addToSla("192.168.2.0",300);
		showSla();
		/*addCall("192.168.0.5",20,"5000");
		addCall("192.168.0.7",20,"400");
		addCall("192.168.0.7",20,"5000");
		removeCall("192.168.0.7","400");
		*/
		showongoingCalls();
		showRessources();
		clean();
		
		
		
		// TODO Auto-generated method stub
		 ServerSocket ssock = null;
		try {
			//ssock.close();
			ssock = new ServerSocket(1236);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	      System.out.println("Listening");
	      
	      while (number>0) {
	         Socket sock = null;
			try {
				sock = ssock.accept();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
	         System.out.println("New client connected "+ number + " Sessions still available ");
	         new Thread(new BandwidthBroker(sock)).start();
	         if (number==0)
	         {
	        	 try {
					ssock.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
	         }
	}
	
	}
}