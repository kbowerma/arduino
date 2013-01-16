//-----------------------------------------------------------------------------
// Source File Name: Echo.java
//
// Description: Sample MIDP-1.0 MIDlet
//              The MIDlet subscribes to topic midlet/echo/request
//              When a publication is received on this topic it is written to the device
//              screen and published back on topic midlet/echo/response.
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
// Version $Id: Echo.java,v 1.7 2005/07/22 09:16:04 iharwood Exp $
//
//-----------------------------------------------------------------------------
package com.ibm.mqttsample.midpapp;

import java.io.IOException;

import javax.microedition.lcdui.Alert;
import javax.microedition.lcdui.AlertType;
import javax.microedition.lcdui.Command;
import javax.microedition.lcdui.CommandListener;
import javax.microedition.lcdui.Display;
import javax.microedition.lcdui.Displayable;
import javax.microedition.lcdui.Form;
import javax.microedition.lcdui.Item;
import javax.microedition.lcdui.TextBox;
import javax.microedition.lcdui.TextField;
import javax.microedition.midlet.MIDlet;
import javax.microedition.midlet.MIDletStateChangeException;
import javax.microedition.rms.RecordStore;
import javax.microedition.rms.RecordStoreException;

import com.ibm.mqtt.MqttAdvancedCallback;
import com.ibm.mqtt.MqttException;
import com.ibm.mqtt.MqttNotConnectedException;

/**
 * Echo is a sample MQ Telemetry Transport application that can run in the J2ME MIDP
 * environment.<BR>
 * The MIDlet will only work in a MIDP 1.0 environment if TCP/IP sockets are supported.
 * TCP/IP socket support is assured in MIDP 2.0. 
 * <P>
 * When the MIDlet is run the first screen to appear on the device is a configuration
 * screen that allows the broker to be set and a MQTT client identifier to be specified.
 * Following this the MIDlet connects to the broker and subscribes to topic
 * midlet/echo/request. When a publication is received on this topic it is displayed on
 * the device screen and published back on topic midlet/echo/response.<BR>
 * The MIDlet also sets up a MQTT Last Will and Testament in case it is unexpectedly
 * disconnected. The Last Will and Testament will be published to topic midlet/echo/lwt
 * indicating that the client has lost its connection.
 */
public class Echo extends MIDlet implements CommandListener, Runnable, MqttAdvancedCallback {
	private Displayable configPanel = null; 
	private Displayable gaugePanel  = null;
	private TextBox     logPanel    = null;
	private Alert       errorPanel  = null;
	private Alert       infoPanel   = null;
	
	private int         logPanelSize = 256;
	
	private WmqttGauge  gauge       = new WmqttGauge();

	private Command     connect     = new Command( "Connect", Command.OK    , 0 );
	private Command     cancel      = new Command( "Cancel" , Command.CANCEL, 0 );

	private RecordStore config = null;

	// WMQTT connection manager object
	private WmqttMgr wmqttConnMgr = null;

    // Indicies for extracting data from the connect form
    private static final int IDX_CLIENT_ID = 0;
    private static final int IDX_IP_ADDR   = 1;
    private static final int IDX_PORT_NUM  = 2;

	// States for the midlet
	private final static int DISCONNECTED  = 0;
	private final static int CONNECTING    = 1;
	private final static int DISCONNECTING = 2;
	private final static int PUBARRIVED    = 3;
		
	private int state = DISCONNECTED;

	// The name of the J2ME RecordStore into which parameters will be saved.
	private final static String RMS_NAME = "WMQTT";
	
	// Synchronisation object for publishArrived
	private Object pubArrivedLock = new Object();
	
	// Objects have access synchronised by pubArrivedLock
	private String newTopic = null;
	private byte[] newData  = null;
	
    /**
     * Echo zero argument constructor. The MIDlet is initialised
     * when the MIDlet startApp method is called.
     */ 
    public Echo() {
        super();
    }

	/**
	 * @see MIDlet#startApp()
	 * startApp creates all the screens required. StartApp is called each time the
	 * application is resumed from a paused state, so this method and all methods
	 * called by it are written to cope with being called multiple times.
	 */
	protected void startApp() throws MIDletStateChangeException {

		if ( config == null ) {
			// Open a RecordStore to hold TCP/IP configuration data
			try {
				config = RecordStore.openRecordStore( RMS_NAME, true );
			} catch ( RecordStoreException rse ) {
				// Run without the record store
				config = null;
			}
		}

		if ( errorPanel == null ) {
			errorPanel = new Alert( "Echo error!", "", null, AlertType.ERROR );
		}
		if ( infoPanel == null ) {
			infoPanel  = new Alert( "Echo info"   , "", null, AlertType.INFO  );
			infoPanel.setTimeout( 3000 );
		}
		createGaugePanel();
		createLogPanel();
		createConfigPanel();

	}

