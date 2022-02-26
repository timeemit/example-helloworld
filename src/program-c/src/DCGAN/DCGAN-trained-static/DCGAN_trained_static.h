// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2022-02-25 (4a8350448) ()

#ifndef _GLOW_BUNDLE_DCGAN_TRAINED_STATIC_H
#define _GLOW_BUNDLE_DCGAN_TRAINED_STATIC_H

// ---------------------------------------------------------------
//                       Common definitions
// ---------------------------------------------------------------
#ifndef _GLOW_BUNDLE_COMMON_DEFS
#define _GLOW_BUNDLE_COMMON_DEFS

// Glow bundle error code for correct execution.
#define GLOW_SUCCESS 0

// Memory alignment definition with given alignment size
// for static allocation of memory.
#define GLOW_MEM_ALIGN(size)  __attribute__((aligned(size)))

// Macro function to get the absolute address of a
// placeholder using the base address of the mutable
// weight buffer and placeholder offset definition.
#define GLOW_GET_ADDR(mutableBaseAddr, placeholderOff)  (((uint8_t*)(mutableBaseAddr)) + placeholderOff)

#endif

// ---------------------------------------------------------------
//                          Bundle API
// ---------------------------------------------------------------
// Model name: "DCGAN_trained_static"
// Total data size: 12800 (bytes)
// Activations allocation efficiency: 1.0000
// Placeholders:
//
//   Name: "A0"
//   Type: float<1 x 1 x 1 x 1>
//   Size: 1 (elements)
//   Size: 4 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "A12"
//   Type: float<1 x 1 x 8 x 8>
//   Size: 64 (elements)
//   Size: 256 (bytes)
//   Offset: 64 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define DCGAN_TRAINED_STATIC_A0   0
#define DCGAN_TRAINED_STATIC_A12  64

// Memory sizes (bytes).
#define DCGAN_TRAINED_STATIC_CONSTANT_MEM_SIZE     10432
#define DCGAN_TRAINED_STATIC_MUTABLE_MEM_SIZE      320
#define DCGAN_TRAINED_STATIC_ACTIVATIONS_MEM_SIZE  2048

// Memory alignment (bytes).
#define DCGAN_TRAINED_STATIC_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int DCGAN_trained_static(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
