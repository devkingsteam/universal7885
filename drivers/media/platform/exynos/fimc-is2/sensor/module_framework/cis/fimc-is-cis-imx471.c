/*
 * Samsung Exynos5 SoC series Sensor driver
 *
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/syscalls.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>

#include <exynos-fimc-is-sensor.h>
#include "fimc-is-hw.h"
#include "fimc-is-core.h"
#include "fimc-is-param.h"
#include "fimc-is-device-sensor.h"
#include "fimc-is-device-sensor-peri.h"
#include "fimc-is-resourcemgr.h"
#include "fimc-is-dt.h"
#include "fimc-is-cis-imx471.h"
#include "fimc-is-cis-imx471-setA.h"
#include "fimc-is-cis-imx471-setB.h"

#include "fimc-is-helper-i2c.h"

#include "interface/fimc-is-interface-library.h"

#define SENSOR_NAME "IMX471"
#define SENSOR_NAME_REAR  "IMX471_REAR"
#define SENSOR_NAME_FRONT "IMX471_FRONT"
#define SENSOR_REAR_DATA 0
#define SENSOR_FRONT_DATA 1
/* #define DEBUG_IMX471_PLL */

static const u32 *sensor_imx471_global;
static u32 sensor_imx471_global_size;
//static const u32 *sensor_imx471_imageQuality;
//static u32 sensor_imx471_imageQuality_size;
static const u32 **sensor_imx471_setfiles;
static const u32 *sensor_imx471_setfile_sizes;
static const struct sensor_pll_info_compact **sensor_imx471_pllinfos;
static u32 sensor_imx471_max_setfile_num;
static bool sensor_imx471_cal_write_flag;
static bool sensor_imx471_read_dpc;
struct dpc_dfct imx471_dpc;

extern struct fimc_is_lib_support gPtr_lib_support;

static void sensor_imx471_set_integration_max_margin(u32 mode, cis_shared_data *cis_data)
{
	BUG_ON(!cis_data);

	switch (mode) {
		case SENSOR_IMX471_4608X3456_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2592_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2240_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2176_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3456X3456_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X2868_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X2152_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X1764_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_2864X2864_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_2304X1728_2X2BIN_60FPS:
		case SENSOR_IMX471_2304X1728_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1296_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1120_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1088_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X1440_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X1080_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X888_2X2BIN_30FPS:
		case SENSOR_IMX471_1728X1728_2X2BIN_30FPS:
		case SENSOR_IMX471_1440X1440_2X2BIN_30FPS:
			cis_data->max_margin_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MAX_MARGIN;
			dbg_sensor(1, "max_margin_coarse_integration_time(%d)\n",
				cis_data->max_margin_coarse_integration_time);
			break;
		default:
			err("[%s] Unsupport imx471 sensor mode\n", __func__);
			cis_data->max_margin_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MAX_MARGIN;
			dbg_sensor(1, "max_margin_coarse_integration_time(%d)\n",
				cis_data->max_margin_coarse_integration_time);
			break;
	}
}

static void sensor_imx471_set_integration_min(u32 mode, cis_shared_data *cis_data)
{
	BUG_ON(!cis_data);

	switch (mode) {
		case SENSOR_IMX471_4608X3456_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2592_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2240_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_4608X2176_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3456X3456_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X2868_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X2152_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_3824X1764_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_2864X2864_QBCREMOSAIC_30FPS:
		case SENSOR_IMX471_2304X1728_2X2BIN_60FPS:
		case SENSOR_IMX471_2304X1728_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1296_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1120_2X2BIN_30FPS:
		case SENSOR_IMX471_2304X1088_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X1440_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X1080_2X2BIN_30FPS:
		case SENSOR_IMX471_1920X888_2X2BIN_30FPS:
		case SENSOR_IMX471_1728X1728_2X2BIN_30FPS:
		case SENSOR_IMX471_1440X1440_2X2BIN_30FPS:
			cis_data->min_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MIN;
			dbg_sensor(1, "min_coarse_integration_time(%d)\n",
				cis_data->min_coarse_integration_time);
			break;
		default:
			err("[%s] Unsupport imx471 sensor mode\n", __func__);
			cis_data->min_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MIN;
			dbg_sensor(1, "min_coarse_integration_time(%d)\n",
				cis_data->min_coarse_integration_time);
			break;
	}
}

static void sensor_imx471_cis_data_calculation(const struct sensor_pll_info_compact *pll_info, cis_shared_data *cis_data)
{
	u32 vt_pix_clk_hz = 0;
	u32 frame_rate = 0, max_fps = 0, frame_valid_us = 0;

	BUG_ON(!pll_info);

	/* 1. get pclk value from pll info */
	vt_pix_clk_hz = pll_info->pclk;

	/* 2. the time of processing one frame calculation (us) */
	cis_data->min_frame_us_time = ((pll_info->frame_length_lines * pll_info->line_length_pck)
								/ (vt_pix_clk_hz / (1000 * 1000)));
	cis_data->cur_frame_us_time = cis_data->min_frame_us_time;

	/* 3. FPS calculation */
	frame_rate = vt_pix_clk_hz / (pll_info->frame_length_lines * pll_info->line_length_pck);
	dbg_sensor(1, "frame_rate (%d) = vt_pix_clk_hz(%d) / "
		KERN_CONT "(pll_info->frame_length_lines(%d) * pll_info->line_length_pck(%d))\n",
		frame_rate, vt_pix_clk_hz, pll_info->frame_length_lines, pll_info->line_length_pck);

	/* calculate max fps */
	max_fps = (vt_pix_clk_hz * 10) / (pll_info->frame_length_lines * pll_info->line_length_pck);
	max_fps = (max_fps % 10 >= 5 ? frame_rate + 1 : frame_rate);

	cis_data->pclk = vt_pix_clk_hz;
	cis_data->max_fps = max_fps;
	cis_data->frame_length_lines = pll_info->frame_length_lines;
	cis_data->line_length_pck = pll_info->line_length_pck;
	cis_data->line_readOut_time = sensor_cis_do_div64((u64)cis_data->line_length_pck * (u64)(1000 * 1000 * 1000), cis_data->pclk);
 	cis_data->rolling_shutter_skew = (cis_data->cur_height - 1) * cis_data->line_readOut_time;
	cis_data->stream_on = false;

	/* Frame valid time calcuration */
	frame_valid_us = sensor_cis_do_div64((u64)cis_data->cur_height * (u64)cis_data->line_length_pck * (u64)(1000 * 1000), cis_data->pclk);
	cis_data->frame_valid_us_time = (int)frame_valid_us;

	dbg_sensor(1, "%s\n", __func__);
	dbg_sensor(1, "Sensor size(%d x %d) setting: SUCCESS!\n", cis_data->cur_width, cis_data->cur_height);
	dbg_sensor(1, "Frame Valid(us): %d\n", frame_valid_us);
	dbg_sensor(1, "rolling_shutter_skew: %lld\n", cis_data->rolling_shutter_skew);

	dbg_sensor(1, "Fps: %d, max fps(%d)\n", frame_rate, cis_data->max_fps);
	dbg_sensor(1, "min_frame_time(%d us)\n", cis_data->min_frame_us_time);
	dbg_sensor(1, "Pixel rate(Mbps): %d\n", cis_data->pclk / 1000000);

	/* Frame period calculation */
	cis_data->frame_time = (cis_data->line_readOut_time * cis_data->cur_height / 1000);
	cis_data->rolling_shutter_skew = (cis_data->cur_height - 1) * cis_data->line_readOut_time;

	dbg_sensor(1, "[%s] frame_time(%d), rolling_shutter_skew(%lld)\n", __func__,
	cis_data->frame_time, cis_data->rolling_shutter_skew);

	/* Constant values */
	cis_data->min_fine_integration_time = SENSOR_IMX471_FINE_INTEGRATION_TIME_MIN;
	cis_data->max_fine_integration_time = SENSOR_IMX471_FINE_INTEGRATION_TIME_MAX;
	cis_data->min_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MIN;
	cis_data->max_margin_coarse_integration_time = SENSOR_IMX471_COARSE_INTEGRATION_TIME_MAX_MARGIN;
	info("%s: done", __func__);
}

void sensor_imx471_cis_data_calc(struct v4l2_subdev *subdev, u32 mode)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	if (mode > sensor_imx471_max_setfile_num) {
		err("invalid mode(%d)!!", mode);
		return;
	}

	/* If check_rev fail when cis_init, one more check_rev in mode_change */
	if (cis->rev_flag == true) {
		cis->rev_flag = false;
		ret = sensor_cis_check_rev(cis);
		if (ret < 0) {
			err("sensor_imx471_check_rev is fail: ret(%d)", ret);
			return;
		}
	}

	sensor_imx471_cis_data_calculation(sensor_imx471_pllinfos[mode], cis->cis_data);
}

