/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Baldur Karlsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "vk_shader_cache.h"
#include "common/shader_cache.h"
#include "data/glsl_shaders.h"
#include "strings/string_utils.h"

enum class FeatureCheck
{
  NoCheck = 0x0,
  ShaderMSAAStorage = 0x1,
  FragmentStores = 0x2,
  NonMetalBackend = 0x4,
  FormatlessWrite = 0x8,
};

BITMASK_OPERATORS(FeatureCheck);

struct BuiltinShaderConfig
{
  BuiltinShader builtin;
  EmbeddedResourceType resource;
  rdcspv::ShaderStage stage;
  FeatureCheck checks;
  bool uniforms;
};

static const BuiltinShaderConfig builtinShaders[] = {
    {BuiltinShader::BlitVS, EmbeddedResource(glsl_blit_vert), rdcspv::ShaderStage::Vertex,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::CheckerboardFS, EmbeddedResource(glsl_checkerboard_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NoCheck, true},
    {BuiltinShader::TexDisplayFS, EmbeddedResource(glsl_texdisplay_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NoCheck, true},
    {BuiltinShader::FixedColFS, EmbeddedResource(glsl_fixedcol_frag), rdcspv::ShaderStage::Fragment,
     FeatureCheck::NoCheck, false},
    {BuiltinShader::TextVS, EmbeddedResource(glsl_vktext_vert), rdcspv::ShaderStage::Vertex,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::TextFS, EmbeddedResource(glsl_vktext_frag), rdcspv::ShaderStage::Fragment,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::MeshVS, EmbeddedResource(glsl_mesh_vert), rdcspv::ShaderStage::Vertex,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::MeshGS, EmbeddedResource(glsl_mesh_geom), rdcspv::ShaderStage::Geometry,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::MeshFS, EmbeddedResource(glsl_mesh_frag), rdcspv::ShaderStage::Fragment,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::MeshCS, EmbeddedResource(glsl_mesh_comp), rdcspv::ShaderStage::Compute,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::QuadResolveFS, EmbeddedResource(glsl_quadresolve_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::FragmentStores, true},
    {BuiltinShader::QuadWriteFS, EmbeddedResource(glsl_quadwrite_frag), rdcspv::ShaderStage::Fragment,
     FeatureCheck::FragmentStores | FeatureCheck::NonMetalBackend, false},
    {BuiltinShader::TrisizeGS, EmbeddedResource(glsl_trisize_geom), rdcspv::ShaderStage::Geometry,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::TrisizeFS, EmbeddedResource(glsl_trisize_frag), rdcspv::ShaderStage::Fragment,
     FeatureCheck::NoCheck, true},
    {BuiltinShader::MS2ArrayCS, EmbeddedResource(glsl_ms2array_comp), rdcspv::ShaderStage::Compute,
     FeatureCheck::FormatlessWrite | FeatureCheck::NonMetalBackend, true},
    {BuiltinShader::Array2MSCS, EmbeddedResource(glsl_array2ms_comp), rdcspv::ShaderStage::Compute,
     FeatureCheck::ShaderMSAAStorage | FeatureCheck::FormatlessWrite | FeatureCheck::NonMetalBackend,
     true},
    {BuiltinShader::DepthMS2ArrayFS, EmbeddedResource(glsl_depthms2arr_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NonMetalBackend, true},
    {BuiltinShader::DepthArray2MSFS, EmbeddedResource(glsl_deptharr2ms_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NonMetalBackend, true},
    {BuiltinShader::TexRemapFloat, EmbeddedResource(glsl_texremap_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NoCheck, true},
    {BuiltinShader::TexRemapUInt, EmbeddedResource(glsl_texremap_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NoCheck, true},
    {BuiltinShader::TexRemapSInt, EmbeddedResource(glsl_texremap_frag),
     rdcspv::ShaderStage::Fragment, FeatureCheck::NoCheck, true},
};

RDCCOMPILE_ASSERT(ARRAY_COUNT(builtinShaders) == arraydim<BuiltinShader>(),
                  "Missing built-in shader config");

struct VulkanBlobShaderCallbacks
{
  bool Create(uint32_t size, byte *data, SPIRVBlob *ret) const
  {
    RDCASSERT(ret);

    SPIRVBlob blob = new std::vector<uint32_t>();

    blob->resize(size / sizeof(uint32_t));

    memcpy(&(*blob)[0], data, size);

    *ret = blob;

    return true;
  }

  void Destroy(SPIRVBlob blob) const { delete blob; }
  uint32_t GetSize(SPIRVBlob blob) const { return (uint32_t)(blob->size() * sizeof(uint32_t)); }
  const byte *GetData(SPIRVBlob blob) const { return (const byte *)blob->data(); }
} VulkanShaderCacheCallbacks;

VulkanShaderCache::VulkanShaderCache(WrappedVulkan *driver)
{
  // Load shader cache, if present
  bool success = LoadShaderCache("vkshaders.cache", m_ShaderCacheMagic, m_ShaderCacheVersion,
                                 m_ShaderCache, VulkanShaderCacheCallbacks);

  // if we failed to load from the cache
  m_ShaderCacheDirty = !success;

  m_pDriver = driver;
  m_Device = driver->GetDev();

  SetCaching(true);

  VkDriverInfo driverVersion = driver->GetDriverInfo();
  const VkPhysicalDeviceFeatures &features = driver->GetDeviceFeatures();

  m_GlobalDefines = "#define HAS_BIT_CONVERSION 1\n";
  if(driverVersion.TexelFetchBrokenDriver())
    m_GlobalDefines += "#define NO_TEXEL_FETCH\n";
  if(driverVersion.RunningOnMetal())
    m_GlobalDefines += "#define METAL_BACKEND\n";

  std::string src;
  rdcspv::CompilationSettings compileSettings;
  compileSettings.lang = rdcspv::InputLanguage::VulkanGLSL;

  for(auto i : indices<BuiltinShader>())
  {
    const BuiltinShaderConfig &config = builtinShaders[i];

    RDCASSERT(config.builtin == (BuiltinShader)i);

    if(config.checks & FeatureCheck::ShaderMSAAStorage)
    {
      if(driverVersion.TexelFetchBrokenDriver() || driverVersion.AMDStorageMSAABrokenDriver() ||
         !features.shaderStorageImageMultisample)
      {
        continue;
      }
    }

    if(config.checks & FeatureCheck::FormatlessWrite)
    {
      if(!features.shaderStorageImageWriteWithoutFormat)
      {
        continue;
      }
    }

    if(config.checks & FeatureCheck::FragmentStores)
    {
      if(!features.fragmentStoresAndAtomics)
        continue;
    }

    if(config.checks & FeatureCheck::NonMetalBackend)
    {
      // for now we don't allow it at all - in future we could check on whether it's been enabled
      // via a more advanced query
      if(driverVersion.RunningOnMetal())
        continue;
    }

    if(config.stage == rdcspv::ShaderStage::Geometry && !features.geometryShader)
      continue;

    std::string defines = m_GlobalDefines;

    if(config.builtin == BuiltinShader::TexRemapFloat)
      defines += std::string("#define UINT_TEX 0\n#define SINT_TEX 0\n");
    else if(config.builtin == BuiltinShader::TexRemapUInt)
      defines += std::string("#define UINT_TEX 1\n#define SINT_TEX 0\n");
    else if(config.builtin == BuiltinShader::TexRemapSInt)
      defines += std::string("#define UINT_TEX 0\n#define SINT_TEX 1\n");

    src = GenerateGLSLShader(GetDynamicEmbeddedResource(config.resource), ShaderType::Vulkan, 430,
                             defines);

    compileSettings.stage = config.stage;
    std::string err = GetSPIRVBlob(compileSettings, src, m_BuiltinShaderBlobs[i]);

    if(!err.empty() || m_BuiltinShaderBlobs[i] == VK_NULL_HANDLE)
    {
      RDCERR("Error compiling builtin %u: %s", (uint32_t)i, err.c_str());
    }
    else
    {
      VkShaderModuleCreateInfo modinfo = {
          VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
          NULL,
          0,
          m_BuiltinShaderBlobs[i]->size() * sizeof(uint32_t),
          m_BuiltinShaderBlobs[i]->data(),
      };

      VkResult vkr =
          driver->vkCreateShaderModule(m_Device, &modinfo, NULL, &m_BuiltinShaderModules[i]);
      RDCASSERTEQUAL(vkr, VK_SUCCESS);

      driver->GetResourceManager()->SetInternalResource(GetResID(m_BuiltinShaderModules[i]));
    }
  }

  SetCaching(false);
}

VulkanShaderCache::~VulkanShaderCache()
{
  if(m_ShaderCacheDirty)
  {
    SaveShaderCache("vkshaders.cache", m_ShaderCacheMagic, m_ShaderCacheVersion, m_ShaderCache,
                    VulkanShaderCacheCallbacks);
  }
  else
  {
    for(auto it = m_ShaderCache.begin(); it != m_ShaderCache.end(); ++it)
      VulkanShaderCacheCallbacks.Destroy(it->second);
  }

  for(size_t i = 0; i < ARRAY_COUNT(m_BuiltinShaderModules); i++)
    m_pDriver->vkDestroyShaderModule(m_Device, m_BuiltinShaderModules[i], NULL);
}

std::string VulkanShaderCache::GetSPIRVBlob(const rdcspv::CompilationSettings &settings,
                                            const std::string &src, SPIRVBlob &outBlob)
{
  RDCASSERT(!src.empty());

  uint32_t hash = strhash(src.c_str());

  char typestr[3] = {'a', 'a', 0};
  typestr[0] += (char)settings.stage;
  typestr[1] += (char)settings.lang;
  hash = strhash(typestr, hash);

  if(m_ShaderCache.find(hash) != m_ShaderCache.end())
  {
    outBlob = m_ShaderCache[hash];
    return "";
  }

  SPIRVBlob spirv = new std::vector<uint32_t>();
  std::string errors = rdcspv::Compile(settings, {src}, *spirv);

  if(!errors.empty())
  {
    std::string logerror = errors;
    if(logerror.length() > 1024)
      logerror = logerror.substr(0, 1024) + "...";

    RDCWARN("Shader compile error:\n%s", logerror.c_str());

    delete spirv;
    outBlob = NULL;
    return errors;
  }

  outBlob = spirv;

  if(m_CacheShaders)
  {
    m_ShaderCache[hash] = spirv;
    m_ShaderCacheDirty = true;
  }

  return errors;
}

void VulkanShaderCache::MakeGraphicsPipelineInfo(VkGraphicsPipelineCreateInfo &pipeCreateInfo,
                                                 ResourceId pipeline)
{
  const VulkanCreationInfo::Pipeline &pipeInfo = m_pDriver->m_CreationInfo.m_Pipeline[pipeline];

  VulkanResourceManager *rm = m_pDriver->GetResourceManager();

  static VkPipelineShaderStageCreateInfo stages[6];
  static VkSpecializationInfo specInfo[6];
  static std::vector<VkSpecializationMapEntry> specMapEntries;

  // the specialization constants can't use more than a uint64_t, so we just over-allocate
  static std::vector<uint64_t> specdata;

  size_t specEntries = 0;

  for(uint32_t i = 0; i < 6; i++)
    specEntries += pipeInfo.shaders[i].specialization.size();

  specMapEntries.resize(specEntries);
  specdata.resize(specEntries);

  VkSpecializationMapEntry *entry = specMapEntries.data();
  uint64_t *data = specdata.data();

  uint32_t stageCount = 0;
  uint32_t dataOffset = 0;

  // reserve space for spec constants
  for(uint32_t i = 0; i < 6; i++)
  {
    if(pipeInfo.shaders[i].module != ResourceId())
    {
      stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stages[stageCount].stage = (VkShaderStageFlagBits)(1 << i);
      stages[stageCount].module = rm->GetCurrentHandle<VkShaderModule>(pipeInfo.shaders[i].module);
      stages[stageCount].pName = pipeInfo.shaders[i].entryPoint.c_str();
      stages[stageCount].pNext = NULL;
      stages[stageCount].pSpecializationInfo = NULL;

      if(!pipeInfo.shaders[i].specialization.empty())
      {
        stages[stageCount].pSpecializationInfo = &specInfo[i];
        specInfo[i].pMapEntries = entry;
        specInfo[i].mapEntryCount = (uint32_t)pipeInfo.shaders[i].specialization.size();

        for(size_t s = 0; s < pipeInfo.shaders[i].specialization.size(); s++)
        {
          entry[s].constantID = pipeInfo.shaders[i].specialization[s].specID;
          entry[s].size = pipeInfo.shaders[i].specialization[s].dataSize;
          entry[s].offset = dataOffset * sizeof(uint64_t);

          data[dataOffset] = pipeInfo.shaders[i].specialization[s].value;

          dataOffset++;
        }

        specInfo[i].dataSize = specdata.size() * sizeof(uint64_t);
        specInfo[i].pData = specdata.data();

        entry += specInfo[i].mapEntryCount;
      }

      stageCount++;
    }
  }

  static VkPipelineVertexInputStateCreateInfo vi = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
  };

  vi.pNext = NULL;

  static VkVertexInputAttributeDescription viattr[128] = {};
  static VkVertexInputBindingDescription vibind[128] = {};

  vi.pVertexAttributeDescriptions = viattr;
  vi.pVertexBindingDescriptions = vibind;

  vi.vertexAttributeDescriptionCount = (uint32_t)pipeInfo.vertexAttrs.size();
  vi.vertexBindingDescriptionCount = (uint32_t)pipeInfo.vertexBindings.size();

  for(uint32_t i = 0; i < vi.vertexAttributeDescriptionCount; i++)
  {
    viattr[i].binding = pipeInfo.vertexAttrs[i].binding;
    viattr[i].offset = pipeInfo.vertexAttrs[i].byteoffset;
    viattr[i].format = pipeInfo.vertexAttrs[i].format;
    viattr[i].location = pipeInfo.vertexAttrs[i].location;
  }

  for(uint32_t i = 0; i < vi.vertexBindingDescriptionCount; i++)
  {
    vibind[i].binding = pipeInfo.vertexBindings[i].vbufferBinding;
    vibind[i].stride = pipeInfo.vertexBindings[i].bytestride;
    vibind[i].inputRate = pipeInfo.vertexBindings[i].perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE
                                                                 : VK_VERTEX_INPUT_RATE_VERTEX;
  }

  static VkPipelineVertexInputDivisorStateCreateInfoEXT vertexDivisor = {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT,
  };
  static VkVertexInputBindingDivisorDescriptionEXT vibindDivisors[128] = {};

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_vertex_attribute_divisor)
  {
    vertexDivisor.pVertexBindingDivisors = vibindDivisors;
    vertexDivisor.vertexBindingDivisorCount = vi.vertexBindingDescriptionCount;

    for(uint32_t i = 0; i < vi.vertexBindingDescriptionCount; i++)
    {
      vibindDivisors[i].binding = i;
      vibindDivisors[i].divisor = pipeInfo.vertexBindings[i].instanceDivisor;
    }

    vertexDivisor.pNext = vi.pNext;
    vi.pNext = &vertexDivisor;
  }

  RDCASSERT(ARRAY_COUNT(viattr) >= pipeInfo.vertexAttrs.size());
  RDCASSERT(ARRAY_COUNT(vibind) >= pipeInfo.vertexBindings.size());

  static VkPipelineInputAssemblyStateCreateInfo ia = {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

  ia.topology = pipeInfo.topology;
  ia.primitiveRestartEnable = pipeInfo.primitiveRestartEnable;

  static VkPipelineTessellationStateCreateInfo tess = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};

  tess.pNext = NULL;

  tess.patchControlPoints = pipeInfo.patchControlPoints;

  static VkPipelineTessellationDomainOriginStateCreateInfo tessDomain = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_KHR_maintenance2)
  {
    tessDomain.domainOrigin = pipeInfo.tessellationDomainOrigin;

    tessDomain.pNext = tess.pNext;
    tess.pNext = &tessDomain;
  }

  static VkPipelineViewportStateCreateInfo vp = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

  static VkViewport views[32] = {};
  static VkRect2D scissors[32] = {};

  memcpy(views, &pipeInfo.viewports[0], pipeInfo.viewports.size() * sizeof(VkViewport));

  vp.pViewports = &views[0];
  vp.viewportCount = (uint32_t)pipeInfo.viewports.size();

  memcpy(scissors, &pipeInfo.scissors[0], pipeInfo.scissors.size() * sizeof(VkRect2D));

  vp.pScissors = &scissors[0];
  vp.scissorCount = (uint32_t)pipeInfo.scissors.size();

  RDCASSERT(ARRAY_COUNT(views) >= pipeInfo.viewports.size());
  RDCASSERT(ARRAY_COUNT(scissors) >= pipeInfo.scissors.size());

  static VkPipelineRasterizationStateCreateInfo rs = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
  };

  rs.pNext = NULL;

  rs.depthClampEnable = pipeInfo.depthClampEnable;
  rs.rasterizerDiscardEnable = pipeInfo.rasterizerDiscardEnable;
  rs.polygonMode = pipeInfo.polygonMode;
  rs.cullMode = pipeInfo.cullMode;
  rs.frontFace = pipeInfo.frontFace;
  rs.depthBiasEnable = pipeInfo.depthBiasEnable;
  rs.depthBiasConstantFactor = pipeInfo.depthBiasConstantFactor;
  rs.depthBiasClamp = pipeInfo.depthBiasClamp;
  rs.depthBiasSlopeFactor = pipeInfo.depthBiasSlopeFactor;
  rs.lineWidth = pipeInfo.lineWidth;

  static VkPipelineRasterizationConservativeStateCreateInfoEXT conservRast = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_conservative_rasterization)
  {
    conservRast.conservativeRasterizationMode = pipeInfo.conservativeRasterizationMode;
    conservRast.extraPrimitiveOverestimationSize = pipeInfo.extraPrimitiveOverestimationSize;

    conservRast.pNext = rs.pNext;
    rs.pNext = &conservRast;
  }

  static VkPipelineRasterizationStateStreamCreateInfoEXT rastStream = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_transform_feedback)
  {
    rastStream.rasterizationStream = pipeInfo.rasterizationStream;

    rastStream.pNext = rs.pNext;
    rs.pNext = &rastStream;
  }

  static VkPipelineRasterizationDepthClipStateCreateInfoEXT depthClipState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_depth_clip_enable)
  {
    depthClipState.depthClipEnable = pipeInfo.depthClipEnable;

    depthClipState.pNext = rs.pNext;
    rs.pNext = &depthClipState;
  }

  static VkPipelineRasterizationLineStateCreateInfoEXT lineRasterState = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_line_rasterization)
  {
    lineRasterState.lineRasterizationMode = pipeInfo.lineRasterMode;
    lineRasterState.stippledLineEnable = pipeInfo.stippleEnabled;
    lineRasterState.lineStippleFactor = pipeInfo.stippleFactor;
    lineRasterState.lineStipplePattern = pipeInfo.stipplePattern;

    lineRasterState.pNext = rs.pNext;
    rs.pNext = &lineRasterState;
  }

  static VkPipelineMultisampleStateCreateInfo msaa = {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
  };

  msaa.pNext = NULL;

  msaa.rasterizationSamples = pipeInfo.rasterizationSamples;
  msaa.sampleShadingEnable = pipeInfo.sampleShadingEnable;
  msaa.minSampleShading = pipeInfo.minSampleShading;
  msaa.pSampleMask = &pipeInfo.sampleMask;
  msaa.alphaToCoverageEnable = pipeInfo.alphaToCoverageEnable;
  msaa.alphaToOneEnable = pipeInfo.alphaToOneEnable;

  static VkPipelineSampleLocationsStateCreateInfoEXT sampleLoc = {
      VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_sample_locations)
  {
    sampleLoc.sampleLocationsEnable = pipeInfo.sampleLocations.enabled;
    sampleLoc.sampleLocationsInfo.sampleLocationGridSize = pipeInfo.sampleLocations.gridSize;
    sampleLoc.sampleLocationsInfo.sampleLocationsPerPixel = pipeInfo.rasterizationSamples;
    sampleLoc.sampleLocationsInfo.sampleLocationsCount =
        (uint32_t)pipeInfo.sampleLocations.locations.size();
    sampleLoc.sampleLocationsInfo.pSampleLocations = pipeInfo.sampleLocations.locations.data();

    sampleLoc.pNext = msaa.pNext;
    msaa.pNext = &sampleLoc;
  }

  static VkPipelineDepthStencilStateCreateInfo ds = {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

  ds.depthTestEnable = pipeInfo.depthTestEnable;
  ds.depthWriteEnable = pipeInfo.depthWriteEnable;
  ds.depthCompareOp = pipeInfo.depthCompareOp;
  ds.depthBoundsTestEnable = pipeInfo.depthBoundsEnable;
  ds.stencilTestEnable = pipeInfo.stencilTestEnable;
  ds.front = pipeInfo.front;
  ds.back = pipeInfo.back;
  ds.minDepthBounds = pipeInfo.minDepthBounds;
  ds.maxDepthBounds = pipeInfo.maxDepthBounds;

  static VkPipelineColorBlendStateCreateInfo cb = {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

  cb.logicOpEnable = pipeInfo.logicOpEnable;
  cb.logicOp = pipeInfo.logicOp;
  memcpy(cb.blendConstants, pipeInfo.blendConst, sizeof(cb.blendConstants));

  static VkPipelineColorBlendAttachmentState atts[32] = {};

  cb.attachmentCount = (uint32_t)pipeInfo.attachments.size();
  cb.pAttachments = atts;

  for(uint32_t i = 0; i < cb.attachmentCount; i++)
  {
    atts[i].blendEnable = pipeInfo.attachments[i].blendEnable;
    atts[i].colorWriteMask = pipeInfo.attachments[i].channelWriteMask;
    atts[i].alphaBlendOp = pipeInfo.attachments[i].alphaBlend.Operation;
    atts[i].srcAlphaBlendFactor = pipeInfo.attachments[i].alphaBlend.Source;
    atts[i].dstAlphaBlendFactor = pipeInfo.attachments[i].alphaBlend.Destination;
    atts[i].colorBlendOp = pipeInfo.attachments[i].blend.Operation;
    atts[i].srcColorBlendFactor = pipeInfo.attachments[i].blend.Source;
    atts[i].dstColorBlendFactor = pipeInfo.attachments[i].blend.Destination;
  }

  RDCASSERT(ARRAY_COUNT(atts) >= pipeInfo.attachments.size());

  static VkDynamicState dynSt[VkDynamicCount];

  static VkPipelineDynamicStateCreateInfo dyn = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};

  dyn.dynamicStateCount = 0;
  dyn.pDynamicStates = dynSt;

  for(uint32_t i = 0; i < VkDynamicCount; i++)
    if(pipeInfo.dynamicStates[i])
      dynSt[dyn.dynamicStateCount++] = ConvertDynamicState((VulkanDynamicStateIndex)i);

  // since we don't have to worry about threading, we point everything at the above static structs

  VkGraphicsPipelineCreateInfo ret = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      NULL,
      pipeInfo.flags,
      stageCount,
      stages,
      &vi,
      &ia,
      &tess,
      &vp,
      &rs,
      &msaa,
      &ds,
      &cb,
      &dyn,
      rm->GetCurrentHandle<VkPipelineLayout>(pipeInfo.layout),
      rm->GetCurrentHandle<VkRenderPass>(pipeInfo.renderpass),
      pipeInfo.subpass,
      VK_NULL_HANDLE,    // base pipeline handle
      0,                 // base pipeline index
  };

  static VkPipelineDiscardRectangleStateCreateInfoEXT discardRects = {
      VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT,
  };

  if(m_pDriver->GetExtensions(GetRecord(m_Device)).ext_EXT_discard_rectangles)
  {
    discardRects.discardRectangleMode = pipeInfo.discardMode;
    discardRects.discardRectangleCount = (uint32_t)pipeInfo.discardRectangles.size();
    discardRects.pDiscardRectangles = pipeInfo.discardRectangles.data();

    discardRects.pNext = ret.pNext;
    ret.pNext = &discardRects;
  }

  // never create derivatives
  ret.flags &= ~VK_PIPELINE_CREATE_DERIVATIVE_BIT;

  pipeCreateInfo = ret;
}

