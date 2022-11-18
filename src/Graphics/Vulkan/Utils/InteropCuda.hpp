/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2021, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SGL_INTEROPCUDA_HPP
#define SGL_INTEROPCUDA_HPP

#include "../Buffers/Buffer.hpp"
#include "../Image/Image.hpp"
#include "SyncObjects.hpp"
#include "Device.hpp"

#if __has_include(<cuda.h>)
#include <cuda.h>
#else
#include "DynlinkCuda.hpp"
#endif

#ifdef _WIN32
typedef void* HANDLE;
#endif

namespace sgl { namespace vk {

/*
 * Utility functions for Vulkan-CUDA driver API interoperability.
 */
struct CudaDeviceApiFunctionTable {
    CUresult ( *cuInit )( unsigned int Flags );
    CUresult ( *cuGetErrorString )( CUresult error, const char **pStr );

    CUresult ( *cuDeviceGet )(CUdevice *device, int ordinal);
    CUresult ( *cuDeviceGetCount )(int *count);
    CUresult ( *cuDeviceGetUuid )(CUuuid *uuid, CUdevice dev);
    CUresult ( *cuDeviceGetAttribute )(int *pi, CUdevice_attribute attrib, CUdevice dev);

    CUresult ( *cuCtxCreate )( CUcontext *pctx, unsigned int flags, CUdevice dev );
    CUresult ( *cuCtxDestroy )( CUcontext ctx );

    CUresult ( *cuStreamCreate )( CUstream *phStream, unsigned int Flags );
    CUresult ( *cuStreamDestroy )( CUstream hStream );
    CUresult ( *cuStreamSynchronize )( CUstream hStream );

    CUresult ( *cuMemAlloc )( CUdeviceptr *dptr, size_t bytesize );
    CUresult ( *cuMemFree )( CUdeviceptr dptr );
    CUresult ( *cuMemcpyDtoH )( void *dstHost, CUdeviceptr srcDevice, size_t ByteCount );
    CUresult ( *cuMemcpyHtoD )( CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount );
    CUresult ( *cuMemAllocAsync )( CUdeviceptr *dptr, size_t bytesize, CUstream hStream );
    CUresult ( *cuMemFreeAsync )( CUdeviceptr dptr, CUstream hStream );
    CUresult ( *cuMemsetD8Async )( CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream );
    CUresult ( *cuMemsetD16Async )( CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream );
    CUresult ( *cuMemsetD32Async )( CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream );
    CUresult ( *cuMemcpyAsync )( CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream );
    CUresult ( *cuMemcpyDtoHAsync )( void *dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream );
    CUresult ( *cuMemcpyHtoDAsync )( CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount, CUstream hStream );
    CUresult ( *cuMemcpy2DAsync )( const CUDA_MEMCPY2D* pCopy, CUstream hStream );
    CUresult ( *cuMemcpy3DAsync )( const CUDA_MEMCPY3D* pCopy, CUstream hStream );

    CUresult ( *cuMipmappedArrayDestroy )(CUmipmappedArray hMipmappedArray);
    CUresult ( *cuMipmappedArrayGetLevel )( CUarray* pLevelArray, CUmipmappedArray hMipmappedArray, unsigned int level );

    CUresult ( *cuTexObjectCreate )( CUtexObject *pTexObject, const CUDA_RESOURCE_DESC *pResDesc, const CUDA_TEXTURE_DESC *pTexDesc, const CUDA_RESOURCE_VIEW_DESC *pResViewDesc );
    CUresult ( *cuTexObjectDestroy )( CUtexObject texObject );
    CUresult ( *cuSurfObjectCreate )( CUsurfObject *pSurfObject, const CUDA_RESOURCE_DESC *pResDesc );
    CUresult ( *cuSurfObjectDestroy )( CUsurfObject surfObject );

    CUresult ( *cuImportExternalMemory )( CUexternalMemory *extMem_out, const CUDA_EXTERNAL_MEMORY_HANDLE_DESC *memHandleDesc );
    CUresult ( *cuExternalMemoryGetMappedBuffer )( CUdeviceptr *devPtr, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_BUFFER_DESC *bufferDesc );
    CUresult ( *cuExternalMemoryGetMappedMipmappedArray )( CUmipmappedArray *mipmap, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC *mipmapDesc );
    CUresult ( *cuDestroyExternalMemory )( CUexternalMemory extMem );

