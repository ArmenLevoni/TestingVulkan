/Users/armkha01/vulkan/sdk/macOS/bin/glslc shader.vert -o vert.spv
/Users/armkha01/vulkan/sdk/macOS/bin/glslc shader.frag   -o frag.spv
/Users/armkha01/vulkan/sdk/macOS/bin/glslc shader.comp   -o comp.spv


/Users/armkha01/vulkan/sdk/macOS/bin/glslc shader.frag -S -o shader.frag.spvasm
/Users/armkha01/vulkan/sdk/macOS/bin/glslc shader.vert -S shader.vert.spvasm

/Users/armkha01/vulkan/sdk/macOS/bin/glslc ed25519_ref10_fe_25_5.comp -o ed25519.spv
