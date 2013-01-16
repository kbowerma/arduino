//-----------------------------------------------------------------------------
//Source File Name: MqttFilePersistence.java
//
//Description: Impementation of the MqttPersistence interface that persists MQTT
// messages to the file system.
//
//Licensed Materials - Property of IBM
//
//5648-C63
//(C) Copyright IBM Corp. 2002, 2005 All Rights Reserved.
//
//US Government Users Restricted Rights - Use, duplication or
//disclosure restricted by GSA ADP Schedule Contract with
//IBM Corp.
//
// Version $Id: MqttFilePersistence.java,v 1.10 2005/12/19 09:49:48 iharwood Exp $
//
//OEM Source Code
//Trace diagnostics have been removed from this version of the source code.
//-----------------------------------------------------------------------------
package com.ibm.mqttsample.persistence;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import com.ibm.mqtt.IMqttClient;
import com.ibm.mqtt.MqttPersistence;
import com.ibm.mqtt.MqttPersistenceException;

/**
 * This class implements the MqttPersistence interface to provide a
 * persistence mechanism that uses the file system.
 * <BR>
 * The directory structure created beneath that specified in the constructor
 * has the form:
 * <pre>
 *   &lt;client Id&gt;\&lt;server&gt;_&lt;port&gt;\sent
 *                              \rcvd
 * </pre>
 * In each directory a file is created for each message key that is sent or received and the file
 * is deleted when all protocol acknowledgements are complete.<BR>
 */                              
public class MqttFilePersistence implements MqttPersistence {
    private String directoryPath            = "";
    private File abstractSentDirPath        = null;
    private File abstractRcvdDirPath        = null;
    private String updEnding                = "u";
    
    private String[] sentFileNamesRestored  = null;
    private String[] rcvdFileNamesRestored = null;

    /** 
     * The default constructor. 
     * The directory structure is created in the current directory
     */
    public MqttFilePersistence() {
    	this( "" );
    }
    
    /**
     * The directory structure is created in the specified directory
     */
    public MqttFilePersistence( String directory ) {
    	directoryPath = directory;
    }
    		
	/**
	 * The clientId, broker TCP/IP address and port uniquely identifies the persistent store
	 * to open. Create directories for both sent and received messages.
	 * @param clientId Identify the client application.
     * @param theConnection A URI as spoecified when connecting the client to describe where the client is connecting to
	 * @throws MqttPersistenceException if the directory structure cannot be created. Exception text gives details.
	 * @see com.ibm.mqtt.MqttPersistence#open(String,String)
	 */
	public void open( String clientId, String theConnection ) throws MqttPersistenceException {
		//System.out.println("1. Opening persistence" );
		String clientDirName = "";

		// Map the protocol connection URL onto a sufficiently unique directory name
		// such that only this connection definition will refer to this persisted state.
		if ( theConnection.startsWith(IMqttClient.LOCAL_ID) ) {
			clientDirName = localConnectionToDirName( theConnection );
		} else if ( theConnection.startsWith(IMqttClient.TCP_ID) ) {
			clientDirName = ipConnectionToDirName( IMqttClient.TCP_ID.length(), theConnection );
		} else {
			throw new MqttPersistenceException("Unrecognised connection method:" + theConnection );
		}
		
		String path = clientId + File.separator + clientDirName + File.separator;
		
		if ( directoryPath.equals( "" ) ) {
			//Relative directory
        	abstractSentDirPath = new File( path + "sent" );
        	abstractRcvdDirPath = new File( path + "rcvd"  );
		} else {
			//Prefix relative directory
        	abstractSentDirPath = new File( directoryPath + File.separator + path + "sent" );
        	abstractRcvdDirPath = new File( directoryPath + File.separator + path + "rcvd" );
		}		
    	
    	// Create the directories to contain the log files
    	if ( !abstractSentDirPath.exists() ) {
        	if ( !abstractSentDirPath.mkdirs() ) {
        		throw new MqttPersistenceException( "open: Failed to create directory " + abstractSentDirPath.getAbsolutePath() );
    	    }	
    	}    

    	if ( !abstractRcvdDirPath.exists() ) {
        	if ( !abstractRcvdDirPath.mkdirs() ) {
        		throw new MqttPersistenceException( "open: Failed to create directory " + abstractRcvdDirPath.getAbsolutePath() );
    	    }	
    	}    
	}
	
