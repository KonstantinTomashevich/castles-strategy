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

const ManagersHub * Manager::GetManagersHub () const
{
    return managersHub_;
}
}
