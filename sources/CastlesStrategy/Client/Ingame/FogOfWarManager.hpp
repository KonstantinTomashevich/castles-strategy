#pragma once
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Texture2D.h>

namespace CastlesStrategy
{
const float DEFAULT_FOG_OF_WAR_UPDATE_DELAY = 1.0f / 60.0f;
const Urho3D::String FOG_OF_WAR_MASK_TEXTURE_RESOURCE_NAME ("FogOfWarMask");
const Urho3D::Color DEFAULT_UNDER_FOG_OF_WAR_MASK_COLOR (0.35f, 0.35f, 0.35f, 1.0f);
const Urho3D::Color DEFAULT_VISIBLE_MASK_COLOR (1.0f, 1.0f, 1.0f, 1.0f);
const Urho3D::IntVector2 DEFAULT_FOG_OF_WAR_MASK_SIZE (512, 512);

class IngameActivity;
class FogOfWarManager : public Urho3D::Object
{
URHO3D_OBJECT (FogOfWarManager, Object)
public:
    explicit FogOfWarManager (IngameActivity *owner);
    virtual ~FogOfWarManager ();

    void SetupFogOfWarMask (const Urho3D::IntVector2 &maskSize, const Urho3D::Vector2 &mapSize);
    void Update (float timeStep);

    float GetUpdateDelay () const;
    void SetUpdateDelay (float updateDelay);
    Urho3D::Texture2D *GetFogOfWarMaskTexture () const;

    const Urho3D::Color &GetUnderFogColor () const;
    void SetUnderFogColor (const Urho3D::Color &underFogColor);

    const Urho3D::Color &GetVisibleColor () const;
    void SetVisibleColor (const Urho3D::Color &visibleColor);

    bool IsFogOfWarEnabled () const;
    void SetFogOfWarEnabled (bool fogOfWarEnabled);

private:
    void UpdateFogOfWarMap ();
    void ReleaseImageAndTexture ();
    // TODO: It's not a best way, later think about better solutions.
    void UpdateMaterialsShaderParameters ();
    // TODO: It's not a best way, later think about better solutions.
    void ResetText3DMaterials ();
    /// Bresenham's procedure.
    void MakeEllipseVisible (int centerX, int centerY, int width, int height);
    void MakeLineVisible (int minX, int maxX, int y);

    IngameActivity *owner_;
    float updateDelay_;
    float untilNextUpdate_;

    Urho3D::Image *fogOfWarMaskImage_;
    Urho3D::SharedPtr <Urho3D::Texture2D> fogOfWarMaskTexture_;

    Urho3D::Color underFogColor_;
    Urho3D::Color visibleColor_;
    Urho3D::Vector2 mapUnitSize_;
    bool fogOfWarEnabled_;
};
}
