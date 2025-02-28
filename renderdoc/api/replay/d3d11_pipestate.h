/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2015-2019 Baldur Karlsson
 * Copyright (c) 2014 Crytek
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

#pragma once

#include "common_pipestate.h"
#include "shader_types.h"

namespace D3D11Pipe
{
DOCUMENT(R"(Describes a single D3D11 input layout element for one vertex input.

.. data:: TightlyPacked

  Value for :data:`byteOffset` that indicates this element is tightly packed.
)");
struct Layout
{
  DOCUMENT("");
  Layout() = default;
  Layout(const Layout &) = default;
  Layout &operator=(const Layout &) = default;

  bool operator==(const Layout &o) const
  {
    return semanticName == o.semanticName && semanticIndex == o.semanticIndex &&
           format == o.format && inputSlot == o.inputSlot && byteOffset == o.byteOffset &&
           perInstance == o.perInstance && instanceDataStepRate == o.instanceDataStepRate;
  }
  bool operator<(const Layout &o) const
  {
    if(!(semanticName == o.semanticName))
      return semanticName < o.semanticName;
    if(!(semanticIndex == o.semanticIndex))
      return semanticIndex < o.semanticIndex;
    if(!(format == o.format))
      return format < o.format;
    if(!(inputSlot == o.inputSlot))
      return inputSlot < o.inputSlot;
    if(!(byteOffset == o.byteOffset))
      return byteOffset < o.byteOffset;
    if(!(perInstance == o.perInstance))
      return perInstance < o.perInstance;
    if(!(instanceDataStepRate == o.instanceDataStepRate))
      return instanceDataStepRate < o.instanceDataStepRate;
    return false;
  }
  DOCUMENT("The semantic name for this input.");
  rdcstr semanticName;

  DOCUMENT("The semantic index for this input.");
  uint32_t semanticIndex = 0;

  DOCUMENT("The :class:`ResourceFormat` describing how the input data is interpreted.");
  ResourceFormat format;

  DOCUMENT("The vertex buffer input slot where the data is sourced from.");
  uint32_t inputSlot = 0;

  DOCUMENT(R"(The byte offset from the start of the vertex data in the vertex buffer from
:data:`inputSlot`.

If the value is :data:`TightlyPacked` then the element is packed tightly after the previous element, or 0
if this is the first element.
)");
  uint32_t byteOffset = 0;

  DOCUMENT("``True`` if the vertex data is instance-rate.");
  bool perInstance = false;

  DOCUMENT(R"(If :data:`perInstance` is ``True`` then this is how many times each instance data is
used before advancing to the next instance.

E.g. if this value is two, then two instances will be drawn with the first instance data, then two
with the next instance data.
)");
  uint32_t instanceDataStepRate = 0;

  // D3D11_APPEND_ALIGNED_ELEMENT
  static const uint32_t TightlyPacked = ~0U;
};

DOCUMENT("Describes a single D3D11 vertex buffer binding.")
struct VertexBuffer
{
  DOCUMENT("");
  VertexBuffer() = default;
  VertexBuffer(const VertexBuffer &) = default;
  VertexBuffer &operator=(const VertexBuffer &) = default;

  bool operator==(const VertexBuffer &o) const
  {
    return resourceId == o.resourceId && byteStride == o.byteStride && byteOffset == o.byteOffset;
  }
  bool operator<(const VertexBuffer &o) const
  {
    if(!(resourceId == o.resourceId))
      return resourceId < o.resourceId;
    if(!(byteStride == o.byteStride))
      return byteStride < o.byteStride;
    if(!(byteOffset == o.byteOffset))
      return byteOffset < o.byteOffset;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the buffer bound to this slot.");
  ResourceId resourceId;

  DOCUMENT("The byte offset from the start of the buffer to the beginning of the vertex data.");
  uint32_t byteOffset = 0;

  DOCUMENT("The byte stride between the start of one set of vertex data and the next.");
  uint32_t byteStride = 0;
};

DOCUMENT("Describes the D3D11 index buffer binding.")
struct IndexBuffer
{
  DOCUMENT("");
  IndexBuffer() = default;
  IndexBuffer(const IndexBuffer &) = default;
  IndexBuffer &operator=(const IndexBuffer &) = default;

  DOCUMENT("The :class:`ResourceId` of the index buffer.");
  ResourceId resourceId;

