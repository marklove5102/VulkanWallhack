//Vulkan Hook 2026

#include <Windows.h>
#include <iostream>
#include <assert.h>
#include <cstdint>
#include <cassert>
#include <cstdio>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>
#include <map>
#include <algorithm>
#include "vulkan/vulkan.h"
#include "minhook/include/MinHook.h"
#pragma comment(lib, "vulkan/vulkan-1.lib")

//=======================================================================================//

#include "main.h"

//=======================================================================================//
// Typedefs
typedef PFN_vkVoidFunction(VKAPI_PTR* PFN_vkGetDeviceProcAddr)(VkDevice, const char*);
typedef PFN_vkVoidFunction(VKAPI_PTR* PFN_vkGetInstanceProcAddr)(VkInstance, const char*);

// Core draw functions
typedef void (VKAPI_PTR* PFN_vkCmdDraw)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndexed)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndexedIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndirectCount)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndexedIndirectCount)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize, uint32_t, uint32_t);

// EXT / Advanced draw functions
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndirectByteCountEXT)(VkCommandBuffer, uint32_t, uint32_t, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMeshTasksEXT)(VkCommandBuffer, uint32_t, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMeshTasksIndirectEXT)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMeshTasksIndirectCountEXT)(VkCommandBuffer, VkBuffer, VkDeviceSize, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMultiEXT)(VkCommandBuffer, uint32_t, const VkMultiDrawInfoEXT*, uint32_t, uint32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMultiIndexedEXT)(VkCommandBuffer, uint32_t, const VkMultiDrawIndexedInfoEXT*, uint32_t, uint32_t, uint32_t, const int32_t*);

// Dynamic state
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthCompareOp)(VkCommandBuffer, VkCompareOp);
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthWriteEnable)(VkCommandBuffer, VkBool32);
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthTestEnable)(VkCommandBuffer, VkBool32);
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthBias)(VkCommandBuffer, float, float, float);
typedef void (VKAPI_PTR* PFN_vkCmdSetViewport_Custom)(VkCommandBuffer, uint32_t, uint32_t, const VkViewport*);
typedef void (VKAPI_PTR* PFN_vkCmdBindVertexBuffers2)(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*, const VkDeviceSize*, const VkDeviceSize*);
typedef void (VKAPI_PTR* PFN_vkCmdExecuteCommands)(VkCommandBuffer, uint32_t, const VkCommandBuffer*);
typedef VkResult(VKAPI_PTR* PFN_vkCreateGraphicsPipelines)(VkDevice, VkPipelineCache, uint32_t, const VkGraphicsPipelineCreateInfo*, const VkAllocationCallbacks*, VkPipeline*);
typedef void(VKAPI_PTR* PFN_vkCmdBindPipeline)(VkCommandBuffer, VkPipelineBindPoint, VkPipeline);
typedef void (VKAPI_PTR* PFN_vkCmdDrawMultiIndexedIndirectEXT)(VkCommandBuffer, uint32_t, VkBuffer, VkDeviceSize, uint32_t, const uint32_t*);

//=======================================================================================//
// Storage / Original Pointers
PFN_vkGetDeviceProcAddr pOriginalGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr pOriginalGetInstanceProcAddr = nullptr;

PFN_vkCmdDraw pOriginalCmdDraw = nullptr;
PFN_vkCmdDrawIndexed pOriginalCmdDrawIndexed = nullptr;
PFN_vkCmdDrawIndirect pOriginalCmdDrawIndirect = nullptr;
PFN_vkCmdDrawIndexedIndirect pOriginalCmdDrawIndexedIndirect = nullptr;
PFN_vkCmdDrawIndirectCount pOriginalCmdDrawIndirectCount = nullptr;
PFN_vkCmdDrawIndexedIndirectCount pOriginalCmdDrawIndexedIndirectCount = nullptr;

