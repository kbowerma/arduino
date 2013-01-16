cd `dirname $0`
 ROOT_PATH=`pwd`
java -Xms256M -Xmx1024M -cp classpath.jar: mq_test1.insertpinvalueintosf_0_1.insertPinValueIntoSF --context=sensorInput $* 