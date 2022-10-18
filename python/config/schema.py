#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Wed 24 Oct 2007 05:22:49 PM CEST

"""A set of utilities to simplify OKS instrospection.
"""
import sys
import re
import logging
from . import ConfigObject

# all supported OKS types are described here
oks_types = {}
oks_types['bool'] = ['bool']
oks_types['integer'] = ['s8', 'u8', 's16', 'u16', 's32']
oks_types['long'] = ['u32', 's64', 'u64']
oks_types['float'] = ['float', 'double']
oks_types['int-number'] = oks_types['integer'] + oks_types['long']
oks_types['number'] = \
    oks_types['long'] + oks_types['integer'] + oks_types['float']
oks_types['time'] = ['date', 'time']
oks_types['string'] = oks_types['time'] + ['string', 'uid', 'enum', 'class']

range_regexp = re.compile(
    r'(?P<s1>-?0?x?[\da-fA-F]+(\.\d+)?)-(?P<s2>-?0?x?[\da-fA-F]+(\.\d+)?)')


def decode_range(s):
    """Decodes a range string representation, returns a tuple with 2 values.

    This is the supported format in regexp representation:
    '([-0x]*\\d+)\\D+-?\\d+'
    """
    if s.find('..') != -1:
        # print 'range: %s => %s' % (s, s.split('..'))
        return s.split('..')
    k = range_regexp.match(s)
    if k:
        # print 'range: %s => %s' % (s, (k.group('s1'), k.group('s2')))
        return (k.group('s1'), k.group('s2'))
    # print 'value: %s' % s
    return s


def str2integer(v, t, max):
    """Converts a value v to integer, irrespectively of its formatting.

    If the number starts with a '0', we convert it using an octal
    representation. Else, we try a decimal conversion. If any of these fail,
    we try an hexa conversion before throwing a ValueError.

    Keyword arguments:

    v -- the value to be converted
    t -- the python type (int or float) to use in the conversion
    """
    if isinstance(v, t):
        return v
    if not v:
        return v
    if isinstance(v, tuple) or isinstance(v, list):
        return [str2integer(k, t) for k in v]
    if isinstance(v, str):
        if v[0] == '*':
            return max
        elif v[0] == '0':
            try:
                return t(v, 8)
            except ValueError:
                return t(v, 16)
        else:
            try:
                return t(v)
            except ValueError:
                return t(v, 16)
    else:
        return t(v)


def to_int(v): return str2integer(v, int, sys.maxsize)


def to_long(v): return str2integer(v, int, 0xffffffffffffffff)


def check_range(v, range, range_re, pytype):
    """Checks the range of the value 'v' to make sure it is inside."""
    if isinstance(v, list):
        for k in v:
            check_range(k, range, range_re, pytype)
        return

    in_range = False
    if type(v) == str and range_re is not None:
        if range_re.match(v):
            in_range = True
    else:
        for k in range:
            if isinstance(k, tuple):
                if v >= k[0] and v <= k[1]:
                    in_range = True
            else:
                if v == k:
                    in_range = True

    if not in_range:
        raise ValueError('Value %s is not in range %s' % (v, range))


def check_relation(v, rel):
    """Checks the value v against the relationship parameters in 'rel'."""
    from config.dal import DalBase

    if not isinstance(v, DalBase):
        raise ValueError('Relationships should be DAL objects, but %s is not' %
                         repr(v))

    # check type
    if rel['type'] not in v.oksTypes():
        raise ValueError('Object %s is not of type or subtype %s' %
                         (repr(v), rel['type']))


def check_cardinality(v, prop):
    """Checks the cardinality of a certain attribute or relationship."""

    # check cardinality
    if prop['multivalue'] and not isinstance(v, list):
        raise ValueError('Multivalued properties must be python lists')
    elif not prop['multivalue'] and isinstance(v, list):
        raise ValueError(
            'Single valued properties cannot be set with python lists')


