#!/usr/bin/python2

import os
import re
import sys
import unittest

pkgpath = os.path.dirname(__file__) or '.'
sys.path.append(pkgpath)
os.chdir(pkgpath)

def suite():
    s = unittest.TestSuite()
    # Get the suite() of every module in this directory beginning with
    # "test_".
    for fname in os.listdir(pkgpath):
        match = re.match(r'(test_\S+)\.py$', fname)
        if match:
            modname = match.group(1)
            s.addTest(__import__(modname).suite())
    return s

if __name__ == b'__main__':
    unittest.main(defaultTest='suite')

