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

#include "pesi_modello.h"

#include "controller.h"
#include "controller_pid.h"


#define NCS_PIN DECK_GPIO_IO3
#define INPUT_SIZE 17         // dimensione del buffer di input
#define N_LAYER 4


void appMain() {
  DEBUG_PRINT("Waiting for activation ...\n");

  while(1) {
    vTaskDelay(M2T(2000));
  }
}

void controllerOutOfTreeInit() {
  // Initialize your controller data here...

  // Call the PID controller instead in this example to make it possible to fly
  controllerPidInit();
}

bool controllerOutOfTreeTest() {
  
  float input_test [INPUT_SIZE] = { 1.4067, -0.5529, -0.0704,  0.0493, -0.0883,  0.5657,  0.0858, -0.9593, 0.1078, -0.6169,  1.5449,  0.2281,  0.5531,  0.1447,  0.6435, -1.1590, 0.6531 };
  float expected_out [4] = {0.1750, -0.0653,  0.1388,  0.2893};
  float output[4];

  ANN ( input_test, output );
  


  for ( int i=0; i<4 ; i++){
    if ( output[i] - expected_out[i] > 0.001f || output[i] - expected_out[i] < -0.001f ) 
      return false;
  }
  DEBUG_PRINT("ANN test passed!\n");
  return controllerPidTest(); // Call the PID controller test to ensure it works as well
}

void controllerOutOfTree(control_t *control, const setpoint_t *setpoint, const sensorData_t *sensors, const state_t *state, const uint32_t tick) {
 
  if(!RATE_DO_EXECUTE(RATE_25_HZ, tick))
    return;

  DEBUG_PRINT("Controller called at tick \n");
  float inputData[ INPUT_SIZE ];
  float output[4];

  inputData[0] = sensors->acc.x;
  inputData[1] = sensors->acc.y;
  inputData[2] = sensors->acc.z;
  inputData[3] = sensors->gyro.x;
  inputData[4] = sensors->gyro.y; 
  inputData[5] = sensors->gyro.z;
  inputData[6] = sensors->mag.x;
  inputData[7] = sensors->mag.y;
  inputData[8] = sensors->mag.z;
  inputData[9] = sensors->baro.pressure; // Altitude from barometer
  inputData[10] = sensors->baro.temperature; // Temperature from barometer
  inputData[11] = sensors->baro.asl; // Altitude above sea level
  inputData[12] = state->attitude.roll; // Roll
  inputData[13] = state->attitude.pitch; // Pitch
  inputData[14] = state->attitude.yaw; // Yaw
  inputData[15] = setpoint->position.x; // Setpoint position X
  inputData[16] = setpoint->position.y; // Setpoint position Y
  // inputData[17] = setpoint->position.z; // Setpoint position Z (not used in this example)

  ANN( inputData, output );

  output[0]=0;
  output[1]=0;
  output[2]=0;
  output[3]=0;
      
  // Override diretto dei comandi motore (solo per test)
  control->thrust = output[0]; 
  control->roll = output[1];  
  control->pitch = output[2]; 
  control->yaw = output[3];
  // Set the control mode to legacy
  control->controlMode = controlModeLegacy;

  DEBUG_PRINT("Control output: roll=%d, pitch=%d, yaw=%d, thrust=%f\n", control->roll, control->pitch, control->yaw, (double) control->thrust);

  
  
}

