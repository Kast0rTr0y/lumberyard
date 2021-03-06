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

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include "ImGuiEditorWindowSystemComponent.h"
#include "ImGuiMainWindow.h"
#include <AzToolsFramework/API/ViewPaneOptions.h>

static const char* s_ImGuiQtViewPaneName = "ImGui Editor";

namespace ImGui
{
    void ImGuiEditorWindowSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<ImGuiEditorWindowSystemComponent, AZ::Component>()
                ->Version(0)
            ;
        }
    }

    void ImGuiEditorWindowSystemComponent::Activate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        ImGuiEditorWindowRequestBus::Handler::BusConnect();
        ImGuiUpdateListenerBus::Handler::BusConnect();
    }

    void ImGuiEditorWindowSystemComponent::Deactivate()
    {
        ImGuiUpdateListenerBus::Handler::BusDisconnect();
        ImGuiEditorWindowRequestBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
    }

    ImGuiEditorWindowSystemComponent::~ImGuiEditorWindowSystemComponent()
    {
        EBUS_EVENT(AzToolsFramework::EditorRequests::Bus, UnregisterViewPane, s_ImGuiQtViewPaneName);
    }

    void ImGuiEditorWindowSystemComponent::OnOpenEditorWindow()
    {
        EBUS_EVENT(AzToolsFramework::EditorRequests::Bus, OpenViewPane, s_ImGuiQtViewPaneName);
    }

    void ImGuiEditorWindowSystemComponent::NotifyRegisterViews()
    {
#if !defined(OTHER_ACTIVE)
        // The Tools->ImGui menu currently crashes trying to enable a render pipeline when this is invoked
        // Disable this menu item for now
        // [GFX TODO][ATOM-4607]
        QtViewOptions options;
        options.canHaveMultipleInstances = false;
        AzToolsFramework::EditorRequests::Bus::Broadcast(
            &AzToolsFramework::EditorRequests::RegisterViewPane,
            s_ImGuiQtViewPaneName,
            "Tools",
            options,
            [](QWidget* parent = nullptr) { return new ImGui::ImGuiMainWindow(parent); });
#endif
    }
}
