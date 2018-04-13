#pragma once

namespace CastlesStrategy
{
class ManagersHub;

class Manager
{
public:
    explicit Manager (ManagersHub *managersHub);
    virtual ~Manager ();

    ManagersHub * GetManagersHub () const;
    virtual void HandleUpdate (float timeStep) = 0;

private:
    ManagersHub *managersHub_;
};
}
