#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Wed 24 Oct 2007 01:53:05 PM CEST

"""Unit test for the Python bindings to the ConfigObject class."""

import unittest
import config


def limit_test(obj, attrname, min, max):
    db = config.Configuration()
    filename = 'test.data.xml'
    db.create_db(filename, ['test.schema.xml'])

    low = db.create_obj("Dummy", "TestDummy-Low")
    low[attrname] = min
    obj.assertEqual(low[attrname], min)

    high = db.create_obj("Dummy", "TestDummy-High")
    high[attrname] = max
    obj.assertEqual(high[attrname], max)

    db.commit()


def limit_test_raise(obj, attrname, min, max):
    db = config.Configuration()
    filename = 'overflow_test.data.xml'
    db.create_db(filename, ['test.schema.xml'])

    low = db.create_obj("Dummy", "TestDummy-Underflow")
    obj.assertRaises(OverflowError, low.__setitem__, attrname, min-1)

    high = db.create_obj("Dummy", "TestDummy-Overflow")
    obj.assertRaises(OverflowError, high.__setitem__, attrname, max+1)

    db.commit()
    import os
    os.unlink(filename)


class ConfigObject(unittest.TestCase):

    def test00_CanReadWriteBool(self):
        limit_test(self, 'bool', False, True)

    def test01_CanReadWriteInt8(self):
        limit_test(self, 'sint8', -2**7, (2**7)-1)
        limit_test_raise(self, 'sint8', -2**7, (2**7)-1)

    def test02_CanReadWriteUInt8(self):
        limit_test(self, 'uint8', 0, (2**8)-1)
        limit_test_raise(self, 'uint8', 0, (2**8)-1)

    def test03_CanReadWriteInt16(self):
        limit_test(self, 'sint16', -2**15, (2**15)-1)
        limit_test_raise(self, 'sint16', -2**15, (2**15)-1)

    def test04_CanReadWriteUInt16(self):
        limit_test(self, 'uint16', 0, (2**16)-1)
        limit_test_raise(self, 'uint16', 0, (2**16)-1)

    def test05_CanReadWriteInt32(self):
        limit_test(self, 'sint32', -2**31, (2**31)-1)
        limit_test_raise(self, 'sint32', -2**31, (2**31)-1)

    def test06_CanReadWriteUInt32(self):
        limit_test(self, 'uint32', 0, (2**32)-1)
        # The next test actually will fail on 32-bit machines if we set the
        # lower limit to 0. It works OK in other platforms. The bug is related
        # to the function PyLong_AsUnsignedLongLong() and variants that convert
        # '-1L' to the highest possible 32-bit unsigned integer not raising an
        # negative OverflowError as in the other convertions above.
        # limit_test_raise(self, 'uint32', 0, (2**32)-1)

    def test07_CanReadWriteInt64(self):
        limit_test(self, 'sint64', -2**63, (2**63)-1)
        limit_test_raise(self, 'sint64', -2**63, (2**63)-1)

    def test08_CanReadWriteUInt64(self):
        limit_test(self, 'uint64', 0, (2**64)-1)
        # The next test actually will fail on 64-bit machines if we set the
        # lower limit to 0. It works OK in other platforms. The bug is related
        # to the function PyLong_AsUnsignedLongLong() and variants that convert
        # '-1L' to the highest possible 64-bit unsigned integer not raising an
        # negative OverflowError as in the other convertions above.
        # limit_test_raise(self, 'uint64', 0, (2**64)-1)

    def test08a_CanReadWriteUInt64WithRange(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        attrname = 'uint64_vector_range'
        min = 2**64 - 16
        max = 2**64 - 1
        val = [min, max]
        t[attrname] = val
        self.assertEqual(t[attrname], val)
        self.assertRaises(ValueError, t.__setitem__, attrname, [0, 1, 2])
        db.commit()

    def test09_CanReadWriteString(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = 'bla'
        attrname = 'string'
        t[attrname] = val
        self.assertEqual(t[attrname], val)
        db.commit()

    def test10_CanReadWriteEnum(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = 'FIRST'
        attrname = 'enum'
        t[attrname] = val
        self.assertEqual(t[attrname], val)
        val = 'DOES NOT EXIST'
        self.assertRaises(ValueError, t.__setitem__, attrname, val)

    def test11_CanReadWriteStringList(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = ["test10", "test20"]
        attrname = 'string_vector'
        t[attrname] = val
        self.assertEqual(t[attrname], val)

    def test12_CanReadWriteDate(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = "2000-Jan-01"
        attrname = 'date'
        t[attrname] = val
        self.assertEqual(t[attrname], val)

    def test13_CanReadWriteTime(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = "2000-Jan-01 00:00:00"
        attrname = 'time'
        t[attrname] = val
        self.assertEqual(t[attrname], val)

    def test14_CanReadWriteClassReference(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj("Dummy", "TestDummy-1")
        val = 'Second'
        attrname = 'classref'
        t[attrname] = val
        self.assertEqual(t[attrname], val)
        self.assertRaises(ValueError, t.__setitem__, attrname, 'DoesNotExist')
        db.commit()

    def test15_CanReadWriteSingleValuedRelations(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        relation = db.create_obj("Dummy", "Single-Relation")
        t = db.create_obj('Third', 'Originator')
        attrname = 'Another'
        self.assertEqual(t[attrname], None)  # problem
        t[attrname] = relation
        self.assertEqual(t[attrname], relation)
        db.commit()

    def test16_CanReadWriteMultiValuedRelations(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        relations = []
        for i in range(10):
            relations.append(db.create_obj("Second", "Relation-%d" % i))
        t = db.create_obj('Third', 'Originator')
        attrname = 'Seconds'
        t[attrname] = relations
        self.assertEqual(t[attrname], relations)
        db.commit()

    def test17_CanRename(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        t = db.create_obj('Dummy', 'TestDummy-1')
        t.rename('TestDummy-2')
        self.assertEqual(t.UID(), 'TestDummy-2')
        t2 = db.get_obj('Dummy', 'TestDummy-2')
        self.assertEqual(t, t2)
        db.commit()


if __name__ == "__main__":
    import sys
    sys.argv.append('-v')
    unittest.main()
