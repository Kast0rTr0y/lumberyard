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

#include <AzCore/Component/ComponentBus.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
class UiFaderInterface
    : public AZ::ComponentBus
{
public: // member functions

    virtual ~UiFaderInterface() {}

    //! Get the fade value. This is a float between 0 and 1. 1 means no fade. 0 means complete fade to invisible.
    virtual float GetFadeValue() = 0;

    //! Set the fade value
    virtual void SetFadeValue(float fade) = 0;

    //! Trigger a fade animation.
    //! \param targetValue The value to end the fade at [0,1]
    //! \param speed       Speed measured in full fade amount per second; 0 means instant
    //! \param listener    The listener to notify when the fade is completed or interrupted
    virtual void Fade(float targetValue, float speed) = 0;

    //! Get whether a fade animation is taking place
    virtual bool IsFading() = 0;

public: // static member data

    //! Only one component on a entity can implement the events
    static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
};

typedef AZ::EBus<UiFaderInterface> UiFaderBus;

////////////////////////////////////////////////////////////////////////////////////////////////////
//! Interface class that listeners need to implement
class UiFaderNotifications
    : public AZ::ComponentBus
{
public:

    //////////////////////////////////////////////////////////////////////////
    // EBusTraits overrides
    static const bool EnableEventQueue = true;
    //////////////////////////////////////////////////////////////////////////

public: // member functions

    virtual ~UiFaderNotifications(){}

    //! Called when the animation triggered by UiFaderInterface::Fade() is done.
    //! The listener is automatically removed from the fader component after this is called.
    virtual void OnFadeComplete() = 0;

    //! Called when the animation triggered by UiFaderInterface::Fade() is interrupted.
    //! The listener is automatically removed from the fader component after this is called.
    virtual void OnFadeInterrupted() = 0;

    //! Called when the fader component is destroyed
    virtual void OnFaderDestroyed() = 0;
};

typedef AZ::EBus<UiFaderNotifications> UiFaderNotificationBus;
