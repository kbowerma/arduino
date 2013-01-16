%~d0
 cd %~dp0
java -Xms256M -Xmx1024M -cp classpath.jar; mq_test1.sfsensorinserter_1_1.SFSensorInserter --context=Default %* 