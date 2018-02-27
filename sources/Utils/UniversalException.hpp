#pragma once
#include <Urho3D/Container/Str.h>

class AnyUniversalException
{
public:
    explicit AnyUniversalException (const Urho3D::String &exception) : exception_ (exception) {}
    AnyUniversalException (const AnyUniversalException &another) : exception_ (another.exception_) {}
    virtual ~AnyUniversalException () {}
    const Urho3D::String &GetException () const { return exception_; }

private:
    Urho3D::String exception_;
};

template <class T> class UniversalException : public AnyUniversalException
{
public:
    explicit UniversalException (const Urho3D::String &exception) : AnyUniversalException (exception) {}
    UniversalException (const UniversalException <T> &another) : AnyUniversalException (another) {}
    virtual ~UniversalException () {}
};
