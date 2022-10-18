#!/usr/bin/env python
# vim: set fileencoding=utf-8 :
# Created by Andre Anjos <andre.dos.anjos@cern.ch>
# Mon 22 Oct 2007 04:12:01 PM CEST

"""A pythonic wrapper over the OKS ConfigObject wrapper.

Necessary to give the user a more pythonic experience than dealing with
std::vector objects and memory management.
"""
import libpyconfig
import logging
from . import dalproperty
from .proxy import make_proxy_class, _DelegateMetaFunction

# Alternative proxying scheme not using metaclasses

# proxy = make_proxy_class(libpyconfig.ConfigObject)

# class _ConfigObject(proxy):
#  def __init__(self, raw_object):
#    obj = libpyconfig.ConfigObject(raw_object)
#    super(_ConfigObject,self).__init__(obj)


class _ConfigObject(object,
                    metaclass=_DelegateMetaFunction):
    memberclass = libpyconfig.ConfigObject


class ConfigObject(_ConfigObject):
    """ConfigObjects are generic representations of objects in OKS."""

    def __init__(self, raw_object, schema, configuration):
        """Initializes a ConfigObject in a certain Configuration database.

        This method will initialize the ConfigObject and recursively any other
        objects under it. If the number of recursions is too big, it may stop
        python from going on. In this case, you may reset the limit with:

        import sys
        sys.setrecursionlimit(10000) # for example

        raw_object -- This is the libpyconfig.ConfigObject to initialize this
        object from.

        schema -- A pointer to the overall schema from the Configuration
        database to which this object is associated.

        configuration -- The database this object belongs too. This is needed
        to bind the database lifetime to this object

        Raises RuntimeError in case of problems.
        """
        super(ConfigObject, self).__init__(raw_object)

        self.__schema__ = schema[self.class_name()]
        self.__overall_schema__ = schema
        self.__cache__ = {}
        self.__configuration__ = configuration

    def __getitem__(self, name):
        """Returns the attribute or relation defined by 'name'.

        If an attribute does not exist, instead of a wrapper exception,
        you get an AttributeError.

        Raises KeyError, if the 'name' is not a valid class item.
        """
        if name in self.__schema__['attribute']:
            return (self.__schema__['attribute'][name]
                    ['co_get_method'](self, name))

        elif name in self.__cache__:
            return self.__cache__[name]

        elif name in self.__schema__['relation']:
            try:
                data = self.__schema__[
                    'relation'][name]['co_get_method'](self, name)
                if self.__schema__['relation'][name]['multivalue']:
                    self.__cache__[name] = \
                        [ConfigObject(k, self.__overall_schema__,
                                      self.__configuration__)
                         for k in data]
                else:
                    self.__cache__[name] = None
                    if data:
                        self.__cache__[name] = \
                            ConfigObject(data, self.__overall_schema__,
                                         self.__configuration__)

            except RuntimeError as e:
                if self.__schema__['relation'][name]['multivalue']:
                    self.__cache__[name] = []
                else:
                    self.__cache__[name] = None
                logging.warning('Problems retrieving relation "%s" of '
                                'object "%s". Resetting and ignoring. '
                                'OKS error: %s' %
                                (name, self.full_name(), str(e)))

            return self.__cache__[name]

        # shout if you get here
        raise KeyError('"%s" is not an attribute or relation of class "%s"' %
                       (name, self.class_name()))

    def __eq__(self, other):
        """True is the 2 objects have the same class and ID and config database
        """
        return (self.class_name() == other.class_name()) and \
               (self.UID() == other.UID())

    def __ne__(self, other):
        """True if the 2 objects *not* have the same class and ID."""
        return not (self == other)

    def __hash__(self):
        """True is the 2 objects have the same class and ID and config database
        """
        return hash(self.full_name())

    def __setitem__(self, name, value):
        """Sets the attribute or relation defined by 'name'.

        This method works as a wrapper around the several set functions
        attached to ConfigObjects, by making them feel a bit more pythonic.
        If attributes do not exist in a certain ConfigObject, we raise an
        AttributeError. If a value cannot be set, we raise a ValueError instead
        of the classical SWIG RuntimeErrors everywhere.

        Raises AttributeError, if the 'name' is not a valid class item.
        Raises ValueError, if I cannot set the value you want to the variable

        Returns 'value', so you can daisy-chain attributes in the normal way.
        """
        try:
            if name in self.__schema__['attribute']:
                self.__schema__['attribute'][name]['co_set_method'](
                    self, name, value)

            elif name in self.__schema__['relation']:
                self.__schema__['relation'][name]['co_set_method'](
                    self, name, value)
                self.__cache__[name] = value

            else:
                # shout if you get here
                raise KeyError('"%s" is not an attribute or relation of '
                               'class "%s"' %
                               (name, self.class_name()))

        except RuntimeError as e:
            raise ValueError("Error setting value of variable '%s' in %s "
                             "to '%s': %s"
                             % (name, repr(self), str(value), str(e)))

        return value

    def __repr__(self):
        return '<ConfigObject \'' + self.full_name() + '\'>'

    def __str__(self):
        return self.full_name() + \
            ' (%d attributes, %d relations), inherits from %s' % \
            (len(self.__schema__['attribute']),
             len(self.__schema__['relation']),
             self.__schema__['superclass'])

    def update_dal(self, d, followup_method, get_method, cache=None,
                   recurse=True):
        """Sets each attribute defined in the DAL object 'd', with the value.

        This method will update the ConfigObject attributes and its
        relationships recursively, cooperatively with the Configuration class.
        The recursion is implemented in a very easy way in these terms.

        Keyword arguments:

        d -- This is the DAL object you are trying to set this
        ConfigObject from.

        followup_method -- The Configuration method to call for the recursion.
        This one varies with the type of change you are performing (adding or
        updating).

        get_method -- The Configuration method to call for retrieving objects
        from the associated database.

        cache -- This is a cache that may be set by the Configuration object if
        necessary. Users should *never* set this variable. This variable is
        there to handle recursions gracefully.

        recurse -- This is a boolean flag that indicates if you want to enable
        recursion or not in the update. If set to 'True' (the default), I'll
        recurse until all objects in the tree are updated. Otherwise, I'll not
        recurse at all and just make sure my attributes and relationships are
        set to what you determine they should be. Please note that if you
        decide to update relationships, that the objects to which you are
        pointing to should be available in the database (directly or indirectly
        through includes) if you choose to do this non-recursively.

        """
        for k in self.__schema__['attribute'].keys():
            if hasattr(d, k) and getattr(d, k) is not None:
                self[k] = getattr(d, k)

        for k, v in self.__schema__['relation'].items():
            if not hasattr(d, k):
                continue

            # if you get here, d has attribute k and it is not None
            if v['multivalue']:
                if not getattr(d, k):
                    self[k] = []
                else:
                    # please, note you cannot just "append" to ConfigObject
                    # multiple relationships, since a new value (i.e. a new
                    # list) is returned each time you use __getitem__.
                    # So, you have to set all in one go.
                    val = []
                    for i in getattr(d, k):
                        if cache and i.fullName() in cache:
                            val.append(cache[i.fullName()])
                        elif recurse:
                            val.append(followup_method(
                                i, cache=cache, recurse=recurse))
                        else:
                            val.append(get_method(i.className(), i.id))
                    self[k] = val

            else:  # this is the simplest case
                i = getattr(d, k)
                if i:
                    if cache and i.fullName() in cache:
                        self[k] = cache[i.fullName()]
                    elif recurse:
                        self[k] = followup_method(
                            i, cache=cache, recurse=recurse)
                    else:
                        self[k] = get_method(i.className(), i.id)
                else:
                    self[k] = None

    def as_dal(self, cache):
        """Returns a DAL representation of myself and my descendents.

        In this implementation, we by-pass the type checking facility to gain
        in time and because we know that if the ConfigObject was set, it must
        conform to OKS in any case.
        """
        dobj = self.__schema__['dal'](id=self.UID())
        for k in dobj.oksTypes():
            cache[k][self.UID()] = dobj

        for a in self.__schema__['attribute'].keys():
            setattr(dobj.__class__, a,
                    property(dalproperty._return_attribute(a, dobj, self[a]),
                             dalproperty._assign_attribute(a)))

        for r in self.__schema__['relation'].keys():
            data = self[r]

            if self.__schema__['relation'][r]['multivalue']:

                getter = dalproperty. \
                    _return_relation(r, multi=True, cache=cache,
                                     data=data, dalobj=dobj)

            else:

                getter = dalproperty. \
                    _return_relation(r, cache=cache,
                                     data=data, dalobj=dobj)

            setattr(dobj.__class__, r,
                    property(getter,
                             dalproperty._assign_relation(r)))

        return dobj

    def set_obj(self, name, value):
        """Sets the sigle-value relation 'name' to the provided 'value'

        """

        # the C++ implementation of set_obj wants
        # a libpyconfig.ConfigObject instance. So
        # we have to extract it from our proxy

        if value is not None:
            value = value._obj

        return super(ConfigObject, self).set_obj(name, value)

    def set_objs(self, name, value):
        """Sets the multi-value relation 'name' to the provided 'value'

        """

        # the C++ implementation of set_objs wants
        # a libpyconfig.ConfigObject instances. So
        # we have to extract them from our proxy

        if value is not None:
            tmp = [e._obj if e is not None else e for e in value]
        else:
            tmp = None

        return super(ConfigObject, self).set_objs(name, tmp)
