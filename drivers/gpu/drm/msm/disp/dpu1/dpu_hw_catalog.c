// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2015-2018, The Linux Foundation. All rights reserved.
 */

#define pr_fmt(fmt)	"[drm:%s:%d] " fmt, __func__, __LINE__
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include "dpu_hw_mdss.h"
#include "dpu_hw_catalog.h"
#include "dpu_kms.h"

#define VIG_MASK \
	(BIT(DPU_SSPP_SRC) | BIT(DPU_SSPP_QOS) |\
	BIT(DPU_SSPP_CSC_10BIT) | BIT(DPU_SSPP_CDP) |\
	BIT(DPU_SSPP_TS_PREFILL) | BIT(DPU_SSPP_EXCL_RECT))

#define VIG_MSM8998_MASK \
	(VIG_MASK | BIT(DPU_SSPP_SCALER_QSEED3))

#define VIG_SDM845_MASK \
	(VIG_MASK | BIT(DPU_SSPP_QOS_8LVL) | BIT(DPU_SSPP_SCALER_QSEED3))

#define VIG_SC7180_MASK \
	(VIG_MASK | BIT(DPU_SSPP_QOS_8LVL) | BIT(DPU_SSPP_SCALER_QSEED4))

#define VIG_SM8250_MASK \
	(VIG_MASK | BIT(DPU_SSPP_QOS_8LVL) | BIT(DPU_SSPP_SCALER_QSEED3LITE))

#define DMA_MSM8998_MASK \
	(BIT(DPU_SSPP_SRC) | BIT(DPU_SSPP_QOS) |\
	BIT(DPU_SSPP_TS_PREFILL) | BIT(DPU_SSPP_TS_PREFILL_REC1) |\
	BIT(DPU_SSPP_CDP) | BIT(DPU_SSPP_EXCL_RECT))

#define DMA_SDM845_MASK \
	(BIT(DPU_SSPP_SRC) | BIT(DPU_SSPP_QOS) | BIT(DPU_SSPP_QOS_8LVL) |\
	BIT(DPU_SSPP_TS_PREFILL) | BIT(DPU_SSPP_TS_PREFILL_REC1) |\
	BIT(DPU_SSPP_CDP) | BIT(DPU_SSPP_EXCL_RECT))

#define DMA_CURSOR_MSM8998_MASK \
	(DMA_MSM8998_MASK | BIT(DPU_SSPP_CURSOR))

#define DMA_CURSOR_SDM845_MASK \
	(DMA_SDM845_MASK | BIT(DPU_SSPP_CURSOR))

#define MIXER_SDM845_MASK \
	(BIT(DPU_MIXER_SOURCESPLIT) | BIT(DPU_DIM_LAYER))

#define MIXER_SC7180_MASK \
	(BIT(DPU_DIM_LAYER))

#define PINGPONG_SDM845_MASK BIT(DPU_PINGPONG_DITHER)

#define PINGPONG_SDM845_SPLIT_MASK \
	(PINGPONG_SDM845_MASK | BIT(DPU_PINGPONG_TE2))

#define MERGE_3D_SM8150_MASK (0)

#define DSPP_MSM8998_MASK BIT(DPU_DSPP_PCC) | BIT(DPU_DSPP_GC)

#define DSPP_SC7180_MASK BIT(DPU_DSPP_PCC)

#define INTF_SDM845_MASK (0)

#define INTF_SC7180_MASK BIT(DPU_INTF_INPUT_CTRL) | BIT(DPU_INTF_TE)

#define DEFAULT_PIXEL_RAM_SIZE		(50 * 1024)
#define DEFAULT_DPU_LINE_WIDTH		2048
#define DEFAULT_DPU_OUTPUT_LINE_WIDTH	2560

#define MAX_HORZ_DECIMATION	4
#define MAX_VERT_DECIMATION	4

#define MAX_UPSCALE_RATIO	20
#define MAX_DOWNSCALE_RATIO	4
#define SSPP_UNITY_SCALE	1

#define STRCAT(X, Y) (X Y)

static const uint32_t plane_formats[] = {
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_RGBX8888,
	DRM_FORMAT_BGRX8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	DRM_FORMAT_ARGB1555,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_BGRA5551,
	DRM_FORMAT_XRGB1555,
	DRM_FORMAT_XBGR1555,
	DRM_FORMAT_RGBX5551,
	DRM_FORMAT_BGRX5551,
	DRM_FORMAT_ARGB4444,
	DRM_FORMAT_ABGR4444,
	DRM_FORMAT_RGBA4444,
	DRM_FORMAT_BGRA4444,
	DRM_FORMAT_XRGB4444,
	DRM_FORMAT_XBGR4444,
	DRM_FORMAT_RGBX4444,
	DRM_FORMAT_BGRX4444,
};

static const uint32_t plane_formats_yuv[] = {
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRX8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBX8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	DRM_FORMAT_ARGB1555,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_BGRA5551,
	DRM_FORMAT_XRGB1555,
	DRM_FORMAT_XBGR1555,
	DRM_FORMAT_RGBX5551,
	DRM_FORMAT_BGRX5551,
	DRM_FORMAT_ARGB4444,
	DRM_FORMAT_ABGR4444,
	DRM_FORMAT_RGBA4444,
	DRM_FORMAT_BGRA4444,
	DRM_FORMAT_XRGB4444,
	DRM_FORMAT_XBGR4444,
	DRM_FORMAT_RGBX4444,
	DRM_FORMAT_BGRX4444,

	DRM_FORMAT_NV12,
	DRM_FORMAT_NV21,
	DRM_FORMAT_NV16,
	DRM_FORMAT_NV61,
	DRM_FORMAT_VYUY,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_YUV420,
	DRM_FORMAT_YVU420,
};

/*************************************************************
 * DPU sub blocks config
 *************************************************************/
/* DPU top level caps */
static const struct dpu_caps msm8998_dpu_caps = {
	.max_mixer_width = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.max_mixer_blendstages = 0x7,
	.qseed_type = DPU_SSPP_SCALER_QSEED3,
	.smart_dma_rev = DPU_SSPP_SMART_DMA_V1,
	.ubwc_version = DPU_HW_UBWC_VER_10,
	.has_src_split = true,
	.has_dim_layer = true,
	.has_idle_pc = true,
	.has_3d_merge = true,
	.max_linewidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.pixel_ram_size = DEFAULT_PIXEL_RAM_SIZE,
	.max_hdeci_exp = MAX_HORZ_DECIMATION,
	.max_vdeci_exp = MAX_VERT_DECIMATION,
};

static const struct dpu_caps sdm845_dpu_caps = {
	.max_mixer_width = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.max_mixer_blendstages = 0xb,
	.qseed_type = DPU_SSPP_SCALER_QSEED3,
	.smart_dma_rev = DPU_SSPP_SMART_DMA_V2,
	.ubwc_version = DPU_HW_UBWC_VER_20,
	.has_src_split = true,
	.has_dim_layer = true,
	.has_idle_pc = true,
	.has_3d_merge = true,
	.max_linewidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.pixel_ram_size = DEFAULT_PIXEL_RAM_SIZE,
	.max_hdeci_exp = MAX_HORZ_DECIMATION,
	.max_vdeci_exp = MAX_VERT_DECIMATION,
};

