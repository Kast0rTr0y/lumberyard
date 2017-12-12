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
#pragma once
#include <CameraFramework/ICameraTransformBehavior.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/RTTI/ReflectContext.h>

namespace Camera
{
    //////////////////////////////////////////////////////////////////////////
    /// Use this behavior to offset the camera's position by a fixed amount
    //////////////////////////////////////////////////////////////////////////
    class OffsetCameraPosition
        : public ICameraTransformBehavior
    {
    public:
        ~OffsetCameraPosition() override = default;
        AZ_RTTI(OffsetCameraPosition, "{DB64D5DA-84B7-45B7-B221-B5A07BDA2F69}", ICameraTransformBehavior)
        static void Reflect(AZ::ReflectContext* reflection);

        //////////////////////////////////////////////////////////////////////////
        // ICameraTransformBehavior
        void AdjustCameraTransform(float deltaTime, const AZ::Transform& initialCameraTransform, const AZ::Transform& targetTransform, AZ::Transform& inOutCameraTransform) override;
        void Activate(AZ::EntityId) override {}
        void Deactivate() override {}

    private:
        //////////////////////////////////////////////////////////////////////////
        // Reflected Data
        AZ::Vector3 m_offset = AZ::Vector3::CreateZero();
        bool m_isRelativeOffset = false;
    };
} // namespace Camera