void VulkanShaderCache::MakeComputePipelineInfo(VkComputePipelineCreateInfo &pipeCreateInfo,
                                                ResourceId pipeline)
{
  const VulkanCreationInfo::Pipeline &pipeInfo = m_pDriver->m_CreationInfo.m_Pipeline[pipeline];

  VulkanResourceManager *rm = m_pDriver->GetResourceManager();

  VkPipelineShaderStageCreateInfo stage;    // Returned by value
  static VkSpecializationInfo specInfo;
  static std::vector<VkSpecializationMapEntry> specMapEntries;

  // the specialization constants can't use more than a uint64_t, so we just over-allocate
  static std::vector<uint64_t> specdata;

  const uint32_t i = 5;    // Compute stage
  RDCASSERT(pipeInfo.shaders[i].module != ResourceId());

  size_t specEntries = pipeInfo.shaders[i].specialization.size();

  specdata.resize(specEntries);
  specMapEntries.resize(specEntries);
  VkSpecializationMapEntry *entry = &specMapEntries[0];

  stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stage.stage = (VkShaderStageFlagBits)(1 << i);
  stage.module = rm->GetCurrentHandle<VkShaderModule>(pipeInfo.shaders[i].module);
  stage.pName = pipeInfo.shaders[i].entryPoint.c_str();
  stage.pNext = NULL;
  stage.pSpecializationInfo = NULL;
  stage.flags = VK_SHADER_STAGE_COMPUTE_BIT;

  uint32_t dataOffset = 0;

  if(!pipeInfo.shaders[i].specialization.empty())
  {
    stage.pSpecializationInfo = &specInfo;
    specInfo.pMapEntries = entry;
    specInfo.mapEntryCount = (uint32_t)pipeInfo.shaders[i].specialization.size();

    for(size_t s = 0; s < pipeInfo.shaders[i].specialization.size(); s++)
    {
      entry[s].constantID = pipeInfo.shaders[i].specialization[s].specID;
      entry[s].size = pipeInfo.shaders[i].specialization[s].dataSize;
      entry[s].offset = dataOffset;

      specdata[dataOffset] = pipeInfo.shaders[i].specialization[s].value;

      dataOffset++;
    }

    specInfo.dataSize = specdata.size();
    specInfo.pData = specdata.data();
  }

  VkComputePipelineCreateInfo ret = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      NULL,
      pipeInfo.flags,
      stage,
      rm->GetCurrentHandle<VkPipelineLayout>(pipeInfo.layout),
      VK_NULL_HANDLE,    // base pipeline handle
      0,                 // base pipeline index
  };

  // never create derivatives
  ret.flags &= ~VK_PIPELINE_CREATE_DERIVATIVE_BIT;

  pipeCreateInfo = ret;
}
