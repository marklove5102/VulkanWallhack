// Globals
bool initonce = false;
int countnum = -1;

#include <fstream>
inline void Log(const char* fmt, ...) {
	char text[4096] = { 0 };
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	std::ofstream logfile("log.txt", std::ios::app);
	if (logfile.is_open()) {
		logfile << text << std::endl;
	}
}

//viewport
thread_local struct ViewportState {
	VkViewport currentViewport = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
	uint32_t firstViewport = 0;
	bool hasViewport = false;
} vkt_;

bool reversedDepth = false;


#include <unordered_map>
#include <shared_mutex>

//pipeline
// Command Buffer State
struct CmdState {
	VkViewport currentViewport;
	uint32_t firstViewport;
	bool hasViewport = false;
};
std::unordered_map<VkCommandBuffer, CmdState> cmdStates;
std::shared_mutex statesMtx;

// Pipeline State
struct PipelineSettings {
	VkBool32 originalDepthTestEnable;
	VkBool32 originalDepthWriteEnable;
	VkCompareOp originalDepthCompareOp;
	uint32_t stride;
};
std::unordered_map<VkPipeline, PipelineSettings> pipelineData;
std::shared_mutex pipeMapMtx;


//std::shared_mutex pipeMapMtx2;
//std::unordered_map<VkPipeline, uint32_t> pipelineToStride;
std::unordered_map<VkCommandBuffer, uint32_t> cmdBufferToStride;


uint32_t fastStrideHash(const uint32_t* data, size_t count) {
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < count; ++i) {
		hash ^= data[i];
		hash *= 16777619u;
	}
	return hash % 100; // Two-digit number
}

#ifdef _UNICODE
# define VTEXT(text) L##text
#else
# define VTEXT(text) text
#endif

/*
void createCommandPool(VkDevice device, uint32_t queueFamilyIndex) {
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = g_queueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(g_device, &poolInfo, nullptr, &g_commandPool) != VK_SUCCESS) {
		Log("failed to create command pool!");
	}
}
*/
//cratepipeline

/*
Disabling ambient occlusion (AO) in Vulkan depends on how AO is implemented in the engine or application you're hooking. Here are some potential ways to disable AO in a Vulkan hook:
1. Hook the Ambient Occlusion Shader

Most modern engines use a shader to apply screen-space ambient occlusion (SSAO) or other AO techniques. You can try hooking into vkCreateShaderModule and detecting the AO shader by analyzing shader code or shader names.

	If you find the AO shader, modify or replace it with a pass-through shader that does nothing.

2. Modify Descriptor Sets (For SSAO Textures and Buffers)

Some engines use SSAO as a fullscreen pass where AO data is stored in a texture. If AO is sampled from a descriptor (e.g., VkDescriptorSet), you can try:

	Hook vkUpdateDescriptorSets and nullify the SSAO texture.
	Replace the AO texture with a blank texture (e.g., a fully white texture so AO has no effect).

Example of replacing AO texture:

VkDescriptorImageInfo blankAOTexture = {};
blankAOTexture.imageView = myWhiteTextureView;  // Use a preloaded 1x1 white texture
blankAOTexture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

VkWriteDescriptorSet writeAO = {};
writeAO.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
writeAO.dstSet = aoDescriptorSet; // Replace with correct set
writeAO.dstBinding = aoBinding; // Find which binding is used for AO
writeAO.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
writeAO.descriptorCount = 1;
writeAO.pImageInfo = &blankAOTexture;

vkUpdateDescriptorSets(device, 1, &writeAO, 0, nullptr);

	You will need to find which descriptor set and binding correspond to SSAO.

3. Disable AO in the Render Pass (SSAO Pass Skip)

	Hook vkCmdBeginRenderPass and check if it's an SSAO pass.
	If you detect an AO render pass, skip rendering for that pass.

Example:

if (isSSAO(pass)) {
	return; // Skip AO render pass
}

	You can detect SSAO passes by looking at VkRenderPassBeginInfo::renderPass and checking if the pass has an AO-related format (e.g., R8 texture, used for AO occlusion).

4. Hook SSAO Compute Shaders (For Ray-Traced AO or Compute AO)

Some engines use compute shaders for AO (especially for ray-traced AO). If you detect an AO compute shader, you can:

	Hook vkCmdDispatch and block SSAO compute dispatches.
	Modify AO compute shader output buffers.

Which Method Should You Use?

	If AO is a fullscreen pass → Try method #2 (descriptor sets) or #3 (skipping render pass).
	If AO is done via shaders → Try #1 (hooking shader modules).
	If AO is computed via compute shaders → Try #4 (hooking compute dispatch).
*/