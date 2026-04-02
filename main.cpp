//Vulkan Hook 2026

#include <Windows.h>
#include <iostream>
#include <assert.h>
#include <cstdint>  // For uintptr_t
#include <cassert>  // For assert()
#include <cstdio>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "vulkan/vulkan.h"
#include "minhook/include/MinHook.h"

#pragma comment(lib, "vulkan/vulkan-1.lib")

//=======================================================================================//

// Globals
bool initonce = false;
int countnum = -1;

//=======================================================================================//

#include "main.h"

//=======================================================================================//

// 1. Typedefs
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndexed)(VkCommandBuffer, uint32_t, uint32_t, uint32_t, int32_t, uint32_t);
typedef void (VKAPI_PTR* PFN_vkCmdDrawIndexedIndirect)(VkCommandBuffer, VkBuffer, VkDeviceSize, uint32_t, uint32_t);
typedef PFN_vkVoidFunction(VKAPI_PTR* PFN_vkGetDeviceProcAddr)(VkDevice, const char*);
typedef PFN_vkVoidFunction(VKAPI_PTR* PFN_vkGetInstanceProcAddr)(VkInstance, const char*);

// 2. Storage
PFN_vkGetDeviceProcAddr pOriginalGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr pOriginalGetInstanceProcAddr = nullptr;
PFN_vkCmdDrawIndexed pOriginalCmdDrawIndexed = nullptr;
PFN_vkCmdDrawIndexedIndirect pOriginalCmdDrawIndexedIndirect = nullptr;

// Define the typedef (matches both EXT and Core 1.3)
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthTestEnable)(VkCommandBuffer, VkBool32);
// Use a unique name to avoid clashing with the header's prototypes
PFN_vkCmdSetDepthTestEnable gp_vkCmdSetDepthTestEnable = nullptr;

// Typedef and global pointer
typedef void (VKAPI_PTR* PFN_vkCmdSetDepthBias)(VkCommandBuffer, float, float, float);
PFN_vkCmdSetDepthBias gp_vkCmdSetDepthBias = nullptr;


void VKAPI_CALL DetourVkCmdDrawIndexed(VkCommandBuffer cmd, uint32_t idxCount, uint32_t instCount, uint32_t firstIdx, int32_t vtxOff, uint32_t firstInst) {

    lognospam1(1, "VkCmdDrawIndexed hooked");
    if (!initonce) {
        // zbuffer
        VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
        depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        initonce = true;
    }

    /*
    //wallhack example
    if(t_.StrideHash == 23)
    if (idxCount == 765 || idxCount == 1032 || idxCount == 25698 || idxCount == 34494)
    {
        //disable z buffer
        //vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_ALWAYS);
        //vkCmdSetDepthWriteEnable(cmd, VK_FALSE);
        //vkCmdSetDepthTestEnableEXT(cmd, VK_FALSE);// Disable Depth Test (Requires Dynamic State)

        vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        vkCmdSetDepthWriteEnable(cmd, VK_FALSE);

        pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);

        //vkCmdSetDepthCompareOp(cmd, VK_COMPARE_OP_LESS);
        //vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
        //vkCmdSetDepthTestEnableEXT(cmd, VK_TRUE);//

        vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        vkCmdSetDepthWriteEnable(cmd, VK_TRUE);
    }
    */

    //logger for logging indexcount
    if (countnum == t_.StrideHash)//idxCount / 100)
        if (GetAsyncKeyState(VK_END) & 1) //press END to log to log.txt
            Log("t_.StrideHash == %d && idxCount == %d", t_.StrideHash, idxCount); //log selected values

    //wallhack selected Stridehash
    if (countnum == t_.StrideHash) {
        if (gp_vkCmdSetDepthBias) {
            // Push the model "forward" toward the camera. 
            // -10.0f is usually enough to clear most walls, but you can go higher (e.g., -100.0f)
            // Parameters: (commandBuffer, constantFactor, clamp, slopeFactor)
            gp_vkCmdSetDepthBias(cmd, -10.0f, 0.0f, -10.0f);
        }

        pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);

        if (gp_vkCmdSetDepthBias) {
            // Reset to 0 so the rest of the world draws correctly
            gp_vkCmdSetDepthBias(cmd, 0.0f, 0.0f, 0.0f);
        }

        //return; //or erase texture
    }
    /*
    //wallhack selected Stridehash
    if (countnum == t_.StrideHash)//idxCount / 100)
    {
        if (gp_vkCmdSetDepthTestEnable) {
            gp_vkCmdSetDepthTestEnable(cmd, VK_FALSE);
        }
        pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);
        if (gp_vkCmdSetDepthTestEnable) {
            gp_vkCmdSetDepthTestEnable(cmd, VK_TRUE);
        }

        //return; //or erase texture
    }
    */
    //hold down P key until a texture is erased, press END to log values of those textures
    if (GetAsyncKeyState('O') & 1) //-
        countnum--;
    if (GetAsyncKeyState('P') & 1) //+
        countnum++;
    if (GetAsyncKeyState('9') & 1) //reset, set to -1
        countnum = -1;


    pOriginalCmdDrawIndexed(cmd, idxCount, instCount, firstIdx, vtxOff, firstInst);
}