/*
void appMain() {
  DEBUG_PRINT("Waiting for activation ...\n");
  // Ensure the Flow Deck is initialized
  if ((pmw3901Init(NCS_PIN) == false)) {
    DEBUG_PRINT("Failed to initialize PMW3901\n");
    vTaskDelete(NULL);
    return;
  }
  
  usecTimerInit();

  while(1) {

    float inputData[ INPUT_SIZE ];
    float output[4];

    usecTimerReset();
    
    readSensors(inputData);
    
    uint64_t timeSensors = usecTimestamp();
    DEBUG_PRINT("\n\nTempo acquisizione sensori: %llu\n", timeSensors );

    for(int i=0; i<14; i++){
      DEBUG_PRINT("%f\t", (double) inputData[i]);
    }
    DEBUG_PRINT("\n");

  
    paramVarId_t idEstimator = paramGetVarId("stabilizer", "estimator");
    uint8_t estimator_type;
    // Get parameter value
    estimator_type = paramGetInt(idEstimator);
    // DEBUG_PRINT("Estimator type is now: %d deg\n", estimator_type);

    usecTimerReset();

    // ANN( inputData, output );
    ANN ( input_test, output );

    uint64_t timeANN = usecTimestamp();
    DEBUG_PRINT("Tempo esecuzione ANN: %llu\n", timeANN );

    for( int i=0; i<4 ; i++ )
      DEBUG_PRINT("Output[%d]: %f\n", i, (double)output[i]);

    usecTimerReset();
    // controllo diretto dei motori
    // setMotors ( output );

    // controllo tramite setpoint
    setSetPoint ( output );

    uint64_t timeMotors = usecTimestamp();
    DEBUG_PRINT("Tempo controllo motori: %llu\n", timeMotors );

    vTaskDelay(M2T(100));
  }
}
*/
/*
void readSensors(float *data){
  
  // flow deck
  motionBurst_t currentMotion;
  pmw3901ReadMotion(NCS_PIN, &currentMotion);
  data[0] = -currentMotion.deltaX;
  data[1] = -currentMotion.deltaY;
  data[2] = currentMotion.squal;                // Surface quality
  
  // accelerazione
  Axis3f acc;
  sensorsReadAcc(&acc);
  data[3] = acc.x;
  data[4] = acc.y;
  data[5] = acc.z;

  // pitch roll yaw
  logVarId_t idYaw = logGetVarId("stateEstimate", "yaw");
  logVarId_t idPitch = logGetVarId("stateEstimate", "pitch");
  logVarId_t idRoll = logGetVarId("stateEstimate", "roll");
  data[6] = logGetFloat(idYaw);
  data[7] = logGetFloat(idPitch);
  data[8] = logGetFloat(idRoll);


  // altitudine del punto di partenza
  logVarId_t idAltEstimated = logGetVarId("stateEstimate", "z");
  data[9] = logGetFloat(idAltEstimated);

  // altitudine dal livello del mare
  logVarId_t idAltBaro = logGetVarId("baro", "asl");
  data[10] = logGetFloat(idAltBaro);

  //magnetrometro
  logVarId_t idMagX = logGetVarId("mag", "x");
  logVarId_t idMagY = logGetVarId("mag", "y");
  logVarId_t idMagZ = logGetVarId("mag", "z");

  data[11] = logGetFloat(idMagX);
  data[12] = logGetFloat(idMagY); 
  data[13] = logGetFloat(idMagZ);

}
*/

void ANN(float* inputData, float* output ){

  arm_matrix_instance_f32 net_weights[4];

  arm_mat_init_f32( &net_weights[0], NET_0_WEIGHT_DIM0, NET_0_WEIGHT_DIM1, (float32_t *)net_0_weight );
  arm_mat_init_f32( &net_weights[1], NET_2_WEIGHT_DIM0, NET_2_WEIGHT_DIM1, (float32_t *)net_2_weight );
  arm_mat_init_f32( &net_weights[2], NET_4_WEIGHT_DIM0, NET_4_WEIGHT_DIM1, (float32_t *)net_4_weight);
  arm_mat_init_f32( &net_weights[3], MEAN_LAYER_WEIGHT_DIM0, MEAN_LAYER_WEIGHT_DIM1, (float32_t *)mean_layer_weight );
  
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_0_weight);
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_2_weight);
  DEBUG_PRINT("puntatore alla memoria: %08lx \n", (long unsigned int)net_4_weight);

  const float* net_bias[4];

  net_bias[0] = net_0_bias;
  net_bias[1] = net_2_bias;
  net_bias[2] = net_4_bias;
  net_bias[3] = mean_layer_bias;

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

/*

void setMotors ( float* outputsANN ){

  // Controllo diretto dei motori
  paramVarId_t idMotorPowerSetEnable = paramGetVarId("motorPowerSet", "enable");
  paramVarId_t idMotorPowerSetM1 = paramGetVarId("motorPowerSet", "m1");
  paramVarId_t idMotorPowerSetM2 = paramGetVarId("motorPowerSet", "m2");
  paramVarId_t idMotorPowerSetM3 = paramGetVarId("motorPowerSet", "m3");
  paramVarId_t idMotorPowerSetM4 = paramGetVarId("motorPowerSet", "m4");
  
  paramSetInt( idMotorPowerSetEnable, 1 );           // Nonzero to override controller with set values
  paramSetInt( idMotorPowerSetM1, (uint16_t) outputsANN[0] );
  paramSetInt( idMotorPowerSetM2, (uint16_t) outputsANN[1] );
  paramSetInt( idMotorPowerSetM3, (uint16_t) outputsANN[2] );
  paramSetInt( idMotorPowerSetM4, (uint16_t) outputsANN[3] );
}

void setSetPoint ( float* outputsANN ){

  setpoint_t setpoint;
  quaternion_t quaternion;

  quaternion.x = outputsANN[0];
  quaternion.y = outputsANN[1];
  quaternion.z = outputsANN[2];
  quaternion.w = outputsANN[3];

  setpoint.attitudeQuaternion = quaternion;
  setpoint.mode.quat = modeAbs;                 // modeAbs o modeVelocity
  setpoint.velocity_body = true;                // true if velocity is given in body frame; false if velocity is given in world frame

  commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_HIGHLEVEL );
}

*/