static const struct dpu_caps sc7180_dpu_caps = {
	.max_mixer_width = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.max_mixer_blendstages = 0x9,
	.qseed_type = DPU_SSPP_SCALER_QSEED4,
	.smart_dma_rev = DPU_SSPP_SMART_DMA_V2,
	.ubwc_version = DPU_HW_UBWC_VER_20,
	.has_dim_layer = true,
	.has_idle_pc = true,
	.max_linewidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.pixel_ram_size = DEFAULT_PIXEL_RAM_SIZE,
};

static const struct dpu_caps sm8150_dpu_caps = {
	.max_mixer_width = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.max_mixer_blendstages = 0xb,
	.qseed_type = DPU_SSPP_SCALER_QSEED3,
	.smart_dma_rev = DPU_SSPP_SMART_DMA_V2, /* TODO: v2.5 */
	.ubwc_version = DPU_HW_UBWC_VER_30,
	.has_src_split = true,
	.has_dim_layer = true,
	.has_idle_pc = true,
	.has_3d_merge = true,
	.max_linewidth = 4096,
	.pixel_ram_size = DEFAULT_PIXEL_RAM_SIZE,
	.max_hdeci_exp = MAX_HORZ_DECIMATION,
	.max_vdeci_exp = MAX_VERT_DECIMATION,
};

static const struct dpu_caps sm8250_dpu_caps = {
	.max_mixer_width = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.max_mixer_blendstages = 0xb,
	.qseed_type = DPU_SSPP_SCALER_QSEED3LITE,
	.smart_dma_rev = DPU_SSPP_SMART_DMA_V2, /* TODO: v2.5 */
	.ubwc_version = DPU_HW_UBWC_VER_40,
	.has_src_split = true,
	.has_dim_layer = true,
	.has_idle_pc = true,
	.has_3d_merge = true,
	.max_linewidth = 4096,
	.pixel_ram_size = DEFAULT_PIXEL_RAM_SIZE,
};

static const struct dpu_mdp_cfg msm8998_mdp[] = {
	{
	.name = "top_0", .id = MDP_TOP,
	.base = 0x0, .len = 0x458,
	.features = 0,
	.highest_bank_bit = 0x2,
	.clk_ctrls[DPU_CLK_CTRL_VIG0] = {
			.reg_off = 0x2AC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG1] = {
			.reg_off = 0x2B4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG2] = {
			.reg_off = 0x2BC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG3] = {
			.reg_off = 0x2C4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_DMA0] = {
			.reg_off = 0x2AC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_DMA1] = {
			.reg_off = 0x2B4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_DMA2] = {
			.reg_off = 0x2C4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_DMA3] = {
			.reg_off = 0x2C4, .bit_off = 12},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR0] = {
			.reg_off = 0x3A8, .bit_off = 15},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR1] = {
			.reg_off = 0x3B0, .bit_off = 15},
	},
};

static const struct dpu_mdp_cfg sdm845_mdp[] = {
	{
	.name = "top_0", .id = MDP_TOP,
	.base = 0x0, .len = 0x45C,
	.features = 0,
	.highest_bank_bit = 0x2,
	.clk_ctrls[DPU_CLK_CTRL_VIG0] = {
			.reg_off = 0x2AC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG1] = {
			.reg_off = 0x2B4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG2] = {
			.reg_off = 0x2BC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG3] = {
			.reg_off = 0x2C4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_DMA0] = {
			.reg_off = 0x2AC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_DMA1] = {
			.reg_off = 0x2B4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR0] = {
			.reg_off = 0x2BC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR1] = {
			.reg_off = 0x2C4, .bit_off = 8},
	},
};

static const struct dpu_mdp_cfg sc7180_mdp[] = {
	{
	.name = "top_0", .id = MDP_TOP,
	.base = 0x0, .len = 0x494,
	.features = 0,
	.highest_bank_bit = 0x3,
	.clk_ctrls[DPU_CLK_CTRL_VIG0] = {
		.reg_off = 0x2AC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_DMA0] = {
		.reg_off = 0x2AC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR0] = {
		.reg_off = 0x2B4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR1] = {
		.reg_off = 0x2C4, .bit_off = 8},
	},
};

static const struct dpu_mdp_cfg sm8250_mdp[] = {
	{
	.name = "top_0", .id = MDP_TOP,
	.base = 0x0, .len = 0x45C,
	.features = 0,
	.highest_bank_bit = 0x3, /* TODO: 2 for LP_DDR4 */
	.clk_ctrls[DPU_CLK_CTRL_VIG0] = {
			.reg_off = 0x2AC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG1] = {
			.reg_off = 0x2B4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG2] = {
			.reg_off = 0x2BC, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_VIG3] = {
			.reg_off = 0x2C4, .bit_off = 0},
	.clk_ctrls[DPU_CLK_CTRL_DMA0] = {
			.reg_off = 0x2AC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_DMA1] = {
			.reg_off = 0x2B4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR0] = {
			.reg_off = 0x2BC, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_CURSOR1] = {
			.reg_off = 0x2C4, .bit_off = 8},
	.clk_ctrls[DPU_CLK_CTRL_REG_DMA] = {
			.reg_off = 0x2BC, .bit_off = 20},
	},
};

/*************************************************************
 * CTL sub blocks config
 *************************************************************/
static const struct dpu_ctl_cfg msm8998_ctl[] = {
	{
	.name = "ctl_0", .id = CTL_0,
	.base = 0x1000, .len = 0x94,
	.features = BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_1", .id = CTL_1,
	.base = 0x1200, .len = 0x94,
	.features = 0
	},
	{
	.name = "ctl_2", .id = CTL_2,
	.base = 0x1400, .len = 0x94,
	.features = BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_3", .id = CTL_3,
	.base = 0x1600, .len = 0x94,
	.features = 0
	},
	{
	.name = "ctl_4", .id = CTL_4,
	.base = 0x1800, .len = 0x94,
	.features = 0
	},
};

static const struct dpu_ctl_cfg sdm845_ctl[] = {
	{
	.name = "ctl_0", .id = CTL_0,
	.base = 0x1000, .len = 0xE4,
	.features = BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_1", .id = CTL_1,
	.base = 0x1200, .len = 0xE4,
	.features = BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_2", .id = CTL_2,
	.base = 0x1400, .len = 0xE4,
	.features = 0
	},
	{
	.name = "ctl_3", .id = CTL_3,
	.base = 0x1600, .len = 0xE4,
	.features = 0
	},
	{
	.name = "ctl_4", .id = CTL_4,
	.base = 0x1800, .len = 0xE4,
	.features = 0
	},
};

