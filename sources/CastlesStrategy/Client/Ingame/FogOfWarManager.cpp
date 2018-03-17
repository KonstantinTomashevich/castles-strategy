#include "FogOfWarManager.hpp"
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/UI/Text3D.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/Image.h>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>

namespace CastlesStrategy
{
FogOfWarManager::FogOfWarManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
        owner_ (owner),
        updateDelay_ (DEFAULT_FOG_OF_WAR_UPDATE_DELAY),
        untilNextUpdate_ (0.0f),

        fogOfWarMaskImage_ (nullptr),
        fogOfWarMaskTexture_ (nullptr),

        underFogColor_ (DEFAULT_UNDER_FOG_OF_WAR_MASK_COLOR),
        visibleColor_ (DEFAULT_VISIBLE_MASK_COLOR),
        mapUnitSize_ ()
{

}

FogOfWarManager::~FogOfWarManager ()
{
    ReleaseImageAndTexture ();

}

void FogOfWarManager::SetupFogOfWarMask (const Urho3D::IntVector2 &maskSize, const Urho3D::Vector2 &mapSize)
{
    ReleaseImageAndTexture ();
    mapUnitSize_ = {maskSize.x_ / mapSize.x_, maskSize.y_ / mapSize.y_};

    fogOfWarMaskImage_ = new Urho3D::Image (context_);
    fogOfWarMaskImage_->SetSize (maskSize.x_, maskSize.y_, 4);
    fogOfWarMaskImage_->Clear (underFogColor_);

    fogOfWarMaskTexture_ = new Urho3D::Texture2D (context_);
    fogOfWarMaskTexture_->SetSize (maskSize.x_, maskSize.y_, Urho3D::Graphics::GetRGBAFormat (), Urho3D::TEXTURE_DYNAMIC);
    fogOfWarMaskTexture_->SetData (fogOfWarMaskImage_);

    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    fogOfWarMaskTexture_->SetName (FOG_OF_WAR_MASK_TEXTURE_RESOURCE_NAME);
    resourceCache->AddManualResource (fogOfWarMaskTexture_);
}

void FogOfWarManager::Update (float timeStep)
{
    if (owner_->GetGameStatus () == GS_PLAYING)
    {
        if (untilNextUpdate_ <= 0.0f)
        {
            UpdateFogOfWarMap ();
            UpdateMaterialsShaderParameters ();
            ResetText3DMaterials ();
            untilNextUpdate_ = updateDelay_;
        }
        else
        {
            untilNextUpdate_ -= timeStep;
        }
    }
}

float FogOfWarManager::GetUpdateDelay () const
{
    return updateDelay_;
}

void FogOfWarManager::SetUpdateDelay (float updateDelay)
{
    updateDelay_ = updateDelay;
}

Urho3D::Texture2D *FogOfWarManager::GetFogOfWarMaskTexture () const
{
    return fogOfWarMaskTexture_;
}

const Urho3D::Color &FogOfWarManager::GetUnderFogColor () const
{
    return underFogColor_;
}

void FogOfWarManager::SetUnderFogColor (const Urho3D::Color &underFogColor)
{
    underFogColor_ = underFogColor;
}

const Urho3D::Color &FogOfWarManager::GetVisibleColor () const
{
    return visibleColor_;
}

void FogOfWarManager::SetVisibleColor (const Urho3D::Color &visibleColor)
{
    visibleColor_ = visibleColor;
}

void FogOfWarManager::UpdateFogOfWarMap ()
{
    DataManager *dataManager = owner_->GetDataManager ();
    Urho3D::Node *unitsNode = owner_->GetScene ()->GetChild ("units");
    if (unitsNode == nullptr)
    {
        return;
    }

    Urho3D::PODVector <Urho3D::Node *> unitsNodes;
    unitsNode->GetChildrenWithComponent <Unit> (unitsNodes);
    fogOfWarMaskImage_->Clear (underFogColor_);

    for (const auto &unitNode : unitsNodes)
    {
        Unit *unit = unitNode->GetComponent <Unit> ();
        if (owner_->GetPlayerType () == PT_OBSERVER ||
                (unit->IsBelongsToFirst () && owner_->GetPlayerType () == PT_FIRST) ||
                (!unit->IsBelongsToFirst () && owner_->GetPlayerType () == PT_SECOND))
        {
            const UnitType &unitType = dataManager->GetUnitTypeByIndex (unit->GetUnitType ());
            MakeEllipseVisible (Urho3D::RoundToInt (unit->GetNode ()->GetPosition ().x_ * mapUnitSize_.x_),
                    Urho3D::RoundToInt (unit->GetNode ()->GetPosition ().z_ * mapUnitSize_.y_),
                    Urho3D::RoundToInt (unitType.GetVisionRange () * mapUnitSize_.x_),
                    Urho3D::RoundToInt (unitType.GetVisionRange () * mapUnitSize_.y_));
        }
    }

    fogOfWarMaskTexture_->SetData (fogOfWarMaskImage_);
}

void FogOfWarManager::ReleaseImageAndTexture ()
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    if (fogOfWarMaskImage_ != nullptr)
    {
        delete fogOfWarMaskImage_;
    }

