#!/usr/bin/env tdaq_python
# Andre dos Anjos <andre.dos.anjos@cern.ch>

"""Unit test for the Python bindings to the Configuration class."""

import unittest
import config


class Configuration(unittest.TestCase):
    """Tests if we can manipulate ConfigurationWrap objects as expected."""

    def test01_CanCreateDB(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        db.commit()

    def test01a_DoesNotCrashIfDBNotThere(self):
        self.assertRaises(RuntimeError, config.Configuration, "test2.data.xml")

    def test02_CanReopenDB(self):
        db = config.Configuration("oksconfig:test.data.xml")
        includes = db.get_includes("test.data.xml")
        self.assertEqual(len(includes), 1)
        self.assertEqual(includes[0], "test.schema.xml")

    def test03_CanAddIncludes(self):
        db = config.Configuration("oksconfig:test.data.xml")
        db.add_include('daq/schema/core.schema.xml')
        includes = db.get_includes()
        self.assertEqual(len(includes), 2)
        db.commit()
        includes = db.get_includes("test.data.xml")
        self.assertEqual(len(includes), 2)

    def test04_CanRemoveIncludes(self):
        db = config.Configuration("oksconfig:test.data.xml")
        includes = db.get_includes()
        self.assertEqual(len(includes), 2)
        db.remove_include(includes[1])
        db.commit()
        includes = db.get_includes()
        self.assertEqual(len(includes), 1)

    def test05_CanCreateObject(self):
        db = config.Configuration("oksconfig:test.data.xml")
        for i in range(10):
            obj = db.create_obj("Dummy", "TestDummy-%d" % i)
        db.commit()

    def test05a_CanCreateObjectFromOtherObject(self):
        db = config.Configuration("oksconfig:test.data.xml")
        master = db.create_obj("Dummy", "MasterDummy")
        for i in range(100, 110):
            obj = db.create_obj("Dummy", "TestDummy-%d" % i)
        db.commit()

    def test06_CanTestForObjects(self):
        db = config.Configuration("oksconfig:test.data.xml")
        for i in range(10):
            self.assertTrue(db.test_object("Dummy", "TestDummy-%d" % i))
        for i in range(1000, 1010):
            self.assertTrue(not db.test_object("Dummy", "TestDummy-%d" % (i)))

    def test07_DetectsExistingObjects(self):
        db = config.Configuration("oksconfig:test.data.xml")
        self.assertRaises(RuntimeError, db.create_obj, "Dummy", "TestDummy-3")

    def test08_CanGetObject(self):
        db = config.Configuration("oksconfig:test.data.xml")
        obj = db.get_obj("Dummy", "TestDummy-4")

    def test08a_CanGetObjects(self):
        db = config.Configuration("oksconfig:test.data.xml")
        objs = db.get_objs("Dummy")
        self.assertEqual(len(objs), 21)

    def test09_DoesNotCrashIfNonExisting(self):
        db = config.Configuration("oksconfig:test.data.xml")
        self.assertRaises(RuntimeError, db.get_obj, "Dummy", "TestDummy-44")

    def test10_CanRemoveObject(self):
        db = config.Configuration("oksconfig:test.data.xml")
        obj = db.get_obj("Dummy", "TestDummy-1")
        db.destroy_obj(obj)
        db.commit()
        self.assertRaises(RuntimeError, db.get_obj, "Dummy", "TestDummy-1")

    def test11_CanCycleThroughStates(self):
        db = config.Configuration("oksconfig:test.data.xml")
        self.assertTrue(db.loaded())
        db.unload()
        self.assertTrue(not db.loaded())
        db.load("test.data.xml")
        self.assertTrue(db.loaded())
        db.unload()
        self.assertTrue(not db.loaded())

    def test12_CanStuffThousands(self):
        amount = 10000
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        for i in range(amount):
            db.create_obj("Dummy", "TestDummy-%d" % i)
        db.commit()

    def test13_CanCommitDeepRelations(self):
        import sys
        sys.setrecursionlimit(10000)

        depth = 10000
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        previous = None
        for i in range(depth):
            obj = db.create_obj("Second", "Object-%d" % i)
            if previous:
                obj['Another'] = previous
            previous = obj
        db.commit()

    def test14_CanRetrieveDeepRelations(self):
        # we test if one can leave the rlevel in gets() to "0" and that works
        import sys
        sys.setrecursionlimit(10000)

        depth = 10000
        db = config.Configuration('oksconfig:test.data.xml')
        # gets the topmost obj.
        obj = db.get_obj('Second', 'Object-%d' % (depth-1))
        counter = 1
        while obj['Another']:
            counter += 1
            obj = obj['Another']  # go deep in the relationship
        self.assertEqual(counter, depth)

    def test15_CommitWithComment(self):
        db = config.Configuration()
        dbfile = 'testcomment.data.xml'
        db.create_db(dbfile, ['test.schema.xml'])
        comment = "My test comment"
        db.commit(comment)
        del db
        with open(dbfile) as textfile:
            line = [l for l in textfile if ' <comment ' in l][0]

        element = line.splitlines()[0]
        self.assertTrue(comment in element)


if __name__ == "__main__":
    import sys
    sys.argv.append('-v')
    unittest.main()