	private String localConnectionToDirName( String theConnection ) {
		return theConnection.substring(IMqttClient.LOCAL_ID.length()) + "_0";
	}

	private String ipConnectionToDirName( int schemeLength, String theConnection ) {
        // Find the second occurance of ':' in tcp://host:port - the one separating host and port
        int cIndex = theConnection.indexOf(':');
        cIndex = theConnection.indexOf(':', cIndex+1);

        // Are any additional parameters specified as name=value pairs?
        int sepIndex = theConnection.indexOf(',');
        
        String address = theConnection.substring( schemeLength, cIndex );
        String port = null;
        if ( sepIndex < 0 ) {
            port = theConnection.substring( cIndex + 1 );
        } else {
            port = theConnection.substring( cIndex + 1, sepIndex );
        }
		
		return address + "_" + port;
	}
	
	/**
	 * There is nothing to do for close as we are using the file system, which it doesn't make sense to close
	 * @see com.ibm.mqtt.MqttPersistence#close()
	 */
	public void close() {
		//System.out.println("2. Closing persistence" );
		
		// Nothing to do for close
	}

	/**
	 * Delete files in both the sent and received directories
	 * @throws MqttPersistenceException An exception is never thrown by this implementation
	 * @see com.ibm.mqtt.MqttPersistence#reset()
	 */
	public void reset() throws MqttPersistenceException {
		deleteLogFiles(abstractSentDirPath);
		deleteLogFiles(abstractRcvdDirPath);
	}
	
	/**
	 *  Delete any log files that have been created.
	 * 
	 * @param dir
	 */
	private void deleteLogFiles(File dir) {
		File logFile = null;
		String[] logFilesNames = dir.list();
		if ( logFilesNames != null ) {
			for( int i = 0; i < logFilesNames.length; i++ ) {
				logFile = new File(dir, logFilesNames[i]);
				logFile.delete();
			}	
		}
	}
	
	/**
	 * Load any sent messages found upon restart.
	 * @throws MqttPersistenceException if errors are detected when recovering the persistence store. The exception text gives details.
	 * @see com.ibm.mqtt.MqttPersistence#getAllSentMessages()
	 */
	public byte[][] getAllSentMessages() throws MqttPersistenceException {
		byte[][] returnArray = null;
		File logFile = null;
		// Load any log files that exist
		sentFileNamesRestored = abstractSentDirPath.list();
		
		if ( sentFileNamesRestored != null ) {
			// When recovering sent messages we need to ensure that we only restore the latest
			// message associated with a particular key. When updSentMessage is called to replace a PUBLISH with a PUBREL
			// there is a small overlap where both the PUBLISH and PUBREL are present in the log. Failure at this point would result
			// in both messages being available for recovery, which would result in duplication, so this function ensures that if
			// both a PUBLISH and PUBREL are found for the same key, then only the PUBREL is recovered.
			for( int i = 0; i < sentFileNamesRestored.length; i++ ) {
				// If the logfile ends with the updEnding then this indicates a PUBREL message. In this case check for the existence
				// of a previous publish and delete it.
				logFile = new File(abstractSentDirPath, sentFileNamesRestored[i]);
				String lf = logFile.getAbsolutePath();
				if ( lf.endsWith( updEnding ) ) {
					// Strip the updEnding off and generate the filename of the original publish and delete it.
					File f = new File( lf.substring( 0, lf.length() - updEnding.length() ) );
					if ( f.exists() ) {
						if ( !f.delete() ) {
							// Fatal error
							throw new MqttPersistenceException("getAllSentMessages: Failed to delete file:" + lf );
						}	
					}	
				}	
			}			
			
			// Regenerate the list of logFiles now that duplicates have been removed.
			sentFileNamesRestored = abstractSentDirPath.list();
   
			// Dimension the return array using the number of files found
			returnArray = new byte[sentFileNamesRestored.length][];

			for( int i = 0; i < sentFileNamesRestored.length; i++ ) {
				try {
					logFile = new File(abstractSentDirPath, sentFileNamesRestored[i]);
					FileInputStream fis = new FileInputStream(logFile);

					// Dimension the data array using the number of bytes available	    		
					byte[] data = new byte[fis.available()];
	    		
					fis.read( data );

					// Add this message to the return array at the correct index.
					returnArray[i] = data;
    	    		
					fis.close();
				} catch( IOException ioe ) {
					throw new MqttPersistenceException( "getAllSentMessages: Failed to read file " + logFile.getAbsolutePath() );
				}	
			}	
		}    
		
		return returnArray;
	}

