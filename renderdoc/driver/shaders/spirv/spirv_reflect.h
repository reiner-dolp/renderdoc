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

#include <vector>
#include "api/replay/renderdoc_replay.h"
#include "spirv_common.h"
#include "spirv_processor.h"

enum class GraphicsAPI : uint32_t;
enum class ShaderStage : uint32_t;
enum class ShaderBuiltin : uint32_t;
struct ShaderReflection;
struct ShaderBindpointMapping;

// extra information that goes along with a ShaderReflection that has extra information for SPIR-V
// patching
struct SPIRVPatchData
{
  struct InterfaceAccess
  {
    // ID of the base variable
    rdcspv::Id ID;

    // ID of the struct parent of this variable
    rdcspv::Id structID;

    // member in the parent struct of this variable (for MemberDecorate)
    uint32_t structMemberIndex = 0;

    // the access chain of indices
    std::vector<uint32_t> accessChain;

    // this is an element of an array that's been exploded after [0].
    // i.e. this is false for non-arrays, and false for element [0] in an array, then true for
    // elements [1], [2], [3], etc..
    bool isArraySubsequentElement = false;
  };

  // matches the input/output signature array, with details of where to fetch the output from in the
  // SPIR-V.
  std::vector<InterfaceAccess> inputs;
  std::vector<InterfaceAccess> outputs;

  // the output topology for tessellation and geometry shaders
  Topology outTopo = Topology::Unknown;
};

struct SpecConstant
{
  SpecConstant() = default;
  SpecConstant(uint32_t id, uint64_t val, size_t size) : specID(id), value(val), dataSize(size) {}
  uint32_t specID = 0;
  uint64_t value = 0;
  size_t dataSize = 0;
};

namespace rdcspv
{
struct SourceFile
{
  SourceLanguage lang;
  rdcstr name;
  rdcstr contents;
};

class Reflector : public Processor
{
public:
  Reflector();
  virtual void Parse(const std::vector<uint32_t> &spirvWords);

  std::string Disassemble(const std::string &entryPoint) const;

  std::vector<std::string> EntryPoints() const;
  ShaderStage StageForEntry(const std::string &entryPoint) const;

  void MakeReflection(const GraphicsAPI sourceAPI, const ShaderStage stage,
                      const std::string &entryPoint, const std::vector<SpecConstant> &specInfo,
                      ShaderReflection &reflection, ShaderBindpointMapping &mapping,
                      SPIRVPatchData &patchData) const;

private:
  virtual void PreParse(uint32_t maxId);
  virtual void PostParse();
  virtual void RegisterOp(Iter iter);
  virtual void UnregisterOp(Iter iter);

  ShaderVariable EvaluateConstant(Id constID, const std::vector<SpecConstant> &specInfo) const;
  rdcstr StringiseConstant(rdcspv::Id id) const;

  void MakeConstantBlockVariables(const DataType &structType, uint32_t arraySize,
                                  uint32_t arrayByteStride, rdcarray<ShaderConstant> &cblock,
                                  SparseIdMap<uint16_t> &pointerTypes,
                                  const std::vector<SpecConstant> &specInfo) const;
  void MakeConstantBlockVariable(ShaderConstant &outConst, SparseIdMap<uint16_t> &pointerTypes,
                                 const DataType &type, const rdcstr &name,
                                 const Decorations &varDecorations,
                                 const std::vector<SpecConstant> &specInfo) const;
  void AddSignatureParameter(const bool isInput, const ShaderStage stage, const Id id,
                             const Id structID, uint32_t &regIndex,
                             const SPIRVPatchData::InterfaceAccess &parentPatch,
                             const rdcstr &varName, const DataType &type,
                             const Decorations &decorations, rdcarray<SigParameter> &sigarray,
                             SPIRVPatchData &patchData,
                             const std::vector<SpecConstant> &specInfo) const;

  rdcstr cmdline;
  DenseIdMap<rdcstr> strings;
  rdcarray<SourceFile> sources;

  struct MemberName
  {
    Id id;
    uint32_t member;
    rdcstr name;
  };

  rdcarray<MemberName> memberNames;
};
};

static const uint32_t SpecializationConstantBindSet = 1234567;
static const uint32_t PushConstantBindSet = 1234568;

void FillSpecConstantVariables(ResourceId shader, const rdcarray<ShaderConstant> &invars,
                               rdcarray<ShaderVariable> &outvars,
                               const std::vector<SpecConstant> &specInfo);

// common function used by any API that utilises SPIR-V
void AddXFBAnnotations(const ShaderReflection &refl, const SPIRVPatchData &patchData,
                       const char *entryName, std::vector<uint32_t> &modSpirv, uint32_t &xfbStride);
