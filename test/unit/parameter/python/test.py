"""Unit tests for parameter library"""

__author__ = "Anders Logg <logg@simula.no>"
__date__ = "2011-03-28"
__copyright__ = "Copyright (C) 2011 " + __author__
__license__  = "GNU LGPL Version 2.1"

import unittest
from dolfin import *

class InputOutput(unittest.TestCase):

    def test_simple(self):

        # Create some parameters
        p0 = Parameters("test")
        p0.add("filename", "foo.txt")
        p0.add("maxiter", 100)
        p0.add("tolerance", 0.001)
        p0.add("monitor_convergence", True)

        # Save to file
        if MPI.process_number() == 0:
            f0 = File("test_parameters.xml")
            f0 << p0
        MPI.barrier()

        # Read from file
        p1 = Parameters()
        f1 = File("test_parameters.xml")
        f1 >> p1

        # Check values
        self.assertEqual(p1.name(), "test")
        self.assertEqual(p1["filename"], "foo.txt")
        self.assertEqual(p1["maxiter"], 100)
        self.assertEqual(p1["tolerance"], 0.001)
        self.assertEqual(p1["monitor_convergence"], True)

    def test_nested(self):

        # Create some nested parameters
        p0 = Parameters("test")
        p00 = Parameters("sub0")
        p00.add("filename", "foo.txt")
        p00.add("maxiter", 100)
        p00.add("tolerance", 0.001)
        p00.add("monitor_convergence", True)
        p0.add("foo", "bar")
        p01 = Parameters(p00);
        p01.rename("sub1");
        p0.add(p00)
        p0.add(p01)

        # Save to file
        if MPI.process_number() == 0:
            f0 = File("test_parameters.xml")
            f0 << p0
        MPI.barrier()

        # Read from file
        p1 = Parameters()
        f1 = File("test_parameters.xml")
        f1 >> p1

        # Check values
        self.assertEqual(p1.name(), "test")
        self.assertEqual(p1["foo"], "bar")
        self.assertEqual(p1["sub0"]["filename"], "foo.txt")
        self.assertEqual(p1["sub0"]["maxiter"], 100)
        self.assertEqual(p1["sub0"]["tolerance"], 0.001)
        self.assertEqual(p1["sub0"]["monitor_convergence"], True)

if __name__ == "__main__":
    print ""
    print "Testing parameter library"
    print "------------------------------------------------"
    unittest.main()