static int sensor_imx471_wait_stream_off_status(cis_shared_data *cis_data)
{
	int ret = 0;
	u32 timeout = 0;

	BUG_ON(!cis_data);

#define STREAM_OFF_WAIT_TIME 250
	while (timeout < STREAM_OFF_WAIT_TIME) {
		if (cis_data->is_active_area == false &&
				cis_data->stream_on == false) {
			pr_debug("actual stream off\n");
			break;
		}
		timeout++;
	}

	if (timeout == STREAM_OFF_WAIT_TIME) {
		pr_err("actual stream off wait timeout\n");
		ret = -1;
	}

	return ret;
}

int sensor_imx471_cis_check_rev(struct fimc_is_cis *cis)
{
	int ret = 0;
	u8 rev = 0, status=0;
	struct i2c_client *client;

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
	}

	I2C_MUTEX_LOCK(cis->i2c_lock);
	/* Specify OTP Page Address for READ - Page63(dec) */
	fimc_is_sensor_write8(client, 0x0A02,  0x3F);

	/* Turn ON OTP Read MODE */
	fimc_is_sensor_write8(client, 0x0A00,  0x01);

	/* Check status - 0x01 : read ready*/
	fimc_is_sensor_read8(client, 0x0A01,  &status);
	if ((status & 0x1) == false)
		err("status fail, (%d)", status);

	/* Readout data 
	 * addr = 0x0018
	 * value = 0x10 ---> MP0 (frist sample)
	 * value = 0x11 ---> MP  (for MP)
	 */
	ret = fimc_is_sensor_read8(client, 0x0018, &rev);
	if (ret < 0) {
		err("fimc_is_sensor_read8 fail (ret %d)", ret);
		I2C_MUTEX_UNLOCK(cis->i2c_lock);
		return ret;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	cis->cis_data->cis_rev = rev;
	probe_info("imx471 rev:%x", rev);

	return 0;
}

/* CIS OPS */
int sensor_imx471_cis_init(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	u32 setfile_index = 0;
	cis_setting_info setinfo;
#ifdef USE_CAMERA_HW_BIG_DATA
	struct cam_hw_param *hw_param = NULL;
	struct fimc_is_device_sensor_peri *sensor_peri = NULL;
#endif
	setinfo.param = NULL;
	setinfo.return_value = 0;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	if (!cis) {
		err("cis is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	BUG_ON(!cis->cis_data);
	memset(cis->cis_data, 0, sizeof(cis_shared_data));

	probe_info("%s imx471 init\n", __func__);
	cis->rev_flag = false;

	ret = sensor_imx471_cis_check_rev(cis);
	if (ret < 0) {
#ifdef USE_CAMERA_HW_BIG_DATA
		sensor_peri = container_of(cis, struct fimc_is_device_sensor_peri, cis);
		if (sensor_peri)
			fimc_is_sec_get_hw_param(&hw_param, sensor_peri->module->position);
		if (hw_param)
			hw_param->i2c_sensor_err_cnt++;
#endif
		warn("sensor_imx471_check_rev is fail when cis init");
		cis->rev_flag = true;
		ret = 0;
	}

	// Check that QSC and DPC Cal is written for Remosaic Capture.
	// false : Not yet write the QSC and DPC
	// true  : Written the QSC and DPC
	sensor_imx471_cal_write_flag = false;

	info("[%s] cis_rev=%#x\n", __func__, cis->cis_data->cis_rev);

	if (cis->cis_data->cis_rev == 0x11) {
		probe_info("%s setfile_A for MP\n", __func__);
		sensor_imx471_global = sensor_imx471_setfile_A_Global;
		sensor_imx471_global_size = sizeof(sensor_imx471_setfile_A_Global) / sizeof(sensor_imx471_setfile_A_Global[0]);
		//sensor_imx471_imageQuality = sensor_imx471_setfile_A_ImageQuality;
		//sensor_imx471_imageQuality_size = sizeof(sensor_imx471_setfile_A_ImageQuality) / sizeof(sensor_imx471_setfile_A_ImageQuality[0]);
		sensor_imx471_setfiles = sensor_imx471_setfiles_A;
		sensor_imx471_setfile_sizes = sensor_imx471_setfile_A_sizes;
		sensor_imx471_pllinfos = sensor_imx471_pllinfos_A;
		sensor_imx471_max_setfile_num = sizeof(sensor_imx471_setfiles_A) / sizeof(sensor_imx471_setfiles_A[0]);
	} else if (cis->cis_data->cis_rev == 0x10) {
		probe_info("%s setfile_B for MP0\n", __func__);
		sensor_imx471_global = sensor_imx471_setfile_B_Global;
		sensor_imx471_global_size = sizeof(sensor_imx471_setfile_B_Global) / sizeof(sensor_imx471_setfile_B_Global[0]);
		//sensor_imx471_imageQuality = sensor_imx471_setfile_B_ImageQuality;
		//sensor_imx471_imageQuality_size = sizeof(sensor_imx471_setfile_B_ImageQuality) / sizeof(sensor_imx471_setfile_B_ImageQuality[0]);
		sensor_imx471_setfiles = sensor_imx471_setfiles_B;
		sensor_imx471_setfile_sizes = sensor_imx471_setfile_B_sizes;
		sensor_imx471_pllinfos = sensor_imx471_pllinfos_B;
		sensor_imx471_max_setfile_num = sizeof(sensor_imx471_setfiles_B) / sizeof(sensor_imx471_setfiles_B[0]);
	} else {
		probe_info("%s chip_rev(%d) is wrong! setfile_B for MP (default)\n", __func__, cis->cis_data->cis_rev);
		sensor_imx471_global = sensor_imx471_setfile_B_Global;
		sensor_imx471_global_size = sizeof(sensor_imx471_setfile_B_Global) / sizeof(sensor_imx471_setfile_B_Global[0]);
		//sensor_imx471_imageQuality = sensor_imx471_setfile_B_ImageQuality;
		//sensor_imx471_imageQuality_size = sizeof(sensor_imx471_setfile_B_ImageQuality) / sizeof(sensor_imx471_setfile_B_ImageQuality[0]);
		sensor_imx471_setfiles = sensor_imx471_setfiles_B;
		sensor_imx471_setfile_sizes = sensor_imx471_setfile_B_sizes;
		sensor_imx471_pllinfos = sensor_imx471_pllinfos_B;
		sensor_imx471_max_setfile_num = sizeof(sensor_imx471_setfiles_B) / sizeof(sensor_imx471_setfiles_B[0]);
	}

	cis->cis_data->product_name = cis->id;
	cis->cis_data->cur_width = SENSOR_IMX471_MAX_WIDTH;
	cis->cis_data->cur_height = SENSOR_IMX471_MAX_HEIGHT;
	cis->cis_data->low_expo_start = 33000;
	cis->need_mode_change = false;
	cis->long_term_mode.sen_strm_off_on_step = 0;

	sensor_imx471_cis_data_calculation(sensor_imx471_pllinfos[setfile_index], cis->cis_data);
	sensor_imx471_set_integration_max_margin(setfile_index, cis->cis_data);
	sensor_imx471_set_integration_min(setfile_index, cis->cis_data);

	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_min_exposure_time, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] min exposure time : %d\n", __func__, setinfo.return_value);
	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_max_exposure_time, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] max exposure time : %d\n", __func__, setinfo.return_value);
	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_min_analog_gain, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] min again : %d\n", __func__, setinfo.return_value);
	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_max_analog_gain, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] max again : %d\n", __func__, setinfo.return_value);
	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_min_digital_gain, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] min dgain : %d\n", __func__, setinfo.return_value);
	setinfo.return_value = 0;
	CALL_CISOPS(cis, cis_get_max_digital_gain, subdev, &setinfo.return_value);
	dbg_sensor(1, "[%s] max dgain : %d\n", __func__, setinfo.return_value);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	return ret;
}

