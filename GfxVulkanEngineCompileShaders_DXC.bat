C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/baseShadervertex_preprocessed.hlsl Shaders/baseShader.hlsl
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/baseShadervertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/vert.spv
C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/baseShaderfragment_preprocessed.hlsl Shaders/baseShader.hlsl 
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/baseShaderfragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/frag.spv
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\vert.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\frag.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
::C:\DXC\bin\x64\dxc.exe -spirv Shaders/baseShader.hlsl -T vs_6_0 -E VSMain -Fo GFXVulkanEngine/x64/Debug/CompiledShaders/vert.spv
::C:\DXC\bin\x64\dxc.exe -spirv Shaders/baseShader.hlsl -T ps_6_0 -E PSMain -Fo GFXVulkanEngine/x64/Debug/CompiledShaders/frag.spv
::pause

C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/shadowMapVertex_preprocessed.hlsl Shaders/dirShadowMapDepth.hlsl
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/shadowMapVertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/shadowMapVert.spv
C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/shadowMapFragment_preprocessed.hlsl Shaders/dirShadowMapDepth.hlsl 
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/shadowMapFragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/shadowMapFrag.spv
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\shadowMapVert.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\shadowMapFrag.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
