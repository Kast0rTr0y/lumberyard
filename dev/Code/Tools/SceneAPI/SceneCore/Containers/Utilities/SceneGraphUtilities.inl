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

#include <AzCore/std/algorithm.h>
#include <AzCore/std/typetraits/is_base_of.h>
#include <SceneAPI/SceneCore/Containers/Scene.h>
#include <SceneAPI/SceneCore/Containers/SceneGraph.h>
#include <SceneAPI/SceneCore/Containers/Views/FilterIterator.h>
#include <SceneAPI/SceneCore/Containers/Utilities/Filters.h>
#include <SceneAPI/SceneCore/Events/GraphMetaInfoBus.h>

namespace AZ
{
    namespace SceneAPI
    {
        namespace Utilities
        {
            template<typename T>
            bool DoesSceneGraphContainDataLike(const Containers::Scene& scene, bool checkVirtualTypes)
            {
                static_assert(AZStd::is_base_of<DataTypes::IGraphObject, T>::value, "Specified type T is not derived from IGraphObject.");

                const Containers::SceneGraph& graph = scene.GetGraph();
                if (checkVirtualTypes)
                {
                    auto contentStorage = graph.GetContentStorage();
                    auto view = Containers::Views::MakeFilterView(contentStorage, Containers::DerivedTypeFilter<T>());
                    for (auto it = view.begin(); it != view.end(); ++it)
                    {
                        AZStd::set<Crc32> types;
                        EBUS_EVENT(Events::GraphMetaInfoBus, GetVirtualTypes, types, scene, graph.ConvertToNodeIndex(it.GetBaseIterator()));
                        // Check if the type is not a virtual type. If it is, this isn't a valid type for T.
                        if (types.empty())
                        {
                            return true;
                        }
                    }
                    return false;
                }
                else
                {
                    Containers::SceneGraph::ContentStorageConstData graphContent = graph.GetContentStorage();
                    auto data = AZStd::find_if(graphContent.begin(), graphContent.end(), Containers::DerivedTypeFilter<T>());
                    return data != graphContent.end();
                }
            }
        } // Utilities
    } // SceneAPI
} // AZ