// The Indirect Hook
void VKAPI_CALL DetourVkCmdDrawIndexedIndirect(
    VkCommandBuffer commandBuffer,
    VkBuffer buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride)
{
    lognospam2(1, "VkCmdDrawIndexedIndirect hooked");

    //logger for logging indexcount
    if (countnum == t_.StrideHash)//idxCount / 100)
        if (GetAsyncKeyState(VK_END) & 1) //press END to log to log.txt
            Log("t_.StrideHash == %d && drawCount == %d && stride == %d", t_.StrideHash, drawCount, stride); //log selected values

    //wallhack selected Stridehash
    if (countnum == t_.StrideHash) {
        if (gp_vkCmdSetDepthBias) {
            // Push the model "forward" toward the camera. 
            // -10.0f is usually enough to clear most walls, but you can go higher (e.g., -100.0f)
            // Parameters: (commandBuffer, constantFactor, clamp, slopeFactor)
            gp_vkCmdSetDepthBias(commandBuffer, -10.0f, 0.0f, -10.0f);
        }

        pOriginalCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);

        if (gp_vkCmdSetDepthBias) {
            // Reset to 0 so the rest of the world draws correctly
            gp_vkCmdSetDepthBias(commandBuffer, 0.0f, 0.0f, 0.0f);
        }

        //return; //or erase texture
    }

    /*
    //wallhack selected Stridehash
    if (countnum == t_.StrideHash)//idxCount / 100)
    {
        if (gp_vkCmdSetDepthTestEnable) {
            gp_vkCmdSetDepthTestEnable(commandBuffer, VK_FALSE);
        }
        pOriginalCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
        if (gp_vkCmdSetDepthTestEnable) {
            gp_vkCmdSetDepthTestEnable(commandBuffer, VK_TRUE);
        }

        //return; //or erase texture
    }
    */

    //hold down P key until a texture is erased, press END to log values of those textures
    if (GetAsyncKeyState('O') & 1) //-
        countnum--;
    if (GetAsyncKeyState('P') & 1) //+
        countnum++;
    if (GetAsyncKeyState('9') & 1) //reset, set to -1
        countnum = -1;

    pOriginalCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

//=======================================================================================//

// 1. Define the typedef
typedef void (VKAPI_PTR* PFN_vkCmdBindVertexBuffers2)(VkCommandBuffer, uint32_t, uint32_t, const VkBuffer*, const VkDeviceSize*, const VkDeviceSize*, const VkDeviceSize*);
PFN_vkCmdBindVertexBuffers2 pOriginalBindVertexBuffers2 = nullptr;

