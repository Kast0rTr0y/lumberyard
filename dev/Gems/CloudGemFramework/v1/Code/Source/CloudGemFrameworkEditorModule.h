/*
* All or portions of this file Copyright(c) Amazon.com, Inc.or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include "CloudGemFramework_precompiled.h"

#include <CloudGemFrameworkModule.h>

namespace CloudGemFramework
{
    class CloudGemFrameworkEditorModule
        : public CloudGemFrameworkModule
    {
    public:
        AZ_RTTI(CloudGemFrameworkEditorModule, "{335D54E9-C076-478B-9848-43381A5FF954}", CloudGemFrameworkModule);

        CloudGemFrameworkEditorModule();

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override;
    };
}


