idf_component_register(
    SRCS "ina.c"          # Укажите исходные файлы
    INCLUDE_DIRS "."       # Путь к заголовкам
    REQUIRES driver
)