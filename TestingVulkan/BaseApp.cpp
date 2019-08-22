//
//  BaseApp.cpp
//  TestingVulkan
//
//  Created by Armen Khachatryan on 20/08/2019.
//  Copyright © 2019 Armen Khachatryan. All rights reserved.
//

#include "BaseApp.hpp"
#include <iostream>
#include <cmath>


VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


void BaseApp::initVulkan() {
    std::cout << "INFO: Vulkan initilization." << std::endl;
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createBuffer();
    /*
    createInDescriptorSetLayout();
    createOutDescriptorSetLayout();*/
    
    createDescriptorSetLayout();
    createDescriptorSet();
    
    createComputePipeline();
    createCommandBuffer();
    setupInputBuffer();
    // Finally, run the recorded command buffer.
    runCommandBuffer();
    
    reportResult();

}
std::vector<const char*> BaseApp::getRequiredExtensions() {
    /*
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
     */
    
    std::vector<const char*> extensions;
    
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

bool BaseApp::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}


void BaseApp::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        
        createInfo.pNext = nullptr;
    }
    
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

}
void BaseApp::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void BaseApp::setupDebugMessenger() {
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);
    
    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}
void BaseApp::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }
    
    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void BaseApp::createLogicalDevice() {

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    
    queueFamilyIndex = getComputeQueueFamilyIndex(); // find queue family with compute capability.
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    
    queueCreateInfo.queueCount = 1;
    
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    
    VkPhysicalDeviceFeatures deviceFeatures = {};
    
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    
    createInfo.pEnabledFeatures = &deviceFeatures;
    
    createInfo.enabledExtensionCount = 0;
    
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    
    vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);
    
    VkPhysicalDeviceProperties pProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &pProperties);
    
    std::cout << "INFO: working with: " << pProperties.deviceName << std::endl;
    std::cout << "INFO: maxComputeWorkGroupCount is: " << pProperties.limits.maxComputeWorkGroupCount << std::endl;
    std::cout << "INFO: maxComputeWorkGroupSize is: " << pProperties.limits.maxComputeWorkGroupSize << std::endl;
    std::cout << "INFO: maxComputeWorkGroupInvocations is: " << pProperties.limits.maxComputeWorkGroupInvocations << std::endl;
}




bool BaseApp::isDeviceSuitable(VkPhysicalDevice device) {
   // Some check should be done to check if device is sutable for the task.
    // vkGetPhysicalDeviceQueueFamilyProperties
    return true;
}



void BaseApp::createBuffer() {
    /*
     We will now create a buffer. We will render the mandelbrot set into this buffer
     in a computer shade later.
     */
    
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = inBufferSize; // buffer size in bytes.
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer is exclusive to a single queue family at a time.
    
    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &inBuffer)); // create buffer.
    
    bufferCreateInfo.size = outBufferSize; // buffer size in bytes.
    VK_CHECK_RESULT(vkCreateBuffer(device, &bufferCreateInfo, NULL, &outBuffer)); // create buffer.
    
    /*
     But the buffer doesn't allocate memory for itself, so we must do that manually.
     */
    
    /*
     First, we find the memory requirements for the buffer.
     */
    VkMemoryRequirements inMemoryRequirements;
    vkGetBufferMemoryRequirements(device, inBuffer, &inMemoryRequirements);
    
    VkMemoryRequirements outMemoryRequirements;
    vkGetBufferMemoryRequirements(device, outBuffer, &outMemoryRequirements);
    /*
     Now use obtained memory requirements info to allocate the memory for the buffer.
     */
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = inMemoryRequirements.size; // specify required memory.
    /*
     There are several types of memory that can be allocated, and we must choose a memory type that:
     1) Satisfies the memory requirements(memoryRequirements.memoryTypeBits).
     2) Satifies our own usage requirements. We want to be able to read the buffer memory from the GPU to the CPU
     with vkMapMemory, so we set VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
     Also, by setting VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memory written by the device(GPU) will be easily
     visible to the host(CPU), without having to call any extra flushing commands. So mainly for convenience, we set
     this flag.
     */
    allocateInfo.memoryTypeIndex = findMemoryType(
                                                  outMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &inBufferMemory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory.
    VK_CHECK_RESULT(vkBindBufferMemory(device, inBuffer, inBufferMemory, 0));
    
    allocateInfo.allocationSize = outMemoryRequirements.size; // specify required memory.

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocateInfo, NULL, &outBufferMemory)); // allocate memory on device.
    
    // Now associate that allocated memory with the buffer. With that, the buffer is backed by actual memory.
    VK_CHECK_RESULT(vkBindBufferMemory(device, outBuffer, outBufferMemory, 0));
    
}

