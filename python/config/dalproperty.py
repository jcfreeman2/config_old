from __future__ import absolute_import


def _return_relation(relation, multi=False, data=None,
                     dalobj=None, cache=None):

    if dalobj:
        setattr(dalobj, '__%s_data' % relation, (data, cache))

    def getter(obj):

        try:
            val = getattr(obj, '__%s' % relation)
        except AttributeError:
            try:
                ldata, lcache = obj.__dict__.pop('__%s_data' % relation)
            except KeyError:
                raise AttributeError

            if not multi:
                ldata = [ldata]

            val = []
            for k in ldata:
                if k.UID() in lcache[k.class_name()]:
                    val.append(lcache[k.class_name()][k.UID()])
                else:
                    val.append(k.as_dal(lcache))

            if not multi:
                val = val[0]

            setattr(obj, '__%s' % relation, val)

        return val

    return getter


def _assign_relation(relation):
    from .dal import DalBase

    def setter(obj, value):
        setattr(obj, '__%s' % relation, value)
        getattr(DalBase, '_updated').add(obj)

    return setter


def _return_attribute(attribute, dalobj=None, value=None):

    if dalobj:
        setattr(dalobj, '__%s' % attribute, value)

    def getter(obj):
        return getattr(obj, '__%s' % attribute)

    return getter


def _assign_attribute(attribute):
    from .dal import DalBase

    def setter(obj, value):
        setattr(obj, '__%s' % attribute, value)
        getattr(DalBase, '_updated').add(obj)

    return setter
