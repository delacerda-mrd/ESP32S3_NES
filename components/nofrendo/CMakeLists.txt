file(GLOB nofrendo_srcs
    "*.c"
    "cpu/*.c"
    "nes/*.c"
    "sndhrdw/*.c"
    "libsnss/*.c"
    "mappers/*.c"
)

idf_component_register(
    SRCS ${nofrendo_srcs}
    INCLUDE_DIRS "." "cpu" "nes" "sndhrdw" "libsnss" "mappers"
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=char-subscripts
    -Wno-error=implicit-fallthrough
    -Wno-error=unused-variable
    -Wno-error=pointer-sign
    -Wno-error=incompatible-pointer-types
    -Wno-implicit-function-declaration
)