PFN_vkCmdDrawIndirectByteCountEXT pOriginalCmdDrawIndirectByteCountEXT = nullptr;
PFN_vkCmdDrawMeshTasksEXT pOriginalCmdDrawMeshTasksEXT = nullptr;
PFN_vkCmdDrawMeshTasksIndirectEXT pOriginalCmdDrawMeshTasksIndirectEXT = nullptr;
PFN_vkCmdDrawMeshTasksIndirectCountEXT pOriginalCmdDrawMeshTasksIndirectCountEXT = nullptr;
PFN_vkCmdDrawMultiEXT pOriginalCmdDrawMultiEXT = nullptr;
PFN_vkCmdDrawMultiIndexedEXT pOriginalCmdDrawMultiIndexedEXT = nullptr;
PFN_vkCmdDrawMultiIndexedIndirectEXT pOriginalCmdDrawMultiIndexedIndirectEXT = nullptr;

PFN_vkCmdSetViewport_Custom pOriginalvkCmdSetViewport = nullptr;
PFN_vkCmdBindVertexBuffers2 pOriginalBindVertexBuffers2 = nullptr;
PFN_vkCmdExecuteCommands pOriginalCmdExecuteCommands = nullptr;
PFN_vkCreateGraphicsPipelines pOriginalCreateGraphicsPipelines = nullptr;
PFN_vkCmdBindPipeline pOriginalCmdBindPipeline = nullptr;

PFN_vkCmdSetDepthCompareOp gp_vkCmdSetDepthCompareOp = nullptr;
PFN_vkCmdSetDepthWriteEnable gp_vkCmdSetDepthWriteEnable = nullptr;
PFN_vkCmdSetDepthTestEnable gp_vkCmdSetDepthTestEnable = nullptr;
PFN_vkCmdSetDepthBias gp_vkCmdSetDepthBias = nullptr;

//=======================================================================================//
// Detour Functions

