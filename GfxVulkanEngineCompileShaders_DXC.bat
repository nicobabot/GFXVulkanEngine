C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/baseShader.hlsl -Fi Shaders/PreprocessedShaders/baseShadervertex_preprocessed.hlsl
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/baseShadervertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/vert.spv
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/baseShader.hlsl -Fi Shaders/PreprocessedShaders/baseShaderfragment_preprocessed.hlsl
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/baseShaderfragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/frag.spv
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\vert.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\frag.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"

C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/dirShadowMapDepth.hlsl -Fi Shaders/PreprocessedShaders/shadowMapVertex_preprocessed.hlsl
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/shadowMapVertex_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/shadowMapVert.spv
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/dirShadowMapDepth.hlsl -Fi Shaders/PreprocessedShaders/shadowMapFragment_preprocessed.hlsl 
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/shadowMapFragment_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/shadowMapFrag.spv
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\shadowMapVert.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\shadowMapFrag.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"

C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/postProcessPresent.hlsl -Fi Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/postProcessPresentVert.spv
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/postProcessPresent.hlsl -Fi Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl 
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/postProcessPresent_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/postProcessPresentFrag.spv
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\postProcessPresentVert.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\postProcessPresentFrag.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"


C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P Shaders/cs_blur.hlsl -Fi Shaders/PreprocessedShaders/cs_blur_preprocessed.hlsl
C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -T cs_6_0 -E main -spirv -Fo CompiledShaders/cs_blur.spv -Zi -O3 Shaders/PreprocessedShaders/cs_blur_preprocessed.hlsl
copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\cs_blur.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"

::C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P -Fi Shaders/PreprocessedShaders/decals_preprocessed.hlsl Shaders/decals.hlsl
::C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/decals_preprocessed.hlsl -T vs_6_2 -E VSMain -Fo CompiledShaders/decalsVert.spv
::C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -P -Fi Shaders/PreprocessedShaders/decals_preprocessed.hlsl Shaders/decals.hlsl 
::C:\VulkanSDK\1.3.204.0\Bin\dxc.exe -spirv -Zi -O3 Shaders/PreprocessedShaders/decals_preprocessed.hlsl -T ps_6_2 -E PSMain -Fo CompiledShaders/decalsFrag.spv
::copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\decalsVert.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
::copy "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\CompiledShaders\decalsFrag.spv" "D:\Documentos\C++Projects\AEngine\GFXVulkanEngine\GFXVulkanEngine\x64\Debug\CompiledShaders\"
