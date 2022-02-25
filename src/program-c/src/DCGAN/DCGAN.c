/**
 * @brief C-based ONNX BPF program
 */
#include <solana_sdk.h>
#include <string.h>
#include "DCGAN-trained-dynamic/DCGAN_trained_dynamic.h"
#include "DCGAN-trained-static/DCGAN_trained_static.h"


GLOW_MEM_ALIGN(DCGAN_TRAINED_STATIC_MEM_ALIGN)
const static uint8_t constantWeight[DCGAN_TRAINED_STATIC_CONSTANT_MEM_SIZE] = {
  #include "DCGAN-trained-static/DCGAN_trained_static.weights.txt"
};

#define HEAP_START_ADDRESS_ (uint64_t)0x300000000
#define HEAP_LENGTH_ (uint64_t)(32 * 1024)


const SymbolTableEntry *getWeightVar(const BundleConfig *config, const char *name) {
  for (unsigned i = 0, e = config->numSymbols; i < e; i++) {
    if (!strncmp(config->symbolTable[i].name, name, strlen(name))) {
      return &config->symbolTable[i];
    }
  }
  return NULL;
}

const SymbolTableEntry *getMutableWeightVar(const BundleConfig *config, const char *name) {
  const SymbolTableEntry *mutableWeightVar = getWeightVar(config, name);
  if (!mutableWeightVar) {
    sol_log("No mutableWeightVar found");
    return NULL;
  }
  if (mutableWeightVar->kind == 0) {
    sol_log("mutableWeightVar kind is immutable");
    return NULL;
  }
  return mutableWeightVar;
}

struct BumpAllocator {
  uint64_t start;
  uint64_t size;
};

void *alignedAlloc(struct BumpAllocator *self, uint64_t size, uint64_t align) {
  uint64_t *pos_ptr = (uint64_t *)self->start;

  uint64_t pos = *pos_ptr;
  if (pos == 0) {
    // First time, set starting position
    pos = self->start + self->size;
  }
  if (pos < size) {
    pos = 0;
  } else {
    pos = pos - size;
  }
  pos &= ~(align - 1);
  if (pos < self->start + sizeof(uint8_t)) {
    return NULL;
  }
  *pos_ptr = pos;
  return (void *)pos;
}

uint8_t *allocateMutableWeightVars(struct BumpAllocator *heap, const BundleConfig *config) {
  sol_log("Aligning memory for mutable weights in heap");
  uint8_t *weights = (uint8_t *)(alignedAlloc(heap, config->mutableWeightVarsMemSize, config->alignment));
  return weights;
}

float *getInferenceResults(const BundleConfig *config, uint8_t *mutableWeightVars) {
  const SymbolTableEntry *outputWeights = getMutableWeightVar(config, "A12");
  float *results = (float *)(mutableWeightVars + outputWeights->offset);
  return results;
}

uint8_t *initMutableWeightVars(struct BumpAllocator *heap, const BundleConfig *config) {
  sol_log("Inside initMutableWeightVars");
  uint8_t *mutableWeightVarsAddr = allocateMutableWeightVars(heap, config);
  sol_log("Allocated mutableWeightVars");

  // A0
  float a0[1] = {-0.330496591178819};
  sol_log("Initialized a0 floats array");
  const SymbolTableEntry *inputA0Var = getMutableWeightVar(config, "A0");
  sol_log("Retrieved a0 inputVar");
  sol_memcpy(mutableWeightVarsAddr + inputA0Var->offset, &a0, sizeof(float) * 1);
  sol_log("memcpyied arrays");

  return mutableWeightVarsAddr;
}

uint8_t *initActivations(struct BumpAllocator *heap, BundleConfig *config) {
  return (uint8_t *)(alignedAlloc(heap, config->activationsMemSize, config->alignment));
}

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

  sol_log("Hello!");

  struct BumpAllocator heap = {HEAP_START_ADDRESS_, HEAP_LENGTH_};
  uint8_t *mutableWeightVarsAddr = initMutableWeightVars(&heap, &DCGAN_trained_dynamic_config);
  sol_log("Initiated Mutable Weights");
  uint8_t *activationsAddr = initActivations(&heap, &DCGAN_trained_dynamic_config);
  sol_log("Initiated Activations");
  sol_log_compute_units();

  int errCode = DCGAN_trained_dynamic((uint8_t *)&constantWeight, mutableWeightVarsAddr, activationsAddr);
  sol_log("Executed DCGAN");
  if (errCode != GLOW_SUCCESS) {
    sol_log("Error running bundle: error code");
  }
  
  float *a21 = getInferenceResults(&DCGAN_trained_dynamic_config, mutableWeightVarsAddr);
  float a21_total = 0;
  for (int i = 0; i < 8 * 8; i++) {
    a21_total += a21[i];
  }
  sol_log_64(0, 0, 0, 0, (int)a21_total);
  
  return SUCCESS;
}

extern uint64_t entrypoint(const uint8_t *input) {
  sol_log("DCGAN C program entrypoint");

  SolAccountInfo accounts[1];
  SolParameters params = (SolParameters){.ka = accounts};

  if (!sol_deserialize(input, &params, SOL_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
  }

  return exec_onnx(&params);
}
