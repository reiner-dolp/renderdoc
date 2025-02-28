/******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Baldur Karlsson
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

namespace DXBC
{
struct CountOffset
{
  int32_t count;
  int32_t offset;
};

// matches D3D11_SHADER_VERSION_TYPE from d3d11shader.h
enum class ShaderType
{
  // D3D10 Shaders
  Pixel = 0,
  Vertex = 1,
  Geometry = 2,

  // D3D11 Shaders
  Hull = 3,
  Domain = 4,
  Compute = 5,

  // D3D12 Shaders
  Library = 6,
  RayGeneration = 7,
  Intersection = 8,
  AnyHit = 9,
  ClosestHit = 10,
  Miss = 11,
  Callable = 12,
  Mesh = 13,
  Amplification = 14,

  // Max value
  Max,
};

/////////////////////////////////////////////////////////////////////////
// the below classes basically mimics the existing reflection interface.
//
// essentially all information comes from the wine project.
/////////////////////////////////////////////////////////////////////////
enum ResourceRetType
{
  RETURN_TYPE_UNKNOWN = 0,
  RETURN_TYPE_UNORM = 1,
  RETURN_TYPE_SNORM,
  RETURN_TYPE_SINT,
  RETURN_TYPE_UINT,
  RETURN_TYPE_FLOAT,
  RETURN_TYPE_MIXED,
  RETURN_TYPE_DOUBLE,
  RETURN_TYPE_CONTINUED,
  RETURN_TYPE_UNUSED,

  NUM_RETURN_TYPES,
};

enum ComponentType
{
  COMPONENT_TYPE_UNKNOWN = 0,
  COMPONENT_TYPE_UINT32,
  COMPONENT_TYPE_SINT32,
  COMPONENT_TYPE_FLOAT32,

  NUM_COMP_TYPES,
};

enum SVSemantic
{
  SVNAME_UNDEFINED = 0,
  SVNAME_POSITION,
  SVNAME_CLIP_DISTANCE,
  SVNAME_CULL_DISTANCE,
  SVNAME_RENDER_TARGET_ARRAY_INDEX,
  SVNAME_VIEWPORT_ARRAY_INDEX,
  SVNAME_VERTEX_ID,
  SVNAME_PRIMITIVE_ID,
  SVNAME_INSTANCE_ID,
  SVNAME_IS_FRONT_FACE,
  SVNAME_SAMPLE_INDEX,

  // following are non-contiguous
  SVNAME_FINAL_QUAD_EDGE_TESSFACTOR0,
  SVNAME_FINAL_QUAD_EDGE_TESSFACTOR = SVNAME_FINAL_QUAD_EDGE_TESSFACTOR0,
  SVNAME_FINAL_QUAD_EDGE_TESSFACTOR1,
  SVNAME_FINAL_QUAD_EDGE_TESSFACTOR2,
  SVNAME_FINAL_QUAD_EDGE_TESSFACTOR3,

  SVNAME_FINAL_QUAD_INSIDE_TESSFACTOR0,
  SVNAME_FINAL_QUAD_INSIDE_TESSFACTOR = SVNAME_FINAL_QUAD_INSIDE_TESSFACTOR0,
  SVNAME_FINAL_QUAD_INSIDE_TESSFACTOR1,

  SVNAME_FINAL_TRI_EDGE_TESSFACTOR0,
  SVNAME_FINAL_TRI_EDGE_TESSFACTOR = SVNAME_FINAL_TRI_EDGE_TESSFACTOR0,
  SVNAME_FINAL_TRI_EDGE_TESSFACTOR1,
  SVNAME_FINAL_TRI_EDGE_TESSFACTOR2,

  SVNAME_FINAL_TRI_INSIDE_TESSFACTOR,

  SVNAME_FINAL_LINE_DETAIL_TESSFACTOR,

  SVNAME_FINAL_LINE_DENSITY_TESSFACTOR,

  SVNAME_TARGET = 64,
  SVNAME_DEPTH,
  SVNAME_COVERAGE,
  SVNAME_DEPTH_GREATER_EQUAL,
  SVNAME_DEPTH_LESS_EQUAL,
};

enum VariableType
{
  VARTYPE_VOID = 0,
  VARTYPE_BOOL,
  VARTYPE_INT,
  VARTYPE_FLOAT,
  VARTYPE_STRING,
  VARTYPE_TEXTURE,
  VARTYPE_TEXTURE1D,
  VARTYPE_TEXTURE2D,
  VARTYPE_TEXTURE3D,
  VARTYPE_TEXTURECUBE,
  VARTYPE_SAMPLER,
  VARTYPE_SAMPLER1D,
  VARTYPE_SAMPLER2D,
  VARTYPE_SAMPLER3D,
  VARTYPE_SAMPLERCUBE,
  VARTYPE_PIXELSHADER,
  VARTYPE_VERTEXSHADER,
  VARTYPE_PIXELFRAGMENT,
  VARTYPE_VERTEXFRAGMENT,
  VARTYPE_UINT,
  VARTYPE_UINT8,
  VARTYPE_GEOMETRYSHADER,
  VARTYPE_RASTERIZER,
  VARTYPE_DEPTHSTENCIL,
  VARTYPE_BLEND,
  VARTYPE_BUFFER,
  VARTYPE_CBUFFER,
  VARTYPE_TBUFFER,
  VARTYPE_TEXTURE1DARRAY,
  VARTYPE_TEXTURE2DARRAY,
  VARTYPE_RENDERTARGETVIEW,
  VARTYPE_DEPTHSTENCILVIEW,
  VARTYPE_TEXTURE2DMS,
  VARTYPE_TEXTURE2DMSARRAY,
  VARTYPE_TEXTURECUBEARRAY,
  VARTYPE_HULLSHADER,
  VARTYPE_DOMAINSHADER,
  VARTYPE_INTERFACE_POINTER,
  VARTYPE_COMPUTESHADER,
  VARTYPE_DOUBLE,
  VARTYPE_RWTEXTURE1D,
  VARTYPE_RWTEXTURE1DARRAY,
  VARTYPE_RWTEXTURE2D,
  VARTYPE_RWTEXTURE2DARRAY,
  VARTYPE_RWTEXTURE3D,
  VARTYPE_RWBUFFER,
  VARTYPE_BYTEADDRESS_BUFFER,
  VARTYPE_RWBYTEADDRESS_BUFFER,
  VARTYPE_STRUCTURED_BUFFER,
  VARTYPE_RWSTRUCTURED_BUFFER,
  VARTYPE_APPEND_STRUCTURED_BUFFER,
  VARTYPE_CONSUME_STRUCTURED_BUFFER,
  VARTYPE_MIN8FLOAT,
  VARTYPE_MIN10FLOAT,
  VARTYPE_MIN16FLOAT,
  VARTYPE_MIN12INT,
  VARTYPE_MIN16INT,
  VARTYPE_MIN16UINT,
};

struct ShaderInputBind
{
  std::string name;

  enum InputType
  {
    TYPE_CBUFFER = 0,
    TYPE_TBUFFER,
    TYPE_TEXTURE,
    TYPE_SAMPLER,
    TYPE_UAV_RWTYPED,
    TYPE_STRUCTURED,
    TYPE_UAV_RWSTRUCTURED,
    TYPE_BYTEADDRESS,
    TYPE_UAV_RWBYTEADDRESS,
    TYPE_UAV_APPEND_STRUCTURED,
    TYPE_UAV_CONSUME_STRUCTURED,
    TYPE_UAV_RWSTRUCTURED_WITH_COUNTER,
  } type;

  constexpr bool IsCBuffer() const { return type == TYPE_CBUFFER; }
  constexpr bool IsSampler() const { return type == TYPE_SAMPLER; }
  constexpr bool IsSRV() const
  {
    return type == TYPE_TBUFFER || type == TYPE_TEXTURE || type == TYPE_STRUCTURED ||
           type == TYPE_BYTEADDRESS;
  }
  constexpr bool IsUAV() const
  {
    return type == TYPE_UAV_RWTYPED || type == TYPE_UAV_RWSTRUCTURED ||
           type == TYPE_UAV_RWBYTEADDRESS || type == TYPE_UAV_APPEND_STRUCTURED ||
           type == TYPE_UAV_CONSUME_STRUCTURED || type == TYPE_UAV_RWSTRUCTURED_WITH_COUNTER;
  }

  uint32_t space;
  uint32_t reg;
  uint32_t bindCount;

  uint32_t flags;
  DXBC::ResourceRetType retType;

  enum Dimension
  {
    DIM_UNKNOWN = 0,
    DIM_BUFFER,
    DIM_TEXTURE1D,
    DIM_TEXTURE1DARRAY,
    DIM_TEXTURE2D,
    DIM_TEXTURE2DARRAY,
    DIM_TEXTURE2DMS,
    DIM_TEXTURE2DMSARRAY,
    DIM_TEXTURE3D,
    DIM_TEXTURECUBE,
    DIM_TEXTURECUBEARRAY,
    DIM_BUFFEREX,
  } dimension;

  uint32_t numSamples;
};

struct CBufferVariable;

enum VariableClass
{
  CLASS_SCALAR = 0,
  CLASS_VECTOR,
  CLASS_MATRIX_ROWS,
  CLASS_MATRIX_COLUMNS,
  CLASS_OBJECT,
  CLASS_STRUCT,
  CLASS_INTERFACE_CLASS,
  CLASS_INTERFACE_POINTER,
};

struct CBufferVariableType
{
  struct Descriptor
  {
    VariableClass varClass;
    VariableType type;
    uint32_t rows;
    uint32_t cols;
    uint32_t elements;
    uint32_t members;
    uint32_t bytesize;
    std::string name;
  } descriptor;

  // if a struct, these are variables for each member (this can obviously nest). Not all
  // elements of the nested member descriptor are valid, as this might not be in a cbuffer,
  // but might be a loose structure
  std::vector<CBufferVariable> members;
};

struct CBufferVariable
{
  std::string name;

  struct
  {
    std::string name;
    uint32_t offset;    // offset in parent (cbuffer or nested struct)
    uint32_t flags;
    std::vector<uint8_t> defaultValue;
    uint32_t startTexture;    // first texture
    uint32_t numTextures;
    uint32_t startSampler;    // first sampler
    uint32_t numSamplers;
  } descriptor;

  // type details of this variable
  CBufferVariableType type;
};

struct CBuffer
{
  std::string name;

  uint32_t space;
  uint32_t reg;
  uint32_t bindCount;

  struct Descriptor
  {
    std::string name;

    enum Type
    {
      TYPE_CBUFFER = 0,
      TYPE_TBUFFER,
      TYPE_INTERFACE_POINTERS,
      TYPE_RESOURCE_BIND_INFO,
    } type;

    uint32_t numVars;
    uint32_t byteSize;
    uint32_t flags;
  } descriptor;

  std::vector<CBufferVariable> variables;
};

struct Reflection
{
  std::vector<ShaderInputBind> SRVs;
  std::vector<ShaderInputBind> UAVs;

  std::vector<ShaderInputBind> Samplers;

  std::vector<CBuffer> CBuffers;

  CBuffer Interfaces;

  std::map<std::string, CBufferVariableType> ResourceBinds;

  std::vector<SigParameter> InputSig;
  std::vector<SigParameter> OutputSig;
  std::vector<SigParameter> PatchConstantSig;

  uint32_t DispatchThreadsDimension[3];
};
};