#if SENSOR_IMX471_CAL_DEBUG
int sensor_imx471_cis_cal_dump(char* name, char *buf, size_t size)
{
	int ret = 0;

	struct file *fp;
	ssize_t tx = -ENOENT;
	int fd, old_mask;
	loff_t pos = 0;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	old_mask = sys_umask(0);

	sys_rmdir(name);
	fd = sys_open(name, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
	if (fd < 0) {
		err("open file error(%d): %s", fd, name);
		sys_umask(old_mask);
		set_fs(old_fs);
		ret = -EINVAL;
		goto p_err;
	}

	fp = fget(fd);
	if (fp) {
		tx = vfs_write(fp, buf, size, &pos);
		if (tx != size) {
			err("fail to write %s. ret %zd", name, tx);
			ret = -ENOENT;
		}

		vfs_fsync(fp, 0);
		fput(fp);
	} else {
		err("fail to get file *: %s", name);
	}

	sys_close(fd);
	sys_umask(old_mask);
	set_fs(old_fs);

p_err:
	return ret;
}
#endif

int sensor_imx471_cis_QuadSensCal_write(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client = NULL;
	struct fimc_is_device_sensor_peri *sensor_peri = NULL;
	struct fimc_is_lib_support *lib = &gPtr_lib_support;
	int position;
	u16 start_addr;
	ulong cal_addr;
	u16 data_size;
	u8 cal_data[SENSOR_IMX471_QUAD_SENS_CAL_SIZE] = {0, };

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	sensor_peri = container_of(cis, struct fimc_is_device_sensor_peri, cis);
	BUG_ON(!sensor_peri);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		return -EINVAL;
	}

	position = sensor_peri->module->position;
	if (position == SENSOR_POSITION_REAR)
		cal_addr = lib->minfo->kvaddr_rear_cal + SENSOR_IMX471_QUAD_SENS_CAL_BASE_REAR;
	else if (position == SENSOR_POSITION_FRONT)
		cal_addr = lib->minfo->kvaddr_front_cal + SENSOR_IMX471_QUAD_SENS_CAL_BASE_FRONT;
	else {
		err("cis_imx471 position(%d) is invalid!\n", position);
		goto p_err;
	}

	memcpy(cal_data, (u16 *)cal_addr, SENSOR_IMX471_QUAD_SENS_CAL_SIZE);

#if SENSOR_IMX471_CAL_DEBUG
	ret = sensor_imx471_cis_cal_dump(SENSOR_IMX471_QSC_DUMP_NAME, (char *)cal_data, (size_t)SENSOR_IMX471_QUAD_SENS_CAL_SIZE);
	if (ret < 0) {
		err("cis_imx471 QSC Cal dump fail(%d)!\n", ret);
		goto p_err;
	}
#endif

	I2C_MUTEX_LOCK(cis->i2c_lock);

	start_addr = SENSOR_IMX471_QUAD_SENS_REG_ADDR;
	data_size = SENSOR_IMX471_QUAD_SENS_CAL_SIZE;
	ret = fimc_is_sensor_write8_sequential(client, start_addr, cal_data, data_size);
	if (ret < 0) {
		err("cis_imx471 QSC write Error(%d)\n", ret);
	}

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);
	return ret;
}

int sensor_imx471_cis_DPC_write(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client = NULL;
	struct fimc_is_device_sensor_peri *sensor_peri = NULL;
	struct fimc_is_lib_support *lib = &gPtr_lib_support;
	int position;
	u16 start_val[2] = {0, };
	u16 count_val[2] = {0, };
	u16 start_addr = 0;
	u16 data_size = 0;
	ulong cal_addr;
	u8 *cal_data = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	sensor_peri = container_of(cis, struct fimc_is_device_sensor_peri, cis);
	BUG_ON(!sensor_peri);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		return -EINVAL;
	}

	position = sensor_peri->module->position;
	if (position == SENSOR_POSITION_REAR)
		cal_addr = lib->minfo->kvaddr_rear_cal + SENSOR_IMX471_DPC_CAL_BASE_REAR;
	else if (position == SENSOR_POSITION_FRONT)
		cal_addr = lib->minfo->kvaddr_front_cal + SENSOR_IMX471_DPC_CAL_BASE_FRONT;
	else {
		err("cis_imx471 position(%d) is invalid!\n", position);
		goto p_err;
	}

	cal_data = kzalloc(SENSOR_IMX471_DPC_CAL_SIZE, GFP_KERNEL);
	if (!cal_data) {
		err("cis_imx471 cal_data alloc fail");
		ret = -ENOMEM;
		goto p_err;
	}

	memcpy(cal_data, (u16 *)cal_addr, SENSOR_IMX471_DPC_CAL_SIZE);

#if SENSOR_IMX471_CAL_DEBUG
	ret = sensor_imx471_cis_cal_dump(SENSOR_IMX471_DPC_DUMP_NAME, (char *)cal_data, (size_t)SENSOR_IMX471_DPC_CAL_SIZE);
	if (ret < 0) {
		err("cis_imx471 QSC Cal dump fail(%d)!\n", ret);
		goto p_err;
	}
#endif

	start_val[0] = cal_data[SENSOR_IMX471_DPC_CAL_SIZE - 4];
	start_val[1] = cal_data[SENSOR_IMX471_DPC_CAL_SIZE - 3];
	count_val[0] = cal_data[SENSOR_IMX471_DPC_CAL_SIZE - 2];
	count_val[1] = cal_data[SENSOR_IMX471_DPC_CAL_SIZE - 1];
	start_addr = ((((start_val[0] & 0x00FF) << 8) & 0xFF00) | (start_val[1] & 0x00FF));
	data_size = ((((count_val[0] & 0x00FF) << 8) & 0xFF00) | (count_val[1] & 0x00FF));

	I2C_MUTEX_LOCK(cis->i2c_lock);
	if (start_addr < 0xFFFF && data_size > 0x0) {
		ret = fimc_is_sensor_write8_sequential(client, start_addr, cal_data, data_size);
		if (ret < 0) {
			err("cis_imx471 DPC write Error(%d).\n", ret);
		}
	}

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	if(cal_data)
		kfree(cal_data);
	
	return ret;
}

int sensor_imx471_cis_log_status(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client = NULL;
	u8 data8 = 0;
	u16 data16 = 0;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	if (!cis) {
		err("cis is NULL");
		ret = -ENODEV;
		goto p_err;
	}

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -ENODEV;
		goto p_err;
	}

	I2C_MUTEX_LOCK(cis->i2c_lock);
	pr_err("[SEN:DUMP] *******************************\n");
	fimc_is_sensor_read16(client, 0x0000, &data16);
	pr_err("[SEN:DUMP] model_id(%x)\n", data16);
	fimc_is_sensor_read8(client, 0x0002, &data8);
	pr_err("[SEN:DUMP] revision_number(%x)\n", data8);
	fimc_is_sensor_read8(client, 0x0005, &data8);
	pr_err("[SEN:DUMP] frame_count(%x)\n", data8);
	fimc_is_sensor_read8(client, 0x0100, &data8);
	pr_err("[SEN:DUMP] mode_select(%x)\n", data8);

	sensor_cis_dump_registers(subdev, sensor_imx471_setfiles[0], sensor_imx471_setfile_sizes[0]);
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	pr_err("[SEN:DUMP] *******************************\n");

p_err:
	return ret;
}

#if USE_GROUP_PARAM_HOLD
static int sensor_imx471_cis_group_param_hold_func(struct v4l2_subdev *subdev, unsigned int hold)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;
	struct i2c_client *client = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	if (hold == cis->cis_data->group_param_hold) {
		pr_debug("already group_param_hold (%d)\n", cis->cis_data->group_param_hold);
		goto p_err;
	}

	ret = fimc_is_sensor_write8(client, 0x0104, hold);
	if (ret < 0)
		goto p_err;

	cis->cis_data->group_param_hold = hold;
	ret = 1;
p_err:
	return ret;
}
#else
static inline int sensor_imx471_cis_group_param_hold_func(struct v4l2_subdev *subdev, unsigned int hold)
{ return 0; }
#endif

/* Input
 *   hold : true - hold, flase - no hold
 * Output
 *   return: 0 - no effect(already hold or no hold)
 *   positive - setted by request
 *   negative - ERROR value
 */
int sensor_imx471_cis_group_param_hold(struct v4l2_subdev *subdev, bool hold)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	ret = sensor_imx471_cis_group_param_hold_func(subdev, hold);
	if (ret < 0)
		goto p_err;

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);
	return ret;
}

