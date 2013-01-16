//-----------------------------------------------------------------------------
// Source File Name: WmqttGauge.java
//
// Description: Gauge that has been extended so that it can automatically
//              progress itself whilst completion of an operation is awaited.
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
// Version $Id: WmqttGauge.java,v 1.4 2005/07/22 09:16:04 iharwood Exp $
//
//-----------------------------------------------------------------------------
package com.ibm.mqttsample.midpapp;

import javax.microedition.lcdui.*;

/**
 * This extends Gauge to make a gauge that can continuously scroll. This functionality
 * is build into a MIDP 2.0 Gauge, but is not available for MIDP 1.0.
 */
public class WmqttGauge extends Gauge implements Runnable {
	private Object   runLock = new Object();
	private boolean runLockNotified = false;
	
	private final static int MAX_VALUE = 100;
	private int curValue = 0;
	
	/**
	 * Create a Gauge with no label
	 */
	public WmqttGauge() {
       this( "" );
	}
	
	/**
	 * Create a Gauge with label the specified label
	 * @param label The label to attach to the gauge
	 */
	public WmqttGauge( String label ) {
       super( label, false, MAX_VALUE, 0 );
	}	

	/**
	 * This Runnable method continuosly scrolls the Gauge until it is notified to stop.
	 */	
	public void run() {
		synchronized( runLock ) {
			while ( !runLockNotified ) {
				curValue += 5;
				if ( curValue > MAX_VALUE ) {
					curValue = 0;
				}	
				setValue( curValue );
				try {
					runLock.wait( 100 );
				} catch ( InterruptedException ie ) {
				}	
			}
			runLockNotified = false;		
		}	
	}		

	/**
	 * Start the gauge scrolling
	 */
	public void start() {
       new Thread( this ).start();
	}
			
	/**
	 * Stop the gauge scrolling
	 */
	public void stop() {
		synchronized( runLock ) {
			runLockNotified = true;
			runLock.notify();
		}	
	}	

}
