#include <uefi.h>

int main(int argc, char **argv){
  efi_status_t status;
  efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  efi_gop_t *gop = NULL;

  status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);

  if (EFI_ERROR(status) || gop == NULL) {
      printf("GOP not available\n");
      return 1;
  }

  uint32_t width          = gop->Mode->Information->HorizontalResolution;
  uint32_t height         = gop->Mode->Information->VerticalResolution;
  uint32_t pixels_per_line = gop->Mode->Information->PixelsPerScanLine;
  uint32_t *framebuffer    = (uint32_t*) gop->Mode->FrameBufferBase;

  for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
          framebuffer[y * pixels_per_line + x] = 0x00223344; // azul escuro, 0x00RRGGBB
      }
  }

  while (1);
  return 0;

}