// Pipeline Creation Hook
VkResult VKAPI_CALL DetourCreateGraphicsPipelines(
    VkDevice device, VkPipelineCache cache, uint32_t count,
    const VkGraphicsPipelineCreateInfo* pCreateInfos,
    const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines)
{
    std::vector<VkGraphicsPipelineCreateInfo> modifiedInfos(pCreateInfos, pCreateInfos + count);
    std::vector<VkPipelineDynamicStateCreateInfo> dynamicStateStructs(count);
    std::vector<std::vector<VkDynamicState>> dynamicStatesArrays(count);

    // Temporary storage for settings to save after successful creation
    std::vector<PipelineSettings> capturedSettings(count);

    for (uint32_t i = 0; i < count; ++i) {
        auto& ci = modifiedInfos[i];

        // 1. Capture Original States
        capturedSettings[i].stride = 0;
        if (ci.pVertexInputState && ci.pVertexInputState->vertexBindingDescriptionCount > 0) {
            capturedSettings[i].stride = ci.pVertexInputState->pVertexBindingDescriptions[0].stride;
        }

        if (ci.pDepthStencilState) {
            capturedSettings[i].originalDepthTestEnable = ci.pDepthStencilState->depthTestEnable;
            capturedSettings[i].originalDepthWriteEnable = ci.pDepthStencilState->depthWriteEnable;
            capturedSettings[i].originalDepthCompareOp = ci.pDepthStencilState->depthCompareOp;
        }
        else {
            capturedSettings[i].originalDepthTestEnable = VK_FALSE;
            capturedSettings[i].originalDepthWriteEnable = VK_FALSE;
            capturedSettings[i].originalDepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        }

        // 2. Force Dynamic States
        if (ci.pDynamicState) {
            dynamicStatesArrays[i].assign(
                ci.pDynamicState->pDynamicStates,
                ci.pDynamicState->pDynamicStates + ci.pDynamicState->dynamicStateCount
            );
        }

        auto addState = [&](VkDynamicState s) {
            for (auto existing : dynamicStatesArrays[i]) if (existing == s) return;
            dynamicStatesArrays[i].push_back(s);
            };
        addState(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
        addState(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
        addState(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
        addState(VK_DYNAMIC_STATE_VIEWPORT); // Ensure viewport is dynamic too

        dynamicStateStructs[i].sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateStructs[i].pNext = nullptr;
        dynamicStateStructs[i].flags = 0;
        dynamicStateStructs[i].pDynamicStates = dynamicStatesArrays[i].data();
        dynamicStateStructs[i].dynamicStateCount = static_cast<uint32_t>(dynamicStatesArrays[i].size());

        ci.pDynamicState = &dynamicStateStructs[i];
    }

    VkResult res = pOriginalCreateGraphicsPipelines(device, cache, count, modifiedInfos.data(), pAllocator, pPipelines);

    if (res == VK_SUCCESS) {
        std::unique_lock<std::shared_mutex> lock(pipeMapMtx);
        for (uint32_t i = 0; i < count; ++i) {
            if (pPipelines[i] != VK_NULL_HANDLE) {
                // Save EVERYTHING to one map to stay synced
                pipelineData[pPipelines[i]] = capturedSettings[i];
            }
        }
    }
    return res;
}

// Pipeline Bind Hook
void VKAPI_CALL DetourCmdBindPipeline(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipeline pipeline) {
    pOriginalCmdBindPipeline(cmd, bindPoint, pipeline);

    if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        std::shared_lock<std::shared_mutex> lock(pipeMapMtx);
        auto it = pipelineData.find(pipeline);
        if (it != pipelineData.end()) {
            // Set the stride for the current command buffer
            cmdBufferToStride[cmd] = it->second.stride;

            // Set default depth states for this pipeline
            gp_vkCmdSetDepthTestEnable(cmd, it->second.originalDepthTestEnable);
            gp_vkCmdSetDepthWriteEnable(cmd, it->second.originalDepthWriteEnable);
            gp_vkCmdSetDepthCompareOp(cmd, it->second.originalDepthCompareOp);
        }
    }
}

//=======================================================================================//

void VKAPI_CALL DetourVkCmdSetViewport(VkCommandBuffer cmd, uint32_t first, uint32_t count, const VkViewport* pVp) {
    //Log("4 VkCmdSetViewport hooked");
    if (pVp != nullptr && count > 0) {
        std::unique_lock<std::shared_mutex> lock(statesMtx);
        CmdState& state = cmdStates[cmd];
        state.currentViewport = pVp[0];
        state.firstViewport = first;
        state.hasViewport = true;
    }
    if (pOriginalvkCmdSetViewport) {
        pOriginalvkCmdSetViewport(cmd, first, count, pVp);
    }
}

void VKAPI_CALL DetourVkCmdDrawIndexed(VkCommandBuffer cmd, uint32_t idxCount, uint32_t instCount, uint32_t firstIdx, int32_t vtxOff, uint32_t firstInst) {
    //Log("1 VkCmdDrawIndexed hooked");
    
    uint32_t currentStride = 0;
    {
        // Get the stride we saved during BindPipeline
        std::shared_lock<std::shared_mutex> lock(pipeMapMtx); // Use same mutex
        auto it = cmdBufferToStride.find(cmd);
        if (it != cmdBufferToStride.end()) currentStride = it->second;
    }

    // Identify monsters via Stride or IndexCount
    // If your logger found 'countnum' matches currentStride, hack it!
    bool isMonster = (currentStride == countnum || (idxCount / 100) == countnum);

    if (isMonster) {
        gp_vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        gp_vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);

        pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);

        // Restore
        gp_vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        gp_vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS_OR_EQUAL);
    }
    else {
        pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);
    }
    
    return pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);
}

