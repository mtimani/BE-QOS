package Network;
import Base.Controller;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.DatagramPacket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.channels.UnresolvedAddressException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.concurrent.LinkedBlockingQueue;

import Tools.LogSystem;

import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InterfaceAddress;
import java.net.NetworkInterface;

public class NetworkManager {
	
	private Controller controller;
	//private DynamicDataManager dynamicDM;
	private LinkedBlockingQueue<AbstractPacket> buffPacket; 
	private boolean th_runningUDP;
	private boolean th_runningTCP;
	private DatagramSocket udpSocket;
	
	private boolean hasValidLocalAddress;

	private static int receivPortUDP;
	private static int receivPortTCP;
	
	private static InetAddress localBroadcastAddr;
	private static InetAddress localHostAddr;
	
	private boolean isConnected;
	
	private byte[] receptionBuffer = new byte[50000000];
	
	//private NetworkManagerThread nmTh;
	
	public enum BroadcastType{
		Broadcast, Multicast
	}
	 
	
	public NetworkManager(LinkedBlockingQueue<AbstractPacket> buffPAcket, InetAddress localHostAdd, InetAddress localBroadcastAdd) {
		//NotifyOther for :
				/*
				 * -> status connection
				 * -> update of users infos (id and pwd)
				 * -> 
				 */
		//receivPortUDP = 9524;

		receivPortUDP = 9550;
		receivPortTCP = 9530;
		
		
		buffPacket = buffPAcket;
	
		
		try {
			udpSocket = new DatagramSocket(receivPortUDP);
		} catch (SocketException e) {
			e.printStackTrace();
		}
	

		NetworkManager.localHostAddr = localHostAdd;
		NetworkManager.localBroadcastAddr = localBroadcastAdd;
		
	
		LogSystem.log2("Status of the app: Connected");
		this.updateUsersInfo();
		this.updateMessageInfo();
		LogSystem.log2("NetworkManager created !");
	
		
	}
	
	
	/**
	 * Allow to send a packet to a specific user using its user ID
	 * 
	 * @param p: an instance of a class that inherit from Abstract packet
	 * @param receiverUID
	 * @throws OfflineUserException 
	 * @throws IOException 
	 * @throws UnknownHostException 
	 * @throws Exception
	 */
	public void sendPacket(AbstractPacket packet, String ipAddr) throws UnknownHostException, IOException  {
		
		Socket socket;

		//throws UnknownHostException, IOException
		socket = new Socket(ipAddr, receivPortTCP);
		
		ObjectOutputStream oos;
		oos = new ObjectOutputStream(socket.getOutputStream());
        oos.writeObject(packet);

		
		socket.close();
		
	}
	
	
	/**
	 * Allow to send a packet to a specific user using its user ID
	 * 
	 * @param p: an instance of a class that inherit from Abstract packet
	 * @param receiverUID
	 * @throws OfflineUserException 
	 * @throws IOException 
	 * @throws UnknownHostException 
	 * @throws Exception
	 */
	public void sendPacket(AbstractPacket packet, InetAddress ipAddr) throws OfflineUserException, UnknownHostException, IOException  {
		
		Socket socket;

		//throws UnknownHostException, IOException
		socket = new Socket(ipAddr, receivPortTCP);
		ObjectOutputStream oos;
		oos = new ObjectOutputStream(socket.getOutputStream());
        oos.writeObject(packet);

		
		socket.close();
		
	}
	
	
	public void notifyBroadcast(AbstractPacket p, BroadcastType b) throws IOException {
		
		if(b == BroadcastType.Broadcast) {
			
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);
			oos.writeObject(p);
			oos.flush();
			byte[] data = baos.toByteArray();
			
			DatagramPacket pack = new DatagramPacket(data, data.length, localBroadcastAddr, receivPortUDP);
				//byte[] addr = {127, 0, 0, 1};
				//DatagramPacket pack = new DatagramPacket(data, data.length, InetAddress.getByAddress(addr), receivPortUDPRemote);
	
			//DatagramSocket dgSock = new DatagramSocket();
			DatagramSocket dgSock = new DatagramSocket();
			
			dgSock.send(pack);
			oos.close();
			dgSock.close();
			
		}
		else {//Multicast
			
		}
		
	}

	/**
	 * This methods aims to update users info by creating a thread whose the role is to receive UDP packet
	 */
	private void updateUsersInfo() {
		
		Thread nmThreadUDP = new Thread(new Runnable() {
			public void run() {
				//The thread in charge of listen packets on the network and store them in buffers
				th_runningUDP = true;
				
				DatagramPacket packet;
				
				while(th_runningUDP) {
					packet = new DatagramPacket(receptionBuffer, receptionBuffer.length);
					
					try {
						LogSystem.log2("Waiting for a new packet [UDP] ");
						udpSocket.receive(packet);
					} catch (IOException e) {
						e.printStackTrace();
					}
					
					LogSystem.log2("Packet received [UDP]");
					NetworkManagerThread nmTh = new NetworkManagerThread(packet, true, buffPacket);
					Thread t = new Thread(nmTh);
					t.start();
				
				}
				udpSocket.close();
				LogSystem.log2("Socket closed [UDP] ");
			}
		});
		
		nmThreadUDP.start();
		
	}
	
	

	/**
	 * This methods aims to update Message info by creating a thread whose the role is to receive TCP packet.
	 * It listen to the receiving Port dedicated to TCP and spawns a thread to store the received packet in the buffer 
	 */
	private void updateMessageInfo() {
		
		Thread nmThreadTCP = new Thread(new Runnable() {
			public void run() {
				//The thread in charge of listen packets on the network and store them in buffers
				th_runningTCP = true;
				
				ServerSocket server;
				try {
					server = new ServerSocket(receivPortTCP);

					while(th_runningTCP) {
						try {
							LogSystem.log2("Waiting for a new packet [TCP]");
							Socket socket = server.accept();
							LogSystem.log2("TCP Packet received");
							
							Thread nmThreadExecTCP = new Thread(new Runnable() {
								public void run() {
									ObjectInputStream ois;
									try {
										ois = new ObjectInputStream(socket.getInputStream());
										AbstractPacket p = null;
										try {
											p = (AbstractPacket) ois.readObject();
											if(p != null) {
												synchronized(buffPacket) {
													buffPacket.add(p);
												}
											}
											
											
											LogSystem.log2("Packet buffered !");
										} catch (ClassNotFoundException e) {
											LogSystem.log1("Failed to read an abstract packet from the received packet", e);
											
										} catch (IOException e) {
											LogSystem.log1("Failed to add the received packet to the buffPacket", e);
										}
									} catch (IOException e1) {
										LogSystem.log1("Failed to generate the ObjectInputStream grom the Socket", e1);
									}
								}
							});
							
							nmThreadExecTCP.start();
						} catch (IOException e) {
							LogSystem.log1("server.accept() failed");
						}
					}
					server.close();
					LogSystem.log2("Socket closed [UDP] ");
					
				} catch (IOException e1) {
					LogSystem.log1("Creation of the ServerSocket failed");
				}
				
				
	
					
					//buffPacket.add(packet);

					
					/*InetAddress address = packet.getAddress();
					int port = packet.getPort();
					packet = new DatagramPacket(buf, buf.length, address, port);
					
					String received 
		            = new String(packet.getData(), 0, packet.getLength());
		           
			        udpsocket.send(packet);*/
			}
		});
		nmThreadTCP.start();
	}
	
	
	
	/**
	 * Research and return the host and its corresponding broadcast address of one of the network in which the host belongs, exception of the loopback.
	 * If none address is found then an exception is raised.
	 * @return an ArrayList holding two  string (ex "15.20.20.1")representing the IP addresses. The first (index 0) is the host and the second the broadcast address.
	 * @throws UnresolvedAddressException
	 */
	
	public static ArrayList<InetAddress> retrieveLocalAddr() throws UnresolvedAddressException {
		
		ArrayList<InetAddress> ipBroadAddrs = new ArrayList<>();
		ArrayList<InetAddress> ipHostAddrs = new ArrayList<>();
		
		
		Enumeration<NetworkInterface> interfaces;
		try {
			interfaces = NetworkInterface.getNetworkInterfaces();
			while (interfaces.hasMoreElements()) 
			{
			    NetworkInterface networkInterface = interfaces.nextElement();
			    try {
					if (networkInterface.isLoopback())
					    continue;
				} catch (SocketException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}    // Do not want to use the loopback interface.
			    for (InterfaceAddress interfaceAddress : networkInterface.getInterfaceAddresses()) 
			    {
			        InetAddress broadcast = interfaceAddress.getBroadcast();
			        InetAddress host = interfaceAddress.getAddress();
			        if (broadcast == null)
			            continue;

			        ipBroadAddrs.add(broadcast);
			        ipHostAddrs.add(host);
			    }
			}
		} catch (SocketException e1) {
			e1.printStackTrace();
		}
		
		if(ipHostAddrs.size() == 0) {
			throw new UnresolvedAddressException();
		}
		
		
		ArrayList<InetAddress> res = new ArrayList<>();
		res.add(ipHostAddrs.get(0));
		res.add(ipBroadAddrs.get(0));
		
		return res;
	}

	public void close() {
		th_runningUDP = false;
		th_runningTCP = false;
	}
	
	public boolean isHostConnected() {
		return isConnected;
	}
	
	public static InetAddress getLocalHostAddr() {
		return localHostAddr;
	}
	
	public static InetAddress getLocalBroadcastAddr() {
		return localBroadcastAddr;
	}
	
	public static int getLocalPortUDP() {
		return receivPortUDP;
	}
	
	public static int getLocalPortTCP() {
		return receivPortTCP;
	}
}
