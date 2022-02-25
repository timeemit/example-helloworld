// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2022-02-25 (4a8350448) ()

#ifndef _GLOW_BUNDLE_DCGAN_TRAINED_DYNAMIC_H
#define _GLOW_BUNDLE_DCGAN_TRAINED_DYNAMIC_H

// #include <stdint.h>

// ---------------------------------------------------------------
//                       Common definitions
// ---------------------------------------------------------------
#ifndef _GLOW_BUNDLE_COMMON_DEFS
#define _GLOW_BUNDLE_COMMON_DEFS

// Glow bundle error code for correct execution.
#define GLOW_SUCCESS 0

#endif

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

// ---------------------------------------------------------------
//                          Bundle API
// ---------------------------------------------------------------
// Model name: "DCGAN_trained_dynamic"
// Total data size: 1458432 (bytes)
// Activations allocation efficiency: 1.0000
// Placeholders:
//
//   Name: "A0"
//   Type: float<1 x 100 x 1 x 1>
//   Size: 100 (elements)
//   Size: 400 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "A21"
//   Type: float<1 x 3 x 16 x 16>
//   Size: 768 (elements)
//   Size: 3072 (bytes)
//   Offset: 448 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Bundle memory configuration (memory layout).
extern BundleConfig DCGAN_trained_dynamic_config;

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int DCGAN_trained_dynamic(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