def coerce(v, attr):
    """Coerces the input value 'v' in the way the attribute expects."""

    # coerce if necessary
    if type(v) != attr['python_class']:
        # coerce in this case
        if attr['python_class'] in [int, int]:

            if isinstance(v, str):
                if len(v) == 0:
                    raise ValueError(
                        'Integer number cannot be assigned an empty string')
                if v[0] == '0':
                    try:
                        return attr['python_class'](v, 8)
                    except ValueError:
                        return attr['python_class'](v, 16)
                else:
                    try:
                        return attr['python_class'](v)
                    except ValueError:
                        return attr['python_class'](v, 16)
            else:
                v = attr['python_class'](v)

        elif attr['python_class'] is bool:
            if isinstance(v, str):
                if v in ['0', 'false']:
                    v = False
                else:
                    v = True
            else:
                v = attr['python_class'](v)

        else:
            v = attr['python_class'](v)

    # check the range of each item, if a range was set
    if attr['range']:
        check_range(v, attr['range'], attr['range_re'], attr['python_class'])

    # for special types, do a special check
    if attr['type'] == 'class':
        # class references must exist in the DAL the time I set it
        from .dal import __dal__
        check_range(v, list(__dal__.keys()),
                    attr['range_re'], attr['python_class'])

    elif attr['type'] == 'date':
        import time
        try:
            time.strptime(v, '%Y-%m-%d')
        except ValueError as e:
            try:
                time.strptime(v, '%d/%m/%y')
            except ValueError as e:
                try:
                    time.strptime(v, '%Y-%b-%d')
                except ValueError as e:
                    raise ValueError(
                        'Date types should have the format '
                        'dd/mm/yy or yyyy-mm-dd or yyyy-mon-dd: %s' % v)

    elif attr['type'] == 'time':
        import time
        try:
            time.strptime(v, '%Y-%m-%d %H:%M:%S')
        except ValueError as e:
            try:
                time.strptime(v, '%d/%m/%y %H:%M:%S')
            except ValueError as e:
                try:
                    time.strptime(v, '%Y-%b-%d %H:%M:%S')
                except ValueError as e:
                    raise ValueError(
                        'Time types should have the format dd/mm/yy HH:MM:SS '
                        'or yyyy-mm-dd HH:MM:SS or yyyy-mon-dd HH:MM:SS:'
                        + str(e))

    return v