// 2. The Detour
void VKAPI_CALL DetourBindVertexBuffers2(
    VkCommandBuffer cmd,
    uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* pBuffers,
    const VkDeviceSize* pOffsets,
    const VkDeviceSize* pSizes,
    const VkDeviceSize* pStrides)
{
    //THIS MAY NOT BE CALLED IN NEW GAMES, so we use CmdBindPipeline instead
    lognospam3(1, "BindVertexBuffers2 hooked");

    if (pStrides) {
        for (uint32_t i = 0; i < bindingCount; ++i) {
            // Your logic here
            uint32_t stride = (uint32_t)pStrides[i];
            if (stride > 0 && stride <= 200) {
                t_.Strides[firstBinding + i] = stride;
            }
        }
        // Update your Hash
        t_.StrideHash = fastStrideHash(t_.Strides, 16);
    }

    pOriginalBindVertexBuffers2(cmd, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}

//=======================================================================================//

#include <map>
#include <mutex>

typedef VkResult(VKAPI_PTR* PFN_vkCreateGraphicsPipelines)(VkDevice, VkPipelineCache, uint32_t, const VkGraphicsPipelineCreateInfo*, const VkAllocationCallbacks*, VkPipeline*);
typedef void(VKAPI_PTR* PFN_vkCmdBindPipeline)(VkCommandBuffer, VkPipelineBindPoint, VkPipeline);

PFN_vkCreateGraphicsPipelines pOriginalCreateGraphicsPipelines = nullptr;
PFN_vkCmdBindPipeline pOriginalCmdBindPipeline = nullptr;

// Thread-safe map to store pipeline strides
std::map<VkPipeline, uint32_t> pipelineToStride;
std::mutex pipelineMutex;

// Hook for Creating Pipelines
VkResult VKAPI_CALL DetourCreateGraphicsPipelines(VkDevice device, VkPipelineCache cache, uint32_t count, const VkGraphicsPipelineCreateInfo* pCreateInfos, const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    VkResult result = pOriginalCreateGraphicsPipelines(device, cache, count, pCreateInfos, pAllocator, pPipelines);

    if (result == VK_SUCCESS) {
        std::lock_guard<std::mutex> lock(pipelineMutex);
        for (uint32_t i = 0; i < count; i++) {
            // Drill down into the vertex input state
            if (pCreateInfos[i].pVertexInputState && pCreateInfos[i].pVertexInputState->vertexBindingDescriptionCount > 0) {
                // Get the stride of the first binding (usually where the main mesh data is)
                uint32_t stride = pCreateInfos[i].pVertexInputState->pVertexBindingDescriptions[0].stride;
                pipelineToStride[pPipelines[i]] = stride;
            }
        }
    }
    return result;
}

// Hook for Binding Pipelines
void VKAPI_CALL DetourCmdBindPipeline(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint, VkPipeline pipeline) {
    lognospam4(1, "CmdBindPipeline hooked");
    if (bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS) {
        std::lock_guard<std::mutex> lock(pipelineMutex);
        if (pipelineToStride.count(pipeline)) {
            // Here is your stride!
            t_.Strides[0] = pipelineToStride[pipeline];
            t_.StrideHash = fastStrideHash(t_.Strides, 1);
        }
    }
    pOriginalCmdBindPipeline(cmd, bindPoint, pipeline);
}

//=======================================================================================//

// The Gateway Hook (Intercepting the function retrieval)
PFN_vkVoidFunction VKAPI_CALL DetourGetDeviceProcAddr(VkDevice device, const char* pName) {
    PFN_vkVoidFunction realFunc = pOriginalGetDeviceProcAddr(device, pName);
    
    if (realFunc && strcmp(pName, "vkCmdDrawIndexed") == 0) {
        Log("Caught in GetDeviceProcAddr");
        // Save the real address so our draw detour can call it
        pOriginalCmdDrawIndexed = (PFN_vkCmdDrawIndexed)realFunc;
        // Return our detour instead of the real function
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndexed;
    }

    if (strcmp(pName, "vkCmdDrawIndexedIndirect") == 0) {
        pOriginalCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)realFunc;
        return (PFN_vkVoidFunction)DetourVkCmdDrawIndexedIndirect;
    }

    if (strcmp(pName, "vkCmdBindVertexBuffers2") == 0 ||
        strcmp(pName, "vkCmdBindVertexBuffers2EXT") == 0) {
        pOriginalBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)realFunc;
        return (PFN_vkVoidFunction)DetourBindVertexBuffers2;
    }

    if (strcmp(pName, "vkCreateGraphicsPipelines") == 0) {
        pOriginalCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)realFunc;
        return (PFN_vkVoidFunction)DetourCreateGraphicsPipelines;
    }

    if (strcmp(pName, "vkCmdBindPipeline") == 0) {
        pOriginalCmdBindPipeline = (PFN_vkCmdBindPipeline)realFunc;
        return (PFN_vkVoidFunction)DetourCmdBindPipeline;
    }

    // capture it
    if (pName && (strcmp(pName, "vkCmdSetDepthTestEnable") == 0 ||
        strcmp(pName, "vkCmdSetDepthTestEnableEXT") == 0)) {
        gp_vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)realFunc;
        return realFunc;
    }

    if (strcmp(pName, "vkCmdSetDepthBias") == 0) {
        gp_vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)realFunc;
        return realFunc;
    }

    //Game may use other drawing function
    //vkCmdDrawIndirectCount
    //..

    return realFunc;
}