void VKAPI_CALL DetourVkCmdDrawIndexedIndirect(
    VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
    //Log("2 VkCmdDrawIndexedIndirect hooked");
    /*
    uint32_t currentStride = 0;
    {
        // Get the stride we saved during BindPipeline
        std::shared_lock<std::shared_mutex> lock(pipeMapMtx); // Use same mutex
        auto it = cmdBufferToStride.find(cmd);
        if (it != cmdBufferToStride.end()) currentStride = it->second;
    }

    uint32_t shortCount = (drawCount >> 12) % 100;
    uint32_t shortbuffer = ((uintptr_t)buffer >> 12) % 100;
    uint32_t shortOffset = (offset >> 8) % 100; // or >> 12

    // Identify monsters via Stride or IndexCount
    // If your logger found 'countnum' matches currentStride, hack it!
    bool isMonster = (currentStride == countnum || shortCount == countnum|| shortbuffer == countnum|| shortOffset==countnum);

    if (isMonster) {
        gp_vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        gp_vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);

        pOriginalCmdDrawIndexedIndirect(cmd, buffer, offset, drawCount, stride);

        // Restore
        gp_vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        gp_vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS_OR_EQUAL);
    }
    else {
        pOriginalCmdDrawIndexedIndirect(cmd, buffer, offset, drawCount, stride);
    }
    */
    return pOriginalCmdDrawIndexedIndirect(cmd, buffer, offset, drawCount, stride);
}

