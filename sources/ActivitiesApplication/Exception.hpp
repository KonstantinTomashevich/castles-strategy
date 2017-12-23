#pragma once
#include <Urho3D/Container/Str.h>

template <class T> class Exception
{
public:
    explicit Exception (const Urho3D::String &exception) : exception_ (exception) {}
    Exception (const Exception &exception) : exception_ (exception.exception_) {}
    virtual ~Exception () {}
    const Urho3D::String &GetException () const { return exception_; }

private:
    Urho3D::String exception_;
};
