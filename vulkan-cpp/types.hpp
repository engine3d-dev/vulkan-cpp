#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <span>
#include <array>
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

namespace vk {
    enum format : uint32_t{
        // Core Formats
        undefined = VK_FORMAT_UNDEFINED,
        r4g4_unorm_pack8 = VK_FORMAT_R4G4_UNORM_PACK8,
        r4g4b4a4_unorm_pack16 = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
        b4g4r4a4_unorm_pack16 = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
        r5g6b5_unorm_pack16 = VK_FORMAT_R5G6B5_UNORM_PACK16,
        b5g6r5_unorm_pack16 = VK_FORMAT_B5G6R5_UNORM_PACK16,
        r5g5b5a1_unorm_pack16 = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
        b5g5r5a1_unorm_pack16 = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
        a1r5g5b5_unorm_pack16 = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
        r8_unorm = VK_FORMAT_R8_UNORM,
        r8_snorm = VK_FORMAT_R8_SNORM,
        r8_uscaled = VK_FORMAT_R8_USCALED,
        r8_sscaled = VK_FORMAT_R8_SSCALED,
        r8_uint = VK_FORMAT_R8_UINT,
        r8_sint = VK_FORMAT_R8_SINT,
        r8_srgb = VK_FORMAT_R8_SRGB,
        r8g8_unorm = VK_FORMAT_R8G8_UNORM,
        r8g8_snorm = VK_FORMAT_R8G8_SNORM,
        r8g8_uscaled = VK_FORMAT_R8G8_USCALED,
        r8g8_sscaled = VK_FORMAT_R8G8_SSCALED,
        r8g8_uint = VK_FORMAT_R8G8_UINT,
        r8g8_sint = VK_FORMAT_R8G8_SINT,
        r8g8_srgb = VK_FORMAT_R8G8_SRGB,
        r8g8b8_unorm = VK_FORMAT_R8G8B8_UNORM,
        r8g8b8_snorm = VK_FORMAT_R8G8B8_SNORM,
        r8g8b8_uscaled = VK_FORMAT_R8G8B8_USCALED,
        r8g8b8_sscaled = VK_FORMAT_R8G8B8_SSCALED,
        r8g8b8_uint = VK_FORMAT_R8G8B8_UINT,
        r8g8b8_sint = VK_FORMAT_R8G8B8_SINT,
        r8g8b8_srgb = VK_FORMAT_R8G8B8_SRGB,
        b8g8r8_unorm = VK_FORMAT_B8G8R8_UNORM,
        b8g8r8_snorm = VK_FORMAT_B8G8R8_SNORM,
        b8g8r8_uscaled = VK_FORMAT_B8G8R8_USCALED,
        b8g8r8_sscaled = VK_FORMAT_B8G8R8_SSCALED,
        b8g8r8_uint = VK_FORMAT_B8G8R8_UINT,
        b8g8r8_sint = VK_FORMAT_B8G8R8_SINT,
        b8g8r8_srgb = VK_FORMAT_B8G8R8_SRGB,
        r8g8b8a8_unorm = VK_FORMAT_R8G8B8A8_UNORM,
        r8g8b8a8_snorm = VK_FORMAT_R8G8B8A8_SNORM,
        r8g8b8a8_uscaled = VK_FORMAT_R8G8B8A8_USCALED,
        r8g8b8a8_sscaled = VK_FORMAT_R8G8B8A8_SSCALED,
        r8g8b8a8_uint = VK_FORMAT_R8G8B8A8_UINT,
        r8g8b8a8_sint = VK_FORMAT_R8G8B8A8_SINT,
        r8g8b8a8_srgb = VK_FORMAT_R8G8B8A8_SRGB,
        b8g8r8a8_unorm = VK_FORMAT_B8G8R8A8_UNORM,
        b8g8r8a8_snorm = VK_FORMAT_B8G8R8A8_SNORM,
        b8g8r8a8_uscaled = VK_FORMAT_B8G8R8A8_USCALED,
        b8g8r8a8_sscaled = VK_FORMAT_B8G8R8A8_SSCALED,
        b8g8r8a8_uint = VK_FORMAT_B8G8R8A8_UINT,
        b8g8r8a8_sint = VK_FORMAT_B8G8R8A8_SINT,
        b8g8r8a8_srgb = VK_FORMAT_B8G8R8A8_SRGB,
        a8b8g8r8_unorm_pack32 = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        a8b8g8r8_snorm_pack32 = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
        a8b8g8r8_uscaled_pack32 = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
        a8b8g8r8_sscaled_pack32 = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
        a8b8g8r8_uint_pack32 = VK_FORMAT_A8B8G8R8_UINT_PACK32,
        a8b8g8r8_sint_pack32 = VK_FORMAT_A8B8G8R8_SINT_PACK32,
        a8b8g8r8_srgb_pack32 = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        a2r10g10b10_unorm_pack32 = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        a2r10g10b10_snorm_pack32 = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
        a2r10g10b10_uscaled_pack32 = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
        a2r10g10b10_sscaled_pack32 = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
        a2r10g10b10_uint_pack32 = VK_FORMAT_A2R10G10B10_UINT_PACK32,
        a2r10g10b10_sint_pack32 = VK_FORMAT_A2R10G10B10_SINT_PACK32,
        a2b10g10r10_unorm_pack32 = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        a2b10g10r10_snorm_pack32 = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
        a2b10g10r10_uscaled_pack32 = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
        a2b10g10r10_sscaled_pack32 = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
        a2b10g10r10_uint_pack32 = VK_FORMAT_A2B10G10R10_UINT_PACK32,
        a2b10g10r10_sint_pack32 = VK_FORMAT_A2B10G10R10_SINT_PACK32,
        r16_unorm = VK_FORMAT_R16_UNORM,
        r16_snorm = VK_FORMAT_R16_SNORM,
        r16_uscaled = VK_FORMAT_R16_USCALED,
        r16_sscaled = VK_FORMAT_R16_SSCALED,
        r16_uint = VK_FORMAT_R16_UINT,
        r16_sint = VK_FORMAT_R16_SINT,
        r16_sfloat = VK_FORMAT_R16_SFLOAT,
        r16g16_unorm = VK_FORMAT_R16G16_UNORM,
        r16g16_snorm = VK_FORMAT_R16G16_SNORM,
        r16g16_uscaled = VK_FORMAT_R16G16_USCALED,
        r16g16_sscaled = VK_FORMAT_R16G16_SSCALED,
        r16g16_uint = VK_FORMAT_R16G16_UINT,
        r16g16_sint = VK_FORMAT_R16G16_SINT,
        r16g16_sfloat = VK_FORMAT_R16G16_SFLOAT,
        r16g16b16_unorm = VK_FORMAT_R16G16B16_UNORM,
        r16g16b16_snorm = VK_FORMAT_R16G16B16_SNORM,
        r16g16b16_uscaled = VK_FORMAT_R16G16B16_USCALED,
        r16g16b16_sscaled = VK_FORMAT_R16G16B16_SSCALED,
        r16g16b16_uint = VK_FORMAT_R16G16B16_UINT,
        r16g16b16_sint = VK_FORMAT_R16G16B16_SINT,
        r16g16b16_sfloat = VK_FORMAT_R16G16B16_SFLOAT,
        r16g16b16a16_unorm = VK_FORMAT_R16G16B16A16_UNORM,
        r16g16b16a16_snorm = VK_FORMAT_R16G16B16A16_SNORM,
        r16g16b16a16_uscaled = VK_FORMAT_R16G16B16A16_USCALED,
        r16g16b16a16_sscaled = VK_FORMAT_R16G16B16A16_SSCALED,
        r16g16b16a16_uint = VK_FORMAT_R16G16B16A16_UINT,
        r16g16b16a16_sint = VK_FORMAT_R16G16B16A16_SINT,
        r16g16b16a16_sfloat = VK_FORMAT_R16G16B16A16_SFLOAT,
        r32_uint = VK_FORMAT_R32_UINT,
        r32_sint = VK_FORMAT_R32_SINT,
        r32_sfloat = VK_FORMAT_R32_SFLOAT,
        rg32_uint = VK_FORMAT_R32G32_UINT,
        rg32_sint = VK_FORMAT_R32G32_SINT,
        rg32_sfloat = VK_FORMAT_R32G32_SFLOAT,
        rgb32_uint = VK_FORMAT_R32G32B32_UINT,
        rgb32_sint = VK_FORMAT_R32G32B32_SINT,
        rgb32_sfloat = VK_FORMAT_R32G32B32_SFLOAT, // Example from prompt: VK_FORMAT_R32G32B32_SFLOAT
        rgba_uint = VK_FORMAT_R32G32B32A32_UINT,
        rgba_sint = VK_FORMAT_R32G32B32A32_SINT,
        rgba_sfloat = VK_FORMAT_R32G32B32A32_SFLOAT,
        r64_uint = VK_FORMAT_R64_UINT,
        r64_sint = VK_FORMAT_R64_SINT,
        r64_sfloat = VK_FORMAT_R64_SFLOAT,
        rg64_uint = VK_FORMAT_R64G64_UINT,
        rg64_sint = VK_FORMAT_R64G64_SINT,
        rg64_sfloat = VK_FORMAT_R64G64_SFLOAT,
        rgb64_uint = VK_FORMAT_R64G64B64_UINT,
        rgb64_sint = VK_FORMAT_R64G64B64_SINT,
        rgb64_sfloat = VK_FORMAT_R64G64B64_SFLOAT,
        rgba64_uint = VK_FORMAT_R64G64B64A64_UINT,
        rgba64_sint = VK_FORMAT_R64G64B64A64_SINT,
        r64g64b64a64_sfloat = VK_FORMAT_R64G64B64A64_SFLOAT,
        b10g11r11_ufloat_pack32 = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        e5b9g9r9_ufloat_pack32 = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
        
