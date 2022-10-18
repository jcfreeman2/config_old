#! /bin/sh

###############################################################################

test_schema_file="/tmp/test.schema.$$.xml"
test_data_file="/tmp/test.data.$$.xml"

rm -f ${test_schema_file} ${test_data_file}
cmd="oks_tutorial ${test_schema_file} ${test_data_file}"

echo 'GENERATE FILES FOR TEST ...'
echo " * execute \"${cmd}\""

${cmd} > /dev/null
if [ ! -f ${test_schema_file} ] ; then
  echo 'ERROR: oks_tutorial failed, can not produce schema file for test'
  exit 1
fi

echo ' * done'
echo ''


###############################################################################

echo '**********************************************************************'
echo '******************** tests using oksconfig plug-in *******************'
echo '**********************************************************************'
echo ''

echo "${1}/config_time_test -d oksconfig:daq/segments/setup-initial.data.xml"
echo ''

if ${1}/config_time_test -d "oksconfig:daq/segments/setup-initial.data.xml"
then
  echo '' 
  echo 'config_time_test test passed' 
else
  echo '' 
  echo 'config_time_test test failed'
  exit 1
fi

echo ''
echo "${1}/config_dump -d oksconfig:${test_data_file} -c Employee"
echo ''

if ${1}/config_dump -d "oksconfig:${test_data_file}" -c "Employee"
then
  echo ''
  echo 'config_dump test passed'
else
  echo ''
  echo 'config_dump test failed'
  exit 1
fi

echo '' 
echo '**********************************************************************'

###############################################################################

rm -f ${test_schema_file} ${test_data_file}

###############################################################################

schema_file="${2}/test/test.schema.xml"
data_file="/tmp/test-config-by-`whoami`.$$"

echo ''
echo ''
echo '**********************************************************************'
echo '************* config_test_rw test using oksconfig plug-in ************'
echo '**********************************************************************'
echo ''

echo "${1}/config_test_rw -d ${data_file} -s ${schema_file} -p oksconfig"
echo ''

if ${1}/config_test_rw -d ${data_file} -s ${schema_file} -p oksconfig
then
  echo '' 
  echo 'config_test_rw test passed' 
else
  echo '' 
  echo 'config_test_rw test failed'
  exit 1
fi

rm -rf ${data_file}*

echo '' 
echo '**********************************************************************'

echo ''
echo ''
echo '**********************************************************************'
echo '************* config_test_rw test using rdbconfig plug-in ************'
echo '**********************************************************************'
echo ''

ipc_file="/tmp/ipc_root.$$.ref"
TDAQ_IPC_INIT_REF="file:${ipc_file}"
export TDAQ_IPC_INIT_REF

write_server_name='config_w'
read_server_name='config_r'
cleanup='ipc_rm -i ".*" -n ".*" -f'
startup_timeout='10'

echo 'destroy running ipc programs ...'
echo "${cleanup}"
${cleanup} > /dev/null 2>&1 || exit 1

trap 'echo "caught signal: destroy running ipc programs ..."; ipc_rm -i ".*" -n ".*"; exit 1' 1 2 15


  # run general ipc_server

echo 'ipc_server &'

ipc_server > /dev/null 2>&1 &
sleep 1 # is used to avoid waiting message

count=0
result=1
while [ $count -lt ${startup_timeout} ] ; do
  test_ipc_server
  result=$?
  if [ $result -eq 0 ] ; then break ; fi
  echo ' * waiting general ipc_server startup ...'
  sleep 1
  count=`expr $count + 1`
done

if [ ! $result -eq 0 ] ; then
  echo "ERROR: failed to run general ipc_server (timeout after $startup_timeout seconds)"
#  exit 1
fi


  # run rdb servers

echo "rdb_writer -d ${write_server_name} &"
rdb_writer -d ${write_server_name} > /dev/null 2>&1 &

echo "rdb_server -d ${read_server_name} -D daq/segments/setup-initial.data.xml &"
rdb_server -d ${read_server_name} -D daq/segments/setup-initial.data.xml > /dev/null 2>&1 &

sleep 1 # is used to avoid waiting message

count=0
result=1
while [ $count -lt ${startup_timeout} ] ; do
  test_corba_server -c 'rdb/writer' -n ${write_server_name}
  result=$?
  if [ $result -eq 0 ] ; then break ; fi
  echo ' * waiting rdb_writer startup ...'
  sleep 1
  count=`expr $count + 1`
done

if [ ! $result -eq 0 ] ; then
  echo "ERROR: failed to run rdb_writer (timeout after $startup_timeout seconds)"
fi


count=0
result=1
while [ $count -lt ${startup_timeout} ] ; do
  test_corba_server -c 'rdb/cursor' -n ${read_server_name}
  result=$?
  if [ $result -eq 0 ] ; then break ; fi
  echo ' * waiting rdb_server startup ...'
  sleep 1
  count=`expr $count + 1`
done

if [ ! $result -eq 0 ] ; then
  echo "ERROR: failed to run rdb_server (timeout after $startup_timeout seconds)"
fi

echo ""
echo ''

echo "${1}/config_test_rw -d ${data_file} -s ${schema_file} -p rdbconfig:${write_server_name}"
echo ''

if ${1}/config_test_rw -d ${data_file} -s ${schema_file} -p "rdbconfig:${write_server_name}"
then
  echo '' 
  echo 'config_test_rw test passed' 
else
  echo '' 
  echo 'config_test_rw test failed'
  exit 1
fi

echo '' 
echo ''
echo ''
echo '**********************************************************************'
echo '************ config_dump java test using rdbconfig plug-in ***********'
echo '**********************************************************************'
echo ''

echo $TDAQ_JAVA_HOME/bin/java -classpath ${1}/config.jar:'$CLASSPATH' config.ConfigDump -d "rdbconfig:${read_server_name}" -C OnlineSegment -O -r
if $TDAQ_JAVA_HOME/bin/java -classpath ${1}/config.jar:$CLASSPATH config.ConfigDump -d "rdbconfig:${read_server_name}" -C OnlineSegment -O -r
then
  echo '' 
  echo 'java config_dump test passed' 
else
  echo '' 
  echo 'java config_dump test failed'
  exit 1
fi

echo '' 
echo 'destroy running servers ...'

echo "rdb_admin -d ${write_server_name} -s"
rdb_admin -d ${write_server_name} -s

echo "rdb_admin -d ${read_server_name} -s"
rdb_admin -d ${read_server_name} -s

echo '' 
echo 'final cleanup...'

${cleanup} > /dev/null 2>&1
rm -rf ${data_file}* ${ipc_file}

echo '' 
echo '**********************************************************************'

