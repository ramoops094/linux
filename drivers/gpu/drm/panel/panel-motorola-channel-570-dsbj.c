// SPDX-License-Identifier: GPL-2.0-only
// Init sequence from Motorola Moto G7 Play (channel) downstream:
//   dsi-panel-mot-dsbj-570-hd-vid-common.dtsi
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

struct ch570_dsbj {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct regulator_bulk_data *supplies;
	struct gpio_desc *reset_gpio;
};

static const struct regulator_bulk_data ch570_dsbj_supplies[] = {
	{ .supply = "vsn" },
	{ .supply = "vsp" },
};

static inline struct ch570_dsbj *to_ch570_dsbj(struct drm_panel *panel)
{
	return container_of_const(panel, struct ch570_dsbj, panel);
}

static void ch570_dsbj_reset(struct ch570_dsbj *ctx)
{
	/* reset-gpios is GPIO_ACTIVE_LOW, values are logical levels. */
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 11000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int ch570_dsbj_on(struct ch570_dsbj *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x0e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x46, 0x5d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4d, 0x93);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xd4, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x10);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x4b, 0x13);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x77);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x23);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x41);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0xff);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x3e, 0x62);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x0c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x00, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x01, 0x67);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x02, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x03, 0x6a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x04, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x05, 0x66);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x07, 0x70);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x08, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x09, 0x6e);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0a, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0b, 0x63);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0c, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0d, 0x6d);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0e, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x0f, 0x64);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x72);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x12, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x13, 0x6b);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x14, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x15, 0x73);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x16, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x17, 0x69);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x18, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x19, 0x61);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1a, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1b, 0x62);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1c, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1d, 0x6f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1e, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x1f, 0x6c);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x20, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x21, 0x65);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x22, 0x18);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x23, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x24, 0x19);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x25, 0x68);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x26, 0x1a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x27, 0x71);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x51, 0x00, 0x00);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x53, 0x2c);
	mipi_dsi_msleep(&dsi_ctx, 5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x55, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x11, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x83, 0x20);
	mipi_dsi_msleep(&dsi_ctx, 5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x84, 0x02);
	mipi_dsi_msleep(&dsi_ctx, 5);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xbc, 0x08);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x06);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x06, 0xc4);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x01);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x2d, 0x60);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xff, 0x98, 0x81, 0x00);
	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x29, 0x00);

	return dsi_ctx.accum_err;
}

static int ch570_dsbj_off(struct ch570_dsbj *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_msleep(&dsi_ctx, 20);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x28);
	mipi_dsi_msleep(&dsi_ctx, 120);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x10);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int ch570_dsbj_prepare(struct drm_panel *panel)
{
	struct ch570_dsbj *ctx = to_ch570_dsbj(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = regulator_bulk_enable(ARRAY_SIZE(ch570_dsbj_supplies), ctx->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators: %d\n", ret);
		return ret;
	}

	ch570_dsbj_reset(ctx);

	ret = ch570_dsbj_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ch570_dsbj_supplies), ctx->supplies);
		return ret;
	}

	return 0;
}

static int ch570_dsbj_unprepare(struct drm_panel *panel)
{
	struct ch570_dsbj *ctx = to_ch570_dsbj(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = ch570_dsbj_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ch570_dsbj_supplies), ctx->supplies);

	return 0;
}

static const struct drm_display_mode ch570_dsbj_mode = {
	.clock = (720 + 52 + 48 + 28) * (1512 + 41 + 20 + 4) * 60 / 1000,
	.hdisplay = 720,
	.hsync_start = 720 + 52,
	.hsync_end = 720 + 52 + 28,
	.htotal = 720 + 52 + 28 + 48,
	.vdisplay = 1512,
	.vsync_start = 1512 + 41,
	.vsync_end = 1512 + 41 + 4,
	.vtotal = 1512 + 41 + 4 + 20,
	.width_mm = 64,
	.height_mm = 128,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int ch570_dsbj_get_modes(struct drm_panel *panel,
			   struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &ch570_dsbj_mode);
}

static const struct drm_panel_funcs ch570_dsbj_panel_funcs = {
	.prepare = ch570_dsbj_prepare,
	.unprepare = ch570_dsbj_unprepare,
	.get_modes = ch570_dsbj_get_modes,
};

static int ch570_dsbj_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return 0;
}

static int ch570_dsbj_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	if (ret < 0)
		return ret;

	dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	return brightness;
}

static const struct backlight_ops ch570_dsbj_bl_ops = {
	.update_status = ch570_dsbj_bl_update_status,
	.get_brightness = ch570_dsbj_bl_get_brightness,
};

static struct backlight_device *
ch570_dsbj_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 4095,
		.max_brightness = 4095,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &ch570_dsbj_bl_ops, &props);
}

static int ch570_dsbj_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct ch570_dsbj *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct ch570_dsbj, panel,
				   &ch570_dsbj_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ret = devm_regulator_bulk_get_const(dev,
					    ARRAY_SIZE(ch570_dsbj_supplies),
					    ch570_dsbj_supplies,
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
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM |
		  MIPI_DSI_MODE_VIDEO_HSE;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = ch570_dsbj_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void ch570_dsbj_remove(struct mipi_dsi_device *dsi)
{
	struct ch570_dsbj *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id ch570_dsbj_of_match[] = {
	{ .compatible = "motorola,channel-570-dsbj" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, ch570_dsbj_of_match);

static struct mipi_dsi_driver ch570_dsbj_driver = {
	.probe = ch570_dsbj_probe,
	.remove = ch570_dsbj_remove,
	.driver = {
		.name = "panel-motorola-channel-570-dsbj",
		.of_match_table = ch570_dsbj_of_match,
	},
};
module_mipi_dsi_driver(ch570_dsbj_driver);

MODULE_AUTHOR("Moto8953-Revived");
MODULE_DESCRIPTION("DRM driver for DSBJ 570 720x1512 video mode DSI panel (Motorola Channel)");
MODULE_LICENSE("GPL");
