/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

/// \file
/// Declaration of Diligent::DeviceContextD3D12Impl class

#include <unordered_map>

#include "DeviceContextD3D12.h"
#include "DeviceContextBase.h"
#include "DeviceContextNextGenBase.h"
#include "GenerateMips.h"
#include "BufferD3D12Impl.h"
#include "TextureViewD3D12Impl.h"
#include "PipelineStateD3D12Impl.h"
#include "D3D12DynamicHeap.h"

namespace Diligent
{

/// Implementation of the Diligent::IDeviceContext interface
class DeviceContextD3D12Impl final : public DeviceContextNextGenBase< DeviceContextBase<IDeviceContextD3D12, BufferD3D12Impl, TextureViewD3D12Impl, PipelineStateD3D12Impl> >
{
public:
    using TDeviceContextBase = DeviceContextNextGenBase< DeviceContextBase<IDeviceContextD3D12, BufferD3D12Impl, TextureViewD3D12Impl, PipelineStateD3D12Impl> >;

    DeviceContextD3D12Impl(IReferenceCounters*          pRefCounters, 
                           class RenderDeviceD3D12Impl* pDevice, 
                           bool                         bIsDeferred, 
                           const EngineD3D12Attribs&    Attribs, 
                           Uint32                       ContextId,
                           Uint32                       CommandQueueId);
    ~DeviceContextD3D12Impl();
    
    virtual void QueryInterface( const Diligent::INTERFACE_ID &IID, IObject **ppInterface )override final;

    virtual void SetPipelineState(IPipelineState *pPipelineState)override final;

    virtual void TransitionShaderResources(IPipelineState *pPipelineState, IShaderResourceBinding *pShaderResourceBinding)override final;

    virtual void CommitShaderResources(IShaderResourceBinding *pShaderResourceBinding, Uint32 Flags)override final;

    virtual void SetStencilRef(Uint32 StencilRef)override final;

    virtual void SetBlendFactors(const float* pBlendFactors = nullptr)override final;

    virtual void SetVertexBuffers( Uint32 StartSlot, Uint32 NumBuffersSet, IBuffer **ppBuffers, Uint32 *pOffsets, Uint32 Flags )override final;
    
    virtual void InvalidateState()override final;

    virtual void SetIndexBuffer( IBuffer *pIndexBuffer, Uint32 ByteOffset )override final;

