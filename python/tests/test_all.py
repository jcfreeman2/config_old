#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Wed 24 Oct 2007 01:55:04 PM CEST

"""Run all unittests integrated
"""

from test_configuration import *
from test_configobject import *
from test_dal import *

if __name__ == "__main__":
    import sys
    sys.argv.append('-v')
    unittest.main()
