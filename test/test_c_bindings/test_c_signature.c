

//#include <gtest/gtest.h>
#include <qi/signature.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define check(retcode, str, ptr) \
{ \
  printf("current signature: %s, pointer: %d ret: %d\n", sig->current, qi_signature_is_pointer(sig), ret); \
  assert(ret == retcode); \
  verify(sig, str, ptr); \
  printf("\n");\
  }

void verify(qi_signature_t *sig, char *current, int ispointer)
{
  assert(sig);
  if (sig->current)
    assert(!strcmp(sig->current, current));
  else
    assert(sig->current == current);
  assert(qi_signature_is_pointer(sig) == ispointer);
}

//TEST(TestCSignature, BasicPrint) {
void test() {
  qi_signature_t *sig;
  int             ret;
  char           *signature = 0;

  printf("Signature: Us**S\n");
  sig = qi_signature_create("Us**S");
  ret = qi_signature_next(sig); check(2, 0, 0);
  ret = qi_signature_next(sig); check(2, 0, 0);
  qi_signature_destroy(sig);

  printf("Signature: s\n");
  sig = qi_signature_create("s");
  ret = qi_signature_next(sig); check(0, "s", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  qi_signature_destroy(sig);

  printf("Signature: s*\n");
  sig = qi_signature_create("s*");
  ret = qi_signature_next(sig); check(0, "s*", 1);
  ret = qi_signature_next(sig); check(1, "", 0);
  qi_signature_destroy(sig);

  printf("Signature: s*[s]{ss}*\n");
  sig = qi_signature_create("s*[s]{ss}*");
  ret = qi_signature_next(sig); check(0, "s*", 1);
  ret = qi_signature_next(sig); check(0, "[s]", 0);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(0, "{ss}*", 1);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  qi_signature_destroy(sig);


  printf("SubSignature: [s*[s]{ss}*]*\n");
  sig = qi_signature_create_subsignature("[s*[s]{ss}*]*");
  ret = qi_signature_next(sig); check(0, "s*", 1);
  ret = qi_signature_next(sig); check(0, "[s]", 0);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(0, "{ss}*", 1);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  //ret = qi_signature_next(sig); check(0, 1, "s", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  ret = qi_signature_next(sig); check(1, "", 0);
  qi_signature_destroy(sig);
}

int main(void) {
  test();
  return 0;
}