  DOCUMENT("The byte offset from the start of the buffer to the beginning of the index data.");
  uint32_t byteOffset = 0;
};

DOCUMENT("Describes the input assembler data.");
struct InputAssembly
{
  DOCUMENT("");
  InputAssembly() = default;
  InputAssembly(const InputAssembly &) = default;
  InputAssembly &operator=(const InputAssembly &) = default;

  DOCUMENT("A list of :class:`D3D11Layout` describing the input layout elements in this layout.");
  rdcarray<Layout> layouts;

  DOCUMENT("The :class:`ResourceId` of the layout object.");
  ResourceId resourceId;

  DOCUMENT("A :class:`ShaderReflection` describing the bytecode used to create the input layout.");
  ShaderReflection *bytecode = NULL;

  DOCUMENT("A list of :class:`D3D11VertexBuffer` with the vertex buffers that are bound.");
  rdcarray<VertexBuffer> vertexBuffers;

  DOCUMENT("The :class:`D3D11IndexBuffer` describing the index buffer.");
  IndexBuffer indexBuffer;
};

DOCUMENT("Describes the details of a D3D11 resource view - any one of UAV, SRV, RTV or DSV.");
struct View
{
  DOCUMENT("");
  View() = default;
  View(const View &) = default;
  View &operator=(const View &) = default;

  bool operator==(const View &o) const
  {
    return viewResourceId == o.viewResourceId && resourceResourceId == o.resourceResourceId &&
           counterResourceId == o.counterResourceId && type == o.type && viewFormat == o.viewFormat &&
           structured == o.structured && bufferStructCount == o.bufferStructCount &&
           elementByteSize == o.elementByteSize && firstElement == o.firstElement &&
           numElements == o.numElements && bufferFlags == o.bufferFlags && firstMip == o.firstMip &&
           numMips == o.numMips && numSlices == o.numSlices && firstSlice == o.firstSlice;
  }
  bool operator<(const View &o) const
  {
    if(!(viewResourceId == o.viewResourceId))
      return viewResourceId < o.viewResourceId;
    if(!(resourceResourceId == o.resourceResourceId))
      return resourceResourceId < o.resourceResourceId;
    if(!(counterResourceId == o.counterResourceId))
      return counterResourceId < o.counterResourceId;
    if(!(type == o.type))
      return type < o.type;
    if(!(viewFormat == o.viewFormat))
      return viewFormat < o.viewFormat;
    if(!(structured == o.structured))
      return structured < o.structured;
    if(!(bufferStructCount == o.bufferStructCount))
      return bufferStructCount < o.bufferStructCount;
    if(!(elementByteSize == o.elementByteSize))
      return elementByteSize < o.elementByteSize;
    if(!(firstElement == o.firstElement))
      return firstElement < o.firstElement;
    if(!(numElements == o.numElements))
      return numElements < o.numElements;
    if(!(bufferFlags == o.bufferFlags))
      return bufferFlags < o.bufferFlags;
    if(!(firstMip == o.firstMip))
      return firstMip < o.firstMip;
    if(!(numMips == o.numMips))
      return numMips < o.numMips;
    if(!(numSlices == o.numSlices))
      return numSlices < o.numSlices;
    if(!(firstSlice == o.firstSlice))
      return firstSlice < o.firstSlice;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the view itself.");
  ResourceId viewResourceId;

  DOCUMENT("The :class:`ResourceId` of the underlying resource the view refers to.");
  ResourceId resourceResourceId;

  DOCUMENT("The :class:`ResourceId` of the resource where the hidden buffer counter is stored.");
  ResourceId counterResourceId;

  DOCUMENT("The :class:`TextureType` of the view type.");
  TextureType type;

  DOCUMENT("The :class:`ResourceFormat` that the view uses.");
  ResourceFormat viewFormat;

  DOCUMENT("``True`` if this view describes a structured buffer.");
  bool structured = false;

  DOCUMENT("If the view has a hidden counter, this stores the current value of the counter.");
  uint32_t bufferStructCount = 0;

  DOCUMENT(R"(The byte size of a single element in the view. Either the byte size of
:data:`viewFormat`, or the structured buffer element size, as appropriate.
)");
  uint32_t elementByteSize = 0;

  DOCUMENT("Valid for buffers - the first element to be used in the view.");
  uint32_t firstElement = 0;
  DOCUMENT("Valid for buffers - the number of elements to be used in the view.");
  uint32_t numElements = 1;

  DOCUMENT("Valid for buffers - the flags for additional view properties.");
  D3DBufferViewFlags bufferFlags = D3DBufferViewFlags::NoFlags;

  DOCUMENT("Valid for textures - the first mip that is available through the view.");
  uint32_t firstMip = 0;
  DOCUMENT("Valid for textures - the number of mip levels in the view.");
  uint32_t numMips = 0;

  DOCUMENT("Valid for texture arrays or 3D textures - the first slice available through the view.");
  uint32_t firstSlice = 0;
  DOCUMENT("Valid for texture arrays or 3D textures - the number of slices in the view.");
  uint32_t numSlices = 1;
};

DOCUMENT("Describes a sampler state object.");
struct Sampler
{
  DOCUMENT("");
  Sampler() = default;
  Sampler(const Sampler &) = default;
  Sampler &operator=(const Sampler &) = default;