def map_coercion(class_name, schema):
    """Given a schema of a class, maps coercion functions from libpyconfig."""

    cls = ConfigObject.ConfigObject

    schema['mapping'] = {}

    for k, v in list(schema['attribute'].items()):
        typename = v['type']
        getname = v['type']
        if getname in oks_types['string']:
            getname = 'string'
        if v['multivalue']:
            typename += '_vec'
            getname += '_vec'
            v['co_get_method'] = getattr(cls, 'get_' + getname)
            v['co_set_method'] = getattr(cls, 'set_' + typename)

        else:
            v['co_get_method'] = getattr(cls, 'get_' + getname)
            v['co_set_method'] = getattr(cls, 'set_' + typename)

        if v['type'] in oks_types['string']:
            v['python_class'] = str
        elif v['type'] in oks_types['bool']:
            v['python_class'] = bool
        elif v['type'] in oks_types['integer']:
            v['python_class'] = to_int
        elif v['type'] in oks_types['long']:
            v['python_class'] = to_long
        elif v['type'] in oks_types['float']:
            v['python_class'] = float

        # split and coerce ranges
        v['range_re'] = None
        if v['range']:
            if v['type'] == 'string':
                v['range_re'] = re.compile(v['range'])
            else:
                v['range'] = [decode_range(j) for j in v['range'].split(',')]
                for j in range(len(v['range'])):
                    if isinstance(v['range'][j], str):
                        v['range'][j] = v['python_class'](v['range'][j])
                    elif len(v['range'][j]) == 1:
                        v['range'][j] = v['python_class'](v['range'][j][0])
                    else:  # len(v['range'][j]) == 2 (the only other case)
                        v['range'][j] = (v['python_class'](v['range'][j][0]),
                                         v['python_class'](v['range'][j][1]))

        # integer numbers have implicit ranges and it is better to check
        elif v['type'] in oks_types['int-number']:
            if v['type'] == 's8':
                v['range'] = [(-2**7, (2**7)-1)]
            if v['type'] == 'u8':
                v['range'] = [(0, (2**8)-1)]
            if v['type'] == 's16':
                v['range'] = [(-2**15, (2**15)-1)]
            if v['type'] == 'u16':
                v['range'] = [(0, (2**16)-1)]
            if v['type'] == 's32':
                v['range'] = [(-2**31, (2**31)-1)]
            if v['type'] == 'u32':
                v['range'] = [(0, (2**32)-1)]
            if v['type'] == 's64':
                v['range'] = [(-2**63, (2**63)-1)]
            if v['type'] == 'u64':
                v['range'] = [(0, (2**64)-1)]

        # coerce initial values
        if v['init-value']:
            try:
                if v['type'] in oks_types['string']:
                    pass
                elif v['multivalue']:
                    v['init-value'] = [coerce(j, v)
                                       for j in v['init-value'].split(',')]
                else:
                    v['init-value'] = coerce(v['init-value'], v)
            except ValueError as e:
                logging.warning('Initial value of "%s.%s" could not be '
                                'coerced: %s' %
                                (class_name, k, e))

        # if the type is a date or time type, and there is not default,
        # set "now"
        if not v['init-value'] and v['type'] == 'date':
            import datetime
            v['init-value'] == datetime.date.today().isoformat()

        # if the type is a date or time type, and there is not default,
        # set "now"
        if not v['init-value'] and v['type'] == 'time':
            import datetime
            now = datetime.datetime.today()
            now.replace(microsecond=0)
            v['init-value'] == now.isoformat(sep=' ')

    for v in list(schema['relation'].values()):
        if v['multivalue']:
            v['co_get_method'] = getattr(cls, 'get_objs')
            v['co_set_method'] = getattr(cls, 'set_objs')

        else:
            v['co_get_method'] = getattr(cls, 'get_obj')
            v['co_set_method'] = getattr(cls, 'set_obj')

    return schema


class Cache(object):
    """Defines a cache for all known schemas at a certain time.
    """

    def __init__(self, config, all=True):
        """Initializes the cache with information from the Configuration
        object.

        This method will browse for all declared classes in the Configuration
        object given as input and will setup the schema for all known classes.
        After this you can still update the cache using the update() method.

        Keyword parameters:

        config -- The config.Configuration object to use as base for the
        current cache.

        all -- A boolean indicating if I should store all the attributes and
        relations from a certain class or just the ones directly associated
        with a class.
        """
        self.data = {}
        self.all = all
        self.update(config)

    def update(self, config):
        """Updates this cache with information from the Configuration object.

        This method will add new classes not yet know to this cache. Classes
        with existing names will not be added. No warning is generated (this
        should be done by the OKS layer in any case.
        """
        for k in config.classes():
            if k in list(self.data.keys()):
                continue
            self.data[k] = {}
            self.data[k]['attribute'] = config.attributes(k, self.all)
            self.data[k]['relation'] = config.relations(k, self.all)
            self.data[k]['superclass'] = config.superclasses(k, self.all)
            self.data[k]['subclass'] = config.subclasses(k, self.all)
            map_coercion(k, self.data[k])

    def update_dal(self, config):
        """Updates this cache with information for DAL.

        This method will add new DAL classes not yet know to this cache.
        Classes with existing DAL representations will not be touched.
        """
        from .dal import generate

        # generate
        klasses = generate(config, [self.data[k]['dal'] for k
                                    in list(self.data.keys())
                                    if 'dal' in self.data[k]])
        # associate
        for k in klasses:
            self.data[k.pyclassName()]['dal'] = k

    def __getitem__(self, key):
        """Gets the description of a certain class."""
        return self.data[key]

    def __str__(self):
        """Prints a nice display of myself"""
        return '%s: %d classes loaded' % \
            (self.__class__.__name__, len(self.data)) + '\n' + \
            str(list(self.data.keys()))
