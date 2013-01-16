//-----------------------------------------------------------------------------
// Source File Name: WmqttMgr.java
//
// Description: Class that wraps the WMQTT API to expose methods that
//              are more specific to the Echo application in Echo.class
//              
// Licensed Materials - Property of IBM
//
// 5648-C63
// (C) Copyright IBM Corp. 2002, 2005 All Rights Reserved.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
//
// Version $Id: WmqttMgr.java,v 1.5 2005/07/22 09:16:04 iharwood Exp $
//
//-----------------------------------------------------------------------------
package com.ibm.mqttsample.midpapp;

import com.ibm.mqtt.IMqttClient;
import com.ibm.mqtt.MqttAdvancedCallback;
import com.ibm.mqtt.MqttClient;
import com.ibm.mqtt.MqttException;
import com.ibm.mqtt.MqttNotConnectedException;
import com.ibm.mqtt.MqttPersistenceException;

/**
 * This class customises the MQTT Java Client API for use by the Echo application.
 */
public class WmqttMgr implements MqttAdvancedCallback {
    private final static String REQ_TOPIC = "midlet/echo/request";   // The topic to which this MIDlet subscribes
    private final static String RESP_TOPIC = "midlet/echo/response"; // The topic to which this MIDlet publishes

    private IMqttClient  wmqttClient   = null;
    private String       clientId      = null;
    private int[]        subQoS        = { 0 };
    private boolean      connected     = false;
    
    private Object       connLock = new Object();
    private boolean      connLockNotified = false;
    
	private Object       subLock = new Object();
	private boolean      subLockNotified = false;

    private MqttAdvancedCallback callback = null;
    
    private boolean      userConnect = true; // Is connect being initialted by the user?
    // LW&T
    private final static String LWT_TOPIC = "midlet/echo/lwt";
    private String lwtMsg = null;
    
    /**
     * The constructor creates the MqttClient object which contains the
     * MQTT API. It also registers this class as the one that will handle
     * MQTT callback events.
     * @param theClientId The MQTT Client identifier to use
     * @param theServer   The TCP/IP address of the broker
     * @param thePort     The TCP/IP port of the broker
     */
    public WmqttMgr( String theClientId, String theServer, int thePort ) throws MqttException {
    	clientId = theClientId;
    	lwtMsg = clientId + " has gone offline";
    	
    	wmqttClient = MqttClient.createMqttClient( MqttClient.TCP_ID + theServer + ":" + thePort, null );
    	wmqttClient.registerAdvancedHandler( this );
    }
    	    	
    /**
     * Disconnect the client from the broker. If any connectionLost processing
     * is occurring then it is also stopped.
     */
    public boolean disconnectClient() {
    	synchronized( connLock ) {
    		connLockNotified = true;
    		connLock.notify();
    	}	

		try {
			wmqttClient.disconnect();
		} catch( MqttPersistenceException mqpe ) {
			// Persistence is not used
		}		
		
		connected = false;
		
		return true;
    }
    	
    /**
     * destroyClient terminates all threads running in the MQTT implementation.
     * Before a new connection can be made a new instance of this
     * class must be instantiated.
     */	
    public boolean destroyClient() {
		
		try {
			Thread.sleep( 100 );
		} catch ( InterruptedException ie ) {
		}
		
		wmqttClient.terminate();
		wmqttClient = null;
			
    	return true;
    }
    
	/**
	 * A user of this class may also want to receive callback events from
	 * MQTT. Only one object may recieve the events.
	 * @param theCallback The Object that wishes to receive MQTT protocol events.
	 */
    public void setCallback( MqttAdvancedCallback theCallback ) {
    	callback = theCallback;
    }	
		
	/**
	 * Connect this MQTT client to the broker
	 */
	public void connectToBroker() throws MqttException {
		userConnect = true;
    	try {
    		connectionLost();
    		connected = true;
    	} catch( MqttException e ) {
    		connected = false;
    		throw e;
    	} finally {
    		userConnect = false;
    	}
	}
		