  bool operator==(const Sampler &o) const
  {
    return resourceId == o.resourceId && addressU == o.addressU && addressV == o.addressV &&
           addressW == o.addressW && borderColor[0] == o.borderColor[0] &&
           borderColor[1] == o.borderColor[1] && borderColor[2] == o.borderColor[2] &&
           borderColor[3] == o.borderColor[3] && compareFunction == o.compareFunction &&
           filter == o.filter && maxAnisotropy == o.maxAnisotropy && maxLOD == o.maxLOD &&
           minLOD == o.minLOD && mipLODBias == o.mipLODBias;
  }
  bool operator<(const Sampler &o) const
  {
    if(!(resourceId == o.resourceId))
      return resourceId < o.resourceId;
    if(!(addressU == o.addressU))
      return addressU < o.addressU;
    if(!(addressV == o.addressV))
      return addressV < o.addressV;
    if(!(addressW == o.addressW))
      return addressW < o.addressW;
    if(!(borderColor[0] == o.borderColor[0]))
      return borderColor[0] < o.borderColor[0];
    if(!(borderColor[1] == o.borderColor[1]))
      return borderColor[1] < o.borderColor[1];
    if(!(borderColor[2] == o.borderColor[2]))
      return borderColor[2] < o.borderColor[2];
    if(!(borderColor[3] == o.borderColor[3]))
      return borderColor[3] < o.borderColor[3];
    if(!(compareFunction == o.compareFunction))
      return compareFunction < o.compareFunction;
    if(!(filter == o.filter))
      return filter < o.filter;
    if(!(maxAnisotropy == o.maxAnisotropy))
      return maxAnisotropy < o.maxAnisotropy;
    if(!(maxLOD == o.maxLOD))
      return maxLOD < o.maxLOD;
    if(!(minLOD == o.minLOD))
      return minLOD < o.minLOD;
    if(!(mipLODBias == o.mipLODBias))
      return mipLODBias < o.mipLODBias;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the sampler state object.");
  ResourceId resourceId;
  DOCUMENT("The :class:`AddressMode` in the U direction.");
  AddressMode addressU = AddressMode::Wrap;
  DOCUMENT("The :class:`AddressMode` in the V direction.");
  AddressMode addressV = AddressMode::Wrap;
  DOCUMENT("The :class:`AddressMode` in the W direction.");
  AddressMode addressW = AddressMode::Wrap;
  DOCUMENT("The RGBA border color.");
  float borderColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  DOCUMENT("The :class:`CompareFunction` for comparison samplers.");
  CompareFunction compareFunction = CompareFunction::AlwaysTrue;
  DOCUMENT("The :class:`TextureFilter` describing the filtering mode.");
  TextureFilter filter;
  DOCUMENT("The maximum anisotropic filtering level to use.");
  uint32_t maxAnisotropy = 0;
  DOCUMENT("The maximum mip level that can be used.");
  float maxLOD = 0.0f;
  DOCUMENT("The minimum mip level that can be used.");
  float minLOD = 0.0f;
  DOCUMENT("A bias to apply to the calculated mip level before sampling.");
  float mipLODBias = 0.0f;

  DOCUMENT(R"(Check if the border color is used in this D3D11 sampler.

:return: ``True`` if the border color is used, ``False`` otherwise.
:rtype: ``bool``
)");
  bool UseBorder() const
  {
    return addressU == AddressMode::ClampBorder || addressV == AddressMode::ClampBorder ||
           addressW == AddressMode::ClampBorder;
  }
};

DOCUMENT("Describes a constant buffer binding.");
struct ConstantBuffer
{
  DOCUMENT("");
  ConstantBuffer() = default;
  ConstantBuffer(const ConstantBuffer &) = default;
  ConstantBuffer &operator=(const ConstantBuffer &) = default;

