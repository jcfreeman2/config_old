#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Tue 30 Sep 2008 04:03:26 PM CEST 

"""
"""

print "[python] Database describes %d classe(s)" % len(database.__schema__.data)
print "[python] Database contains %d object(s)" % len(database.get_all_dals())
print "[python] Database contains %d include(s)" % len(database.getIncludes())