//=======================================================================================//

// The Instance Gateway Hook
PFN_vkVoidFunction VKAPI_CALL DetourGetInstanceProcAddr(VkInstance instance, const char* pName) {

    // Get the real function
    PFN_vkVoidFunction realFunc = pOriginalGetInstanceProcAddr(instance, pName);

    if (!pName) return realFunc;
    
    if (realFunc && pName) {
        // If the game asks for vkGetDeviceProcAddr here, we must return OUR detour
        if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
            return (PFN_vkVoidFunction)DetourGetDeviceProcAddr;
        }

        // Catch the draw call if it's requested at instance level
        if (strcmp(pName, "vkCmdDrawIndexed") == 0) {
            Log("Caught in GetInstanceProcAddr");
            pOriginalCmdDrawIndexed = (PFN_vkCmdDrawIndexed)realFunc;
            return (PFN_vkVoidFunction)DetourVkCmdDrawIndexed;
        }
    }

    return realFunc;
}

//=======================================================================================//

// Update your HookThread
DWORD WINAPI HookThread(LPVOID lpParam) {
    //Log("1 - Initializing");
    if (MH_Initialize() != MH_OK) return 1;

    HMODULE hVulkan = GetModuleHandleA("vulkan-1.dll");
    if (!hVulkan) return 1;

    // Get the core loader exports
    void* pGetDevice = (void*)GetProcAddress(hVulkan, "vkGetDeviceProcAddr");
    void* pGetInstance = (void*)GetProcAddress(hVulkan, "vkGetInstanceProcAddr");

    // Hook GetDeviceProcAddr
    if (MH_CreateHook(pGetDevice, &DetourGetDeviceProcAddr, reinterpret_cast<LPVOID*>(&pOriginalGetDeviceProcAddr)) == MH_OK) {
        MH_EnableHook(pGetDevice);
    }

    // Hook GetInstanceProcAddr (Crucial for id Tech)
    if (MH_CreateHook(pGetInstance, &DetourGetInstanceProcAddr, reinterpret_cast<LPVOID*>(&pOriginalGetInstanceProcAddr)) == MH_OK) {
        MH_EnableHook(pGetInstance);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hMod, DWORD reason, LPVOID res) {
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) { return CallNextHookEx(NULL, code, wParam, lParam); }