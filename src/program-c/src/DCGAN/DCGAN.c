/**
 * @brief C-based ONNX BPF program
 */
#include <solana_sdk.h>
#include <string.h>

#ifndef _GLOW_BUNDLE_COMMON_DEFS
#define _GLOW_BUNDLE_COMMON_DEFS

// Glow bundle error code for correct execution.
#define GLOW_SUCCESS 0

// Glow DYNAMIC API

// Type describing a symbol table entry of a generated bundle.
typedef struct SymbolTableEntry {
  // Name of a variable.
  const char *name;
  // Offset of the variable inside the memory area.
  uint64_t offset;
  // The number of elements inside this variable.
  uint64_t size;
  // Variable kind: 1 if it is a mutable variable, 0 otherwise.
  char kind;
} SymbolTableEntry;

// Type describing the config of a generated bundle.
typedef struct BundleConfig {
  // Size of the constant weight variables memory area.
  uint64_t constantWeightVarsMemSize;
  // Size of the mutable weight variables memory area.
  uint64_t mutableWeightVarsMemSize;
  // Size of the activations memory area.
  uint64_t activationsMemSize;
  // Alignment to be used for weights and activations.
  uint64_t alignment;
  // Number of symbols in the symbol table.
  uint64_t numSymbols;
  // Symbol table.
  const SymbolTableEntry *symbolTable;
} BundleConfig;

// Glow STATIC API

// Memory alignment definition with given alignment size
// for static allocation of memory.
#define GLOW_MEM_ALIGN(size)  __attribute__((aligned(size)))

// Macro function to get the absolute address of a
// placeholder using the base address of the mutable
// weight buffer and placeholder offset definition.
#define GLOW_GET_ADDR(mutableBaseAddr, placeholderOff)  (((uint8_t*)(mutableBaseAddr)) + placeholderOff)
#endif

#include "DCGAN-trained-dynamic/DCGAN_trained_dynamic.h"
#include "DCGAN-trained-static/DCGAN_trained_static.h"

GLOW_MEM_ALIGN(DCGAN_TRAINED_STATIC_MEM_ALIGN)
const static uint8_t constantWeight[DCGAN_TRAINED_STATIC_CONSTANT_MEM_SIZE] = {
  #include "DCGAN-trained-static/DCGAN_trained_static.weights.txt"
};

#define HEAP_START_ADDRESS_ (uint64_t)0x300000000
#define HEAP_LENGTH_ (uint64_t)(32 * 1024)

#define NZ 1
static const int IMG_SIZE = 8;
static const char *INPUT_VAR = "A0";
static const char *OUTPUT_VAR = "A12";


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
  const SymbolTableEntry *outputWeights = getMutableWeightVar(config, OUTPUT_VAR);
  float *results = (float *)(mutableWeightVars + outputWeights->offset);
  return results;
}

uint8_t *initMutableWeightVars(struct BumpAllocator *heap, const BundleConfig *config) {
  sol_log("Inside initMutableWeightVars");
  uint8_t *mutableWeightVarsAddr = allocateMutableWeightVars(heap, config);
  sol_log("Allocated mutableWeightVars");

  // A0 to somehow be computed dynamically or passed in from client
  float a0[NZ] = {0.47423121693043396,-0.5282680746784354,0.32941354491598535,0.8367122231652785,-0.9242990564027492,1.034541210871827,-1.3287637456932053,-0.41286506301773557,-0.09930055704305152,-0.013535512771139147,0.9553513527217113,0.05656428035571681,-0.36961798896662623,-0.027792667177632,1.5054505013132415,0.874266708141471,-0.8390414336155666,-0.98005548415165,0.6125328951111776,0.04891161400787087,0.5509755307746311,0.10761513330983134,0.1716293020433471,0.5920849565300466,0.14841621669549057,1.254210042600469,-0.3097030008239946,0.12622657155451214,-0.04191966271520088,0.9111061659699424,0.30841995722782156,0.09941792989307767,0.36701637160155554,-0.9348107950145716,-1.633922840448225,-1.024198976626713,1.0162380216217874,0.056417135986911275,0.979220166240875,-1.4611572977220904,1.0695556824495032,-1.1134468334133132,-0.05143702181003288,-1.8505955727315424,-0.11767996032387879,-0.3263432271774499,0.48315065605069574,0.5047781553059054,-0.8672224699703893,0.35264537905405263,0.8524724710093221,1.0021354808876095,-0.5382513178124323,-0.41645452549537326,0.2803919987764258,0.5372233499343916,-0.12937902659679293,-0.8550112233464158,-0.34291646334718895,0.5306289832589425,-0.019471483295322875,-0.4608088995925443,-0.5045502002322817,1.5956620820279201,-0.36358177887247156,-1.3270981022648056,-1.4161124144782749,0.4115769322290359,-1.4102690808657228,0.7245529137542382,-1.3946695326400453,-1.1008322524986809,1.300306151430027,-0.6081017588259483,-1.453615335295353,1.657788962868899,0.8492693074021435,-0.47499863433713146,0.7980171921422408,1.2100003914741897,-0.8563073701034595,-0.24149752651936698,0.4217386513885322,-0.24152538828018938,0.27352109588373613,0.8138689982095947,1.2152638544213743,-1.269303977369789,0.06558248832600873,0.15626193606349084,0.40544550925975403,1.1670524040056431,-0.7364212984521898,-0.43519686375709216,-0.30816363390091556,1.222397796189663,1.515498762364911,-0.6833775255503822,0.3592660130857487,-1.3153641497475044,};
  sol_log("Initialized a0 floats array");
  const SymbolTableEntry *inputA0Var = getMutableWeightVar(config, INPUT_VAR);
  sol_log("Retrieved a0 inputVar");
  sol_memcpy(mutableWeightVarsAddr + inputA0Var->offset, &a0, sizeof(float) * NZ);
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

  // The data must be large enough to hold an uint32_t value
  if (greeted_account->data_len < sizeof(uint32_t)) {
    sol_log("Greeted account data length too small to hold uint32_t value");
    return ERROR_INVALID_ACCOUNT_DATA;
  }

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
  
  float *output = getInferenceResults(&DCGAN_trained_dynamic_config, mutableWeightVarsAddr);
  float output_total = 0;
  for (int i = 0; i < IMG_SIZE * IMG_SIZE; i++) {
    output_total += output[i];
  }

  // Increment and store the vector total in the output
  float *account_total = (float *)greeted_account->data;
  *account_total = output_total;
  
  return ERROR_CUSTOM_ZERO;
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
