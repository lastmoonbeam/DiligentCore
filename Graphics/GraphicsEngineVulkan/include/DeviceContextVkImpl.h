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
/// Declaration of Diligent::DeviceContextVkImpl class
#include <unordered_map>

#include "DeviceContextVk.h"
#include "DeviceContextBase.h"
#include "DeviceContextNextGenBase.h"
#include "VulkanUtilities/VulkanCommandBufferPool.h"
#include "VulkanUtilities/VulkanCommandBuffer.h"
#include "VulkanUploadHeap.h"
#include "VulkanDynamicHeap.h"
#include "ResourceReleaseQueue.h"
#include "DescriptorPoolManager.h"
#include "PipelineLayout.h"
#include "GenerateMipsVkHelper.h"
#include "BufferVKImpl.h"
#include "TextureViewVKImpl.h"
#include "PipelineStateVKImpl.h"
#include "HashUtils.h"

namespace Diligent
{

/// Implementation of the Diligent::IDeviceContext interface
class DeviceContextVkImpl final : public DeviceContextNextGenBase< DeviceContextBase<IDeviceContextVk, BufferVkImpl, TextureViewVkImpl, PipelineStateVkImpl> >
{
public:
    using TDeviceContextBase = DeviceContextNextGenBase< DeviceContextBase<IDeviceContextVk, BufferVkImpl, TextureViewVkImpl, PipelineStateVkImpl> >;

    DeviceContextVkImpl(IReferenceCounters*                   pRefCounters,
                        class RenderDeviceVkImpl*             pDevice,
                        bool                                  bIsDeferred,
                        const EngineVkAttribs&                Attribs,
                        Uint32                                ContextId,
                        Uint32                                CommandQueueId,
                        std::shared_ptr<GenerateMipsVkHelper> GenerateMipsHelper);
    ~DeviceContextVkImpl();
    
    virtual void QueryInterface( const Diligent::INTERFACE_ID& IID, IObject** ppInterface )override final;

    virtual void SetPipelineState(IPipelineState* pPipelineState)override final;

    virtual void TransitionShaderResources(IPipelineState* pPipelineState, IShaderResourceBinding* pShaderResourceBinding)override final;

    virtual void CommitShaderResources(IShaderResourceBinding* pShaderResourceBinding, Uint32 Flags)override final;

    virtual void SetStencilRef(Uint32 StencilRef)override final;

    virtual void SetBlendFactors(const float* pBlendFactors = nullptr)override final;

    virtual void SetVertexBuffers( Uint32 StartSlot, Uint32 NumBuffersSet, IBuffer **ppBuffers, Uint32* pOffsets, Uint32 Flags )override final;
    
    virtual void InvalidateState()override final;

    virtual void SetIndexBuffer( IBuffer* pIndexBuffer, Uint32 ByteOffset )override final;

