from __future__ import absolute_import
from .Configuration import Configuration
from .ConfigObject import ConfigObject
from . import dal
from . import schema


def updated_dals():
    return dal.DalBase.updated()


updated_dals.__doc__ = dal.DalBase.updated.__doc__


def reset_updated_dals():
    dal.DalBase.reset_updated_list()


reset_updated_dals.__doc__ = dal.DalBase.reset_updated_list.__doc__