int sensor_imx471_cis_read_dpc(struct v4l2_subdev *subdev)
{
	int i = 0;
	int j = 0;
	int ret = 0;
	int index = 0;
	int extra_size = 0;
	u8 *temp_buf = NULL;
	struct fimc_is_cis *cis = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);

	temp_buf = kzalloc(sizeof(u8) * 400, GFP_KERNEL);
	if (!temp_buf) {
		probe_err("temp_buf is NULL");
		ret = -ENOMEM;
		goto p_err;
	}

	memset(imx471_dpc.fd_dfct_data, 0x0, sizeof(*imx471_dpc.fd_dfct_data));
	memset(temp_buf, 0x0, sizeof(*temp_buf));

	I2C_MUTEX_LOCK(cis->i2c_lock);
	fimc_is_sensor_read16(cis->client, 0x7678, &imx471_dpc.fd_num);

	index = (imx471_dpc.fd_num * 25) / 8;
	if ((imx471_dpc.fd_num * 25) % 8 != 0)
		index++;
	
	info("[%s] fd dfct num(%x), index = %d\n", __func__, imx471_dpc.fd_num, index);

	for (i = 0; i < index; i++) {
		fimc_is_sensor_read8(cis->client, 0x8B00 + i, temp_buf + i);
		dbg_sensor(1, "fd dfct (0x%02x)\n", *(temp_buf + i));
	}

	extra_size = 1;
	for (i = 0, j = 0; j < imx471_dpc.fd_num; j++) {
		imx471_dpc.fd_dfct_data[j] = (((*(temp_buf + i) >> (extra_size - 1)) << 24) | (*(temp_buf + i + 1) << 16) \
			| (*(temp_buf + i + 2) << 8) | ((*(temp_buf + i + 3) >> (8 - extra_size)) << (8 - extra_size))) >> (8 - extra_size);

		if (++extra_size > 8)
			extra_size = 1;
		if (extra_size == 8)
			i = i + 4;
		else
			i = i + 3;
	}

	memset(imx471_dpc.sg_dfct_data, 0x0, sizeof(imx471_dpc.sg_dfct_data));
	memset(temp_buf, 0x0, sizeof(*temp_buf));

	fimc_is_sensor_read16(cis->client, 0x767A, &imx471_dpc.sg_num);

	index = (imx471_dpc.sg_num * 25) / 8;
	if ((imx471_dpc.sg_num * 25) % 8 != 0)
		index++;
	
	info("[%s] sg dfct num(%x), index = %d\n", __func__, imx471_dpc.sg_num, index);

	for (i = 0; i < index; i++) {
		fimc_is_sensor_read8(cis->client, 0x8B10 + i, temp_buf + i);
		dbg_sensor(1, "sg dfct (0x%02x)\n", *(temp_buf + i));
	}

	for (i = 0, j = 0; j < imx471_dpc.sg_num; j++) {
		imx471_dpc.sg_dfct_data[j] = (((*(temp_buf + i) >> (extra_size - 1)) << 24) | (*(temp_buf + i + 1) << 16) \
			| (*(temp_buf + i + 2) << 8) | ((*(temp_buf + i + 3) >> (8 - extra_size)) << (8 - extra_size))) >> (8 - extra_size);

		if (++extra_size > 8)
			extra_size = 1;
		if (extra_size == 8)
			i = i + 4;
		else
			i = i + 3;
	}

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	if(temp_buf)
		kfree(temp_buf);

	return ret;
}

int sensor_imx471_cis_set_global_setting(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	/* setfile global setting is at camera entrance */
	info("[%s] global setting start\n", __func__);
	ret = sensor_cis_set_registers(subdev, sensor_imx471_global, sensor_imx471_global_size);
	if (ret < 0) {
		err("sensor_imx471_set_global registers fail!!");
		goto p_err;
	}
	dbg_sensor(1, "[%s] global setting done\n", __func__);

	if (!sensor_imx471_read_dpc) {
		sensor_imx471_cis_read_dpc(subdev);
		sensor_imx471_read_dpc = true;
	}

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	// Check that QSC and DPC Cal is written for Remosaic Capture.
	// false : Not yet write the QSC and DPC
	// true  : Written the QSC and DPC
	sensor_imx471_cal_write_flag = false;
	return ret;
}

int sensor_imx471_cis_mode_change(struct v4l2_subdev *subdev, u32 mode)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	if (mode > sensor_imx471_max_setfile_num) {
		err("invalid mode(%d)!!", mode);
		ret = -EINVAL;
		goto p_err;
	}

	/* If check_rev(Sensor ID in OTP) of IMX471 fail when cis_init, one more check_rev in mode_change */
	if (cis->rev_flag == true) {
		cis->rev_flag = false;
		ret = sensor_cis_check_rev(cis);
		if (ret < 0) {
			err("sensor_imx471_check_rev is fail");
			goto p_err;
		}
		info("[%s] cis_rev=%#x\n", __func__, cis->cis_data->cis_rev);
	}

#if 0 /* cis_data_calculation is called in module_s_format */
	sensor_imx471_cis_data_calculation(sensor_imx471_pllinfos[mode], cis->cis_data);
#endif
	sensor_imx471_set_integration_max_margin(mode, cis->cis_data);

	I2C_MUTEX_LOCK(cis->i2c_lock);

	info("[%s] mode=%d, mode change setting start\n", __func__, mode);
	ret = sensor_cis_set_registers(subdev, sensor_imx471_setfiles[mode], sensor_imx471_setfile_sizes[mode]);
	if (ret < 0) {
		err("sensor_imx471_set_registers fail!!");
		goto p_err;
	}
	dbg_sensor(1, "[%s] mode changed(%d)\n", __func__, mode);
#if 0
	if (mode >= SENSOR_IMX471_5664X4248_QBCREMOSAIC_30FPS
		&& mode <= SENSOR_IMX471_4248X4248_QBCREMOSAIC_30FPS
		&& sensor_imx471_cal_write_flag == false) {
		sensor_imx471_cal_write_flag = true;
		
		info("[%s] %d mode is QBC Remosaic Mode! Write QSC and DPC data.\n", __func__, mode);
		ret = sensor_imx471_cis_QuadSensCal_write(subdev);
		if (ret < 0) {
			err("sensor_imx471_Quad_Sens_Cal_write fail!! (%d)", ret);
			goto p_err;
		}
		ret = sensor_imx471_cis_DPC_write(subdev);
		if (ret < 0) {
			err("sensor_imx471_DPC_write fail!! (%d)", ret);
			goto p_err;
		}
	}
#endif

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);
	return ret;
}

/* TODO: Sensor set size sequence(sensor done, sensor stop, 3AA done in FW case */
int sensor_imx471_cis_set_size(struct v4l2_subdev *subdev, cis_shared_data *cis_data)
{
	int ret = 0;
	bool binning = false;
	u32 ratio_w = 0, ratio_h = 0, start_x = 0, start_y = 0, end_x = 0, end_y = 0;
	u32 even_x= 0, odd_x = 0, even_y = 0, odd_y = 0;
	struct i2c_client *client = NULL;
	struct fimc_is_cis *cis = NULL;
#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif
	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	BUG_ON(!cis);

	dbg_sensor(1, "[MOD:D:%d] %s\n", cis->id, __func__);

	if (unlikely(!cis_data)) {
		err("cis data is NULL");
		if (unlikely(!cis->cis_data)) {
			ret = -EINVAL;
			goto p_err;
		} else {
			cis_data = cis->cis_data;
		}
	}

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	/* Wait actual stream off */
	ret = sensor_imx471_wait_stream_off_status(cis_data);
	if (ret) {
		err("Must stream off\n");
		ret = -EINVAL;
		goto p_err;
	}

	binning = cis_data->binning;
	if (binning) {
		ratio_w = (SENSOR_IMX471_MAX_WIDTH / cis_data->cur_width);
		ratio_h = (SENSOR_IMX471_MAX_HEIGHT / cis_data->cur_height);
	} else {
		ratio_w = 1;
		ratio_h = 1;
	}

	if (((cis_data->cur_width * ratio_w) > SENSOR_IMX471_MAX_WIDTH) ||
		((cis_data->cur_height * ratio_h) > SENSOR_IMX471_MAX_HEIGHT)) {
		err("Config max sensor size over~!!\n");
		ret = -EINVAL;
		goto p_err;
	}

	I2C_MUTEX_LOCK(cis->i2c_lock);
	/* 1. page_select */
	ret = fimc_is_sensor_write16(client, 0x6028, 0x2000);
	if (ret < 0)
		 goto p_err;

	/* 2. pixel address region setting */
	start_x = ((SENSOR_IMX471_MAX_WIDTH - cis_data->cur_width * ratio_w) / 2) & (~0x1);
	start_y = ((SENSOR_IMX471_MAX_HEIGHT - cis_data->cur_height * ratio_h) / 2) & (~0x1);
	end_x = start_x + (cis_data->cur_width * ratio_w - 1);
	end_y = start_y + (cis_data->cur_height * ratio_h - 1);

	if (!(end_x & (0x1)) || !(end_y & (0x1))) {
		err("Sensor pixel end address must odd\n");
		ret = -EINVAL;
		goto p_err;
	}

	ret = fimc_is_sensor_write16(client, 0x0344, start_x);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x0346, start_y);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x0348, end_x);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x034A, end_y);
	if (ret < 0)
		 goto p_err;

	/* 3. output address setting */
	ret = fimc_is_sensor_write16(client, 0x034C, cis_data->cur_width);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x034E, cis_data->cur_height);
	if (ret < 0)
		 goto p_err;

	/* If not use to binning, sensor image should set only crop */
	if (!binning) {
		dbg_sensor(1, "Sensor size set is not binning\n");
		goto p_err;
	}

	/* 4. sub sampling setting */
	even_x = 1;	/* 1: not use to even sampling */
	even_y = 1;
	odd_x = (ratio_w * 2) - even_x;
	odd_y = (ratio_h * 2) - even_y;

	ret = fimc_is_sensor_write16(client, 0x0380, even_x);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x0382, odd_x);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x0384, even_y);
	if (ret < 0)
		 goto p_err;
	ret = fimc_is_sensor_write16(client, 0x0386, odd_y);
	if (ret < 0)
		 goto p_err;

	/* 5. binnig setting */
	ret = fimc_is_sensor_write8(client, 0x0900, binning);	/* 1:  binning enable, 0: disable */
	if (ret < 0)
		goto p_err;
	ret = fimc_is_sensor_write8(client, 0x0901, (ratio_w << 4) | ratio_h);
	if (ret < 0)
		goto p_err;

	/* 6. scaling setting: but not use */
	/* scaling_mode (0: No scaling, 1: Horizontal, 2: Full) */
	ret = fimc_is_sensor_write16(client, 0x0400, 0x0000);
	if (ret < 0)
		goto p_err;
	/* down_scale_m: 1 to 16 upwards (scale_n: 16(fixed))
	down scale factor = down_scale_m / down_scale_n */
	ret = fimc_is_sensor_write16(client, 0x0404, 0x0010);
	if (ret < 0)
		goto p_err;

	cis_data->frame_time = (cis_data->line_readOut_time * cis_data->cur_height / 1000);
	cis->cis_data->rolling_shutter_skew = (cis->cis_data->cur_height - 1) * cis->cis_data->line_readOut_time;
	dbg_sensor(1, "[%s] frame_time(%d), rolling_shutter_skew(%lld)\n", __func__, cis->cis_data->frame_time,
				cis->cis_data->rolling_shutter_skew);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec) * 1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	I2C_MUTEX_UNLOCK(cis->i2c_lock);
	return ret;
}