static const struct dpu_ctl_cfg sc7180_ctl[] = {
	{
	.name = "ctl_0", .id = CTL_0,
	.base = 0x1000, .len = 0xE4,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
	{
	.name = "ctl_1", .id = CTL_1,
	.base = 0x1200, .len = 0xE4,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
	{
	.name = "ctl_2", .id = CTL_2,
	.base = 0x1400, .len = 0xE4,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
};

static const struct dpu_ctl_cfg sm8150_ctl[] = {
	{
	.name = "ctl_0", .id = CTL_0,
	.base = 0x1000, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG) | BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_1", .id = CTL_1,
	.base = 0x1200, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG) | BIT(DPU_CTL_SPLIT_DISPLAY)
	},
	{
	.name = "ctl_2", .id = CTL_2,
	.base = 0x1400, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
	{
	.name = "ctl_3", .id = CTL_3,
	.base = 0x1600, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
	{
	.name = "ctl_4", .id = CTL_4,
	.base = 0x1800, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
	{
	.name = "ctl_5", .id = CTL_5,
	.base = 0x1a00, .len = 0x1e0,
	.features = BIT(DPU_CTL_ACTIVE_CFG)
	},
};

/*************************************************************
 * SSPP sub blocks config
 *************************************************************/

/* SSPP common configuration */

#define _VIG_SBLK(num, sdma_pri, qseed_ver) \
	{ \
	.maxdwnscale = MAX_DOWNSCALE_RATIO, \
	.maxupscale = MAX_UPSCALE_RATIO, \
	.smart_dma_priority = sdma_pri, \
	.src_blk = {.name = STRCAT("sspp_src_", num), \
		.id = DPU_SSPP_SRC, .base = 0x00, .len = 0x150,}, \
	.scaler_blk = {.name = STRCAT("sspp_scaler", num), \
		.id = qseed_ver, \
		.base = 0xa00, .len = 0xa0,}, \
	.csc_blk = {.name = STRCAT("sspp_csc", num), \
		.id = DPU_SSPP_CSC_10BIT, \
		.base = 0x1a00, .len = 0x100,}, \
	.format_list = plane_formats_yuv, \
	.num_formats = ARRAY_SIZE(plane_formats_yuv), \
	.virt_format_list = plane_formats, \
	.virt_num_formats = ARRAY_SIZE(plane_formats), \
	}

#define _DMA_SBLK(num, sdma_pri) \
	{ \
	.maxdwnscale = SSPP_UNITY_SCALE, \
	.maxupscale = SSPP_UNITY_SCALE, \
	.smart_dma_priority = sdma_pri, \
	.src_blk = {.name = STRCAT("sspp_src_", num), \
		.id = DPU_SSPP_SRC, .base = 0x00, .len = 0x150,}, \
	.format_list = plane_formats, \
	.num_formats = ARRAY_SIZE(plane_formats), \
	.virt_format_list = plane_formats, \
	.virt_num_formats = ARRAY_SIZE(plane_formats), \
	}

static const struct dpu_sspp_sub_blks msm8998_vig_sblk_0 =
				_VIG_SBLK("0", 0, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks msm8998_vig_sblk_1 =
				_VIG_SBLK("1", 0, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks msm8998_vig_sblk_2 =
				_VIG_SBLK("2", 0, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks msm8998_vig_sblk_3 =
				_VIG_SBLK("3", 0, DPU_SSPP_SCALER_QSEED3);

static const struct dpu_sspp_sub_blks sdm845_vig_sblk_0 =
				_VIG_SBLK("0", 5, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks sdm845_vig_sblk_1 =
				_VIG_SBLK("1", 6, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks sdm845_vig_sblk_2 =
				_VIG_SBLK("2", 7, DPU_SSPP_SCALER_QSEED3);
static const struct dpu_sspp_sub_blks sdm845_vig_sblk_3 =
				_VIG_SBLK("3", 8, DPU_SSPP_SCALER_QSEED3);

static const struct dpu_sspp_sub_blks sdm845_dma_sblk_0 = _DMA_SBLK("8", 1);
static const struct dpu_sspp_sub_blks sdm845_dma_sblk_1 = _DMA_SBLK("9", 2);
static const struct dpu_sspp_sub_blks sdm845_dma_sblk_2 = _DMA_SBLK("10", 3);
static const struct dpu_sspp_sub_blks sdm845_dma_sblk_3 = _DMA_SBLK("11", 4);

#define SSPP_BLK(_name, _id, _base, _features, \
		_sblk, _xinid, _type, _clkctrl) \
	{ \
	.name = _name, .id = _id, \
	.base = _base, .len = 0x1c8, \
	.features = _features, \
	.sblk = &_sblk, \
	.xin_id = _xinid, \
	.type = _type, \
	.clk_ctrl = _clkctrl \
	}

static const struct dpu_sspp_cfg msm8998_sspp[] = {
	SSPP_BLK("sspp_0", SSPP_VIG0, 0x4000, VIG_MSM8998_MASK,
		msm8998_vig_sblk_0, 0,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG0),
	SSPP_BLK("sspp_1", SSPP_VIG1, 0x6000, VIG_MSM8998_MASK,
		msm8998_vig_sblk_1, 4,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG1),
	SSPP_BLK("sspp_2", SSPP_VIG2, 0x8000, VIG_MSM8998_MASK,
		msm8998_vig_sblk_2, 8, SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG2),
	SSPP_BLK("sspp_3", SSPP_VIG3, 0xa000, VIG_MSM8998_MASK,
		msm8998_vig_sblk_3, 12,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG3),
	SSPP_BLK("sspp_8", SSPP_DMA0, 0x24000,  DMA_MSM8998_MASK,
		sdm845_dma_sblk_0, 1, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA0),
	SSPP_BLK("sspp_9", SSPP_DMA1, 0x26000,  DMA_MSM8998_MASK,
		sdm845_dma_sblk_1, 5, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA1),
	SSPP_BLK("sspp_10", SSPP_DMA2, 0x28000,  DMA_CURSOR_MSM8998_MASK,
		sdm845_dma_sblk_2, 9, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR0),
	SSPP_BLK("sspp_11", SSPP_DMA3, 0x2a000,  DMA_CURSOR_MSM8998_MASK,
		sdm845_dma_sblk_3, 13, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR1),
};

static const struct dpu_sspp_cfg sdm845_sspp[] = {
	SSPP_BLK("sspp_0", SSPP_VIG0, 0x4000, VIG_SDM845_MASK,
		sdm845_vig_sblk_0, 0,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG0),
	SSPP_BLK("sspp_1", SSPP_VIG1, 0x6000, VIG_SDM845_MASK,
		sdm845_vig_sblk_1, 4,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG1),
	SSPP_BLK("sspp_2", SSPP_VIG2, 0x8000, VIG_SDM845_MASK,
		sdm845_vig_sblk_2, 8, SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG2),
	SSPP_BLK("sspp_3", SSPP_VIG3, 0xa000, VIG_SDM845_MASK,
		sdm845_vig_sblk_3, 12,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG3),
	SSPP_BLK("sspp_8", SSPP_DMA0, 0x24000,  DMA_SDM845_MASK,
		sdm845_dma_sblk_0, 1, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA0),
	SSPP_BLK("sspp_9", SSPP_DMA1, 0x26000,  DMA_SDM845_MASK,
		sdm845_dma_sblk_1, 5, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA1),
	SSPP_BLK("sspp_10", SSPP_DMA2, 0x28000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_2, 9, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR0),
	SSPP_BLK("sspp_11", SSPP_DMA3, 0x2a000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_3, 13, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR1),
};

static const struct dpu_sspp_sub_blks sc7180_vig_sblk_0 =
				_VIG_SBLK("0", 4, DPU_SSPP_SCALER_QSEED4);

static const struct dpu_sspp_cfg sc7180_sspp[] = {
	SSPP_BLK("sspp_0", SSPP_VIG0, 0x4000, VIG_SC7180_MASK,
		sc7180_vig_sblk_0, 0,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG0),
	SSPP_BLK("sspp_8", SSPP_DMA0, 0x24000,  DMA_SDM845_MASK,
		sdm845_dma_sblk_0, 1, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA0),
	SSPP_BLK("sspp_9", SSPP_DMA1, 0x26000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_1, 5, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR0),
	SSPP_BLK("sspp_10", SSPP_DMA2, 0x28000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_2, 9, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR1),
};

static const struct dpu_sspp_sub_blks sm8250_vig_sblk_0 =
				_VIG_SBLK("0", 5, DPU_SSPP_SCALER_QSEED3LITE);
static const struct dpu_sspp_sub_blks sm8250_vig_sblk_1 =
				_VIG_SBLK("1", 6, DPU_SSPP_SCALER_QSEED3LITE);
static const struct dpu_sspp_sub_blks sm8250_vig_sblk_2 =
				_VIG_SBLK("2", 7, DPU_SSPP_SCALER_QSEED3LITE);
static const struct dpu_sspp_sub_blks sm8250_vig_sblk_3 =
				_VIG_SBLK("3", 8, DPU_SSPP_SCALER_QSEED3LITE);

static const struct dpu_sspp_cfg sm8250_sspp[] = {
	SSPP_BLK("sspp_0", SSPP_VIG0, 0x4000, VIG_SM8250_MASK,
		sm8250_vig_sblk_0, 0,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG0),
	SSPP_BLK("sspp_1", SSPP_VIG1, 0x6000, VIG_SM8250_MASK,
		sm8250_vig_sblk_1, 4,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG1),
	SSPP_BLK("sspp_2", SSPP_VIG2, 0x8000, VIG_SM8250_MASK,
		sm8250_vig_sblk_2, 8, SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG2),
	SSPP_BLK("sspp_3", SSPP_VIG3, 0xa000, VIG_SM8250_MASK,
		sm8250_vig_sblk_3, 12,  SSPP_TYPE_VIG, DPU_CLK_CTRL_VIG3),
	SSPP_BLK("sspp_8", SSPP_DMA0, 0x24000,  DMA_SDM845_MASK,
		sdm845_dma_sblk_0, 1, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA0),
	SSPP_BLK("sspp_9", SSPP_DMA1, 0x26000,  DMA_SDM845_MASK,
		sdm845_dma_sblk_1, 5, SSPP_TYPE_DMA, DPU_CLK_CTRL_DMA1),
	SSPP_BLK("sspp_10", SSPP_DMA2, 0x28000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_2, 9, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR0),
	SSPP_BLK("sspp_11", SSPP_DMA3, 0x2a000,  DMA_CURSOR_SDM845_MASK,
		sdm845_dma_sblk_3, 13, SSPP_TYPE_DMA, DPU_CLK_CTRL_CURSOR1),
};

/*************************************************************
 * MIXER sub blocks config
 *************************************************************/

#define LM_BLK(_name, _id, _base, _fmask, _sblk, _pp, _lmpair, _dspp) \
	{ \
	.name = _name, .id = _id, \
	.base = _base, .len = 0x320, \
	.features = _fmask, \
	.sblk = _sblk, \
	.pingpong = _pp, \
	.lm_pair_mask = (1 << _lmpair), \
	.dspp = _dspp \
	}

/* MSM8998 */

static const struct dpu_lm_sub_blks msm8998_lm_sblk = {
	.maxwidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.maxblendstages = 7, /* excluding base layer */
	.blendstage_base = { /* offsets relative to mixer base */
		0x20, 0x50, 0x80, 0xb0, 0x230,
		0x260, 0x290
	},
};

static const struct dpu_lm_cfg msm8998_lm[] = {
	LM_BLK("lm_0", LM_0, 0x44000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_0, LM_2, DSPP_0),
	LM_BLK("lm_1", LM_1, 0x45000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_1, LM_5, DSPP_1),
	LM_BLK("lm_2", LM_2, 0x46000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_2, LM_0, 0),
	LM_BLK("lm_3", LM_3, 0x47000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_MAX, 0, 0),
	LM_BLK("lm_4", LM_4, 0x48000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_MAX, 0, 0),
	LM_BLK("lm_5", LM_5, 0x49000, MIXER_SDM845_MASK,
		&msm8998_lm_sblk, PINGPONG_3, LM_1, 0),
};

/* SDM845 */

static const struct dpu_lm_sub_blks sdm845_lm_sblk = {
	.maxwidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.maxblendstages = 11, /* excluding base layer */
	.blendstage_base = { /* offsets relative to mixer base */
		0x20, 0x38, 0x50, 0x68, 0x80, 0x98,
		0xb0, 0xc8, 0xe0, 0xf8, 0x110
	},
};

static const struct dpu_lm_cfg sdm845_lm[] = {
	LM_BLK("lm_0", LM_0, 0x44000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_0, LM_1, 0),
	LM_BLK("lm_1", LM_1, 0x45000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_1, LM_0, 0),
	LM_BLK("lm_2", LM_2, 0x46000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_2, LM_5, 0),
	LM_BLK("lm_3", LM_3, 0x0, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_MAX, 0, 0),
	LM_BLK("lm_4", LM_4, 0x0, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_MAX, 0, 0),
	LM_BLK("lm_5", LM_5, 0x49000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_3, LM_2, 0),
};

/* SC7180 */

static const struct dpu_lm_sub_blks sc7180_lm_sblk = {
	.maxwidth = DEFAULT_DPU_OUTPUT_LINE_WIDTH,
	.maxblendstages = 7, /* excluding base layer */
	.blendstage_base = { /* offsets relative to mixer base */
		0x20, 0x38, 0x50, 0x68, 0x80, 0x98, 0xb0
	},
};

static const struct dpu_lm_cfg sc7180_lm[] = {
	LM_BLK("lm_0", LM_0, 0x44000, MIXER_SC7180_MASK,
		&sc7180_lm_sblk, PINGPONG_0, LM_1, DSPP_0),
	LM_BLK("lm_1", LM_1, 0x45000, MIXER_SC7180_MASK,
		&sc7180_lm_sblk, PINGPONG_1, LM_0, 0),
};

/* SM8150 */

static const struct dpu_lm_cfg sm8150_lm[] = {
	LM_BLK("lm_0", LM_0, 0x44000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_0, LM_1, DSPP_0),
	LM_BLK("lm_1", LM_1, 0x45000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_1, LM_0, DSPP_1),
	LM_BLK("lm_2", LM_2, 0x46000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_2, LM_3, 0),
	LM_BLK("lm_3", LM_3, 0x47000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_3, LM_2, 0),
	LM_BLK("lm_4", LM_4, 0x48000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_4, LM_5, 0),
	LM_BLK("lm_5", LM_5, 0x49000, MIXER_SDM845_MASK,
		&sdm845_lm_sblk, PINGPONG_5, LM_4, 0),
};

/*************************************************************
 * DSPP sub blocks config
 *************************************************************/
static const struct dpu_dspp_sub_blks msm8998_dspp_sblk = {
	.pcc = {.id = DPU_DSPP_PCC, .base = 0x1700,
		.len = 0x90, .version = 0x10007},
	.gc = { .id = DPU_DSPP_GC, .base = 0x17c0,
		.len = 0x90, .version = 0x10007},
};

static const struct dpu_dspp_sub_blks sc7180_dspp_sblk = {
	.pcc = {.id = DPU_DSPP_PCC, .base = 0x1700,
		.len = 0x90, .version = 0x10000},
};

static const struct dpu_dspp_sub_blks sm8150_dspp_sblk = {
	.pcc = {.id = DPU_DSPP_PCC, .base = 0x1700,
		.len = 0x90, .version = 0x40000},
};

#define DSPP_BLK(_name, _id, _base, _mask, _sblk) \
		{\
		.name = _name, .id = _id, \
		.base = _base, .len = 0x1800, \
		.features = _mask, \
		.sblk = _sblk \
		}

static const struct dpu_dspp_cfg msm8998_dspp[] = {
	DSPP_BLK("dspp_0", DSPP_0, 0x54000, DSPP_MSM8998_MASK,
		 &msm8998_dspp_sblk),
	DSPP_BLK("dspp_1", DSPP_1, 0x56000, DSPP_MSM8998_MASK,
		 &msm8998_dspp_sblk),
};

static const struct dpu_dspp_cfg sc7180_dspp[] = {
	DSPP_BLK("dspp_0", DSPP_0, 0x54000, DSPP_SC7180_MASK,
		 &sc7180_dspp_sblk),
};

static const struct dpu_dspp_cfg sm8150_dspp[] = {
	DSPP_BLK("dspp_0", DSPP_0, 0x54000, DSPP_SC7180_MASK,
		 &sm8150_dspp_sblk),
	DSPP_BLK("dspp_1", DSPP_1, 0x56000, DSPP_SC7180_MASK,
		 &sm8150_dspp_sblk),
	DSPP_BLK("dspp_2", DSPP_2, 0x58000, DSPP_SC7180_MASK,
		 &sm8150_dspp_sblk),
	DSPP_BLK("dspp_3", DSPP_3, 0x5a000, DSPP_SC7180_MASK,
		 &sm8150_dspp_sblk),
};

/*************************************************************
 * PINGPONG sub blocks config
 *************************************************************/
static const struct dpu_pingpong_sub_blks sdm845_pp_sblk_te = {
	.te2 = {.id = DPU_PINGPONG_TE2, .base = 0x2000, .len = 0x0,
		.version = 0x1},
	.dither = {.id = DPU_PINGPONG_DITHER, .base = 0x30e0,
		.len = 0x20, .version = 0x10000},
};

static const struct dpu_pingpong_sub_blks sdm845_pp_sblk = {
	.dither = {.id = DPU_PINGPONG_DITHER, .base = 0x30e0,
		.len = 0x20, .version = 0x10000},
};

#define PP_BLK_TE(_name, _id, _base, _merge_3d) \
	{\
	.name = _name, .id = _id, \
	.base = _base, .len = 0xd4, \
	.features = PINGPONG_SDM845_SPLIT_MASK, \
	.merge_3d = _merge_3d, \
	.sblk = &sdm845_pp_sblk_te \
	}
#define PP_BLK(_name, _id, _base, _merge_3d) \
	{\
	.name = _name, .id = _id, \
	.base = _base, .len = 0xd4, \
	.features = PINGPONG_SDM845_MASK, \
	.merge_3d = _merge_3d, \
	.sblk = &sdm845_pp_sblk \
	}

static const struct dpu_pingpong_cfg msm8998_pp[] = {
	PP_BLK_TE("pingpong_0", PINGPONG_0, 0x70000, 0),
	PP_BLK_TE("pingpong_1", PINGPONG_1, 0x70800, 0),
	PP_BLK("pingpong_2", PINGPONG_2, 0x71000, 0),
	PP_BLK("pingpong_3", PINGPONG_3, 0x71800, 0),
};

static const struct dpu_pingpong_cfg sdm845_pp[] = {
	PP_BLK_TE("pingpong_0", PINGPONG_0, 0x70000, 0),
	PP_BLK_TE("pingpong_1", PINGPONG_1, 0x70800, 0),
	PP_BLK("pingpong_2", PINGPONG_2, 0x71000, 0),
	PP_BLK("pingpong_3", PINGPONG_3, 0x71800, 0),
};

static struct dpu_pingpong_cfg sc7180_pp[] = {
	PP_BLK_TE("pingpong_0", PINGPONG_0, 0x70000, 0),
	PP_BLK_TE("pingpong_1", PINGPONG_1, 0x70800, 0),
};

static const struct dpu_pingpong_cfg sm8150_pp[] = {
	PP_BLK_TE("pingpong_0", PINGPONG_0, 0x70000, MERGE_3D_0),
	PP_BLK_TE("pingpong_1", PINGPONG_1, 0x70800, MERGE_3D_0),
	PP_BLK("pingpong_2", PINGPONG_2, 0x71000, MERGE_3D_1),
	PP_BLK("pingpong_3", PINGPONG_3, 0x71800, MERGE_3D_1),
	PP_BLK("pingpong_4", PINGPONG_4, 0x72000, MERGE_3D_2),
	PP_BLK("pingpong_5", PINGPONG_5, 0x72800, MERGE_3D_2),
};

/*************************************************************
 * MERGE_3D sub blocks config
 *************************************************************/
#define MERGE_3D_BLK(_name, _id, _base) \
	{\
	.name = _name, .id = _id, \
	.base = _base, .len = 0x100, \
	.features = MERGE_3D_SM8150_MASK, \
	.sblk = NULL \
	}

static const struct dpu_merge_3d_cfg sm8150_merge_3d[] = {
	MERGE_3D_BLK("merge_3d_0", MERGE_3D_0, 0x83000),
	MERGE_3D_BLK("merge_3d_1", MERGE_3D_1, 0x83100),
	MERGE_3D_BLK("merge_3d_2", MERGE_3D_2, 0x83200),
};

/*************************************************************
 * INTF sub blocks config
 *************************************************************/
#define INTF_BLK(_name, _id, _base, _type, _ctrl_id, _progfetch, _features) \
	{\
	.name = _name, .id = _id, \
	.base = _base, .len = 0x280, \
	.features = _features, \
	.type = _type, \
	.controller_id = _ctrl_id, \
	.prog_fetch_lines_worst_case = _progfetch \
	}

static const struct dpu_intf_cfg msm8998_intf[] = {
	INTF_BLK("intf_0", INTF_0, 0x6A000, INTF_DP, 0, 25, INTF_SDM845_MASK),
	INTF_BLK("intf_1", INTF_1, 0x6A800, INTF_DSI, 0, 25, INTF_SDM845_MASK),
	INTF_BLK("intf_2", INTF_2, 0x6B000, INTF_DSI, 1, 25, INTF_SDM845_MASK),
	INTF_BLK("intf_3", INTF_3, 0x6B800, INTF_HDMI, 0, 25, INTF_SDM845_MASK),
};

static const struct dpu_intf_cfg sdm845_intf[] = {
	INTF_BLK("intf_0", INTF_0, 0x6A000, INTF_DP, 0, 24, INTF_SDM845_MASK),
	INTF_BLK("intf_1", INTF_1, 0x6A800, INTF_DSI, 0, 24, INTF_SDM845_MASK),
	INTF_BLK("intf_2", INTF_2, 0x6B000, INTF_DSI, 1, 24, INTF_SDM845_MASK),
	INTF_BLK("intf_3", INTF_3, 0x6B800, INTF_DP, 1, 24, INTF_SDM845_MASK),
};

static const struct dpu_intf_cfg sc7180_intf[] = {
	INTF_BLK("intf_0", INTF_0, 0x6A000, INTF_DP, 0, 24, INTF_SC7180_MASK),
	INTF_BLK("intf_1", INTF_1, 0x6A800, INTF_DSI, 0, 24, INTF_SC7180_MASK),
};

static const struct dpu_intf_cfg sm8150_intf[] = {
	INTF_BLK("intf_0", INTF_0, 0x6A000, INTF_DP, 0, 24, INTF_SC7180_MASK),
	INTF_BLK("intf_1", INTF_1, 0x6A800, INTF_DSI, 0, 24, INTF_SC7180_MASK),
	INTF_BLK("intf_2", INTF_2, 0x6B000, INTF_DSI, 1, 24, INTF_SC7180_MASK),
	INTF_BLK("intf_3", INTF_3, 0x6B800, INTF_DP, 1, 24, INTF_SC7180_MASK),
};

/*************************************************************
 * VBIF sub blocks config
 *************************************************************/
/* VBIF QOS remap */
static const u32 msm8998_rt_pri_lvl[] = {1, 2, 2, 2};
static const u32 msm8998_nrt_pri_lvl[] = {1, 1, 1, 1};
static const u32 sdm845_rt_pri_lvl[] = {3, 3, 4, 4, 5, 5, 6, 6};
static const u32 sdm845_nrt_pri_lvl[] = {3, 3, 3, 3, 3, 3, 3, 3};

static const struct dpu_vbif_dynamic_ot_cfg msm8998_ot_rdwr_cfg[] = {
	{
		.pps = 1088 * 1920 * 30,
		.ot_limit = 2,
	},
	{
		.pps = 1088 * 1920 * 60,
		.ot_limit = 6,
	},
	{
		.pps = 3840 * 2160 * 30,
		.ot_limit = 16,
	},
};

static const struct dpu_vbif_cfg msm8998_vbif[] = {
	{
	.name = "vbif_0", .id = VBIF_0,
	.base = 0, .len = 0x1040,
	.default_ot_rd_limit = 32,
	.default_ot_wr_limit = 32,
	.features = BIT(DPU_VBIF_QOS_REMAP) | BIT(DPU_VBIF_QOS_OTLIM),
	.xin_halt_timeout = 0x4000,
	.dynamic_ot_rd_tbl = {
		.count = ARRAY_SIZE(msm8998_ot_rdwr_cfg),
		.cfg = msm8998_ot_rdwr_cfg,
		},
	.dynamic_ot_wr_tbl = {
		.count = ARRAY_SIZE(msm8998_ot_rdwr_cfg),
		.cfg = msm8998_ot_rdwr_cfg,
		},
	.qos_rt_tbl = {
		.npriority_lvl = ARRAY_SIZE(msm8998_rt_pri_lvl),
		.priority_lvl = msm8998_rt_pri_lvl,
		},
	.qos_nrt_tbl = {
		.npriority_lvl = ARRAY_SIZE(msm8998_nrt_pri_lvl),
		.priority_lvl = msm8998_nrt_pri_lvl,
		},
	.memtype_count = 14,
	.memtype = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	},
};

static const struct dpu_vbif_cfg sdm845_vbif[] = {
	{
	.name = "vbif_0", .id = VBIF_0,
	.base = 0, .len = 0x1040,
	.features = BIT(DPU_VBIF_QOS_REMAP),
	.xin_halt_timeout = 0x4000,
	.qos_rt_tbl = {
		.npriority_lvl = ARRAY_SIZE(sdm845_rt_pri_lvl),
		.priority_lvl = sdm845_rt_pri_lvl,
		},
	.qos_nrt_tbl = {
		.npriority_lvl = ARRAY_SIZE(sdm845_nrt_pri_lvl),
		.priority_lvl = sdm845_nrt_pri_lvl,
		},
	.memtype_count = 14,
	.memtype = {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	},
};

static const struct dpu_reg_dma_cfg sdm845_regdma = {
	.base = 0x0, .version = 0x1, .trigger_sel_off = 0x119c
};

static const struct dpu_reg_dma_cfg sm8150_regdma = {
	.base = 0x0, .version = 0x00010001, .trigger_sel_off = 0x119c
};

static const struct dpu_reg_dma_cfg sm8250_regdma = {
	.base = 0x0,
	.version = 0x00010002,
	.trigger_sel_off = 0x119c,
	.xin_id = 7,
	.clk_ctrl = DPU_CLK_CTRL_REG_DMA,
};

/*************************************************************
 * PERF data config
 *************************************************************/

/* SSPP QOS LUTs */
static const struct dpu_qos_lut_entry msm8998_qos_linear[] = {
	{.fl = 4,  .lut = 0x1b},
	{.fl = 5,  .lut = 0x5b},
	{.fl = 6,  .lut = 0x15b},
	{.fl = 7,  .lut = 0x55b},
	{.fl = 8,  .lut = 0x155b},
	{.fl = 9,  .lut = 0x555b},
	{.fl = 10, .lut = 0x1555b},
	{.fl = 11, .lut = 0x5555b},
	{.fl = 12, .lut = 0x15555b},
	{.fl = 13, .lut = 0x55555b},
	{.fl = 14, .lut = 0},
	{.fl = 1,  .lut = 0x1b},
	{.fl = 0,  .lut = 0}
};

static const struct dpu_qos_lut_entry sdm845_qos_linear[] = {
	{.fl = 4, .lut = 0x357},
	{.fl = 5, .lut = 0x3357},
	{.fl = 6, .lut = 0x23357},
	{.fl = 7, .lut = 0x223357},
	{.fl = 8, .lut = 0x2223357},
	{.fl = 9, .lut = 0x22223357},
	{.fl = 10, .lut = 0x222223357},
	{.fl = 11, .lut = 0x2222223357},
	{.fl = 12, .lut = 0x22222223357},
	{.fl = 13, .lut = 0x222222223357},
	{.fl = 14, .lut = 0x1222222223357},
	{.fl = 0, .lut = 0x11222222223357}
};

static const struct dpu_qos_lut_entry msm8998_qos_macrotile[] = {
	{.fl = 10, .lut = 0x1aaff},
	{.fl = 11, .lut = 0x5aaff},
	{.fl = 12, .lut = 0x15aaff},
	{.fl = 13, .lut = 0x55aaff},
	{.fl = 1,  .lut = 0x1aaff},
	{.fl = 0,  .lut = 0},
};

static const struct dpu_qos_lut_entry sc7180_qos_linear[] = {
	{.fl = 0, .lut = 0x0011222222335777},
};

static const struct dpu_qos_lut_entry sm8150_qos_linear[] = {
	{.fl = 0, .lut = 0x0011222222223357 },
};

static const struct dpu_qos_lut_entry sdm845_qos_macrotile[] = {
	{.fl = 10, .lut = 0x344556677},
	{.fl = 11, .lut = 0x3344556677},
	{.fl = 12, .lut = 0x23344556677},
	{.fl = 13, .lut = 0x223344556677},
	{.fl = 14, .lut = 0x1223344556677},
	{.fl = 0, .lut = 0x112233344556677},
};

static const struct dpu_qos_lut_entry sc7180_qos_macrotile[] = {
	{.fl = 0, .lut = 0x0011223344556677},
};

static const struct dpu_qos_lut_entry msm8998_qos_nrt[] = {
	{.fl = 0, .lut = 0x0},
};

static const struct dpu_qos_lut_entry sdm845_qos_nrt[] = {
	{.fl = 0, .lut = 0x0},
};

static const struct dpu_qos_lut_entry sc7180_qos_nrt[] = {
	{.fl = 0, .lut = 0x0},
};

static const struct dpu_perf_cfg msm8998_perf_data = {
	.max_bw_low = 6700000,
	.max_bw_high = 6700000,
	.min_core_ib = 2400000,
	.min_llcc_ib = 800000,
	.min_dram_ib = 800000,
	.core_ib_ff = "6.0",
	.core_clk_ff = "1.0",
	.comp_ratio_rt =
	"NV12/5/1/1.23 AB24/5/1/1.23 XB24/5/1/1.23",
	.comp_ratio_nrt =
	"NV12/5/1/1.25 AB24/5/1/1.25 XB24/5/1/1.25",
	.undersized_prefill_lines = 2,
	.xtra_prefill_lines = 2,
	.dest_scale_prefill_lines = 3,
	.macrotile_prefill_lines = 4,
	.yuv_nv12_prefill_lines = 8,
	.linear_prefill_lines = 1,
	.downscaling_prefill_lines = 1,
	.amortizable_threshold = 25,
	.min_prefill_lines = 25,
	.danger_lut_tbl = {0xf, 0xffff, 0x0},
	.safe_lut_tbl = {0xfffc, 0xff00, 0xffff},
	.qos_lut_tbl = {
		{.nentry = ARRAY_SIZE(msm8998_qos_linear),
		.entries = msm8998_qos_linear
		},
		{.nentry = ARRAY_SIZE(msm8998_qos_macrotile),
		.entries = msm8998_qos_macrotile
		},
		{.nentry = ARRAY_SIZE(msm8998_qos_nrt),
		.entries = msm8998_qos_nrt
		},
	},
	.cdp_cfg = {
		{.rd_enable = 1, .wr_enable = 1},
		{.rd_enable = 1, .wr_enable = 0}
	},
	.clk_inefficiency_factor = 200,
	.bw_inefficiency_factor = 120,
};

static const struct dpu_perf_cfg sdm845_perf_data = {
	.max_bw_low = 6800000,
	.max_bw_high = 6800000,
	.min_core_ib = 2400000,
	.min_llcc_ib = 800000,
	.min_dram_ib = 800000,
	.core_ib_ff = "6.0",
	.core_clk_ff = "1.0",
	.comp_ratio_rt =
	"NV12/5/1/1.23 AB24/5/1/1.23 XB24/5/1/1.23",
	.comp_ratio_nrt =
	"NV12/5/1/1.25 AB24/5/1/1.25 XB24/5/1/1.25",
	.undersized_prefill_lines = 2,
	.xtra_prefill_lines = 2,
	.dest_scale_prefill_lines = 3,
	.macrotile_prefill_lines = 4,
	.yuv_nv12_prefill_lines = 8,
	.linear_prefill_lines = 1,
	.downscaling_prefill_lines = 1,
	.amortizable_threshold = 25,
	.min_prefill_lines = 24,
	.danger_lut_tbl = {0xf, 0xffff, 0x0},
	.qos_lut_tbl = {
		{.nentry = ARRAY_SIZE(sdm845_qos_linear),
		.entries = sdm845_qos_linear
		},
		{.nentry = ARRAY_SIZE(sdm845_qos_macrotile),
		.entries = sdm845_qos_macrotile
		},
		{.nentry = ARRAY_SIZE(sdm845_qos_nrt),
		.entries = sdm845_qos_nrt
		},
	},
	.cdp_cfg = {
		{.rd_enable = 1, .wr_enable = 1},
		{.rd_enable = 1, .wr_enable = 0}
	},
};

static const struct dpu_perf_cfg sc7180_perf_data = {
	.max_bw_low = 6800000,
	.max_bw_high = 6800000,
	.min_core_ib = 2400000,
	.min_llcc_ib = 800000,
	.min_dram_ib = 1600000,
	.min_prefill_lines = 24,
	.danger_lut_tbl = {0xff, 0xffff, 0x0},
	.qos_lut_tbl = {
		{.nentry = ARRAY_SIZE(sc7180_qos_linear),
		.entries = sc7180_qos_linear
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_macrotile),
		.entries = sc7180_qos_macrotile
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_nrt),
		.entries = sc7180_qos_nrt
		},
	},
	.cdp_cfg = {
		{.rd_enable = 1, .wr_enable = 1},
		{.rd_enable = 1, .wr_enable = 0}
	},
	.clk_inefficiency_factor = 105,
	.bw_inefficiency_factor = 120,
};

static const struct dpu_perf_cfg sm8150_perf_data = {
	.max_bw_low = 12800000,
	.max_bw_high = 12800000,
	.min_core_ib = 2400000,
	.min_llcc_ib = 800000,
	.min_dram_ib = 800000,
	.danger_lut_tbl = {0xf, 0xffff, 0x0},
	.qos_lut_tbl = {
		{.nentry = ARRAY_SIZE(sm8150_qos_linear),
		.entries = sm8150_qos_linear
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_macrotile),
		.entries = sc7180_qos_macrotile
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_nrt),
		.entries = sc7180_qos_nrt
		},
		/* TODO: macrotile-qseed is different from macrotile */
	},
	.cdp_cfg = {
		{.rd_enable = 1, .wr_enable = 1},
		{.rd_enable = 1, .wr_enable = 0}
	},
};

static const struct dpu_perf_cfg sm8250_perf_data = {
	.max_bw_low = 13700000,
	.max_bw_high = 16600000,
	.min_core_ib = 4800000,
	.min_llcc_ib = 0,
	.min_dram_ib = 800000,
	.danger_lut_tbl = {0xf, 0xffff, 0x0},
	.qos_lut_tbl = {
		{.nentry = ARRAY_SIZE(sc7180_qos_linear),
		.entries = sc7180_qos_linear
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_macrotile),
		.entries = sc7180_qos_macrotile
		},
		{.nentry = ARRAY_SIZE(sc7180_qos_nrt),
		.entries = sc7180_qos_nrt
		},
		/* TODO: macrotile-qseed is different from macrotile */
	},
	.cdp_cfg = {
		{.rd_enable = 1, .wr_enable = 1},
		{.rd_enable = 1, .wr_enable = 0}
	},
};

/*************************************************************
 * Hardware catalog init
 *************************************************************/

/*
 * msm8998_cfg_init(): populate sdm845 dpu sub-blocks reg offsets
 * and instance counts.
 */
static void msm8998_cfg_init(struct dpu_mdss_cfg *dpu_cfg)
{
	*dpu_cfg = (struct dpu_mdss_cfg){
		.caps = &msm8998_dpu_caps,
		.mdp_count = ARRAY_SIZE(msm8998_mdp),
		.mdp = msm8998_mdp,
		.ctl_count = ARRAY_SIZE(msm8998_ctl),
		.ctl = msm8998_ctl,
		.sspp_count = ARRAY_SIZE(msm8998_sspp),
		.sspp = msm8998_sspp,
		.mixer_count = ARRAY_SIZE(msm8998_lm),
		.mixer = msm8998_lm,
		.pingpong_count = ARRAY_SIZE(msm8998_pp),
		.pingpong = msm8998_pp,
		.intf_count = ARRAY_SIZE(msm8998_intf),
		.intf = msm8998_intf,
		.vbif_count = ARRAY_SIZE(msm8998_vbif),
		.vbif = msm8998_vbif,
		.reg_dma_count = 0,
		.perf = msm8998_perf_data,
		/* There are no AD4 interrupts (bits 8-9) */
		.mdss_irqs = 0xff,
	};
}

/*
 * sdm845_cfg_init(): populate sdm845 dpu sub-blocks reg offsets
 * and instance counts.
 */
static void sdm845_cfg_init(struct dpu_mdss_cfg *dpu_cfg)
{
	*dpu_cfg = (struct dpu_mdss_cfg){
		.caps = &sdm845_dpu_caps,
		.mdp_count = ARRAY_SIZE(sdm845_mdp),
		.mdp = sdm845_mdp,
		.ctl_count = ARRAY_SIZE(sdm845_ctl),
		.ctl = sdm845_ctl,
		.sspp_count = ARRAY_SIZE(sdm845_sspp),
		.sspp = sdm845_sspp,
		.mixer_count = ARRAY_SIZE(sdm845_lm),
		.mixer = sdm845_lm,
		.pingpong_count = ARRAY_SIZE(sdm845_pp),
		.pingpong = sdm845_pp,
		.intf_count = ARRAY_SIZE(sdm845_intf),
		.intf = sdm845_intf,
		.vbif_count = ARRAY_SIZE(sdm845_vbif),
		.vbif = sdm845_vbif,
		.reg_dma_count = 1,
		.dma_cfg = sdm845_regdma,
		.perf = sdm845_perf_data,
		.mdss_irqs = 0x3ff,
	};
}

/*
 * sc7180_cfg_init(): populate sc7180 dpu sub-blocks reg offsets
 * and instance counts.
 */
static void sc7180_cfg_init(struct dpu_mdss_cfg *dpu_cfg)
{
	*dpu_cfg = (struct dpu_mdss_cfg){
		.caps = &sc7180_dpu_caps,
		.mdp_count = ARRAY_SIZE(sc7180_mdp),
		.mdp = sc7180_mdp,
		.ctl_count = ARRAY_SIZE(sc7180_ctl),
		.ctl = sc7180_ctl,
		.sspp_count = ARRAY_SIZE(sc7180_sspp),
		.sspp = sc7180_sspp,
		.mixer_count = ARRAY_SIZE(sc7180_lm),
		.mixer = sc7180_lm,
		.dspp_count = ARRAY_SIZE(sc7180_dspp),
		.dspp = sc7180_dspp,
		.pingpong_count = ARRAY_SIZE(sc7180_pp),
		.pingpong = sc7180_pp,
		.intf_count = ARRAY_SIZE(sc7180_intf),
		.intf = sc7180_intf,
		.vbif_count = ARRAY_SIZE(sdm845_vbif),
		.vbif = sdm845_vbif,
		.reg_dma_count = 1,
		.dma_cfg = sdm845_regdma,
		.perf = sc7180_perf_data,
		.mdss_irqs = 0x3f,
	};
}

/*
 * sm8150_cfg_init(): populate sm8150 dpu sub-blocks reg offsets
 * and instance counts.
 */
static void sm8150_cfg_init(struct dpu_mdss_cfg *dpu_cfg)
{
	*dpu_cfg = (struct dpu_mdss_cfg){
		.caps = &sm8150_dpu_caps,
		.mdp_count = ARRAY_SIZE(sdm845_mdp),
		.mdp = sdm845_mdp,
		.ctl_count = ARRAY_SIZE(sm8150_ctl),
		.ctl = sm8150_ctl,
		.sspp_count = ARRAY_SIZE(sdm845_sspp),
		.sspp = sdm845_sspp,
		.mixer_count = ARRAY_SIZE(sm8150_lm),
		.mixer = sm8150_lm,
		.dspp_count = ARRAY_SIZE(sm8150_dspp),
		.dspp = sm8150_dspp,
		.pingpong_count = ARRAY_SIZE(sm8150_pp),
		.pingpong = sm8150_pp,
		.merge_3d_count = ARRAY_SIZE(sm8150_merge_3d),
		.merge_3d = sm8150_merge_3d,
		.intf_count = ARRAY_SIZE(sm8150_intf),
		.intf = sm8150_intf,
		.vbif_count = ARRAY_SIZE(sdm845_vbif),
		.vbif = sdm845_vbif,
		.reg_dma_count = 1,
		.dma_cfg = sm8150_regdma,
		.perf = sm8150_perf_data,
		.mdss_irqs = 0x3ff,
	};
}

/*
 * sm8250_cfg_init(): populate sm8250 dpu sub-blocks reg offsets
 * and instance counts.
 */
static void sm8250_cfg_init(struct dpu_mdss_cfg *dpu_cfg)
{
	*dpu_cfg = (struct dpu_mdss_cfg){
		.caps = &sm8250_dpu_caps,
		.mdp_count = ARRAY_SIZE(sm8250_mdp),
		.mdp = sm8250_mdp,
		.ctl_count = ARRAY_SIZE(sm8150_ctl),
		.ctl = sm8150_ctl,
		.sspp_count = ARRAY_SIZE(sm8250_sspp),
		.sspp = sm8250_sspp,
		.mixer_count = ARRAY_SIZE(sm8150_lm),
		.mixer = sm8150_lm,
		.dspp_count = ARRAY_SIZE(sm8150_dspp),
		.dspp = sm8150_dspp,
		.pingpong_count = ARRAY_SIZE(sm8150_pp),
		.pingpong = sm8150_pp,
		.merge_3d_count = ARRAY_SIZE(sm8150_merge_3d),
		.merge_3d = sm8150_merge_3d,
		.intf_count = ARRAY_SIZE(sm8150_intf),
		.intf = sm8150_intf,
		.vbif_count = ARRAY_SIZE(sdm845_vbif),
		.vbif = sdm845_vbif,
		.reg_dma_count = 1,
		.dma_cfg = sm8250_regdma,
		.perf = sm8250_perf_data,
		.mdss_irqs = 0xff,
	};
}

static const struct dpu_mdss_hw_cfg_handler cfg_handler[] = {
	{ .hw_rev = DPU_HW_VER_300, .cfg_init = msm8998_cfg_init},
	{ .hw_rev = DPU_HW_VER_301, .cfg_init = msm8998_cfg_init},
	{ .hw_rev = DPU_HW_VER_400, .cfg_init = sdm845_cfg_init},
	{ .hw_rev = DPU_HW_VER_401, .cfg_init = sdm845_cfg_init},
	{ .hw_rev = DPU_HW_VER_500, .cfg_init = sm8150_cfg_init},
	{ .hw_rev = DPU_HW_VER_501, .cfg_init = sm8150_cfg_init},
	{ .hw_rev = DPU_HW_VER_600, .cfg_init = sm8250_cfg_init},
	{ .hw_rev = DPU_HW_VER_620, .cfg_init = sc7180_cfg_init},
};

void dpu_hw_catalog_deinit(struct dpu_mdss_cfg *dpu_cfg)
{
	kfree(dpu_cfg);
}

struct dpu_mdss_cfg *dpu_hw_catalog_init(u32 hw_rev)
{
	int i;
	struct dpu_mdss_cfg *dpu_cfg;

	dpu_cfg = kzalloc(sizeof(*dpu_cfg), GFP_KERNEL);
	if (!dpu_cfg)
		return ERR_PTR(-ENOMEM);

	for (i = 0; i < ARRAY_SIZE(cfg_handler); i++) {
		if (cfg_handler[i].hw_rev == hw_rev) {
			cfg_handler[i].cfg_init(dpu_cfg);
			dpu_cfg->hwversion = hw_rev;
			return dpu_cfg;
		}
	}

	DPU_ERROR("unsupported chipset id:%X\n", hw_rev);
	dpu_hw_catalog_deinit(dpu_cfg);
	return ERR_PTR(-ENODEV);
}

