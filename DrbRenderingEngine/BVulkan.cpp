#define VK_USE_PLATFORM_WIN32_KHR
#include "BVulkan.h"
#pragma comment(lib,"vulkan-1.lib")
#ifdef max
#undef max
#undef min
#endif
//#define MULTI_SAMPLE 1
unsigned char * LoadFileContent(const char *path, int &filesize) {
	unsigned char*fileContent = nullptr;
	filesize = 0;
	FILE*pFile = fopen(path, "rb");
	if (pFile) {
		fseek(pFile, 0, SEEK_END);
		int nLen = ftell(pFile);
		if (nLen > 0) {
			rewind(pFile);
			fileContent = new unsigned char[nLen + 1];
			fread(fileContent, sizeof(unsigned char), nLen, pFile);
			fileContent[nLen] = '\0';
			filesize = nLen;
		}
		fclose(pFile);
	}
	return fileContent;
}
struct Matrix4x4f {
	float mData[16];
};
struct ABufferObject{
	VkBuffer mBuffer;
	VkDeviceMemory mMemory;
	ABufferObject() {
		mBuffer = 0;
		mMemory = 0;
	}
	~ABufferObject() {
		if (mBuffer != 0) {
			vkDestroyBuffer(GetVulkanDevice(), mBuffer, nullptr);
			vkFreeMemory(GetVulkanDevice(), mMemory, nullptr);
		}
	}
};
struct AUniformBuffer {
	VkBuffer mBuffer;
	VkDeviceMemory mMemory;
	AUniformBufferType mType;
	std::vector<Matrix4x4f> mMatrices;
	std::vector<Vector4f> mVectors;
	AUniformBuffer() {
		mBuffer = 0;
		mMemory = 0;
	}
	~AUniformBuffer() {
		if (mBuffer != 0) {
			vkDestroyBuffer(GetVulkanDevice(), mBuffer, nullptr);
			vkFreeMemory(GetVulkanDevice(), mMemory, nullptr);
		}
	}
};
struct AProgram{
	VkPipelineShaderStageCreateInfo mShaderStage[2];
	int mShaderStageCount;
	VkShaderModule mVertexShader, mFragmentShader;
	VkDescriptorSetLayout mDescriptorSetLayout;
	std::vector<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;
	std::vector<VkDescriptorPoolSize> mDescriptorPoolSize;
	std::vector<VkWriteDescriptorSet> mWriteDescriptor;
	VkDescriptorSet mDescriptorSet;
	VkDescriptorPool mDescriptorPool;
	AUniformBuffer mVertexShaderMatrixUniformBuffer;
	AUniformBuffer mFragmentShaderMatrixUniformBuffer;
	AUniformBuffer mVertexShaderVectorUniformBuffer;
	AUniformBuffer mFragmentShaderVectorUniformBuffer;
	GraphicPipeline mGraphicPipeline;
	AProgram() {
		mShaderStageCount = 0;
		mVertexShader = 0;
		mFragmentShader = 0;
		mDescriptorPool = 0;
		mDescriptorSetLayout = 0;
		mDescriptorSet = 0;
	}
	~AProgram() {
		if (mVertexShader != 0) {
			vkDestroyShaderModule(GetVulkanDevice(), mVertexShader, nullptr);
		}
		if (mFragmentShader != 0) {
			vkDestroyShaderModule(GetVulkanDevice(), mFragmentShader, nullptr);
		}
		if(mDescriptorPool!=0)vkDestroyDescriptorPool(GetVulkanDevice(), mDescriptorPool, nullptr);
		if(mDescriptorSetLayout!=0)vkDestroyDescriptorSetLayout(GetVulkanDevice(), mDescriptorSetLayout, nullptr);
		for (int i = 0; i < mWriteDescriptor.size(); ++i) {
			VkWriteDescriptorSet *wd = &mWriteDescriptor[i];
			if (wd->pBufferInfo != nullptr) {
				delete wd->pBufferInfo;
			}
			if (wd->pImageInfo != nullptr) {
				delete wd->pImageInfo;
			}
		}
	}
};
static void* sWindowsHWND;
static VkInstance sVulkanInstance;
static VkDebugReportCallbackEXT sVulkanDebugger;
static VkSurfaceKHR sVulkanSurface;
static VkPhysicalDevice sPhysicalDevice;
static VkDevice sLogicDevice;
static VkQueue sGraphicQueue, sPresentQueue;
static int sGraphicQueueFamily, sPresentQueueFamily;
static int sViewportWidth = 0, sViewportHeight = 0;
static const char* sEnabledLayers[] = {
	"VK_LAYER_LUNARG_standard_validation"
};
static const uint32_t sEnabledLayerCount = 1;
static const char* sEnabledExtensions[] ={
	VK_KHR_SURFACE_EXTENSION_NAME ,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME
};
static const uint32_t sEnabledExtensionCount = 3;
static const uint32_t sEnabledDeviceExtensionCount = 1;
static const char* sEnabledDeviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static VkSampleCountFlagBits  sMaxSampleCount;
static SwapChainInfo sSwapChainInfo;
static VkSwapchainKHR sSwapChain;
static VkFormat sSwapChainImageFormat;
static VkExtent2D sSwapChainExtent;
static std::vector<VkImage> sSwapChainImages;
static std::vector<VkImageView> sSwapChainImageViews;
static VulkanFrameBuffer *sFrameBuffers;
static int sFrameBufferCount = 0;
static VulkanTexture *sColorBuffer, *sDepthBuffer;
static VkRenderPass sRenderPass;
static VkCommandPool sCommandPool;
static VkSemaphore sReadyToRender, sReadyToPresent;
static uint32_t sCurrentRenderTargetIndex = 0;
static float sClearColor[] = {0.0f,0.0f,0.0f,1.0f};
static float sClearDepth = 1.0f;
static uint32_t sClearStencil = 0;
static VkCommandBuffer sMainCommand = nullptr;

static AProgram* sCurrentProgram=nullptr;
static VulkanTexture * sCurrentTexture=nullptr;
static VulkanTexture * sDefaultVulkanTexture=nullptr;
static AVulkanHandle sCurrentVertexBuffer;

static PFN_vkCreateDebugReportCallbackEXT __vkCreateDebugReportCallback=nullptr;
static PFN_vkDestroyDebugReportCallbackEXT __vkDestroyDebugReportCallback = nullptr;
static PFN_vkCreateWin32SurfaceKHR __vkCreateWin32SurfaceKHR = nullptr;