	/**
	 * Subscribe to the request topic. The request topic is defined in this class.<BR>
	 * This method blocks until the subscribe is acknowledged by the broker.
	 */	
	public boolean subscribe() throws Exception {
		String[] topics = new String[1];
		int[] qosarr = new int[1];
		topics[0] = REQ_TOPIC;
		qosarr[0] = 1;
		
		boolean ret = false;

		try {
			synchronized( subLock ) {
				wmqttClient.subscribe( topics, qosarr );
				while( !subLockNotified ) {
					subLock.wait();
				}
				subLockNotified = false;
			}
			ret = true;
		} catch( Exception e ) {
			// Subscribe failed
			ret = false;
		}		
		
		return ret;
	}
			
	/**
	 * Unsubscribe from the request topic.<BR>
	 * This method blocks until the unsubscribe is acknowledged by the broker.
	 */	
	public void unsubscribe() throws Exception {
		String[] topics = new String[1];
		topics[0] = REQ_TOPIC;

		synchronized( subLock ) {
			wmqttClient.unsubscribe( topics );
			while( !subLockNotified ) {
				subLock.wait();
			}
			subLockNotified = false;
		}
	}
		
	/**
	 * Publish data to the response topic. The response topic is defined in this class.
	 */	
	public void publishResponse( byte[] responseData ) throws MqttNotConnectedException, MqttException, Exception {
		wmqttClient.publish( RESP_TOPIC, responseData, 1, false );
	}	
	
	/**
	 * Handle a broken MQTT connection.<BR>
	 * This reconnects to the broker and resubscribes to the request topic in the
	 * event of the MQTT connection unexpectedly breaking.
	 * @see com.ibm.mqtt.MqttAdvancedCallback#connectionLost()
	 */
	public void connectionLost() throws MqttException {
		boolean reconnected = false;
	
		synchronized( connLock ) {
			while( !reconnected && !connLockNotified ) {
				try {
					wmqttClient.connect( clientId, true, (short)30, LWT_TOPIC, 0, lwtMsg, false );
					reconnected = true;
				} catch( MqttException mqe ) {
					// Some sort of MQTT error has occurred - retry.
					if ( userConnect ) {
						// If the connect is initiated by the user feed the execption back to the API
						throw mqe;
					}
					// An else block could display an error Alert panel on the device.
				}	
			
				try {
					connLock.wait( 2000 );
				} catch( InterruptedException ie ) {
				}			
			}
			connLockNotified = false;
		}	
	
		if ( reconnected ) {
			try {
				String requestTopic[] = { REQ_TOPIC };
				wmqttClient.subscribe( requestTopic, subQoS );
				
			} catch ( MqttException e ) {
				disconnectClient();
				destroyClient();
				throw e;
			}			
		}		
			
	}
	

	/**
	 * Receives a publication from the MQTT client and pass it onto the class that registered
	 * an interest in receiving MQTT protocol events.
	 * @see com.ibm.mqtt.MqttAdvancedCallback#publishArrived(String, byte[], int, boolean)
	 */
	public void publishArrived( String topic, byte[] data, int qos, boolean retained) throws Exception {
		callback.publishArrived( topic, data, qos, retained );
	}

	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#published( int )
	 */
	public void published( int msgId ) {
		callback.published( msgId );
	}
	
	/**
	 * This methods notifies the subscribe method in this class when the subscribe
	 * has been acknowledged.
	 * @see com.ibm.mqtt.MqttAdvancedCallback#subscribed(int, byte[])
	 */
	public void subscribed(int aMsgId, byte[] grantedQoS) {
		synchronized( subLock ) {
			subLockNotified = true;
			subLock.notify();
		}
	}

	/**
	 * This methods notifies the unsubscribe method in this class when the unsubscribe
	 * has been acknowledged.
	 * @see com.ibm.mqtt.MqttAdvancedCallback#unsubscribed(int)
	 */
	public void unsubscribed(int aMsgId) {
		synchronized( subLock ) {
			subLockNotified = true;
			subLock.notify();
		}
	}

	/**
	 * Display usage for this MIDlet
	 * @return A string containing usage information that may be displayed on the device screen.
	 */
	public static String getUsage() {
		// NOTE return string shouldn't be longer than Echo.logPanelSize
		StringBuffer usage = new StringBuffer("Publish to topic \'");
		usage.append(REQ_TOPIC);
		usage.append("\'\nEcho is published to topic \'");
		usage.append(RESP_TOPIC);
		usage.append("\'\nOn unexpected termination a publication is sent to topic \'");
		usage.append(LWT_TOPIC);
		usage.append("\'\n");		
				
		return usage.toString();
	}
}
