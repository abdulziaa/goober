cmake_minimum_required(VERSION 3.5)

if(ESP_PLATFORM) # don't run these lines of code on non-ESP platforms!
    idf_component_register(
        SRCS
            "goober.c"
    )
endif()