int sensor_imx471_cis_stream_on(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	cis_shared_data *cis_data;
	struct fimc_is_device_sensor_peri *sensor_peri = NULL;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	sensor_peri = container_of(cis, struct fimc_is_device_sensor_peri, cis);
	BUG_ON(!sensor_peri);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	cis_data = cis->cis_data;

	dbg_sensor(1, "[MOD:D:%d] %s\n", cis->id, __func__);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	sensor_imx471_cis_group_param_hold_func(subdev, 0x01);

#ifdef DEBUG_IMX471_PLL
	{
	u16 pll;
	fimc_is_sensor_read16(client, 0x0300, &pll);
	dbg_sensor(1, "______ vt_pix_clk_div(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0302, &pll);
	dbg_sensor(1, "______ vt_sys_clk_div(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0304, &pll);
	dbg_sensor(1, "______ pre_pll_clk_div(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0306, &pll);
	dbg_sensor(1, "______ pll_multiplier(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x030a, &pll);
	dbg_sensor(1, "______ op_sys_clk_div(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x030c, &pll);
	dbg_sensor(1, "______ op_prepllck_div(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x030e, &pll);
	dbg_sensor(1, "______ op_pll_multiplier(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0310, &pll);
	dbg_sensor(1, "______ pll_mult_driv(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0340, &pll);
	dbg_sensor(1, "______ frame_length_lines(%x)\n", pll);
	fimc_is_sensor_read16(client, 0x0342, &pll);
	dbg_sensor(1, "______ line_length_pck(%x)\n", pll);
	}
#endif

	info("[%s] start\n", __func__);
	/* here Add for Master mode in dual */
	fimc_is_sensor_write8(client, 0x3040, 0x01);
	fimc_is_sensor_write8(client, 0x3F71, 0x01);
	/* Sensor stream on */
	fimc_is_sensor_write8(client, 0x0100, 0x01);

	sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	cis_data->stream_on = true;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	return ret;
}

int sensor_imx471_cis_stream_off(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	cis_shared_data *cis_data;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	cis_data = cis->cis_data;

	dbg_sensor(1, "[MOD:D:%d] %s\n", cis->id, __func__);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	sensor_imx471_cis_group_param_hold_func(subdev, 0x00);

	fimc_is_sensor_write8(client, 0x0100, 0x00);
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	cis_data->stream_on = false;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	return ret;
}

int sensor_imx471_cis_set_exposure_time(struct v4l2_subdev *subdev, struct ae_param *target_exposure)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	cis_shared_data *cis_data;

	u32 vt_pic_clk_freq_mhz = 0;
	u16 long_coarse_int = 0;
	u16 short_coarse_int = 0;
	u32 line_length_pck = 0;
	u32 min_fine_int = 0;
	u8 arrayBuf[4];

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!target_exposure);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	if ((target_exposure->long_val <= 0) || (target_exposure->short_val <= 0)) {
		err("[%s] invalid target exposure(%d, %d)\n", __func__,
				target_exposure->long_val, target_exposure->short_val);
		ret = -EINVAL;
		goto p_err;
	}

	cis_data = cis->cis_data;

	dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), target long(%d), short(%d)\n", cis->id, __func__,
			cis_data->sen_vsync_count, target_exposure->long_val, target_exposure->short_val);

	vt_pic_clk_freq_mhz = cis_data->pclk / (1000 * 1000);
	line_length_pck = cis_data->line_length_pck;
	min_fine_int = cis_data->min_fine_integration_time;

	long_coarse_int = ((target_exposure->long_val * vt_pic_clk_freq_mhz) - min_fine_int) / line_length_pck;
	short_coarse_int = ((target_exposure->short_val * vt_pic_clk_freq_mhz) - min_fine_int) / line_length_pck;

	if (long_coarse_int > cis_data->max_coarse_integration_time) {
		dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), long coarse(%d) max(%d)\n", cis->id, __func__,
			cis_data->sen_vsync_count, long_coarse_int, cis_data->max_coarse_integration_time);
		long_coarse_int = cis_data->max_coarse_integration_time;
	}

	if (short_coarse_int > cis_data->max_coarse_integration_time) {
		dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), short coarse(%d) max(%d)\n", cis->id, __func__,
			cis_data->sen_vsync_count, short_coarse_int, cis_data->max_coarse_integration_time);
		short_coarse_int = cis_data->max_coarse_integration_time;
	}

	if (long_coarse_int < cis_data->min_coarse_integration_time) {
		dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), long coarse(%d) min(%d)\n", cis->id, __func__,
			cis_data->sen_vsync_count, long_coarse_int, cis_data->min_coarse_integration_time);
		long_coarse_int = cis_data->min_coarse_integration_time;
	}

	if (short_coarse_int < cis_data->min_coarse_integration_time) {
		dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), short coarse(%d) min(%d)\n", cis->id, __func__,
			cis_data->sen_vsync_count, short_coarse_int, cis_data->min_coarse_integration_time);
		short_coarse_int = cis_data->min_coarse_integration_time;
	}

	cis_data->cur_long_exposure_coarse = long_coarse_int;
	cis_data->cur_short_exposure_coarse = short_coarse_int;

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	//Long exposure
	arrayBuf[0] = (cis_data->cur_long_exposure_coarse & 0xFF00) >> 8;
	arrayBuf[1] = cis_data->cur_long_exposure_coarse & 0xFF;
	ret = fimc_is_sensor_write8_array(client, SENSOR_IMX471_COARSE_INTEG_TIME_ADDR, arrayBuf, 2);
	if (ret < 0)
		goto p_err;

	dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), vt_pic_clk_freq_mhz (%d),"
		KERN_CONT "line_length_pck(%d), min_fine_int (%d)\n", cis->id, __func__,
		cis_data->sen_vsync_count, vt_pic_clk_freq_mhz, line_length_pck, min_fine_int);
	dbg_sensor(1, "[MOD:D:%d] %s, vsync_cnt(%d), frame_length_lines(%#x),"
		KERN_CONT "long_coarse_int %#x, short_coarse_int %#x\n", cis->id, __func__,
		cis_data->sen_vsync_count, cis_data->frame_length_lines, long_coarse_int, short_coarse_int);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_get_min_exposure_time(struct v4l2_subdev *subdev, u32 *min_expo)
{
	int ret = 0;
	struct fimc_is_cis *cis = NULL;
	cis_shared_data *cis_data = NULL;
	u32 min_integration_time = 0;
	u32 min_coarse = 0;
	u32 min_fine = 0;
	u32 vt_pic_clk_freq_mhz = 0;
	u32 line_length_pck = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!min_expo);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	vt_pic_clk_freq_mhz = cis_data->pclk / (1000 * 1000);
	if (vt_pic_clk_freq_mhz == 0) {
		pr_err("[MOD:D:%d] %s, Invalid vt_pic_clk_freq_mhz(%d)\n", cis->id, __func__, vt_pic_clk_freq_mhz);
		goto p_err;
	}
	line_length_pck = cis_data->line_length_pck;
	min_coarse = cis_data->min_coarse_integration_time;
	min_fine = cis_data->min_fine_integration_time;

	min_integration_time = ((line_length_pck * min_coarse) + min_fine) / vt_pic_clk_freq_mhz;
	*min_expo = min_integration_time;

	dbg_sensor(1, "[%s] min integration time %d\n", __func__, min_integration_time);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	return ret;
}

