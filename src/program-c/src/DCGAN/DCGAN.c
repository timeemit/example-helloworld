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
  const SymbolTableEntry *outputWeights = getMutableWeightVar(config, "A21");
  float *results = (float *)(mutableWeightVars + outputWeights->offset);
  return results;
}

uint8_t *initMutableWeightVars(struct BumpAllocator *heap, const BundleConfig *config) {
  sol_log("Inside initMutableWeightVars");
  uint8_t *mutableWeightVarsAddr = allocateMutableWeightVars(heap, config);
  sol_log("Allocated mutableWeightVars");

  // A0
  float a0[100] = {-0.330496591178819,-0.1231000694883476,-0.7984483569105248,0.22884050979484838,0.19133013653187203,0.6125432633506432,-0.3062297294509822,1.0621468486456525,-0.13854478723182956,1.6253202990605455,-1.2921923024748088,-0.8245019447922518,-0.3484332092407531,0.2515842402200945,0.5050110273556433,0.5261650965955069,0.5292802029457101,2.373710768054056,-0.901953394106964,0.27267471388643527,-0.4640220056850321,-0.9992854092485265,1.1362781567449485,-1.4728383260529165,0.5024673230453545,2.063521704044521,-0.8757086405834694,1.7561532306877148,0.4269235137119685,-1.0104569750473213,-0.0924002249646279,0.8661966923912441,0.7393806661354905,-0.450125815871679,-2.0960913742988887,2.145051266883868,-0.8189645021746585,0.30959437452794786,-1.1778617897532098,-1.3491713225138504,-0.3865572440052873,0.16604380514491116,-0.9422587782876317,0.8142563971080369,0.37041475511845906,-1.1040355694676451,1.0393968917802179,-1.0619365436731334,1.5533090604076045,1.4828508049133635,0.5840048993456167,-1.5936262016079425,1.8515082284477808,-0.9004774098499381,0.0337090547279996,-0.8119526798818553,-0.13969882957560703,0.8312266461792213,0.6961731096910785,0.20312120136308248,-0.502502685327624,-1.5199178763306262,-1.5413632731392575,-0.39145961377460975,-0.4081975588457413,0.6789438149132425,-0.308714334175275,-0.2556060510726535,-0.18526892439565112,0.5677979044395169,-0.4599260372134303,0.44453309055381685,0.068890016694916,-0.6585068953825021,-0.49670542930208583,-0.24238306853560115,-0.9012605220918253,-0.46096146827340856,-1.3935301779016442,-1.3042804557379828,-0.042897411974992423,0.9020564121431454,-0.9870285664204828,-0.9821673581175556,-1.2642175428463416,-2.109012466629439,-0.48006113342366163,-1.607553613374988,0.04795717422446833,1.64664335954184,-1.4103020146316114,-1.4000530910369868,-1.7079762446725424,-0.5044828787389366,-1.0949316318236406,0.8465237980666261,-1.978311213767623,1.8166799276628214,1.2458177364149676,0.16308031000744463,};
  sol_log("Initialized a0 floats array");
  const SymbolTableEntry *inputA0Var = getMutableWeightVar(config, "A0");
  sol_log("Retrieved a0 inputVar");
  sol_memcpy(mutableWeightVarsAddr + inputA0Var->offset, &a0, sizeof(float) * 100);
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
  for (int i = 0; i < 16 * 16; i++) {
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
