#pragma once
#include <Urho3D/Container/Str.h>

class AnyUniversalException
{
public:
    explicit AnyUniversalException (const Urho3D::String &exception) noexcept : exception_ (exception) {}
    AnyUniversalException (const AnyUniversalException &another) noexcept : exception_ (another.exception_) {}
    virtual ~AnyUniversalException () {}
    const Urho3D::String &GetException () const noexcept { return exception_; }

private:
    Urho3D::String exception_;
};

template <class T> class UniversalException : public AnyUniversalException
{
public:
    explicit UniversalException (const Urho3D::String &exception) noexcept : AnyUniversalException (exception) {}
    UniversalException (const UniversalException <T> &another) noexcept : AnyUniversalException (another) {}
    virtual ~UniversalException () {}
};
