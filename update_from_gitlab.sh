#!/bin/bash
#
# after running this once:
# git remote add upstream https://gitlab.com/kobily/sf2000_multicore
#
# these two lines will update to the latest from gitlab source
git fetch upstream
git merge upstream/master