int sensor_imx471_cis_get_max_exposure_time(struct v4l2_subdev *subdev, u32 *max_expo)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;
	u32 max_integration_time = 0;
	u32 max_coarse_margin = 0;
	u32 max_fine_margin = 0;
	u32 max_coarse = 0;
	u32 max_fine = 0;
	u32 vt_pic_clk_freq_mhz = 0;
	u32 line_length_pck = 0;
	u32 frame_length_lines = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!max_expo);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	vt_pic_clk_freq_mhz = cis_data->pclk / (1000 * 1000);
	if (vt_pic_clk_freq_mhz == 0) {
		pr_err("[MOD:D:%d] %s, Invalid vt_pic_clk_freq_mhz(%d)\n", cis->id, __func__, vt_pic_clk_freq_mhz);
		goto p_err;
	}
	line_length_pck = cis_data->line_length_pck;
	frame_length_lines = cis_data->frame_length_lines;

	max_coarse_margin = cis_data->max_margin_coarse_integration_time;
	max_fine_margin = line_length_pck - cis_data->min_fine_integration_time;
	max_coarse = frame_length_lines - max_coarse_margin;
	max_fine = cis_data->max_fine_integration_time;

	max_integration_time = ((line_length_pck * max_coarse) + max_fine) / vt_pic_clk_freq_mhz;

	*max_expo = max_integration_time;

	/* TODO: Is this values update hear? */
	cis_data->max_margin_fine_integration_time = max_fine_margin;
	cis_data->max_coarse_integration_time = max_coarse;

	dbg_sensor(1, "[%s] max integration time %d, max margin fine integration %d, max coarse integration %d\n",
			__func__, max_integration_time, cis_data->max_margin_fine_integration_time, cis_data->max_coarse_integration_time);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	return ret;
}

int sensor_imx471_cis_adjust_frame_duration(struct v4l2_subdev *subdev,
						u32 input_exposure_time,
						u32 *target_duration)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;

	u32 vt_pic_clk_freq_mhz = 0;
	u32 line_length_pck = 0;
	u32 frame_length_lines = 0;
	u32 frame_duration = 0;
	u32 max_frame_us_time = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!target_duration);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	vt_pic_clk_freq_mhz = cis_data->pclk / (1000 * 1000);
	line_length_pck = cis_data->line_length_pck;
	frame_length_lines = ((vt_pic_clk_freq_mhz * input_exposure_time) / line_length_pck);
	frame_length_lines += cis_data->max_margin_coarse_integration_time;

	frame_duration = (frame_length_lines * line_length_pck) / vt_pic_clk_freq_mhz;
	max_frame_us_time = 1000000/cis->min_fps;

	dbg_sensor(1, "[%s](vsync cnt = %d) input exp(%d), adj duration, frame duraion(%d), min_frame_us(%d)\n",
			__func__, cis_data->sen_vsync_count, input_exposure_time, frame_duration, cis_data->min_frame_us_time);
	dbg_sensor(1, "[%s](vsync cnt = %d) adj duration, frame duraion(%d), min_frame_us(%d), max_frame_us_time(%d)\n",
			__func__, cis_data->sen_vsync_count, frame_duration, cis_data->min_frame_us_time, max_frame_us_time);

	*target_duration = MAX(frame_duration, cis_data->min_frame_us_time);
	*target_duration = MIN(frame_duration, max_frame_us_time);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

	return ret;
}

int sensor_imx471_cis_set_frame_duration(struct v4l2_subdev *subdev, u32 frame_duration)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	cis_shared_data *cis_data;

	u32 vt_pic_clk_freq_mhz = 0;
	u32 line_length_pck = 0;
	u16 frame_length_lines = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	cis_data = cis->cis_data;

	if (frame_duration < cis_data->min_frame_us_time) {
		dbg_sensor(1, "frame duration is less than min(%d)\n", frame_duration);
		frame_duration = cis_data->min_frame_us_time;
	}

	vt_pic_clk_freq_mhz = cis_data->pclk / (1000 * 1000);
	line_length_pck = cis_data->line_length_pck;

	frame_length_lines = (u16)((vt_pic_clk_freq_mhz * frame_duration) / line_length_pck);

	dbg_sensor(1, "[MOD:D:%d] %s, vt_pic_clk_freq_mhz(%#x) frame_duration = %d us,"
			KERN_CONT "(line_length_pck%#x), frame_length_lines(%#x)\n",
			cis->id, __func__, vt_pic_clk_freq_mhz, frame_duration,
			line_length_pck, frame_length_lines);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	ret = fimc_is_sensor_write16(client, 0x0340, frame_length_lines);
	if (ret < 0)
		goto p_err;

	cis_data->cur_frame_us_time = frame_duration;
	cis_data->frame_length_lines = frame_length_lines;
	cis_data->max_coarse_integration_time = cis_data->frame_length_lines - cis_data->max_margin_coarse_integration_time;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_set_frame_rate(struct v4l2_subdev *subdev, u32 min_fps)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;

	u32 frame_duration = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	if (min_fps > cis_data->max_fps) {
		err("[MOD:D:%d] %s, request FPS is too high(%d), set to max(%d)\n",
			cis->id, __func__, min_fps, cis_data->max_fps);
		min_fps = cis_data->max_fps;
	}

	if (min_fps == 0) {
		err("[MOD:D:%d] %s, request FPS is 0, set to min FPS(1)\n",
			cis->id, __func__);
		min_fps = 1;
	}

	frame_duration = (1 * 1000 * 1000) / min_fps;

	dbg_sensor(1, "[MOD:D:%d] %s, set FPS(%d), frame duration(%d)\n",
			cis->id, __func__, min_fps, frame_duration);

	ret = sensor_imx471_cis_set_frame_duration(subdev, frame_duration);
	if (ret < 0) {
		err("[MOD:D:%d] %s, set frame duration is fail(%d)\n",
			cis->id, __func__, ret);
		goto p_err;
	}

	cis_data->min_frame_us_time = frame_duration;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:

	return ret;
}

u32 sensor_imx471_cis_calc_again_code(u32 permille)
{
	return 1024 - (1024000 / permille);
}

u32 sensor_imx471_cis_calc_again_permile(u32 code)
{
	return 1024000 / (1024 - code);
}

int sensor_imx471_cis_adjust_analog_gain(struct v4l2_subdev *subdev, u32 input_again, u32 *target_permile)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;

	u32 again_code = 0;
	u32 again_permile = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!target_permile);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	again_code = sensor_cis_calc_again_code(input_again);

	if (again_code > cis_data->max_analog_gain[0]) {
		again_code = cis_data->max_analog_gain[0];
	} else if (again_code < cis_data->min_analog_gain[0]) {
		again_code = cis_data->min_analog_gain[0];
	}

	again_permile = sensor_imx471_cis_calc_again_permile(again_code);

	dbg_sensor(1, "[%s] min again(%d), max(%d), input_again(%d), code(%d), permile(%d)\n", __func__,
			cis_data->max_analog_gain[0],
			cis_data->min_analog_gain[0],
			input_again,
			again_code,
			again_permile);

	*target_permile = again_permile;

	return ret;
}

int sensor_imx471_cis_set_analog_gain(struct v4l2_subdev *subdev, struct ae_param *again)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;

	u16 analog_gain = 0;
	u8 arrayBuf[2];

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!again);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	analog_gain = (u16)sensor_imx471_cis_calc_again_code(again->val);

	if (analog_gain < cis->cis_data->min_analog_gain[0]) {
		analog_gain = cis->cis_data->min_analog_gain[0];
	}

	if (analog_gain > cis->cis_data->max_analog_gain[0]) {
		analog_gain = cis->cis_data->max_analog_gain[0];
	}

	dbg_sensor(1, "[MOD:D:%d] %s(vsync cnt = %d), input_again = %d us, analog_gain(%#x)\n",
		cis->id, __func__, cis->cis_data->sen_vsync_count, again->val, analog_gain);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	// Analog gain
	arrayBuf[0] = (analog_gain & 0xFF00) >> 8;
	arrayBuf[1] = analog_gain & 0xFF;
	ret = fimc_is_sensor_write8_array(client, SENSOR_IMX471_ANALOG_GAIN_ADDR, arrayBuf, 2);
	if (ret < 0)
		goto p_err;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_get_analog_gain(struct v4l2_subdev *subdev, u32 *again)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;

	u16 analog_gain = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!again);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	ret = fimc_is_sensor_read16(client, SENSOR_IMX471_ANALOG_GAIN_ADDR, &analog_gain);
	if (ret < 0)
		goto p_err;

	*again = sensor_imx471_cis_calc_again_permile(analog_gain);

	dbg_sensor(1, "[MOD:D:%d] %s, cur_again = %d us, analog_gain(%#x)\n",
			cis->id, __func__, *again, analog_gain);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_get_min_analog_gain(struct v4l2_subdev *subdev, u32 *min_again)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;
	u16 read_value = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!min_again);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	read_value = SENSOR_IMX471_MIN_ANALOG_GAIN_SET_VALUE;

	cis_data->min_analog_gain[0] = read_value;
	cis_data->min_analog_gain[1] = sensor_imx471_cis_calc_again_permile(cis_data->min_analog_gain[0]);
	*min_again = cis_data->min_analog_gain[1];

	dbg_sensor(1, "[%s] code %d, permile %d\n", __func__, cis_data->min_analog_gain[0],
		cis_data->min_analog_gain[1]);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

	return ret;
}

