/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "StdAfx.h"
#include "Sprite.h"
#include <ITexture.h>
#include <CryPath.h>
#include <IRenderer.h>
#include <ISerialize.h>
#include <AzFramework/API/ApplicationAPI.h>

namespace
{
    const char* const spriteExtension = "sprite";

    // Increment this when the Sprite Serialize(TSerialize) function
    // changes to be incompatible with previous data
    uint32 spriteFileVersionNumber = 2;
    const char* spriteVersionNumberTag = "versionNumber";

    const char* allowedSpriteTextureExtensions[] = {
        "tif", "jpg", "jpeg", "tga", "bmp", "png", "gif", "dds"
    };
    const int numAllowedSpriteTextureExtensions = AZ_ARRAY_SIZE(allowedSpriteTextureExtensions);

    bool IsValidSpriteTextureExtension(const char* extension)
    {
        for (int i = 0; i < numAllowedSpriteTextureExtensions; ++i)
        {
            if (strcmp(extension, allowedSpriteTextureExtensions[i]) == 0)
            {
                return true;
            }
        }

        return false;
    }

    bool ReplaceSpriteExtensionWithTextureExtension(const string& spritePath, string& texturePath)
    {
        string testPath;
        for (int i = 0; i < numAllowedSpriteTextureExtensions; ++i)
        {
            testPath = PathUtil::ReplaceExtension(spritePath, allowedSpriteTextureExtensions[i]);
            AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
            if (fileIO && fileIO->Exists(testPath.c_str()))
            {
                texturePath = testPath;
                return true;
            }
        }

        return false;
    }

    //! \brief Reads a Vec2 tuple (as a string) into an AZ::Vector2
    //!
    //! Example XML string data: "1.0 2.0"
    void SerializeAzVector2(TSerialize ser, const char* attributeName, AZ::Vector2& azVec2)
    {
        if (ser.IsReading())
        {
            string stringVal;
            ser.Value(attributeName, stringVal);
            stringVal.replace(',', ' ');
            char* pEnd = nullptr;
            float uVal = strtof(stringVal.c_str(), &pEnd);
            float vVal = strtof(pEnd, nullptr);
            azVec2.Set(uVal, vVal);
        }
        else
        {
            Vec2 legacyVec2(azVec2.GetX(), azVec2.GetY());
            ser.Value(attributeName, legacyVec2);
        }
    }

