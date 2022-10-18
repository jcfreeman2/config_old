#!/bin/bash

eval $(get_lcg_env --prepend --python --ld --path pycodestyle)
pycodestyle python/config/*.py python/tests/*.py