	/**
	 * @see MIDlet#pauseApp()
	 */
	protected void pauseApp() {
	}

	/**
	 * @see MIDlet#destroyApp(boolean)
	 */
	protected void destroyApp(boolean flag) throws MIDletStateChangeException {
		notifyDestroyed();
	}

	/**
	 * Whenever a significant piece of work needs to be done by the MIDlet
	 * it sets the state appropriately and starts a Thread using this Runnable
	 * to do the work.
	 */
	public void run() {
		
		if ( state == CONNECTING ) {
			// Connect to the broker
			
			// Get the input parameters from the ui
			String clientId	= ((TextField)((Form)configPanel).get(IDX_CLIENT_ID)).getString();
			String broker	= ((TextField)((Form)configPanel).get(IDX_IP_ADDR)).getString();
			String portStr  = ((TextField)((Form)configPanel).get(IDX_PORT_NUM)).getString();
			
			int port = 1883;
			try {
				port = Integer.parseInt( portStr );
			} catch( NumberFormatException nfe ) {
				port = 1883;
			}		
			
			// Store the parameters in an RMS store
            try {
                // BIG Workaround. You should be able to set specific
                // records in the store. This doesn't seem to work reliably
                // on some MIDP runtimes. As a workaround create a new store each time.
                config.closeRecordStore();
                RecordStore.deleteRecordStore(RMS_NAME);
                config = RecordStore.openRecordStore( RMS_NAME, true );
            } catch ( RecordStoreException rse1 ) {
            	config = null;
            }

            if ( config != null ) {
                try {
                    config.addRecord( broker.getBytes()  , 0, broker.length()   );
                    config.addRecord( portStr.getBytes() , 0, portStr.length()  );
                    config.addRecord( clientId.getBytes(), 0, clientId.length() );
                } catch( RecordStoreException rse2 ) {
                }
            }

			// Connect to the broker
			boolean wmqttConnected = true;
			try {
				wmqttConnMgr = new WmqttMgr( clientId, broker, port );
				wmqttConnMgr.setCallback( this );
				wmqttConnMgr.connectToBroker();
			} catch ( MqttException e ) {
				wmqttConnected = false;
				String errMsg = "WMQTT connect failed\n";
				if ( e.getMessage() != null ) {
					errMsg += "\n" + e + "\n";
				}
				if ( e.getCause() != null ) {
					errMsg += "\n" + e.getCause();
				}
				errorPanel.setString( errMsg );
				Display.getDisplay(this).setCurrent( errorPanel, configPanel );
			}		

			// Subscribe for echo requests
			if ( wmqttConnected ) {
				try {
					if ( wmqttConnMgr.subscribe() ) {
						infoPanel.setString("Connected and subscribed!");
						Display.getDisplay(this).setCurrent( infoPanel, logPanel );
					} else {
						errorPanel.setString("Subscribe failed!");
						Display.getDisplay(this).setCurrent( errorPanel, configPanel );
					}
				} catch(Exception e ) {
					errorPanel.setString("Subscribe failed!\n" + e.getMessage() );
					Display.getDisplay(this).setCurrent( errorPanel, configPanel );
				}
			}	
					
			gauge.stop();
			
		} else if ( state == DISCONNECTING ) {
			// Unsubscribe and disconnect from the broker
			
			try {
				wmqttConnMgr.unsubscribe();
			} catch ( Exception e ) {
				// Unable to unsubscribe. If the application connected using the
				// WMQTT clean session option, then the subscription will be
				// automatically removed anyway.
			}		
			
			wmqttConnMgr.disconnectClient();
			wmqttConnMgr.destroyClient();
			wmqttConnMgr = null;
			
			Display.getDisplay(this).setCurrent( configPanel );

			gauge.stop();
			
		} else if ( state == PUBARRIVED ) {
			// Process a publishArrived event.			

			synchronized( pubArrivedLock ) {
				// Display the data on the device screen - protect against data overflow (too large for the screen)
				String logString = "Topic: " + newTopic + "\n\nData : " + new String( newData );
				
				// Truncate the data if necessary to fit in the panel
				if ( logString.length() > logPanelSize ) {
					logString = logString.substring(0, logPanelSize);
				}
				
				try {
					logPanel.setString( logString );
				} catch( Throwable t ) {
					t.printStackTrace();
				}
				
				// Echo the publication back on the response topic.
				try {
					wmqttConnMgr.publishResponse( newData );
				} catch( MqttNotConnectedException mqnce	) {
					errorPanel.setString("publishArrived: WMQTT not connected!");
					Display.getDisplay(this).setCurrent( errorPanel, logPanel );
				} catch ( MqttException mqe ) {
					errorPanel.setString("publishArrived: MqttException:" + mqe.getMessage() );
					Display.getDisplay(this).setCurrent( errorPanel, logPanel );
				} catch ( Exception e ) {
					errorPanel.setString("publishArrived: Exception:" + e.getMessage() );
					Display.getDisplay(this).setCurrent( errorPanel, logPanel );
				}			
			}	
			
		}	
	}	
	
