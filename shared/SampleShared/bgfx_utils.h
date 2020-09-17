/*
 * Copyright 2011-2020 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */
#pragma once
#include "pch.h"
#ifndef BGFX_UTILS_H_HEADER_GUARD
#define BGFX_UTILS_H_HEADER_GUARD

#include <bx/pixelformat.h>
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include "bounds.h"
#include <d3dcommon.h>  //ID3DBlob
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrExtensionContext.h>

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
#include <wil/resource.h>

namespace stl = tinystl;

template <typename bgfx_handle_t>
struct bgfx_handle_wrapper_t : public bgfx_handle_t {
    bgfx_handle_wrapper_t() {
        this->idx = bgfx::kInvalidHandle;
    }

    bgfx_handle_wrapper_t(const bgfx_handle_t& handle) {
        this->idx = handle.idx;
    }

    bgfx_handle_wrapper_t(const uint16_t& handle) {
        this->idx = handle;
    }

    operator uint16_t() const {
        return this->idx;
    }
};

template <typename bgfx_handle_t, typename close_fn_t = void (*)(bgfx_handle_t), close_fn_t close_fn = bgfx::destroy>
using unique_bgfx_handle = wil::unique_any<bgfx_handle_wrapper_t<bgfx_handle_t>,
                                           close_fn_t,
                                           close_fn,
                                           wil::details::pointer_access_all,
                                           bgfx_handle_wrapper_t<bgfx_handle_t>,
                                           decltype(bgfx::kInvalidHandle),
                                           bgfx::kInvalidHandle,
                                           bgfx_handle_wrapper_t<bgfx_handle_t>>;

template <typename bgfx_handle_t,
          typename close_fn_t = void (*)(bgfx_handle_t),
          close_fn_t close_fn = bgfx::destroy>
using shared_bgfx_handle = wil::shared_any<unique_bgfx_handle<bgfx_handle_t, close_fn_t, close_fn>>;

namespace sample::bg {
    struct Swapchain {
        virtual ~Swapchain() = default;

        xr::SwapchainHandle Handle;
        DXGI_FORMAT Format{DXGI_FORMAT_UNKNOWN};
        uint32_t Width{0};
        uint32_t Height{0};
        uint32_t ArraySize{0};
    };
    struct SwapchainD3D11 : public sample::bg::Swapchain {
        std::vector<XrSwapchainImageD3D11KHR> Images;
    };

    struct SwapchainD3D12 : public sample::bg::Swapchain {
        // std::vector<XrSwapchainImageD3D12KHR> Images;
    };
}
///
void* load(const char* _filePath, uint32_t* _size = NULL);

///
void unload(void* _ptr);

///
bgfx::ShaderHandle loadShader(const char* _name);

///
bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);

///
bgfx::TextureHandle loadTexture(const wchar_t* _name, uint64_t _flags = BGFX_TEXTURE_NONE|BGFX_SAMPLER_NONE, uint8_t _skip = 0, bgfx::TextureInfo* _info = NULL, bimg::Orientation::Enum* _orientation = NULL);

///
//bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat);

///
void calcTangents(void* _vertices, uint16_t _numVertices, bgfx::VertexLayout _layout, const uint16_t* _indices, uint32_t _numIndices);

/// Returns true if both internal transient index and vertex buffer have
/// enough space.
///
/// @param[in] _numVertices Number of vertices.
/// @param[in] _layout Vertex layout.
/// @param[in] _numIndices Number of indices.
///
inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
	return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout)
		&& (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices) )
		;
}

///
inline uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
{
	const float src[] =
	{
		_x * 0.5f + 0.5f,
		_y * 0.5f + 0.5f,
		_z * 0.5f + 0.5f,
		_w * 0.5f + 0.5f,
	};
	uint32_t dst;
	bx::packRgba8(&dst, src);
	return dst;
}

///
struct MeshState
{
	struct Texture
	{
		uint32_t            m_flags;
		bgfx::UniformHandle m_sampler;
		bgfx::TextureHandle m_texture;
		uint8_t             m_stage;
	};

	Texture             m_textures[4];
	uint64_t            m_state;
	bgfx::ProgramHandle m_program;
	uint8_t             m_numTextures;
	bgfx::ViewId        m_viewId;
};

struct Primitive
{
	uint32_t m_startIndex;
	uint32_t m_numIndices;
	uint32_t m_startVertex;
	uint32_t m_numVertices;
	
	Sphere m_sphere;
	Aabb m_aabb;
	Obb m_obb;
};

typedef stl::vector<Primitive> PrimitiveArray;

struct Group
{
	Group();
	void reset();
	
	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
	uint16_t m_numVertices;
	uint8_t* m_vertices;
	uint32_t m_numIndices;
	uint16_t* m_indices;
	Sphere m_sphere;
	Aabb m_aabb;
	Obb m_obb;
	PrimitiveArray m_prims;
};
typedef stl::vector<Group> GroupArray;

struct Mesh
{
	void load(bx::ReaderSeekerI* _reader, bool _ramcopy);
	void unload();
	void submit(bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state) const;
	void submit(const MeshState*const* _state, uint8_t _numPasses, const float* _mtx, uint16_t _numMatrices) const;

	bgfx::VertexLayout m_layout;
	GroupArray m_groups;
};

///
Mesh* meshLoad(const char* _filePath, bool _ramcopy = false);

///
void meshUnload(Mesh* _mesh);

///
MeshState* meshStateCreate();

///
void meshStateDestroy(MeshState* _meshState);

///
void meshSubmit(const Mesh* _mesh, bgfx::ViewId _id, bgfx::ProgramHandle _program, const float* _mtx, uint64_t _state = BGFX_STATE_MASK);

///
void meshSubmit(const Mesh* _mesh, const MeshState*const* _state, uint8_t _numPasses, const float* _mtx, uint16_t _numMatrices = 1);

///
struct Args
{
	Args(int _argc, const char* const* _argv);

	bgfx::RendererType::Enum m_type;
	uint16_t m_pciId;
};

#endif // BGFX_UTILS_H_HEADER_GUARD
