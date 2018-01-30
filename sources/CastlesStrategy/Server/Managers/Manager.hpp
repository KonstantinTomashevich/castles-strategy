#pragma once

namespace CastlesStrategy
{
class ManagersHub;

class Manager
{
public:
    explicit Manager (ManagersHub *managersHub);
    virtual ~Manager ();

    const ManagersHub * GetManagersHub () const;
    virtual void HandleUpdate (float timeStep) = 0;

private:
    ManagersHub *managersHub_;
};
}