        // Depth/Stencil Formats
        d16_unorm = VK_FORMAT_D16_UNORM,
        x8_d24_unorm_pack32 = VK_FORMAT_X8_D24_UNORM_PACK32,
        d32_sfloat = VK_FORMAT_D32_SFLOAT,
        s8_uint = VK_FORMAT_S8_UINT,
        d16_unorm_s8_uint = VK_FORMAT_D16_UNORM_S8_UINT,
        d24_unorm_s8_uint = VK_FORMAT_D24_UNORM_S8_UINT,
        d32_sfloat_s8_uint = VK_FORMAT_D32_SFLOAT_S8_UINT,
        
        // Compressed Formats (BC)
        bc1_rgb_unorm_block = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
        bc1_rgb_srgb_block = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        bc1_rgba_unorm_block = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        bc1_rgba_srgb_block = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        bc2_unorm_block = VK_FORMAT_BC2_UNORM_BLOCK,
        bc2_srgb_block = VK_FORMAT_BC2_SRGB_BLOCK,
        bc3_unorm_block = VK_FORMAT_BC3_UNORM_BLOCK,
        bc3_srgb_block = VK_FORMAT_BC3_SRGB_BLOCK,
        bc4_unorm_block = VK_FORMAT_BC4_UNORM_BLOCK,
        bc4_snorm_block = VK_FORMAT_BC4_SNORM_BLOCK,
        bc5_unorm_block = VK_FORMAT_BC5_UNORM_BLOCK,
        bc5_snorm_block = VK_FORMAT_BC5_SNORM_BLOCK,
        bc6h_ufloat_block = VK_FORMAT_BC6H_UFLOAT_BLOCK,
        bc6h_sfloat_block = VK_FORMAT_BC6H_SFLOAT_BLOCK,
        bc7_unorm_block = VK_FORMAT_BC7_UNORM_BLOCK,
        bc7_srgb_block = VK_FORMAT_BC7_SRGB_BLOCK,
        
        // Compressed Formats (ETC2/EAC)
        etc2_r8g8b8_unorm_block = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
        etc2_r8g8b8_srgb_block = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
        etc2_r8g8b8a1_unorm_block = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
        etc2_r8g8b8a1_srgb_block = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
        etc2_r8g8b8a8_unorm_block = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
        etc2_r8g8b8a8_srgb_block = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
        eac_r11_unorm_block = VK_FORMAT_EAC_R11_UNORM_BLOCK,
        eac_r11_snorm_block = VK_FORMAT_EAC_R11_SNORM_BLOCK,
        eac_r11g11_unorm_block = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
        eac_r11g11_snorm_block = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
        