// find memory type with desired properties.
uint32_t BaseApp::findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    
    /*
     How does this search work?
     See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description.
     */
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }
    return -1;
}


void BaseApp::createDescriptorSetLayout() {

    
    /*
     Here we specify a descriptor set layout. This allows us to bind our descriptors to
     resources in the shader.
     */
    
    /*
     Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to the binding point
     0. This binds to
     layout(std140, binding = 0) buffer buf
     in the compute shader.
     */
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[SET_LAYOUT_COUNT] = {};
    descriptorSetLayoutBinding[0].binding = 0; // binding = 0
    descriptorSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding[0].descriptorCount = 1;
    descriptorSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    descriptorSetLayoutBinding[1].binding = 0; // binding = 0
    descriptorSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBinding[1].descriptorCount = 1;
    descriptorSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    
    
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo[SET_LAYOUT_COUNT] = {};
    descriptorSetLayoutCreateInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo[0].bindingCount = 1;
    descriptorSetLayoutCreateInfo[0].pBindings = &descriptorSetLayoutBinding[0];
    
    descriptorSetLayoutCreateInfo[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo[1].bindingCount = 1;
    descriptorSetLayoutCreateInfo[1].pBindings = &descriptorSetLayoutBinding[1];
    
    
    // Create the descriptor set layout.
    //VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, descriptorSetLayouts));

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo[0], nullptr, &descriptorSetLayouts[0]));
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo[1], nullptr, &descriptorSetLayouts[1]));

}


void BaseApp::createDescriptorSet() {
    /*
     So we will allocate a descriptor set here.
     But we need to first create a descriptor pool to do that.
     */
    
    /*
     Our descriptor pool can only allocate a single storage buffer.
     */
    VkDescriptorPoolSize DescriptorPoolSize = {};
    DescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    DescriptorPoolSize.descriptorCount = 2;
    
    
    //VkDescriptorPoolSize pPoolSizes[2] = {inDescriptorPoolSize, outDescriptorPoolSize};
    
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets = 2; // we only need to allocate one descriptor set from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &DescriptorPoolSize;
    
    // create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL, &descriptorPool));
    
    /*
     With the pool allocated, we can now allocate the descriptor set.
     */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool; // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount = 2; // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts;
    
    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets));


    /*
     Next, we need to connect our actual storage buffer with the descrptor.
     We use vkUpdateDescriptorSets() to update the descriptor set.
     */
    
    // Specify the buffer to bind to the descriptor.
    VkDescriptorBufferInfo descriptorBufferInfo[SET_LAYOUT_COUNT] = {};
    descriptorBufferInfo[0].buffer = inBuffer;
    descriptorBufferInfo[0].offset = 0;
    descriptorBufferInfo[0].range = inBufferSize;
    
    // Specify the buffer to bind to the descriptor.
    descriptorBufferInfo[1].buffer = outBuffer;
    descriptorBufferInfo[1].offset = 0;
    descriptorBufferInfo[1].range = outBufferSize;
    
    
    
    VkWriteDescriptorSet writeDescriptorSet[SET_LAYOUT_COUNT] = {};
    writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[0].dstSet = descriptorSets[0]; // write to this descriptor set.
    writeDescriptorSet[0].dstBinding = 0; // write to the first, and only binding.
    writeDescriptorSet[0].descriptorCount = 1; // update a single descriptor.
    writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    writeDescriptorSet[0].pBufferInfo = &descriptorBufferInfo[0];
    
    writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[1].dstSet = descriptorSets[1]; // write to this descriptor set.
    writeDescriptorSet[1].dstBinding = 0; // write to the first, and only binding.
    writeDescriptorSet[1].descriptorCount = 1; // update a single descriptor.
    writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // storage buffer.
    writeDescriptorSet[1].pBufferInfo = &descriptorBufferInfo[1];
    
    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet[0], 0, NULL);
    // perform the update of the descriptor set.
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet[1], 0, NULL);

    
}

