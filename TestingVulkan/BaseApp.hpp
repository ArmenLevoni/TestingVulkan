//
//  BaseApp.hpp
//  TestingVulkan
//
//  Created by Armen Khachatryan on 20/08/2019.
//  Copyright © 2019 Armen Khachatryan. All rights reserved.
//

#ifndef BaseApp_hpp
#define BaseApp_hpp
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <vector>
#include <iostream>

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                                                                 \
{                                                                                                        \
VkResult res = (f);                                                                                    \
if (res != VK_SUCCESS)                                                                                \
{                                                                                                    \
printf("Fatal : VkResult is %d in %s at line %d\n", res,  __FILE__, __LINE__); \
assert(res == VK_SUCCESS);                                                                        \
}                                                                                                    \
}

const int WORK_TOTAL_SIZE = 16;
const int WORKGROUP_SIZE = 16; // Workgroup size in compute shader.

class BaseApp {

public:
    const char* shaderName = "ed25519.spv";
    
    struct fe25519 {
        int value [10];
    };
    struct duble_fe25519 {
        fe25519 value [2];
    };

    uint32_t inBufferSize; // size of `buffer` in bytes.
    uint32_t outBufferSize; // size of `buffer` in bytes.
    
protected:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    
    static const uint32_t SET_LAYOUT_COUNT = 2;
    
    // Vulkan objects:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    
    /*
     In order to execute commands on a device(GPU), the commands must be submitted
     to a queue. The commands are stored in a command buffer, and this command buffer
     is given to the queue.
     There will be different kinds of queues on the device. Not all queues support
     graphics operations, for instance. For this application, we at least want a queue
     that supports compute operations.
     */
    VkQueue queue; // a queue supporting compute operations.
    /*
     Groups of queues that have the same capabilities(for instance, they all supports graphics and computer operations),
     are grouped into queue families.
     
     When submitting a command buffer, you must specify to which queue in the family you are submitting to.
     This variable keeps track of the index of that queue in its family.
     */
    uint32_t queueFamilyIndex;

    
    /*
     The pipeline specifies the pipeline that all graphics and compute commands pass though in Vulkan.
     We will be creating a simple compute pipeline in this application.
     */
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkShaderModule computeShaderModule;
    
    /*
     The command buffer is used to record commands, that will be submitted to a queue.
     To allocate such command buffers, we use a command pool.
     */
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    
    /*
     Descriptors represent resources in shaders. They allow us to use things like
     uniform buffers, storage buffers and images in GLSL.
     A single descriptor represents a single resource, and several descriptors are organized
     into descriptor sets, which are basically just collections of descriptors.
     */
    
    
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets [SET_LAYOUT_COUNT];

    VkDescriptorSetLayout descriptorSetLayouts[SET_LAYOUT_COUNT];
    VkDescriptorSetLayout descriptorSetLayout;
    
    /*
     The memory that backs the buffer is bufferMemory.
     */
    VkBuffer inBuffer;
    VkDeviceMemory inBufferMemory;
    
    /*
     The memory that backs the buffer is bufferMemory.
     */
    VkBuffer outBuffer;
    VkDeviceMemory outBufferMemory;
    

    public:
    void run() {
        // std::cout << "INFO: Let's start: "<<enableValidationLayers<< std::endl;
        initVulkan();
        mainLoop();
        cleanup();
    }
    
    protected:
    void initVulkan();

    void mainLoop();

    void cleanup ();

    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        
        return VK_FALSE;
    }
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    void createLogicalDevice();
    
    
    // Returns the index of a queue family that supports compute operations.
    uint32_t getComputeQueueFamilyIndex();
    void createBuffer();
    uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties);
    void createInDescriptorSetLayout();
    void createOutDescriptorSetLayout();
    void createDescriptorSet();
    uint32_t* readFile(uint32_t& length, const char* filename);
    void createComputePipeline();
    void createCommandBuffer();
    void runCommandBuffer();
    void createDescriptorSetLayout();
    void reportResult();
    void setupInputBuffer();
};


#endif /* BaseApp_hpp */
