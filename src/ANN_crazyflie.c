/**
 * ,---------,       ____  _ __
 * |  ,-^-,  |      / __ )(_) /_______________ _____  ___
 * | (  O  ) |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * | / ,--´  |    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *    +------`   /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2019 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 
 */


#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "ANN_crazyflie.h"

#include "app.h"

#include "FreeRTOS.h"
#include "task.h"

// #define DEBUG_MODULE "ANN_crazyflie"
#include "debug.h"

#include "log.h"
#include "param.h"
#include "static_mem.h"
// #include "crtp.h"
#include "pmw3901.h" // Optical flow deck driver
#include "deck.h"    // For deck GPIO pin definitions

#include "sensors.h"
// #include "state_estimator.h"
#include "stabilizer.h"
#include "stabilizer_types.h"
#include "commander.h"
#include "usec_time.h"

#include "arm_math.h"

// #include "pesi_modello.h"
#include "pesi_modello_rnd.h"

#include "controller.h"
#include "controller_pid.h"


#define NCS_PIN DECK_GPIO_IO3
#define INPUT_SIZE 17         // dimensione del buffer di input


/*
void appMain() {
  DEBUG_PRINT("Waiting for activation ...\n");

  while(1) {
    vTaskDelay(M2T(2000));
  }
}
*/
void controllerOutOfTreeInit() {
  // Initialize your controller data here...

  paramVarId_t idEstimator = paramGetVarId("stabilizer", "estimator");
  paramSetInt(idEstimator, 0);
  // Call the PID controller instead in this example to make it possible to fly
  controllerPidInit();
}

bool controllerOutOfTreeTest() {

  if(TEST_ANN){
    float input_test [INPUT_SIZE] = { 1.4067, -0.5529, -0.0704,  0.0493, -0.0883,  0.5657,  0.0858, -0.9593, 0.1078, -0.6169,  1.5449,  0.2281,  0.5531,  0.1447,  0.6435, -1.1590, 0.6531 };
    float expected_out [4] = {0.1750, -0.0653,  0.1388,  0.2893};
    float output[4];

    ANN ( input_test, output );
    
    for ( int i=0; i<4 ; i++){
      if ( output[i] - expected_out[i] > 0.001f || output[i] - expected_out[i] < -0.001f ) 
        return false;
    }
    DEBUG_PRINT("ANN test passed\n");
  }
  return true;

  //return controllerPidTest(); // Call the PID controller test to ensure it works as well 
}

void controllerOutOfTree(control_t *control, const setpoint_t *setpoint, const sensorData_t *sensors, const state_t *state, const uint32_t tick) {
 
  if(!RATE_DO_EXECUTE(RATE_100_HZ, tick))
    return;

  uint64_t start = usecTimestamp();

  float inputData[ INPUT_SIZE ];
  float output[4];

  logVarId_t idDeltaX = logGetVarId("motion", "deltaX");
  logVarId_t idDeltaY = logGetVarId("motion", "deltaY");
  // logVarId_t idSqual = logGetVarId("motion", "squal");

  inputData[0] = sensors->acc.x;
  inputData[1] = sensors->acc.y;
  inputData[2] = sensors->acc.z;
  inputData[3] = sensors->gyro.x;
  inputData[4] = sensors->gyro.y; 
  inputData[5] = sensors->gyro.z;
  inputData[6] = sensors->mag.x;
  inputData[7] = sensors->mag.y;
  inputData[8] = sensors->mag.z;
  inputData[9] = sensors->baro.pressure;      // Altitude from barometer
  inputData[10] = sensors->baro.temperature;  // Temperature from barometer
  inputData[11] = sensors->baro.asl;          // Altitude above sea level
  inputData[12] = state->attitude.roll;       // Roll
  inputData[13] = state->attitude.pitch;      // Pitch
  inputData[14] = state->attitude.yaw;        // Yaw
  
  // FLOWDECK SENSORS
  inputData[15] = logGetInt(idDeltaX);        // Optical flow X delta
  inputData[16] = logGetInt(idDeltaY);        // Optical flow Y delta
  //inputData[17] = logGetInt(idSqual);       // Optical flow surface quality 

  // SETPOINT DATA
  //inputData[] = setpoint->position.x;       // Setpoint position X
  //inputData[] = setpoint->position.y;       // Setpoint position Y
  //inputData[] = setpoint->position.z;       // Setpoint position Z 

  ANN( inputData, output );

  /*
  output[0]=0;
  output[1]=0;
  output[2]=0;
  output[3]=0;
  */

  // Override diretto dei comandi motore (solo per test)
  control->thrust = output[0]; 
  control->roll = output[1];  
  control->pitch = output[2]; 
  control->yaw = output[3];
  // Set the control mode to legacy
  control->controlMode = controlModeLegacy;

  uint64_t end = usecTimestamp();
  
  if(!(tick % 2000)) { 
    DEBUG_PRINT("Controller execution time: %lu us\n", (unsigned long)(end - start));
    DEBUG_PRINT("Control output: roll=%d, pitch=%d, yaw=%d, thrust=%f\n", control->roll, control->pitch, control->yaw, (double) control->thrust);
  }
  
  // controllerPid(control, setpoint, sensors, state, tick); // Call the PID controller to handle the control logic
}