// Read file into array of bytes, and cast to uint32_t*, then return.
// The data has been padded, so that it fits into an array uint32_t.
uint32_t* BaseApp::readFile(uint32_t& length, const char* filename) {
    
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not find or open file: %s\n", filename);
    }
    
    // get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    long filesizepadded = long(ceil(filesize / 4.0)) * 4;
    
    // read file contents.
    char *str = new char[filesizepadded];
    fread(str, filesize, sizeof(char), fp);
    fclose(fp);
    
    // data padding.
    for (int i = filesize; i < filesizepadded; i++) {
        str[i] = 0;
    }
    
    length = filesizepadded;
    return (uint32_t *)str;
}



void BaseApp::createComputePipeline() {
    /*
     We create a compute pipeline here.
     */
    
    /*
     Create a shader module. A shader module basically just encapsulates some shader code.
     */
    uint32_t filelength;
    // the code in comp.spv was created by running the command:
    // glslangValidator.exe -V shader.comp
    uint32_t* code = readFile(filelength, shaderName);
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = code;
    createInfo.codeSize = filelength;
    
    VK_CHECK_RESULT(vkCreateShaderModule(device, &createInfo, NULL, &computeShaderModule));
    delete[] code;
    
    /*
     Now let us actually create the compute pipeline.
     A compute pipeline is very simple compared to a graphics pipeline.
     It only consists of a single stage with a compute shader.
     So first we specify the compute shader stage, and it's entry point(main).
     */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";
    
    
    /*
     The pipeline layout allows the pipeline to access descriptor sets.
     So we just specify the descriptor set layout we created earlier.
     */
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = SET_LAYOUT_COUNT;
    //VkDescriptorSetLayout   pSetLayouts[2] = {inDescriptorSetLayout, outDescriptorSetLayout};
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
    
    VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout));
    
    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    
    
    /*
     Now, we finally create the compute pipeline.
     */
    VK_CHECK_RESULT(vkCreateComputePipelines(
                                             device, VK_NULL_HANDLE,
                                             1, &pipelineCreateInfo,
                                             NULL, &pipeline));
}

// Returns the index of a queue family that supports compute operations.
uint32_t BaseApp::getComputeQueueFamilyIndex() {
    uint32_t queueFamilyCount;
    
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    
    // Retrieve all queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
    
    // Now find a family that supports compute.
    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i) {
        VkQueueFamilyProperties props = queueFamilies[i];
        
        if (props.queueCount > 0 && (props.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            // found a queue with compute. We're done!
            break;
        }
    }
    
    if (i == queueFamilies.size()) {
        throw std::runtime_error("could not find a queue family that supports operations");
    }
    
    return i;
}
void BaseApp::createCommandBuffer() {
    /*
     We are getting closer to the end. In order to send commands to the device(GPU),
     we must first record commands into a command buffer.
     To allocate a command buffer, we must first create a command pool. So let us do that.
     */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    // the queue family of this command pool. All command buffers allocated from this command pool,
    // must be submitted to queues of this family ONLY.
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    VK_CHECK_RESULT(vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool));
    
    /*
     Now allocate a command buffer from the command pool.
     */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool; // specify the command pool to allocate from.
    // if the command buffer is primary, it can be directly submitted to queues.
    // A secondary buffer has to be called from some primary command buffer, and cannot be directly
    // submitted to a queue. To keep things simple, we use a primary command buffer.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1; // allocate a single command buffer.
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)); // allocate command buffer.
    
    /*
     Now we shall start recording commands into the newly allocated command buffer.
     */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // the buffer is only submitted and used once in this application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo)); // start recording commands.
    
    /*
     We need to bind a pipeline, AND a descriptor set before we dispatch.
     The validation layer will NOT give warnings if you forget these, so be very careful not to forget them.
     */
   
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, SET_LAYOUT_COUNT, descriptorSets, 0, NULL);
    
    /*
     Calling vkCmdDispatch basically starts the compute pipeline, and executes the compute shader.
     The number of workgroups is specified in the arguments.
     If you are already familiar with compute shaders from OpenGL, this should be nothing new to you.
     */
    //vkCmdDispatch(commandBuffer, (uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);
    vkCmdDispatch(commandBuffer, (uint32_t)ceil(WORK_TOTAL_SIZE / float(WORKGROUP_SIZE)), 1, 1);
    
    
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer)); // end recording commands.
}

