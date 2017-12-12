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

#include "StdAfx.h"
#include <AzCore/Debug/Profiler.h>
#include <IJobManager_JobDelegator.h>

namespace
{
    static void cryAsyncMemcpy_Int(
        void* dst
        , const void* src
        , size_t size
        , int nFlags
        , volatile int* sync)
    {
        AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::System);

        cryMemcpy(dst, src, size, nFlags);
        if (sync)
        {
            CryInterlockedDecrement(sync);
        }
    }
}

DECLARE_JOB("CryAsyncMemcpy", TCryAsyncMemcpy, cryAsyncMemcpy_Int);

#if !defined(CRY_ASYNC_MEMCPY_DELEGATE_TO_CRYSYSTEM)
CRY_ASYNC_MEMCPY_API void cryAsyncMemcpy(
#else
CRY_ASYNC_MEMCPY_API void cryAsyncMemcpyDelegate(
#endif
    void* dst
    , const void* src
    , size_t size
    , int nFlags
    , volatile int* sync)
{
    TCryAsyncMemcpy job(dst, src, size, nFlags, sync);
    job.Run();
}