	/**
	 * Load any received messages found upon restart
	 * @throws MqttPersistenceException if errors are detected when recovering the persistence store. The exception text gives details.
	 * @see com.ibm.mqtt.MqttPersistence#getAllReceivedMessages()
	 */
	public byte[][] getAllReceivedMessages()
		throws MqttPersistenceException {
		byte[][] returnArray = null;
		File logFile = null;
		
		// Load any log files that exist
		rcvdFileNamesRestored = abstractRcvdDirPath.list();
		
		if ( rcvdFileNamesRestored != null ) {
			// Dimension the return array using the number of files found
			returnArray = new byte[rcvdFileNamesRestored.length][];

			for( int i = 0; i < rcvdFileNamesRestored.length; i++ ) {
				try {
					logFile = new File(abstractRcvdDirPath, rcvdFileNamesRestored[i]);
					FileInputStream fis = new FileInputStream(logFile);
	    		
					// Dimension the data array using the number of bytes available	    		
					byte[] data = new byte[fis.available()];
	    		
					fis.read( data );
	    		
					// Add this message to the return array at the correct index.
					returnArray[i] = data;
    	    		
					fis.close();
				} catch( IOException ioe ) {
					throw new MqttPersistenceException( "getAllReceivedMessages: Failed to read file " + logFile.getAbsolutePath() );
				}	
			}	
		}    
		
		return returnArray;
	}

	/**
	 * Add a message that is about to be sent to the persistence store
	 * @param key An integer that identifies the sent message
	 * @param message A byte array that represents the message
	 * @throws MqttPersistenceException if the attempt to add the message to the persistent store fails. The exception text gives further details
	 * @see com.ibm.mqtt.MqttPersistence#addSentMessage(int, byte[])
	 */
	public void addSentMessage(int key, byte[] message)
		throws MqttPersistenceException {
		// System.out.println("6. Adding a sent message" );

		File f = new File( abstractSentDirPath, new Integer(key).toString() );
					
		// If the file already exists then delete it
		if ( f.exists() ) {
			f.delete();
		}
			
        FileOutputStream fos;
		try {
    		fos = new FileOutputStream( f );
		} catch( FileNotFoundException fnfe ) {
			throw new MqttPersistenceException("addSentMessage: FileNotFoundException - " + f.getAbsolutePath() );
		}		
		
		try {
    		fos.write( message );
		} catch( IOException ioe1 ) {	
			throw new MqttPersistenceException("addSentMessage: IOException writing to file " + f.getAbsolutePath() );
		}	
		try {
    		fos.close();
		} catch( IOException ioe2 ) {
		}		
	}

	/**
	 * Update a message that has been previously sent
	 * <BR>
     * For QoS 2 messages that are sent two files are created. The first one is simply named with the message key and
     * contains the PUBLISH message sent. The second file is the message key appended with the letter "u" and this contains
     * the PUBREL sent following successful receipt of the PUBLISH by the broker.<BR>
     * Separate files are used to ensure that if a failure occurs during the update the file
     * generated during the addSentMessage processing still exists for recovery.
	 * 
	 * @param key An integer that identifies the message to update
	 * @param message A byte array that represents the new message
	 * @throws MqttPersistenceException if the attempt to update the message in the persistent store fails. The exception text gives further details
	 * @see com.ibm.mqtt.MqttPersistence#updSentMessage(int, byte[])
	 */
	public void updSentMessage(int key, byte[] message)
		throws MqttPersistenceException {
		// System.out.println("7. Updating a sent message" );

        // Generate the names of:
        // 1. The file name that was created when the PUBLISH was sent
        // 2. The file name that will be created for the PUBREL
        // Separate files are used to ensure that if a failure occurs during the update the file
        // generated during the addSentMessage processing still exists for recovery
        String addName = new Integer(key).toString();
        String updName = (new Integer(key).toString()) + updEnding;

        // Generate file objects corresponding to the file names
		File addFile = new File( abstractSentDirPath, addName );
		File updFile = new File( abstractSentDirPath, updName );

        // Write the update to the update file								
        FileOutputStream fos;
		try {
    		fos = new FileOutputStream( updFile );
		} catch( FileNotFoundException fnfe ) {
			throw new MqttPersistenceException("updSentMessage: FileNotFoundException - " + updFile.getAbsolutePath() );
		}		
		
		try {
    		fos.write( message );
		} catch( IOException ioe1 ) {	
			throw new MqttPersistenceException("updSentMessage: IOException writing to file " + updFile.getAbsolutePath() );
		}	
		try {
    		fos.close();
		} catch( IOException ioe2 ) {
		}		

		// Now the the update has succeeded delete the file containing the publish
		// If a failure occurs between the updated PUBREL file being created and the PUBLISH file being deleted
		// then the restart processing (method getAllSentMessages) will ensure that only the PUBREL message
		// is restored.
		if ( addFile.exists() ) {
			addFile.delete();
		}
	}

