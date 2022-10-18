"""Contains the base class for DAL types and auxiliary methods.

This module defines the PyDALBase class that is used as the base type for all
DAL classes. A few utilities are also available.
"""
# This variable holds the global list of DAL classes ever generated for the
# current python session. This speeds up generation and makes python resolve
# types in the correct, expected way.
__dal__ = {}

# Homogeneous comparison between strings and regular expressions


def __strcmp__(v1, v2):
    return v1 == v2


def __recmp__(pat, v):
    return pat.match(v) is not None


def prettyprint_cardinality(not_null, multivalue):
    """Returns a nice string representation for an object cardinality"""
    if not_null:
        if multivalue:
            return '1..*'
        else:
            return '1..1'

    else:
        if multivalue:
            return '0..*'
        else:
            return '0..1'


def prettyprint_range(attr):
    """Prints the range of an attribute in a nice way"""
    to_print = list(attr['range'])  # copy
    for k in range(len(to_print)):
        if isinstance(to_print[k], tuple):
            to_print[k] = '..'.join([str(i) for i in to_print[k]])
    return ', '.join([str(i) for i in to_print])


def prettyprint_doc(entry):
    """Pretty prints a schema Cache entry, to be used by __doc__ strings"""
    from .schema import oks_types

    akeys = list(entry['attribute'].keys())
    akeys.sort()
    retval = '    Attributes:'
    if len(akeys) == 0:
        retval += ' None'
    retval += '\n'
    for k in akeys:
        retval += '      - "' + k + '": ' + \
            entry['attribute'][k]['description'].strip() + '\n'
        retval += '          oks-type: ' + entry['attribute'][k]['type'] + '\n'
        retval += '          cardinality: ' + \
            prettyprint_cardinality(entry['attribute'][k]['not-null'],
                                    entry['attribute'][k]['multivalue']) + '\n'
        if entry['attribute'][k]['range']:
            retval += '          range: ' + \
                prettyprint_range(entry['attribute'][k]) + '\n'
        if entry['attribute'][k]['init-value']:
            retval += '          initial value: ' + \
                str(entry['attribute'][k]['init-value']) + '\n'

    rkeys = list(entry['relation'].keys())
    rkeys.sort()
    retval += '\n    Relationships:'
    if len(rkeys) == 0:
        retval += ' None'
    retval += '\n'
    for k in rkeys:
        retval += '      - "' + k + '": ' + \
            entry['relation'][k]['description'].strip() + '\n'
        retval += '          oks-class: ' + entry['relation'][k]['type'] + '\n'
        retval += '          cardinality: ' + \
            prettyprint_cardinality(entry['relation'][k]['not-null'],
                                    entry['relation'][k]['multivalue']) + '\n'
        retval += '          aggregated: ' + \
            str(entry['relation'][k]['aggregation']) + '\n'
    return retval[:-1]