        // Compressed Formats (ASTC)
        astc_4x4_unorm_block = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        astc_4x4_srgb_block = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        astc_5x4_unorm_block = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        astc_5x4_srgb_block = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        astc_5x5_unorm_block = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        astc_5x5_srgb_block = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        astc_6x5_unorm_block = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        astc_6x5_srgb_block = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        astc_6x6_unorm_block = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        astc_6x6_srgb_block = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        astc_8x5_unorm_block = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        astc_8x5_srgb_block = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        astc_8x6_unorm_block = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        astc_8x6_srgb_block = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        astc_8x8_unorm_block = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        astc_8x8_srgb_block = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        astc_10x5_unorm_block = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        astc_10x5_srgb_block = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        astc_10x6_unorm_block = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        astc_10x6_srgb_block = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        astc_10x8_unorm_block = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        astc_10x8_srgb_block = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        astc_10x10_unorm_block = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        astc_10x10_srgb_block = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        astc_12x10_unorm_block = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        astc_12x10_srgb_block = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        astc_12x12_unorm_block = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        astc_12x12_srgb_block = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        
        // YCbCr Formats (KHR Extension, now Core in 1.1)
        g8b8g8r8_422_unorm = VK_FORMAT_G8B8G8R8_422_UNORM,
        b8g8r8g8_422_unorm = VK_FORMAT_B8G8R8G8_422_UNORM,
        g8_b8_r8_3plane_420_unorm = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
        g8_b8r8_2plane_420_unorm = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        g8_b8_r8_3plane_422_unorm = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
        g8_b8r8_2plane_422_unorm = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
        g8_b8_r8_3plane_444_unorm = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
        r10x6_unorm_pack16 = VK_FORMAT_R10X6_UNORM_PACK16,
        r10x6g10x6_unorm_2pack16 = VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
        r10x6g10x6b10x6a10x6_unorm_4pack16 = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        g10x6b10x6g10x6r10x6_422_unorm_4pack16 = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        b10x6g10x6r10x6g10x6_422_unorm_4pack16 = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        g10x6_b10x6_r10x6_3plane_420_unorm_3pack16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        g10x6_b10x6r10x6_2plane_420_unorm_3pack16 = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        g10x6_b10x6_r10x6_3plane_422_unorm_3pack16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        g10x6_b10x6r10x6_2plane_422_unorm_3pack16 = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        g10x6_b10x6_r10x6_3plane_444_unorm_3pack16 = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        r12x4_unorm_pack16 = VK_FORMAT_R12X4_UNORM_PACK16,
        r12x4g12x4_unorm_2pack16 = VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
        r12x4g12x4b12x4a12x4_unorm_4pack16 = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        g12x4b12x4g12x4r12x4_422_unorm_4pack16 = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        b12x4g12x4r12x4g12x4_422_unorm_4pack16 = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        g12x4_b12x4_r12x4_3plane_420_unorm_3pack16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        g12x4_b12x4r12x4_2plane_420_unorm_3pack16 = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        g12x4_b12x4_r12x4_3plane_422_unorm_3pack16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        g12x4_b12x4r12x4_2plane_422_unorm_3pack16 = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        g12x4_b12x4_r12x4_3plane_444_unorm_3pack16 = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        g16b16g16r16_422_unorm = VK_FORMAT_G16B16G16R16_422_UNORM,
        b16g16r16g16_422_unorm = VK_FORMAT_B16G16R16G16_422_UNORM,
        g16_b16_r16_3plane_420_unorm = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
        g16_b16r16_2plane_420_unorm = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
        g16_b16_r16_3plane_422_unorm = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
        g16_b16r16_2plane_422_unorm = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
        g16_b16_r16_3plane_444_unorm = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        
        // EXT/Vendor YCbCr Formats
        g8_b8r8_2plane_444_unorm = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
        g10x6_b10x6r10x6_2plane_444_unorm_3pack16 = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        g12x4_b12x4r12x4_2plane_444_unorm_3pack16 = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        g16_b16r16_2plane_444_unorm = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
        
        // EXT Packed Formats
        a4r4g4b4_unorm_pack16 = VK_FORMAT_A4R4G4B4_UNORM_PACK16,
        a4b4g4r4_unorm_pack16 = VK_FORMAT_A4B4G4R4_UNORM_PACK16,
        
        // ASTC SFLOAT Formats
        astc_4x4_sfloat_block = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
        astc_5x4_sfloat_block = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
        astc_5x5_sfloat_block = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
        astc_6x5_sfloat_block = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
        astc_6x6_sfloat_block = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
        astc_8x5_sfloat_block = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
        astc_8x6_sfloat_block = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
        astc_8x8_sfloat_block = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
        astc_10x5_sfloat_block = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
        astc_10x6_sfloat_block = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
        astc_10x8_sfloat_block = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
        astc_10x10_sfloat_block = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
        astc_12x10_sfloat_block = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
        astc_12x12_sfloat_block = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
        
        // PVRTC Formats (IMG Vendor)
        pvrtc1_2bpp_unorm_block_img = VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
        pvrtc1_4bpp_unorm_block_img = VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
        pvrtc2_2bpp_unorm_block_img = VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
        pvrtc2_4bpp_unorm_block_img = VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
        pvrtc1_2bpp_srgb_block_img = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
        pvrtc1_4bpp_srgb_block_img = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
        pvrtc2_2bpp_srgb_block_img = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
        pvrtc2_4bpp_srgb_block_img = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
        
        // Vendor Specific (NV)
        r16g16_sfixed5_nv = VK_FORMAT_R16G16_SFIXED5_NV,
        
        // KHR/EXT Aliases (A1B5G5R5, A8)
        a1b5g5r5_unorm_pack16_khr = VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR,
        a8_unorm_khr = VK_FORMAT_A8_UNORM_KHR,

