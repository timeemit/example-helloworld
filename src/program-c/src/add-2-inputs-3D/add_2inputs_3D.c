/**
 * @brief C-based ONNX BPF program
 */
#include <solana_sdk.h>
#include "add_2inputs_3D.h"

GLOW_MEM_ALIGN(ADD_2INPUTS_3D_MEM_ALIGN)
static uint8_t constantWeight[ADD_2INPUTS_3D_CONSTANT_MEM_SIZE] = {
  #include "./add_2inputs_3D.weights.txt"
};

uint64_t exec_onnx(SolParameters *params) {

  if (params->ka_num < 1) {
    sol_log("Greeted account not included in the instruction");
    return ERROR_NOT_ENOUGH_ACCOUNT_KEYS;
  }

  // Get the account to say hello to
  SolAccountInfo *greeted_account = &params->ka[0];

  // The account must be owned by the program in order to modify its data
  if (!SolPubkey_same(greeted_account->owner, params->program_id)) {
    sol_log("Greeted account does not have the correct program id");
    return ERROR_INCORRECT_PROGRAM_ID;
  }

  GLOW_MEM_ALIGN(ADD_2INPUTS_3D_MEM_ALIGN)
  uint8_t mutableWeight[ADD_2INPUTS_3D_MUTABLE_MEM_SIZE];

  GLOW_MEM_ALIGN(ADD_2INPUTS_3D_MEM_ALIGN)
  uint8_t activations[ADD_2INPUTS_3D_ACTIVATIONS_MEM_SIZE];

  uint8_t *xAddr = GLOW_GET_ADDR(mutableWeight, ADD_2INPUTS_3D_X);
  uint8_t *yAddr = GLOW_GET_ADDR(mutableWeight, ADD_2INPUTS_3D_Y);
  uint8_t *zAddr = GLOW_GET_ADDR(mutableWeight, ADD_2INPUTS_3D_Z);

  float x[8] = {3.85, 4.3, 4.75, 5.2, 5.65, 6.1, 6.55, 7.};
  float y[8] = {2.85, 3.3, 3.75, 4.2, 4.65, 5.1, 5.55, 6.};
  sol_log_array((uint8_t *) x, 8);
  sol_log_array((uint8_t *) y, 8);
  sol_memcpy(xAddr, x, 8 * sizeof(float));
  sol_memcpy(yAddr, y, 8 * sizeof(float));

  sol_log("Adding!");
  int errCode = add_2inputs_3D(constantWeight, mutableWeight, activations);
  float *z = (float *)(zAddr);
  sol_log_array((uint8_t *) z, 8);

  // The data must be large enough to hold an uint32_t value
  // if (greeted_account->data_len < sizeof(uint32_t)) {
  //   sol_log("Greeted account data length too small to hold uint32_t value");
  //   return ERROR_INVALID_ACCOUNT_DATA;
  // }

  // Increment and store the number of times the account has been greeted
  // uint32_t *num_greets = (uint32_t *)greeted_account->data;
  // *num_greets += 1;

  sol_log("Hello!");
  
  return SUCCESS;
}

extern uint64_t entrypoint(const uint8_t *input) {
  sol_log("add 2inputs 3D C program entrypoint");

  SolAccountInfo accounts[1];
  SolParameters params = (SolParameters){.ka = accounts};

  if (!sol_deserialize(input, &params, SOL_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
  }

  return exec_onnx(&params);
}
