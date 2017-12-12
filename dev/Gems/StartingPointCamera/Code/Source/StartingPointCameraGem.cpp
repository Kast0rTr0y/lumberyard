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
#include <platform_impl.h>

#include "CameraTargetAcquirers/AcquireByEntityId.h"
#include "CameraTargetAcquirers/AcquireByTag.h"
#include "CameraTransformBehaviors/FollowTargetFromDistance.h"
#include "CameraLookAtBehaviors/OffsetPosition.h"
#include "CameraTransformBehaviors/FollowTargetFromAngle.h"
#include "CameraTransformBehaviors/Rotate.h"
#include "CameraTransformBehaviors/OffsetCameraPosition.h"
#include "CameraLookAtBehaviors/SlideAlongAxisBasedOnAngle.h"
#include "CameraLookAtBehaviors/RotateCameraLookAt.h"
#include "CameraTransformBehaviors/FaceTarget.h"

#include <AzCore/Module/Module.h>

#include <AzFramework/Metrics/MetricsPlainTextNameRegistration.h>

namespace StartingPointCamera
{
    struct StartingPointCameraGemComponent
        : public AZ::Component
    {
        ~StartingPointCameraGemComponent() override = default;
        AZ_COMPONENT(StartingPointCameraGemComponent, "{728DF62E-6787-4A16-8F07-8A45BECADAD7}");
        static void Reflect(AZ::ReflectContext* reflection)
        {
            Camera::AcquireByEntityId::Reflect(reflection);
            Camera::AcquireByTag::Reflect(reflection);
            Camera::FollowTargetFromDistance::Reflect(reflection);
            Camera::OffsetPosition::Reflect(reflection);
            Camera::FollowTargetFromAngle::Reflect(reflection);
            Camera::Rotate::Reflect(reflection);
            Camera::OffsetCameraPosition::Reflect(reflection);
            Camera::SlideAlongAxisBasedOnAngle::Reflect(reflection);
            Camera::RotateCameraLookAt::Reflect(reflection);
            Camera::FaceTarget::Reflect(reflection);
        }
        void Activate() override {}
        void Deactivate() override {}
    };

    class StartingPointCameraModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(StartingPointCameraModule, "{87B6E891-9C64-4C5D-9FA1-4079BF6D902D}", AZ::Module);

        StartingPointCameraModule()
            : AZ::Module()
        {
            m_descriptors.insert(m_descriptors.end(), {
                StartingPointCameraGemComponent::CreateDescriptor(),
            });






            // This is an internal Amazon gem, so register it's components for metrics tracking, otherwise the name of the component won't get sent back.
            // IF YOU ARE A THIRDPARTY WRITING A GEM, DO NOT REGISTER YOUR COMPONENTS WITH EditorMetricsComponentRegistrationBus
            AZStd::vector<AZ::Uuid> typeIds;
            typeIds.reserve(m_descriptors.size());
            for (AZ::ComponentDescriptor* descriptor : m_descriptors)
            {
                typeIds.emplace_back(descriptor->GetUuid());
            }
            EBUS_EVENT(AzFramework::MetricsPlainTextNameRegistrationBus, RegisterForNameSending, typeIds);
        }
    };
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(StartingPointCamera_834070b9537d44df83559e2045c3859f, StartingPointCamera::StartingPointCameraModule)