        // Aliases to Core Formats
        // ASTC SFLOAT EXT Aliases
        astc_4x4_sfloat_block_ext = VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
        astc_5x4_sfloat_block_ext = VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
        astc_5x5_sfloat_block_ext = VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
        astc_6x5_sfloat_block_ext = VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
        astc_6x6_sfloat_block_ext = VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
        astc_8x5_sfloat_block_ext = VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
        astc_8x6_sfloat_block_ext = VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
        astc_8x8_sfloat_block_ext = VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
        astc_10x5_sfloat_block_ext = VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
        astc_10x6_sfloat_block_ext = VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
        astc_10x8_sfloat_block_ext = VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
        astc_10x10_sfloat_block_ext = VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
        astc_12x10_sfloat_block_ext = VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
        astc_12x12_sfloat_block_ext = VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
        
        // YCbCr KHR Aliases
        g8b8g8r8_422_unorm_khr = VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
        b8g8r8g8_422_unorm_khr = VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
        g8_b8_r8_3plane_420_unorm_khr = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
        g8_b8r8_2plane_420_unorm_khr = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
        g8_b8_r8_3plane_422_unorm_khr = VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
        g8_b8r8_2plane_422_unorm_khr = VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
        g8_b8_r8_3plane_444_unorm_khr = VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
        r10x6_unorm_pack16_khr = VK_FORMAT_R10X6_UNORM_PACK16_KHR,
        r10x6g10x6_unorm_2pack16_khr = VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
        r10x6g10x6b10x6a10x6_unorm_4pack16_khr = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
        g10x6b10x6g10x6r10x6_422_unorm_4pack16_khr = VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
        b10x6g10x6r10x6g10x6_422_unorm_4pack16_khr = VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
        g10x6_b10x6_r10x6_3plane_420_unorm_3pack16_khr = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
        g10x6_b10x6r10x6_2plane_420_unorm_3pack16_khr = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
        g10x6_b10x6_r10x6_3plane_422_unorm_3pack16_khr = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
        g10x6_b10x6r10x6_2plane_422_unorm_3pack16_khr = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
        g10x6_b10x6_r10x6_3plane_444_unorm_3pack16_khr = VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
        r12x4_unorm_pack16_khr = VK_FORMAT_R12X4_UNORM_PACK16_KHR,
        r12x4g12x4_unorm_2pack16_khr = VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
        r12x4g12x4b12x4a12x4_unorm_4pack16_khr = VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
        g12x4b12x4g12x4r12x4_422_unorm_4pack16_khr = VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
        b12x4g12x4r12x4g12x4_422_unorm_4pack16_khr = VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
        g12x4_b12x4_r12x4_3plane_420_unorm_3pack16_khr = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
        g12x4_b12x4r12x4_2plane_420_unorm_3pack16_khr = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
        g12x4_b12x4_r12x4_3plane_422_unorm_3pack16_khr = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
        g12x4_b12x4r12x4_2plane_422_unorm_3pack16_khr = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
        g12x4_b12x4_r12x4_3plane_444_unorm_3pack16_khr = VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
        g16b16g16r16_422_unorm_khr = VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
        b16g16r16g16_422_unorm_khr = VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
        g16_b16_r16_3plane_420_unorm_khr = VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
        g16_b16r16_2plane_420_unorm_khr = VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
        g16_b16_r16_3plane_422_unorm_khr = VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
        g16_b16r16_2plane_422_unorm_khr = VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
        g16_b16_r16_3plane_444_unorm_khr = VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
        
        // YCbCr EXT Aliases
        g8_b8r8_2plane_444_unorm_ext = VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT,
        g10x6_b10x6r10x6_2plane_444_unorm_3pack16_ext = VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT,
        g12x4_b12x4r12x4_2plane_444_unorm_3pack16_ext = VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT,
        g16_b16r16_2plane_444_unorm_ext = VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT,
        
        // A4 Packed EXT Aliases
        a4r4g4b4_unorm_pack16_ext = VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT,
        a4b4g4r4_unorm_pack16_ext = VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT,
        
        // NV Alias
        r16g16_s10_5_nv = VK_FORMAT_R16G16_S10_5_NV,
        
        // Max Enum
        max_enum_format = VK_FORMAT_MAX_ENUM
    };

    enum image_aspect_flags : uint32_t {
        color_bit = VK_IMAGE_ASPECT_COLOR_BIT,
        depth_bit = VK_IMAGE_ASPECT_DEPTH_BIT,
        stencil_bit = VK_IMAGE_ASPECT_STENCIL_BIT,
        metadata_bit = VK_IMAGE_ASPECT_METADATA_BIT,
        plane0_bit = VK_IMAGE_ASPECT_PLANE_0_BIT,
        plane1_bit = VK_IMAGE_ASPECT_PLANE_1_BIT,
        plane2_bit = VK_IMAGE_ASPECT_PLANE_2_BIT,
        none = VK_IMAGE_ASPECT_NONE,
        memory_plane0_bit_ext = VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT,
        memory_plane1_bit_ext = VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT,
        memory_plane2_bit_ext = VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT,
        plane1_bit_khr   = VK_IMAGE_ASPECT_PLANE_1_BIT_KHR,
        plane2_bit_khr = VK_IMAGE_ASPECT_PLANE_2_BIT_KHR,
        none_khr = VK_IMAGE_ASPECT_NONE_KHR,
        bits_max_enum = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM
    };

    /**
     * @brief message sevierity explicitly to max size of a byte
     */
    enum message : uint32_t {
        verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        info = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
        warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        max_num = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT
    };

    enum debug : uint32_t {
        general = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,
        validation = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        performance = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        device_address_binding_ext = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
        max_enum = VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT
    };

    enum class api_version : uint32_t {
        vk_1_2 = VK_API_VERSION_1_2,
        vk_1_3 = VK_API_VERSION_1_3, // vulkan version 1.3
        // vk_1_2, // vulkan version 1.4
    };

    struct debug_message_utility {
        uint32_t severity;
        uint32_t message_type;
        uint32_t (*callback)(VkDebugUtilsMessageSeverityFlagBitsEXT,
                             VkDebugUtilsMessageTypeFlagsEXT,
                             const VkDebugUtilsMessengerCallbackDataEXT*,
                             void*) = nullptr;
    };

    struct application_params {
        std::string name = "";
        api_version version;
        std::span<const char*> validations;
        std::span<const char*> extensions;
    };

