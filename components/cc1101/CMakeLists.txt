set(component_srcs "cc1101.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS ".")