void BaseApp::runCommandBuffer() {
    /*
     Now we shall finally submit the recorded command buffer to a queue.
     */
    
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1; // submit a single command buffer
    submitInfo.pCommandBuffers = &commandBuffer; // the command buffer to submit.
    
    /*
     We create a fence.
     */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(device, &fenceCreateInfo, NULL, &fence));
    
    /*
     We submit the command buffer on the queue, at the same time giving a fence.
     */
    VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
    /*
     The command will not have finished executing until the fence is signalled.
     So we wait here.
     We will directly after this read our buffer from the GPU,
     and we will not be sure that the command has finished executing unless we wait for the fence.
     Hence, we use a fence here.
     */
    VK_CHECK_RESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
    
    vkDestroyFence(device, fence, NULL);
}

void BaseApp::setupInputBuffer() {
    void* inMappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, inBufferMemory, 0, inBufferSize, 0, &inMappedMemory);
    duble_fe25519* inPmappedMemory = (duble_fe25519 *) inMappedMemory;
    for (int i = 0; i < WORK_TOTAL_SIZE; i += 1) {
        inPmappedMemory[i].value[0].value[0] = 10;
        inPmappedMemory[i].value[0].value[1] = 11;
        inPmappedMemory[i].value[0].value[2] = 12;
        inPmappedMemory[i].value[0].value[3] = 13;
        inPmappedMemory[i].value[0].value[4] = 14;
        inPmappedMemory[i].value[0].value[5] = 15;
        inPmappedMemory[i].value[0].value[6] = 16;
        inPmappedMemory[i].value[0].value[7] = 17;
        inPmappedMemory[i].value[0].value[8] = 18;
        inPmappedMemory[i].value[0].value[9] = 19;
        
        inPmappedMemory[i].value[1].value[0] = 0;
        inPmappedMemory[i].value[1].value[1] = 1;
        inPmappedMemory[i].value[1].value[2] = 2;
        inPmappedMemory[i].value[1].value[3] = 3;
        inPmappedMemory[i].value[1].value[4] = 4;
        inPmappedMemory[i].value[1].value[5] = 5;
        inPmappedMemory[i].value[1].value[6] = 6;
        inPmappedMemory[i].value[1].value[7] = 7;
        inPmappedMemory[i].value[1].value[8] = 8;
        inPmappedMemory[i].value[1].value[9] = 9;
        
    }
    //vkUnmapMemory(device, inBufferMemory);
}

void BaseApp::reportResult() {
    /*
    void* inMappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, inBufferMemory, 0, inBufferSize, 0, &inMappedMemory);
    duble_fe25519* inPmappedMemory = (duble_fe25519 *) inMappedMemory;
    */
    void* outMappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(device, outBufferMemory, 0, outBufferSize, 0, &outMappedMemory);
    fe25519* outPmappedMemory = (fe25519 *) outMappedMemory;
    
    // Get the color data from the buffer, and cast it to bytes.
    // We save the data to a vector.
    std::vector<int> image1;
    image1.reserve(WORK_TOTAL_SIZE * 10);
    
    for (int i = 0; i < 1; i += 1) {
        image1.push_back(((outPmappedMemory[i].value[0])));
        image1.push_back(((outPmappedMemory[i].value[1])));
        image1.push_back(((outPmappedMemory[i].value[2])));
        image1.push_back(((outPmappedMemory[i].value[3])));
        image1.push_back(((outPmappedMemory[i].value[4])));
        image1.push_back(((outPmappedMemory[i].value[5])));
        image1.push_back(((outPmappedMemory[i].value[6])));
        image1.push_back(((outPmappedMemory[i].value[7])));
        image1.push_back(((outPmappedMemory[i].value[8])));
        image1.push_back(((outPmappedMemory[i].value[9])));
        
    }

    // Done reading, so unmap.
    //vkUnmapMemory(device, inBufferMemory);
    vkUnmapMemory(device, outBufferMemory);
    
    for (int i = 0; i < 10; i += 1) {
        std::cout << "INFO: Output was " << outPmappedMemory[0].value[i] << std::endl;
    }
}

void BaseApp::cleanup() {
    /*
     Clean up all Vulkan Resources.
     */
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkFreeMemory(device, inBufferMemory, NULL);
    vkDestroyBuffer(device, inBuffer, NULL);
    vkFreeMemory(device, outBufferMemory, NULL);
    vkDestroyBuffer(device, outBuffer, NULL);
    vkDestroyShaderModule(device, computeShaderModule, NULL);
    vkDestroyDescriptorPool(device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    
}