    struct swapchain_params {
        uint32_t width;
        uint32_t height;
        uint32_t present_index = -1;

        VkFormat depth; // depth format
    };

    struct filter_range {
        VkFilter min;
        VkFilter max;
    };

    struct layer_properties {
        std::string name;
        uint32_t version;
        uint32_t impl_version;
        std::string description;
    };

    //! @brief vk::physical defines what kinds of physical device specification
    //! to use that is available based on your current physical hardware
    //! specifications.
    enum class physical {
        integrated,
        discrete,
        virtualized,
        cpu,
        max_enum,
        other
    };

    /**
     * @brief Enumeration represents configuration for the physical device
     *
     * Defines the configuration for the VkPhysicalDevice handler to be created
     * with
     */
    struct physical_enumeration {
        physical device_type;
    };

    struct surface_params {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR format;
    };

    struct queue_params {
        uint32_t family = -1;
        uint32_t index = -1;
    };

    struct queue_indices {
        uint32_t graphics = -1;
        uint32_t compute = -1;
        uint32_t transfer = -1;
    };

    struct device_params {
        std::span<float> queue_priorities{};
        std::span<const char*>
          extensions{}; // Can add VK_KHR_SWAPCHAIN_EXTENSION_NAME to this
                        // extension
        uint8_t queue_family_index = 0;
    };

    // raw image handlers
    // struct image {
    //     VkImage image = nullptr;
    //     VkImageView view = nullptr;
    // };

    // sampler + raw image handlers
    // struct sampled_image {
    //     VkImage image = nullptr;
    //     VkImageView view = nullptr;
    //     VkSampler sampler = nullptr;
    //     VkDeviceMemory device_memory = nullptr;
    // };

    //! @brief enumeration if an image is provided
    // struct swapchain_image_enumeration {
    //     VkImage image = nullptr;
    //     VkFormat format;
    //     // VkImageAspectFlags aspect;
    //     image_aspect_flags aspect;
    //     uint32_t layer_count = 0;
    //     uint32_t mip_levels = 1;
    // };

    // Image enumeration for creating a brand new VkImage/VkImageView handlers
    // struct image_enumeration {
    //     uint32_t width = -1;
    //     uint32_t height = -1;
    //     VkFormat format;
    //     // VkImageAspectFlags aspect;
    //     image_aspect_flags aspect;
    //     uint32_t layer_count = 1;
    //     uint32_t mip_levels = 1;
    // };

    /**
     * @param renderpass vulkan requires framebuffers to know renderpasses up
     * front
     * @param views framebuffers provide the actual image views that will serve
     * as attachments to the renderpass handle
     * @param extent are for framebuffers to pass in frame image views to the
     * screen. Should be the size of the window viewport specified
     */
    struct framebuffer_params {
        VkRenderPass renderpass = nullptr;
        std::span<VkImageView> views;
        VkExtent2D extent;
    };

    /**
     * @param Primary
     * submission: immediately to vulkan queue for executing on device
     * execution: vkQueueSubmit primarily submits primary command buffers
     *
     * @param Seconary
     * submission: Cannot be submitted directly to vulkan queue
     * execution: Indirect execution by being called from primary command
     * buffers using `vkCmdExecuteCommands`
     *
     * When executed within render-pass instance, they inherit
     * renderpass/subpass state from primary command buffer
     *
     * Can be allocated and recorded in parallel, which can leverage
     * devices that support multi-core processing
     *
     * Have independent lifetimes that can be managed independently of
     * primary command buffers, allowing for more flexible resource management
     */
    enum class command_levels : uint32_t {
        primary = 0,
        secondary = 1,
        max_enum = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM,
    };

    /**
     * @param transient represents VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
     *
     * Usage: specifies command buffers allocated from pool are short-lived,
     * meaning they will reset or be freed in a short time-frame. Flag may also
     * be used by implementation to control memory allocation behavior within
     * the pool
     *
     * @param reset represents VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
     *
     * Usage: Allows command buffers allocated from the pool to be individually
     * reset to their initial state; either by calling vkResetCommandBuffer or
     * implicit reset when calling vkBeginCommandBuffer
     *
     * @param protected_bit represents VK_COMMAND_POOL_CREATE_PROTECTED_BIT
     *
     * Usage: Specifies command buffers allocated from pool are protected
     * command buffers; meaning the the memory allocated with the command pool
     * is protected
     *
     *
     */
    enum command_pool_flags : uint32_t {
        transient = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,     // represents VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
        reset = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,         // represents VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        protected_bit = VK_COMMAND_POOL_CREATE_PROTECTED_BIT, // represents VK_COMMAND_POOL_CREATE_PROTECTED_BIT
        max_enum_bit = VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM, // represents VK_COMMAND_POOL_CREATE_FLAG_BITS_MAX_ENUM
    };

    /**
     * @brief settings for specifying command buffers to construct
     *
     * @param Levels are for specifying the kind of command buffer being
     * constructed
     *
     * queue_index specifies the queue family associated with this
     * command buffer
     *
     * pool_flag in the context of the command buffers are used to
     * control memory allocation behavior within the command buffer pool to
     * allocate
     *
     * @param VK_COMMAND_POOL_CREATE_TRANSIENT_BIT Indicates command buffers
     * allocated from pool will be short-lived, meaning they'll reset or freed
     * quickly
     * @param VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT Allows individual
     * command buffers allocated from the pool to be reset to their initial
     * state using `vkResetCommandBuffer`
     * @param VK_COMMAND_POOL_CREATE_PROTECTED_BIT Designates command buffers
     * allocated from the pool as protective command buffers, implying they are
     * stored in protected memory and access to them is restricted to authorized
     * operations
     */
    struct command_params {

        command_levels levels;
        uint32_t queue_index = -1;
        // VkCommandPoolCreateFlagBits pool_flag;
        command_pool_flags flags;
    };

    struct renderpass_attachments {
        std::span<VkClearColorValue> clear_values{};
        std::span<VkAttachmentDescription> attachments{};
        std::span<VkSubpassDescription> subpass_descriptions{};
        std::span<VkSubpassDependency> dependencies{};
    };

