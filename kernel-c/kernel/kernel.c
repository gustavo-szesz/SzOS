#include "../drivers/screen.h"

void main() {
  clear_screen();
  // kprint_at("X", 1, 6);
  kprint_at("Bem vindo, C!\n\n\n", 30, 1);
  kprint("Esse OS esta sendo criado para estudo sobre Sistemas Opercionais "
         "e Web\n\n\nProximos passos: Implementar um dirver NIC-Ethernet");

  // kprint_at("This text spans multiple lines", 75, 1);
  //  kprint_at("There is a line\nbreak", 0, 20);
  //  kprint("There is a line\nbreak");
  // kprint_at("Gustavo H. Szesz", 50, 23);
  kprint_at("github.com/gustavo-szesz", 50, 23);
}
