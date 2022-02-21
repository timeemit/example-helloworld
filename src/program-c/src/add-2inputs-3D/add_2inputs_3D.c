/**
 * @brief C-based ONNX BPF program
 */
#include <solana_sdk.h>
#include <string.h>
#include "add-2inputs-3D-dynamic/add_2inputs_3D_dynamic.h"
#include "add-2inputs-3D-static/add_2inputs_3D_static.h"


GLOW_MEM_ALIGN(ADD_2INPUTS_3D_STATIC_MEM_ALIGN)
const static uint8_t constantWeight[ADD_2INPUTS_3D_STATIC_CONSTANT_MEM_SIZE] = {
  #include "add-2inputs-3D-static/add_2inputs_3D_static.weights.bin"
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
  uint8_t *weights = (uint8_t *)(alignedAlloc(heap, config->mutableWeightVarsMemSize, config->alignment));
  return weights;
}

float *getInferenceResults(const BundleConfig *config, uint8_t *mutableWeightVars) {
  const SymbolTableEntry *outputWeights = getMutableWeightVar(config, "Z");
  float *results = (float *)(mutableWeightVars + outputWeights->offset);
  return results;
}

uint8_t *initMutableWeightVars(struct BumpAllocator *heap, const BundleConfig *config) {
  uint8_t *mutableWeightVarsAddr = allocateMutableWeightVars(heap, config);

  // X
  float x[8] = {3.85, 4.3, 4.75, 5.2, 5.65, 6.1, 6.55, 7.};
  const SymbolTableEntry *inputXVar = getMutableWeightVar(config, "X");
  sol_memcpy(mutableWeightVarsAddr + inputXVar->offset, &x, sizeof(float) * 8);

  // Y
  float y[8] = {2.85, 3.3, 3.75, 4.2, 4.65, 5.1, 5.55, 6.};
  const SymbolTableEntry *inputYVar = getMutableWeightVar(config, "Y");
  sol_memcpy(mutableWeightVarsAddr + inputYVar->offset, &y, sizeof(float) * 8);

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
  uint8_t *mutableWeightVarsAddr = initMutableWeightVars(&heap, &add_2inputs_3D_dynamic_config);
  uint8_t *activationsAddr = initActivations(&heap, &add_2inputs_3D_dynamic_config);

  int errCode = add_2inputs_3D_dynamic((uint8_t *)&constantWeight, mutableWeightVarsAddr, activationsAddr);
  if (errCode != GLOW_SUCCESS) {
    sol_log("Error running bundle: error code");
  }
  
  float *z = getInferenceResults(&add_2inputs_3D_dynamic_config, mutableWeightVarsAddr);
  float z_total = 0;
  for (int i = 0; i < 8; i++) {
    z_total += z[i];
  }
  // (3.85 + 4.3 + 4.75 + 5.2 + 5.65 + 6.1 + 6.55 + 7) * 2 - 8 == 78.8, or 0x4e
  sol_log_64(0, 0, 0, 0, (int)z_total);
  
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
