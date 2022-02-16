// Bundle API auto-generated header file. Do not edit!
// Glow Tools version: 2022-02-15 (5241e3c8c) ()

#ifndef _GLOW_BUNDLE_ADD_2INPUTS_3D_H
#define _GLOW_BUNDLE_ADD_2INPUTS_3D_H

// #include <stdint.h>

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
// Model name: "add_2inputs_3D"
// Total data size: 192 (bytes)
// Activations allocation efficiency: 0.0000
// Placeholders:
//
//   Name: "X"
//   Type: float<1 x 2 x 4>
//   Size: 8 (elements)
//   Size: 32 (bytes)
//   Offset: 0 (bytes)
//
//   Name: "Y"
//   Type: float<1 x 2 x 4>
//   Size: 8 (elements)
//   Size: 32 (bytes)
//   Offset: 64 (bytes)
//
//   Name: "Z"
//   Type: float<1 x 2 x 4>
//   Size: 8 (elements)
//   Size: 32 (bytes)
//   Offset: 128 (bytes)
//
// NOTE: Placeholders are allocated within the "mutableWeight"
// buffer and are identified using an offset relative to base.
// ---------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Placeholder address offsets within mutable buffer (bytes).
#define ADD_2INPUTS_3D_X  0
#define ADD_2INPUTS_3D_Y  64
#define ADD_2INPUTS_3D_Z  128

// Memory sizes (bytes).
#define ADD_2INPUTS_3D_CONSTANT_MEM_SIZE     0
#define ADD_2INPUTS_3D_MUTABLE_MEM_SIZE      192
#define ADD_2INPUTS_3D_ACTIVATIONS_MEM_SIZE  0

// Memory alignment (bytes).
#define ADD_2INPUTS_3D_MEM_ALIGN  64

// Bundle entry point (inference function). Returns 0
// for correct execution or some error code otherwise.
int add_2inputs_3D(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);

#ifdef __cplusplus
}
#endif
#endif