class DalBase(object):
    """This class is used to represent any DAL object in the system. """

    # This will keep track of the dal objects that gets updated
    _updated = set()

    @staticmethod
    def updated():
        """Returns a set of DAL objects that were modified in this DB session
        """
        return set(DalBase._updated)

    @staticmethod
    def reset_updated_list():
        """Reset the set keeping track of modified DAL objects
        """
        DalBase._updated.clear()

    def __init__(self, id, **kwargs):
        """Constructs an object by setting its id (UID in OKS jargon) at least.

        This method will initialize an object of the DalBase type, by setting
        its internal properties (with schema cross-checking where it is
        possible). The user should at least set the object's id, which at this
        moment is not checked for uniqueness.

        Keyword arguments:

        id -- This is the unique identifier (per database) that the user wants
        to assign to this object. This identifier will be used as the OKS
        identifier when and if this object is ever serialized in an OKS
        database.

        **kwargs -- This is a set of attributes and relationships that must
        exist in the associated DAL class that inherits from this base.
        """
        from . import dalproperty
        prop = property(dalproperty._return_attribute('id', self, id),
                        dalproperty._assign_attribute('id'))
        setattr(self.__class__, 'id', prop)
        self.__reset_identity__()

        self.__touched__ = []  # optimization
        for k, v in kwargs.items():
            setattr(self, k, v)

    def __reset_identity__(self):
        self.__fullname__ = '%s@%s' % (self.id, self.className())
        self.__hashvalue__ = hash(self.__fullname__)

    def className(self):
        return self.__class__.pyclassName()

    def isDalType(self, val):
        cmp = __strcmp__
        if hasattr(val, 'match'):
            cmp = __recmp__
        return True in [cmp(val, k) for k in self.__class__.__okstypes__]

    def oksTypes(self):
        return self.__class__.pyoksTypes()

    def fullName(self):
        return self.__fullname__

    def copy(self, other):
        """Copies attributes and relationships from the other component.

        This will copy whatever relevant attributes and relationships from
        another component into myself. The implemented algorithm starts by
        iterating on my own schema and looking for the counter part on the
        other class's schema, only matching values are copied. This is useful
        to copy values from base class objects or templated class objects.

        Arguments:

        other -- This is the other dal object you are trying to copy.
        """
        for k, v in self.__schema__['attribute'].items():
            if k not in other.__schema__['attribute']:
                continue
            setattr(self, k, getattr(other, k))
        for k, v in self.__schema__['relation'].items():
            if k not in other.__schema__['relation']:
                continue
            obj = getattr(other, k)
            try:
                setattr(self, k, list(obj))
            except TypeError:
                setattr(self, k, obj)

    def rename(self, new_name):
        """
        Rename the DAL object to a new name.

        This will store the old name in a hidden attribute and when
        Configuration.update_dal() is called we use it to check
        if there is an existing object with that name in the database.

        If yes, we call the underlying ConfigObject.rename() method
        transparently. If the old name does not exist in the database,
        nothing special is done.

        This is the only 'official' way to rename an object on the
        DAL level. Just changing the 'id' attribute will not have
        the same effect.
        """
        if self.id == new_name:
            return

        if not hasattr(self, '__old_id'):
            setattr(self, '__old_id', getattr(self, 'id'))

        self.id = new_name

    def __repr__(self):
        """Returns a nice representation of this object."""
        return "<%s>" % (self.__fullname__)

    def __str__(self):
        """Returns human readable information about the object."""

        retval = "%s(id='%s'" % (self.className(), self.id)

        for a, v in self.__schema__['attribute'].items():
            retval += ',\n  %s = ' % a
            if hasattr(self, a):
                retval += str(getattr(self, a))
            else:
                retval += 'None'
                if v['init-value']:
                    retval += ", # defaults to '%s'" % v['init-value']
                else:
                    if v['not-null']:
                        retval += ', # MUST be set, there is not default!'

        for r, v in self.__schema__['relation'].items():
            retval += ',\n  %s = ' % r
            rel = None
            if hasattr(self, r):
                rel = getattr(self, r)

            if rel is None:
                retval += "<unset>"
            elif isinstance(rel, list):
                retval += str([repr(k) for k in getattr(self, r)])
                retval += ']'
            else:
                retval += repr(getattr(self, r))

        if retval[-2:] == ',\n':
            retval = retval[:-2]
        retval += ')'

        return retval

    def __eq__(self, other):
        """True is the 2 objects have the same class and ID."""
        return self.__hashvalue__ == hash(other)

    def __ne__(self, other):
        """True if the 2 objects *not* have the same class and ID."""
        return self.__hashvalue__ != hash(other)

    def __gt__(self, other):
        """True if the object is greater than the other alphabetically."""
        if self.className() == other.className():
            return self.id > other.id
        return self.className() > other.className()

    def __lt__(self, other):
        """True if the class is smaller than the other.  """
        if self.className() == other.className():
            return self.id < other.id
        return (self.className() < other.className())

    def __ge__(self, other):
        """True if the object is greater or equal than the other
        alphabetically.
        """
        return (self > other) or (self == other)

    def __le__(self, other):
        """Returns True if the class is smaller or equal than the other. """
        return (self < other) or (self == other)

    def __hash__(self):
        """This method is meant to be used to allow DAL objects as map keys."""
        return self.__hashvalue__

    def __getall__(self, comp=None):
        """Get all relations, includding a link to myself"""
        top = False
        if not comp:
            top = True
            comp = {}

        if self.__fullname__ in comp:
            return

        comp[self.__fullname__] = self
        for r in list(self.__schema__['relation'].keys()):

            if not getattr(self, r):
                continue

            if isinstance(getattr(self, r), list):
                for k in getattr(self, r):
                    k.__getall__(comp)
            else:
                getattr(self, r).__getall__(comp)

        if top:
            return comp

    def get(self, className, idVal=None, lookBaseClasses=False):
        """Get components in the object based on class name and/or id.

        This method runs trough the components of its relationships and
        returns a sorted list (sorting based on class name and object ID)
        containing references to all components that match the search criteria.

        Keyword Parameters (may be named):

        className -- The name of the class to look for. Should be a string

        idVal -- The id of the object to look for. If not set (or set to None),
        the search will be based only on the class name. If set, it must be
        either a string or an object that defines a match() method (such as a
        regular expression).

        lookBaseClasses -- If True and parameter to be search is a class, the
        method will look also through the base classes names, so if value =
        Application, for instance the method will return all objects of class
        Application or that inherit from the Application class.

        Returns a list with all the components that matched the search
        criteria, if idVal is not set or is a type that defines a match()
        method such as a regular expression. Otherwise (if it is a string)
        returns a single object, if any is found following the criterias for
        className and a exact idVal match.
        """
        retval = []

        cmp_class = __strcmp__
        if hasattr(className, 'match'):
            cmp_class = __recmp__

        for v in self.__getall__().values():
            if cmp_class(className, v.__class__.__name__) or \
                    (lookBaseClasses and v.isDalType(className)):
                if idVal:
                    if type(idVal) == str:
                        if idVal == v.id:
                            return v
                        else:
                            continue

                    # if idVal is set and is not a string,
                    # we just go brute force...
                    if idVal.match(v.id):
                        retval.append(v)
                else:
                    # if idVal is not set and we are sure the class matched,
                    # just append
                    retval.append(v)

        if isinstance(idVal, str):
            raise KeyError('Did not find %s@%s under %s' %
                           (idVal, className, self.fullName()))
        return retval

    def __getattr__(self, par):
        """Returns a given attribute or relationship.

        This method returns an attribute or relationship from the current
        object, or throws an AttributeError if no such thing exists. It sets
        the field touched, so it does not get called twice.
        """

        if par in self.__schema__['attribute']:
            if self.__schema__['attribute'][par]['init-value']:
                setattr(self, par, self.__schema__[
                        'attribute'][par]['init-value'])
            else:
                if self.__schema__['attribute'][par]['multivalue']:
                    setattr(self, par, [])
                else:
                    return None  # in this case, does not set anything
            return getattr(self, par)

        elif par in self.__schema__['relation']:
            if self.__schema__['relation'][par]['multivalue']:
                setattr(self, par, [])
            else:
                return None
            return getattr(self, par)

        raise AttributeError("'%s' object has no attribute/relation '%s'" %
                             (self.className(), par))

    def setattr_nocheck(self, par, val):
        """Sets an attribute by-passing the built-in type check."""

        self.__dict__[par] = val
        if par in list(self.__schema__['relation'].keys()):
            self.__touched__.append(par)
        return val

    def __setattr__(self, par, val):
        """Sets an object attribute or relationship.

        This method overrides the default setattr method, so it can apply
        existence and type verification on class attributes. If the attribute
        to be set starts with '__', or the passed value is None,
        no verification is performed. If the value to set an attribute is a
        list, the type verification is performed in every component of that
        list.

        N.B.: This method takes a reference to the object being passed. It does
        not copy the value, so, if you do a.b = c, and then you apply changes
        to 'c', these changes will be also applied to 'a.b'.

        Parameters:

        par -- The name of the parameter (attribute or relationship)

        val -- The value that will be attributed to 'par'.

        Raises AttributeError if the parameter does not exist.

        Raises ValueError if the value you passed cannot be coerced to a
        compatible OKS python type for the attribute or relationship you are
        trying to set.
        """
        from .schema import coerce, check_relation, check_cardinality
        from . import dalproperty

        # no checks for control parameters
        if par[0:2] == '__':
            self.__dict__[par] = val
            return

        # and for the id it is special
        if par == 'id':
            if isinstance(val, str):
                prop = getattr(self.__class__, par)
                prop.__set__(self, val)
                self.__reset_identity__()
                return
            else:
                raise ValueError(
                    'The "id" attribute of a DAL object must be a string')

        if par in list(self.__schema__['attribute'].keys()):

            # If val is None, skip checks
            if val is None:
                self.__dict__[par] = val

            try:
                if val is not None:
                    check_cardinality(val, self.__schema__['attribute'][par])
                    if self.__schema__['attribute'][par]['multivalue']:
                        result = \
                            [coerce(v, self.__schema__['attribute'][par])
                             for v in val]
                    else:
                        result = coerce(val, self.__schema__['attribute'][par])
                else:
                    result = val

                try:
                    prop = getattr(self.__class__, par)

                except AttributeError:

                    prop = property(dalproperty._return_attribute(par),
                                    dalproperty._assign_attribute(par))
                    setattr(self.__class__, par, prop)

                prop.__set__(self, result)

            except ValueError as e:
                raise ValueError('Problems setting attribute "%s" '
                                 'at object %s: %s' %
                                 (par, self.fullName(), str(e)))

        elif par in list(self.__schema__['relation'].keys()):

            try:
                # If val is None, skip checks
                if val is not None:
                    check_cardinality(val, self.__schema__['relation'][par])

                    tmpval = \
                        val if self.__schema__[
                            'relation'][par]['multivalue'] else [val]
                    for v in tmpval:
                        check_relation(v, self.__schema__['relation'][par])

                try:
                    prop = getattr(self.__class__, par)

                except AttributeError:

                    multi = self.__schema__['relation'][par]['multivalue']
                    prop = property(
                        dalproperty._return_relation(par, multi=multi),
                        dalproperty._assign_relation(par))
                    setattr(self.__class__, par, prop)

                prop.__set__(self, val)
                self.__touched__.append(par)

            except ValueError as e:
                raise ValueError('Problems setting relation "%s" at '
                                 'object %s: %s' %
                                 (par, self.fullName(), str(e)))

        else:
            raise AttributeError('Parameter "%s" is not ' % par +
                                 'part of class "%s" or any of its '
                                 'parent classes' %
                                 (self.className()))


