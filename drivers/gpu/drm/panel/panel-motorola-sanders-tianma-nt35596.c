// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 tigelaa <tigelaa@proton.me>
// Init sequence from Motorola Moto G5S Plus (sanders) downstream:
//   dsi-panel-mot-tianma-550-1080p-vid-common.dtsi
//   (MotorolaMobilityLLC/kernel-msm, oreo-8.1.0-release-sanders)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct tianma_nt35596 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data tianma_nt35596_supplies[] = {
	{ .supply = "vsn" },
	{ .supply = "vsp" },
};

static inline struct tianma_nt35596 *to_tianma_nt35596(struct drm_panel *panel)
{
	return container_of_const(panel, struct tianma_nt35596, panel);
}

static void tianma_nt35596_reset(struct tianma_nt35596 *ctx)
{
	/* reset-gpios is GPIO_ACTIVE_LOW, so these are logical levels:
	 * 0 = deasserted (physical high), 1 = asserted (physical low).
	 * The sequence must end deasserted or the panel stays in reset. */
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(20000, 21000);
}

static int tianma_nt35596_on(struct tianma_nt35596 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0xee);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x40);
	mipi_dsi_msleep(&dsi_ctx, 10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x05);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0xb6);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x04);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x07);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xfb, 0x01);
	mipi_dsi_dcs_set_tear_on_multi(&dsi_ctx,
				       MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x5e, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_POWER_SAVE, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd3, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x0e);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 130);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 10);

	return dsi_ctx.accum_err;
}

static int tianma_nt35596_off(struct tianma_nt35596 *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 35);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int tianma_nt35596_prepare(struct drm_panel *panel)
{
	struct tianma_nt35596 *ctx = to_tianma_nt35596(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(tianma_nt35596_supplies),
				    ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	tianma_nt35596_reset(ctx);

	ret = tianma_nt35596_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(tianma_nt35596_supplies),
				       ctx->supplies);
		return ret;
	}

	return 0;
}

static int tianma_nt35596_unprepare(struct drm_panel *panel)
{
	struct tianma_nt35596 *ctx = to_tianma_nt35596(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = tianma_nt35596_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(tianma_nt35596_supplies),
			       ctx->supplies);

	return 0;
}

static const struct drm_display_mode tianma_nt35596_mode = {
	.clock = (1080 + 96 + 4 + 16) * (1920 + 14 + 6 + 10) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 96,
	.hsync_end = 1080 + 96 + 4,
	.htotal = 1080 + 96 + 4 + 16,
	.vdisplay = 1920,
	.vsync_start = 1920 + 14,
	.vsync_end = 1920 + 14 + 6,
	.vtotal = 1920 + 14 + 6 + 10,
	.width_mm = 68,
	.height_mm = 121,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int tianma_nt35596_get_modes(struct drm_panel *panel,
				    struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector,
						    &tianma_nt35596_mode);
}

static const struct drm_panel_funcs tianma_nt35596_panel_funcs = {
	.prepare = tianma_nt35596_prepare,
	.unprepare = tianma_nt35596_unprepare,
	.get_modes = tianma_nt35596_get_modes,
};

static int tianma_nt35596_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct tianma_nt35596 *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct tianma_nt35596, panel,
				   &tianma_nt35596_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(tianma_nt35596_supplies),
					    tianma_nt35596_supplies,
					    &ctx->supplies);
	if (ret < 0)
		return ret;

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_VIDEO_HSE | MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void tianma_nt35596_remove(struct mipi_dsi_device *dsi)
{
	struct tianma_nt35596 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id tianma_nt35596_of_match[] = {
	{ .compatible = "motorola,sanders-tianma-nt35596" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, tianma_nt35596_of_match);

static struct mipi_dsi_driver tianma_nt35596_driver = {
	.probe = tianma_nt35596_probe,
	.remove = tianma_nt35596_remove,
	.driver = {
		.name = "panel-motorola-sanders-tianma-nt35596",
		.of_match_table = tianma_nt35596_of_match,
	},
};
module_mipi_dsi_driver(tianma_nt35596_driver);

MODULE_AUTHOR("tigelaa <tigelaa@proton.me>");
MODULE_DESCRIPTION("DRM driver for Tianma NT35596 1080p video mode dsi panel (Motorola Sanders)");
MODULE_LICENSE("GPL");
