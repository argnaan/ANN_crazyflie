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

#include "arm_math.h"

#include "pesi_modello.h"



#define NCS_PIN DECK_GPIO_IO3
#define INPUT_SIZE 17         // dimensione del buffer di input



void appMain() {
  DEBUG_PRINT("Waiting for activation ...\n");
  // Ensure the Flow Deck is initialized
  if ((pmw3901Init(NCS_PIN) == false)) { 
    DEBUG_PRINT("Failed to initialize PMW3901\n");
    vTaskDelete(NULL);
    return;
}

  while(1) {

    float inputData[ INPUT_SIZE ];
    
    readSensors(inputData);

    for(int i=0; i<10; i++){
      DEBUG_PRINT("%f\t", (double) inputData[i]);
    }
    DEBUG_PRINT("\n");

  
    paramVarId_t idEstimator = paramGetVarId("stabilizer", "estimator");
    uint8_t estimator_type;
    // Get parameter value
    estimator_type = paramGetInt(idEstimator);
    DEBUG_PRINT("Estimator type is now: %d deg\n", estimator_type);



    ANN( inputData );



    vTaskDelay(M2T(100));
  }
}

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

}

void ANN(float* inputData){

  arm_matrix_instance_f32 net_weights[4];

  arm_mat_init_f32( &net_weights[0], NET_0_WEIGHT_DIM0, NET_0_WEIGHT_DIM1, (float32_t *)net_0_weight );
  arm_mat_init_f32( &net_weights[1], NET_2_WEIGHT_DIM0, NET_2_WEIGHT_DIM1, (float32_t *)net_2_weight );
  arm_mat_init_f32( &net_weights[2], NET_4_WEIGHT_DIM0, NET_4_WEIGHT_DIM1, (float32_t *)net_4_weight);
  arm_mat_init_f32( &net_weights[3], MEAN_LAYER_WEIGHT_DIM0, MEAN_LAYER_WEIGHT_DIM1, (float32_t *)mean_layer_weight );
  
  const float* net_bias[4];

  net_bias[0] = net_0_bias;
  net_bias[1] = net_2_bias;
  net_bias[2] = net_4_bias;
  net_bias[3] = mean_layer_bias;

  float buffer_1[256];
  float buffer_2[256];
  memcpy(buffer_1, inputData, INPUT_SIZE*sizeof(float));

  for( int l = 0 ; l < 4 ; l++)
  {
    int n = net_weights[l].numRows;
    // arm_mat_vec_mult_f32 (const arm_matrix_instance_f32 *pSrcMat, const float32_t *pVec, float32_t *pDst)
    arm_mat_vec_mult_f32( &net_weights[l], buffer_1, buffer_2);
    
    // arm_add_f32 (const float32_t *pSrcA, const float32_t *pSrcB, float32_t *pDst, uint32_t blockSize)
    arm_add_f32( buffer_2, net_bias[l], buffer_1, n );    

    // attivazione
    relu_f32 ( buffer_1 , n );
  }

}

void relu_f32 ( float* vec, int n){
  for ( int i=0; i<n ; i++)
    vec[i] = (vec[i]>0) ? vec[i] : 0.0f;
}