  bool operator==(const ConstantBuffer &o) const
  {
    return resourceId == o.resourceId && vecOffset == o.vecOffset && vecCount == o.vecCount;
  }
  bool operator<(const ConstantBuffer &o) const
  {
    if(!(resourceId == o.resourceId))
      return resourceId < o.resourceId;
    if(!(vecOffset == o.vecOffset))
      return vecOffset < o.vecOffset;
    if(!(vecCount == o.vecCount))
      return vecCount < o.vecCount;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the buffer.");
  ResourceId resourceId;

  DOCUMENT(R"(The offset of the buffer binding, in units of ``float4`` (16 bytes).

If the capture isn't using the D3D11.1 binding methods, this offset will be 0.
)");
  uint32_t vecOffset = 0;

  DOCUMENT(R"(The size of the buffer binding, in units of ``float4`` (16 bytes).

If the capture isn't using the D3D11.1 binding methods, this offset will be 4096 (64 kiB).
)");
  uint32_t vecCount = 0;
};

DOCUMENT("Describes a D3D11 shader stage.");
struct Shader
{
  DOCUMENT("");
  Shader() = default;
  Shader(const Shader &) = default;
  Shader &operator=(const Shader &) = default;

  DOCUMENT("The :class:`ResourceId` of the shader itself.");
  ResourceId resourceId;

  DOCUMENT("A :class:`ShaderReflection` describing the reflection data for this shader.");
  ShaderReflection *reflection = NULL;
  DOCUMENT(R"(A :class:`ShaderBindpointMapping` to match :data:`reflection` with the bindpoint
mapping data.
)");
  ShaderBindpointMapping bindpointMapping;

  DOCUMENT("A :class:`ShaderStage` identifying which stage this shader is bound to.");
  ShaderStage stage = ShaderStage::Vertex;

  DOCUMENT("A list of :class:`D3D11View` with the bound SRVs.");
  rdcarray<View> srvs;

  DOCUMENT("A list of :class:`D3D11View` with the bound UAVs - only valid for the compute stage.");
  rdcarray<View> uavs;

  DOCUMENT("A list of :class:`D3D11Sampler` with the bound samplers.");
  rdcarray<Sampler> samplers;

  DOCUMENT("A list of :class:`D3D11ConstantBuffer` with the bound constant buffers.");
  rdcarray<ConstantBuffer> constantBuffers;

  DOCUMENT("A list of ``str`` with the bound class instance names.");
  rdcarray<rdcstr> classInstances;
};

DOCUMENT("Describes a binding on the D3D11 stream-out stage.");
struct StreamOutBind
{
  DOCUMENT("");
  StreamOutBind() = default;
  StreamOutBind(const StreamOutBind &) = default;
  StreamOutBind &operator=(const StreamOutBind &) = default;

  bool operator==(const StreamOutBind &o) const
  {
    return resourceId == o.resourceId && byteOffset == o.byteOffset;
  }
  bool operator<(const StreamOutBind &o) const
  {
    if(!(resourceId == o.resourceId))
      return resourceId < o.resourceId;
    if(!(byteOffset == o.byteOffset))
      return byteOffset < o.byteOffset;
    return false;
  }
  DOCUMENT("The :class:`ResourceId` of the buffer.");
  ResourceId resourceId;

  DOCUMENT("The byte offset of the stream-output binding.");
  uint32_t byteOffset = 0;
};

DOCUMENT("Describes the stream-out stage bindings.");
struct StreamOut
{
  DOCUMENT("");
  StreamOut() = default;
  StreamOut(const StreamOut &) = default;
  StreamOut &operator=(const StreamOut &) = default;

