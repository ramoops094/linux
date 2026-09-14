// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 tigelaa <tigelaa@proton.me>
// Init sequence from Motorola Moto G5S Plus (sanders) downstream:
//   dsi-panel-mot-djn-550-1080p-vid-common.dtsi
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

struct djn_ili7807d {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data djn_ili7807d_supplies[] = {
	{ .supply = "vsn" },
	{ .supply = "vsp" },
};

static inline struct djn_ili7807d *to_djn_ili7807d(struct drm_panel *panel)
{
	return container_of_const(panel, struct djn_ili7807d, panel);
}

static void djn_ili7807d_reset(struct djn_ili7807d *ctx)
{
	/* reset-gpios is GPIO_ACTIVE_LOW, so these are logical levels:
	 * 0 = deasserted (physical high), 1 = asserted (physical low).
	 * The sequence must end deasserted or the panel stays in reset. */
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(5000, 6000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(40000, 41000);
}

static int djn_ili7807d_on(struct djn_ili7807d *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x78, 0x07, 0x05);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x03, 0x50);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x04, 0x02);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0xff, 0x78, 0x07, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x51, 0x0c, 0xcc);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x55, 0x01);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 10);
	mipi_dsi_generic_write_seq_multi(&dsi_ctx, 0x35, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 10);

	return dsi_ctx.accum_err;
}

static int djn_ili7807d_off(struct djn_ili7807d *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 35);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int djn_ili7807d_prepare(struct drm_panel *panel)
{
	struct djn_ili7807d *ctx = to_djn_ili7807d(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(djn_ili7807d_supplies),
				    ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	djn_ili7807d_reset(ctx);

	ret = djn_ili7807d_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(djn_ili7807d_supplies),
				       ctx->supplies);
		return ret;
	}

	return 0;
}

static int djn_ili7807d_unprepare(struct drm_panel *panel)
{
	struct djn_ili7807d *ctx = to_djn_ili7807d(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = djn_ili7807d_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(djn_ili7807d_supplies),
			       ctx->supplies);

	return 0;
}

static const struct drm_display_mode djn_ili7807d_mode = {
	.clock = (1080 + 106 + 4 + 104) * (1920 + 24 + 16 + 24) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 106,
	.hsync_end = 1080 + 106 + 4,
	.htotal = 1080 + 106 + 4 + 104,
	.vdisplay = 1920,
	.vsync_start = 1920 + 24,
	.vsync_end = 1920 + 24 + 16,
	.vtotal = 1920 + 24 + 16 + 24,
	.width_mm = 68,
	.height_mm = 121,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int djn_ili7807d_get_modes(struct drm_panel *panel,
				  struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector,
						    &djn_ili7807d_mode);
}

static const struct drm_panel_funcs djn_ili7807d_panel_funcs = {
	.prepare = djn_ili7807d_prepare,
	.unprepare = djn_ili7807d_unprepare,
	.get_modes = djn_ili7807d_get_modes,
};

static int djn_ili7807d_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct djn_ili7807d *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct djn_ili7807d, panel,
				   &djn_ili7807d_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(djn_ili7807d_supplies),
					    djn_ili7807d_supplies,
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

static void djn_ili7807d_remove(struct mipi_dsi_device *dsi)
{
	struct djn_ili7807d *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id djn_ili7807d_of_match[] = {
	{ .compatible = "motorola,sanders-djn-ili7807d" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, djn_ili7807d_of_match);

static struct mipi_dsi_driver djn_ili7807d_driver = {
	.probe = djn_ili7807d_probe,
	.remove = djn_ili7807d_remove,
	.driver = {
		.name = "panel-motorola-sanders-djn-ili7807d",
		.of_match_table = djn_ili7807d_of_match,
	},
};
module_mipi_dsi_driver(djn_ili7807d_driver);

MODULE_AUTHOR("tigelaa <tigelaa@proton.me>");
MODULE_DESCRIPTION("DRM driver for DJN ILI7807D 1080p video mode dsi panel (Motorola Sanders)");
MODULE_LICENSE("GPL");