	/**
	 * Delete a message that has been sucessfully sent
	 * @param key An integer that identifies the message to delete
	 * @throws MqttPersistenceException No exceptions are thrown by this implementation. If delete fails then the messages will
	 * be retried when the program is next restarted ( as this is the only time the persistence log is resynched with the program). This may result
	 * in duplicate QoS 1 messages. QoS 2 message will also be retried, but not duplicated.
	 * @see com.ibm.mqtt.MqttPersistence#delSentMessage(int)
	 */
	public void delSentMessage(int key) throws MqttPersistenceException {
		// System.out.println("8. Deleting a sent message - " + key );

        // When it comes to deleting messages we don't know if we are deleting a
        // log record generated by addSentMessage(PUBLISH) or updSentMessage(PUBREL), so
        // just try and delete both
        String fName = new Integer(key).toString();
        
		File addFile = new File( abstractSentDirPath, fName );
		File updFile = new File( abstractSentDirPath, fName + updEnding );
		
		// If the file exists then delete it
		if ( addFile.exists() ) {
			addFile.delete();
		}
		if ( updFile.exists() ) {
			updFile.delete();
		}
	}

	/**
	 * Add a message that is has been received to the persistence store
	 * @param key An integer that identifies the received message
	 * @param message A byte array that represents the message
	 * @throws MqttPersistenceException if the attempt to add the message to the persistent store fails. The exception text gives further details
	 * @see com.ibm.mqtt.MqttPersistence#addReceivedMessage(int, byte[])
	 */
	public void addReceivedMessage(int key, byte[] message)
		throws MqttPersistenceException {
		//System.out.println("9. Adding a received message" );

		File f = new File( abstractRcvdDirPath, new Integer(key).toString() );
					
		// If the file already exists then delete it
		if ( f.exists() ) {
			f.delete();
		}
			
        FileOutputStream fos;
		try {
    		fos = new FileOutputStream( f );
		} catch( FileNotFoundException fnfe ) {
			throw new MqttPersistenceException("addReceivedMessage: FileNotFoundException - " + f.getAbsolutePath() );
		}		
		
		try {
    		fos.write( message );
		} catch( IOException ioe1 ) {	
			throw new MqttPersistenceException("addReceivedMessage: IOException writing to file " + f.getAbsolutePath() );
		}	
		try {
    		fos.close();
		} catch( IOException ioe2 ) {
		}		
	}

	/**
	 * Delete a message that has been sucessfully received
	 * @param key An integer that identifies the message to delete
	 * @throws MqttPersistenceException If delete fails then there is a small chance
	 * of QoS 2 message being duplicated, so notify the protocol implementation that there is a problem.
	 * @see com.ibm.mqtt.MqttPersistence#delReceivedMessage(int)
	 */
	public void delReceivedMessage(int key) throws MqttPersistenceException {
		//System.out.println("10. Deleting a received message" );

		File f = new File( abstractRcvdDirPath, new Integer(key).toString() );
		
		// If the file exists then delete it
		if ( f.exists() ) {
			if ( !f.delete() ) {
				throw new MqttPersistenceException("delReceivedMessage: Failed to delete file " + f.getAbsolutePath() );
			}	
		}
	}

}
