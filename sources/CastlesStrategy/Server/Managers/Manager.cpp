#include "Manager.hpp"

namespace CastlesStrategy
{
Manager::Manager (ManagersHub *managersHub) :
        managersHub_ (managersHub)
{

}

Manager::~Manager ()
{

}

ManagersHub *Manager::GetManagersHub () const
{
    return managersHub_;
}
}