    CUresult ( *cuImportExternalSemaphore )( CUexternalSemaphore *extSem_out, const CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC *semHandleDesc );
    CUresult ( *cuSignalExternalSemaphoresAsync )( const CUexternalSemaphore *extSemArray, const CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS *paramsArray, unsigned int numExtSems, CUstream stream );
    CUresult ( *cuWaitExternalSemaphoresAsync )( const CUexternalSemaphore *extSemArray, const CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS *paramsArray, unsigned int numExtSems, CUstream stream );
    CUresult ( *cuDestroyExternalSemaphore )( CUexternalSemaphore extSem );

    CUresult ( *cuModuleLoad )( CUmodule* module, const char* fname );
    CUresult ( *cuModuleLoadData )( CUmodule* module, const void* image );
    CUresult ( *cuModuleLoadDataEx )( CUmodule* module, const void* image, unsigned int numOptions, CUjit_option* options, void** optionValues );
    CUresult ( *cuModuleLoadFatBinary )( CUmodule* module, const void* fatCubin );
    CUresult ( *cuModuleUnload )( CUmodule hmod );
    CUresult ( *cuModuleGetFunction )( CUfunction* hfunc, CUmodule hmod, const char* name );
    CUresult ( *cuModuleGetGlobal )( CUdeviceptr* dptr, size_t* bytes, CUmodule hmod, const char* name );
    CUresult ( *cuLaunchKernel )( CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void** kernelParams, void** extra );
};

DLL_OBJECT extern CudaDeviceApiFunctionTable g_cudaDeviceApiFunctionTable;

#ifndef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#endif

DLL_OBJECT void _checkCUresult(CUresult cuResult, const char* text, const char* locationText);
#define checkCUresult(cuResult, text) _checkCUresult(cuResult, text, __FILE__ ":" TOSTRING(__LINE__))

DLL_OBJECT bool initializeCudaDeviceApiFunctionTable();
DLL_OBJECT bool getIsCudaDeviceApiFunctionTableInitialized();
DLL_OBJECT void freeCudaDeviceApiFunctionTable();

/**
 * Returns the closest matching CUDA device.
 * @param device The Vulkan device.
 * @param cuDevice The CUDA device (if true is returned).
 * @return Whether a matching CUDA device was found.
 */
DLL_OBJECT bool getMatchingCudaDevice(sgl::vk::Device* device, CUdevice* cuDevice);


/**
 * A CUDA driver API CUexternalSemaphore object created from a Vulkan semaphore.
 * Both binary and timeline semaphores are supported, but timeline semaphores require at least CUDA 11.2.
 */
class DLL_OBJECT SemaphoreVkCudaDriverApiInterop : public vk::Semaphore {
public:
    explicit SemaphoreVkCudaDriverApiInterop(
            Device* device, VkSemaphoreCreateFlags semaphoreCreateFlags = 0,
            VkSemaphoreType semaphoreType = VK_SEMAPHORE_TYPE_BINARY, uint64_t timelineSemaphoreInitialValue = 0);
    ~SemaphoreVkCudaDriverApiInterop() override;

    /// Signal semaphore.
    void signalSemaphoreCuda(CUstream stream, unsigned long long timelineValue = 0);

    /// Wait on semaphore.
    void waitSemaphoreCuda(CUstream stream, unsigned long long timelineValue = 0);

private:
    CUexternalSemaphore cuExternalSemaphore = {};
};

typedef std::shared_ptr<SemaphoreVkCudaDriverApiInterop> SemaphoreVkCudaDriverApiInteropPtr;


/**
 * A CUDA driver API CUdeviceptr object created from a Vulkan buffer.
 */
class DLL_OBJECT BufferCudaDriverApiExternalMemoryVk
{
public:
    explicit BufferCudaDriverApiExternalMemoryVk(vk::BufferPtr& vulkanBuffer);
    virtual ~BufferCudaDriverApiExternalMemoryVk();

