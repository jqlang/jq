/* For malloc() */
#include <stdlib.h>
/* For printf() */
#include <stdio.h>

#ifdef HAVE_UUID_H
#pragma message("using uuid/uuid.h")
#include <uuid/uuid.h>
#endif

// #ifdef __PYX_HAVE__getuuid
// #pragma message("using python to get uuid")
// #include <getuuid.h>
// #endif



#ifdef HAVE_UUID_H
char* get_uuid(void){
    unsigned char *uuid_binary_value = malloc(37);
    char *uuid_value = malloc(37);
    uuid_generate_random(uuid_binary_value);
    uuid_unparse(uuid_binary_value, uuid_value);
    return uuid_value;
}
#else
// #ifdef __PYX_HAVE__getuuid
// char* get_uuid(void){
//     char* uuid_value = __pyx_f_7getuuid_getuuid()
//     return uuid_value;
// }
// #else
#pragma message("using /dev/urandom to get uuid")
char* get_uuid(void) {

  FILE *fp;
  unsigned char *uuid_binary_value = malloc(37);
  fp = fopen("/dev/urandom", "rb");
  fread(uuid_binary_value, 1, 36, fp);
  fclose(fp);

  char *temp = malloc(37);
  
  for (int i = 0; i < 16; ++i){
      sprintf(temp+i*2, "%02X", uuid_binary_value[i]);
  }
  free(uuid_binary_value);

  char *uuid_value = malloc(37);
  sprintf(
        uuid_value, 
        "%.8s-%.4s-%.4s-%.4s-%s", 
        temp, 
        temp+8, 
        temp+12, 
        temp+16, 
        temp+20
  );
  free(temp);

  return uuid_value;
}
#endif
// #endif