  DOCUMENT("A list of ``D3D11StreamOutBind`` with the bound buffers.");
  rdcarray<StreamOutBind> outputs;
};

DOCUMENT("Describes a rasterizer state object.");
struct RasterizerState
{
  DOCUMENT("");
  RasterizerState() = default;
  RasterizerState(const RasterizerState &) = default;
  RasterizerState &operator=(const RasterizerState &) = default;

  DOCUMENT("The :class:`ResourceId` of the rasterizer state object.");
  ResourceId resourceId;
  DOCUMENT("The polygon :class:`FillMode`.");
  FillMode fillMode = FillMode::Solid;
  DOCUMENT("The polygon :class:`CullMode`.");
  CullMode cullMode = CullMode::NoCull;
  DOCUMENT(R"(``True`` if counter-clockwise polygons are front-facing.
``False`` if clockwise polygons are front-facing.
)");
  bool frontCCW = false;
  DOCUMENT("The fixed depth bias value to apply to z-values.");
  int32_t depthBias = 0;
  DOCUMENT(R"(The clamp value for calculated depth bias from :data:`depthBias` and
:data:`slopeScaledDepthBias`
)");
  float depthBiasClamp = 0.0f;
  DOCUMENT("The slope-scaled depth bias value to apply to z-values.");
  float slopeScaledDepthBias = 0.0f;
  DOCUMENT("``True`` if pixels outside of the near and far depth planes should be clipped.");
  bool depthClip = false;
  DOCUMENT("``True`` if the scissor test should be applied.");
  bool scissorEnable = false;
  DOCUMENT("``True`` if the quadrilateral MSAA algorithm should be used on MSAA targets.");
  bool multisampleEnable = false;
  DOCUMENT(
      "``True`` if lines should be anti-aliased. Ignored if :data:`multisampleEnable` is "
      "``False``.");
  bool antialiasedLines = false;
  DOCUMENT(R"(A sample count to force rasterization to when UAV rendering or rasterizing, or 0 to
not force any sample count.
)");
  uint32_t forcedSampleCount = 0;
  DOCUMENT("The current :class:`ConservativeRaster` mode.");
  ConservativeRaster conservativeRasterization = ConservativeRaster::Disabled;
};

DOCUMENT("Describes the rasterization state of the D3D11 pipeline.");
struct Rasterizer
{
  DOCUMENT("");
  Rasterizer() = default;
  Rasterizer(const Rasterizer &) = default;
  Rasterizer &operator=(const Rasterizer &) = default;

  DOCUMENT("A list of :class:`Viewport` with the bound viewports.");
  rdcarray<Viewport> viewports;

  DOCUMENT("A list of :class:`Scissor` with the bound scissor regions.");
  rdcarray<Scissor> scissors;

  DOCUMENT("A :class:`D3D11RasterizerState` with the details of the rasterization state.");
  RasterizerState state;
};

DOCUMENT("Describes a depth-stencil state object.");
struct DepthStencilState
{
  DOCUMENT("");
  DepthStencilState() = default;
  DepthStencilState(const DepthStencilState &) = default;
  DepthStencilState &operator=(const DepthStencilState &) = default;

  DOCUMENT("The :class:`ResourceId` of the depth-stencil state object.");
  ResourceId resourceId;
  DOCUMENT("``True`` if depth testing should be performed.");
  bool depthEnable = false;
  DOCUMENT("The :class:`CompareFunction` to use for testing depth values.");
  CompareFunction depthFunction = CompareFunction::AlwaysTrue;
  DOCUMENT("``True`` if depth values should be written to the depth target.");
  bool depthWrites = false;
  DOCUMENT("``True`` if stencil operations should be performed.");
  bool stencilEnable = false;

  DOCUMENT("A :class:`StencilFace` describing what happens for front-facing polygons.");
  StencilFace frontFace;
  DOCUMENT("A :class:`StencilFace` describing what happens for back-facing polygons.");
  StencilFace backFace;
};

DOCUMENT("Describes a blend state object.");
struct BlendState
{
  DOCUMENT("");
  BlendState() = default;
  BlendState(const BlendState &) = default;
  BlendState &operator=(const BlendState &) = default;

  DOCUMENT("The :class:`ResourceId` of the blend state object.");
  ResourceId resourceId;

  DOCUMENT("``True`` if alpha-to-coverage should be used when blending to an MSAA target.");
  bool alphaToCoverage = false;
  DOCUMENT(R"(``True`` if independent blending for each target should be used.

``False`` if the first blend should be applied to all targets.
)");
  bool independentBlend = false;

