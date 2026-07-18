/*
 * hardware_cfg.h
 *
 *  Created on: Nov 19, 2025
 *      Author: yaofa
 */

#ifndef CONFIG_HARDWARE_CFG_H_
#define CONFIG_HARDWARE_CFG_H_

#define ENCODER_RESOLUTION  (500 * 4)   /* 编码器线数*4（四倍频） */
#define GEAR_RATIO          28          /* 减速比 */
#define SAMPLE_PERIOD_MS    20          /* 计算周期 */
#define ENC_PULSES_PER_OUT_REV  (ENCODER_RESOLUTION * GEAR_RATIO) /* 输出轴每转脉冲 */

#endif /* CONFIG_HARDWARE_CFG_H_ */