void VKAPI_CALL DetourVkCmdDrawIndexedIndirectCount(
    VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset,
    VkBuffer countBuffer, VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount, uint32_t stride)
{
    //Log("3 VkCmdDrawIndexedIndirectCount hooked");
    auto pOrig = pOriginalCmdDrawIndexedIndirectCount;
    if (!pOrig) return;
    
    CmdState localState;
    bool found = false;
    {
        std::shared_lock<std::shared_mutex> lock(statesMtx);
        auto it = cmdStates.find(cmd);
        if (it != cmdStates.end()) {
            localState = it->second;
            found = true;
        }
    }

    // Identify the draw call
    uint32_t currentStride = 0;
    {
        std::shared_lock<std::shared_mutex> lock(pipeMapMtx);
        auto it = cmdBufferToStride.find(cmd);
        if (it != cmdBufferToStride.end()) currentStride = it->second;
    }

    // Selection logic
    uint32_t shortCount = (maxDrawCount >> 12) % 100;
    uint32_t shortbuffer = ((uintptr_t)buffer >> 12) % 100;
    uint32_t shortOffset = (offset >> 8) % 100; // or >> 12
    uint32_t shortCountBuffer = ((uintptr_t)countBuffer >> 12) % 100;
    uint32_t shortCountOffset = (countBufferOffset >> 8) % 50;

    bool shouldApplyDepthHack = (currentStride == countnum || shortCount == countnum || shortbuffer == countnum || 
        shortOffset == countnum || shortCountBuffer == countnum || shortCountOffset == countnum);

    if (found && shouldApplyDepthHack && localState.hasViewport) {
        // --- APPLY HACK ---
        const VkViewport originalVp = localState.currentViewport;
        VkViewport hVp = originalVp;

        // Depth range adjustment
        hVp.minDepth = reversedDepth ? 0.0f : 0.9f;
        hVp.maxDepth = reversedDepth ? 0.1f : 1.0f;

        // Disable depth testing to see through walls
        //gp_vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        //gp_vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
        //gp_vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);

        pOriginalvkCmdSetViewport(cmd, localState.firstViewport, 1, &hVp);

        // Draw the hacked version
        pOrig(cmd, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);

        // --- RESTORE STATE ---
        // Instead of hardcoding, we just reset to standard behavior or let the next call handle it
        //gp_vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        //gp_vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
        //gp_vkCmdSetDepthCompareOp(cmd, reversedDepth ? VK_COMPARE_OP_GREATER_OR_EQUAL : VK_COMPARE_OP_LESS_OR_EQUAL);
        pOriginalvkCmdSetViewport(cmd, localState.firstViewport, 1, &originalVp);
    }
    else {
        // Draw normally if it's not the target object
        pOrig(cmd, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }
    
    // Logic for finding the 'countnum' ID
    if (GetAsyncKeyState('O') & 1) countnum--;
    if (GetAsyncKeyState('P') & 1) countnum++;
    pOrig(cmd, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void VKAPI_CALL DetourBindVertexBuffers2(
    VkCommandBuffer cmd, uint32_t firstBinding, uint32_t bindingCount,
    const VkBuffer* pBuffers, const VkDeviceSize* pOffsets,
    const VkDeviceSize* pSizes, const VkDeviceSize* pStrides)
{
    //Log("A");
    pOriginalBindVertexBuffers2(cmd, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

void VKAPI_CALL DetourVkCmdDraw(VkCommandBuffer cmd, uint32_t vCount, uint32_t iCount, uint32_t firstV, uint32_t firstI) {
    //Log("1");
    pOriginalCmdDraw(cmd, vCount, iCount, firstV, firstI);
}

void VKAPI_CALL DetourVkCmdDrawIndirect(VkCommandBuffer cmd, VkBuffer buf, VkDeviceSize off, uint32_t drawCount, uint32_t stride) {
    //Log("2");
    pOriginalCmdDrawIndirect(cmd, buf, off, drawCount, stride);
}

void VKAPI_CALL DetourVkCmdDrawIndirectCount(VkCommandBuffer cmd, VkBuffer buf, VkDeviceSize off, VkBuffer cntBuf, VkDeviceSize cntOff, uint32_t maxDraw, uint32_t stride) {
    auto pOrig2 = pOriginalCmdDrawIndirectCount;
    if (!pOrig2) return;
    //Log("3");
    pOriginalCmdDrawIndirectCount(cmd, buf, off, cntBuf, cntOff, maxDraw, stride);
}

void VKAPI_CALL DetourCmdExecuteCommands(VkCommandBuffer containerCmd, uint32_t count, const VkCommandBuffer* pCmds) {
    //Log("4");
    pOriginalCmdExecuteCommands(containerCmd, count, pCmds);
}

void VKAPI_CALL DetourVkCmdDrawMeshTasksEXT(
    VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
    if (!pOriginalCmdDrawMeshTasksEXT) return;
    //Log("5");
    pOriginalCmdDrawMeshTasksEXT(cmd, groupCountX, groupCountY, groupCountZ);
}

void VKAPI_CALL DetourVkCmdDrawMeshTasksIndirectCountEXT(
    VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset,
    VkBuffer countBuffer, VkDeviceSize countBufferOffset,
    uint32_t maxDrawCount, uint32_t stride)
{
    if (!pOriginalCmdDrawMeshTasksIndirectCountEXT) return;
    //Log("6");
    pOriginalCmdDrawMeshTasksIndirectCountEXT(cmd, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}

void VKAPI_CALL DetourVkCmdDrawMultiIndexedIndirectEXT(
    VkCommandBuffer cmd, uint32_t drawCount, VkBuffer buffer,
    VkDeviceSize offset, uint32_t stride, const uint32_t* pIndexCounts)
{
    //Log("7");
    pOriginalCmdDrawMultiIndexedIndirectEXT(cmd, drawCount, buffer, offset, stride, pIndexCounts);
}

// New simple detours for all requested functions
void VKAPI_CALL DetourVkCmdDrawIndirectByteCountEXT(VkCommandBuffer cmd, uint32_t instanceCount, uint32_t firstInstance,
    VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride) {
    //Log("8 VkCmdDrawIndirectByteCountEXT hooked");
    pOriginalCmdDrawIndirectByteCountEXT(cmd, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}

void VKAPI_CALL DetourVkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) {
    //Log("9 VkCmdDrawMeshTasksIndirectEXT hooked");
    pOriginalCmdDrawMeshTasksIndirectEXT(cmd, buffer, offset, drawCount, stride);
}

void VKAPI_CALL DetourVkCmdDrawMultiEXT(VkCommandBuffer cmd, uint32_t drawCount, const VkMultiDrawInfoEXT* pVertexInfo,
    uint32_t instanceCount, uint32_t firstInstance, uint32_t stride) {
    //Log("10 VkCmdDrawMultiEXT hooked");
    pOriginalCmdDrawMultiEXT(cmd, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}

void VKAPI_CALL DetourVkCmdDrawMultiIndexedEXT(VkCommandBuffer cmd, uint32_t drawCount, const VkMultiDrawIndexedInfoEXT* pIndexInfo,
    uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, const int32_t* pVertexOffset) {
    //Log("11 VkCmdDrawMultiIndexedEXT hooked");
    pOriginalCmdDrawMultiIndexedEXT(cmd, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}

//=======================================================================================//
// GetDeviceProcAddr Hook
PFN_vkVoidFunction VKAPI_CALL DetourGetDeviceProcAddr(VkDevice device, const char* pName) {
    if (!pName) return pOriginalGetDeviceProcAddr(device, pName);

    PFN_vkVoidFunction realFunc = pOriginalGetDeviceProcAddr(device, pName);
    if (!realFunc) return nullptr;

    // Core draw functions
    if (strcmp(pName, "vkCmdDraw") == 0) {
        pOriginalCmdDraw = (PFN_vkCmdDraw)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDraw;
    }
    if (strcmp(pName, "vkCmdDrawIndexed") == 0) {
        pOriginalCmdDrawIndexed = (PFN_vkCmdDrawIndexed)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndexed;
    }
    if (strcmp(pName, "vkCmdDrawIndirect") == 0) {
        pOriginalCmdDrawIndirect = (PFN_vkCmdDrawIndirect)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndirect;
    }
    if (strcmp(pName, "vkCmdDrawIndexedIndirect") == 0) {
        pOriginalCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndexedIndirect;
    }
    if (strstr(pName, "vkCmdDrawIndirectCount")) {
        pOriginalCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndirectCount;
    }
    if (strstr(pName, "vkCmdDrawIndexedIndirectCount")) {
        pOriginalCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndexedIndirectCount;
    }

    // EXT functions
    if (strcmp(pName, "vkCmdDrawIndirectByteCountEXT") == 0) {
        pOriginalCmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndirectByteCountEXT;
    }
    if (strstr(pName, "vkCmdDrawMeshTasksEXT") && !strstr(pName, "Indirect")) {
        pOriginalCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawMeshTasksEXT;
    }
    if (strcmp(pName, "vkCmdDrawMeshTasksIndirectEXT") == 0) {
        pOriginalCmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawMeshTasksIndirectEXT;
    }
    if (strstr(pName, "vkCmdDrawMeshTasksIndirectCountEXT")) {
        pOriginalCmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawMeshTasksIndirectCountEXT;
    }
    if (strcmp(pName, "vkCmdDrawMultiEXT") == 0) {
        pOriginalCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawMultiEXT;
    }
    if (strcmp(pName, "vkCmdDrawMultiIndexedEXT") == 0) {
        pOriginalCmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawMultiIndexedEXT;
    }

    // Other important hooks
    if (strcmp(pName, "vkCmdSetViewport") == 0 || strcmp(pName, "vkCmdSetViewportEXT") == 0) {
        pOriginalvkCmdSetViewport = (PFN_vkCmdSetViewport_Custom)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdSetViewport;
    }
    if (strcmp(pName, "vkCreateGraphicsPipelines") == 0) {
        pOriginalCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)realFunc;
        return (PFN_vkVoidFunction)DetourCreateGraphicsPipelines;
    }
    if (strcmp(pName, "vkCmdBindPipeline") == 0) {
        pOriginalCmdBindPipeline = (PFN_vkCmdBindPipeline)realFunc;
        return (PFN_vkVoidFunction)DetourCmdBindPipeline;
    }
    if (strcmp(pName, "vkCmdBindVertexBuffers2") == 0 || strcmp(pName, "vkCmdBindVertexBuffers2EXT") == 0) {
        pOriginalBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)realFunc;
        return (PFN_vkVoidFunction)DetourBindVertexBuffers2;
    }
    if (strcmp(pName, "vkCmdExecuteCommands") == 0) {
        pOriginalCmdExecuteCommands = (PFN_vkCmdExecuteCommands)realFunc;
        return (PFN_vkVoidFunction)DetourCmdExecuteCommands;
    }

    // Dynamic state setters
    if (strcmp(pName, "vkCmdSetDepthTestEnable") == 0 || strcmp(pName, "vkCmdSetDepthTestEnableEXT") == 0) {
        gp_vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)realFunc;
        return realFunc;
    }
    if (strcmp(pName, "vkCmdSetDepthWriteEnable") == 0 || strcmp(pName, "vkCmdSetDepthWriteEnableEXT") == 0) {
        gp_vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)realFunc;
        return realFunc;
    }
    if (strcmp(pName, "vkCmdSetDepthCompareOp") == 0 || strcmp(pName, "vkCmdSetDepthCompareOpEXT") == 0) {
        gp_vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)realFunc;
        return realFunc;
    }
    if (strcmp(pName, "vkCmdSetDepthBias") == 0) {
        gp_vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)realFunc;
        return realFunc;
    }

    return realFunc;
}

//=======================================================================================//
// GetInstanceProcAddr Hook
PFN_vkVoidFunction VKAPI_CALL DetourGetInstanceProcAddr(VkInstance instance, const char* pName) {
    PFN_vkVoidFunction realFunc = pOriginalGetInstanceProcAddr(instance, pName);
    if (!pName) return realFunc;

    if (realFunc && pName) {
        if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction)DetourGetDeviceProcAddr;
        }
        if (strcmp(pName, "vkCmdDrawIndexed") == 0) {
            //Log("Caught in GetInstanceProcAddr");
            pOriginalCmdDrawIndexed = (PFN_vkCmdDrawIndexed)realFunc;
            return (PFN_vkVoidFunction)DetourVkCmdDrawIndexed;
        }
    }
    return realFunc;
}

