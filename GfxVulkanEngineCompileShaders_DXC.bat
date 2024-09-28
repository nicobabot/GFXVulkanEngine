C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/baseShadervertex_preprocessed.hlsl Shaders/baseShader.hlsl
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/baseShadervertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/vert.spv
C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/baseShaderfragment_preprocessed.hlsl Shaders/baseShader.hlsl 
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/baseShaderfragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/frag.spv
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\vert.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\frag.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
::C:\DXC\bin\x64\dxc.exe -spirv Shaders/baseShader.hlsl -T vs_6_0 -E VSMain -Fo GFXVulkanEngine/x64/Debug/CompiledShaders/vert.spv
::C:\DXC\bin\x64\dxc.exe -spirv Shaders/baseShader.hlsl -T ps_6_0 -E PSMain -Fo GFXVulkanEngine/x64/Debug/CompiledShaders/frag.spv
::pause

C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/shadowMapVertex_preprocessed.hlsl Shaders/dirShadowMapDepth.hlsl
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/shadowMapVertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/shadowMapVert.spv
C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/shadowMapFragment_preprocessed.hlsl Shaders/dirShadowMapDepth.hlsl 
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/shadowMapFragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/shadowMapFrag.spv
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\shadowMapVert.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\shadowMapFrag.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"

C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl Shaders/postProcessPresent.hlsl
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/postProcessPresentVert.spv
C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl Shaders/postProcessPresent.hlsl 
C:\DXC\bin\x64\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/postProcessPresentFrag.spv
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\postProcessPresentVert.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\postProcessPresentFrag.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"


C:\DXC\bin\x64\dxc.exe -P -Fi Shaders/PreprocessedShaders/cs_blur_preprocessed.hlsl Shaders/cs_blur.hlsl
C:\DXC\bin\x64\dxc.exe -T cs_6_0 -E main -spirv -Fo CompiledShaders/cs_blur.spv -Zi -O3 Shaders/PreprocessedShaders/cs_blur_preprocessed.hlsl
copy "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\CompiledShaders\cs_blur.spv" "C:\Users\nicob\source\repos\GFXVulkanEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"