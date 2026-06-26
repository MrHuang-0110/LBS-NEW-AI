#include "mem.h"
#include "iic.h"
#include "lis2mdl_reg.h"
#include "lsm6ds3tr-c_reg.h"

static DEV_MEM mem;
static uint8_t mfxstate_9x[FX_STATE_SIZE];

static float yaw_offset = 0.0f;
static float pitch_offset = 0.0f;
static float roll_offset = 0.0f;

stmdev_ctx_t dev_ctx;
stmdev_ctx_t lis2mdl_dev_ctx;

static void platform_delay(uint32_t ms);

static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,uint16_t len);
static int32_t lis2mdl_platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);
static int32_t lis2mdl_platform_read(void *handle, uint8_t reg, uint8_t *bufp,uint16_t len);
static void platform_delay(uint32_t ms);
DEV_MEM *getMemHandle(void)
{ 
   return &mem;
}
static void platform_delay(uint32_t ms)
{ 
   vTaskDelay(ms);
} 
static float normalize_angle(float angle) {
    angle = fmodf(angle, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    if (angle > 180.0f) angle -= 360.0f;  
    return angle;
}

static int32_t lis2mdl_platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{

  reg |= 0x80;
  Sensors_I2C_WriteRegister(LIS2MDL_I2C_ADD, reg,len, (uint8_t*) bufp);
  return 0;
}


static int32_t lis2mdl_platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  reg |= 0x80;
  Sensors_I2C_ReadRegister(LIS2MDL_I2C_ADD,reg,len, bufp);
  return 0;
}


static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
  Sensors_I2C_WriteRegister(LSM6DS3TR_C_I2C_ADD_L, reg,len, (uint8_t*) bufp);
  return 0;
}


static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  Sensors_I2C_ReadRegister(LSM6DS3TR_C_I2C_ADD_L,reg,len, bufp);
  return 0;
}

static bool readAccRawgRaw(int16_t *accRaw,int16_t *gRaw,float *mg,float *mdps)
{
	 lsm6ds3tr_c_reg_t reg;
	 lsm6ds3tr_c_status_reg_get(&dev_ctx,&reg.status_reg);

	 if(reg.status_reg.xlda && reg.status_reg.gda)
	 {
		memset(accRaw,0,3 * sizeof(int16_t));
		lsm6ds3tr_c_acceleration_raw_get(&dev_ctx,accRaw);

		mg[0] = lsm6ds3tr_c_from_fs2g_to_mg(accRaw[0]);
		mg[1] = lsm6ds3tr_c_from_fs2g_to_mg(accRaw[1]);
		mg[2] = lsm6ds3tr_c_from_fs2g_to_mg(accRaw[2]);

		memset(gRaw,0,3 * sizeof(int16_t));
		lsm6ds3tr_c_angular_rate_raw_get(&dev_ctx,gRaw);
		mdps[0] = lsm6ds3tr_c_from_fs2000dps_to_mdps(gRaw[0]);
		mdps[1] = lsm6ds3tr_c_from_fs2000dps_to_mdps(gRaw[1]);
		mdps[2] = lsm6ds3tr_c_from_fs2000dps_to_mdps(gRaw[2]);
		return true;
	 }

	 return false;
}

bool read_acc_gray(void)
{ 
  return readAccRawgRaw(mem.acc_raw_data,mem.gy_raw_data, mem.acceleration_mg,mem.angular_rate_mdps);   
}
void read_magnetic(void)
{ 
   if(mem.Lis2mdlid == LIS2MDL_ID)
	 { 
			memset(mem.data_raw_magnetic, 0x00, 3 * sizeof(int16_t));
			lis2mdl_magnetic_raw_get(&lis2mdl_dev_ctx, mem.data_raw_magnetic);
			mem.magnetic_mG[0] = lis2mdl_from_lsb_to_mgauss(mem.data_raw_magnetic[0]);
			mem.magnetic_mG[1] = lis2mdl_from_lsb_to_mgauss(mem.data_raw_magnetic[1]);
			mem.magnetic_mG[2] = lis2mdl_from_lsb_to_mgauss(mem.data_raw_magnetic[2]);	   
	 }
}