  DOCUMENT("A list of :class:`ColorBlend` describing the blend operations for each target.");
  rdcarray<ColorBlend> blends;

  DOCUMENT("The constant blend factor to use in blend equations.");
  float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  DOCUMENT("The mask determining which samples are written to.");
  uint32_t sampleMask = ~0U;
};

DOCUMENT("Describes the current state of the output-merger stage of the D3D11 pipeline.");
struct OutputMerger
{
  DOCUMENT("");
  OutputMerger() = default;
  OutputMerger(const OutputMerger &) = default;
  OutputMerger &operator=(const OutputMerger &) = default;

  DOCUMENT("A :class:`D3D11DepthStencilState` with the details of the depth-stencil state.");
  DepthStencilState depthStencilState;
  DOCUMENT("A :class:`D3D11BlendState` with the details of the blend state.");
  BlendState blendState;

  DOCUMENT("A list of :class:`D3D11View` describing the bound render targets.");
  rdcarray<View> renderTargets;

  DOCUMENT("Which slot in the output targets is the first UAV.");
  uint32_t uavStartSlot = 0;
  DOCUMENT("A list of :class:`D3D11View` describing the bound UAVs.");
  rdcarray<View> uavs;

  DOCUMENT("A :class:`D3D11View` with details of the bound depth-stencil target.");
  View depthTarget;
  DOCUMENT("``True`` if depth access to the depth-stencil target is read-only.");
  bool depthReadOnly = false;
  DOCUMENT("``True`` if stencil access to the depth-stencil target is read-only.");
  bool stencilReadOnly = false;
};

DOCUMENT("Describes the current state of predicated rendering.");
struct Predication
{
  DOCUMENT("");
  Predication() = default;
  Predication(const Predication &) = default;
  Predication &operator=(const Predication &) = default;

  DOCUMENT("The :class:`ResourceId` of the active predicate.");
  ResourceId resourceId;

  DOCUMENT("The value to go along with the predicate.");
  bool value = false;

  DOCUMENT("``True`` if the current predicate would render.");
  bool isPassing = false;
};

DOCUMENT("The full current D3D11 pipeline state.");
struct State
{
#if !defined(RENDERDOC_EXPORTS)
  // disallow creation/copy of this object externally
  State() = delete;
  State(const State &) = delete;
#endif

  DOCUMENT("A :class:`D3D11InputAssembly` describing the input assembly pipeline stage.");
  InputAssembly inputAssembly;

  DOCUMENT("A :class:`D3D11Shader` describing the vertex shader stage.");
  Shader vertexShader;
  DOCUMENT("A :class:`D3D11Shader` describing the hull shader stage.");
  Shader hullShader;
  DOCUMENT("A :class:`D3D11Shader` describing the domain shader stage.");
  Shader domainShader;
  DOCUMENT("A :class:`D3D11Shader` describing the geometry shader stage.");
  Shader geometryShader;
  DOCUMENT("A :class:`D3D11Shader` describing the pixel shader stage.");
  Shader pixelShader;
  DOCUMENT("A :class:`D3D11Shader` describing the compute shader stage.");
  Shader computeShader;

  DOCUMENT("A :class:`D3D11StreamOut` describing the stream-out pipeline stage.");
  StreamOut streamOut;

  DOCUMENT("A :class:`D3D11Rasterizer` describing the rasterizer pipeline stage.");
  Rasterizer rasterizer;

  DOCUMENT("A :class:`D3D11OutputMerger` describing the output merger pipeline stage.");
  OutputMerger outputMerger;

  DOCUMENT("A :class:`D3D11Predication` describing the predicated rendering state.");
  Predication predication;
};

};    // namespace D3D11Pipe

DECLARE_REFLECTION_STRUCT(D3D11Pipe::Layout);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::VertexBuffer);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::IndexBuffer);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::InputAssembly);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::View);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::Sampler);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::ConstantBuffer);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::Shader);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::StreamOutBind);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::StreamOut);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::RasterizerState);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::Rasterizer);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::DepthStencilState);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::BlendState);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::OutputMerger);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::Predication);
DECLARE_REFLECTION_STRUCT(D3D11Pipe::State);
