#!/bin/bash
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Fri 23 Nov 2007 12:24:50 PM CET
export TDAQ_PYTHONPATH=${TDAQ_INST_PATH}/share/lib/python:${TDAQ_INST_PATH}/${CMTCONFIG}/lib:${TDAQ_PYTHONPATH}
export TDAQ_DB_PATH=${1}/python/tests:${TDAQ_DB_PATH}
echo "TDAQ_PYTHONPATH=$TDAQ_PYTHONPATH"
echo "TDAQ_DB_PATH=$TDAQ_DB_PATH"
tdaq_python ${1}/python/tests/test_all.py
exit $?