void ANN(float* inputData, float* output ){

  arm_matrix_instance_f32 net_weights[N_LAYER];
  const float* net_bias[N_LAYER];

  
  arm_mat_init_f32( &net_weights[0], NET_0_WEIGHT_DIM0, NET_0_WEIGHT_DIM1, (float32_t *)net_0_weight );
  net_bias[0] = net_0_bias;
  arm_mat_init_f32( &net_weights[1], NET_1_WEIGHT_DIM0, NET_1_WEIGHT_DIM1, (float32_t *)net_1_weight );
  net_bias[1] = net_1_bias;
  arm_mat_init_f32( &net_weights[2], NET_2_WEIGHT_DIM0, NET_2_WEIGHT_DIM1, (float32_t *)net_2_weight);
  net_bias[2] = net_2_bias;
  #if N_LAYER > 3
  arm_mat_init_f32( &net_weights[3], NET_3_WEIGHT_DIM0, NET_3_WEIGHT_DIM1, (float32_t *)net_3_weight );
  net_bias[3] = net_3_bias;
  #if N_LAYER > 4
  arm_mat_init_f32( &net_weights[4], NET_4_WEIGHT_DIM0, NET_4_WEIGHT_DIM1, (float32_t *)net_4_weight );
  net_bias[4] = net_4_bias;
  #if N_LAYER > 5
  arm_mat_init_f32( &net_weights[5], NET_5_WEIGHT_DIM0, NET_5_WEIGHT_DIM1, (float32_t *)net_5_weight );
  net_bias[5] = net_5_bias;
  #endif
  #endif
  #endif
  /*
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_0_weight);
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_2_weight);
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_4_weight);
  */

  float buffer_1[256];
  float buffer_2[256];
  memcpy(buffer_1, inputData, INPUT_SIZE*sizeof(float));

  for( int l = 0 ; l < N_LAYER ; l++)
  {
    int n = net_weights[l].numRows;
    // arm_mat_vec_mult_f32 (const arm_matrix_instance_f32 *pSrcMat, const float32_t *pVec, float32_t *pDst)
    arm_mat_vec_mult_f32( &net_weights[l], buffer_1, buffer_2);
    
    // arm_add_f32 (const float32_t *pSrcA, const float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
    arm_add_f32( buffer_2, net_bias[l], buffer_1, n );    

    // attivazione
    if( l < N_LAYER-1) 
      relu_f32 ( buffer_1 , n );
  }
  
  for(int i=0; i<4; i++)
    output[i] = buffer_1[i];

}

void relu_f32 ( float* vec, int n){
  for ( int i=0; i<n ; i++)
    vec[i] = (vec[i]>0) ? vec[i] : 0.0f;
}
