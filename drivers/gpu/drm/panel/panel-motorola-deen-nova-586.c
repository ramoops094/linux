// SPDX-License-Identifier: GPL-2.0-only
// Init sequence from Motorola Moto G7 Play (channel) downstream:
//   dsi-panel-mot-nova-586-hd-vid-common.dtsi
//   (electimon/android_kernel_motorola_channel, android-9.0)

#include <linux/backlight.h>
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

struct dn586_nova {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data dn586_nova_supplies[] = {
	{ .supply = "vsn" },
	{ .supply = "vsp" },
	{ .supply = "iovcc" },
};

static inline struct dn586_nova *to_dn586_nova(struct drm_panel *panel)
{
	return container_of_const(panel, struct dn586_nova, panel);
}

static void dn586_nova_reset(struct dn586_nova *ctx)
{
	/* reset-gpios is GPIO_ACTIVE_LOW, values are logical levels. */
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 1100);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int dn586_nova_on(struct dn586_nova *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0xcc);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29);
	mipi_dsi_msleep(&dsi_ctx, 100);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int dn586_nova_off(struct dn586_nova *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_msleep(&dsi_ctx, 10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int dn586_nova_prepare(struct drm_panel *panel)
{
	struct dn586_nova *ctx = to_dn586_nova(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(dn586_nova_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	dn586_nova_reset(ctx);

	ret = dn586_nova_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(dn586_nova_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int dn586_nova_unprepare(struct drm_panel *panel)
{
	struct dn586_nova *ctx = to_dn586_nova(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = dn586_nova_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(dn586_nova_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode dn586_nova_mode = {
	.clock = (720 + 328 + 32 + 4) * (1512 + 6 + 8 + 2) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 328,
	.hsync_end = 720 + 328 + 4,
	.htotal = 720 + 328 + 4 + 32,
	.vdisplay = 1512,
	.vsync_start = 1512 + 6,
	.vsync_end = 1512 + 6 + 2,
	.vtotal = 1512 + 6 + 2 + 8,
	.width_mm = 64,
	.height_mm = 128,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int dn586_nova_get_modes(struct drm_panel *panel,
			   struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &dn586_nova_mode);
}

static const struct drm_panel_funcs dn586_nova_panel_funcs = {
	.prepare = dn586_nova_prepare,
	.unprepare = dn586_nova_unprepare,
	.get_modes = dn586_nova_get_modes,
};


static int dn586_nova_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct dn586_nova *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct dn586_nova, panel,
				   &dn586_nova_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(dn586_nova_supplies),
					    dn586_nova_supplies,
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
			  MIPI_DSI_MODE_NO_EOT_PACKET |
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

static void dn586_nova_remove(struct mipi_dsi_device *dsi)
{
	struct dn586_nova *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id dn586_nova_of_match[] = {
	{ .compatible = "motorola,deen-nova-586" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, dn586_nova_of_match);

static struct mipi_dsi_driver dn586_nova_driver = {
	.probe = dn586_nova_probe,
	.remove = dn586_nova_remove,
	.driver = {
		.name = "panel-motorola-deen-nova-586",
		.of_match_table = dn586_nova_of_match,
	},
};
module_mipi_dsi_driver(dn586_nova_driver);

MODULE_AUTHOR("Moto8953-Revived");
MODULE_DESCRIPTION("DRM driver for Nova 586 720x1520 video mode DSI panel (Motorola Deen)");
MODULE_LICENSE("GPL");