class DalType(type):
    """
    This class is the metaclass that every DAL class will be created with.

    DalType is a metaclass (something like a C++ template) that allows us to
    create DAL classes without having to go through the 'exec' burden all the
    time and being, therefore, much faster than that mechanism. The idea is
    that we create DAL types everytime we see a new class and archive this in a
    cache, together with the configuration object. Everytime an object of a
    certain OKS type is needed by the user, we make use of the generated class
    living in that cache to make it a new DAL object.

    The DAL type consistency checks are limited by the amount of generic
    functionality one can extract by looking at the C++ Configuration class.

    The work here is modelled after the old PyDALBase implementation that used
    to live in the "genconfig" package.
    """

    def __init__(cls, name, bases, dct):
        """Class constructor.

        Keyword Parameters:

        cls -- This is a pointer to the class being constructed

        name -- The name that the class will have

        bases -- These are the classes, objects of the new generated type will
        inherit from. It is useful in our context, to express the OKS
        inheritance relations between the classes.

        dct -- This is a dictionary that will contain mappings between
        methods/attributes of the newly generated class and values or methods
        that will be bound to it. The dictionary should contain a pointer to
        the class schema, and that should be called '__schema__'.
        """
        # set all types as expected by the DAL
        alltypes = [name]
        for b in bases:
            if hasattr(b, 'pyoksTypes'):
                for t in b.pyoksTypes():
                    if t not in alltypes:
                        alltypes.append(t)

        super(DalType, cls).__init__(name, bases, dct)
        cls.__okstypes__ = alltypes

    def pyclassName(cls):
        """Returns this class name"""
        return cls.__name__

    def pyoksTypes(cls):
        """Returns a join of this class and base class names"""
        return cls.__okstypes__


