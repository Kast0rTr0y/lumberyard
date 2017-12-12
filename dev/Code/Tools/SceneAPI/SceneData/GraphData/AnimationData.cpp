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

#include <SceneAPI/SceneData/GraphData/AnimationData.h>

namespace AZ
{
    namespace SceneData
    {
        namespace GraphData
        {
            AnimationData::AnimationData()
                : m_timeStepBetweenFrames(1/30.0) // default value
            {
            }

            void AnimationData::AddKeyFrame(const Transform& keyFrameTransform)
            {
                m_keyFrames.push_back(keyFrameTransform);
            }

            void AnimationData::ReserveKeyFrames(size_t count)
            {
                m_keyFrames.reserve(count);
            }

            void AnimationData::SetTimeStepBetweenFrames(double timeStep)
            {
                m_timeStepBetweenFrames = timeStep;
            }

            size_t AnimationData::GetKeyFrameCount() const
            {
                return m_keyFrames.size();
            }

            const Transform& AnimationData::GetKeyFrame(size_t index) const
            {
                AZ_Assert(index < m_keyFrames.size(), "GetTranslationKeyFrame index %i is out of range for frame size %i", index, m_keyFrames.size());
                return m_keyFrames[index];
            }

            double AnimationData::GetTimeStepBetweenFrames() const
            {
                return m_timeStepBetweenFrames;
            }

        }
    }
}