    if (resourceCache != nullptr && fogOfWarMaskTexture_.NotNull ())
    {
        fogOfWarMaskTexture_.Reset ();
        resourceCache->ReleaseResource <Urho3D::Texture2D> (FOG_OF_WAR_MASK_TEXTURE_RESOURCE_NAME, false);
    }
}

void FogOfWarManager::UpdateMaterialsShaderParameters ()
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::PODVector <Urho3D::Material *> materials;
    resourceCache->GetResources <Urho3D::Material> (materials);

    for (Urho3D::Material *material : materials)
    {
        if (material->GetShaderParameter ("FogOfWarEnabled").GetInt () > 0)
        {
            material->SetShaderParameter ("DefaultColor", underFogColor_);
            material->SetShaderParameter ("MapMinPoint", Urho3D::Vector3::ZERO);
            material->SetShaderParameter ("MapMaxPoint", Urho3D::Vector3 (
                    fogOfWarMaskImage_->GetWidth () / mapUnitSize_.x_, 0.0f,
                    fogOfWarMaskImage_->GetHeight () / mapUnitSize_.y_));

            //* Fucking magic of Urho3D shader parameters. Without resetting they can magically become zeros.
            material->SetShaderParameter ("FogOfWarEnabled", 1);
            material->SetShaderParameter ("Unit", material->GetShaderParameter ("Unit").GetInt ());
            //*
            material->SetShaderParameter ("MinModifier", underFogColor_.ToVector3 ().Length () * 1.01f);
            material->SetTexture (Urho3D::TU_ENVIRONMENT, fogOfWarMaskTexture_);
        }
    }
}

void FogOfWarManager::ResetText3DMaterials ()
{
    Urho3D::PODVector <Urho3D::Text3D *> texts;
    owner_->GetScene ()->GetComponents <Urho3D::Text3D> (texts, true);

    for (Urho3D::Text3D *text : texts)
    {
        // More Urho3D shader magic. Without it Unit and FogOfWarEnabled parameters become zeros in shader.
        text->SetMaterial (text->GetMaterial ());
    }
}

void FogOfWarManager::MakeEllipseVisible (int centerX, int centerY, int width, int height)
{
    // Bresenham's procedure.
    int a2 = width * width;
    int b2 = height * height;
    int fa2 = 4 * a2;
    int fb2 = 4 * b2;
    int x, y, sigma;

    // First half.
    for (x = 0, y = height, sigma = 2 * b2 + a2 * (1 - 2 * height); b2 * x <= a2 * y; x++)
    {
        MakeLineVisible (centerX - x, centerX + x, centerY + y);
        MakeLineVisible (centerX - x, centerX + x, centerY - y);

        if (sigma >= 0)
        {
            sigma += fa2 * (1 - y);
            y--;
        }

        sigma += b2 * ((4 * x) + 6);
    }

    // Second half.
    for (x = width, y = 0, sigma = 2 * a2 + b2 * (1 - 2 * width); a2 * y <= b2 * x; y++)
    {
        MakeLineVisible (centerX - x, centerX + x, centerY + y);
        MakeLineVisible (centerX - x, centerX + x, centerY - y);

        if (sigma >= 0)
        {
            sigma += fb2 * (1 - x);
            x--;
        }

        sigma += a2 * ((4 * y) + 6);
    }
}

void FogOfWarManager::MakeLineVisible (int minX, int maxX, int y)
{
    for (int x = minX; x <= maxX; x++)
    {
        fogOfWarMaskImage_->SetPixel (x, y, visibleColor_);
    }
}
}
