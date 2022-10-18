#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Mon 22 Oct 2007 11:44:17 AM CEST

"""A pythonic wrapper over the OKS Configuration wrapper.

Necessary to give the user a more pythonic experience than dealing with
std::vector objects and memory management.
"""
import libpyconfig
from . import schema
from . import ConfigObject
import logging
from .proxy import make_proxy_class, _DelegateMetaFunction

# Alternative proxying scheme not using metaclasses

# proxy = make_proxy_class(libpyconfig.Configuration)

# class _Configuration(proxy):
#  def __init__(self, connection):

#    obj = libpyconfig.Configuration(connection)
#    super(_Configuration, self).__init__(obj)


class _Configuration(object,
                     metaclass=_DelegateMetaFunction):
    memberclass = libpyconfig.Configuration


class Configuration(_Configuration):
    """Access OKS/RDB configuration databases from python.
    """

    def __core_init__(self):
        self.__schema__ = schema.Cache(self, all=True)
        self.__schema__.update_dal(self)

        # initialize the inner set of configuration files available
        self.databases = []
        self.active_database = None
        if self.get_impl_param():
            self.databases.append(self.get_impl_param())
            self.active_database = self.get_impl_param()

        # we keep a cache of DAL'ed objects
        self.__cache__ = {}
        self.__initialize_cache__()

    def __init__(self, connection='oksconfig:'):
        """Initializes a Configuration database.

        Keyword arguments:

        connection -- A connection string, in the form of <backend>:<database>
        name, where <backend> may be set to be 'oksconfig' or 'rdbconfig' and
        <database> is either the name of the database XML file (in the case of
        'oksconfig') or the name of a database associated with an RDB server
        (in the case of 'rdbconfig').

        Warning: To use the RDB server, the IPC subsystem has to be initialized
        beforehand and as this is not done by this package. If the parameter
        'connection' is empty, the default is whatever is the default for the
        config::Configuration C++ class, which at this time boils down to look
        if TDAQ_DB is set and take that default.

        Raises RuntimeError, in case of problems.
        """

        super(Configuration, self).__init__(connection)
        self.__core_init__()

    def databases(self):
        """Returns a list of associated databases which are opened"""
        return self.databases

    def set_active(self, name):
        """Sets a database to become active when adding new objects.

        This method raises NameError in case a database cannot be made active.
        """

        if name in self.databases:
            self.active_database = name
        else:
            raise NameError('database "%s is not loaded in this object' % name)

    def __initialize_cache__(self):
        """Initializes the internal DAL cache"""
        for k in self.classes():
            if k not in self.__cache__:
                self.__cache__[k] = {}

    def __update_cache__(self, objs):
        """Updates the internal DAL cache"""
        for obj in objs:
            for oks_type in obj.oksTypes():
                self.__cache__[oks_type][obj.id] = obj

    def __delete_cache__(self, objs):
        """Updates the internal DAL cache"""
        for obj in objs:
            for oks_type in obj.oksTypes():
                if obj.id in self.__cache__[oks_type]:
                    del self.__cache__[oks_type][obj.id]

    def __retrieve_cache__(self, class_name, id=None):
        """Retrieves all objects that match a certain class_name/id"""
        if id:
            if id in self.__cache__[class_name]:
                return self.__cache__[class_name][id]
        else:
            return iter(self.__cache__[class_name].values())

    def get_objs(self, class_name, query=''):
        """Returns a python list of ConfigObject's with the given class name.

        Keyword arguments:

        class_name -- This is the name of the OKS Class to be used for the
        search. It has to be amongst one of the classes returned by the
        "classes()" method.

        query -- This is specific OKS query you may want to perform to reduce
        the returned subset. By default it is empty, what makes me return all
        objects for which the class (or base class) matches the 'class_name'
        parameter you set.

        Returns a (python) list with config.ConfigObject's
        """
        objs = super(Configuration, self).get_objs(class_name, query)
        return [ConfigObject.ConfigObject(k, self.__schema__, self)
                for k in objs]

    def attributes(self, class_name, all=False):
        """Returns a list of attributes of the named class

        This method will return a list of direct (not inherited) attributes of
        a certain class as a python list. If the 'all' flag is set to True,
        then direct and inherited attributes are returned.

        Keyword arguments:

        class_name -- This is the class of the object you want to inspect.

        all -- If set to 'True', returns direct and inherited attributes,
        otherwise, only direct attributes (the default).

        Raises RuntimeError on problems
        """
        return super(Configuration, self).attributes(class_name, all)

    def relations(self, class_name, all=False):
        """Returns a list of attributes of the named class

        This method will return a list of direct (not inherited) relationships
        of a certain class as a python list. If the 'all' flag is set to True,
        then direct and inherited attributes are returned.

        Keyword arguments:

        class_name -- This is the class of the object you want to inspect.

        all -- If set to 'True', returns direct and inherited relationships,
        otherwise, only direct relationships (the default).

        Raises RuntimeError on problems
        """
        return super(Configuration, self).relations(class_name, all)

    def superclasses(self, class_name, all=False):
        """Returns a list of superclasses of the named class

        This method will return a list of direct (not inherited) superclasses
        of a certain class as a python list. If the 'all' flag is set to True,
        then direct and inherited attributes are returned.

        Keyword arguments:

        class_name -- This is the class of the object you want to inspect.

        all -- If set to 'True', returns direct and inherited superclasses,
        otherwise, only direct superclasses (the default).

        Raises RuntimeError on problems
        """
        return super(Configuration, self).superclasses(class_name, all)

    def subclasses(self, class_name, all=False):
        """Returns a list of subclasses of the named class

        This method will return a list of direct (not inherited) subclasses of
        a certain class as a python list. If the 'all' flag is set to True,
        then direct and inherited attributes are returned.

        Keyword arguments:

        class_name -- This is the class of the object you want to inspect.

        all -- If set to 'True', returns direct and inherited subclasses,
        otherwise, only direct subclasses (the default).

        Raises RuntimeError on problems
        """
        return super(Configuration, self).subclasses(class_name, all)

    def classes(self):
        """Returns a list of all classes loaded in this Configuration."""
        return list(super(Configuration, self).classes())

    def create_db(self, db_name, includes):
        """Creates a new database on the specified server, sets it active.

        This method creates a new database on the specified server. If the
        server is not specified, what is returned by get_impl_name() is used.
        After the creation, this database immediately becomes the "active"
        database where new objects will be created at. You can reset that
        using the "set_active()" call in objects of this class.

        Keyword parameters:

        db_name -- The name of the database to create.

        includes -- A list of includes this database will have.
        """
        super(Configuration, self).create_db(db_name, includes)

        # we take this opportunity to update the class cache we have
        self.__schema__.update(self)
        self.__schema__.update_dal(self)
        self.__initialize_cache__()

        # and to set the current available databases and active database
        if db_name not in self.databases:
            self.databases.append(db_name)
        self.active_database = db_name

    def get_includes(self, at=None):
        """Returns a a list of all includes in a certain database.

        Keyword arguments:

        at -- This is the name of the database you want to get the includes
        from. If set to 'None' (the default) we use whatever
        self.active_database is set to hoping for the best.
        """
        if not at:
            at = self.active_database
        return super(Configuration, self).get_includes(at)

    def remove_include(self, include, at=None):
        """Removes a included file in a certain database.

        This method will remove include files from the active database or from
        any other include database if mentioned.

        Keyword parameters:

        include -- A single include to remove from the database.

        at -- This is the name of database you want to remove the include(s)
        from, If set to None (the default), I'll simply use the value of
        'self.active_database', hoping for the best.
        """
        if not at:
            at = self.active_database
        super(Configuration, self).remove_include(at, include)

    def add_include(self, include, at=None):
        """Adds a new include to the database.

        This method includes new files in the include section of your database.
        You can specify the file to which you want to add the include file.
        If you don't, it uses the last opened (active) file.

        Keyword parameters:

        include -- This is a single include to add into your database

        at -- This is the name of database you want to add the include at,
        If set to None (the default), I'll simply use the value of
        'self.active_database', hoping for the best.
        """
        if not at:
            at = self.active_database
        if include not in self.get_includes(at):
            super(Configuration, self).add_include(at, include)
            # we take this opportunity to update the class cache we have
            self.__schema__.update(self)
            self.__schema__.update_dal(self)
            self.__initialize_cache__()

    def __str__(self):
        return self.get_impl_spec() + \
            ', %d classes loaded' % len(self.classes())

    def __repr__(self):
        return '<Configuration \'' + self.get_impl_spec() + '\'>'

    def create_obj(self, class_name, uid, at=None):
        """Creates a new ConfigObject, related with the database you specify.

        Keyword arguments:

        class_name -- This is the name of the OKS Class to be used for the
        newly created object. It has to be amongst one of the classes returned
        by the "classes()" method.

        uid -- This is the UID of the object inside the OKS database.

        at -- This is either the name of database you want to create the object
        at, or another ConfigObject that will be used as a reference to
        determine at which database to create the new object. If set to None
        (the default), I'll simply use the value of 'self.active_database',
        hoping for the best.
        """
        if not at:
            at = self.active_database
        obj = super(Configuration, self).create_obj(at, class_name, uid)
        return ConfigObject.ConfigObject(obj, self.__schema__, self)

    def get_obj(self, class_name, uid):
        """Retrieves a ConfigObject you specified.

        Keyword arguments:

        class_name -- This is the name of the OKS Class to be used for the
        search. It has to be amongst one of the classes returned by the
        "classes()" method.

        uid -- This is the UID of the object inside the OKS database.

        """
        obj = super(Configuration, self).get_obj(class_name, uid)
        return ConfigObject.ConfigObject(obj, self.__schema__, self)

    def add_dal(self, dal_obj, at=None, cache=None, recurse=True):
        """Updates the related ConfigObject in the database using a DAL
        reflection.

        This method will take the properties of the DAL object passed as
        parameter and will try to either create or update the relevant
        ConfigObject in the database, at the file specified. It does this
        recursively, in colaboration with the ConfigObject class.

        Keyword arguments:

        dal_obj -- This is the DAL object that will be used for the operation.
        It should be a reflection of the ConfigObject you want to create.

        at -- This is either the name of database you want to create the object
        at, or another ConfigObject that will be used as a reference to
        determine at which database to create the new object. If set to None
        (the default), I'll simply use the value of 'self.active_database',
        hoping for the best.

        cache -- This is a cache that may be set by the Configuration object if
        necessary. Users should *never* set this variable. This variable is
        there to handle recursions gracefully.

        recurse -- This is a boolean flag that indicates if you want to enable
        recursion or not in the update. If set to 'True' (the default), I'll
        recurse until all objects in the tree that do *not* exist yet in the
        database are created (existing objects are gracefully ignored).
        Otherwise, I'll not recurse at all and just make sure the attributes
        and relationships of the object passed as parameter are set to what you
        determine they should be.  Please note that if you decide to update
        relationships, that the objects to which you are pointing to should be
        available in the database (directly or indirectly through includes) if
        you choose to do this non-recursively.

        Returns the ConfigObject you wanted to create or update, that you may
        ignore for practical purposes.
        """
        if not cache:
            cache = {}

        if self.test_object(dal_obj.className(), dal_obj.id):
            obj = super(Configuration, self).get_obj(
                dal_obj.className(), dal_obj.id)
            co = ConfigObject.ConfigObject(obj, self.__schema__, self)
            cache[dal_obj.fullName()] = co
            self.__update_cache__([dal_obj])

        else:
            if not at:
                at = self.active_database
            obj = super(Configuration, self) \
                .create_obj(at, dal_obj.className(),
                            dal_obj.id)
            co = ConfigObject.ConfigObject(obj, self.__schema__, self)
            cache[dal_obj.fullName()] = co
            co.update_dal(dal_obj, self.add_dal, self.get_obj, cache=cache,
                          recurse=recurse)
            self.__update_cache__([dal_obj])

        return co

    def update_dal(self, dal_obj, ignore_error=True, at=None, cache=None,
                   recurse=False):
        """Updates the related ConfigObject in the database using a DAL
        reflection.

        This method will take the properties of the DAL object passed as
        parameter and will try to either create or update the relevant
        ConfigObject in the database, at the file specified. It does this
        recursively, in colaboration with the ConfigObject class.

        Keyword arguments:

        dal_obj -- This is the DAL object that will be used for the operation.
        It should be a reflection of the ConfigObject you want to create.

        ignore_error -- This flag will make me ignore errors related to the
        update of objects in this database. It is useful if you want to
        overwrite as much as you can and leave the other objects which you
        cannot write to untouched.
        Otherwise, objects you cannot touch (write permissions or other
        problems), when tried to be set, will raise a 'ValueError'.

        at -- This is either the name of database you want to create the
        object at, or another ConfigObject that will be used as a reference to
        determine at which database to create the new object. If set to None
        (the default), I'll simply use the value of 'self.active_database',
        hoping for the best.

        cache -- This is a cache that may be set by the Configuration object if
        necessary. Users should *never* set this variable. This variable is
        there to handle recursions gracefully.

        recurse -- If this flag is set, the update will recurse until all
        objects linked from the given object are updated. This encompasses
        'include-file' objects. The default is a safe "False". Please,
        understand the impact of what you are doing before setting this to
        'True'.

        Returns the ConfigObject you wanted to create or update, that you may
        ignore for practical purposes.
        """
        if not cache:
            cache = {}
        co_func = self.update_dal_permissive
        if not ignore_error:
            co_func = self.update_dal_pedantic

        if hasattr(dal_obj, '__old_id'):
            old_id = getattr(dal_obj, '__old_id')
            if old_id != dal_obj.id:
                old = super(Configuration, self).get_obj(
                      dal_obj.className(), old_id)
                if old:
                    old.rename(dal_obj.id)
                    delattr(dal_obj, '__old_id')

        if self.test_object(dal_obj.className(), dal_obj.id):
            obj = super(Configuration, self).get_obj(
                dal_obj.className(), dal_obj.id)
            co = ConfigObject.ConfigObject(obj, self.__schema__, self)
            cache[dal_obj.fullName()] = co
            try:
                co.update_dal(dal_obj, co_func, self.get_obj, cache=cache,
                              recurse=recurse)
            except ValueError as e:
                if not ignore_error:
                    raise
                else:
                    logging.warning(
                        'Ignoring error in setting %s: %s'
                        % (repr(co), str(e)))
            self.__update_cache__([dal_obj])

        else:
            if not at:
                at = self.active_database
            obj = super(Configuration, self) \
                .create_obj(at, dal_obj.className(),
                            dal_obj.id)
            co = ConfigObject.ConfigObject(obj, self.__schema__, self)
            cache[dal_obj.fullName()] = co
            co.update_dal(dal_obj, co_func, self.get_obj, cache=cache,
                          recurse=recurse)
            self.__update_cache__([dal_obj])

        return co

    def update_dal_permissive(self, dal_obj, at=None, cache=None,
                              recurse=False):
        """Alias to update_dal() with ignore_error=True"""
        return self.update_dal(dal_obj, True, at, cache, recurse)

    def update_dal_pedantic(self, dal_obj, at=None, cache=None, recurse=False):
        """Alias to update_dal() with ignore_error=False"""
        return self.update_dal(dal_obj, False, at, cache, recurse)

    def get_dal(self, class_name, uid):
        """Retrieves a DAL reflection of a ConfigObject in this OKS database.

        This method acts recursively, until all objects deriving from the
        object you are retrieving have been returned. It is possible to reach
        the python limit using this. If that is the case, make sure to extend
        this limit with the following technique:

        import sys
        sys.setrecursionlimit(10000) # for example

        Keyword arguments:

        class_name -- The name of the OKS class of the object you are trying to
        retrieve

        uid -- This is the object's UID.

        Returns DAL representations of object in this Configuration database.
        """
        if uid not in self.__cache__[class_name]:
            obj = self.get_obj(class_name, uid)
            obj.as_dal(self.__cache__)
        return self.__cache__[class_name][uid]

    def get_dals(self, class_name):
        """Retrieves (multiple) DAL reflections of ConfigObjects in this
        database.

        This method acts recursively, until all objects deriving from the
        object you are retrieving have been returned.

        Keyword arguments:

        class_name -- The name of the OKS class of the objects you are trying
        to retrieve

        Returns DAL representations of objects in this Configuration database.
        """

        for k in self.get_objs(class_name):
            if k.UID() not in self.__cache__[class_name]:
                k.as_dal(self.__cache__)
        return list(self.__cache__[class_name].values())

    def get_all_dals(self):
        """Retrieves (multiple) DAL reflections of ConfigObjects in this
        database.

        This method acts recursively, until all objects deriving from the
        object you are retrieving have been returned.

        Returns DAL representations of objects in this Configuration database.
        """
        from .ConfigObject import ConfigObject as CO

        # get the unique values existing in the cache (probably few)
        retval = {}
        for v in self.__cache__.values():
            for k in v.values():
                if k.fullName() not in retval:
                    retval[k.fullName()] = k

        # put all objects in the cache and update the return list
        for class_name in self.classes():
            for k in super(Configuration,
                           self).get_objs(class_name,
                                          '(this (object-id \"\" !=))'):
                if k.UID() not in self.__cache__[class_name]:
                    j = CO(k, self.__schema__, self).as_dal(self.__cache__)
                    retval[j.fullName()] = j
                else:
                    j = self.__cache__[class_name][k.UID()]
                    retval[j.fullName()] = j

        return retval

    def destroy_dal(self, dal_obj):
        """Destroyes the Database counterpart of the DAL object given.

        This method will destroy the equivalent ConfigObject reflection from
        this Configuration object.

        Keyword parameters:

        dal_obj -- This is the DAL reflection of the object you want to delete.

        """
        if self.test_object(dal_obj.className(), dal_obj.id):
            obj = self.get_obj(dal_obj.className(), dal_obj.id)
            self.destroy_obj(obj)
        self.__delete_cache__((dal_obj,))

    def destroy_obj(self, obj):
        """Destroyes the given database object.

        This method will destroy the given ConfigObject object.

        """

        # the C++ implementation of destroy_obj wants
        # a libpyconfig.ConfigObject instance. So
        # we have to extract it from our proxy
        return super(Configuration, self).destroy_obj(obj._obj)
