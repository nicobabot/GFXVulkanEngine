C:/VulkanSDK/1.3.261.1/Bin/glslc.exe Shaders/shader.vert -O0 -g -o CompiledShaders/vert.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe Shaders/shader.frag -O0 -g -o CompiledShaders/frag.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe Shaders/brdfShader.frag -O0 -g -o CompiledShaders/brdfFrag.spv
C:/VulkanSDK/1.3.261.1/Bin/glslc.exe -fshader-stage=compute Shaders/compute.glsl -O0 -g -o CompiledShaders/compute.spv
::pause