#include "../drivers/screen.h"
#include "util.h"

void print_all_rows(void) {
  for (int i = 0; i < 24; i++) {
    char str[255];
    int_to_ascii(i, str);
    kprint_at(str, 0, i);
  }
  kprint_at("This text forces the kernel to scroll. Row 0 will disappear. ", 60, 24);
  kprint("And with this text, the kernel will scroll again, and row 1 will disappear too!");
}

void main() {
  clear_screen();
  // kprint_at("X", 1, 6);
  //kprint_at("Bem vindo!\n\n\n", 30, 1);
  //kprint("Esse OS esta sendo criado para estudo sobre Sistemas Opercionais "
  //       "e Web\n\n\nProximos passos: Implementar um dirver NIC-Ethernet");

  // kprint_at("This text spans multiple lines", 75, 1);
  //  kprint_at("There is a line\nbreak", 0, 20);
  //  kprint("There is a line\nbreak");
  // kprint_at("Gustavo H. Szesz", 50, 23);
  //kprint_at("github.com/gustavo-szesz", 50, 23);

  print_all_rows();
  print_all_rows();
  kprint("\nKernel execution completed.");
}