def get_classes(m):
    """Returns a map with classes in a module, the key is the class name."""

    # assesses all classes from other modules, correlate with names
    map = {}
    for k in dir(m):
        if k.find('__') == 0:
            continue
        map[k] = getattr(m, k)
    return map


def generate(configuration, other_dals=[]):
    """Generates the DAL python access layer for the configuration passed.

    This method will generate the python DAL access layer for all classes
    declared through the config.Configuration object passed. If this file
    includes other schemas, the classes for those schemas will also be
    generated, unless, classes with matching names are passed through the
    "other_dals" parameters.

    This method will re-use classes generated in other calls to this method,
    either directly (in DAL binding to a python module) or while you created
    Configuration type objects. So, you can call this as many times as you want
    without incurring in much overhead.

    Keyword parameters:

    configuration -- The config.Configuration object that you want the prepare
    the DAL for.

    other_dals -- This is a list of classes that contain other DALs that should
    be considered for the inheritance structure of the classes that are going
    to be generated here. These classes will not be regenerated. This parameter
    can be either a list of modules or classes that won't be regenerated, but
    re-used by this generation method.

    Returns the DAL classes you asked for.
    """
    from types import ModuleType as module

    klasses = []

    other_classes = {}
    for k in other_dals:
        if isinstance(k, module):
            other_classes.update(get_classes(k))
        else:
            other_classes[k.pyclassName()] = k

    # we can save a few loops here, we order by number of bases
    to_generate = {}
    for k in configuration.classes():
        if k in other_classes:
            continue

        N = len(configuration.superclasses(k))
        if N in to_generate:
            to_generate[N].append(k)
        else:
            to_generate[N] = [k]

    ordered = []
    run_order = list(to_generate.keys())
    run_order.sort()
    for k in run_order:
        ordered += to_generate[k]

    # generate what we need to
    while ordered:
        next = ordered[0]  # gets the first one, no matter what it is

        # if I generated this before, just re-use the class,
        # so python can check the types in the way the user expects
        if next in __dal__:
            klasses.append(__dal__[next])
            other_classes[next] = __dal__[next]

        # else, I need to generate a brand new class here and
        # add it to my __dal__
        else:
            bases = configuration.superclasses(next)
            bases = [other_classes.get(k, None) for k in bases]
            bases.append(DalBase)
            if None in bases:  # cannot yet generate for this one, rotate
                ordered.append(next)
            else:  # we can generate this one now
                klasses.append(DalType(next, tuple(bases),
                                       {'__schema__':
                                        configuration.__schema__[next]}))
                # so we can use this next time
                other_classes[next] = klasses[-1]
                klasses[-1].__doc__ = prettyprint_doc(
                    configuration.__schema__[next])
                __dal__[next] = klasses[-1]

        del ordered[0]

    return klasses