    enum subpass_contents : uint32_t {
        inline_bit = VK_SUBPASS_CONTENTS_INLINE, // represents VK_SUBPASS_CONTENTS_INLINE
        secondary_command = VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, // represents VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
        inline_and_secondary_command_khr = VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR, // represents
                      // VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_KHR
                      // and
                      // VK_SUBPASS_CONTENTS_INLINE_AND_SECONDARY_COMMAND_BUFFERS_EXT
        max_enum_content = VK_SUBPASS_CONTENTS_MAX_ENUM // represents VK_SUBPASS_CONTENTS_MAX_ENUM
    };

    enum class image_layout : uint32_t {
        undefined = VK_IMAGE_LAYOUT_UNDEFINED,                                          // VK_IMAGE_LAYOUT_UNDEFINED
        general = VK_IMAGE_LAYOUT_GENERAL,                                              // VK_IMAGE_LAYOUT_GENERAL
        color_optimal = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,                       // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        depth_stencil_optimal = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,       // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        depth_stencil_read_only_optimal = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,     // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_READ_ONLY_OPTIMAL
        present_src_khr = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,                              // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        shader_read_only_optimal = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // enum class format : uint64_t {
    //     rgb32_sfloat, // Represent R32G32B32_SFLOAT
    //     rg32_sfloat,  // Represent R32G32_SFLOAT
    // };

    enum buffer : uint8_t {
        uniform = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,                        // represents VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        storage = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,                        // represents VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
        combined_image_sampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // represents VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        sampled_only_image = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE               // represents VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
    };

    enum image_usage : uint32_t {
        transfer_src_bit = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        transfer_dst_bit = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        sampled_bit = VK_IMAGE_USAGE_SAMPLED_BIT,
        storage_bit = VK_IMAGE_USAGE_STORAGE_BIT,
        color_attachment_bit = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        depth_stencil_bit = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        transient_attachment_bit = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        input_attachment_bit= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        video_decide_dst_bit_khr = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
        video_decode_src_bit_khr = VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
        video_decode_dpb_bit_khr = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
        fragment_density_map_bit_ext = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
        fragment_shading_rate_attachment_bit_khr = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        host_transfer_bit_ext = VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT,
        video_encode_dst_bit_khr = VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
        video_encode_src_bit_khr = VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
        video_encode_dpb_bit_khr = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
        attachment_feedback_loop_bit_ext = VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT,
        invocation_mask_bit_huawei = VK_IMAGE_USAGE_INVOCATION_MASK_BIT_HUAWEI,
        sample_weight_bit_qcom = VK_IMAGE_USAGE_SAMPLE_WEIGHT_BIT_QCOM,
        sample_block_mtch_bit_qcom = VK_IMAGE_USAGE_SAMPLE_BLOCK_MATCH_BIT_QCOM,
        shading_rate_image_bit_nv = VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV,
        max_enum_usage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM
    };

    enum sampler_address_mode : uint32_t {
        repeat = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        mirrored_repeat = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        clamp_to_border = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        mirror_clamp_to_edge = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
        mirror_clamp_to_edge_khr = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE_KHR,
        max_enum_address_mode = VK_SAMPLER_ADDRESS_MODE_MAX_ENUM
    };

    /**
     * @brief Refers to the input rate
     *
     * vertex - refers to most common rate. Indicates GPU to move next data
     * entry (next vertex data) into a buffer for every single vertex that's
     * processed.
     *       - Used for vertex attributes that change per vertex on the mesh
     *       - vertex attribute-only data
     *       - Per-object based specification in the next data entry
     *
     * instance - refers to data entry per-instance. Specifying to the GPU that
     *           the data entry in the buffer is to be after the instance of the
     * object itself.
     *         - Typically used for instanced rendering. Specfying next entry of
     * data to be after instanced drawn, could be shared. Therefore instance is
     * an option to choose from if vertex data is across as a per-instance
     * basis.
     *        - instance-based specification next data entry
     *
     */
    enum class input_rate : uint8_t {
        vertex,
        instance,
        max_enum,
    };

    //! @brief Equivalent to doing VkSampleCountFlagBits but simplified
    enum class sample_bit : uint32_t {
        count_1 = VK_SAMPLE_COUNT_1_BIT,
        count_2 = VK_SAMPLE_COUNT_2_BIT,
        count_4 = VK_SAMPLE_COUNT_4_BIT,
        count_8 = VK_SAMPLE_COUNT_8_BIT,
        count_16 = VK_SAMPLE_COUNT_16_BIT,
        count_32 = VK_SAMPLE_COUNT_32_BIT,
        count_64 = VK_SAMPLE_COUNT_64_BIT,
        max_enum = VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM
    };

    //! @brief Equivalent to VkAttachmentLoadOp
    enum class attachment_load : uint32_t {
        load = VK_ATTACHMENT_LOAD_OP_LOAD,  // LOAD_OP_LOAD
        clear = VK_ATTACHMENT_LOAD_OP_CLEAR,     // LOAD_OP_CLEAR
        dont_care = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // lOAD_OP_DONT_CARE
        none_khr = VK_ATTACHMENT_LOAD_OP_NONE_KHR,  // LOAD_OP_NONE_KHR
        none_ext = VK_ATTACHMENT_LOAD_OP_NONE_KHR,  // LOAD_OP_NONE_EXT
        max_enum = VK_ATTACHMENT_LOAD_OP_MAX_ENUM,  // LOAD_OP_MAX_ENUM
    };

    //! @brief Equivalent to VkAttachmentStoreOp
    enum class attachment_store : uint32_t {
        store = VK_ATTACHMENT_STORE_OP_STORE, // STORE_OP_STORE
        dont_care = VK_ATTACHMENT_STORE_OP_DONT_CARE, // STORE_OP_CLEAR
        none_khr = VK_ATTACHMENT_STORE_OP_NONE_KHR,  // STORE_OP_NONE
        none_qcom = VK_ATTACHMENT_STORE_OP_NONE_QCOM, // STORE_OP_NONE_EXT
        none_ext = VK_ATTACHMENT_STORE_OP_NONE_EXT,  // STORE_OP_NONE_KHR
        max_enum = VK_ATTACHMENT_STORE_OP_MAX_ENUM,  // STORE_OP_MAX_ENUM
    };

