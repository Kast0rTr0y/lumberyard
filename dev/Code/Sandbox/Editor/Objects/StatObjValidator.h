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
// Original file Copyright Crytek GMBH or its affiliates, used under license.

// This class is supposed to validate CGF with assigned material.
// Some of the asset issues may be diagnosed only when SurfaceType is known.

#ifndef CRYINCLUDE_EDITOR_OBJECTS_STATOBJVALIDATOR_H
#define CRYINCLUDE_EDITOR_OBJECTS_STATOBJVALIDATOR_H
#pragma once

class CStatObjValidator
{
public:
    CStatObjValidator();

    void Validate(IStatObj* statObj, CMaterial* editorMaterial, IPhysicalEntity* physEntity);
    bool IsValid() const { return m_isValid; }

    QString GetDescription() const { return m_description; }
private:
    bool m_isValid;
    QString m_description;
};

#endif // CRYINCLUDE_EDITOR_OBJECTS_STATOBJVALIDATOR_H