def module(name, schema, other_dals=[], backend='oksconfig', db=None):
    """Creates a new python module with the OKS schema files passed as
    parameter.

    This method creates a new module for the user, using the schema files
    passed as parameter. Classes from other DALs are not re-created, but just
    re-used. This is an example usage:

    import config.dal
    dal = config.dal.module('dal', 'dal/schema/core.schema.xml')
    DFdal = config.dal.module('DFdal', 'DFConfiguration/schema/df.schema.xml',
                              [dal])

    This will generate two python dals in the current context. One that binds
    everything available in the first schema file and a second one that binds
    everything else defined in the DF OKS schema file.

    Keyword parameters:

    name -- The name of the python module to create. It should match the name
    of the variable you are attributing to, but it is not strictly required by
    the python interpreter, just a good practice.

    schema -- This is a list of OKS schema files that should be considered. You
    can also pass OKS datafiles to this one, which actually includes the schema
    files you want to have a DAL for. It will just work.

    other_dals -- This is a list of other DAL modules that I'll not regenerate,
    and which classes will *not* make part of the returned module. In fact,
    this parameter is only used to restrict the amount of output classes since
    once class is generated internally, it is not regenerated a second time.
    In other words creating twice the same DAL implies in almost no overhead.

    backend -- This is the OKS backend to use when retrieving the schemas. By
    default it is set to 'oksconfig', which is what we
    """
    import types
    from .Configuration import Configuration

    retval = types.ModuleType(name)
    if isinstance(schema, str):
        schema = [schema]
    for s in schema:
        if db is None:
            db = Configuration(backend + ':' + s)
        else:
            db.load(s)
            db.__core_init__()

        for k in generate(db, other_dals):
            retval.__dict__[k.pyclassName()] = k
    return retval