    enum class pipeline_stage_flags : uint32_t {
        top_of_pipe = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        draw_indirect = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        vertex_input_bit = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        vertex_shader_bit = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        tesselation_control_shader_bit = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        tessellation_evaluation_shader_bit = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
        geoemtry_shaders_bit = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
        fragment_shaders_bit = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        early_fragment_shader_bit = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        late_fragment_tests_bit = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        color_attachment_output = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        compute_shader_bit = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        transient_bit = VK_PIPELINE_STAGE_TRANSFER_BIT,
        bottom_of_pipe = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        host_bit = VK_PIPELINE_STAGE_HOST_BIT,
        all_graphics_bit = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        all_commands_bit = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        none = VK_PIPELINE_STAGE_NONE,
        transform_feedback_bit_ext = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
        conditional_rendering_bit_ext = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
        acceleration_build_bit_khr = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        ray_tracing_shader_bit_khr = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        fragment_density_process_bit_ext = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
        command_preprocess_bit_nv = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
        task_shader_bit_ext = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
        mesh_shader_bit_ext = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
        shading_rate_image_bit_nv = VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV,
        ray_tracing_shader_bit_nv = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV,
        task_shader_bit_nv = VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV,
        mesh_shader_bit_nv = VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV,
        none_khr = VK_PIPELINE_STAGE_NONE_KHR,
        flag_bits_max_enum = VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM
    };

    //! @brief Equivalent to VkPipelineBindPoint
    enum class pipeline_bind_point : uint64_t {
        graphics = VK_PIPELINE_BIND_POINT_GRAPHICS,                             // VK_PIPELINE_BIND_POINT_GRAPHICS
        compute = VK_PIPELINE_BIND_POINT_COMPUTE,                               // VK_PIPELINE_BIND_POINT_COMPUTE
        ray_tracing_khr = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,               // VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR
        subpass_shading_hauwei = VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI, // VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI
        ray_tracing_nv = VK_PIPELINE_BIND_POINT_RAY_TRACING_NV,                 // VK_PIPELINE_BIND_POINT_RAY_TRACING_NV
        max_enum = VK_PIPELINE_BIND_POINT_MAX_ENUM                              // VK_PIPELINE_BIND_POINT_MAX_ENUM
    };

    /**
     * @brief Specifies a specific attachment that a renderpass may operate
     * using
     *
     * Renderpasses can consist of multiple attachment that can be specified
     * under this one struct to represent both the VkAttachmentDescription and
     * VkAttachmentReference
     */
    struct attachment {
        VkFormat format;
        image_layout layout; // specify for VkAttachmentReference
        sample_bit samples;
        attachment_load load;
        attachment_store store;
        attachment_load stencil_load;
        attachment_store stencil_store;
        image_layout initial_layout;
        image_layout final_layout;
    };

    struct renderpass_begin_params {
        VkCommandBuffer current_command = nullptr;
        VkExtent2D extent;
        VkFramebuffer current_framebuffer = nullptr;
        std::array<float, 4> color;
        subpass_contents subpass;
    };

    //! @brief Bits that 1-to-1 map to VkCommandUsageFlags
    enum command_usage : uint32_t {
        one_time_submit = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // Represents VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        renderpass_continue_bit = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, // Represents VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
        simulatneous_use_bit = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // Represents VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
        max_bit = VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM // Represents VK_COMMAND_BUFFER_USAGE_FLAG_BITS_MAX_ENUM
    };

    /**
     * @brief memory_property is a representation of vulkan's
     * VkMemoryPropertyFlags.
     *
     * @param device_local_bit
     *
     * Meaning: indicates memory allocated with this type is most efficient for
     * the GPU to access. \n
     *
     * Implications: The memory with this bit typically
     * resides on the GPU's VRAM. Accessing memory directly from GPU's since its
     * faster. \n
     *
     * Usage: For resources that are primarily accessed by the GPU in the
     * case of textures, vertex buffers, and framebuffers. If a memory type has
     * this bit associated with it, the heap memory will also have be set along
     * with VK_MEMORY_HEAP_DEVICE_LOCAL_BIT. \n
     *
     * @param host_visible_bit
     *
     * Meaning: Indicates memory alloated can be mapped to host's (CPU) address
     * space using the vkMapMemory API. \n
     *
     * Implications: ALlows CPU to directly
     * read from and write to memory. Crucial for transferring data between CPU
     * to GPU. \n
     *
     * Usage: Use-case is for staging buffers, where data initially
     * uploaded from CPU before being copied to device-local memory or for
     * resourcfes that need frequent CPU updates. \n
     *
     * @param host_coherent_bit
     *
     * Meaning: Indicates host cache managemnet commands
     * (vkFlushMappedMemoryRanges  and vkInvalidateMappedMemoryRanges) are not
     * needed. Writes made by host will automatically become visible to the
     * device, and writes made by device will automatically be visible to the
     * host. \n
     *
     * Implications: Simplifies memory synchronization between CPU and GPU.
     * Though can lead to slower CPU access if it means bypassing the CPU caches
     * or involving more complex cache coherence protocols. \n
     *
     * Usage: Used with 'VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT' for easy data
     * transfers, especially for frequent updated data where manual flushing
     * would be cumbersome. \n
     *
     *
     * @param host_cached_bit
     *
     * Meaning: Indicates memory allocated with this type is cached on the host
     * (CPU). \n
     *
     * Implications: Host memory accesses (read/writes) to this memory
     * type will go through CPU cache heirarchy. Significantly improves
     * performance where random access patterns. If not set on `HOST_VISIBLE`
     * memory, CPU accesses are often uncached and write-combined, meanming
     * writes should be sequential and reads should be avoided for good
     * performance. \n
     *
     * Usage: Does well for CPU-side reading of data written to GPU (screenshots
     * or feedback data) and for CPU-side writing of data to be accessed
     * randomly. Flag usually implies explicit cache management
     * (flushing/invalidating) is required if `HOST_COHERENT_BIT` is not also
     * set. \n
     *
     *
     */
    enum memory_property : uint32_t {
        device_local_bit = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        host_visible_bit = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        host_coherent_bit = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        host_cached_bit = VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
        lazily_allocated_bit = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
        device_protected_bit = VK_MEMORY_PROPERTY_PROTECTED_BIT,
        device_coherent_bit_amd = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD,
        device_uncached_bit_amd = VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD,
        rdma_capable_bit_nv = VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV,
        flag_bits_max_enum = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM
    };

