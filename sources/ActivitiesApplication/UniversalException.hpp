#pragma once
#include <Urho3D/Container/Str.h>

template <class T> class UniversalException
{
public:
    explicit UniversalException (const Urho3D::String &exception) : exception_ (exception) {}
    UniversalException (const UniversalException &exception) : exception_ (exception.exception_) {}
    virtual ~UniversalException () {}
    const Urho3D::String &GetException () const { return exception_; }

private:
    Urho3D::String exception_;
};