#define vkCreateDebugReportCallback __vkCreateDebugReportCallback
#define vkDestroyDebugReportCallback __vkDestroyDebugReportCallback
#define vkCreateWin32SurfaceKHR __vkCreateWin32SurfaceKHR
VkSurfaceKHR GetVulkanSurface() {
	return sVulkanSurface;
}
VkInstance GetVulkanInstance() {
	return sVulkanInstance;
}
int GetGraphicQueueFamily() {
	return sGraphicQueueFamily;
}
int GetPresentQueueFamily() {
	return sPresentQueueFamily;
}
bool CanEnableInstanceLayer(const char *layerName){
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (layerCount == 0){
		return false;
	}
	VkLayerProperties *layers = (VkLayerProperties*)alloca(layerCount * sizeof(VkLayerProperties));
	vkEnumerateInstanceLayerProperties(&layerCount, layers);
	bool layerFound = false;
	for (uint32_t i = 0; i < layerCount; ++i){
		if (strcmp(layerName, layers[i].layerName) == 0){
			layerFound = true;
			break;
		}
	}
	return layerFound;
}
void aClearColor(float r, float g, float b, float a) {
	sClearColor[0] = r;
	sClearColor[1] = g;
	sClearColor[2] = b;
	sClearColor[3] = a;
}
void aClearDepthStencil(float depth, uint32_t stencil) {
	sClearDepth = depth;
	sClearStencil = stencil;
}
static void GenSampler(VulkanTexture*texture) {
	if (texture->mSampler != 0) {
		vkDestroySampler(GetVulkanDevice(), texture->mSampler, nullptr);
	}
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = texture->mMinFilter;
	samplerInfo.magFilter = texture->mMagFilter;
	samplerInfo.addressModeU = texture->mWrapU;
	samplerInfo.addressModeV = texture->mWrapV;
	samplerInfo.addressModeW = texture->mWrapW;
	samplerInfo.anisotropyEnable = texture->mbEnableAnisotropy;
	samplerInfo.maxAnisotropy = texture->mMaxAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	if (vkCreateSampler(GetVulkanDevice(), &samplerInfo, nullptr, &texture->mSampler) != VK_SUCCESS) {
		printf("failed to create texture sampler!");
	}
}
VkCommandBuffer aBeginRendering(VkCommandBuffer cmd) {
	VkCommandBuffer commandbuffer;
	if (cmd!=nullptr){
		commandbuffer = cmd;
	}
	else {
		aBeginOneTimeCommandBuffer(&commandbuffer);
	}
	VkFramebuffer render_target = AquireRenderTarget();
	VkRenderPass render_pass = GetGlobalRenderPass();
	VkClearValue clearValues[2] = {};
	clearValues[0].color = { sClearColor[0],sClearColor[1],sClearColor[2],sClearColor[3] };
	clearValues[1].depthStencil = { sClearDepth, sClearStencil };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.framebuffer = render_target;
	renderPassInfo.renderPass = render_pass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { uint32_t(sViewportWidth),uint32_t(sViewportHeight) };
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;
	vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	sMainCommand = commandbuffer;
	return commandbuffer;
}
void aEndRenderingCommand() {
	vkCmdEndRenderPass(sMainCommand);
	vkEndCommandBuffer(sMainCommand);
}
void aSwapBuffers(VkCommandBuffer commandbuffer) {
	if (commandbuffer == nullptr) {
		commandbuffer = sMainCommand;
	}
	VulkanSubmitDrawCommand(&commandbuffer, 1);
	VulkanSwapBuffers();
	aDeleteCommandBuffer(&commandbuffer, 1);
	commandbuffer = nullptr;
}
VulkanTexture::VulkanTexture(VkImageAspectFlags imageAspectFlag) {
	mImage = 0;
	mImageView = 0;
	mMemory = 0;
	mSampler = 0;
	mImageAspectFlag = imageAspectFlag;
	mMinFilter = VK_FILTER_LINEAR;
	mMagFilter = VK_FILTER_LINEAR;
	mWrapU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	mWrapV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	mWrapW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	mbEnableAnisotropy = VK_FALSE;
	mMaxAnisotropy = 0.0f;
}
VulkanTexture::~VulkanTexture() {
	if (mMemory != 0) {
		vkFreeMemory(sLogicDevice, mMemory, nullptr);
	}
	if (mImageView != 0) {
		vkDestroyImageView(sLogicDevice, mImageView, nullptr);
	}
	if (mImage != 0) {
		vkDestroyImage(sLogicDevice, mImage, nullptr);
	}
	if (mSampler!=0){
		vkDestroySampler(sLogicDevice, mSampler, nullptr);
	}
}
VulkanFrameBuffer::VulkanFrameBuffer() {
	mFrameBuffer = 0;
	mColorBuffer = 0;
	mDepthBuffer = 0;
	mResolveBuffer = 0;
	mSampleCount = VK_SAMPLE_COUNT_1_BIT;
}
VulkanFrameBuffer::~VulkanFrameBuffer() {
	if (mFrameBuffer != 0) {
		vkDestroyFramebuffer(sLogicDevice, mFrameBuffer, nullptr);
	}
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData){
	printf("[DEBUG]:%s\n", msg);
	return VK_FALSE;
}
static bool InitVulkanInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "HeckVulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "BattleFire";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = sEnabledExtensionCount;
	createInfo.ppEnabledExtensionNames = sEnabledExtensions;

	if (CanEnableInstanceLayer(sEnabledLayers[0])){
		createInfo.enabledLayerCount = sEnabledLayerCount;
		createInfo.ppEnabledLayerNames = sEnabledLayers;
	}else{
		printf("cannot find the validation layer VK_LAYER_LUNARG_standard_validation\n");
		createInfo.enabledLayerCount = 0;
	}
	if (vkCreateInstance(&createInfo, nullptr, &sVulkanInstance) != VK_SUCCESS){
		printf("create vulkan instance fail\n");
		return false;
	}
	return true;
}
static bool InitDebugger(){
	VkDebugReportCallbackCreateInfoEXT createDebugInfo = {};
	createDebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createDebugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createDebugInfo.pfnCallback = debugCallback;
	if (vkCreateDebugReportCallback(sVulkanInstance, &createDebugInfo, nullptr, &sVulkanDebugger) != VK_SUCCESS){
		return false;
	}
	return true;
}
static bool InitSurface(){
	VkWin32SurfaceCreateInfoKHR createSurfaceInfo;
	createSurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createSurfaceInfo.hwnd = (HWND)sWindowsHWND;
	createSurfaceInfo.hinstance = GetModuleHandle(nullptr);
	createSurfaceInfo.pNext = nullptr;
	createSurfaceInfo.flags = 0;
	if (vkCreateWin32SurfaceKHR(sVulkanInstance, &createSurfaceInfo, nullptr, &sVulkanSurface) != VK_SUCCESS) {
		return false;
	}
	return true;
}
static VkSampleCountFlagBits GetMaxUsableSampleCount() {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(sPhysicalDevice, &physicalDeviceProperties);
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts>physicalDeviceProperties.limits.framebufferDepthSampleCounts?
		physicalDeviceProperties.limits.framebufferDepthSampleCounts: physicalDeviceProperties.limits.framebufferColorSampleCounts;
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

	return VK_SAMPLE_COUNT_1_BIT;
}
static bool SelectPhysicDevice(){
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(sVulkanInstance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		printf("cannot find any gpu on your computer!\n");
		return false;
	}
	VkPhysicalDevice* devices = (VkPhysicalDevice*)alloca(deviceCount * sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(sVulkanInstance, &deviceCount, devices);

	for (uint32_t i = 0; i < deviceCount; ++i) {
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
		vkGetPhysicalDeviceFeatures(devices[i], &deviceFeatures);
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, nullptr);
		VkQueueFamilyProperties* queueFamilies = (VkQueueFamilyProperties*)alloca(queueFamilyCount * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilies);
		sPresentQueueFamily = -1;
		sGraphicQueueFamily = -1;
		for (uint32_t j = 0; j < queueFamilyCount; ++j) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, sVulkanSurface, &presentSupport);
			if (queueFamilies[j].queueCount > 0 && presentSupport){
				sPresentQueueFamily = j;
			}
			if (queueFamilies[j].queueCount > 0 && queueFamilies[j].queueFlags & VK_QUEUE_GRAPHICS_BIT){
				sGraphicQueueFamily = j;
			}
			if (sGraphicQueueFamily != -1 && sPresentQueueFamily != -1){
				sPhysicalDevice = devices[i];
				sMaxSampleCount = GetMaxUsableSampleCount();
				printf("select physic device %u/%u\n", i, deviceCount);
				printf("maxPushConstantsSize %u\n", deviceProperties.limits.maxPushConstantsSize);
				printf("maxViewports %u\n", deviceProperties.limits.maxViewports);
				printf("maxColorAttachments %u\n", deviceProperties.limits.maxColorAttachments);
				printf("maxUniformBufferRange %u\n", deviceProperties.limits.maxUniformBufferRange);
				printf("maxDescriptorSetInputAttachments %u\n", deviceProperties.limits.maxDescriptorSetInputAttachments);
				printf("maxDescriptorSetSampledImages %u\n", deviceProperties.limits.maxDescriptorSetSampledImages);
				printf("maxDescriptorSetSamplers %u\n", deviceProperties.limits.maxDescriptorSetSamplers);
				printf("maxDescriptorSetStorageBuffers %u\n", deviceProperties.limits.maxDescriptorSetStorageBuffers);
				printf("maxDescriptorSetStorageBuffersDynamic %u\n", deviceProperties.limits.maxDescriptorSetStorageBuffersDynamic);
				printf("maxDescriptorSetStorageImages %u\n", deviceProperties.limits.maxDescriptorSetStorageImages);
				printf("maxDescriptorSetUniformBuffers %u\n", deviceProperties.limits.maxDescriptorSetUniformBuffers);
				printf("maxDescriptorSetUniformBuffersDynamic %u\n", deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic);
				printf("line width %f~%f\n", deviceProperties.limits.lineWidthRange[0], deviceProperties.limits.lineWidthRange[1]);
				printf("point size %f~%f\n", deviceProperties.limits.pointSizeRange[0], deviceProperties.limits.pointSizeRange[1]);
				printf("max framebuffer size %ux%u\n", deviceProperties.limits.maxFramebufferWidth, deviceProperties.limits.maxFramebufferHeight);
				return true;
			}
		}
	}
	return false;
}
static bool CreateLogicDevice(){
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { sGraphicQueueFamily, sPresentQueueFamily };
	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies){
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;

	VkDeviceCreateInfo createDeviceInfo = {};
	createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
	createDeviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();

	createDeviceInfo.pEnabledFeatures = &deviceFeatures;
	createDeviceInfo.enabledExtensionCount = 0;

	createDeviceInfo.enabledExtensionCount = sEnabledDeviceExtensionCount;
	createDeviceInfo.ppEnabledExtensionNames = sEnabledDeviceExtensions;
	if (CanEnableInstanceLayer(sEnabledLayers[0])){
		createDeviceInfo.enabledLayerCount = sEnabledLayerCount;
		createDeviceInfo.ppEnabledLayerNames = sEnabledLayers;
	}else{
		createDeviceInfo.enabledLayerCount = 0;
	}
	if (vkCreateDevice(sPhysicalDevice, &createDeviceInfo, nullptr, &sLogicDevice) != VK_SUCCESS){
		printf("create vulkan device fail\n");
		return false;
	}
	vkGetDeviceQueue(sLogicDevice, sGraphicQueueFamily, 0, &sGraphicQueue);
	vkGetDeviceQueue(sLogicDevice, sPresentQueueFamily, 0, &sPresentQueue);
	return true;
}
SwapChainInfo GetSwapChainInfo() {
	return sSwapChainInfo;
}
void InitSwapChainInfo(){
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(sPhysicalDevice, sVulkanSurface, &sSwapChainInfo.mCapabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(sPhysicalDevice, sVulkanSurface, &formatCount, nullptr);

	if (formatCount != 0) {
		sSwapChainInfo.mFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(sPhysicalDevice, sVulkanSurface, &formatCount, sSwapChainInfo.mFormats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(sPhysicalDevice, sVulkanSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		sSwapChainInfo.mPresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(sPhysicalDevice, sVulkanSurface, &presentModeCount, sSwapChainInfo.mPresentModes.data());
	}
}
static VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return{ VK_FORMAT_B8G8R8A8_UNORM ,VK_COLORSPACE_SRGB_NONLINEAR_KHR };
	}
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}
static VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes){
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_MAILBOX_KHR;
}
static VkExtent2D SelectExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
		return capabilities.currentExtent;
	}else{
		VkExtent2D actualExtent = { uint32_t(sViewportWidth), uint32_t(sViewportHeight) };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	}
}
static bool CreateSwapChain(){
	VkSurfaceFormatKHR surfaceFormat = SelectSurfaceFormat(sSwapChainInfo.mFormats);
	VkPresentModeKHR presentMode = SelectPresentMode(sSwapChainInfo.mPresentModes);
	VkExtent2D extent = SelectExtent(sSwapChainInfo.mCapabilities);

	uint32_t imageCount = sSwapChainInfo.mCapabilities.minImageCount;
	if (sSwapChainInfo.mCapabilities.maxImageCount > 0 && imageCount > sSwapChainInfo.mCapabilities.maxImageCount) {
		imageCount = sSwapChainInfo.mCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = sVulkanSurface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	printf("image count %u\n", imageCount);
	uint32_t queueFamilyIndices[] = { (uint32_t)sGraphicQueueFamily, (uint32_t)sPresentQueueFamily };

	if (sGraphicQueueFamily != sPresentQueueFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = sSwapChainInfo.mCapabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(sLogicDevice, &createInfo, nullptr, &sSwapChain) != VK_SUCCESS){
		printf("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(sLogicDevice, sSwapChain, &imageCount, nullptr);
	sFrameBufferCount = imageCount;
	sSwapChainImages.resize(sFrameBufferCount);
	vkGetSwapchainImagesKHR(sLogicDevice, sSwapChain, &imageCount, sSwapChainImages.data());

	sSwapChainImageFormat = surfaceFormat.format;
	sSwapChainExtent = extent;
	sSwapChainImageViews.resize(sFrameBufferCount);
	for (int i = 0; i < sFrameBufferCount; i++) {
		sSwapChainImageViews[i] = Create2DImageView(sSwapChainImages[i], sSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT ,1);
	}
	return true;
}
static VkFormat FindDepthFormat() {
	if (IsFormatSupported(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	}
	if (IsFormatSupported(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)){
		return VK_FORMAT_D24_UNORM_S8_UINT;
	}
	if (IsFormatSupported(VK_FORMAT_D16_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		return VK_FORMAT_D16_UNORM_S8_UINT;
	}
	return VK_FORMAT_UNDEFINED;
}
static void CreateDepthBuffer() {
	sDepthBuffer = new VulkanTexture(VK_IMAGE_ASPECT_DEPTH_BIT);
	VkFormat depthFormat = FindDepthFormat();
	CreateImage(sDepthBuffer, sViewportWidth, sViewportHeight, depthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, GetGlobalFrameBufferSampleCount());
	sDepthBuffer->mImageView = Create2DImageView(sDepthBuffer->mImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}
static void CreateColorBuffer() {
	sColorBuffer = new VulkanTexture;
	VkFormat colorFormat = GetSwapChainImageFormat();

	CreateImage(sColorBuffer,sViewportWidth, sViewportHeight, colorFormat, 
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, GetGlobalFrameBufferSampleCount());
	sColorBuffer->mImageView = Create2DImageView(sColorBuffer->mImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}
static bool CreateRenderPass() {
	VkSampleCountFlagBits sample_count = GetGlobalFrameBufferSampleCount();
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = sSwapChainImageFormat;
	colorAttachment.samples = sample_count;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (sample_count==VK_SAMPLE_COUNT_1_BIT){
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}
	else {
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = sample_count;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = GetSwapChainImageFormat();
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription attachments[3];
	int attachment_count = 2;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if (sample_count!=VK_SAMPLE_COUNT_1_BIT){
		subpass.pResolveAttachments = &colorAttachmentResolveRef;
		attachment_count = 3;
		memcpy(&attachments[0], &colorAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[1], &depthAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[2], &colorAttachmentResolve, sizeof(VkAttachmentDescription));
	}
	else {
		memcpy(&attachments[0], &colorAttachment, sizeof(VkAttachmentDescription));
		memcpy(&attachments[1], &depthAttachment, sizeof(VkAttachmentDescription));
	}

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachment_count;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(GetVulkanDevice(), &renderPassInfo, nullptr, &sRenderPass) != VK_SUCCESS) {
		printf("create render pass fail\n");
		return false;
	}
	return true;
}
static bool CreateFrameBuffers() {
	if (sFrameBuffers==nullptr){
		sFrameBuffers = new VulkanFrameBuffer[sFrameBufferCount];
	}
	VkSampleCountFlagBits sample_count = GetGlobalFrameBufferSampleCount();
	int attachment_count = 2;
	if (sample_count!=VK_SAMPLE_COUNT_1_BIT){
		attachment_count = 3;
	}
	for (int i = 0; i < sFrameBufferCount; i++) {
		aFrameBufferSampleCount(&sFrameBuffers[i], sample_count);
		aFrameBufferDepthBuffer(&sFrameBuffers[i], sDepthBuffer->mImageView);
		aFrameBufferResolveBuffer(&sFrameBuffers[i], GetSwapChainImageView(i));
		if (sample_count != VK_SAMPLE_COUNT_1_BIT) {
			aFrameBufferColorBuffer(&sFrameBuffers[i], sColorBuffer->mImageView);
		}
		aFrameBufferFinish(&sFrameBuffers[i]);
	}
	return true;
}
static bool CreateCommandPool(){
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = sGraphicQueueFamily;
	poolInfo.flags = 0; 
	if (vkCreateCommandPool(sLogicDevice, &poolInfo, nullptr, &sCommandPool) != VK_SUCCESS){
		printf("create command pool fail\n");
		return false;
	}
	return true;
}
static void CreateSemaphores(){
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(sLogicDevice, &semaphoreInfo, nullptr, &sReadyToRender) != VK_SUCCESS ||
		vkCreateSemaphore(sLogicDevice, &semaphoreInfo, nullptr, &sReadyToPresent) != VK_SUCCESS){
		printf("create semaphore fail\n");
	}
}
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(sPhysicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	return 0;
}
bool InitVulkan(void*param, int width, int height) {
	sWindowsHWND = param;
	sViewportWidth = width;
	sViewportHeight = height;
	if (InitVulkanInstance()) {
		__vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(sVulkanInstance, "vkCreateDebugReportCallbackEXT");
		__vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(sVulkanInstance, "vkDestroyDebugReportCallbackEXT");
		__vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(sVulkanInstance, "vkCreateWin32SurfaceKHR");
		if (InitDebugger()){
			if (InitSurface()){
				if (SelectPhysicDevice()){
					if (CreateLogicDevice()){
						InitSwapChainInfo();
						CreateSwapChain();
						CreateColorBuffer();
						CreateDepthBuffer();
						CreateRenderPass();
						CreateFrameBuffers();
						CreateCommandPool();
						VkSemaphoreCreateInfo semaphoreInfo = {};
						semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
						if (vkCreateSemaphore(sLogicDevice, &semaphoreInfo, nullptr, &sReadyToRender) != VK_SUCCESS ||
							vkCreateSemaphore(sLogicDevice, &semaphoreInfo, nullptr, &sReadyToPresent) != VK_SUCCESS){
							printf("create semaphore fail\n");
						}
						//init default texture unit
						sDefaultVulkanTexture = new VulkanTexture;
						unsigned char * pixel = new unsigned char[32*32*4];
						memset(pixel, 0, 32 * 32 * 4);
						VkDeviceSize imageSize = 32 * 32 * 4;
						aTexImage2D(sDefaultVulkanTexture, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 32, 32, pixel);
						GenSampler(sDefaultVulkanTexture);
						delete[] pixel;
						return true;
					}
				}
			}
		}
	}
	return false;
}
int GetFrameBufferCount() {
	return sFrameBufferCount;
}
VkFramebuffer GetFrameBuffer(int index) {
	return sFrameBuffers[index].mFrameBuffer;
}
VkImageView GetSwapChainImageView(int index) {
	return sSwapChainImageViews[index];
}
void aViewport(int width, int height) {
	if (sViewportWidth!=width || sViewportHeight!=height){
		sViewportWidth = width;
		sViewportHeight = height;
		vkDeviceWaitIdle(sLogicDevice);
		delete sDepthBuffer;
		for (int i = 0; i < sFrameBufferCount; ++i) {
			vkDestroyImageView(sLogicDevice, sSwapChainImageViews[i], nullptr);
			vkDestroyFramebuffer(sLogicDevice, sFrameBuffers[i].mFrameBuffer, nullptr);
		}
		vkDestroySwapchainKHR(sLogicDevice, sSwapChain, nullptr);
		CreateSwapChain();
		CreateDepthBuffer();
		CreateFrameBuffers();
	}
}
int GetViewportWidth() {
	return sViewportWidth;
}
int GetViewportHeight() {
	return sViewportHeight;
}
void VulkanCleanUp() {
	delete sDefaultVulkanTexture;
	delete sColorBuffer;
	delete sDepthBuffer;
	delete[] sFrameBuffers;
	vkDestroyRenderPass(sLogicDevice, sRenderPass, nullptr);
	for (int i = 0; i < sFrameBufferCount; ++i) {
		vkDestroyImageView(sLogicDevice, sSwapChainImageViews[i], nullptr);
	}
	vkDestroySemaphore(sLogicDevice, sReadyToRender, nullptr);
	vkDestroySemaphore(sLogicDevice, sReadyToPresent, nullptr);
	vkDestroyCommandPool(sLogicDevice, sCommandPool, nullptr);
	vkDestroySwapchainKHR(sLogicDevice, sSwapChain, nullptr);
	vkDestroyDevice(sLogicDevice, nullptr);
}
VkRenderPass GetGlobalRenderPass() {
	return sRenderPass;
}
VkQueue GetGraphicQueue() {
	return sGraphicQueue;
}
VkQueue GetPresentQueue() {
	return sPresentQueue;
}
VkSemaphore GetReadyToRenderSemaphore() {
	return sReadyToRender;
}
VkSemaphore GetReadyToPresentSemaphore() {
	return sReadyToPresent;
}
VkSwapchainKHR GetSwapchain() {
	return sSwapChain;
}
uint32_t GetCurrentRenderTargetIndex() {
	return sCurrentRenderTargetIndex;
}
VkFramebuffer AquireRenderTarget() {
	vkAcquireNextImageKHR(sLogicDevice, sSwapChain, std::numeric_limits<uint64_t>::max(), sReadyToRender, VK_NULL_HANDLE, &sCurrentRenderTargetIndex);
	return sFrameBuffers[sCurrentRenderTargetIndex].mFrameBuffer;
}
void VulkanSubmitDrawCommand(VkCommandBuffer*commandbuffer, int count) {
	VkSemaphore waitSemaphores[] = { sReadyToRender };
	VkSemaphore signalSemaphores[] = { sReadyToPresent };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.pCommandBuffers = commandbuffer;
	submitInfo.commandBufferCount = count;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(sGraphicQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		printf("vkQueueSubmit failed!\n");
	}
}
void VulkanSwapBuffers() {
	VkSemaphore signalSemaphores[] = { sReadyToPresent };

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pSwapchains = &sSwapChain;
	presentInfo.swapchainCount = 1;
	presentInfo.pImageIndices = &sCurrentRenderTargetIndex;

	vkQueuePresentKHR(sPresentQueue, &presentInfo);
	vkQueueWaitIdle(sPresentQueue);
}
VkDevice GetVulkanDevice() {
	return sLogicDevice;
}
VkPhysicalDevice GetVulkanPhysicalDevice() {
	return sPhysicalDevice;
}
VkCommandPool GetCommandPool() {
	return sCommandPool;
}
VkFormat GetSwapChainImageFormat() {
	return sSwapChainImageFormat;
}
VkSampleCountFlagBits GetGlobalFrameBufferSampleCount() {
#ifndef MULTI_SAMPLE
	return VK_SAMPLE_COUNT_1_BIT;
#else
	return sMaxSampleCount;
#endif
}
void aClearColorBuffer(VkCommandBuffer commandbuffer, float r, float g, float b, float a, int color_buffer_index) {
	VkClearAttachment attachment = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	attachment.clearValue.color = {r,g,b,a};
	attachment.colorAttachment = 0;
	VkClearRect rect = {};
	rect.baseArrayLayer = 0;
	rect.layerCount = 1;
	rect.rect.offset.x = 0;
	rect.rect.offset.y = 0;
	rect.rect.extent.width = sViewportWidth;
	rect.rect.extent.height = sViewportHeight;
	vkCmdClearAttachments(commandbuffer, 1, &attachment, 1, &rect);
}
void aClearDepthBuffer(VkCommandBuffer commandbuffer, float depth) {
	VkClearAttachment attachment = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	attachment.clearValue.depthStencil.depth = depth;
	attachment.colorAttachment = 0;
	VkClearRect rect = {};
	rect.baseArrayLayer = 0;
	rect.layerCount = 1;
	rect.rect.offset.x = 0;
	rect.rect.offset.y = 0;
	rect.rect.extent.width = sViewportWidth;
	rect.rect.extent.height = sViewportHeight;
	vkCmdClearAttachments(commandbuffer, 1, &attachment, 1, &rect);
}
void aClearStencilBuffer(VkCommandBuffer commandbuffer, uint32_t stencil) {
	VkClearAttachment attachment = {};
	attachment.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
	attachment.clearValue.depthStencil.stencil = stencil;
	attachment.colorAttachment = 0;
	VkClearRect rect = {};
	rect.baseArrayLayer = 0;
	rect.layerCount = 1;
	rect.rect.offset.x = 0;
	rect.rect.offset.y = 0;
	rect.rect.extent.width = sViewportWidth;
	rect.rect.extent.height = sViewportHeight;
	vkCmdClearAttachments(commandbuffer, 1, &attachment, 1, &rect);
}
void CreateImage(VulkanTexture*texture,uint32_t w, uint32_t h, VkFormat f, VkImageUsageFlags usage,VkSampleCountFlagBits sample_count, int mipmaplevel) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = {w,h,1};
	imageInfo.mipLevels = mipmaplevel;
	imageInfo.arrayLayers = 1;
	imageInfo.format = f;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = sample_count;
	if (vkCreateImage(sLogicDevice, &imageInfo, nullptr, &texture->mImage) != VK_SUCCESS) {
		printf("failed to create image!");
	}
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(sLogicDevice, texture->mImage, &memRequirements);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (vkAllocateMemory(sLogicDevice, &allocInfo, nullptr, &texture->mMemory) != VK_SUCCESS) {
		printf("failed to allocate image memory!");
	}
	vkBindImageMemory(sLogicDevice, texture->mImage, texture->mMemory, 0);
}
void CreateImageCube(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits numSample, int mipmap_level) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipmap_level;
	imageInfo.arrayLayers = 6;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = numSample;
	imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; 
	if (vkCreateImage(sLogicDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		printf("failed to create image!");
	}
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(sLogicDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(sLogicDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		printf("failed to allocate image memory!");
	}
	vkBindImageMemory(sLogicDevice, image, imageMemory, 0);
}
VkImageView Create2DImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, int mipmap_level) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipmap_level;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;
	VkImageView imageView;
	if (vkCreateImageView(sLogicDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		printf("failed to create texture image view!");
	}
	return imageView;
}
VkImageView CreateCubeMapImageView(VkImage image, VkFormat format, int mipmap_level) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.image = image;
	viewInfo.format = format;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, uint32_t(mipmap_level), 0, 6 };

	VkImageView imageView;
	if (vkCreateImageView(sLogicDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		return 0;
	}
	return imageView;
}

VkResult aGenCommandBuffer(VkCommandBuffer * command_buffer, int count,VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = level;
	allocInfo.commandPool = sCommandPool;
	allocInfo.commandBufferCount = count;
	allocInfo.pNext = nullptr;
	return vkAllocateCommandBuffers(sLogicDevice, &allocInfo, command_buffer);
}
void aDeleteCommandBuffer(VkCommandBuffer * command_buffer, int count) {
	vkFreeCommandBuffers(sLogicDevice, sCommandPool, count, command_buffer);
}
VkResult aBeginOneTimeCommandBuffer(VkCommandBuffer * command_buffer) {
	VkResult ret = aGenCommandBuffer(command_buffer, 1);
	if (ret != VK_SUCCESS) {
		return ret;
	}
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pNext = nullptr;
	beginInfo.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(*command_buffer, &beginInfo);
	return ret;
}
VkResult aEndOneTimeCommandBuffer(VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);
	aWaitForCommmandFinish(command_buffer);
	vkFreeCommandBuffers(sLogicDevice, sCommandPool, 1, &command_buffer);
	return VK_SUCCESS;
}
void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer;
	aBeginOneTimeCommandBuffer(&commandBuffer);
	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	aEndOneTimeCommandBuffer(commandBuffer);
}
VkResult aGenBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult ret = vkCreateBuffer(sLogicDevice, &bufferInfo, nullptr, &buffer);
	if (ret != VK_SUCCESS) {
		printf("failed to create buffer!\n");
		return ret;
	}
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(sLogicDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	ret = vkAllocateMemory(sLogicDevice, &allocInfo, nullptr, &bufferMemory);
	if (ret != VK_SUCCESS) {
		printf("failed to allocate buffer memory!");
		return ret;
	}
	vkBindBufferMemory(sLogicDevice, buffer, bufferMemory, 0);
	return VK_SUCCESS;
}
void aDeleteBuffer(VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	vkDestroyBuffer(sLogicDevice, buffer, nullptr);
	vkFreeMemory(sLogicDevice, bufferMemory, nullptr);
}
void aBufferSubData(VkBuffer buffer, VkBufferUsageFlags usage, const  void* data, VkDeviceSize size) {
	VkBuffer tempbuffer;
	VkDeviceMemory tempbuffer_memory;
	aGenBuffer(size, usage, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, tempbuffer, tempbuffer_memory);
	void* dst;
	vkMapMemory(sLogicDevice, tempbuffer_memory, 0, size, 0, &dst);
	memcpy(dst, data, size_t(size));
	vkUnmapMemory(sLogicDevice, tempbuffer_memory);
	CopyBuffer(tempbuffer, buffer, size);
	vkDestroyBuffer(sLogicDevice, tempbuffer, nullptr);
	vkFreeMemory(sLogicDevice, tempbuffer_memory, nullptr);
}
void aCreateShader(VkShaderModule& shaderModule, unsigned char *code, int code_len) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code_len;
	createInfo.pCode = (uint32_t*)code;
	if (vkCreateShaderModule(sLogicDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		printf("create render shader module fail\n");
	}
}
void aDestroyShader(VkShaderModule&shaderModule) {
	vkDestroyShaderModule(sLogicDevice, shaderModule, nullptr);
}
void aMapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void**ppData) {
	vkMapMemory(sLogicDevice, memory, offset, size, flags, ppData);
}
void aUnmapMemory(VkDeviceMemory memory) {
	vkUnmapMemory(sLogicDevice, memory);
}
void aFrameBufferSampleCount(VulkanFrameBuffer*framebuffer, VkSampleCountFlagBits sample_count) {
	framebuffer->mSampleCount = sample_count;
}
void aFrameBufferColorBuffer(VulkanFrameBuffer*framebuffer, VkImageView colorbuffer) {
	framebuffer->mColorBuffer = colorbuffer;
}
void aFrameBufferDepthBuffer(VulkanFrameBuffer*framebuffer, VkImageView depthbuffer) {
	framebuffer->mDepthBuffer = depthbuffer;
}
void aFrameBufferResolveBuffer(VulkanFrameBuffer*framebuffer, VkImageView resolvebuffer) {
	framebuffer->mResolveBuffer = resolvebuffer;
}
void aFrameBufferFinish(VulkanFrameBuffer*framebuffer) {
	VkImageView attachments[3];
	int attachment_count = 2;
	if (framebuffer->mSampleCount != VK_SAMPLE_COUNT_1_BIT) {
		attachment_count = 3;
	}
	if (framebuffer->mSampleCount != VK_SAMPLE_COUNT_1_BIT) {
		attachments[0] = framebuffer->mColorBuffer;
		attachments[1] = framebuffer->mDepthBuffer;
		attachments[2] = framebuffer->mResolveBuffer;
	}
	else {
		attachments[0] = framebuffer->mResolveBuffer;
		attachments[1] = framebuffer->mDepthBuffer;
	}
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = sRenderPass;
	framebufferInfo.pAttachments = attachments;
	framebufferInfo.attachmentCount = attachment_count;
	framebufferInfo.width = sViewportWidth;
	framebufferInfo.height = sViewportHeight;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(GetVulkanDevice(), &framebufferInfo, nullptr, &framebuffer->mFrameBuffer) != VK_SUCCESS) {
		printf("create frame buffer fail\n");
		return;
	}
}
bool IsFormatSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags features) {
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(sPhysicalDevice, format, &props);
	bool support = false;
	switch (tiling){
	case VK_IMAGE_TILING_OPTIMAL:
		support = (props.optimalTilingFeatures&features) == features;
		break;
	case VK_IMAGE_TILING_LINEAR:
		support = (props.linearTilingFeatures&features) == features;
		break;
	}
	return support;
}
static bool HasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
void InitSrcAccessMask(VkImageLayout oldLayout, VkImageMemoryBarrier&barrier) {
	switch (oldLayout) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		barrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		printf("InitSrcAccessMask no %d\n",oldLayout);
		break;
	}
}
void InitDstAccessMask(VkImageLayout newLayout, VkImageMemoryBarrier&barrier) {
	switch (newLayout){
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (barrier.srcAccessMask == 0) {
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		printf("InitDstAccessMask no %d\n", newLayout);
		break;
	}
}
void SetImageLayout(VkCommandBuffer commandBuffer,VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout,
	VkImageSubresourceRange subresourceRange,VkPipelineStageFlags srcStageMask,VkPipelineStageFlags dstStageMask){
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;
	barrier.subresourceRange = subresourceRange;
	InitSrcAccessMask(oldLayout, barrier);
	InitDstAccessMask(newLayout, barrier);
	vkCmdPipelineBarrier(commandBuffer,srcStageMask,dstStageMask,0,0, nullptr,0, nullptr,1, &barrier);
}
void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer;
	aBeginOneTimeCommandBuffer(&commandBuffer);
	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	aEndOneTimeCommandBuffer(commandBuffer);
}
void aGenTexture(void**param) {
	*param = new VulkanTexture;
}
void aTexImage2D(void*param, VkFormat format, VkImageAspectFlags aspectFlags, int width, int height, const void * pixel) {
	VulkanTexture*texture = (VulkanTexture*)param;
	VkDeviceSize imageSize = width * height;
	if (format== VK_FORMAT_R8G8B8A8_UNORM){
		imageSize *= 4;
	}
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (pixel != nullptr) {
		aGenBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(sLogicDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixel, static_cast<size_t>(imageSize));
		vkUnmapMemory(sLogicDevice, stagingBufferMemory);
	}
	CreateImage(texture,width, height, format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	VkCommandBuffer commandbuffer;
	aBeginOneTimeCommandBuffer(&commandbuffer);
	VkImageSubresourceRange subresourceRange = { aspectFlags ,0,1,0,1};
	SetImageLayout(commandbuffer, texture->mImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	aEndOneTimeCommandBuffer(commandbuffer);
	if (pixel != nullptr) {
		CopyBufferToImage(stagingBuffer, texture->mImage, width, height);
		subresourceRange = { aspectFlags ,0,1,0,1 };
		aBeginOneTimeCommandBuffer(&commandbuffer);
		SetImageLayout(commandbuffer, texture->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
		aEndOneTimeCommandBuffer(commandbuffer);
		vkDestroyBuffer(sLogicDevice, stagingBuffer, nullptr);
		vkFreeMemory(sLogicDevice, stagingBufferMemory, nullptr);
	}
	texture->mImageView = Create2DImageView(texture->mImage, format, aspectFlags,1);
}
void aTexCube(void*param, VkFormat format, int width, int height, const void * pixel) {
	VulkanTexture*texture = (VulkanTexture*)param;
	int channel_count = 3;
	if (format==VK_FORMAT_R8G8B8A8_UNORM){
		channel_count = 4;
	}
	int offset_unit = width * height * channel_count;
	int imageSize = offset_unit * 6;
	int offset = 0;
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	if (pixel != nullptr) {
		aGenBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(sLogicDevice, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixel, imageSize);
		vkUnmapMemory(sLogicDevice, stagingBufferMemory);
	}
	CreateImageCube(width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture->mImage, texture->mMemory, VK_SAMPLE_COUNT_1_BIT, 1);
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	for (uint32_t face = 0; face < 6; face++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = face;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = width;
		bufferCopyRegion.imageExtent.height = height;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;
		bufferCopyRegions.push_back(bufferCopyRegion);
		offset += offset_unit;
	}
	VkCommandBuffer commandbuffer;
	aBeginOneTimeCommandBuffer(&commandbuffer);
	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT ,0,1,0,6 };
	SetImageLayout(commandbuffer, texture->mImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	aEndOneTimeCommandBuffer(commandbuffer);
	if (pixel != nullptr) {
		aBeginOneTimeCommandBuffer(&commandbuffer);
		vkCmdCopyBufferToImage(
			commandbuffer,
			stagingBuffer,
			texture->mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);
		aEndOneTimeCommandBuffer(commandbuffer);
		aBeginOneTimeCommandBuffer(&commandbuffer);
		SetImageLayout(commandbuffer, texture->mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);
		aEndOneTimeCommandBuffer(commandbuffer);
		vkDestroyBuffer(sLogicDevice, stagingBuffer, nullptr);
		vkFreeMemory(sLogicDevice, stagingBufferMemory, nullptr);
	}
	texture->mImageView = CreateCubeMapImageView(texture->mImage, format, 1);
}
void aWaitForCommmandFinish(VkCommandBuffer commandbuffer) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandbuffer;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;
	VkFence fence;
	vkCreateFence(sLogicDevice, &fenceCreateInfo, nullptr, &fence);
	vkQueueSubmit(sGraphicQueue, 1, &submitInfo, fence);

	vkWaitForFences(sLogicDevice, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(sLogicDevice, fence, nullptr);
}
AVulkanHandle glGenBuffer() {
	return new ABufferObject;
}
void glBufferData(AVulkanHandle buffer, int size, void *data) {
	ABufferObject *vbo = (ABufferObject *)buffer;
	aGenVertexBuffer(size, vbo->mBuffer, vbo->mMemory);
	aBufferSubVertexData(vbo->mBuffer, data, size);
}
void glDeleteBufferObject(AVulkanHandle buffer) {
	ABufferObject *vbo = (ABufferObject *)buffer;
	delete vbo;
}
AVulkanHandle aCreateProgram() {
	AProgram *program = new AProgram;
	program->mVertexShader = 0;
	program->mFragmentShader = 0;
	memset(program->mShaderStage, 0, sizeof(VkPipelineShaderStageCreateInfo) * 2);
	return program;
}
void aAttachVertexShader(AVulkanHandle p, VkShaderModule shader) {
	AProgram *program = (AProgram*)p;
	program->mShaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	program->mShaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	program->mShaderStage[0].module = shader;
	program->mShaderStage[0].pName = "main";
	program->mVertexShader = shader;
}
void aAttachFragmentShader(AVulkanHandle p, VkShaderModule shader) {
	AProgram *program = (AProgram*)p;
	program->mShaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	program->mShaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	program->mShaderStage[1].module = shader;
	program->mShaderStage[1].pName = "main";
	program->mFragmentShader = shader;
}
static void ConfigUniformBuffer(AProgram *program,int binding, AUniformBuffer *uniformbuffer, VkShaderStageFlags shader_stage, VkDescriptorType t= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = binding;
	uboLayoutBinding.descriptorType = t;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = shader_stage;
	uboLayoutBinding.pImmutableSamplers = nullptr; 
	program->mDescriptorSetLayoutBindings.push_back(uboLayoutBinding);

	VkDescriptorPoolSize poolsize = {};
	poolsize.type = t;
	poolsize.descriptorCount = 1;
	program->mDescriptorPoolSize.push_back(poolsize);

	VkDescriptorBufferInfo *bufferInfo = new VkDescriptorBufferInfo;
	bufferInfo->buffer = uniformbuffer->mBuffer;
	bufferInfo->offset = 0;
	if (uniformbuffer->mType == kUniformBufferTypeMatrix) {
		bufferInfo->range = sizeof(Matrix4x4f)*uniformbuffer->mMatrices.size();
	} else {
		bufferInfo->range = sizeof(Vector4f)*uniformbuffer->mVectors.size();
	}
	VkWriteDescriptorSet descriptorWriter = {};
	descriptorWriter.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter.dstSet = program->mDescriptorSet;
	descriptorWriter.dstBinding = binding;
	descriptorWriter.dstArrayElement = 0;
	descriptorWriter.descriptorType = t;
	descriptorWriter.descriptorCount = 1;
	descriptorWriter.pBufferInfo = bufferInfo;
	program->mWriteDescriptor.push_back(descriptorWriter);
}
void BindingSampler2D(AProgram *program, int binding, VkImageView texture, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = binding;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	program->mDescriptorSetLayoutBindings.push_back(samplerLayoutBinding);

	VkDescriptorPoolSize poolsize = {};
	poolsize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolsize.descriptorCount = 1;
	program->mDescriptorPoolSize.push_back(poolsize);

	VkDescriptorImageInfo *imageInfo = new VkDescriptorImageInfo;
	imageInfo->imageLayout = layout;
	imageInfo->imageView = texture;
	imageInfo->sampler = sampler;

	VkWriteDescriptorSet descriptorWriter = {};
	descriptorWriter.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter.dstSet = program->mDescriptorSet;
	descriptorWriter.dstBinding = binding;
	descriptorWriter.dstArrayElement = 0;
	descriptorWriter.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWriter.descriptorCount = 1;
	descriptorWriter.pImageInfo = imageInfo;
	program->mWriteDescriptor.push_back(descriptorWriter);
}

void InitDescriptorSetLayout(AVulkanHandle param) {
	AProgram*program = (AProgram*)param;
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(program->mDescriptorSetLayoutBindings.size());
	layoutInfo.pBindings = program->mDescriptorSetLayoutBindings.data();
	if (vkCreateDescriptorSetLayout(GetVulkanDevice(), &layoutInfo, nullptr, &program->mDescriptorSetLayout) != VK_SUCCESS) {
		printf("failed to create descriptor set layout!\n");
	}
}
void InitDescriptorSet(AVulkanHandle param) {
	AProgram*program = (AProgram*)param;
	if (program->mWriteDescriptor.empty()){
		return;
	}
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = program->mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &program->mDescriptorSetLayout;
	if (vkAllocateDescriptorSets(GetVulkanDevice(), &allocInfo, &program->mDescriptorSet) != VK_SUCCESS) {
		printf("failed to allocate descriptor sets!\n");
	}
	for (int i = 0; i < program->mWriteDescriptor.size(); ++i) {
		program->mWriteDescriptor[i].dstSet = program->mDescriptorSet;
	}
	vkUpdateDescriptorSets(GetVulkanDevice(), static_cast<uint32_t>(program->mWriteDescriptor.size()), program->mWriteDescriptor.data(), 0, nullptr);
}
void InitDescriptorPool(AVulkanHandle param) {
	AProgram*program = (AProgram*)param;
	if (program->mDescriptorPoolSize.empty()){
		return;
	}
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(program->mDescriptorPoolSize.size());
	poolInfo.pPoolSizes = program->mDescriptorPoolSize.data();
	poolInfo.maxSets = 1;
	if (vkCreateDescriptorPool(GetVulkanDevice(), &poolInfo, nullptr, &program->mDescriptorPool) != VK_SUCCESS) {
		printf("failed to create descriptor pool!\n");
	}
}
void aLinkProgram(AVulkanHandle p) {
	AProgram *program = (AProgram*)p;
	program->mVertexShaderVectorUniformBuffer.mType = kUniformBufferTypeVector;
	program->mVertexShaderVectorUniformBuffer.mVectors.resize(8);
	program->mVertexShaderMatrixUniformBuffer.mType = kUniformBufferTypeMatrix;
	program->mVertexShaderMatrixUniformBuffer.mMatrices.resize(8);
	program->mFragmentShaderVectorUniformBuffer.mType = kUniformBufferTypeVector;
	program->mFragmentShaderVectorUniformBuffer.mVectors.resize(8);
	program->mFragmentShaderMatrixUniformBuffer.mType = kUniformBufferTypeMatrix;
	program->mFragmentShaderMatrixUniformBuffer.mMatrices.resize(8);
	aGenBuffer(sizeof(Vector4f) * 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		program->mVertexShaderVectorUniformBuffer.mBuffer, program->mVertexShaderVectorUniformBuffer.mMemory);
	aGenBuffer(sizeof(Matrix4x4f) * 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		program->mVertexShaderMatrixUniformBuffer.mBuffer, program->mVertexShaderMatrixUniformBuffer.mMemory);
	aGenBuffer(sizeof(Vector4f) * 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		program->mFragmentShaderVectorUniformBuffer.mBuffer, program->mFragmentShaderVectorUniformBuffer.mMemory);
	aGenBuffer(sizeof(Matrix4x4f) * 8, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		program->mFragmentShaderMatrixUniformBuffer.mBuffer, program->mFragmentShaderMatrixUniformBuffer.mMemory);
	ConfigUniformBuffer(program, 0, &program->mVertexShaderVectorUniformBuffer, VK_SHADER_STAGE_VERTEX_BIT);
	ConfigUniformBuffer(program, 1, &program->mVertexShaderMatrixUniformBuffer, VK_SHADER_STAGE_VERTEX_BIT);
	ConfigUniformBuffer(program, 2, &program->mFragmentShaderVectorUniformBuffer, VK_SHADER_STAGE_FRAGMENT_BIT);
	ConfigUniformBuffer(program, 3, &program->mFragmentShaderMatrixUniformBuffer, VK_SHADER_STAGE_FRAGMENT_BIT);
	BindingSampler2D(program, 4, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 5, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 6, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 7, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 8, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 9, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	BindingSampler2D(program, 10, sDefaultVulkanTexture->mImageView, sDefaultVulkanTexture->mSampler);
	InitDescriptorSetLayout(program);
	InitDescriptorPool(program);
	InitDescriptorSet(program);
	aSetDescriptorSetLayout(&program->mGraphicPipeline,&program->mDescriptorSetLayout);
	program->mGraphicPipeline.mViewport = { 0.0f,0.0f,float(sViewportWidth),float(sViewportHeight),0.0f,1.0f };
	program->mGraphicPipeline.mScissor = { {0,0},{uint32_t(sViewportWidth),uint32_t(sViewportHeight)} };
	aSetShaderStage(&program->mGraphicPipeline,program->mShaderStage, 2);
	aSetColorAttachmentCount(&program->mGraphicPipeline, 1);
	aSetRenderPass(&program->mGraphicPipeline, GetGlobalRenderPass());
	aCreateGraphicPipeline(&program->mGraphicPipeline);
}
void glUseProgram(AVulkanHandle program) {
	sCurrentProgram = (AProgram*)program;
}
void aDeleteProgram(AVulkanHandle p) {
	AProgram *program = (AProgram*)p;
	delete program;
}
void glBindVertexBuffer(AVulkanHandle buffer) {
	sCurrentVertexBuffer = buffer;
}
void glDrawArrays(int mode, int offset, int count) {
	aSetDynamicState(&sCurrentProgram->mGraphicPipeline, sMainCommand);
	ABufferObject*vbo = (ABufferObject*)sCurrentVertexBuffer;
	VkBuffer vertexBuffers[] = { vbo->mBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindPipeline(sMainCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, sCurrentProgram->mGraphicPipeline.mPipeline);
	vkCmdBindVertexBuffers(sMainCommand, 0, 1, vertexBuffers, offsets);
	if (sCurrentProgram->mDescriptorSet != 0) {
		vkCmdBindDescriptorSets(sMainCommand, VK_PIPELINE_BIND_POINT_GRAPHICS, sCurrentProgram->mGraphicPipeline.mPipelineLayout, 0, 1, &sCurrentProgram->mDescriptorSet, 0, nullptr);
	}
	vkCmdDraw(sMainCommand, count, 1, offset, 0);
}
GraphicPipeline::GraphicPipeline() :
	mPipelineLayout(0),
	mPipeline(0) {
	mInputAssemblyState = {};
	mInputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	mInputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	mInputAssemblyState.primitiveRestartEnable = VK_FALSE;
	mViewport = {};
	mScissor = {};
	mViewportState = { };
	mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	mViewportState.viewportCount = 1;
	mViewportState.pViewports = &mViewport;
	mViewportState.scissorCount = 1;
	mViewportState.pScissors = &mScissor;

	mRasterizer = {};
	mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	mRasterizer.depthClampEnable = VK_FALSE;
	mRasterizer.rasterizerDiscardEnable = VK_FALSE;
	mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	mRasterizer.lineWidth = 1.0f;
	mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	mRasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	mRasterizer.depthBiasEnable = VK_TRUE;
	mRasterizer.depthBiasConstantFactor = 0.0f;
	mRasterizer.depthBiasClamp = 0.0f;
	mRasterizer.depthBiasSlopeFactor = 0.0f;

	mDepthStencilState = {};
	mDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	mDepthStencilState.depthTestEnable = VK_TRUE;
	mDepthStencilState.depthWriteEnable = VK_TRUE;
	mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	mDepthStencilState.depthBoundsTestEnable = VK_FALSE;
	mDepthStencilState.minDepthBounds = 0.0f;
	mDepthStencilState.maxDepthBounds = 1.0f;
	mDepthStencilState.stencilTestEnable = VK_FALSE;
	mDepthStencilState.front = {};
	mDepthStencilState.back = {};

	mMultisampleState = {};
	mMultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mMultisampleState.sampleShadingEnable = VK_TRUE;
	mMultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	mMultisampleState.minSampleShading = 1.0f;
	mMultisampleState.pSampleMask = nullptr;
	mMultisampleState.alphaToCoverageEnable = VK_FALSE;
	mMultisampleState.alphaToOneEnable = VK_FALSE;

	mColorBlendState = {};
	mColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	mColorBlendState.logicOpEnable = VK_FALSE;
	mColorBlendState.logicOp = VK_LOGIC_OP_COPY;
	mColorBlendState.attachmentCount = 0;
	mColorBlendState.pAttachments = nullptr;
	mColorBlendState.blendConstants[0] = 0.0f;
	mColorBlendState.blendConstants[1] = 0.0f;
	mColorBlendState.blendConstants[2] = 0.0f;
	mColorBlendState.blendConstants[3] = 0.0f;

	mDescriptorSetLayout = nullptr;
	mShaderStage = nullptr;
	mShaderStageCount = 0;
	mDescriptorSetLayoutCount = 0;
	mRenderPass = 0;
	mSampleCount = VK_SAMPLE_COUNT_1_BIT;

	mPushConstantShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	mPushConstantCount = 8;

	mDepthConstantFactor = 0.0f;
	mDepthClamp = 0.0f;
	mDepthSlopeFactor = 0.0f;
}
GraphicPipeline::~GraphicPipeline() {
	CleanUp();
}
void GraphicPipeline::CleanUp() {
	if (mPipeline != 0) {
		vkDestroyPipeline(GetVulkanDevice(), mPipeline, nullptr);
	}
	if (mPipelineLayout != 0) {
		vkDestroyPipelineLayout(GetVulkanDevice(), mPipelineLayout, nullptr);
	}
}
void aInitPipelineLayout(AVulkanHandle param) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	VkPushConstantRange pushconstantRange = {};
	pushconstantRange.stageFlags = pipeline->mPushConstantShaderStage;
	pushconstantRange.offset = 0;
	pushconstantRange.size = pipeline->mPushConstantCount * sizeof(float) * 4;
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pSetLayouts = pipeline->mDescriptorSetLayout;
	pipelineLayoutInfo.setLayoutCount = pipeline->mDescriptorSetLayoutCount;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushconstantRange;
	if (vkCreatePipelineLayout(GetVulkanDevice(), &pipelineLayoutInfo, nullptr, &pipeline->mPipelineLayout) != VK_SUCCESS) {
		printf("create pipeline layout fail\n");
		return;
	}
}
void aSetDescriptorSetLayout(AVulkanHandle param,VkDescriptorSetLayout*layout, int count) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDescriptorSetLayout = layout;
	pipeline->mDescriptorSetLayoutCount = count;
	aInitPipelineLayout(param);
}
void aSetShaderStage(AVulkanHandle param, VkPipelineShaderStageCreateInfo*ss, int count) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mShaderStage = ss;
	pipeline->mShaderStageCount = count;
}
void aSetRenderPass(AVulkanHandle param, VkRenderPass render_pass) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mRenderPass = render_pass;
}
void aSetBufferSampleCount(AVulkanHandle param, VkSampleCountFlagBits sample_count) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mSampleCount = sample_count;
}
void aSetColorAttachmentCount(AVulkanHandle param, int count) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mColorBlendAttachmentStates.resize(count);
	for (int i = 0; i < count; i++) {
		pipeline->mColorBlendAttachmentStates[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipeline->mColorBlendAttachmentStates[i].blendEnable = VK_FALSE;
		pipeline->mColorBlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].colorBlendOp = VK_BLEND_OP_ADD;
		pipeline->mColorBlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipeline->mColorBlendAttachmentStates[i].alphaBlendOp = VK_BLEND_OP_ADD;
	}
}
void aCreateGraphicPipeline(AVulkanHandle param) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	const auto &bindingDescription = Vertex::getBindingDescription();
	const auto &attributeDescriptions = Vertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	pipeline->mColorBlendState.attachmentCount = pipeline->mColorBlendAttachmentStates.size();
	pipeline->mColorBlendState.pAttachments = pipeline->mColorBlendAttachmentStates.data();
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_DEPTH_BIAS
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 4;
	dynamicState.pDynamicStates = dynamicStates;
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = pipeline->mShaderStageCount;
	pipelineInfo.pStages = pipeline->mShaderStage;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &pipeline->mInputAssemblyState;
	pipelineInfo.pViewportState = &pipeline->mViewportState;
	pipelineInfo.pRasterizationState = &pipeline->mRasterizer;
	pipelineInfo.pMultisampleState = &pipeline->mMultisampleState;
	pipelineInfo.pDepthStencilState = &pipeline->mDepthStencilState;
	pipelineInfo.pColorBlendState = &pipeline->mColorBlendState;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipeline->mPipelineLayout;
	pipelineInfo.renderPass = pipeline->mRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	if (vkCreateGraphicsPipelines(GetVulkanDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline->mPipeline) != VK_SUCCESS) {
		printf("create pipeline fail\n");
	}
}
void aSetCullMode(AVulkanHandle param, VkCullModeFlags mode) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mRasterizer.cullMode = mode;
}
void aSetFrontFace(AVulkanHandle param, VkFrontFace front) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mRasterizer.frontFace = front;
}
void aEnableBlend(AVulkanHandle param, int attachment_index, VkBool32 enable) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mColorBlendAttachmentStates[attachment_index].blendEnable = enable;
}
void aEnableZTest(AVulkanHandle param, VkBool32 enable) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDepthStencilState.depthTestEnable = enable;
}
void aEnableZWrite(AVulkanHandle param, VkBool32 enable) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDepthStencilState.depthWriteEnable = enable;
}
void aEnableStencilTest(AVulkanHandle param, VkBool32 enable, VkCompareOp compare, VkStencilOp failed, VkStencilOp depthfailed, VkStencilOp passed) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDepthStencilState.stencilTestEnable = enable;
	pipeline->mDepthStencilState.back.compareOp = compare;
	pipeline->mDepthStencilState.back.failOp = failed;
	pipeline->mDepthStencilState.back.depthFailOp = depthfailed;
	pipeline->mDepthStencilState.back.passOp = passed;
	pipeline->mDepthStencilState.back.compareMask = 0xff;
	pipeline->mDepthStencilState.back.writeMask = 0xff;
	pipeline->mDepthStencilState.back.reference = 1;
	pipeline->mDepthStencilState.front = pipeline->mDepthStencilState.back;
}
void aBlend(AVulkanHandle param, int attachment_index, VkBlendFactor src_color, VkBlendFactor src_alpha, VkBlendFactor dst_color, VkBlendFactor dst_alpha) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mColorBlendAttachmentStates[attachment_index].srcColorBlendFactor = src_color;
	pipeline->mColorBlendAttachmentStates[attachment_index].dstColorBlendFactor = dst_color;
	pipeline->mColorBlendAttachmentStates[attachment_index].srcAlphaBlendFactor = src_alpha;
	pipeline->mColorBlendAttachmentStates[attachment_index].dstAlphaBlendFactor = dst_alpha;
}
void aBlendOp(AVulkanHandle param, int attachment_index, VkBlendOp color, VkBlendOp alpha) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mColorBlendAttachmentStates[attachment_index].colorBlendOp = color;
	pipeline->mColorBlendAttachmentStates[attachment_index].alphaBlendOp = alpha;
}
void aSetDepthFunc(AVulkanHandle param, VkCompareOp func) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDepthStencilState.depthCompareOp = func;
}
void aSetPrimitiveType(AVulkanHandle param, VkPrimitiveTopology t, VkBool32 restart) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mInputAssemblyState.topology = t;
	pipeline->mInputAssemblyState.primitiveRestartEnable = restart;
}
void aSetConstant(AVulkanHandle param, int index, Vector4f&v) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	memcpy(&pipeline->mPushConstants[index], v.v, sizeof(Vector4f));
}
void aSetDynamicState(AVulkanHandle param, VkCommandBuffer commandbuffer) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	vkCmdSetViewport(commandbuffer, 0, 1, &pipeline->mViewport);
	vkCmdSetScissor(commandbuffer, 0, 1, &pipeline->mScissor);
	vkCmdSetDepthBias(commandbuffer, pipeline->mDepthConstantFactor, pipeline->mDepthClamp, pipeline->mDepthSlopeFactor);
	vkCmdPushConstants(commandbuffer, pipeline->mPipelineLayout, pipeline->mPushConstantShaderStage, 0, sizeof(Vector4f)*pipeline->mPushConstantCount, pipeline->mPushConstants);
}
void aSetDepthBias(AVulkanHandle param, float constant, float clamp, float slope) {
	GraphicPipeline*pipeline = (GraphicPipeline*)param;
	pipeline->mDepthConstantFactor = constant;
	pipeline->mDepthClamp = clamp;
	pipeline->mDepthSlopeFactor = slope;
}