    enum class shader_stage {
        vertex = VK_SHADER_STAGE_VERTEX_BIT,
        fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        compute = VK_SHADER_STAGE_COMPUTE_BIT,
        geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
        all_graphics = VK_SHADER_STAGE_ALL_GRAPHICS,
        all = VK_SHADER_STAGE_ALL,
        raygen_khr = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        raygen_nv = VK_SHADER_STAGE_RAYGEN_BIT_NV,
        any_hit_kht = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        closest_hit_khr = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        miss_bit_khr = VK_SHADER_STAGE_MISS_BIT_KHR,
        intersection_khr = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        callable_bit_khr = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        task_bit_ext = VK_SHADER_STAGE_TASK_BIT_EXT,
        mesh_bit_ext = VK_SHADER_STAGE_MESH_BIT_EXT,
        supass_shading_huawei = VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI,
        undefined
    };

    enum class descriptor_layout_flag {
        update_after_bind_pool =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, // represents
                                                                      // VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT
        push_descriptor_khr =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR, // represents
                                                                   // VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR
        descriptor_buffer_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, // represents
                                                                     // VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT
        embedded_immutable_samplers_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT, // represents VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT
        indirect_bindable_bit_nv =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV, // represents
                                                                    // VK_DESCRIPTOR_SET_LAYOUT_CREATE_INDIRECT_BINDABLE_BIT_NV
        host_only_pool_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT, // represents
                                                                  // VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT
        per_stage_bit_nv =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV, // represents
                                                            // VK_DESCRIPTOR_SET_LAYOUT_CREATE_PER_STAGE_BIT_NV
        update_after_bind_pool_bit_ext =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT, // represents
                                                                          // VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT
        host_only_pool_bit_valve =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE, // represents
                                                                    // VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_VALVE
        flag_bits_max_enum =
          VK_DESCRIPTOR_SET_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM // represents
                                                             // VK_DESCRIPTOR_SET_LAYOUT_CREATE_FLAG_BITS_MAX_ENUM
    };

    //! @brief high-level specification for a shader source
    struct shader_source {
        std::string filename;
        shader_stage stage = shader_stage::undefined;
    };

    //! @brief Represent the vulkan shader module that will get utilized by
    //! VkPipeline
    struct shader_handle {
        VkShaderModule module = nullptr;
        shader_stage stage = shader_stage::undefined;
    };

    struct vertex_attribute_entry {
        uint32_t location;
        format format;
        uint32_t stride;
    };

    struct vertex_attribute {
        uint32_t binding;
        std::span<vertex_attribute_entry> entries;
        uint32_t stride;
        input_rate input_rate;
    };

    struct vertex_input {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normals;
        glm::vec2 uv;

        bool operator==(const vertex_input& other) const {
            return position == other.position and color == other.color and
                   uv == other.uv and normals == other.normals;
        }
    };

    //! @brief struct for copying from staging buffer to a destination
    struct buffer_copy_info {
        VkBuffer src;
        VkBuffer dst;
    };

    struct vertex_params {
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        std::span<vertex_input> vertices;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct index_params {
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        std::span<uint32_t> indices;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct uniform_params {
        // VkPhysicalDevice physical_handle=nullptr;
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        uint32_t size_bytes = 0;
        std::string debug_name;
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    struct descriptor_binding_point {
        uint32_t binding;
        shader_stage stage;
    };

    struct descriptor_entry {
        buffer type;
        descriptor_binding_point binding_point;
        uint32_t descriptor_count;
    };

    struct write_image {
        VkSampler sampler=nullptr;
        VkImageView view=nullptr;
        // VkImageLayout image_layout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_layout layout;
    };
    struct write_buffer {
        VkBuffer buffer=nullptr;
        uint32_t offset=0;
        uint32_t range=0;
    };

    struct write_buffer_descriptor {
        uint32_t dst_binding;
        std::span<const write_buffer> uniforms;
    };

    struct write_image_descriptor {
        uint32_t dst_binding;
        std::span<const write_image> sample_images;
    };

    struct image_extent {
        uint32_t width = 1;
        uint32_t height = 1;
    };

    struct image_params {
        image_extent extent;
        VkFormat format;
        memory_property property = memory_property::device_local_bit;
        image_aspect_flags aspect = image_aspect_flags::color_bit;
        // VkImageUsageFlags usage;
        uint32_t usage;
        VkImageCreateFlags image_flags = 0;
        VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D;
        uint32_t mip_levels = 1;
        uint32_t layer_count = 1;
        uint32_t array_layers = 1;
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        filter_range range{ .min = VK_FILTER_LINEAR, .max = VK_FILTER_LINEAR, };
        // VkSamplerAddressMode addrses_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        // VkSamplerAddressMode addrses_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        // VkSamplerAddressMode addrses_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        uint32_t addrses_mode_u = sampler_address_mode::repeat;
        uint32_t addrses_mode_v = sampler_address_mode::repeat;
        uint32_t addrses_mode_w = sampler_address_mode::repeat;
    };

    struct buffer_parameters {
        VkDeviceSize device_size = 0;
        VkPhysicalDeviceMemoryProperties physical_memory_properties;
        memory_property property_flags;
        VkBufferUsageFlags usage;
        VkSharingMode share_mode = VK_SHARING_MODE_EXCLUSIVE;
        const char* debug_name="NA";
        PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    };

    // Used by vk::copy(const VkCommandBuffer& p_current,  )
    struct copy_info {
        uint32_t width;
        uint32_t height;
        uint32_t array_layers = 1;
    };

    struct image_barrier_info {
        VkFormat format;
        VkImageLayout old_layout;
        VkImageLayout new_layout;
        uint32_t level_count = 1;
        VkImageAspectFlagBits aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        uint32_t base_array_count = 0;
        uint32_t layer_count = 1;
    };

    struct write_info {
        uint64_t offset = 0;
        uint64_t size_bytes = 0;
    };

};