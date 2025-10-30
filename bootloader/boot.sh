echo "Select what to do:"
echo "1) compilar bootloader"
echo "2) qemu "

read option

case $option in
    1) 
      nasm -fbin boot_sect_main.asm -o out/bootloader.bin
        ;;
    2) 
        qemu-system-x86_64 out/bootloader.bin -boot c
        ;;
    *) 
        echo "Opcao invalida"   
        ;;
esac