    //! \return The number of child <Cell> tags off the <SpriteSheet> parent tag.
    int GetNumSpriteSheetCellTags(const XmlNodeRef& root)
    {
        int numCellTags = 0;
        XmlNodeRef spriteSheetTag = root->findChild("SpriteSheet");
        if (spriteSheetTag)
        {
            numCellTags = spriteSheetTag->getChildCount();
        }
        return numCellTags;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// STATIC MEMBER DATA
////////////////////////////////////////////////////////////////////////////////////////////////////

CSprite::CSpriteHashMap* CSprite::s_loadedSprites;
AZStd::string CSprite::s_emptyString;

////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
CSprite::CSprite()
    : m_texture(nullptr)
    , m_numSpriteSheetCellTags(0)
{
    AddRef();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSprite::~CSprite()
{
    if (m_texture)
    {
        // In order to avoid the texture being deleted while there are still commands on the render
        // thread command queue that use it, we queue a command to delete the texture onto the
        // command queue.
        SResourceAsync* pInfo = new SResourceAsync();
        pInfo->eClassName = eRCN_Texture;
        pInfo->pResource = m_texture;
        gEnv->pRenderer->ReleaseResourceAsync(pInfo);
    }

    s_loadedSprites->erase(m_pathname);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const string& CSprite::GetPathname() const
{
    return m_pathname;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const string& CSprite::GetTexturePathname() const
{
    return m_texturePathname;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ISprite::Borders CSprite::GetBorders() const
{
    return m_borders;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::SetBorders(Borders borders)
{
    m_borders = borders;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::SetCellBorders(int cellIndex, Borders borders)
{
    if (CellIndexWithinRange(cellIndex))
    {
        m_spriteSheetCells[cellIndex].borders = borders;
    }
    else
    {
        SetBorders(borders);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ITexture* CSprite::GetTexture() const
{
    // If the sprite is associated with a render target, the sprite does not own the texture.
    // In this case, find the texture by name when it is requested.
    if (!m_texture && !m_pathname.empty())
    {
        return gEnv->pRenderer->EF_GetTextureByName(m_pathname.c_str());
    }
    return m_texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::Serialize(TSerialize ser)
{
    // When reading, get sprite-sheet info from XML tag data, otherwise get
    // it from this sprite object directly.
    const bool hasSpriteSheetCells = ser.IsReading() ? m_numSpriteSheetCellTags > 0 : !GetSpriteSheetCells().empty();

    if (!hasSpriteSheetCells && ser.BeginOptionalGroup("Sprite", true))
    {
        ser.Value("m_left", m_borders.m_left);
        ser.Value("m_right", m_borders.m_right);
        ser.Value("m_top", m_borders.m_top);
        ser.Value("m_bottom", m_borders.m_bottom);

        ser.EndGroup();
    }

    if (hasSpriteSheetCells && ser.BeginOptionalGroup("SpriteSheet", true))
    {
        const int numSpriteSheetCells = ser.IsReading() ? m_numSpriteSheetCellTags : GetSpriteSheetCells().size();
        for (int i = 0; i < numSpriteSheetCells; ++i)
        {
            ser.BeginOptionalGroup("Cell", true);

            if (ser.IsReading())
            {
                m_spriteSheetCells.push_back(SpriteSheetCell());
            }

            string aliasTemp;
            if (ser.IsReading())
            {
                ser.Value("alias", aliasTemp);
                m_spriteSheetCells[i].alias = aliasTemp.c_str();
            }
            else
            {
                aliasTemp = m_spriteSheetCells[i].alias.c_str();
                ser.Value("alias", aliasTemp);
            }

            SerializeAzVector2(ser, "topLeft", m_spriteSheetCells[i].uvCellCoords.TopLeft());
            SerializeAzVector2(ser, "topRight", m_spriteSheetCells[i].uvCellCoords.TopRight());
            SerializeAzVector2(ser, "bottomRight", m_spriteSheetCells[i].uvCellCoords.BottomRight());
            SerializeAzVector2(ser, "bottomLeft", m_spriteSheetCells[i].uvCellCoords.BottomLeft());

            if (ser.BeginOptionalGroup("Sprite", true))
            {
                ser.Value("m_left", m_spriteSheetCells[i].borders.m_left);
                ser.Value("m_right", m_spriteSheetCells[i].borders.m_right);
                ser.Value("m_top", m_spriteSheetCells[i].borders.m_top);
                ser.Value("m_bottom", m_spriteSheetCells[i].borders.m_bottom);

                ser.EndGroup();
            }

            ser.EndGroup();
        }

        ser.EndGroup();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSprite::SaveToXml(const string& pathname)
{
    // NOTE: The input pathname has to be a path that can used to save - so not an Asset ID
    // because of this we do not store the pathname

    XmlNodeRef root =  GetISystem()->CreateXmlNode("Sprite");
    std::unique_ptr<IXmlSerializer> pSerializer(GetISystem()->GetXmlUtils()->CreateXmlSerializer());
    ISerialize* pWriter = pSerializer->GetWriter(root);
    TSerialize ser = TSerialize(pWriter);

    ser.Value(spriteVersionNumberTag, spriteFileVersionNumber);
    Serialize(ser);

    return root->saveToFile(pathname);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSprite::AreBordersZeroWidth() const
{
    return (m_borders.m_left == 0 && m_borders.m_right == 1 && m_borders.m_top == 0 && m_borders.m_bottom == 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSprite::AreCellBordersZeroWidth(int cellIndex) const
{
    if (CellIndexWithinRange(cellIndex))
    {
        return m_spriteSheetCells[cellIndex].borders.m_left == 0
               && m_spriteSheetCells[cellIndex].borders.m_right == 1
               && m_spriteSheetCells[cellIndex].borders.m_top == 0
               && m_spriteSheetCells[cellIndex].borders.m_bottom == 1;
    }
    else
    {
        return AreBordersZeroWidth();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
AZ::Vector2 CSprite::GetSize() const
{
    ITexture* texture = GetTexture();
    if (texture)
    {
        return AZ::Vector2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()));
    }
    else
    {
        return AZ::Vector2(0.0f, 0.0f);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
AZ::Vector2 CSprite::GetCellSize(int cellIndex) const
{
    AZ::Vector2 textureSize(GetSize());

    if (CellIndexWithinRange(cellIndex))
    {
        // Assume top width is same as bottom width
        const float normalizedCellWidth =
            m_spriteSheetCells[cellIndex].uvCellCoords.TopRight().GetX() -
            m_spriteSheetCells[cellIndex].uvCellCoords.TopLeft().GetX();

        // Similar, assume height of cell is same for left and right sides
        const float normalizedCellHeight =
            m_spriteSheetCells[cellIndex].uvCellCoords.BottomLeft().GetY() -
            m_spriteSheetCells[cellIndex].uvCellCoords.TopLeft().GetY();

        textureSize.SetX(textureSize.GetX() * normalizedCellWidth);
        textureSize.SetY(textureSize.GetY() * normalizedCellHeight);
    }

    return textureSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const ISprite::SpriteSheetCellContainer& CSprite::GetSpriteSheetCells() const
{
    return m_spriteSheetCells;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::SetSpriteSheetCells(const SpriteSheetCellContainer& cells)
{
    m_spriteSheetCells = cells;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::ClearSpriteSheetCells()
{
    m_spriteSheetCells.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::AddSpriteSheetCell(const SpriteSheetCell& spriteSheetCell)
{
    m_spriteSheetCells.push_back(spriteSheetCell);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
AZ::Vector2 CSprite::GetCellUvSize(int cellIndex) const
{
    AZ::Vector2 result(1.0f, 1.0f);

    if (CellIndexWithinRange(cellIndex))
    {
        result.SetX(m_spriteSheetCells[cellIndex].uvCellCoords.TopRight().GetX() - m_spriteSheetCells[cellIndex].uvCellCoords.TopLeft().GetX());
        result.SetY(m_spriteSheetCells[cellIndex].uvCellCoords.BottomLeft().GetY() - m_spriteSheetCells[cellIndex].uvCellCoords.TopLeft().GetY());
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const UiTransformInterface::RectPoints& CSprite::GetCellUvCoords(int cellIndex) const
{
    if (CellIndexWithinRange(cellIndex))
    {
        return m_spriteSheetCells[cellIndex].uvCellCoords;
    }

    static UiTransformInterface::RectPoints rectPoints(0.0f, 1.0f, 0.0f, 1.0f);
    return rectPoints;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ISprite::Borders CSprite::GetCellUvBorders(int cellIndex) const
{
    if (CellIndexWithinRange(cellIndex))
    {
        return m_spriteSheetCells[cellIndex].borders;
    }

    return m_borders;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
ISprite::Borders CSprite::GetTextureSpaceCellUvBorders(int cellIndex) const
{
    Borders textureSpaceBorders(m_borders);

    if (CellIndexWithinRange(cellIndex))
    {
        const float cellWidth = GetCellUvSize(cellIndex).GetX();
        const float cellMinUCoord = GetCellUvCoords(cellIndex).TopLeft().GetX();
        const float cellNormalizedLeftBorder = GetCellUvBorders(cellIndex).m_left * cellWidth;
        textureSpaceBorders.m_left = cellNormalizedLeftBorder;

        const float cellMaxUCoord = GetCellUvCoords(cellIndex).TopRight().GetX();
        const float cellNormalizedRightBorder = GetCellUvBorders(cellIndex).m_right * cellWidth;
        textureSpaceBorders.m_right = cellNormalizedRightBorder;

        const float cellHeight = GetCellUvSize(cellIndex).GetY();
        const float cellMinVCoord = GetCellUvCoords(cellIndex).TopLeft().GetY();
        const float cellNormalizedTopBorder = GetCellUvBorders(cellIndex).m_top * cellHeight;
        textureSpaceBorders.m_top = cellNormalizedTopBorder;

        const float cellMaxVCoord = GetCellUvCoords(cellIndex).BottomLeft().GetY();
        const float cellNormalizedBottomBorder = GetCellUvBorders(cellIndex).m_bottom * cellHeight;
        textureSpaceBorders.m_bottom = cellNormalizedBottomBorder;
    }

    return textureSpaceBorders;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
const AZStd::string& CSprite::GetCellAlias(int cellIndex) const
{
    if (CellIndexWithinRange(cellIndex))
    {
        return m_spriteSheetCells[cellIndex].alias;
    }

    return s_emptyString;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::SetCellAlias(int cellIndex, const AZStd::string& cellAlias)
{
    if (CellIndexWithinRange(cellIndex))
    {
        m_spriteSheetCells[cellIndex].alias = cellAlias;
    }
}

bool CSprite::IsSpriteSheet() const
{
    return m_spriteSheetCells.size() > 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int CSprite::GetCellIndexFromAlias(const AZStd::string& cellAlias) const
{
    int result = 0;
    int indexIter = 0;
    for (auto spriteCell : m_spriteSheetCells)
    {
        if (cellAlias == spriteCell.alias)
        {
            result = indexIter;
            break;
        }
        ++indexIter;
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC STATIC MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::Initialize()
{
    s_loadedSprites = new CSpriteHashMap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::Shutdown()
{
    delete s_loadedSprites;
    s_loadedSprites = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSprite* CSprite::LoadSprite(const string& pathname)
{
    // the input string could be in any form. So make it normalized
    // NOTE: it should not be a full path at this point. If called from the UI editor it will
    // have been transformed to a game path. If being called with a hard coded path it should be a
    // game path already - it is not good for code to be using full paths.
    AZStd::string assetPath(pathname.c_str());
    EBUS_EVENT(AzFramework::ApplicationRequests::Bus, NormalizePath, assetPath);
    string gamePath = assetPath.c_str();

    // check the extension and work out the pathname of the sprite file and the texture file
    // currently it works if the input path is either a sprite file or a texture file
    const char* extension = PathUtil::GetExt(gamePath.c_str());

    string spritePath;
    string texturePath;
    if (strcmp(extension, spriteExtension) == 0)
    {
        spritePath = gamePath;

        // look for a texture file with the same name
        if (!ReplaceSpriteExtensionWithTextureExtension(spritePath, texturePath))
        {
            gEnv->pSystem->Warning(VALIDATOR_MODULE_SHINE, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE,
                gamePath.c_str(), "No texture file found for sprite: %s, no sprite will be used", gamePath.c_str());
            return nullptr;
        }
    }
    else if (IsValidSpriteTextureExtension(extension))
    {
        texturePath = gamePath;
        spritePath = PathUtil::ReplaceExtension(gamePath, spriteExtension);
    }
    else
    {
        gEnv->pSystem->Warning(VALIDATOR_MODULE_SHINE, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE,
            gamePath.c_str(), "Invalid file extension for sprite: %s, no sprite will be used", gamePath.c_str());
        return nullptr;
    }

    // test if the sprite is already loaded, if so return loaded sprite
    auto result = s_loadedSprites->find(spritePath);
    CSprite* loadedSprite = (result == s_loadedSprites->end()) ? nullptr : result->second;

    if (loadedSprite)
    {
        loadedSprite->AddRef();
        return loadedSprite;
    }

    // load the texture file
    uint32 loadTextureFlags = (FT_USAGE_ALLOWREADSRGB | FT_DONT_STREAM);
    ITexture* texture = gEnv->pRenderer->EF_LoadTexture(texturePath.c_str(), loadTextureFlags);

    if (!texture || !texture->IsTextureLoaded())
    {
        gEnv->pSystem->Warning(
            VALIDATOR_MODULE_SHINE,
            VALIDATOR_WARNING,
            VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE,
            texturePath.c_str(),
            "No texture file found for sprite: %s, no sprite will be used. "
            "NOTE: File must be in current project or a gem.",
            spritePath.c_str());
        return nullptr;
    }

    // create Sprite object
    CSprite* sprite = new CSprite;

    sprite->m_texture = texture;
    sprite->m_pathname = spritePath;
    sprite->m_texturePathname = texturePath;

    // try to load the sprite side-car file if it exists, it is optional and if it does not
    // exist we just stay with default values
    AZ::IO::FileIOBase* fileIO = AZ::IO::FileIOBase::GetInstance();
    if (fileIO && fileIO->Exists(sprite->m_pathname.c_str()))
    {
        sprite->LoadFromXmlFile();
    }

    // add sprite to list of loaded sprites
    (*s_loadedSprites)[sprite->m_pathname] = sprite;

    return sprite;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
CSprite* CSprite::CreateSprite(const string& renderTargetName)
{
    // test if the sprite is already loaded, if so return loaded sprite
    auto result = s_loadedSprites->find(renderTargetName);
    CSprite* loadedSprite = (result == s_loadedSprites->end()) ? nullptr : result->second;

    if (loadedSprite)
    {
        loadedSprite->AddRef();
        return loadedSprite;
    }

    // create Sprite object
    CSprite* sprite = new CSprite;

    sprite->m_texture = nullptr;
    sprite->m_pathname = renderTargetName;
    sprite->m_texturePathname.clear();

    // add sprite to list of loaded sprites
    (*s_loadedSprites)[sprite->m_pathname] = sprite;

    return sprite;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CSprite::ReplaceSprite(ISprite** baseSprite, ISprite* newSprite)
{
    if (baseSprite)
    {
        if (newSprite)
        {
            newSprite->AddRef();
        }

        SAFE_RELEASE(*baseSprite);

        *baseSprite = newSprite;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSprite::CellIndexWithinRange(int cellIndex) const
{
    return cellIndex >= 0 && cellIndex < m_spriteSheetCells.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSprite::LoadFromXmlFile()
{
    XmlNodeRef root = GetISystem()->LoadXmlFromFile(m_pathname.c_str());

    if (!root)
    {
        gEnv->pSystem->Warning(VALIDATOR_MODULE_SHINE, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE,
            m_pathname.c_str(),
            "No sprite file found for sprite: %s, default sprite values will be used", m_pathname.c_str());
        return false;
    }

    std::unique_ptr<IXmlSerializer> pSerializer(GetISystem()->GetXmlUtils()->CreateXmlSerializer());
    ISerialize* pReader = pSerializer->GetReader(root);

    TSerialize ser = TSerialize(pReader);

    uint32 versionNumber = spriteFileVersionNumber;
    ser.Value(spriteVersionNumberTag, versionNumber);
    const bool validVersionNumber = versionNumber >= 1 && versionNumber <= spriteFileVersionNumber;
    if (!validVersionNumber)
    {
        gEnv->pSystem->Warning(VALIDATOR_MODULE_SHINE, VALIDATOR_WARNING, VALIDATOR_FLAG_FILE | VALIDATOR_FLAG_TEXTURE,
            m_pathname.c_str(),
            "Unsupported version number found for sprite file: %s, default sprite values will be used",
            m_pathname.c_str());
        return false;
    }

    // The serializer doesn't seem to have good support for parsing a variable
    // number of tags of the same type, so we count up the children ourselves
    // before starting serialization.
    m_numSpriteSheetCellTags = GetNumSpriteSheetCellTags(root);
    Serialize(ser);

    return true;
}