int sensor_imx471_cis_get_max_analog_gain(struct v4l2_subdev *subdev, u32 *max_again)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;
	u16 read_value = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!max_again);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;

	read_value = SENSOR_IMX471_MAX_ANALOG_GAIN_SET_VALUE;

	cis_data->max_analog_gain[0] = read_value;
	cis_data->max_analog_gain[1] = sensor_imx471_cis_calc_again_permile(cis_data->max_analog_gain[0]);
	*max_again = cis_data->max_analog_gain[1];

	dbg_sensor(1, "[%s] code %d, permile %d\n", __func__, cis_data->max_analog_gain[0],
		cis_data->max_analog_gain[1]);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

	return ret;
}

int sensor_imx471_cis_set_digital_gain(struct v4l2_subdev *subdev, struct ae_param *dgain)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	cis_shared_data *cis_data;

	u16 long_gain = 0;
	u16 short_gain = 0;
	u8 dgains[2] = {0};

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!dgain);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	cis_data = cis->cis_data;

	long_gain = (u16)sensor_cis_calc_dgain_code(dgain->long_val);
	short_gain = (u16)sensor_cis_calc_dgain_code(dgain->short_val);

	if (long_gain < cis->cis_data->min_digital_gain[0]) {
		long_gain = cis->cis_data->min_digital_gain[0];
	}
	if (long_gain > cis->cis_data->max_digital_gain[0]) {
		long_gain = cis->cis_data->max_digital_gain[0];
	}

	if (short_gain < cis->cis_data->min_digital_gain[0]) {
		short_gain = cis->cis_data->min_digital_gain[0];
	}
	if (short_gain > cis->cis_data->max_digital_gain[0]) {
		short_gain = cis->cis_data->max_digital_gain[0];
	}

	dbg_sensor(1, "[MOD:D:%d] %s(vsync cnt = %d), input_dgain = %d/%d us,"
			KERN_CONT "long_gain(%#x), short_gain(%#x)\n",
			cis->id, __func__, cis->cis_data->sen_vsync_count,
			dgain->long_val, dgain->short_val, long_gain, short_gain);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	// Set current short & long digitial gain
	// 0x0218 ~ 0x0219 : ST_DIG_GAIN_GLOBAL
	// 0x020E ~ 0x020F : DIG_GAIN_GLOBAL
	if (fimc_is_vender_wdr_mode_on(cis_data)) {
		dgains[0] = (short_gain & 0xFF00) >> 8;
		dgains[1] = short_gain & 0xFF;
		ret = fimc_is_sensor_write8_array(client, SENSOR_IMX471_SOHT_DIG_GAIN_ADDR, dgains, 2);
		if (ret < 0) {
			goto p_err;
		}
	}
	dgains[0] = (long_gain & 0xFF00) >> 8;
	dgains[1] = long_gain & 0xFF;
	ret = fimc_is_sensor_write8_array(client, SENSOR_IMX471_DIG_GAIN_ADDR, dgains, 2);
	if (ret < 0) {
		goto p_err;
	}

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_get_digital_gain(struct v4l2_subdev *subdev, u32 *dgain)
{
	int ret = 0;
	int hold = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;

	u16 digital_gain = 0;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!dgain);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	ret = fimc_is_sensor_read16(client, SENSOR_IMX471_DIG_GAIN_ADDR, &digital_gain);
	if (ret < 0)
		goto p_err;

	*dgain = sensor_cis_calc_dgain_permile(digital_gain);

	dbg_sensor(1, "[MOD:D:%d] %s, cur_dgain = %d us, digital_gain(%#x)\n",
			cis->id, __func__, *dgain, digital_gain);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);

	return ret;
}

int sensor_imx471_cis_get_min_digital_gain(struct v4l2_subdev *subdev, u32 *min_dgain)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!min_dgain);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;
	cis_data->min_digital_gain[0] = SENSOR_IMX471_MIN_DIGITAL_GAIN_SET_VALUE;
	cis_data->min_digital_gain[1] = sensor_cis_calc_dgain_permile(cis_data->min_digital_gain[0]);

	*min_dgain = cis_data->min_digital_gain[1];

	dbg_sensor(1, "[%s] code %d, permile %d\n", __func__, cis_data->min_digital_gain[0],
		cis_data->min_digital_gain[1]);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

	return ret;
}

int sensor_imx471_cis_get_max_digital_gain(struct v4l2_subdev *subdev, u32 *max_dgain)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	cis_shared_data *cis_data;

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);
	BUG_ON(!max_dgain);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	cis_data = cis->cis_data;
	cis_data->max_digital_gain[0] = SENSOR_IMX471_MAX_DIGITAL_GAIN_SET_VALUE;
	cis_data->max_digital_gain[1] = sensor_cis_calc_dgain_permile(cis_data->max_digital_gain[0]);

	*max_dgain = cis_data->max_digital_gain[1];

	dbg_sensor(1, "[%s] code %d, permile %d\n", __func__, cis_data->max_digital_gain[0],
		cis_data->max_digital_gain[1]);

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

	return ret;
}

int sensor_imx471_cis_long_term_exposure(struct v4l2_subdev *subdev)
{
	int ret = 0;
	struct fimc_is_cis *cis;
	struct fimc_is_long_term_expo_mode *lte_mode;
	unsigned char cit_lshift_val = 0;

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);
	lte_mode = &cis->long_term_mode;

	/* LTE mode or normal mode set */
	if (lte_mode->sen_strm_off_on_enable) {
		if (lte_mode->expo[0] > 500000) {
			cit_lshift_val = (unsigned char)(lte_mode->expo[0] / 500000);
			cit_lshift_val = cit_lshift_val / 2;
			lte_mode->expo[0] = 500000;
			ret |= fimc_is_sensor_write8(cis->client, 0x3100, cit_lshift_val);
		}
	} else {
		cit_lshift_val = 0;
		ret |= fimc_is_sensor_write8(cis->client, 0x3100, cit_lshift_val);
	}

	info("%s enable(%d) %d %d", __func__, lte_mode->sen_strm_off_on_enable, cit_lshift_val, lte_mode->expo[0]);

	if (ret < 0) {
		pr_err("ERR[%s]: LTE register setting fail\n", __func__);
		return ret;
	}

	return ret;
}

int sensor_imx471_cis_set_wb_gain(struct v4l2_subdev *subdev, struct wb_gains wb_gains)
{
	int ret = 0;
	int hold = 0;
	int mode = 0;
	struct fimc_is_cis *cis;
	struct i2c_client *client;
	u16 abs_gains[4] = {0, };	//[0]=gr, [1]=r, [2]=b, [3]=gb

#ifdef DEBUG_SENSOR_TIME
	struct timeval st, end;
	do_gettimeofday(&st);
#endif

	BUG_ON(!subdev);

	cis = (struct fimc_is_cis *)v4l2_get_subdevdata(subdev);

	BUG_ON(!cis);
	BUG_ON(!cis->cis_data);

	if (!cis->use_wb_gain)
		return ret;

	client = cis->client;
	if (unlikely(!client)) {
		err("client is NULL");
		ret = -EINVAL;
		goto p_err;
	}

	mode = cis->cis_data->sens_config_index_cur;

	if (mode > SENSOR_IMX471_2864X2864_QBCREMOSAIC_30FPS)
		return 0;

	dbg_sensor(1, "[SEN:%d]%s:DDK vlaue: wb_gain_gr(%d), wb_gain_r(%d), wb_gain_b(%d), wb_gain_gb(%d)\n",
		cis->id, __func__, wb_gains.gr, wb_gains.r, wb_gains.b, wb_gains.gb);

	abs_gains[0] = (u16)((wb_gains.gr / 4) & 0xFFFF);
	abs_gains[1] = (u16)((wb_gains.r / 4) & 0xFFFF);
	abs_gains[2] = (u16)((wb_gains.b / 4) & 0xFFFF);
	abs_gains[3] = (u16)((wb_gains.gb / 4) & 0xFFFF);

	dbg_sensor(1, "[SEN:%d]%s, abs_gain_gr(0x%4X), abs_gain_r(0x%4X), abs_gain_b(0x%4X), abs_gain_gb(0x%4X)\n",
		cis->id, __func__, abs_gains[0], abs_gains[1], abs_gains[2], abs_gains[3]);

	I2C_MUTEX_LOCK(cis->i2c_lock);
	hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x01);
	if (hold < 0) {
		ret = hold;
		goto p_err;
	}

	ret = fimc_is_sensor_write16_array(client, SENSOR_IMX471_ABS_GAIN_GR_SET_ADDR, abs_gains, 4);
	if (ret < 0)
		goto p_err;