	/**
	 * CommandListener interface
	 * Handle events from the user when buttons on the GUI are pressed.
	 */
	public void commandAction( Command c, Displayable d ) {

		if ( d.equals( configPanel ) ) {
			if ( c.equals( connect ) ) {
				// Proceed
				gauge.setLabel( "Connecting..." );
				gauge.start();

				Display.getDisplay(this).setCurrent( gaugePanel );
				
				// Reinitialise the log panel with the usage
				createLogPanel();
				
				state = CONNECTING;
				new Thread( this ).start();
								
			} else {
				// Cancel
				try {
					destroyApp( true );
				} catch( Exception e ) {
				}		
			}		
		} else if ( d.equals( logPanel ) ) {
			// The only command we expect is cancel
			// Cancel - revert to configuration page
			gauge.setLabel( "Disconnecting..." );
			state = DISCONNECTING;

			gauge.start();
			Display.getDisplay(this).setCurrent( gaugePanel );
			new Thread( this ).start();
			
		}	
	}

	/**
	 * Create the configuration panel. Add the necessary fields to
	 * a form and return the completed Displayable object.
	 */
    public void createConfigPanel() {
    	if ( configPanel == null ) {
			Item[] items = new Item[3];
			String ipAddress = "127.0.0.1";
			String portNum = "1883";
			String clientId = "MQTT_MIDLET";

			// Query the record store (if we have opened it) for any previously set
			// IP address, port number or client identifier
			if ( config != null ) {
				byte[] ip   = null;
				byte[] port = null;
				byte[] cid = null;
				try {
					ip   = config.getRecord(1);
					if ( ip != null ) {
						ipAddress =  new String(ip);
					}
					port = config.getRecord(2);
					if ( port != null ) {
						portNum =  new String(port);
					}
					cid = config.getRecord(3);
					if ( cid != null ) {
						clientId =  new String(cid);
					}
				} catch ( RecordStoreException rse ) {
					// Don't worry if something fails. The user can enter the information again
				}
			}

			// Build up the GUI objects
			TextField t1 = new TextField( "Client Id", clientId, 100, TextField.ANY );
			TextField t2 = new TextField( "IP address", ipAddress, 100, TextField.ANY );
			TextField t3 = new TextField( "IP port", portNum, 4, TextField.NUMERIC );

			items[IDX_CLIENT_ID] = (Item)t1;
			items[IDX_IP_ADDR] = (Item)t2;
			items[IDX_PORT_NUM] = (Item)t3;
			Form f = new Form( "Connection", items );

			f.addCommand( connect );
			f.addCommand( cancel );
        
			f.setCommandListener( this );

			configPanel = f;
			Display.getDisplay(this).setCurrent( configPanel );
    	}
    }

	/**
	 * Create a simple text area where publications are displayed as they arrive.
	 */
	public void createLogPanel() {
		if ( logPanel == null ) {
			TextBox t = new TextBox("Log:", null, logPanelSize, TextField.ANY );
			t.addCommand( cancel );
        
			t.setCommandListener(this);
			
			logPanel = t;
		}
		logPanel.setString( WmqttMgr.getUsage() );
	}
		
	/**
	 * Create a panel in which the continuously scrolling gauge may be displayed
	 * whilst the MIDlet is waiting for certain events to happen. Add a gauge to
	 * a form and return the Displayable object.
	 */
	public void createGaugePanel() {
		if ( gaugePanel == null ) {
			Item[] items = new Item[1];
			items[0] = gauge; 

			Form f = new Form( "Progress", items );
			
			gaugePanel = f;
		}
	}
	
	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#connectionLost()
	 */
	public void connectionLost() throws IOException, MqttException, Exception {
		// NULL method. connectionLost is implemented in WmqttMgr
	}
	

	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#publishArrived(String, byte[], int, boolean)
	 * The WMQTT publish() method cannot be called in the publishArrived callback thread, so start a new thread to do the publish.
	 */
	public void publishArrived( String topic, byte[] data, int qos, boolean retained) throws Exception {
		synchronized( pubArrivedLock ) {
			newTopic = topic;
			newData  = data;
		}	
		
		state = PUBARRIVED;
		new Thread( this ).start();
	}

	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#published( int )
	 */
	public void published( int msgId ) {
	}
	
	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#subscribed(int, byte[])
	 */
	public void subscribed(int aMsgId, byte[] grantedQoS) {
	}

	/**
	 * @see com.ibm.mqtt.MqttAdvancedCallback#unsubscribed(int)
	 */
	public void unsubscribed(int aMsgId) {
	}

}

