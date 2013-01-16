cd `dirname $0`
 ROOT_PATH=`pwd`
java -Xms256M -Xmx1024M -cp classpath.jar: mq_test1.sfsensorinserter_1_0.SFSensorInserter --context=Default $* 