#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Wed 24 Oct 2007 01:53:05 PM CEST

"""Unit test for the Python bindings to the DAL."""

import unittest
import config

# binds a new dal into the module named "test"
test = config.dal.module('test', 'test.schema.xml')

SIZE_TEST = 1000
RECURSION_TEST = 500


def test_good(test, obj, field, values):
    """Tests I can set good expected values to certain fields"""
    for v in values:
        setattr(obj, field, v)
        test.assertEqual(getattr(obj, field), v)


def test_bad(test, obj, field, values):
    """Tests I can not set bad values to certain fields"""
    for v in values:
        test.assertRaises(ValueError, setattr, obj, field, v)


class DalTest(unittest.TestCase):

    def test00_CanReadWriteBool(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'bool', [True, False, 0, 1])

    def test01_CanReadWriteInt8(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'sint8', [-2**7, 2**7-1, 0, 0xa])
        test_bad(self, obj, 'sint8', [-2**7-1, 2**7, 0xab])

    def test02_CanReadWriteUInt8(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint8', [0, 2**8-1, 42, 0xab])
        test_bad(self, obj, 'uint8', [-1, 2**8, 0xabc])

    def test03_CanReadWriteInt16(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'sint16', [-2**15, 2**15-1, 42, 0xa])
        test_bad(self, obj, 'sint8', [-2**15-1, 2**15, 0xabcd])

    def test04_CanReadWriteUInt16(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint16', [0, 2**16-1, 42, 0xabcd])
        test_bad(self, obj, 'uint16', [-1, 2**16, 0xabcde])

    def test05_CanReadWriteInt32(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'sint32', [-2**31, 2**31-1, 42, 0xa])
        test_bad(self, obj, 'sint32', [-2**31-1, 2**31, 0xabcdabcdabcd])

    def test06_CanReadWriteUInt32(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint32', [0, 2**32-1, 42, 0xabcd])
        test_bad(self, obj, 'uint32', [-1, 2**32, 0xabcdeabcdeabcde])

    def test07_CanReadWriteInt64(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'sint64', [-2**63, 2**63-1, 42, 0xa])
        test_bad(self, obj, 'sint64', [-2**63-1,
                                       2**63, 0xabcdabcdabcdabcabcabc])

    def test08_CanReadWriteUInt64(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint64', [0, 2**64-1, 42, 0xabcd])
        test_bad(self, obj, 'uint64',
                 [-1, 2**64, 0xabcdeabcdeabcdeabcabcffabc])

    def test09_CanReadWriteStringsWithRanges(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'string_vector', [
                  ['test01', 'test09'], ['test30']])
        test_bad(self, obj, 'string_vector', [['Test01', 'test111'], ['abcd']])

    def test10_CanReadWrite64bitVectors(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint64_vector', [[0, 1], [0]])
        test_bad(self, obj, 'uint64_vector', [[0, -1], [2**65]])

    def test11_CanReadWrite64bitVectorsAndWithRange(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'uint64_vector_range', [[18446744073709551600,
                                                      18446744073709551615],
                                                     [18446744073709551610]])
        test_bad(self, obj, 'uint64_vector_range', [[0, -1], 10])

    def test12_CanReadWriteEnum(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'enum', ['FIRST', 'SECOND'])
        test_bad(self, obj, 'string_vector', ['first', 'zeroth'])

    def test13_CanReadWriteDate(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'date', ['31/12/73', '01/01/70'])
        test_bad(self, obj, 'date', ['31/00/19', '-1'])

    def test14_CanReadWriteTime(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'time', [
                  '31/12/73 02:03:04', '01/01/70 00:00:59'])
        test_bad(self, obj, 'time', ['31/12/73 24:23:22', '-1'])

    def test15_CanReadWriteClassReference(self):
        obj = test.Dummy('Test-1')
        test_good(self, obj, 'classref', ['Dummy', 'Third'])
        test_bad(self, obj, 'classref', ['DoesNotExistInDal'])

    def test16_CanNotSetUnexistingAttribute(self):
        obj = test.Dummy('Test-1')
        self.assertRaises(AttributeError, setattr, obj, 'doesNotExist', 24)

    def test17_AttributeInheritance(self):
        obj = test.Third('Test-1')
        test_good(self, obj, 'classref', ['Second', 'Third', 'Dummy'])
        test_bad(self, obj, 'classref', ['DoesNotExistInDal'])

    def test18_CanCompareDals(self):
        obj1 = test.Dummy('Test-1')
        obj2 = test.Dummy('Test-1')
        self.assertEqual(obj1, obj2)
        obj3 = test.Second('Test-1')
        self.assertNotEqual(obj1, obj3)
        obj4 = test.Dummy('Test-2')
        self.assertNotEqual(obj1, obj4)

    def test19_CanReadWriteSingleValuedRelations(self):
        obj1 = test.Second('Test-1')
        obj2 = test.Third('Test-2')
        obj3 = test.Dummy('Test-3')
        test_good(self, obj2, 'Single', [None, obj1])
        test_bad(self, obj2, 'Single', [obj3, 4])

    def test20_CanReadWriteMultiValuedRelations(self):
        obj1 = test.Second('Test-1')
        obj2 = test.Third('Test-2')
        obj3 = test.Dummy('Test-3')
        test_good(self, obj1, 'Dummy', [[], [obj3, obj2], [obj2]])
        test_bad(self, obj2, 'Seconds', [[obj1, obj3]])

    def test21_CanAttributeFromConstructor(self):
        obj = test.Dummy('Test-1', string_vector=['test10', 'test20'])
        self.assertEqual(obj.string_vector, ['test10', 'test20'])
        obj2 = test.Second('Test-2', string_vector=['test20', 'test30'],
                           Dummy=[obj])
        self.assertEqual(obj2.Dummy, [obj])

    def test22_CanReadWriteDalOnConfiguration(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        obj3 = test.Dummy('Test-3', string_vector=['test05', 'test06'])
        obj1 = test.Second('Test-1', Dummy=[obj3], string_vector=['test30'])
        obj2 = test.Third('Test-2', string_vector=['test10'], Seconds=[obj1])
        db.update_dal(obj2, recurse=True)  # should serialize everybody
        db.commit()
        ret = db.get_dal('Third', 'Test-2')
        self.assertEqual(ret, obj2)
        self.assertEqual(ret.Seconds[0], obj1)
        self.assertEqual(ret.Seconds[0].Dummy[0], obj3)

    def test23_CanSearchForDalsAtConfiguration(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        obj3 = test.Dummy('Test-3', string_vector=['test05', 'test06'])
        obj1 = test.Second('Test-1', Dummy=[obj3], string_vector=['test30'])
        obj2 = test.Third('Test-2', string_vector=['test10'], Seconds=[obj1])
        db.update_dal(obj2, recurse=True)  # should serialize everybody
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        ret = db.get_dal('Third', 'Test-2')
        self.assertEqual(ret, obj2)
        self.assertEqual(ret.Seconds[0], obj1)
        self.assertEqual(ret.Seconds[0].Dummy[0], obj3)
        self.assertEqual(len(db.get_dals('Dummy')), 3)

    def test24_CanReadWriteBigDatabases(self):
        import sys
        number = SIZE_TEST
        previous = None
        objs = []
        for i in range(number):
            objs.append(test.Second("Object-%d" % i))
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        for k in objs:
            db.update_dal(k, recurse=True)
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        objs = db.get_dals('Second')
        self.assertEqual(len(objs), number)

    def test25_CanRecurseALot(self):
        import sys
        sys.setrecursionlimit(50000)
        depth = RECURSION_TEST  # 10000 seems to be impossible!
        previous = None
        for i in range(depth):
            obj = test.Second("Object-%d" % i)
            if previous:
                obj.Another = previous
            previous = obj
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        db.update_dal(previous, recurse=True)
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        top = db.get_dal('Second', 'Object-%d' % (depth-1))
        counter = 0
        while top:
            counter += 1
            top = top.Another
        self.assertEqual(counter, depth)

    def test26_CanModifyDatabases(self):
        import sys
        sys.setrecursionlimit(50000)
        depth = RECURSION_TEST
        db = config.Configuration('oksconfig:test.data.xml')
        top = db.get_dal('Second', 'Object-%d' % (depth-1))
        all = [top]
        while top.Another:
            all.append(top.Another)
            top = top.Another
        self.assertEqual(len(all), depth)

        # reverse the linking order and update the database
        all = list(reversed(all))
        for i, k in enumerate(all):
            if i < (len(all)-1):
                k.Another = all[i+1]
            else:
                k.Another = None

        # update the opened database
        db.update_dal(all[0], recurse=True)
        db.commit()
        del db

        # reopen and check
        db = config.Configuration('oksconfig:test.data.xml')
        top = db.get_dal('Second', 'Object-0')  # that should be on the top now
        counter = 0
        while top:
            counter += 1
            top = top.Another
        self.assertEqual(counter, depth)

    def test27_CanSearchUsingBaseClasses(self):
        # reopen and check
        db = config.Configuration('oksconfig:test.data.xml')
        self.assertEqual(len(db.get_dals('Dummy')), RECURSION_TEST)

    def test28_CanHandleInheritanceAtSearch(self):
        # reopen and check
        db = config.Configuration('oksconfig:test.data.xml')
        objs = db.get_dals('Dummy')
        self.assertEqual(len(objs), RECURSION_TEST)
        for k in objs:
            self.assertEqual(k.className(), 'Second')

    def test29_CanWriteInfiniteRecursion(self):
        obj1 = test.Second('Obj-1')
        obj2 = test.Second('Obj-2')
        obj1.Another = obj2
        obj2.Another = obj1
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        db.update_dal(obj1, recurse=True)
        db.commit()

    def test30_CanReadInfiniteRecursion(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj2 = db.get_dal('Second', 'Obj-2')
        self.assertEqual(obj1.Another, obj2)
        self.assertEqual(obj2.Another, obj1)

    def test31_CanWriteBiggerRecursion(self):
        obj1 = test.Second('Obj-1')
        obj2 = test.Second('Obj-2')
        obj3 = test.Second('Obj-3')
        obj4 = test.Second('Obj-4')
        obj1.Another = obj2
        obj2.Another = obj3
        obj3.Another = obj4
        obj4.Another = obj1
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        db.update_dal(obj1, recurse=True)
        db.commit()

    def test32_CanReadBiggerRecursion(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj2 = db.get_dal('Second', 'Obj-2')
        obj3 = db.get_dal('Second', 'Obj-3')
        obj4 = db.get_dal('Second', 'Obj-4')
        self.assertEqual(obj1.Another, obj2)
        self.assertEqual(obj2.Another, obj3)
        self.assertEqual(obj3.Another, obj4)
        self.assertEqual(obj4.Another, obj1)

    def test33_CanHandleDalDestruction(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj2 = db.get_dal('Second', 'Obj-2')
        obj3 = db.get_dal('Second', 'Obj-3')
        obj4 = db.get_dal('Second', 'Obj-4')
        obj2.Another = None
        # have to first eliminate ALL references
        db.update_dal(obj2, recurse=True)
        db.destroy_dal(obj3)
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj2 = db.get_dal('Second', 'Obj-2')
        self.assertRaises(RuntimeError, db.get_dal, 'Second', 'Obj-3')
        obj4 = db.get_dal('Second', 'Obj-4')

    def test34_CanModifyNonRecursively(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj1.Another = None
        obj2 = db.get_dal('Second', 'Obj-2')
        obj2.string = 'xyzabc'
        db.update_dal(obj1, recurse=False)
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        self.assertNotEqual(obj1.Another, obj2)
        self.assertEqual(obj1.Another, None)
        obj2 = db.get_dal('Second', 'Obj-2')
        self.assertNotEqual(obj2.string, 'xyzabc')

    def test35_CannotMoveIfNotIncluded(self):
        obj11 = test.Second('Obj-11')
        obj22 = test.Second('Obj-22')
        obj33 = test.Second('Obj-33')
        obj44 = test.Second('Obj-44')
        obj11.Another = obj22
        obj22.Another = obj33
        obj33.Another = obj44
        obj44.Another = obj11
        db = config.Configuration()
        db.create_db("test2.data.xml", ['test.schema.xml'])
        db.update_dal(obj11, recurse=True)
        db.commit()
        del db

        db = config.Configuration('oksconfig:test.data.xml')
        obj1 = db.get_dal('Second', 'Obj-1')
        obj1.Another = obj22
        self.assertRaises(RuntimeError, db.update_dal, obj1, recurse=False)
        db.add_include('test2.data.xml')
        db.update_dal(obj1, recurse=False)
        self.assertEqual(True, obj22 in db.get_all_dals())
        db.commit()
        del db

    def test36_CanSearchForObjects(self):
        db = config.Configuration()
        db.create_db("test.data.xml", ['test.schema.xml'])
        obj3 = test.Dummy('Test-3', string_vector=['test05', 'test06'])
        obj1 = test.Second('Test-1', Dummy=[obj3], string_vector=['test30'])
        obj2 = test.Third('Test-2', string_vector=['test10'], Seconds=[obj1])
        db.update_dal(obj2, recurse=True)  # should serialize everybody
        db.commit()
        del db
        db = config.Configuration('oksconfig:test.data.xml')
        ret = db.get_dal('Third', 'Test-2')

        import re
        self.assertEqual(ret.get('Second', 'Test-1'), obj1)
        self.assertEqual(type(ret.get('Second')), list)
        self.assertEqual(
            type(ret.get('Second', re.compile(r'Test-[0-9]'))), list)
        self.assertEqual(len(ret.get('Second', re.compile(r'Test-[0-9]'))), 1)
        self.assertEqual(
            len(ret.get('Second', re.compile(r'Test-[0-9]'), True)), 2)

    def test37_BookkeepUpdatedObjects(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj = db.get_dal('Dummy', 'Test-3')
        config.reset_updated_dals()
        self.assertEqual(len(config.updated_dals()), 0)
        obj.bool = False
        self.assertEqual(len(config.updated_dals()), 1)
        self.assertEqual(config.updated_dals(), set((obj,)))
        obj.sint8 = 0
        self.assertEqual(len(config.updated_dals()), 1)

        obj1 = db.get_dal('Second', 'Test-1')
        self.assertEqual(len(config.updated_dals()), 1)

        obj1.bool = False
        self.assertEqual(len(config.updated_dals()), 2)
        self.assertEqual(config.updated_dals(), set((obj, obj1)))

    def test38_CanRename(self):
        db = config.Configuration('oksconfig:test.data.xml')
        obj = db.get_dal('Dummy', 'Test-3')
        obj.rename('Test-4')
        db.update_dal(obj)
        obj2 = db.get_dal('Dummy', 'Test-4')
        self.assertEqual(obj, obj2)
        db.commit()


if __name__ == "__main__":
    import sys
    sys.argv.append('-v')
    unittest.main()