#ifdef DEBUG_SENSOR_TIME
	do_gettimeofday(&end);
	dbg_sensor(1, "[%s] time %lu us\n", __func__, (end.tv_sec - st.tv_sec)*1000000 + (end.tv_usec - st.tv_usec));
#endif

p_err:
	if (hold > 0) {
		hold = sensor_imx471_cis_group_param_hold_func(subdev, 0x00);
		if (hold < 0)
			ret = hold;
	}
	I2C_MUTEX_UNLOCK(cis->i2c_lock);
	return ret;
}

static struct fimc_is_cis_ops cis_ops_imx471 = {
	.cis_init = sensor_imx471_cis_init,
	.cis_log_status = sensor_imx471_cis_log_status,
	.cis_group_param_hold = sensor_imx471_cis_group_param_hold,
	.cis_set_global_setting = sensor_imx471_cis_set_global_setting,
	.cis_mode_change = sensor_imx471_cis_mode_change,
	.cis_set_size = sensor_imx471_cis_set_size,
	.cis_stream_on = sensor_imx471_cis_stream_on,
	.cis_stream_off = sensor_imx471_cis_stream_off,
	.cis_set_exposure_time = sensor_imx471_cis_set_exposure_time,
	.cis_get_min_exposure_time = sensor_imx471_cis_get_min_exposure_time,
	.cis_get_max_exposure_time = sensor_imx471_cis_get_max_exposure_time,
	.cis_adjust_frame_duration = sensor_imx471_cis_adjust_frame_duration,
	.cis_set_frame_duration = sensor_imx471_cis_set_frame_duration,
	.cis_set_frame_rate = sensor_imx471_cis_set_frame_rate,
	.cis_adjust_analog_gain = sensor_imx471_cis_adjust_analog_gain,
	.cis_set_analog_gain = sensor_imx471_cis_set_analog_gain,
	.cis_get_analog_gain = sensor_imx471_cis_get_analog_gain,
	.cis_get_min_analog_gain = sensor_imx471_cis_get_min_analog_gain,
	.cis_get_max_analog_gain = sensor_imx471_cis_get_max_analog_gain,
	.cis_set_digital_gain = sensor_imx471_cis_set_digital_gain,
	.cis_get_digital_gain = sensor_imx471_cis_get_digital_gain,
	.cis_get_min_digital_gain = sensor_imx471_cis_get_min_digital_gain,
	.cis_get_max_digital_gain = sensor_imx471_cis_get_max_digital_gain,
	.cis_compensate_gain_for_extremely_br = sensor_cis_compensate_gain_for_extremely_br,
#if 0
	.cis_compensate_gain_for_extremely_br = sensor_imx471_cis_compensate_gain_under_ext_br,
#endif
	.cis_wait_streamoff = sensor_cis_wait_streamoff,
	.cis_data_calculation = sensor_imx471_cis_data_calc,
	.cis_set_long_term_exposure = sensor_imx471_cis_long_term_exposure,
	.cis_set_wb_gains = sensor_imx471_cis_set_wb_gain,
#ifdef USE_FACE_UNLOCK_AE_AWB_INIT
	.cis_set_initial_exposure = sensor_cis_set_initial_exposure,
#endif
};

int cis_imx471_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret = 0;
	struct fimc_is_core *core = NULL;
	struct v4l2_subdev *subdev_cis = NULL;
	struct fimc_is_cis *cis = NULL;
	struct fimc_is_device_sensor *device = NULL;
	struct fimc_is_device_sensor_peri *sensor_peri = NULL;
	u32 sensor_id[FIMC_IS_STREAM_COUNT] = {0, };
	u32 sensor_id_len;
	const u32 *sensor_id_spec;
	char const *setfile;
	struct device *dev;
	struct device_node *dnode;
	int i;

	BUG_ON(!client);
	BUG_ON(!fimc_is_dev);

	sensor_imx471_read_dpc = false;

	core = (struct fimc_is_core *)dev_get_drvdata(fimc_is_dev);
	if (!core) {
		probe_info("core device is not yet probed");
		return -EPROBE_DEFER;
	}

	dev = &client->dev;
	dnode = dev->of_node;

	sensor_id_spec = of_get_property(dnode, "id", &sensor_id_len);
	if (!sensor_id_spec) {
		err("sensor_id num read is fail(%d)", ret);
		goto p_err;
	}

	sensor_id_len /= sizeof(*sensor_id_spec);

	probe_info("%s sensor_id_spec %d, sensor_id_len %d\n", __func__,
			*sensor_id_spec, sensor_id_len);

	ret = of_property_read_u32_array(dnode, "id", sensor_id, sensor_id_len);
	if (ret) {
		err("sensor_id read is fail(%d)", ret);
		goto p_err;
	}

	for (i = 0; i < sensor_id_len; i++) {
		probe_info("%s sensor_id %d\n", __func__, sensor_id[i]);
		device = &core->sensor[sensor_id[i]];

		sensor_peri = find_peri_by_cis_id(device, SENSOR_NAME_IMX471);
		if (!sensor_peri) {
			probe_info("sensor peri is not yet probed");
			return -EPROBE_DEFER;
		}
	}

	for (i = 0; i < sensor_id_len; i++) {
		device = &core->sensor[sensor_id[i]];
		sensor_peri = find_peri_by_cis_id(device, SENSOR_NAME_IMX471);

		cis = &sensor_peri->cis;
		subdev_cis = kzalloc(sizeof(struct v4l2_subdev), GFP_KERNEL);
		if (!subdev_cis) {
			probe_err("subdev_cis is NULL");
			ret = -ENOMEM;
			goto p_err;
		}

		sensor_peri->subdev_cis = subdev_cis;

		cis->id = SENSOR_NAME_IMX471;
		cis->subdev = subdev_cis;
		cis->device = sensor_id[i];
		cis->client = client;
		sensor_peri->module->client = cis->client;
		cis->i2c_lock = NULL;
		cis->ctrl_delay = N_PLUS_TWO_FRAME;

		cis->cis_data = kzalloc(sizeof(cis_shared_data), GFP_KERNEL);
		if (!cis->cis_data) {
			err("cis_data is NULL");
			ret = -ENOMEM;
			goto p_err;
		}

		cis->cis_ops = &cis_ops_imx471;

		/* belows are depend on sensor cis. MUST check sensor spec */
		cis->bayer_order = OTF_INPUT_ORDER_BAYER_RG_GB;

		if (of_property_read_bool(dnode, "sensor_f_number")) {
			ret = of_property_read_u32(dnode, "sensor_f_number", &cis->aperture_num);
			if (ret) {
				warn("f-number read is fail(%d)",ret);
			}
		} else {
			cis->aperture_num = F2_2;
		}

		probe_info("%s f-number %d\n", __func__, cis->aperture_num);

		cis->use_dgain = true;
		cis->hdr_ctrl_by_again = false;
		cis->use_wb_gain = false;
#ifdef USE_FACE_UNLOCK_AE_AWB_INIT
		cis->use_initial_ae = of_property_read_bool(dnode, "use_initial_ae");

		probe_info("%s use_initial_ae(%d)\n", __func__, cis->use_initial_ae);
#endif

		v4l2_set_subdevdata(subdev_cis, cis);
		v4l2_set_subdev_hostdata(subdev_cis, device);
		snprintf(subdev_cis->name, V4L2_SUBDEV_NAME_SIZE, "cis-subdev.%d", cis->id);
	}

	ret = of_property_read_string(dnode, "setfile", &setfile);
	if (ret) {
		err("setfile index read fail(%d), take default setfile!!", ret);
		setfile = "default";
	}

	probe_info("%s done\n", __func__);

p_err:
	return ret;
}

static int cis_imx471_remove(struct i2c_client *client)
{
	int ret = 0;
	return ret;
}

static const struct of_device_id exynos_fimc_is_cis_imx471_match[] = {
	{
		.compatible = "samsung,exynos5-fimc-is-cis-imx471-rear",
		.data = (void *)SENSOR_REAR_DATA,
	},
	{
		.compatible = "samsung,exynos5-fimc-is-cis-imx471-front",
		.data = (void *)SENSOR_FRONT_DATA,
	},
	{},
};
MODULE_DEVICE_TABLE(of, exynos_fimc_is_cis_imx471_match);

static const struct i2c_device_id cis_imx471_idt[] = {
	{ SENSOR_NAME_REAR, SENSOR_REAR_DATA },
	{ SENSOR_NAME_FRONT, SENSOR_FRONT_DATA },
	{},
};

static struct i2c_driver cis_imx471_driver = {
	.driver = {
		.name	= SENSOR_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = exynos_fimc_is_cis_imx471_match
	},
	.probe	= cis_imx471_probe,
	.remove	= cis_imx471_remove,
	.id_table = cis_imx471_idt
};
module_i2c_driver(cis_imx471_driver);