    inline const sgl::vk::BufferPtr& getVulkanBuffer() { return vulkanBuffer; }
    [[nodiscard]] inline CUdeviceptr getCudaDevicePtr() const { return cudaDevicePtr; }

protected:
    sgl::vk::BufferPtr vulkanBuffer;
    CUexternalMemory cudaExternalMemoryBuffer{};
    CUdeviceptr cudaDevicePtr{};

#ifdef _WIN32
    HANDLE handle = nullptr;
#else
    int fileDescriptor = -1;
#endif
};

typedef std::shared_ptr<BufferCudaDriverApiExternalMemoryVk> BufferCudaDriverApiExternalMemoryVkPtr;
typedef BufferCudaDriverApiExternalMemoryVk BufferCudaExternalMemoryVk;
typedef std::shared_ptr<BufferCudaExternalMemoryVk> BufferCudaExternalMemoryVkPtr;


/**
 * A CUDA driver API CUmipmappedArray object created from a Vulkan image.
 */
class DLL_OBJECT ImageCudaExternalMemoryVk
{
public:
    explicit ImageCudaExternalMemoryVk(vk::ImagePtr& vulkanImage);
    ImageCudaExternalMemoryVk(
            vk::ImagePtr& vulkanImage, VkImageViewType imageViewType, bool surfaceLoadStore);
    virtual ~ImageCudaExternalMemoryVk();

    inline const sgl::vk::ImagePtr& getVulkanImage() { return vulkanImage; }
    [[nodiscard]] inline CUmipmappedArray getCudaMipmappedArray() const { return cudaMipmappedArray; }
    CUarray getCudaMipmappedArrayLevel(uint32_t level = 0);

    /*
     * Asynchronous copy from a device pointer to level 0 mipmap level.
     */
    void memcpyCudaDtoA2DAsync(CUdeviceptr devicePtr, CUstream stream);
    void memcpyCudaDtoA3DAsync(CUdeviceptr devicePtr, CUstream stream);

protected:
    void _initialize(vk::ImagePtr& _vulkanImage, VkImageViewType imageViewType, bool surfaceLoadStore);

    sgl::vk::ImagePtr vulkanImage;
    CUexternalMemory cudaExternalMemoryBuffer{};
    CUmipmappedArray cudaMipmappedArray{};

    // Cache for storing the array for mipmap level 0.
    CUarray cudaArrayLevel0{};

#ifdef _WIN32
    HANDLE handle = nullptr;
#else
    int fileDescriptor = -1;
#endif
};

typedef std::shared_ptr<ImageCudaExternalMemoryVk> ImageCudaExternalMemoryVkPtr;
typedef ImageCudaExternalMemoryVk ImageCudaDriverApiExternalMemoryVk;
typedef std::shared_ptr<ImageCudaDriverApiExternalMemoryVk> ImageCudaDriverApiExternalMemoryVkPtr;

class TextureCudaExternalMemoryVk {
public:
    explicit TextureCudaExternalMemoryVk(vk::TexturePtr& vulkanTexture);
    TextureCudaExternalMemoryVk(
            vk::ImagePtr& vulkanImage, const ImageSamplerSettings& samplerSettings);
    TextureCudaExternalMemoryVk(
            vk::ImagePtr& vulkanImage, const ImageSamplerSettings& samplerSettings,
            VkImageViewType imageViewType);
    TextureCudaExternalMemoryVk(
            vk::ImagePtr& vulkanImage, const ImageSamplerSettings& samplerSettings,
            VkImageViewType imageViewType, VkImageSubresourceRange imageSubresourceRange);
    ~TextureCudaExternalMemoryVk();

    [[nodiscard]] inline CUtexObject getCudaTextureObject() const { return cudaTextureObject; }
    inline const sgl::vk::ImagePtr& getVulkanImage() { return imageCudaExternalMemory->getVulkanImage(); }
    inline const ImageCudaExternalMemoryVkPtr& getImageCudaExternalMemory() { return imageCudaExternalMemory; }

protected:
    CUtexObject cudaTextureObject{};
    ImageCudaExternalMemoryVkPtr imageCudaExternalMemory;
};

typedef std::shared_ptr<TextureCudaExternalMemoryVk> TextureCudaExternalMemoryVkPtr;

class SurfaceCudaExternalMemoryVk {
public:
    SurfaceCudaExternalMemoryVk(vk::ImagePtr& vulkanImageView, VkImageViewType imageViewType);
    explicit SurfaceCudaExternalMemoryVk(vk::ImageViewPtr& vulkanImageView);
    ~SurfaceCudaExternalMemoryVk();

    [[nodiscard]] inline CUtexObject getCudaSurfaceObject() const { return cudaSurfaceObject; }
    inline const sgl::vk::ImagePtr& getVulkanImage() { return imageCudaExternalMemory->getVulkanImage(); }

protected:
    CUsurfObject cudaSurfaceObject{};
    ImageCudaExternalMemoryVkPtr imageCudaExternalMemory;
};

typedef std::shared_ptr<SurfaceCudaExternalMemoryVk> SurfaceCudaExternalMemoryVkPtr;

}}

#endif //SGL_INTEROPCUDA_HPP