    virtual void SetViewports( Uint32 NumViewports, const Viewport* pViewports, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetScissorRects( Uint32 NumRects, const Rect* pRects, Uint32 RTWidth, Uint32 RTHeight )override final;

    virtual void SetRenderTargets( Uint32 NumRenderTargets, ITextureView* ppRenderTargets[], ITextureView* pDepthStencil )override final;

    virtual void Draw( DrawAttribs &DrawAttribs )override final;

    virtual void DispatchCompute( const DispatchComputeAttribs &DispatchAttrs )override final;

    virtual void ClearDepthStencil( ITextureView* pView, Uint32 ClearFlags, float fDepth, Uint8 Stencil)override final;

    virtual void ClearRenderTarget( ITextureView* pView, const float *RGBA )override final;

    virtual void Flush()override final;
    
    virtual void FinishCommandList(class ICommandList **ppCommandList)override final;

    virtual void ExecuteCommandList(class ICommandList* pCommandList)override final;

    virtual void SignalFence(IFence* pFence, Uint64 Value)override final;

    void TransitionImageLayout(class TextureVkImpl &TextureVk, VkImageLayout NewLayout);
    void TransitionImageLayout(class TextureVkImpl &TextureVk, VkImageLayout OldLayout, VkImageLayout NewLayout, const VkImageSubresourceRange& SubresRange);
    virtual void TransitionImageLayout(ITexture* pTexture, VkImageLayout NewLayout)override final;

    void BufferMemoryBarrier(class BufferVkImpl &BufferVk, VkAccessFlags NewAccessFlags);
    virtual void BufferMemoryBarrier(IBuffer* pBuffer, VkAccessFlags NewAccessFlags)override final;

    void AddWaitSemaphore(VkSemaphore Semaphore, VkPipelineStageFlags WaitDstStageMask)
    {
        m_WaitSemaphores.push_back(Semaphore);
        m_WaitDstStageMasks.push_back(WaitDstStageMask);
    }
    void AddSignalSemaphore(VkSemaphore Semaphore)
    {
        m_SignalSemaphores.push_back(Semaphore);
    }

    void UpdateBufferRegion(class BufferVkImpl* pBuffVk, Uint64 DstOffset, Uint64 NumBytes, VkBuffer vkSrcBuffer, Uint64 SrcOffset);
    void UpdateBufferRegion(class BufferVkImpl* pBuffVk, const void* pData, Uint64 DstOffset, Uint64 NumBytes);

    void CopyBufferRegion(class BufferVkImpl* pSrcBuffVk, class BufferVkImpl* pDstBuffVk, Uint64 SrcOffset, Uint64 DstOffset, Uint64 NumBytes);
    void CopyTextureRegion(class TextureVkImpl* pSrcTexture, class TextureVkImpl* pDstTexture, const VkImageCopy &CopyRegion);

    void UpdateTextureRegion(const void*          pSrcData,
                             Uint32               SrcStride,
                             Uint32               SrcDepthStride,
                             class TextureVkImpl& TextureVk,
                             Uint32               MipLevel,
                             Uint32               Slice,
                             const Box&           DstBox);

    void MapTexture(TextureVkImpl&            TextureVk,
                    Uint32                    MipLevel,
                    Uint32                    ArraySlice,
                    MAP_TYPE                  MapType,
                    Uint32                    MapFlags,
                    const Box&                MapRegion,
                    MappedTextureSubresource& MappedData);

    void UnmapTexture(TextureVkImpl& TextureVk,
                      Uint32         MipLevel,
                      Uint32         ArraySlice);

    void GenerateMips(class TextureViewVkImpl& TexView)
    {
        m_GenerateMipsHelper->GenerateMips(TexView, *this, *m_GenerateMipsSRB);
    }

    Uint32 GetContextId()const{return m_ContextId;}

    size_t GetNumCommandsInCtx()const { return m_State.NumCommands; }

    VulkanUtilities::VulkanCommandBuffer& GetCommandBuffer()
    {
        EnsureVkCmdBuffer();
        return m_CommandBuffer;
    }

    virtual void FinishFrame()override final;

    VkDescriptorSet AllocateDynamicDescriptorSet(VkDescriptorSetLayout SetLayout)
    {
        // Descriptor pools are externally synchronized, meaning that the application must not allocate 
        // and/or free descriptor sets from the same pool in multiple threads simultaneously (13.2.3)
        return m_DynamicDescrSetAllocator.Allocate(SetLayout, "");
    }

    VulkanDynamicAllocation AllocateDynamicSpace(Uint32 SizeInBytes, Uint32 Alignment);

    void ResetRenderTargets();
    Int64 GetContextFrameNumber()const{return m_ContextFrameNumber;}

private:
    void CommitRenderPassAndFramebuffer();
    void CommitVkVertexBuffers();
    void TransitionVkVertexBuffers();
    void CommitViewports();
    void CommitScissorRects();
    
    inline void EnsureVkCmdBuffer();
    inline void DisposeVkCmdBuffer(Uint32 CmdQueue, VkCommandBuffer vkCmdBuff, Uint64 FenceValue);
    inline void DisposeCurrentCmdBuffer(Uint32 CmdQueue, Uint64 FenceValue);

    struct BufferToTextureCopyInfo
    {
        Uint32  RowSize        = 0;
        Uint32  Stride         = 0;
        Uint32  StrideInTexels = 0;
        Uint32  DepthStride    = 0;
        Uint32  MemorySize     = 0;
        Uint32  RowCount       = 0;
        Box     Region;
    };
    BufferToTextureCopyInfo GetBufferToTextureCopyInfo(const TextureDesc& TexDesc,
                                                       Uint32             MipLevel,
                                                       const Box&         Region)const;

    void CopyBufferToTexture(VkBuffer         vkBuffer,
                             Uint32           BufferOffset,
                             Uint32           BufferRowStrideInTexels,
                             const Box&       Region,
                             TextureVkImpl&   TextureVk,
                             Uint32           MipLevel,
                             Uint32           ArraySlice);


    void DvpLogRenderPass_PSOMismatch();

    VulkanUtilities::VulkanCommandBuffer m_CommandBuffer;

    const Uint32 m_NumCommandsToFlush = 192;
    struct ContextState
    {
        /// Flag indicating if currently committed vertex buffers are up to date
        bool CommittedVBsUpToDate = false;

        /// Flag indicating if currently committed index buffer is up to date
        bool CommittedIBUpToDate = false;

        Uint32 NumCommands = 0;
    }m_State;


    /// Render pass that matches currently bound render targets.
    /// This render pass may or may not be currently set in the command buffer
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;

    /// Framebuffer that matches currently bound render targets.
    /// This framebuffer may or may not be currently set in the command buffer
    VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;

    FixedBlockMemoryAllocator m_CmdListAllocator;

    // Semaphores are not owned by the command context
    std::vector<VkSemaphore>           m_WaitSemaphores;
    std::vector<VkPipelineStageFlags>  m_WaitDstStageMasks;
    std::vector<VkSemaphore>           m_SignalSemaphores;

    // List of fences to signal next time the command context is flushed
    std::vector<std::pair<Uint64, RefCntAutoPtr<IFence> > > m_PendingFences;

    std::unordered_map<BufferVkImpl*, VulkanUploadAllocation> m_UploadAllocations;

    struct MappedTextureKey
    {
        TextureVkImpl* const Texture;
        Uint32         const MipLevel;
        Uint32         const ArraySlice;

        bool operator == (const MappedTextureKey& rhs)const
        {
            return Texture    == rhs.Texture  &&
                   MipLevel   == rhs.MipLevel &&
                   ArraySlice == rhs.ArraySlice;
        }
        struct Hasher
        {
            size_t operator()(const MappedTextureKey& Key)const
            {
                return ComputeHash(Key.Texture, Key.MipLevel, Key.ArraySlice);
            }
        };
    };
    struct MappedTexture
    {
        BufferToTextureCopyInfo CopyInfo;
        VulkanDynamicAllocation Allocation;
    };
    std::unordered_map<MappedTextureKey, MappedTexture, MappedTextureKey::Hasher> m_MappedTextures;


    VulkanUtilities::VulkanCommandBufferPool m_CmdPool;
    VulkanUploadHeap                         m_UploadHeap;
    VulkanDynamicHeap                        m_DynamicHeap;
    DynamicDescriptorSetAllocator            m_DynamicDescrSetAllocator;

    PipelineLayout::DescriptorSetBindInfo m_DescrSetBindInfo;
    std::shared_ptr<GenerateMipsVkHelper> m_GenerateMipsHelper;
    RefCntAutoPtr<IShaderResourceBinding> m_GenerateMipsSRB;

    // In Vulkan we can't bind null vertex buffer, so we have to create a dummy VB
    RefCntAutoPtr<IBuffer> m_DummyVB;
};

}
