# DTA
MOV_ONLY/MOV отслеживает через команду ассемблера mov перемещение n помеченных байтов (задаётся через параметр командной строки), и определяет, какие байты попали в sink; ADD/ADD отслеживает через mov и add, попала ли информация из n помеченных байтов в sink

# Установка для Linux x86_64 
При установленной библиотеке QBDI:
```
mkdir build
cd build
cmake ..
cmake --build .
```
