void readSensors ( float *data );

void ANN ( float* inputData, float* output );

void relu_f32 ( float* vec, int n );

void setMotors ( float* outputsANN );

void setSetPoint ( float* outputsANN );