//=======================================================================================//
// Hook Thread
DWORD WINAPI HookThread(LPVOID lpParam) {
    if (MH_Initialize() != MH_OK) return 1;
    HMODULE hVulkan = GetModuleHandleA("vulkan-1.dll");
    if (!hVulkan) return 1;

    void* pGetDevice = (void*)GetProcAddress(hVulkan, "vkGetDeviceProcAddr");
    void* pGetInstance = (void*)GetProcAddress(hVulkan, "vkGetInstanceProcAddr");

    if (MH_CreateHook(pGetDevice, &DetourGetDeviceProcAddr, reinterpret_cast<LPVOID*>(&pOriginalGetDeviceProcAddr)) == MH_OK) {
        MH_EnableHook(pGetDevice);
    }
    if (MH_CreateHook(pGetInstance, &DetourGetInstanceProcAddr, reinterpret_cast<LPVOID*>(&pOriginalGetInstanceProcAddr)) == MH_OK) {
        MH_EnableHook(pGetInstance);
    }
    return 0;
}

//=======================================================================================//
BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID res) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
    }
    return TRUE;
}

/*
vkCmdDraw
vkCmdDrawIndexed
vkCmdDrawIndirect
vkCmdDrawIndexedIndirect
vkCmdDrawIndirectCount
vkCmdDrawIndexedIndirectCount
vkCmdDrawIndirectByteCountEXT
vkCmdDrawMeshTasksEXT
vkCmdDrawMeshTasksIndirectEXT
vkCmdDrawMeshTasksIndirectCountEXT
vkCmdDrawMultiEXT
vkCmdDrawMultiIndexedEXT
*/