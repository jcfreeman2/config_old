#include "config/SubscriptionCriteria.h"
#include "config/DalObject.h"

void
ConfigurationSubscriptionCriteria::add(const std::string& class_name)
{
  m_classes_subscription.insert(class_name);
}

void
ConfigurationSubscriptionCriteria::add(const std::string& class_name, const std::string& object_id)
{
  m_objects_subscription[class_name].insert(object_id);
}

void
ConfigurationSubscriptionCriteria::add(const ::DalObject& object)
{
  add(object.class_name(), object.UID());
}

void
ConfigurationSubscriptionCriteria::remove(const std::string& class_name)
{
  m_classes_subscription.erase(class_name);
}

void
ConfigurationSubscriptionCriteria::remove(const std::string& class_name, const std::string& object_id)
{
  ObjectMap::iterator i = m_objects_subscription.find(class_name);
  if (i != m_objects_subscription.end())
    {
      i->second.erase(object_id);
      if (i->second.empty())
        {
          m_objects_subscription.erase(i);
        }
    }
}

void
ConfigurationSubscriptionCriteria::remove(const ::DalObject& object)
{
  remove(object.class_name(), object.UID());
}

std::ostream&
operator<<(std::ostream& s, const ::ConfigurationSubscriptionCriteria& criteria)
{
  s << "Subscription criteria:\n";

  // print out classes subscription
  s << "  classes subscription: ";

  if (criteria.get_classes_subscription().empty())
    {
      s << "(null)\n";
    }
  else
    {
      s << std::endl;
      for (const auto& i : criteria.get_classes_subscription())
        {
          s << "    \"" << i << "\"\n";
        }
    }

  // print out objects subscription
  s << "  objects subscription: ";

  if (criteria.get_objects_subscription().empty())
    {
      s << "(null)\n";
    }
  else
    {
      s << std::endl;
      for (const auto& i : criteria.get_objects_subscription())
        {
          s << "    objects of class \"" << i.first << "\":\n";
          for (const auto& j : i.second)
            {
              s << "      \"" << j << "\":\n";
            }
        }
    }

  return s;
}
