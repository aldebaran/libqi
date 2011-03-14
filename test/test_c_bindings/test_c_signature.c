

//#include <gtest/gtest.h>
#include <qi/signature.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define check(retcode, lvl, str, ptr) \
{ \
  printf("current signature: %s, ret: %d\n", sig->current, ret); \
  printf(" level: %d, pointer: %d\n", sig->level, qi_signature_is_pointer(sig)); \
  assert(ret == retcode); \
  verify(sig, lvl, str, ptr); \
  printf("\n");\
  }

void verify(qi_signature_t *sig, int level, char *current, int ispointer)
{
  assert(sig);
  assert(sig->level == level);
  assert(!strcmp(sig->current, current));
  assert(qi_signature_is_pointer(sig) == ispointer);
}

//TEST(TestCSignature, BasicPrint) {
void test() {
  qi_signature_t *sig;
  int             ret;
  char           *signature = 0;

  printf("Signature: Us**S\n");
  signature = strdup("Us**S");
  sig = qi_signature_create(signature);
  ret = qi_signature_next(sig); check(2, -1, "Us**S", 0);
  ret = qi_signature_next(sig); check(2, -1, "Us**S", 0);
  qi_signature_destroy(sig);

  free(signature);
  printf("Signature: s\n");
  signature = strdup("s");
  sig = qi_signature_create(signature);
  ret = qi_signature_next(sig); check(0, 0, "s", 0);
  ret = qi_signature_next(sig); check(1, 0, "", 0);
  ret = qi_signature_next(sig); check(1, 0, "", 0);
  qi_signature_destroy(sig);

  free(signature);
  printf("Signature: s*\n");
  signature = strdup("s*");
  sig = qi_signature_create(signature);
  ret = qi_signature_next(sig); check(0, 0, "s*", 1);
  ret = qi_signature_next(sig); check(1, 0, "", 0);
  qi_signature_destroy(sig);

  free(signature);
  printf("Signature: s*[s]{ss}*\n");
  signature = strdup("s*[s]{ss}*");
  sig = qi_signature_create(signature);
  ret = qi_signature_next(sig); check(0, 0, "s*", 1);
  ret = qi_signature_next(sig); check(0, 0, "[s]", 0);
  ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(0, 0, "{ss}*", 1);
  ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(1, 0, "", 0);
  ret = qi_signature_next(sig); check(1, 0, "", 0);
  qi_signature_destroy(sig);

//  free(signature);
//  printf("Signature: s\n");

//  signature = strdup("s*[s*]*{s*s*}*");
//  ret = qi_signature_create(&sig, signature);
//  assert(ret == 0);
//  verify(&sig, 0, "s*", 1);

//  free(signature);
//  signature = strdup("s*[s*]*{s*[{s*s*}*]*}*");
}

int main(void) {
  test();
  return 0;
}