    virtual void SetViewports( Uint32 NumViewports, const Viewport *pViewports, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetScissorRects( Uint32 NumRects, const Rect *pRects, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetRenderTargets( Uint32 NumRenderTargets, ITextureView *ppRenderTargets[], ITextureView *pDepthStencil )override final;

    virtual void Draw( DrawAttribs &DrawAttribs )override final;

    virtual void DispatchCompute( const DispatchComputeAttribs &DispatchAttrs )override final;

    virtual void ClearDepthStencil( ITextureView *pView, Uint32 ClearFlags, float fDepth, Uint8 Stencil)override final;

    virtual void ClearRenderTarget( ITextureView *pView, const float *RGBA )override final;

    virtual void Flush()override final;

    virtual void FinishFrame()override final;
    
    virtual void FinishCommandList(class ICommandList **ppCommandList)override final;

    virtual void ExecuteCommandList(class ICommandList *pCommandList)override final;

    virtual void SignalFence(IFence* pFence, Uint64 Value)override final;

    virtual void TransitionTextureState(ITexture *pTexture, D3D12_RESOURCE_STATES State)override final;

    virtual void TransitionBufferState(IBuffer *pBuffer, D3D12_RESOURCE_STATES State)override final;

    ///// Clears the state caches. This function is called once per frame
    ///// (before present) to release all outstanding objects
    ///// that are only kept alive by references in the cache
    //void ClearShaderStateCache();

    ///// Number of different shader types (Vertex, Pixel, Geometry, Domain, Hull, Compute)
    //static constexpr int NumShaderTypes = 6;

    void UpdateBufferRegion(class BufferD3D12Impl *pBuffD3D12, D3D12DynamicAllocation& Allocation, Uint64 DstOffset, Uint64 NumBytes);
    void UpdateBufferRegion(class BufferD3D12Impl *pBuffD3D12, const void *pData, Uint64 DstOffset, Uint64 NumBytes);
    void CopyBufferRegion(class BufferD3D12Impl *pSrcBuffD3D12, class BufferD3D12Impl *pDstBuffD3D12, Uint64 SrcOffset, Uint64 DstOffset, Uint64 NumBytes);
    void CopyTextureRegion(class TextureD3D12Impl *pSrcTexture, Uint32 SrcSubResIndex, const D3D12_BOX* pD3D12SrcBox,
                           class TextureD3D12Impl *pDstTexture, Uint32 DstSubResIndex, Uint32 DstX, Uint32 DstY, Uint32 DstZ);
    void CopyTextureRegion(IBuffer*                 pSrcBuffer,
                           Uint32                   SrcOffset,
                           Uint32                   SrcStride,
                           Uint32                   SrcDepthStride,
                           class TextureD3D12Impl&  TextureD3D12,
                           Uint32 DstSubResIndex,
                           const Box& DstBox);
    void CopyTextureRegion(ID3D12Resource*         pd3d12Buffer,
                           Uint32                  SrcOffset,
                           Uint32                  SrcStride,
                           Uint32                  SrcDepthStride,
                           Uint32                  BufferSize,
                           class TextureD3D12Impl& TextureD3D12,
                           Uint32                  DstSubResIndex,
                           const Box&              DstBox);

    void UpdateTextureRegion(const void*             pSrcData,
                             Uint32                  SrcStride,
                             Uint32                  SrcDepthStride,
                             class TextureD3D12Impl& TextureD3D12,
                             Uint32                  DstSubResIndex,
                             const Box&              DstBox);

    void MapTexture( class TextureD3D12Impl&   TextureD3D12,
                     Uint32                    MipLevel,
                     Uint32                    ArraySlice,
                     MAP_TYPE                  MapType,
                     Uint32                    MapFlags,
                     const Box&                MapRegion,
                     MappedTextureSubresource& MappedData );

    void UnmapTexture( class TextureD3D12Impl&   TextureD3D12,
                       Uint32                    MipLevel,
                       Uint32                    ArraySlice);

    void GenerateMips(class TextureViewD3D12Impl *pTexView);

    D3D12DynamicAllocation AllocateDynamicSpace(size_t NumBytes, size_t Alignment);
    
    Uint32 GetContextId()const{return m_ContextId;}

    size_t GetNumCommandsInCtx()const { return m_State.NumCommands; }

    Int64 GetCurrentFrameNumber()const {return m_ContextFrameNumber; }

private:
    void CommitD3D12IndexBuffer(VALUE_TYPE IndexType);
    void CommitD3D12VertexBuffers(class GraphicsContext &GraphCtx);
    void TransitionD3D12VertexBuffers(class GraphicsContext &GraphCtx);
    void CommitRenderTargets();
    void CommitViewports();
    void CommitScissorRects(class GraphicsContext &GraphCtx, bool ScissorEnable);
    void Flush(bool RequestNewCmdCtx);
    void RequestCommandContext(RenderDeviceD3D12Impl* pDeviceD3D12Impl);

    struct TextureUploadSpace
    {
        D3D12DynamicAllocation Allocation;
        Uint32                 AlignedOffset = 0;
        Uint32                 Stride        = 0;
        Uint32                 DepthStride   = 0;
        Uint32                 RowSize       = 0;
        Uint32                 RowCount      = 0;
        Box                    Region;
    };
    TextureUploadSpace AllocateTextureUploadSpace(TEXTURE_FORMAT     TexFmt,
                                                  const Box&         Region);


    friend class SwapChainD3D12Impl;
    inline class CommandContext& GetCmdContext()
    {
        // Make sure that the number of commands in the context is at least one,
        // so that the context cannot be disposed by Flush()
        m_State.NumCommands = m_State.NumCommands != 0 ? m_State.NumCommands : 1;
        return *m_CurrCmdCtx;
    }
    std::unique_ptr<CommandContext, STDDeleterRawMem<CommandContext> > m_CurrCmdCtx;

    struct State
    {
        size_t                  NumCommands = 0;
        
        CComPtr<ID3D12Resource> CommittedD3D12IndexBuffer;
        VALUE_TYPE              CommittedIBFormat                  = VT_UNDEFINED;
        Uint32                  CommittedD3D12IndexDataStartOffset = 0;

        // Flag indicating if currently committed D3D12 vertex buffers are up to date
        bool bCommittedD3D12VBsUpToDate = false;

        // Fl indicating if currently committed D3D11 index buffer is up to date
        bool bCommittedD3D12IBUpToDate  = false;

        class ShaderResourceCacheD3D12* pCommittedResourceCache = nullptr;
    }m_State;

    CComPtr<ID3D12CommandSignature> m_pDrawIndirectSignature;
    CComPtr<ID3D12CommandSignature> m_pDrawIndexedIndirectSignature;
    CComPtr<ID3D12CommandSignature> m_pDispatchIndirectSignature;
    
    GenerateMipsHelper m_MipsGenerator;

    D3D12DynamicHeap m_DynamicHeap;
    
    // Every context must use its own allocator that maintains individual list of retired descriptor heaps to 
    // avoid interference with other command contexts
    // The allocations in heaps are discarded at the end of the frame.
    DynamicSuballocationsManager m_DynamicGPUDescriptorAllocator[2];

    FixedBlockMemoryAllocator m_CmdListAllocator;

    std::vector<std::pair<Uint64, RefCntAutoPtr<IFence> > > m_PendingFences;

    struct MappedTextureKey
    {
        TextureD3D12Impl* const Texture;
        UINT              const Subresource;

        bool operator == (const MappedTextureKey& rhs)const
        {
            return Texture      == rhs.Texture &&
                   Subresource  == rhs.Subresource;
        }
        struct Hasher
        {
            size_t operator()(const MappedTextureKey& Key)const
            {
                return ComputeHash(Key.Texture, Key.Subresource);
            }
        };
    };
    std::unordered_map<MappedTextureKey, TextureUploadSpace, MappedTextureKey::Hasher> m_MappedTextures;
};

}