float get_continuous_yaw(DEV_MEM *mem, float *last_raw_yaw, float *yaw_offset) {
    float current_raw_yaw = mem->yaw;
    float diff = current_raw_yaw - *last_raw_yaw; 
    if (diff > 180.0f) {
        *yaw_offset -= 360.0f;
    } else if (diff < -180.0f) {
        *yaw_offset += 360.0f;
    }
    
    *last_raw_yaw = current_raw_yaw;
    return current_raw_yaw + *yaw_offset;
}
float calculate_angle_difference(float previous_angle, float current_angle) {
    float diff = current_angle - previous_angle;
    
    // 处理角度跨越边界的情况
    if (diff > 180.0f) {
        // 比如从-179到179，实际上是顺时针旋转了2度
        diff -= 360.0f;
    } else if (diff < -180.0f) {
        // 比如从179到-179，实际上是逆时针旋转了2度
        diff += 360.0f;
    }
    
    return diff;
}
void lsm6ds3tr_c_motion_fx_determin(void){
	
  #define FILTER_ALPHA 0.1f
	
	MFX_input_t mfx_data_in;
 // MFX_MagCal_input_t mag_data_in;
	
   static TickType_t xLastTick;
   TickType_t xCurrentTick;
  

	 xCurrentTick = xTaskGetTickCount();
   mem.dt = (xCurrentTick - xLastTick) * portTICK_PERIOD_MS / 1000.0f;
   xLastTick = xCurrentTick;
	 
  read_acc_gray();
  read_magnetic();		
	
	/* MotionFX 算法库，计算四元数，参考自 AlgoBuilded 生成代码 */
	mfx_data_in.acc[0] = mem.acceleration_mg[0] * FROM_MG_TO_G;
	mfx_data_in.acc[1] = mem.acceleration_mg[1] * FROM_MG_TO_G;
	mfx_data_in.acc[2] = mem.acceleration_mg[2] * FROM_MG_TO_G;

	mfx_data_in.gyro[0] = mem.angular_rate_mdps[0] * FROM_MDPS_TO_DPS;
	mfx_data_in.gyro[1] = mem.angular_rate_mdps[1] * FROM_MDPS_TO_DPS;
	mfx_data_in.gyro[2] = mem.angular_rate_mdps[2] * FROM_MDPS_TO_DPS;

	mfx_data_in.mag[0] = (mem.magnetic_mG[0] * 0.001f)/50.0f;
	mfx_data_in.mag[1] = (mem.magnetic_mG[1] * 0.001f)/50.0f;
	mfx_data_in.mag[2] = (mem.magnetic_mG[2] * 0.001f)/50.0f;

	/* 跟传感器输出速率ODR相关，△time 要比采样周期小 */
  //float delta_time = DELATE_TIME;
	//newAi->dmem.dt = (dt*25.0f)/1000000.0f;
  //mem.dt = (dt*25.0f)/1000000.0f;
	//mem.dt = dt/1000.0f;
	MotionFX_propagate(mfxstate_9x, &mem.mfx_6x, &mfx_data_in, &mem.dt);
	MotionFX_update(mfxstate_9x, &mem.mfx_6x, &mfx_data_in, &mem.dt, NULL);
	
	mem.yaw = normalize_angle(mem.mfx_6x.rotation[0] - yaw_offset);
	mem.pitch = normalize_angle(mem.mfx_6x.rotation[2] - pitch_offset);
  mem.roll = normalize_angle(mem.mfx_6x.rotation[1] - roll_offset);
	 
   mem.is_refresh = true;  
}
void resetyaw(void)
{     
    yaw_offset = mem.mfx_6x.rotation[0];
    pitch_offset = mem.mfx_6x.rotation[2];
    roll_offset = mem.mfx_6x.rotation[1];	
}
void memu_fx_init(void)
{ 
   MFX_knobs_t iKnobs;
   MFX_knobs_t *ipKnobs = &iKnobs;

   uint8_t CheckSetState;
 

   dev_ctx.write_reg = platform_write;
   dev_ctx.read_reg = platform_read;
   dev_ctx.mdelay = platform_delay;
   dev_ctx.handle = NULL;

   platform_delay(200);

   lsm6ds3tr_c_device_id_get(&dev_ctx, &mem.Lsm6ds3id);

   if(mem.Lsm6ds3id != LSM6DS3TR_C_ID)
	       return;
 
   uint8_t rst;
   lsm6ds3tr_c_reset_set(&dev_ctx, PROPERTY_ENABLE);
	 
	  do
	  {
         lsm6ds3tr_c_reset_get(&dev_ctx, &rst);
		     vTaskDelay(1);
	  }while(rst);


	  lsm6ds3tr_c_xl_full_scale_set(&dev_ctx, LSM6DS3TR_C_2g);
	  lsm6ds3tr_c_gy_full_scale_set(&dev_ctx, LSM6DS3TR_C_2000dps);
	  lsm6ds3tr_c_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

	  lsm6ds3tr_c_xl_power_mode_set(&dev_ctx, LSM6DS3TR_C_XL_HIGH_PERFORMANCE);
	  do
	  {
       lsm6ds3tr_c_xl_power_mode_get(&dev_ctx,&CheckSetState);
		   vTaskDelay(1);
	  }while(CheckSetState!=LSM6DS3TR_C_XL_HIGH_PERFORMANCE);
		
	 lsm6ds3tr_c_xl_data_rate_set(&dev_ctx, LSM6DS3TR_C_XL_ODR_52Hz);
	 lsm6ds3tr_c_gy_data_rate_set(&dev_ctx, LSM6DS3TR_C_GY_ODR_52Hz);

   lis2mdl_dev_ctx.write_reg = lis2mdl_platform_write;
   lis2mdl_dev_ctx.read_reg = lis2mdl_platform_read;
   lis2mdl_dev_ctx.mdelay = platform_delay;
   lis2mdl_dev_ctx.handle = NULL;
	 vTaskDelay(15);

   lis2mdl_device_id_get(&lis2mdl_dev_ctx, &mem.Lis2mdlid);
	if(mem.Lis2mdlid == LIS2MDL_ID)
	{
		uint8_t rst;
		lis2mdl_reset_set(&lis2mdl_dev_ctx, PROPERTY_ENABLE);
		do
		{
			lis2mdl_reset_get(&lis2mdl_dev_ctx, &rst);
			vTaskDelay(1);
		}while(rst);
  	/* Enable Block Data Update */
    lis2mdl_block_data_update_set(&lis2mdl_dev_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate */
    lis2mdl_data_rate_set(&lis2mdl_dev_ctx, LIS2MDL_ODR_50Hz);
    /* Set / Reset sensor mode */
    lis2mdl_set_rst_mode_set(&lis2mdl_dev_ctx, LIS2MDL_SENS_OFF_CANC_EVERY_ODR);
    /* Enable temperature compensation */
    lis2mdl_offset_temp_comp_set(&lis2mdl_dev_ctx, PROPERTY_ENABLE);
    /* Set device in continuous mode */
    lis2mdl_operating_mode_set(&lis2mdl_dev_ctx, LIS2MDL_CONTINUOUS_MODE);
	}


	MotionFX_initialize((MFXState_t *)mfxstate_9x);
	MotionFX_getKnobs(mfxstate_9x, ipKnobs);

	ipKnobs->gbias_acc_th_sc = GBIAS_ACC_TH_SC_9X;
	ipKnobs->gbias_gyro_th_sc = GBIAS_GYRO_TH_SC_9X;
	ipKnobs->gbias_mag_th_sc = GBIAS_MAG_TH_SC_9X;

	ipKnobs->LMode = 1;
	/* modx 代表 MotionFX_update 函数调用频率，
	 *	modx = 1，每调用 MotionFX_propagate 函数一次，可调用 MotionFX_update 函数一次，适用于STM32F4系列处理器，
	 *	modx = 2，每调用 MotionFX_propagate 函数两次，可调用 MotionFX_update 函数一次，适用于STM32F1系列处理器。
	 */
		ipKnobs->modx = 1;

		/* 设置内部结构参数 */
		MotionFX_setKnobs(mfxstate_9x, ipKnobs);

		/* 使能6轴 MotionFX 引擎*/
		if(mem.Lis2mdlid != LIS2MDL_ID)
		{
		  MotionFX_enable_6X(mfxstate_9x, MFX_ENGINE_ENABLE);
		  MotionFX_enable_9X(mfxstate_9x, MFX_ENGINE_DISABLE);
		}
		else
		{
		  MotionFX_enable_6X(mfxstate_9x, MFX_ENGINE_DISABLE);
		  MotionFX_enable_9X(mfxstate_9x, MFX_ENGINE_ENABLE);
		}
    MotionFX_MagCal_init(ALGO_PERIOD, 1); 
}
void waite_yaw_stab(void)
{ 
	 uint8_t count = 10;
   
	 while(count--)
	 {
	    mem.is_refresh = false;
		  while(!mem.is_refresh){ 
				extern void motor_delay_exit(uint32_t tick);
			  motor_delay_exit(10);
			}
	 }
}
uint16_t get_raw_grayz(void)
{ 
  return mem.gy_raw_data[2];
}
float get_yaw(void)
{ 
  return mem.yaw;
}
