/**
 * @brief C-based ONNX BPF program
 */
#include <solana_sdk.h>
#include "bundles-DCGAN-trained-dynamic/DCGAN_trained_dynamic.h"
#include "bundles-DCGAN-trained-static/DCGAN_trained_static.h"

GLOW_MEM_ALIGN(DCGAN_TRAINED_STATIC_MEM_ALIGN);
static const uint8_t constantWeight[DCGAN_TRAINED_STATIC_CONSTANT_MEM_SIZE] = {
  #include "bundles-DCGAN-trained-static/DCGAN_trained_static.weights.txt"
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

  float A0[64] = {0.9682414579846843,0.4481339758611895,-0.21684779752066421,-0.6259036638240845,0.6928551155266521,-0.5680150315342275,-0.3630327413743314,1.3187900306194995,0.34875318070557354,0.37449335858901117,-0.6447175505591933,-0.06939972166681546,-0.9593643831450365,-1.7357827264821168,0.10807244274762555,1.1086462291000239,0.7758008006908281,-1.2396190387549701,1.9388724271308517,-0.30446445269284056,0.7892547450432318,-1.2400015150423314,0.6172704963984191,-0.9448116305908916,0.02175708058959569,-1.1009891458915817,-0.1586483528973825,2.083149625035324,0.543028028331908,-1.0795153533935757,1.4471179625642339,-0.8115908980457811,0.9321504681915636,-0.061751450484871594,0.9928048268564711,-0.8060383819498921,1.098663722073884,-1.0191686715469959,1.4191238884976003,-1.0795833691461323,-0.544084417957494,0.9094919859997544,0.3512557862381586,1.2998994080515027,0.3200853321391851,0.08074969356220645,-0.47485497005854205,0.4857738145706917,1.8594731358865562,-1.0935485933254523,-0.1568911219258466,1.4441761102955806,-0.2576019284776974,-0.9781976287123395,-1.1144833379949541,0.3975267940249539,0.02984528074299735,-1.2156450174306734,-0.8165624413450648,-0.30421897145297716,0.09813807426077088,-0.8524258525025421,0.5393741155853573,-0.30866471418767216};

  sol_log("Generating");
  int errCode = DCGAN_trained_dynamic((uint8_t *) constantWeight, mutableWeight, activations);
  
  if (errCode != GLOW_SUCCESS) {
    sol_log("Glow Exception");
  }

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
