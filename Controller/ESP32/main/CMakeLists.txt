idf_component_register(
    SRCS 
    "main.c"
    
    "./libraries/AddressStrip/src/WS2812B.c"
    "./libraries/AddressStrip/src/WS2812B_Utils.c"
    "./libraries/AddressStrip/src/AddressStrip.c"
    "./libraries/FFT/src/FFT.c"

    "./modules/Periphery.c"
    "./modules/SPIFFS.c"
    "./modules/Settings.c"
    "./modules/BLEServer.c"
    "./modules/AudioProcess.c"
    
    INCLUDE_DIRS 
    "."
    "./libraries/AddressStrip/src/"
    "./libraries/FFT/src/"
    "./modules/"
    
    PRIV_REQUIRES
    REQUIRES
)

spiffs_create_partition_image(storage ../partition FLASH_IN_PROJECT)
