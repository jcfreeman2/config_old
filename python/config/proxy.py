#!/usr/bin/env python


"""Proxing/Delegation tools

Provide several tools to implement proxying/delegation of objects. The proxying
instances expose the same public interface of the proxied object, but avoiding
inheritance. This allows to control the reference counts of the proxied object.

"""
# Explicit proxying mechanism not using metaclasses


class Proxy(object):
    """A very basic holder class

    Just holds the reference to a provided object.

    """

    def __init__(self, obj):
        super(Proxy, self).__init__()
        self._obj = obj


def make_proxy_class(theclass):
    """Builds a delegation class out of a given type.

    Uses the Proxy class to generate a new proxy class exposing the same
    interface of the provided class and delegating the method calls to
    the hosted object instance.

    """
    def make_method(name):
        def method(self, *args, **kwds):
            return getattr(self._obj, name)(*args, **kwds)
        return method

    base = Proxy
    namespace = {}
    for methodname in dir(theclass):
        if not methodname.startswith('__'):
            namespace[methodname] = make_method(methodname)

    proxyclass = type("%s%s" % (theclass.__name__, base.__name__),
                      (base,), namespace)

    return proxyclass


# We do use a function instead of a class for the meta definition to avoid
# problems with inheritance of the constructed classes. In fact, using a
# function the type of the resulting classes is still 'type', therefore
# the meta functionalities are not implemented in the subclasses

def _DelegateMetaFunction(clsName, bases, atts):
    """ Implements a delegation pattern using a metaclass approach

    A class using this meta mechanism should have 'memberclass' class attribute
    initialized at the class of the instance to proxied. The metaclass will
    make sure the delegate class will expose all the public methods of the
    proxied one.
    Moreover, the metaclass will provide the delegate class with a '__init__'
    function instantiating a 'memberclass' object, storing it in 'self._obj'.
    The delegate class constructor method will therefore accept all the
    arguments accepted by the proxied class constructor.
    The delegate class uses slots

    """
    memberclass = atts['memberclass']

    def make_method(name):
        def method(self, *args, **kwds):
            return getattr(self._obj, name)(*args, **kwds)
        method.__name__ = name
        return method

    for methodname in dir(memberclass):
        if not methodname.startswith('__'):
            atts[methodname] = make_method(methodname)

    atts['__slots__'] = ['_obj', ]

    def initfun(self, *args, **kwds):
        # We force the proxied and delegate __init__
        # functions take the same arguments
        # This can be easily changed with some input from the
        # delegate class
        obj = memberclass(*args, **kwds)
        self._obj = obj

    initfun.__doc__ = \
        """Instantiate %s and store the instance in 'self._obj'

        """ % str(memberclass)
    initfun.__name__ = '__init__'

    atts['__init__'] = initfun

    return type(clsName, bases, atts)
